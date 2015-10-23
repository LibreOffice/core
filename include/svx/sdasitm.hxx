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
#include <svx/svxdllapi.h>
#include <map>
#include <unordered_map>

class SVX_DLLPUBLIC SdrCustomShapeGeometryItem : public SfxPoolItem
{
public:
    typedef std::pair < const OUString, const OUString > PropertyPair;

private:
    struct SVX_DLLPUBLIC PropertyPairHash
    {
        size_t operator()( const SdrCustomShapeGeometryItem::PropertyPair& ) const;
    };
    typedef std::unordered_map <PropertyPair, sal_Int32, PropertyPairHash> PropertyPairHashMap;
    typedef std::unordered_map<OUString, sal_Int32, OUStringHash> PropertyHashMap;

    PropertyHashMap     aPropHashMap;
    PropertyPairHashMap aPropPairHashMap;

    css::uno::Sequence< css::beans::PropertyValue > aPropSeq;

    public:

            TYPEINFO_OVERRIDE();

            SdrCustomShapeGeometryItem();
            SdrCustomShapeGeometryItem( const css::uno::Sequence< css::beans::PropertyValue >& );
            SdrCustomShapeGeometryItem( SvStream& rIn, sal_uInt16 nVersion );
            virtual ~SdrCustomShapeGeometryItem();

            virtual bool                operator==( const SfxPoolItem& ) const override;
            virtual bool GetPresentation(SfxItemPresentation ePresentation,
                                            SfxMapUnit eCoreMetric, SfxMapUnit ePresentationMetric,
                                            OUString &rText, const IntlWrapper * = 0) const override;

            virtual SfxPoolItem*        Create( SvStream&, sal_uInt16 nItem ) const override;
            virtual SvStream&           Store( SvStream&, sal_uInt16 nVersion ) const override;

            virtual SfxPoolItem*        Clone( SfxItemPool* pPool = NULL ) const override;
            virtual sal_uInt16          GetVersion( sal_uInt16 nFileFormatVersion ) const override;

            virtual bool                QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
            virtual bool                PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

            const css::uno::Sequence< css::beans::PropertyValue >& GetGeometry() const { return aPropSeq;}

            css::uno::Any* GetPropertyValueByName( const OUString& rPropName );
            const css::uno::Any* GetPropertyValueByName( const OUString& rPropName ) const;
            css::uno::Any* GetPropertyValueByName( const OUString& rPropName, const OUString& rPropName2 );
            const css::uno::Any* GetPropertyValueByName( const OUString& rPropName, const OUString& rPropName2 ) const;

            void SetPropertyValue( const css::beans::PropertyValue& rPropVal );
            void SetPropertyValue( const OUString& rSequenceName, const css::beans::PropertyValue& rPropVal );

            void ClearPropertyValue( const OUString& rPropertyName );
};

class SdrCustomShapeReplacementURLItem : public SfxStringItem
{
    public:
            SdrCustomShapeReplacementURLItem();
};

inline SdrOnOffItem makeSdrTextWordWrapItem( bool bAuto ) {
    return SdrOnOffItem( SDRATTR_TEXT_WORDWRAP, bAuto );
}

inline SdrOnOffItem makeSdrTextAutoGrowSizeItem( bool bAuto ) {
    return SdrOnOffItem( SDRATTR_TEXT_AUTOGROWSIZE, bAuto ) ;
}

// some useful inline methods

inline size_t SdrCustomShapeGeometryItem::PropertyPairHash::operator()( const SdrCustomShapeGeometryItem::PropertyPair &r1 ) const
{
    return (size_t)r1.first.hashCode() + r1.second.hashCode();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
