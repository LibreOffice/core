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
// Has to be in sync with svx/inc/galtheme.hrc
#define GALLERY_THEME_3D                1
#define GALLERY_THEME_BULLETS       3
#define GALLERY_THEME_HOMEPAGE      10
#define GALLERY_THEME_POWERPOINT    16
#define GALLERY_THEME_SOUNDS        18
#define GALLERY_THEME_USERSOUNDS    21

#define GALLERY_THEME_FONTWORK          37
#define GALLERY_THEME_FONTWORK_VERTICAL 38

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
