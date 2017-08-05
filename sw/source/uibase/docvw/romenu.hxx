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
#ifndef INCLUDED_SW_SOURCE_UIBASE_DOCVW_ROMENU_HXX
#define INCLUDED_SW_SOURCE_UIBASE_DOCVW_ROMENU_HXX

#include <editeng/brushitem.hxx>
#include <vcl/graph.hxx>
#include <vcl/menu.hxx>
#include <svl/stritem.hxx>

class SwView;
class SfxDispatcher;
class ImageMap;
class INetImage;

class SwReadOnlyPopup
{
    VclBuilder aBuilder;
    ScopedVclPtr<PopupMenu> xMenu;
    sal_uInt16 nReadonlyOpenurl;
    sal_uInt16 nReadonlyOpendoc;
    sal_uInt16 nReadonlyEditdoc;
    sal_uInt16 nReadonlySelectionMode;
    sal_uInt16 nReadonlyReload;
    sal_uInt16 nReadonlyReloadFrame;
    sal_uInt16 nReadonlySourceview;
    sal_uInt16 nReadonlyBrowseBackward;
    sal_uInt16 nReadonlyBrowseForward;
    sal_uInt16 nReadonlySaveGraphic;
    sal_uInt16 nReadonlyGraphictogallery;
    sal_uInt16 nReadonlyTogallerylink;
    sal_uInt16 nReadonlyTogallerycopy;
    sal_uInt16 nReadonlySaveBackground;
    sal_uInt16 nReadonlyBackgroundtogallery;
    sal_uInt16 nReadonlyBackgroundTogallerylink;
    sal_uInt16 nReadonlyBackgroundTogallerycopy;
    sal_uInt16 nReadonlyCopylink;
    sal_uInt16 nReadonlyCopyGraphic;
    sal_uInt16 nReadonlyLoadGraphic;
    sal_uInt16 nReadonlyGraphicoff;
    sal_uInt16 nReadonlyFullscreen;
    sal_uInt16 nReadonlyCopy;

    SwView &rView;
    SvxBrushItem aBrushItem;
    const Point &rDocPos;
                Graphic aGraphic;
    OUString    sURL,
                sTargetFrameName,
                sDescription;
    OUString    sGrfName;
    std::vector<OUString> aThemeList;
    bool        bGrfToGalleryAsLnk;
    ImageMap*   pImageMap;
    INetImage*  pTargetURL;

    void Check( sal_uInt16 nMID, sal_uInt16 nSID, SfxDispatcher const &rDis );
    OUString SaveGraphic( sal_uInt16 nId );

public:
    SwReadOnlyPopup(const Point &rDPos, SwView &rV);
    PopupMenu& GetMenu() const { return *xMenu; }
    ~SwReadOnlyPopup();

    void Execute( vcl::Window* pWin, const Point &rPPos );
    void Execute( vcl::Window* pWin, sal_uInt16 nId );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
