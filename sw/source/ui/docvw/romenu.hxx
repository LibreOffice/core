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
#ifndef _ROMENU_HXX
#define _ROMENU_HXX

#include <vcl/graph.hxx>
#include <tools/list.hxx>
#include <vcl/menu.hxx>
#include <svl/stritem.hxx>

class SwView;
class SfxDispatcher;
class SvxBrushItem;
class ImageMap;
class INetImage;

class SwReadOnlyPopup : public PopupMenu
{
          SwView &rView;
    const SvxBrushItem *pItem;
    const Point &rDocPos;
                Graphic aGraphic;
    String      sURL,
                sTargetFrameName,
                sDescription,
                sGrfName;
    List        aThemeList;
    BOOL        bGrfToGalleryAsLnk;
    ImageMap*   pImageMap;
    INetImage*  pTargetURL;

    void Check( USHORT nMID, USHORT nSID, SfxDispatcher &rDis );
    String SaveGraphic( USHORT nId );

    using PopupMenu::Execute;

public:
    SwReadOnlyPopup( const Point &rDPos, SwView &rV );
    ~SwReadOnlyPopup();

    void Execute( Window* pWin, const Point &rPPos );
    void Execute( Window* pWin, USHORT nId );
};

void GetPreferedExtension( String &rExt, const Graphic &rGrf );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
