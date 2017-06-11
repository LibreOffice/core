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

#ifndef INCLUDED_SVX_GALLERY_HXX
#define INCLUDED_SVX_GALLERY_HXX

#include <svx/svxdllapi.h>
#include <tools/urlobj.hxx>
#include <vector>

// Defines for preinstalled themes
#define GALLERY_THEME_3D                1
#define GALLERY_THEME_BULLETS           3
#define GALLERY_THEME_HOMEPAGE          10
#define GALLERY_THEME_HTMLBUTTONS       15
#define GALLERY_THEME_POWERPOINT        16
#define GALLERY_THEME_RULERS            17
#define GALLERY_THEME_SOUNDS            18
#define GALLERY_THEME_MYTHEME           20
#define GALLERY_THEME_USERSOUNDS        21
#define GALLERY_THEME_DUMMY5            36
#define GALLERY_THEME_FONTWORK          37
#define GALLERY_THEME_FONTWORK_VERTICAL 38

#define RID_GALLERY_THEME_START          (6000)

// ATTENTION: This list has to be in sync with
//  * share/gallery (theme files are identified by id)
//
// Since galleries may be copied from older offices to newer, do *never* change
// this IDs except adding new ones (and adapting GALLERY_THEME_LAST). The ID
// *is* written into the binary file *.thm (which is a gallery theme combined
// of three files, *.thm, *.sdv and *.sdg)

#define RID_GALLERY_THEME_3D             (RID_GALLERY_THEME_START + 1)
#define RID_GALLERY_THEME_ANIMATIONS     (RID_GALLERY_THEME_START + 2)
#define RID_GALLERY_THEME_BULLETS        (RID_GALLERY_THEME_START + 3)
#define RID_GALLERY_THEME_OFFICE         (RID_GALLERY_THEME_START + 4)
#define RID_GALLERY_THEME_FLAGS          (RID_GALLERY_THEME_START + 5)
#define RID_GALLERY_THEME_FLOWCHARTS     (RID_GALLERY_THEME_START + 6)
#define RID_GALLERY_THEME_EMOTICONS      (RID_GALLERY_THEME_START + 7)
#define RID_GALLERY_THEME_PHOTOS         (RID_GALLERY_THEME_START + 8)
#define RID_GALLERY_THEME_BACKGROUNDS    (RID_GALLERY_THEME_START + 9)
#define RID_GALLERY_THEME_HOMEPAGE       (RID_GALLERY_THEME_START + 10)
#define RID_GALLERY_THEME_INTERACTION    (RID_GALLERY_THEME_START + 11)
#define RID_GALLERY_THEME_MAPS           (RID_GALLERY_THEME_START + 12)
#define RID_GALLERY_THEME_PEOPLE         (RID_GALLERY_THEME_START + 13)
#define RID_GALLERY_THEME_SURFACES       (RID_GALLERY_THEME_START + 14)

#define RID_GALLERY_THEME_SOUNDS         (RID_GALLERY_THEME_START + 18)
#define RID_GALLERY_THEME_SYMBOLS        (RID_GALLERY_THEME_START + 19)
#define RID_GALLERY_THEME_MYTHEME        (RID_GALLERY_THEME_START + 20)

#define RID_GALLERY_THEME_ARROWS         (RID_GALLERY_THEME_START + 22)
#define RID_GALLERY_THEME_BALLOONS       (RID_GALLERY_THEME_START + 23)
#define RID_GALLERY_THEME_KEYBOARD       (RID_GALLERY_THEME_START + 24)
#define RID_GALLERY_THEME_TIME           (RID_GALLERY_THEME_START + 25)
#define RID_GALLERY_THEME_PRESENTATION   (RID_GALLERY_THEME_START + 26)
#define RID_GALLERY_THEME_CALENDAR       (RID_GALLERY_THEME_START + 27)
#define RID_GALLERY_THEME_NAVIGATION     (RID_GALLERY_THEME_START + 28)
#define RID_GALLERY_THEME_COMMUNICATION  (RID_GALLERY_THEME_START + 29)
#define RID_GALLERY_THEME_FINANCES       (RID_GALLERY_THEME_START + 30)
#define RID_GALLERY_THEME_COMPUTER       (RID_GALLERY_THEME_START + 31)
#define RID_GALLERY_THEME_CLIMA          (RID_GALLERY_THEME_START + 32)
#define RID_GALLERY_THEME_EDUCATION      (RID_GALLERY_THEME_START + 33)
#define RID_GALLERY_THEME_TROUBLE        (RID_GALLERY_THEME_START + 34)
#define RID_GALLERY_THEME_SCREENBEANS    (RID_GALLERY_THEME_START + 35)

#define RID_GALLERY_THEME_COMPUTERS      (RID_GALLERY_THEME_START + 39)
#define RID_GALLERY_THEME_DIAGRAMS       (RID_GALLERY_THEME_START + 40)
#define RID_GALLERY_THEME_ENVIRONMENT    (RID_GALLERY_THEME_START + 41)
#define RID_GALLERY_THEME_FINANCE        (RID_GALLERY_THEME_START + 42)
#define RID_GALLERY_THEME_TRANSPORT      (RID_GALLERY_THEME_START + 43)
#define RID_GALLERY_THEME_TXTSHAPES      (RID_GALLERY_THEME_START + 44)

#define RID_GALLERY_THEME_FIRST          RID_GALLERY_THEME_3D
#define RID_GALLERY_THEME_LAST           RID_GALLERY_THEME_TXTSHAPES

class Gallery;
class VCDrawModel;
class FmFormModel;
class SdrModel;
class Graphic;
class FmFormModel;
class BitmapEx;
class OutputDevice;

class SVX_DLLPUBLIC GalleryExplorer
{
public:

    static bool                 FillThemeList( std::vector<OUString>& rThemeList );

                                // FillObjList is filling rObjList with Strings of the internal Gallery Object URL
    static bool                 FillObjList( const OUString& rThemeName, std::vector<OUString> &rObjList );
    static bool                 FillObjList( const sal_uInt32 nThemeId, std::vector<OUString> &rObjList );

                                // FillObjTitleList is filling the rList with the title for each gallery object
    static bool                 FillObjListTitle( const sal_uInt32 nThemeId, std::vector< OUString >& rList );

    static bool                 InsertURL( const OUString& rThemeName, const OUString& rURL );
    static bool                 InsertURL( sal_uInt32 nThemeId, const OUString& rURL );

    static bool                 GetGraphicObj( const OUString& rThemeName, sal_uInt32 nPos,
                                               Graphic* pGraphic, BitmapEx* pThumb = nullptr,
                                               bool bProgress = false );
    static bool                 GetGraphicObj( sal_uInt32 nThemeId, sal_uInt32 nPos,
                                               Graphic* pGraphic, BitmapEx* pThumb = nullptr,
                                               bool bProgress = false );

    static sal_uInt32           GetSdrObjCount( const OUString& rThemeName );
    static sal_uInt32           GetSdrObjCount( sal_uInt32 nThemeId );

    static bool                 GetSdrObj( const OUString& rThemeName, sal_uInt32 nSdrModelPos,
                                           SdrModel* pModel, BitmapEx* pThumb = nullptr );
    static bool                 GetSdrObj( sal_uInt32 nThemeId, sal_uInt32 nSdrModelPos,
                                           SdrModel* pModel, BitmapEx* pThumb = nullptr );

    static bool                 BeginLocking( const OUString& rThemeName );
    static bool                 BeginLocking( sal_uInt32 nThemeId );

    static bool                 EndLocking( const OUString& rThemeName );
    static bool                 EndLocking( sal_uInt32 nThemeId );
};

#endif // INCLUDED_SVX_GALLERY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
