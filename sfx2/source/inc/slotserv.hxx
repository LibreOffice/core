/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SFX2_SOURCE_INC_SLOTSERV_HXX
#define INCLUDED_SFX2_SOURCE_INC_SLOTSERV_HXX

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
};

inline SfxSlotServer::SfxSlotServer( const SfxSlot &rSlot, sal_uInt16 nShell ):
    _pSlot( &rSlot),
    _nShellLevel( nShell )
{
}

inline SfxSlotServer::SfxSlotServer():
    _pSlot(nullptr),
    _nShellLevel(0)
{
}

inline sal_uInt16 SfxSlotServer::GetShellLevel() const
{
    return _nShellLevel;
}

inline const SfxSlot* SfxSlotServer::GetSlot() const
{
    return _pSlot;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
