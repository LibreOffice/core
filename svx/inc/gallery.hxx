/*************************************************************************
 *
 *  $RCSfile: gallery.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 14:42:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _GALLERY_HXX_
#define _GALLERY_HXX_

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#include <vector>

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// -----------
// - Defines -
// -----------

#define SGA_FORMAT_NONE     0x00000000L
#define SGA_FORMAT_STRING   0x00000001L
#define SGA_FORMAT_GRAPHIC  0x00000010L
#define SGA_FORMAT_SOUND    0x00000100L
#define SGA_FORMAT_OLE          0x00001000L
#define SGA_FORMAT_SVDRAW   0x00010000L
#define SGA_FORMAT_ALL      0xFFFFFFFFL

#define SVX_GALLERY() (GalleryExplorer::GetGallery())

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
class Graphic;
class FmFormModel;
class Bitmap;
class OutputDevice;

class SVX_DLLPUBLIC GalleryExplorer
{
private:

    SVX_DLLPRIVATE static Gallery*              ImplGetGallery();

public:

    static GalleryExplorer*     GetGallery();

public:

    INetURLObject               GetURL() const;
    String                      GetFilterName() const;
    Graphic                     GetGraphic() const;
    BOOL                        GetVCDrawModel( FmFormModel& rModel ) const;
    BOOL                        IsLinkage() const;

    static BOOL                 FillThemeList( List& rThemeList );

                                // FillObjList is filling rObjList with Strings of the internal Gallery Object URL
    static BOOL                 FillObjList( const String& rThemeName, List& rObjList );
    static BOOL                 FillObjList( ULONG nThemeId, List& rObjList );

                                // FillObjTitleList is filling the rList with the title for each gallery object
    static sal_Bool             FillObjListTitle( const sal_uInt32 nThemeId, std::vector< rtl::OUString >& rList );

    static BOOL                 InsertURL( const String& rThemeName, const String& rURL );
    static BOOL                 InsertURL( ULONG nThemeId, const String& rURL );

    static BOOL                 InsertURL( const String& rThemeName, const String& rURL,
                                           const ULONG nSgaFormat /* = SGA_FORMAT_ALL */ );
    static BOOL                 InsertURL( ULONG nThemeId, const String& rURL,
                                           const ULONG nSgaFormat /* = SGA_FORMAT_ALL */ );

    static ULONG                GetObjCount( const String& rThemeName );
    static ULONG                GetObjCount( ULONG nThemeId );

    static BOOL                 GetGraphicObj( const String& rThemeName, ULONG nPos,
                                               Graphic* pGraphic = NULL, Bitmap* pThumb = NULL,
                                               BOOL bProgess = FALSE );
    static BOOL                 GetGraphicObj( ULONG nThemeId, ULONG nPos,
                                               Graphic* pGraphic = NULL, Bitmap* pThumb = NULL,
                                               BOOL bProgess = FALSE );

    static BOOL                 InsertGraphicObj( const String& rThemeName, const Graphic& rGraphic );
    static BOOL                 InsertGraphicObj( ULONG nThemeId, const Graphic& rGraphic );

    static ULONG                GetSdrObjCount( const String& rThemeName );
    static ULONG                GetSdrObjCount( ULONG nThemeId );

    static BOOL                 GetSdrObj( const String& rThemeName, ULONG nSdrModelPos,
                                           FmFormModel* pModel = NULL, Bitmap* pThumb = NULL );
    static BOOL                 GetSdrObj( ULONG nThemeId, ULONG nSdrModelPos,
                                           FmFormModel* pModel = NULL, Bitmap* pThumb = NULL );

    static BOOL                 InsertSdrObj( const String& rThemeName, FmFormModel& rModel );
    static BOOL                 InsertSdrObj( ULONG nThemeId, FmFormModel& rModel );

    static BOOL                 BeginLocking( const String& rThemeName );
    static BOOL                 BeginLocking( ULONG nThemeId );

    static BOOL                 EndLocking( const String& rThemeName );
    static BOOL                 EndLocking( ULONG nThemeId );

    static BOOL                 DrawCentered( OutputDevice* pOut, const FmFormModel& rModel );
};

#endif // _GALLERY_HXX_
