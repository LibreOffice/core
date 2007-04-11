/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdasitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:08:11 $
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

#ifndef _SDASITM_HXX
#define _SDASITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif
#ifndef _SDOOITM_HXX
#include <svx/sdooitm.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#include <hash_map>
#include <map>

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SdrCustomShapeEngineItem : public SfxStringItem
{
    public :
            SdrCustomShapeEngineItem();
            SdrCustomShapeEngineItem( const String& rCustomShapeEngine );
};

class SdrCustomShapeDataItem : public SfxStringItem
{
    public :
            SdrCustomShapeDataItem();
            SdrCustomShapeDataItem( const String& rCustomShapeData );
};

class SVX_DLLPUBLIC SdrCustomShapeGeometryItem : public SfxPoolItem
{
public:
    typedef std::pair < const ::rtl::OUString, const ::rtl::OUString > PropertyPair;

private:
    struct SVX_DLLPRIVATE PropertyEq
    {
        bool operator()( const rtl::OUString&, const rtl::OUString& ) const;
    };
    struct SVX_DLLPRIVATE PropertyPairEq
    {
        bool operator()( const SdrCustomShapeGeometryItem::PropertyPair&, const SdrCustomShapeGeometryItem::PropertyPair& ) const;
    };
    struct SVX_DLLPRIVATE PropertyPairHash
    {
        size_t operator()( const SdrCustomShapeGeometryItem::PropertyPair &r1 ) const;
    };
    typedef std::hash_map < PropertyPair, sal_Int32, PropertyPairHash, PropertyPairEq > PropertyPairHashMap;
    typedef std::hash_map< rtl::OUString, sal_Int32, rtl::OUStringHash, PropertyEq > PropertyHashMap;

    PropertyHashMap     aPropHashMap;
    PropertyPairHashMap aPropPairHashMap;

    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aPropSeq;

    public :

            TYPEINFO();

            SdrCustomShapeGeometryItem();
            SdrCustomShapeGeometryItem( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& );
            SdrCustomShapeGeometryItem( SvStream& rIn, sal_uInt16 nVersion );
            ~SdrCustomShapeGeometryItem();

            virtual int                 operator==( const SfxPoolItem& ) const;
            virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePresentation,
                                            SfxMapUnit eCoreMetric, SfxMapUnit ePresentationMetric,
                                                String &rText, const IntlWrapper * = 0) const;

            virtual SfxPoolItem*        Create( SvStream&, sal_uInt16 nItem ) const;
            virtual SvStream&           Store( SvStream&, sal_uInt16 nVersion ) const;

            virtual SfxPoolItem*        Clone( SfxItemPool* pPool = NULL ) const;
            virtual sal_uInt16          GetVersion( sal_uInt16 nFileFormatVersion ) const;

            virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
            virtual sal_Bool            PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

            const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& GetGeometry() const;

#ifdef SDR_ISPOOLABLE
            virtual int IsPoolable() const;
#endif
            com::sun::star::uno::Any* GetPropertyValueByName( const rtl::OUString& rPropName );
            com::sun::star::uno::Any* GetPropertyValueByName( const rtl::OUString& rPropName, const rtl::OUString& rPropName2 );

            void SetPropertyValue( const com::sun::star::beans::PropertyValue& rPropVal );
            void SetPropertyValue( const rtl::OUString& rSequenceName, const com::sun::star::beans::PropertyValue& rPropVal );

            void ClearPropertyValue( const rtl::OUString& rPropertyName );
            void ClearPropertyValue( const rtl::OUString& rSequenceName, const rtl::OUString& rPropertyName );
};

class SdrCustomShapeReplacementURLItem : public SfxStringItem
{
    public :
            SdrCustomShapeReplacementURLItem();
            SdrCustomShapeReplacementURLItem( const String& rCustomShapeReplacementURL );
};

//---------------------------
// class SdrTextWordWrapItem
//---------------------------
class SdrTextWordWrapItem : public SdrOnOffItem {
public:
    SdrTextWordWrapItem( BOOL bAuto = FALSE ):  SdrOnOffItem( SDRATTR_TEXT_WORDWRAP, bAuto ) {}
    SdrTextWordWrapItem( SvStream& rIn )  :     SdrOnOffItem( SDRATTR_TEXT_WORDWRAP, rIn )   {}
};

//-------------------------------
// class SdrTextAutoGrowSizeItem
//-------------------------------
class SdrTextAutoGrowSizeItem : public SdrOnOffItem {
public:
    SdrTextAutoGrowSizeItem( BOOL bAuto = FALSE ):      SdrOnOffItem( SDRATTR_TEXT_AUTOGROWSIZE, bAuto ) {}
    SdrTextAutoGrowSizeItem( SvStream& rIn )   :        SdrOnOffItem( SDRATTR_TEXT_AUTOGROWSIZE, rIn )   {}
};

#endif

