/*************************************************************************
 *
 *  $RCSfile: sdattr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:28 $
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

#ifndef _SDATTR_HXX
#define _SDATTR_HXX

#ifndef _COM_SUN_STAR_PRESENTATION_FADEEFFECT_HPP_
#include <com/sun/star/presentation/FadeEffect.hpp>
#endif

#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFX_HRC
#include <sfx2/sfx.hrc>
#endif

#ifndef _SD_GLOB_HXX
#include "glob.hxx"
#endif
#ifndef _SD_FADEDEF_H
#include "fadedef.h"
#endif
#ifndef _SD_DIADEF_H
#include "diadef.h"
#endif

//------------------------------------------------------------------

// Anfang Page-Attribute
#define ATTR_PAGE_START         SID_SD_START + 1234
#define ATTR_PAGE_NAME          ATTR_PAGE_START
#define ATTR_PAGE_BACKGROUND    ATTR_PAGE_START + 1
#define ATTR_PAGE_OBJECTS       ATTR_PAGE_START + 2
#define ATTR_PAGE_LAYOUT        ATTR_PAGE_START + 3
#define ATTR_PAGE_END           ATTR_PAGE_LAYOUT

// Layer-Attribute
#define ATTR_LAYER_START        ATTR_PAGE_END + 1
#define ATTR_LAYER_NAME         ATTR_LAYER_START
#define ATTR_LAYER_VISIBLE      ATTR_LAYER_START + 1
#define ATTR_LAYER_PRINTABLE    ATTR_LAYER_START + 2
#define ATTR_LAYER_LOCKED       ATTR_LAYER_START + 3
#define ATTR_LAYER_THISPAGE     ATTR_LAYER_START + 4
#define ATTR_LAYER_END          ATTR_LAYER_THISPAGE

// Anfang Dia-Attribute
#define ATTR_DIA_START          ATTR_LAYER_END + 1
#define ATTR_DIA_EFFECT         ATTR_DIA_START
#define ATTR_DIA_SPEED          ATTR_DIA_START + 1
#define ATTR_DIA_AUTO           ATTR_DIA_START + 2
#define ATTR_DIA_TIME           ATTR_DIA_START + 3
#define ATTR_DIA_SOUND          ATTR_DIA_START + 4
#define ATTR_DIA_SOUNDFILE      ATTR_DIA_START + 5
#define ATTR_DIA_END            ATTR_DIA_SOUNDFILE

// Praesentations-Attribute
#define ATTR_PRESENT_START              ATTR_DIA_END + 1
#define ATTR_PRESENT_ALL                ATTR_PRESENT_START
#define ATTR_PRESENT_CUSTOMSHOW         ATTR_PRESENT_START + 1
#define ATTR_PRESENT_DIANAME            ATTR_PRESENT_START + 2
#define ATTR_PRESENT_ENDLESS            ATTR_PRESENT_START + 3
#define ATTR_PRESENT_MANUEL             ATTR_PRESENT_START + 4
#define ATTR_PRESENT_MOUSE              ATTR_PRESENT_START + 5
#define ATTR_PRESENT_PEN                ATTR_PRESENT_START + 6
#define ATTR_PRESENT_NAVIGATOR          ATTR_PRESENT_START + 7
#define ATTR_PRESENT_CHANGE_PAGE        ATTR_PRESENT_START + 8
#define ATTR_PRESENT_ALWAYS_ON_TOP      ATTR_PRESENT_START + 9
#define ATTR_PRESENT_FULLSCREEN         ATTR_PRESENT_START + 10
#define ATTR_PRESENT_START_ACTUAL_PAGE  ATTR_PRESENT_START + 11
#define ATTR_PRESENT_ANIMATION_ALLOWED  ATTR_PRESENT_START + 12
#define ATTR_PRESENT_PAUSE_TIMEOUT      ATTR_PRESENT_START + 13
#define ATTR_PRESENT_SHOW_PAUSELOGO     ATTR_PRESENT_START + 14
#define ATTR_PRESENT_END                ATTR_PRESENT_SHOW_PAUSELOGO

// Transformations-Attribute
#define ATTR_TRANSF_START       ATTR_PRESENT_END + 1
#define ATTR_TRANSF_POS_X       ATTR_TRANSF_START
#define ATTR_TRANSF_POS_Y       ATTR_TRANSF_START + 1
#define ATTR_TRANSF_WIDTH       ATTR_TRANSF_START + 2
#define ATTR_TRANSF_HEIGHT      ATTR_TRANSF_START + 3
#define ATTR_TRANSF_SIZE_POINT  ATTR_TRANSF_START + 4
#define ATTR_TRANSF_ROT_X       ATTR_TRANSF_START + 5
#define ATTR_TRANSF_ROT_Y       ATTR_TRANSF_START + 6
#define ATTR_TRANSF_ANGLE       ATTR_TRANSF_START + 7
#define ATTR_TRANSF_END         ATTR_TRANSF_ANGLE

// Raster-Attribute
#define ATTR_GRID_START         ATTR_TRANSF_END + 1
#define ATTR_GRID_USE           ATTR_GRID_START
#define ATTR_GRID_DRAW_X        ATTR_GRID_START + 1
#define ATTR_GRID_DRAW_Y        ATTR_GRID_START + 2
#define ATTR_GRID_DIVISION_X    ATTR_GRID_START + 3
#define ATTR_GRID_DIVISION_Y    ATTR_GRID_START + 4
#define ATTR_GRID_SNAP_X        ATTR_GRID_START + 5
#define ATTR_GRID_SNAP_Y        ATTR_GRID_START + 6
#define ATTR_GRID_ORIGIN_X      ATTR_GRID_START + 7
#define ATTR_GRID_ORIGIN_Y      ATTR_GRID_START + 8
#define ATTR_GRID_END           ATTR_GRID_ORIGIN_Y

// Bildschirm-Attribute
#define ATTR_DISPLAY_START              ATTR_GRID_END + 1
#define ATTR_DISPLAY_FUNCTIONBAR        ATTR_DISPLAY_START
#define ATTR_DISPLAY_OBJECTBAR          ATTR_DISPLAY_START + 1
#define ATTR_DISPLAY_STATUSBAR          ATTR_DISPLAY_START + 2
#define ATTR_DISPLAY_TOOLBAR            ATTR_DISPLAY_START + 3
#define ATTR_DISPLAY_GRID               ATTR_DISPLAY_START + 4
#define ATTR_DISPLAY_RULER              ATTR_DISPLAY_START + 5
#define ATTR_DISPLAY_HELPLINES          ATTR_DISPLAY_START + 6
#define ATTR_DISPLAY_STRANGE_GRAPHIC    ATTR_DISPLAY_START + 7
#define ATTR_DISPLAY_ADJUSTLINES        ATTR_DISPLAY_START + 8
#define ATTR_DISPLAY_END                ATTR_DISPLAY_ADJUSTLINES

#define ATTR_TBX_DUMMY                  ATTR_DISPLAY_END + 1

// Animations-Attribute
#define ATTR_ANIMATION_START            ATTR_TBX_DUMMY + 1
#define ATTR_ANIMATION_ACTIVE           ATTR_ANIMATION_START
#define ATTR_ANIMATION_EFFECT           ATTR_ANIMATION_START + 1
#define ATTR_ANIMATION_SPEED            ATTR_ANIMATION_START + 2
#define ATTR_ANIMATION_INVISIBLE        ATTR_ANIMATION_START + 3
#define ATTR_ANIMATION_FADEOUT          ATTR_ANIMATION_START + 4
#define ATTR_ANIMATION_COLOR            ATTR_ANIMATION_START + 5
#define ATTR_ANIMATION_SOUNDON          ATTR_ANIMATION_START + 6
#define ATTR_ANIMATION_SOUNDFILE        ATTR_ANIMATION_START + 7
#define ATTR_ANIMATION_PLAYFULL         ATTR_ANIMATION_START + 8
#define ATTR_ANIMATION_TRANSPCOLOR      ATTR_ANIMATION_START + 9
#define ATTR_ANIMATION_TEXTEFFECT       ATTR_ANIMATION_START + 10
#define ATTR_ANIMATION_END              ATTR_ANIMATION_TEXTEFFECT

#define ATTR_ACTION_START               ATTR_ANIMATION_END + 1
#define ATTR_ACTION                     ATTR_ACTION_START
#define ATTR_ACTION_EFFECT              ATTR_ACTION_START + 1
#define ATTR_ACTION_EFFECTSPEED         ATTR_ACTION_START + 2
#define ATTR_ACTION_FILENAME            ATTR_ACTION_START + 3
#define ATTR_ACTION_SOUNDON             ATTR_ACTION_START + 4
#define ATTR_ACTION_PLAYFULL            ATTR_ACTION_START + 5
#define ATTR_ACTION_END                 ATTR_ACTION_PLAYFULL

#define ATTR_PRINTOPTIONS_START         ATTR_ACTION_END + 1
#define ATTR_PRINTOPTIONS_PRINT         ATTR_PRINTOPTIONS_START
#define ATTR_PRINTOPTIONS_PAGESIZE      ATTR_PRINTOPTIONS_START + 1
#define ATTR_PRINTOPTIONS_FADEOUT       ATTR_PRINTOPTIONS_START + 2
#define ATTR_PRINTOPTIONS_DATE          ATTR_PRINTOPTIONS_START + 3
#define ATTR_PRINTOPTIONS_TIME          ATTR_PRINTOPTIONS_START + 4
#define ATTR_PRINTOPTIONS_PAGENAME      ATTR_PRINTOPTIONS_START + 5
#define ATTR_PRINTOPTIONS_END           ATTR_PRINTOPTIONS_PAGENAME

#define ATTR_COPY_START                 ATTR_PRINTOPTIONS_END + 1
#define ATTR_COPY_NUMBER                ATTR_COPY_START
#define ATTR_COPY_MOVE_X                ATTR_COPY_START + 1
#define ATTR_COPY_MOVE_Y                ATTR_COPY_START + 2
#define ATTR_COPY_ANGLE                 ATTR_COPY_START + 3
#define ATTR_COPY_WIDTH                 ATTR_COPY_START + 4
#define ATTR_COPY_HEIGHT                ATTR_COPY_START + 5
#define ATTR_COPY_START_COLOR           ATTR_COPY_START + 6
#define ATTR_COPY_END_COLOR             ATTR_COPY_START + 7
#define ATTR_COPY_END                   ATTR_COPY_END_COLOR

#define ATTR_SNAPLINE_START             ATTR_COPY_END + 1
#define ATTR_SNAPLINE_KIND              ATTR_SNAPLINE_START
#define ATTR_SNAPLINE_X                 ATTR_SNAPLINE_START + 1
#define ATTR_SNAPLINE_Y                 ATTR_SNAPLINE_START + 2
#define ATTR_SNAPLINE_END               ATTR_SNAPLINE_Y

#define ATTR_OPTIONS_START              ATTR_SNAPLINE_END + 1
#define ATTR_OPTIONS                    ATTR_OPTIONS_START
#define ATTR_OPTIONS_LAYOUT             ATTR_OPTIONS_START + 1
#define ATTR_OPTIONS_CONTENTS           ATTR_OPTIONS_START + 2
#define ATTR_OPTIONS_MISC               ATTR_OPTIONS_START + 3
#define ATTR_OPTIONS_SNAP               ATTR_OPTIONS_START + 4
#define ATTR_OPTIONS_PRINT              ATTR_OPTIONS_START + 5
#define ATTR_OPTIONS_END                ATTR_OPTIONS_PRINT

#define ATTR_OPTIONS_SCALE_START        ATTR_OPTIONS_END + 1
#define ATTR_OPTIONS_SCALE_X            ATTR_OPTIONS_SCALE_START
#define ATTR_OPTIONS_SCALE_Y            ATTR_OPTIONS_SCALE_START + 1
#define ATTR_OPTIONS_SCALE_WIDTH        ATTR_OPTIONS_SCALE_START + 2
#define ATTR_OPTIONS_SCALE_HEIGHT       ATTR_OPTIONS_SCALE_START + 3
#define ATTR_OPTIONS_SCALE_END          ATTR_OPTIONS_SCALE_HEIGHT

#define ATTR_PUBLISH_START              ATTR_OPTIONS_SCALE_END + 1
#define ATTR_PUBLISH_LOCATION           ATTR_PUBLISH_START
#define ATTR_PUBLISH_WITHCONTENTSPAGE   ATTR_PUBLISH_START + 1
#define ATTR_PUBLISH_RESOLUTION         ATTR_PUBLISH_START + 2
#define ATTR_PUBLISH_MODE               ATTR_PUBLISH_START + 3
#define ATTR_PUBLISH_FORMAT             ATTR_PUBLISH_START + 4
#define ATTR_PUBLISH_COMPRESSION        ATTR_PUBLISH_START + 5
#define ATTR_PUBLISH_WITHNOTES          ATTR_PUBLISH_START + 6
#define ATTR_PUBLISH_AUTHOR             ATTR_PUBLISH_START + 9
#define ATTR_PUBLISH_EMAIL              ATTR_PUBLISH_START + 10
#define ATTR_PUBLISH_WWW                ATTR_PUBLISH_START + 11
#define ATTR_PUBLISH_INFO               ATTR_PUBLISH_START + 12
#define ATTR_PUBLISH_WITHDOWNLOAD       ATTR_PUBLISH_START + 13
#define ATTR_PUBLISH_WITHSTAROFFICE     ATTR_PUBLISH_START + 14
#define ATTR_PUBLISH_BUTTONS            ATTR_PUBLISH_START + 15
#define ATTR_PUBLISH_USERATTR           ATTR_PUBLISH_START + 16
#define ATTR_PUBLISH_BACKCOLOR          ATTR_PUBLISH_START + 17
#define ATTR_PUBLISH_TEXTCOLOR          ATTR_PUBLISH_START + 18
#define ATTR_PUBLISH_LINKCOLOR          ATTR_PUBLISH_START + 19
#define ATTR_PUBLISH_VLINKCOLOR         ATTR_PUBLISH_START + 20
#define ATTR_PUBLISH_ALINKCOLOR         ATTR_PUBLISH_START + 21
#define ATTR_PUBLISH_USEATTRIBS         ATTR_PUBLISH_START + 22
#define ATTR_PUBLISH_USECOLOR           ATTR_PUBLISH_START + 23
#define ATTR_PUBLISH_WEBSHOW            ATTR_PUBLISH_START + 24
#define ATTR_PUBLISH_SCRIPT             ATTR_PUBLISH_START + 25
#define ATTR_PUBLISH_URLPATH            ATTR_PUBLISH_START + 26
#define ATTR_PUBLISH_CGIPATH            ATTR_PUBLISH_START + 27
#define ATTR_PUBLISH_INDEX              ATTR_PUBLISH_START + 28
#define ATTR_PUBLISH_SLIDECHG           ATTR_PUBLISH_START + 29
#define ATTR_PUBLISH_SLIDEDURATION      ATTR_PUBLISH_START + 30
#define ATTR_PUBLISH_ENDLESS            ATTR_PUBLISH_START + 31
#define ATTR_PUBLISH_SLIDESOUND         ATTR_PUBLISH_START + 32
#define ATTR_PUBLISH_END                ATTR_PUBLISH_SLIDESOUND

#define ATTR_PRESLAYOUT_START           ATTR_PUBLISH_END + 1
#define ATTR_PRESLAYOUT_NAME            ATTR_PRESLAYOUT_START
#define ATTR_PRESLAYOUT_LOAD            ATTR_PRESLAYOUT_START + 1
#define ATTR_PRESLAYOUT_MASTER_PAGE     ATTR_PRESLAYOUT_START + 2
#define ATTR_PRESLAYOUT_CHECK_MASTERS   ATTR_PRESLAYOUT_START + 3
#define ATTR_PRESLAYOUT_END             ATTR_PRESLAYOUT_CHECK_MASTERS

// Pack & Go Attribute
#define ATTR_PACKNGO_START              ATTR_PRESLAYOUT_END + 1
#define ATTR_PACKNGO_UNLINK             ATTR_PACKNGO_START
#define ATTR_PACKNGO_COMPRESS           ATTR_PACKNGO_START + 1
#define ATTR_PACKNGO_SPANNING           ATTR_PACKNGO_START + 2
#define ATTR_PACKNGO_MEDIUMSIZE         ATTR_PACKNGO_START + 3
#define ATTR_PACKNGO_END                ATTR_PACKNGO_MEDIUMSIZE

//==================================================================
//  Layer-Attribute
//==================================================================

class SdAttrLayerName : public SfxStringItem
{
public:
    SdAttrLayerName() :
        SfxStringItem( ATTR_LAYER_NAME, String( RTL_CONSTASCII_USTRINGPARAM( "neue Ebene" ))) {}
    SdAttrLayerName( const String& aStr ) :
        SfxStringItem( ATTR_LAYER_NAME, aStr ) {}
};


//------------------------------------------------------------------

class SdAttrLayerVisible : public SfxBoolItem
{
public:
    SdAttrLayerVisible( BOOL bValue = TRUE ) :
        SfxBoolItem( ATTR_LAYER_VISIBLE, bValue ) {}
};

//------------------------------------------------------------------

class SdAttrLayerPrintable : public SfxBoolItem
{
public:
    SdAttrLayerPrintable( BOOL bValue = TRUE ) :
        SfxBoolItem( ATTR_LAYER_PRINTABLE, bValue ) {}
};

//------------------------------------------------------------------

class SdAttrLayerLocked : public SfxBoolItem
{
public:
    SdAttrLayerLocked( BOOL bValue = FALSE ) :
        SfxBoolItem( ATTR_LAYER_LOCKED, bValue ) {}
};

//------------------------------------------------------------------

class SdAttrLayerThisPage : public SfxBoolItem
{
public:
    SdAttrLayerThisPage( BOOL bValue = FALSE ) :
        SfxBoolItem( ATTR_LAYER_THISPAGE, bValue ) {}
};

//------------------------------------------------------------------

class DiaEffectItem : public SfxEnumItem
{
public:
            TYPEINFO();
            DiaEffectItem( ::com::sun::star::presentation::FadeEffect eFade = com::sun::star::presentation::FadeEffect_NONE );
            DiaEffectItem( SvStream& rIn );

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, USHORT nVer ) const;
            USHORT          GetValueCount() const { return FADE_EFFECT_COUNT; }
            ::com::sun::star::presentation::FadeEffect      GetValue() const
                            { return (::com::sun::star::presentation::FadeEffect) SfxEnumItem::GetValue(); }
};

//------------------------------------------------------------------

class DiaSpeedItem : public SfxEnumItem
{
public:
            TYPEINFO();
            DiaSpeedItem( FadeSpeed = FADE_SPEED_MEDIUM );
            DiaSpeedItem( SvStream& rIn );

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, USHORT nVer ) const;
            USHORT          GetValueCount() const { return FADE_SPEED_COUNT; }
            FadeSpeed       GetValue() const
                            { return (FadeSpeed) SfxEnumItem::GetValue(); }
};

//------------------------------------------------------------------

class DiaAutoItem : public SfxEnumItem
{
public:
            TYPEINFO();
            DiaAutoItem( PresChange = PRESCHANGE_MANUAL );
            DiaAutoItem( SvStream& rIn );

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, USHORT nVer ) const;
            USHORT          GetValueCount() const { return PRESCHANGE_COUNT; }
            PresChange      GetValue() const { return (PresChange) SfxEnumItem::GetValue(); }
};

//------------------------------------------------------------------

class DiaTimeItem : public SfxUInt32Item
{
public:
            TYPEINFO();
            DiaTimeItem( UINT32 nValue = 0L );

    virtual SfxPoolItem* Clone( SfxItemPool* pPool = 0 ) const;
    virtual int          operator==( const SfxPoolItem& ) const;
};

#endif // _SDATTR_HXX


