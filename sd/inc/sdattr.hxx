/*************************************************************************
 *
 *  $RCSfile: sdattr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 12:23:02 $
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


