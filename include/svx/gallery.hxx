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

#ifndef _GALLERY_HXX_
#define _GALLERY_HXX_

#include <tools/solar.h>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <vector>
#include "svx/svxdllapi.h"

// Defines for preinstalled themes
// Has to be in sync with svx/inc/galtheme.hrc
#define GALLERY_THEME_3D                1
#define GALLERY_THEME_BULLETS       3
#define GALLERY_THEME_HOMEPAGE      10
#define GALLERY_THEME_HTMLBUTTONS   15
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

    static bool                 FillThemeList( std::vector<String>& rThemeList );

                                // FillObjList is filling rObjList with Strings of the internal Gallery Object URL
    static sal_Bool                 FillObjList( const OUString& rThemeName, std::vector<String> &rObjList );
    static sal_Bool                 FillObjList( const sal_uInt32 nThemeId, std::vector<String> &rObjList );
    static bool                 FillObjList( const sal_uInt32 nThemeId, std::vector<OUString> &rObjList );

                                // FillObjTitleList is filling the rList with the title for each gallery object
    static sal_Bool             FillObjListTitle( const sal_uInt32 nThemeId, std::vector< OUString >& rList );

    static sal_Bool                 InsertURL( const OUString& rThemeName, const OUString& rURL );
    static sal_Bool                 InsertURL( sal_uIntPtr nThemeId, const OUString& rURL );

    static sal_Bool                 GetGraphicObj( const OUString& rThemeName, sal_uIntPtr nPos,
                                               Graphic* pGraphic = NULL, BitmapEx* pThumb = NULL,
                                               sal_Bool bProgess = sal_False );
    static sal_Bool                 GetGraphicObj( sal_uIntPtr nThemeId, sal_uIntPtr nPos,
                                               Graphic* pGraphic = NULL, BitmapEx* pThumb = NULL,
                                               sal_Bool bProgess = sal_False );

    static sal_uIntPtr              GetSdrObjCount( const OUString& rThemeName );
    static sal_uIntPtr              GetSdrObjCount( sal_uIntPtr nThemeId );

    static sal_Bool                 GetSdrObj( const OUString& rThemeName, sal_uIntPtr nSdrModelPos,
                                           SdrModel* pModel = NULL, BitmapEx* pThumb = NULL );
    static sal_Bool                 GetSdrObj( sal_uIntPtr nThemeId, sal_uIntPtr nSdrModelPos,
                                           SdrModel* pModel = NULL, BitmapEx* pThumb = NULL );

    static sal_Bool                 BeginLocking( const OUString& rThemeName );
    static sal_Bool                 BeginLocking( sal_uIntPtr nThemeId );

    static sal_Bool                 EndLocking( const OUString& rThemeName );
    static sal_Bool                 EndLocking( sal_uIntPtr nThemeId );
};

#endif // _GALLERY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
