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

#define GALLERY_THEME_DUMMY5            36

#define GALLERY_THEME_FONTWORK          37
#define GALLERY_THEME_FONTWORK_VERTICAL 38

#define GALLERY_THEME_SHAPES_POLYGONS            39
#define GALLERY_THEME_SHAPES_1            40
#define GALLERY_THEME_SHAPES_2            41
#define GALLERY_THEME_ANIMALS            42
#define GALLERY_THEME_CARS            43
#define GALLERY_THEME_BUGS            44
#define GALLERY_THEME_CISCO_OTHER            45
#define GALLERY_THEME_CISCO_MEDIA            46
#define GALLERY_THEME_CISCO_PRODUCTS            47
#define GALLERY_THEME_CISCO_WAN_LAN            48
#define GALLERY_THEME_DOMINO_USUAL            49
#define GALLERY_THEME_DOMINO_NUMBERED            50
#define GALLERY_THEME_ELECTRONICS_PARTS_1            51
#define GALLERY_THEME_ELECTRONICS_PARTS_2            52
#define GALLERY_THEME_ELECTRONICS_PARTS_3            53
#define GALLERY_THEME_ELECTRONICS_PARTS_4            54
#define GALLERY_THEME_ELECTRONICS_CIRCUIT            55
#define GALLERY_THEME_ELECTRONICS_SIGNS            56
#define GALLERY_THEME_ELECTRONICS_GAUGES            57
#define GALLERY_THEME_PEOPLE_1            58
#define GALLERY_THEME_ARCHITECTURE_OVERLAY            59
#define GALLERY_THEME_ARCHITECTURE_FURNITURES            60
#define GALLERY_THEME_ARCHITECTURE_BUILDINGS            61
#define GALLERY_THEME_ARCHITECTURE_BATHROOM_KITCHEN            62
#define GALLERY_THEME_ARCHITECTURE_KITCHEN            63
#define GALLERY_THEME_ARCHITECTURE_WINDOWS_DOORS            64
#define GALLERY_THEME_FLOWCHARTS_1            65
#define GALLERY_THEME_FLOWCHARTS_2            66
#define GALLERY_THEME_FORALSTUDIOA            67
#define GALLERY_THEME_PHOTOS_FAUNA            68
#define GALLERY_THEME_PHOTOS_BUILDINGS            69
#define GALLERY_THEME_PHOTOS_PLANTS            70
#define GALLERY_THEME_PHOTOS_STATUES            71
#define GALLERY_THEME_PHOTOS_LANDSCAPES            72
#define GALLERY_THEME_PHOTOS_CITIES            73
#define GALLERY_THEME_PHOTOS_FLOWERS            74
#define GALLERY_THEME_THERAPEUTICS_GENERAL            75
#define GALLERY_THEME_WEATHER            76
#define GALLERY_THEME_VECHILES            77
#define GALLERY_THEME_SIGNS            78
#define GALLERY_THEME_BLUE_MAN            79
#define GALLERY_THEME_CHEMISTRY_AMINO_ACIDS            80
#define GALLERY_THEME_LOGICAL_SIGNS            81
#define GALLERY_THEME_LOGICAL_GATES            82
#define GALLERY_THEME_LOGOS            83
#define GALLERY_THEME_SMILES            84
#define GALLERY_THEME_ARROWS_1            85
#define GALLERY_THEME_CLOCK_01_CLOCK            86
#define GALLERY_THEME_CLOCK_02_CLOCK            87
#define GALLERY_THEME_CLOCK_03_CLOCK            88
#define GALLERY_THEME_CLOCK_04_CLOCK            89
#define GALLERY_THEME_CLOCK_05_CLOCK            90
#define GALLERY_THEME_CLOCK_06_CLOCK            91
#define GALLERY_THEME_CLOCK_07_CLOCK            92
#define GALLERY_THEME_CLOCK_08_CLOCK            93
#define GALLERY_THEME_CLOCK_09_CLOCK            94
#define GALLERY_THEME_CLOCK_10_CLOCK            95
#define GALLERY_THEME_CLOCK_11_CLOCK            96
#define GALLERY_THEME_CLOCK_12_CLOCK            97
#define GALLERY_THEME_PNEUMATIC_PARTS            98
#define GALLERY_THEME_COMPUTER_GENERAL            99
#define GALLERY_THEME_COMPUTER_NETWORK            100
#define GALLERY_THEME_COMPUTER_NETWORK_DEVICES            101
#define GALLERY_THEME_COMPUTER_WIFI            102
#define GALLERY_THEME_NUMBERS            103
#define GALLERY_THEME_SIGNS_DANGER            104
#define GALLERY_THEME_OBJECTS            105
#define GALLERY_THEME_MAPS_AFRICA            106
#define GALLERY_THEME_MAPS_UNITED_STATES_OF_AMERICA            107
#define GALLERY_THEME_MAPS_AUSTRALIA            108
#define GALLERY_THEME_MAPS_ASIA            109
#define GALLERY_THEME_MAPS_SOUTH_AMERICA            110
#define GALLERY_THEME_MAPS_EUROPE            111
#define GALLERY_THEME_MAPS_EUROPE_1            112
#define GALLERY_THEME_MAPS_FRANCE            113
#define GALLERY_THEME_MAPS_FRANCE_COUNTRIES            114
#define GALLERY_THEME_MAPS_SIGNS            115
#define GALLERY_THEME_MAPS_CANADA            116
#define GALLERY_THEME_MAPS_CONTINENTS            117
#define GALLERY_THEME_MAPS_MIDDLE_EAST            118
#define GALLERY_THEME_MAPS_MIDDLE_AMERICA            119
#define GALLERY_THEME_MAPS_MIDDLE_AGES            120
#define GALLERY_THEME_MAPS_MEXICO            121
#define GALLERY_THEME_MAPS_ANCIENT_TIMES            122
#define GALLERY_THEME_MAPS_SYMBOLS            123
#define GALLERY_THEME_MAPS_HISTORY_1900            124
#define GALLERY_THEME_MAPS_WORLD            125
#define GALLERY_THEME_CROPS            126
#define GALLERY_THEME_FRACTIONS            127
#define GALLERY_THEME_FLAGS_1            128
#define GALLERY_THEME_MUSIC_INSTRUMENTS            129
#define GALLERY_THEME_MUSIC_SHEET_MUSIC            130
#define GALLERY_THEME_SPECIAL_PICTOGRAMM            131
#define GALLERY_THEME_PHOTOS_CELEBRATION            132
#define GALLERY_THEME_PHOTOS_FOODSANDDRINKS            133
#define GALLERY_THEME_PHOTOS_HUMANS            134
#define GALLERY_THEME_PHOTOS_OBJECTS            135
#define GALLERY_THEME_PHOTOS_SPACE            136
#define GALLERY_THEME_PHOTOS_TRAVEL            137
#define GALLERY_THEME_OPENOFFICEORG_LOGOS            138
#define GALLERY_THEME_RELIGION            139
#define GALLERY_THEME_BUILDINGS            140
#define GALLERY_THEME_HOMEPAGE2            141
#define GALLERY_THEME_ELEMENTSBULLETS2            142

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

    static bool                 FillThemeList( std::vector<String>& rThemeList );

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
                                           SdrModel* pModel = NULL, Bitmap* pThumb = NULL );
    static BOOL                 GetSdrObj( ULONG nThemeId, ULONG nSdrModelPos,
                                           SdrModel* pModel = NULL, Bitmap* pThumb = NULL );

    static BOOL                 InsertSdrObj( const String& rThemeName, FmFormModel& rModel );
    static BOOL                 InsertSdrObj( ULONG nThemeId, FmFormModel& rModel );

    static BOOL                 BeginLocking( const String& rThemeName );
    static BOOL                 BeginLocking( ULONG nThemeId );

    static BOOL                 EndLocking( const String& rThemeName );
    static BOOL                 EndLocking( ULONG nThemeId );

    static BOOL                 DrawCentered( OutputDevice* pOut, const FmFormModel& rModel );
};

#endif // _GALLERY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
