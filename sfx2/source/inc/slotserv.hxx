/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SFXSLOTSERV_HXX
#define _SFXSLOTSERV_HXX

#include <tools/solar.h>

class SfxSlot;

class SfxSlotServer
{
private:
    const SfxSlot*      _pSlot;
    sal_uInt16              _nShellLevel;

public:
                        SfxSlotServer( const SfxSlot &rSlot, sal_uInt16 nShell );
                        SfxSlotServer();

    sal_uInt16              GetShellLevel() const;
    void                SetShellLevel(sal_uInt16 nLevel) { _nShellLevel = nLevel; }
    void                SetSlot(const SfxSlot* pSlot) {
                            _pSlot = pSlot;
                        }
    const SfxSlot*      GetSlot() const;
    void                Invalidate() { _pSlot = 0; }
};

//--------------------------------------------------------------------

inline SfxSlotServer::SfxSlotServer( const SfxSlot &rSlot, sal_uInt16 nShell ):
    _pSlot( &rSlot),
    _nShellLevel( nShell )
{
}

//--------------------------------------------------------------------

inline SfxSlotServer::SfxSlotServer():
    _pSlot(0),
    _nShellLevel(0)
{
}

//--------------------------------------------------------------------

inline sal_uInt16 SfxSlotServer::GetShellLevel() const
{
    return _nShellLevel;
}

//--------------------------------------------------------------------

inline const SfxSlot* SfxSlotServer::GetSlot() const
{
    return _pSlot;
}


#endif
