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

#ifndef _SDASITM_HXX
#define _SDASITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdooitm.hxx>
#include <svl/stritem.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>
#include <hash_map>
#include <map>
#include "svx/svxdllapi.h"

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

            virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
            virtual sal_Bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

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
    SdrTextWordWrapItem( sal_Bool bAuto = sal_False ):  SdrOnOffItem( SDRATTR_TEXT_WORDWRAP, bAuto ) {}
    SdrTextWordWrapItem( SvStream& rIn )  :     SdrOnOffItem( SDRATTR_TEXT_WORDWRAP, rIn )   {}
};

//-------------------------------
// class SdrTextAutoGrowSizeItem
//-------------------------------
class SdrTextAutoGrowSizeItem : public SdrOnOffItem {
public:
    SdrTextAutoGrowSizeItem( sal_Bool bAuto = sal_False ):      SdrOnOffItem( SDRATTR_TEXT_AUTOGROWSIZE, bAuto ) {}
    SdrTextAutoGrowSizeItem( SvStream& rIn )   :        SdrOnOffItem( SDRATTR_TEXT_AUTOGROWSIZE, rIn )   {}
};

#endif

