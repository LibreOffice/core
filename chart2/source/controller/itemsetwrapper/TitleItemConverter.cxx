/*************************************************************************
 *
 *  $RCSfile: TitleItemConverter.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:28 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "TitleItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "CharacterPropertyItemConverter.hxx"
#include "MultipleItemConverter.hxx"

#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#include <drafts/com/sun/star/chart2/XTitled.hpp>

#include <functional>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace
{
::chart::wrapper::ItemPropertyMapType & lcl_GetTitlePropertyMap()
{
    static ::chart::wrapper::ItemPropertyMapType aTitlePropertyMap(
        ::chart::wrapper::MakeItemPropertyMap
        ( SCHATTR_TEXT_STACKED,   C2U( "StackCharacters" ))
        );

    return aTitlePropertyMap;
};
} // anonymous namespace

namespace chart
{
namespace wrapper
{

// ========================================

class FormattedStringsConverter : public MultipleItemConverter
{
public:
    FormattedStringsConverter(
        const uno::Sequence< uno::Reference< chart2::XFormattedString > > & aStrings,
        SfxItemPool & rItemPool );
    virtual ~FormattedStringsConverter();

protected:
    virtual const USHORT * GetWhichPairs() const;
};

// ----------------------------------------

FormattedStringsConverter::FormattedStringsConverter(
    const uno::Sequence< uno::Reference< chart2::XFormattedString > > & aStrings,
    SfxItemPool & rItemPool )
        : MultipleItemConverter( rItemPool )
{
    for( sal_Int32 i = 0; i < aStrings.getLength(); ++i )
    {
        uno::Reference< beans::XPropertySet > xProp( aStrings[ i ], uno::UNO_QUERY );
        if( xProp.is())
            m_aConverters.push_back( new CharacterPropertyItemConverter( xProp, rItemPool ));
    }
}

FormattedStringsConverter::~FormattedStringsConverter()
{
}

const USHORT * FormattedStringsConverter::GetWhichPairs() const
{
    return nCharacterPropertyWhichPairs;
}

// ========================================

TitleItemConverter::TitleItemConverter(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel  ) :
        ItemConverter( rPropertySet, rItemPool )
{
    m_aConverters.push_back( new GraphicPropertyItemConverter(
                                 rPropertySet, rItemPool, rDrawModel,
                                 GraphicPropertyItemConverter::LINE_AND_FILL_PROPERTIES ));

    // CharacterProperties are not at the title but at its contained XFormattedString objects
    // take the first formatted string in the sequence
    uno::Reference< chart2::XTitle > xTitle( rPropertySet, uno::UNO_QUERY );
    if( xTitle.is())
    {
        uno::Sequence< uno::Reference< chart2::XFormattedString > > aStringSeq( xTitle->getText());
        if( aStringSeq.getLength() > 0 )
        {
            m_aConverters.push_back( new FormattedStringsConverter( aStringSeq, rItemPool ));
        }
    }
}

TitleItemConverter::~TitleItemConverter()
{
    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     DeleteItemConverterPtr() );
}

void TitleItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     FillItemSetFunc( rOutItemSet ));

    // own items
    ItemConverter::FillItemSet( rOutItemSet );
}

bool TitleItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = false;

    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ApplyItemSetFunc( rItemSet, bResult ));

    // own items
    return ItemConverter::ApplyItemSet( rItemSet ) || bResult;
}

const USHORT * TitleItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nTitleWhichPairs;
}

bool TitleItemConverter::GetItemPropertyName( USHORT nWhichId, ::rtl::OUString & rOutName ) const
{
    ItemPropertyMapType & rMap( lcl_GetTitlePropertyMap());
    ItemPropertyMapType::const_iterator aIt( rMap.find( nWhichId ));

    if( aIt == rMap.end())
        return false;

    rOutName =(*aIt).second;
    return true;
}


bool TitleItemConverter::ApplySpecialItem(
    USHORT nWhichId, const SfxItemSet & rItemSet ) const
{
    bool bChanged = false;

    switch( nWhichId )
    {
        case SCHATTR_TEXT_DEGREES:
        {
            // convert int to double (divided by 100)
            double fVal = static_cast< double >(
                reinterpret_cast< const SfxInt32Item & >(
                    rItemSet.Get( nWhichId )).GetValue()) / 100.0;
            double fOldVal = 0.0;
            bool bPropExisted =
                ( GetPropertySet()->getPropertyValue( C2U( "TextRotation" )) >>= fOldVal );

            if( ! bPropExisted ||
                ( bPropExisted && fOldVal != fVal ))
            {
                GetPropertySet()->setPropertyValue( C2U( "TextRotation" ), uno::makeAny( fVal ));
                bChanged = true;
            }
        }
        break;
    }

    return bChanged;
}

void TitleItemConverter::FillSpecialItem(
    USHORT nWhichId, SfxItemSet & rOutItemSet ) const
{
    switch( nWhichId )
    {
        case SCHATTR_TEXT_DEGREES:
        {
            // convert double to int (times 100)
            double fVal;

            if( GetPropertySet()->getPropertyValue( C2U( "TextRotation" )) >>= fVal )
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
