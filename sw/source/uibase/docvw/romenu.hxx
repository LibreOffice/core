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
#include <vcl/builder.hxx>
#include <vcl/graph.hxx>
#include <vcl/menu.hxx>

class SwView;
class SfxDispatcher;
class ImageMap;
class INetImage;

class SwReadOnlyPopup
{
    VclBuilder m_aBuilder;
    ScopedVclPtr<PopupMenu> m_xMenu;
    sal_uInt16 const m_nReadonlyOpenurl;
    sal_uInt16 const m_nReadonlyOpendoc;
    sal_uInt16 const m_nReadonlyEditdoc;
    sal_uInt16 const m_nReadonlySelectionMode;
    sal_uInt16 const m_nReadonlyReload;
    sal_uInt16 const m_nReadonlyReloadFrame;
    sal_uInt16 const m_nReadonlySourceview;
    sal_uInt16 const m_nReadonlyBrowseBackward;
    sal_uInt16 const m_nReadonlyBrowseForward;
    sal_uInt16 const m_nReadonlySaveGraphic;
    sal_uInt16 const m_nReadonlyGraphictogallery;
    sal_uInt16 const m_nReadonlyTogallerylink;
    sal_uInt16 const m_nReadonlyTogallerycopy;
    sal_uInt16 const m_nReadonlySaveBackground;
    sal_uInt16 const m_nReadonlyBackgroundtogallery;
    sal_uInt16 const m_nReadonlyBackgroundTogallerylink;
    sal_uInt16 const m_nReadonlyBackgroundTogallerycopy;
    sal_uInt16 const m_nReadonlyCopylink;
    sal_uInt16 const m_nReadonlyLoadGraphic;
    sal_uInt16 const m_nReadonlyGraphicoff;
    sal_uInt16 const m_nReadonlyFullscreen;
    sal_uInt16 const m_nReadonlyCopy;

    SwView &m_rView;
    std::shared_ptr<SvxBrushItem> m_aBrushItem;
                Graphic m_aGraphic;
    OUString    m_sURL,
                m_sTargetFrameName;
    OUString    m_sGrfName;
    std::vector<OUString> m_aThemeList;
    bool        m_bGrfToGalleryAsLnk;

    void Check( sal_uInt16 nMID, sal_uInt16 nSID, SfxDispatcher const &rDis );
    OUString SaveGraphic( sal_uInt16 nId );

public:
    SwReadOnlyPopup(const Point &rDPos, SwView &rV);
    PopupMenu& GetMenu() const { return *m_xMenu; }
    ~SwReadOnlyPopup();

    void Execute( vcl::Window* pWin, const Point &rPPos );
    void Execute( vcl::Window* pWin, sal_uInt16 nId );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
