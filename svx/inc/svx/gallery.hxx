/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _GALLERY_HXX_
#define _GALLERY_HXX_

#include <tools/solar.h>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <vector>
#include "svx/svxdllapi.h"

// -----------
// - Defines -
// -----------

// Defines for preinstalled themes
#define GALLERY_THEME_3D                1
#define GALLERY_THEME_ANIMATIONS    2
#define GALLERY_THEME_BULLETS       3
#define GALLERY_THEME_OFFICE            4
#define GALLERY_THEME_FLAGS             5
#define GALLERY_THEME_FLOWCHARTS    6
#define GALLERY_THEME_EMOTICONS         7
#define GALLERY_THEME_PHOTOS        8
#define GALLERY_THEME_BACKGROUNDS   9
#define GALLERY_THEME_HOMEPAGE      10
#define GALLERY_THEME_INTERACTION   11
#define GALLERY_THEME_MAPS              12
#define GALLERY_THEME_PEOPLE            13
#define GALLERY_THEME_SURFACES      14
#define GALLERY_THEME_HTMLBUTTONS   15
#define GALLERY_THEME_POWERPOINT    16
#define GALLERY_THEME_RULERS        17
#define GALLERY_THEME_SOUNDS        18
#define GALLERY_THEME_SYMBOLS       19
#define GALLERY_THEME_MYTHEME       20
#define GALLERY_THEME_USERSOUNDS    21
#define GALLERY_THEME_ARROWS            22
#define GALLERY_THEME_BALLOONS          23
#define GALLERY_THEME_KEYBOARD          24
#define GALLERY_THEME_TIME              25
#define GALLERY_THEME_PRESENTATION      26
#define GALLERY_THEME_CALENDAR          27
#define GALLERY_THEME_NAVIGATION        28
#define GALLERY_THEME_COMMUNICATION     29
#define GALLERY_THEME_FINANCES          30
#define GALLERY_THEME_COMPUTER          31
#define GALLERY_THEME_CLIMA             32
#define GALLERY_THEME_EDUCATION         33
#define GALLERY_THEME_TROUBLE           34
#define GALLERY_THEME_SCREENBEANS       35
#define GALLERY_THEME_FONTWORK          36
#define GALLERY_THEME_FONTWORK_VERTICAL 37

#define GALLERY_THEME_DUMMY5            38

// -------------------
// - GalleryExplorer -
// -------------------

class List;
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

    static sal_Bool                 FillThemeList( List& rThemeList );

                                // FillObjList is filling rObjList with Strings of the internal Gallery Object URL
    static sal_Bool                 FillObjList( const String& rThemeName, List& rObjList );
    static sal_Bool                 FillObjList( sal_uIntPtr nThemeId, List& rObjList );

                                // FillObjTitleList is filling the rList with the title for each gallery object
    static sal_Bool             FillObjListTitle( const sal_uInt32 nThemeId, std::vector< rtl::OUString >& rList );

    static sal_Bool                 InsertURL( const String& rThemeName, const String& rURL );
    static sal_Bool                 InsertURL( sal_uIntPtr nThemeId, const String& rURL );

    static sal_uIntPtr              GetObjCount( const String& rThemeName );
    static sal_uIntPtr              GetObjCount( sal_uIntPtr nThemeId );

    static sal_Bool                 GetGraphicObj( const String& rThemeName, sal_uIntPtr nPos,
                                               Graphic* pGraphic = NULL, BitmapEx* pThumb = NULL,
                                               sal_Bool bProgess = sal_False );
    static sal_Bool                 GetGraphicObj( sal_uIntPtr nThemeId, sal_uIntPtr nPos,
                                               Graphic* pGraphic = NULL, BitmapEx* pThumb = NULL,
                                               sal_Bool bProgess = sal_False );

    static sal_Bool                 InsertGraphicObj( const String& rThemeName, const Graphic& rGraphic );
    static sal_Bool                 InsertGraphicObj( sal_uIntPtr nThemeId, const Graphic& rGraphic );

    static sal_uIntPtr              GetSdrObjCount( const String& rThemeName );
    static sal_uIntPtr              GetSdrObjCount( sal_uIntPtr nThemeId );

    static sal_Bool                 GetSdrObj( const String& rThemeName, sal_uIntPtr nSdrModelPos,
                                           SdrModel* pModel = NULL, BitmapEx* pThumb = NULL );
    static sal_Bool                 GetSdrObj( sal_uIntPtr nThemeId, sal_uIntPtr nSdrModelPos,
                                           SdrModel* pModel = NULL, BitmapEx* pThumb = NULL );

    static sal_Bool                 InsertSdrObj( const String& rThemeName, FmFormModel& rModel );
    static sal_Bool                 InsertSdrObj( sal_uIntPtr nThemeId, FmFormModel& rModel );

    static sal_Bool                 BeginLocking( const String& rThemeName );
    static sal_Bool                 BeginLocking( sal_uIntPtr nThemeId );

    static sal_Bool                 EndLocking( const String& rThemeName );
    static sal_Bool                 EndLocking( sal_uIntPtr nThemeId );
};

#endif // _GALLERY_HXX_
