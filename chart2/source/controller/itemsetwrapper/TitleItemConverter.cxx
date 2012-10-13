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

#include "TitleItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "CharacterPropertyItemConverter.hxx"
#include "MultipleItemConverter.hxx"
#include <svl/intitem.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/chart2/XTitled.hpp>

#include <functional>
#include <algorithm>

using namespace ::com::sun::star;

namespace
{
::comphelper::ItemPropertyMapType & lcl_GetTitlePropertyMap()
{
    static ::comphelper::ItemPropertyMapType aTitlePropertyMap(
        ::comphelper::MakeItemPropertyMap
        IPM_MAP_ENTRY( SCHATTR_TEXT_STACKED, "StackCharacters", 0 )
        );

    return aTitlePropertyMap;
};
} // anonymous namespace

namespace chart
{
namespace wrapper
{

// ========================================

class FormattedStringsConverter : public ::comphelper::MultipleItemConverter
{
public:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    FormattedStringsConverter(
        const uno::Sequence< uno::Reference< chart2::XFormattedString > > & aStrings,
        SfxItemPool & rItemPool,
        ::std::auto_ptr< awt::Size > pRefSize,
        const uno::Reference< beans::XPropertySet > & xParentProp );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    virtual ~FormattedStringsConverter();

protected:
    virtual const sal_uInt16 * GetWhichPairs() const;
};

// ----------------------------------------

SAL_WNODEPRECATED_DECLARATIONS_PUSH
FormattedStringsConverter::FormattedStringsConverter(
    const uno::Sequence< uno::Reference< chart2::XFormattedString > > & aStrings,
    SfxItemPool & rItemPool,
    ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize,
    const uno::Reference< beans::XPropertySet > & xParentProp ) :
        MultipleItemConverter( rItemPool )
{
    bool bHasRefSize = (pRefSize.get() && xParentProp.is());
    for( sal_Int32 i = 0; i < aStrings.getLength(); ++i )
    {
        uno::Reference< beans::XPropertySet > xProp( aStrings[ i ], uno::UNO_QUERY );
        if( xProp.is())
        {
            if( bHasRefSize )
                m_aConverters.push_back( new CharacterPropertyItemConverter(
                                             xProp, rItemPool,
                                             ::std::auto_ptr< awt::Size >( new awt::Size( *pRefSize )),
                                             "ReferencePageSize" ,
                                             xParentProp ));
            else
                m_aConverters.push_back( new CharacterPropertyItemConverter( xProp, rItemPool ));
        }
    }
}
SAL_WNODEPRECATED_DECLARATIONS_POP

FormattedStringsConverter::~FormattedStringsConverter()
{
}

const sal_uInt16 * FormattedStringsConverter::GetWhichPairs() const
{
    return nCharacterPropertyWhichPairs;
}

// ========================================

SAL_WNODEPRECATED_DECLARATIONS_PUSH
TitleItemConverter::TitleItemConverter(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
    ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize ) :
        ItemConverter( rPropertySet, rItemPool )
{
    m_aConverters.push_back( new GraphicPropertyItemConverter(
                                 rPropertySet, rItemPool, rDrawModel,
                                 xNamedPropertyContainerFactory,
                                 GraphicPropertyItemConverter::LINE_AND_FILL_PROPERTIES ));

    // CharacterProperties are not at the title but at its contained XFormattedString objects
    // take the first formatted string in the sequence
    uno::Reference< chart2::XTitle > xTitle( rPropertySet, uno::UNO_QUERY );
    if( xTitle.is())
    {
        uno::Sequence< uno::Reference< chart2::XFormattedString > > aStringSeq( xTitle->getText());
        if( aStringSeq.getLength() > 0 )
        {
            m_aConverters.push_back(
                new FormattedStringsConverter( aStringSeq, rItemPool, pRefSize, rPropertySet ));
        }
    }
}
SAL_WNODEPRECATED_DECLARATIONS_POP

TitleItemConverter::~TitleItemConverter()
{
    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::DeleteItemConverterPtr() );
}

void TitleItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::FillItemSetFunc( rOutItemSet ));

    // own items
    ItemConverter::FillItemSet( rOutItemSet );
}

bool TitleItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = false;

    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ::comphelper::ApplyItemSetFunc( rItemSet, bResult ));

    // own items
    return ItemConverter::ApplyItemSet( rItemSet ) || bResult;
}

const sal_uInt16 * TitleItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nTitleWhichPairs;
}

bool TitleItemConverter::GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const
{
    ::comphelper::ItemPropertyMapType & rMap( lcl_GetTitlePropertyMap());
    ::comphelper::ItemPropertyMapType::const_iterator aIt( rMap.find( nWhichId ));

    if( aIt == rMap.end())
        return false;

    rOutProperty =(*aIt).second;
    return true;
}


bool TitleItemConverter::ApplySpecialItem(
    sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
    throw( uno::Exception )
{
    bool bChanged = false;

    switch( nWhichId )
    {
        case SCHATTR_TEXT_DEGREES:
        {
            // convert int to double (divided by 100)
            double fVal = static_cast< double >(
                static_cast< const SfxInt32Item & >(
                    rItemSet.Get( nWhichId )).GetValue()) / 100.0;
            double fOldVal = 0.0;
            bool bPropExisted =
                ( GetPropertySet()->getPropertyValue( "TextRotation" ) >>= fOldVal );

            if( ! bPropExisted ||
                ( bPropExisted && fOldVal != fVal ))
            {
                GetPropertySet()->setPropertyValue( "TextRotation" , uno::makeAny( fVal ));
                bChanged = true;
            }
        }
        break;
    }

    return bChanged;
}

void TitleItemConverter::FillSpecialItem(
    sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
{
    switch( nWhichId )
    {
        case SCHATTR_TEXT_DEGREES:
        {
            // convert double to int (times 100)
            double fVal = 0;

            if( GetPropertySet()->getPropertyValue( "TextRotation" ) >>= fVal )
            {
                rOutItemSet.Put( SfxInt32Item( nWhichId, static_cast< sal_Int32 >(
                                                   ::rtl::math::round( fVal * 100.0 ) ) ));
            }
        }
        break;
   }
}

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
