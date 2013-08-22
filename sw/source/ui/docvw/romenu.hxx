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
#ifndef _ROMENU_HXX
#define _ROMENU_HXX

#include <vcl/graph.hxx>
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
                sDescription;
    OUString    sGrfName;
    std::vector<String> aThemeList;
    sal_Bool        bGrfToGalleryAsLnk;
    ImageMap*   pImageMap;
    INetImage*  pTargetURL;

    void Check( sal_uInt16 nMID, sal_uInt16 nSID, SfxDispatcher &rDis );
    String SaveGraphic( sal_uInt16 nId );

    using PopupMenu::Execute;

public:
    SwReadOnlyPopup( const Point &rDPos, SwView &rV );
    ~SwReadOnlyPopup();

    void Execute( Window* pWin, const Point &rPPos );
    void Execute( Window* pWin, sal_uInt16 nId );
};

void GetPreferedExtension( String &rExt, const Graphic &rGrf );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
