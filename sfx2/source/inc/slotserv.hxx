/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFXSLOTSERV_HXX
#define _SFXSLOTSERV_HXX

#include <tools/solar.h>

class SfxSlot;

class SfxSlotServer
{
private:
    const SfxSlot*      _pSlot;
    USHORT              _nShellLevel;

public:
                        SfxSlotServer( const SfxSlot &rSlot, USHORT nShell );
                        SfxSlotServer();

    USHORT              GetShellLevel() const;
    void                SetShellLevel(USHORT nLevel) { _nShellLevel = nLevel; }
    void                SetSlot(const SfxSlot* pSlot) {
                            _pSlot = pSlot;
                        }
    const SfxSlot*      GetSlot() const;
    void                Invalidate() { _pSlot = 0; }
};

//--------------------------------------------------------------------

inline SfxSlotServer::SfxSlotServer( const SfxSlot &rSlot, USHORT nShell ):
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

inline USHORT SfxSlotServer::GetShellLevel() const
{
    return _nShellLevel;
}

//--------------------------------------------------------------------

inline const SfxSlot* SfxSlotServer::GetSlot() const
{
    return _pSlot;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
