////////////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License                                                           //
//                                                                                //
// Copyright (c) 2018, pa.eeapai@gmail.com                                        //
// All rights reserved.                                                           //
//                                                                                //
// Redistribution and use in source and binary forms, with or without             //
// modification, are permitted provided that the following conditions are met:    //
//                                                                                //
// * Redistributions of source code must retain the above copyright notice, this  //
//   list of conditions and the following disclaimer.                             //
//                                                                                //
// * Redistributions in binary form must reproduce the above copyright notice,    //
//   this list of conditions and the following disclaimer in the documentation    //
//   and/or other materials provided with the distribution.                       //
//                                                                                //
// * Neither the name of the copyright holder nor the names of its                //
//   contributors may be used to endorse or promote products derived from         //
//   this software without specific prior written permission.                     //
//                                                                                //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"    //
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE      //
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE //
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE   //
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL     //
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR     //
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER     //
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,  //
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  //
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.           //
////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "WinCommFactory.h"

static HMODULE s_hCommDLL = NULL;

typedef int (fnListConnection)(EWinCommType eOfType, unsigned long dwConnection, char * pszDestination, int nMaxAddressLength);
static fnListConnection *s_pfnListConnection = nullptr;

typedef ICommDevice * (fnCreateCommDevice)(EWinCommType eOfType);
static fnCreateCommDevice *s_pfnCreateCommDevice = nullptr;

typedef void (fnDestroyCommDevice)(ICommDevice *pCommDevice);
static fnDestroyCommDevice *s_pfnDestroyCommDevice = nullptr;

static bool s_ensureCommDllLoaded()
{
  if ( !s_hCommDLL )
  {
    s_hCommDLL = ::LoadLibraryA("WinComm.dll");
  }
  if ( !s_hCommDLL )
  {
    return false;
  }

  if ( !s_pfnListConnection )
  {
    s_pfnListConnection = (fnListConnection *)GetProcAddress(s_hCommDLL, "ListConnection");
  }
  if ( !s_pfnListConnection )
  {
    return false;
  }

  if ( !s_pfnCreateCommDevice )
  {
    s_pfnCreateCommDevice = (fnCreateCommDevice *)GetProcAddress(s_hCommDLL, "CreateCommDevice");
  }
  if ( !s_pfnCreateCommDevice )
  {
    return false;
  }

  if ( !s_pfnDestroyCommDevice )
  {
    s_pfnDestroyCommDevice = (fnDestroyCommDevice *)GetProcAddress(s_hCommDLL, "DestroyCommDevice");
  }
  if ( !s_pfnDestroyCommDevice )
  {
    return false;
  }
  
  return true;
}

void CWinCommDevice::SetType(EWinCommType eType)
{
  if ( m_eType != eType )
  {
    destroy();
  }
  m_eType = eType;
}

int CWinCommDevice::ListConnection(EWinCommType eOfType, unsigned long dwConnection, char * pszDestination, int nMaxAddressLength)
{
  if ( !s_ensureCommDllLoaded() )
  {
    return -1;
  }
  return s_pfnListConnection(eOfType, dwConnection, pszDestination, nMaxAddressLength);
}
void CWinCommDevice::create()
{
  if ( !s_ensureCommDllLoaded() )
  {
    return;
  }

  m_pCommDevice = s_pfnCreateCommDevice(m_eType);
}

void CWinCommDevice::destroy()
{
  if ( !s_ensureCommDllLoaded() )
  {
    return;
  }

  if ( m_pCommDevice )
  {
    s_pfnDestroyCommDevice(m_pCommDevice);
    m_pCommDevice = nullptr;
  }
}

void CWinCommDevice::Connect(const char * pcszWhereTo)
{
  if ( GetStatus() )
  {
    Disconnect();
  }
  if ( !m_pCommDevice )
  {
    create();
  }
  if ( m_pCommDevice )
  {
    m_pCommDevice->Connect(pcszWhereTo);
  }
}

int CWinCommDevice::GetStatus() const
{
  if ( m_pCommDevice )
  {
    return m_pCommDevice->GetStatus();
  }
  return ICommDevice::connectionDown;
}

uint32_t CWinCommDevice::Send(const void *pSource, uint32_t dwByteCount)
{
  if ( m_pCommDevice )
  {
    return m_pCommDevice->Send(pSource, dwByteCount);
  }
  return 0;
}

uint32_t CWinCommDevice::Receive(void *pDestination, uint32_t dwMaxByteCount)
{
  if ( m_pCommDevice )
  {
    return m_pCommDevice->Receive(pDestination, dwMaxByteCount);
  }
  return 0;
}

void CWinCommDevice::Disconnect()
{
  if ( m_pCommDevice )
  {
    m_pCommDevice->Disconnect();
  }
}
