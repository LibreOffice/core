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

#include <TitleItemConverter.hxx>
#include "SchWhichPairs.hxx"
#include <ItemPropertyMap.hxx>
#include <GraphicPropertyItemConverter.hxx>
#include <CharacterPropertyItemConverter.hxx>
#include <MultipleItemConverter.hxx>
#include <svx/sdangitm.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <memory>

using namespace ::com::sun::star;

namespace chart::wrapper {

namespace {

ItemPropertyMapType & lcl_GetTitlePropertyMap()
{
    static ItemPropertyMapType aTitlePropertyMap{
        {SCHATTR_TEXT_STACKED, {"StackCharacters", 0}}};
    return aTitlePropertyMap;
};

class FormattedStringsConverter : public MultipleItemConverter
{
public:
    FormattedStringsConverter(
        const uno::Sequence< uno::Reference< chart2::XFormattedString > > & aStrings,
        SfxItemPool & rItemPool,
        const std::optional<awt::Size>& pRefSize,
        const uno::Reference< beans::XPropertySet > & xParentProp );

protected:
    virtual const WhichRangesContainer& GetWhichPairs() const override;
};

} // anonymous namespace

FormattedStringsConverter::FormattedStringsConverter(
    const uno::Sequence< uno::Reference< chart2::XFormattedString > > & aStrings,
    SfxItemPool & rItemPool,
    const std::optional<awt::Size>& pRefSize,
    const uno::Reference< beans::XPropertySet > & xParentProp ) :
        MultipleItemConverter( rItemPool )
{
    bool bHasRefSize = (pRefSize && xParentProp.is());
    for( uno::Reference< chart2::XFormattedString > const & formattedStr : aStrings )
    {
        uno::Reference< beans::XPropertySet > xProp( formattedStr, uno::UNO_QUERY );
        if( xProp.is())
        {
            if( bHasRefSize )
                m_aConverters.emplace_back(
                    new CharacterPropertyItemConverter(
                        xProp, rItemPool, pRefSize, u"ReferencePageSize"_ustr, xParentProp));
            else
                m_aConverters.emplace_back( new CharacterPropertyItemConverter( xProp, rItemPool ));
        }
    }
}

const WhichRangesContainer& FormattedStringsConverter::GetWhichPairs() const
{
    return nCharacterPropertyWhichPairs;
}

TitleItemConverter::TitleItemConverter(
    const uno::Reference<beans::XPropertySet> & rPropertySet,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
    const std::optional<awt::Size>& pRefSize ) :
        ItemConverter( rPropertySet, rItemPool )
{
    m_aConverters.emplace_back( new GraphicPropertyItemConverter(
                                 rPropertySet, rItemPool, rDrawModel,
                                 xNamedPropertyContainerFactory,
                                 GraphicObjectType::LineAndFillProperties ));

    // CharacterProperties are not at the title but at its contained XFormattedString objects
    // take the first formatted string in the sequence
    uno::Reference< chart2::XTitle > xTitle( rPropertySet, uno::UNO_QUERY );
    if( xTitle.is())
    {
        uno::Sequence< uno::Reference< chart2::XFormattedString > > aStringSeq( xTitle->getText());
        if( aStringSeq.hasElements() )
        {
            m_aConverters.emplace_back(
                new FormattedStringsConverter( aStringSeq, rItemPool, pRefSize, rPropertySet ));
        }
    }
}

TitleItemConverter::~TitleItemConverter()
{
}

void TitleItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    for( const auto& pConv : m_aConverters )
        pConv->FillItemSet( rOutItemSet );

    // own items
    ItemConverter::FillItemSet( rOutItemSet );
}

bool TitleItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = false;

    for( const auto& pConv : m_aConverters )
        bResult = pConv->ApplyItemSet( rItemSet ) || bResult;

    // own items
    return ItemConverter::ApplyItemSet( rItemSet ) || bResult;
}

const WhichRangesContainer& TitleItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nTitleWhichPairs;
}

bool TitleItemConverter::GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const
{
    ItemPropertyMapType & rMap( lcl_GetTitlePropertyMap());
    ItemPropertyMapType::const_iterator aIt( rMap.find( nWhichId ));

    if( aIt == rMap.end())
        return false;

    rOutProperty =(*aIt).second;
    return true;
}

bool TitleItemConverter::ApplySpecialItem(
    sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
{
    bool bChanged = false;

    switch( nWhichId )
    {
        case SCHATTR_TEXT_DEGREES:
        {
            // convert int to double (divided by 100)
            double fVal = static_cast< double >(
                static_cast< const SdrAngleItem & >(
                    rItemSet.Get( nWhichId )).GetValue().get()) / 100.0;
            double fOldVal = 0.0;
            bool bPropExisted =
                ( GetPropertySet()->getPropertyValue( u"TextRotation"_ustr ) >>= fOldVal );

            if( ! bPropExisted || fOldVal != fVal )
            {
                GetPropertySet()->setPropertyValue( u"TextRotation"_ustr , uno::Any( fVal ));
                bChanged = true;
            }
        }
        break;
    }

    return bChanged;
}

void TitleItemConverter::FillSpecialItem(
    sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
{
    switch( nWhichId )
    {
        case SCHATTR_TEXT_DEGREES:
        {
            // convert double to int (times 100)
            double fVal = 0;

            if( GetPropertySet()->getPropertyValue( u"TextRotation"_ustr ) >>= fVal )
            {
                rOutItemSet.Put( SdrAngleItem( SCHATTR_TEXT_DEGREES, Degree100(static_cast< sal_Int32 >(
                                                   ::rtl::math::round( fVal * 100.0 ) ) )));
            }
        }
        break;
   }
}

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
