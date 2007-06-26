/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdattr.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-26 13:38:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _SDATTR_HRC
#include "sdattr.hrc"
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

class SdAttrLayerTitle : public SfxStringItem
{
public:
    SdAttrLayerTitle() : SfxStringItem( ATTR_LAYER_TITLE, XubString()) {}
    SdAttrLayerTitle( const String& aStr ) : SfxStringItem( ATTR_LAYER_TITLE, aStr ) {}
};

//------------------------------------------------------------------

class SdAttrLayerDesc : public SfxStringItem
{
public:
    SdAttrLayerDesc() : SfxStringItem( ATTR_LAYER_DESC, XubString()) {}
    SdAttrLayerDesc( const String& aStr ) : SfxStringItem( ATTR_LAYER_DESC, aStr ) {}
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


