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

#ifndef _SV_SALMENU_H
#define _SV_SALMENU_H

#include <tools/solar.h>
#include <vcl/bitmap.hxx>
#include <salmenu.hxx>


class X11SalMenu : public SalMenu
{
public:
    X11SalMenu() {}
    virtual ~X11SalMenu();

    virtual sal_Bool VisibleMenuBar();  // must return TRUE to actually DISPLAY native menu bars
                            // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos );
    virtual void RemoveItem( unsigned nPos );
    virtual void SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos );
    virtual void SetFrame( const SalFrame* pFrame );
    virtual void CheckItem( unsigned nPos, sal_Bool bCheck );
    virtual void EnableItem( unsigned nPos, sal_Bool bEnable );
    virtual void SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const OUString& rText );
    virtual void SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage);
    virtual void SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const OUString& rKeyName );
    virtual void GetSystemMenuData( SystemMenuData* pData );
};

class X11SalMenuItem : public SalMenuItem
{
public:
    X11SalMenuItem() {}
        virtual ~X11SalMenuItem();
};

#endif // _SV_SALMENU_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
