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

#ifndef INCLUDED_SVX_SDASITM_HXX
#define INCLUDED_SVX_SDASITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdooitm.hxx>
#include <svl/stritem.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>
#include <boost/unordered_map.hpp>
#include <map>
#include <svx/svxdllapi.h>

class SdrCustomShapeEngineItem : public SfxStringItem
{
    public :
            SdrCustomShapeEngineItem();
};

class SdrCustomShapeDataItem : public SfxStringItem
{
    public :
            SdrCustomShapeDataItem();
};

class SVX_DLLPUBLIC SdrCustomShapeGeometryItem : public SfxPoolItem
{
public:
    typedef std::pair < const OUString, const OUString > PropertyPair;

private:
    struct SVX_DLLPUBLIC PropertyEq
    {
        bool operator()( const OUString&, const OUString& ) const;
    };
    struct SVX_DLLPUBLIC PropertyPairEq
    {
        bool operator()( const SdrCustomShapeGeometryItem::PropertyPair&, const SdrCustomShapeGeometryItem::PropertyPair& ) const;
    };
    struct SVX_DLLPUBLIC PropertyPairHash
    {
        size_t operator()( const SdrCustomShapeGeometryItem::PropertyPair& ) const;
    };
    typedef boost::unordered_map < PropertyPair, sal_Int32, PropertyPairHash, PropertyPairEq > PropertyPairHashMap;
    typedef boost::unordered_map< OUString, sal_Int32, OUStringHash, PropertyEq > PropertyHashMap;

    PropertyHashMap     aPropHashMap;
    PropertyPairHashMap aPropPairHashMap;

    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aPropSeq;

    public :

            TYPEINFO();

            SdrCustomShapeGeometryItem();
            SdrCustomShapeGeometryItem( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& );
            SdrCustomShapeGeometryItem( SvStream& rIn, sal_uInt16 nVersion );
            ~SdrCustomShapeGeometryItem();

            virtual bool                operator==( const SfxPoolItem& ) const;
            virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePresentation,
                                            SfxMapUnit eCoreMetric, SfxMapUnit ePresentationMetric,
                                            OUString &rText, const IntlWrapper * = 0) const;

            virtual SfxPoolItem*        Create( SvStream&, sal_uInt16 nItem ) const;
            virtual SvStream&           Store( SvStream&, sal_uInt16 nVersion ) const;

            virtual SfxPoolItem*        Clone( SfxItemPool* pPool = NULL ) const;
            virtual sal_uInt16          GetVersion( sal_uInt16 nFileFormatVersion ) const;

            virtual bool                QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
            virtual bool                PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

            const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& GetGeometry() const;

            com::sun::star::uno::Any* GetPropertyValueByName( const OUString& rPropName );
            com::sun::star::uno::Any* GetPropertyValueByName( const OUString& rPropName, const OUString& rPropName2 );

            void SetPropertyValue( const com::sun::star::beans::PropertyValue& rPropVal );
            void SetPropertyValue( const OUString& rSequenceName, const com::sun::star::beans::PropertyValue& rPropVal );

            void ClearPropertyValue( const OUString& rPropertyName );
};

class SdrCustomShapeReplacementURLItem : public SfxStringItem
{
    public :
            SdrCustomShapeReplacementURLItem();
};


// class SdrTextWordWrapItem

class SdrTextWordWrapItem : public SdrOnOffItem {
public:
    SdrTextWordWrapItem( bool bAuto = false ):  SdrOnOffItem( SDRATTR_TEXT_WORDWRAP, bAuto ) {}
    SdrTextWordWrapItem( SvStream& rIn )  :     SdrOnOffItem( SDRATTR_TEXT_WORDWRAP, rIn )   {}
};


// class SdrTextAutoGrowSizeItem

class SdrTextAutoGrowSizeItem : public SdrOnOffItem {
public:
    SdrTextAutoGrowSizeItem( bool bAuto = false ):      SdrOnOffItem( SDRATTR_TEXT_AUTOGROWSIZE, bAuto ) {}
    SdrTextAutoGrowSizeItem( SvStream& rIn )   :        SdrOnOffItem( SDRATTR_TEXT_AUTOGROWSIZE, rIn )   {}
};

// some useful inline methods

inline bool SdrCustomShapeGeometryItem::PropertyEq::operator()( const OUString& r1, const OUString& r2 ) const
{
    return r1.equals( r2 );
}

inline bool SdrCustomShapeGeometryItem::PropertyPairEq::operator()( const SdrCustomShapeGeometryItem::PropertyPair& r1, const SdrCustomShapeGeometryItem::PropertyPair& r2 ) const
{
    return ( r1.first.equals( r2.first ) ) && ( r1.second.equals( r2.second ) );
}

inline size_t SdrCustomShapeGeometryItem::PropertyPairHash::operator()( const SdrCustomShapeGeometryItem::PropertyPair &r1 ) const
{
    return (size_t)r1.first.hashCode() + r1.second.hashCode();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
