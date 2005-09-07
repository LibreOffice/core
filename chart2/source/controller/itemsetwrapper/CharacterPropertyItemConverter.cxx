/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CharacterPropertyItemConverter.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:28:47 $
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
#include "CharacterPropertyItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"
#include "RelativeSizeHelper.hxx"

#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX
#define ITEMID_UNDERLINE EE_CHAR_UNDERLINE
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#define ITEMID_FONT EE_CHAR_FONTINFO
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX
#define ITEMID_CROSSEDOUT EE_CHAR_STRIKEOUT
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX
#define ITEMID_POSTURE EE_CHAR_ITALIC
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX
#define ITEMID_WEIGHT EE_CHAR_WEIGHT
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX
#define ITEMID_FONTHEIGHT EE_CHAR_FONTHEIGHT
#include <svx/fhgtitem.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif

using namespace ::com::sun::star;

namespace
{
::comphelper::ItemPropertyMapType & lcl_GetCharacterPropertyPropertyMap()
{
    static ::comphelper::ItemPropertyMapType aCharacterPropertyMap(
        ::comphelper::MakeItemPropertyMap
        ( EE_CHAR_COLOR,                  C2U( "CharColor" ))
        ( EE_CHAR_LANGUAGE,               C2U( "CharLocale" ))
//         ( EE_CHAR_FONTHEIGHT,             C2U( "CharHeight" ))
//         ( EE_CHAR_ITALIC,                 C2U( "CharPosture" ))
//         ( EE_CHAR_WEIGHT,                 C2U( "CharWeight" ))
//         ( EE_CHAR_STRIKEOUT,              C2U( "CharStrikeout" ))
        ( EE_CHAR_WLM,                    C2U( "CharWordMode" ))
        ( EE_CHAR_SHADOW,                 C2U( "CharShadowed" ))
        ( EE_CHAR_RELIEF,                 C2U( "CharRelief" ))
        ( EE_CHAR_OUTLINE,                C2U( "CharContoured" ))
        ( EE_CHAR_EMPHASISMARK,           C2U( "CharEmphasis" ))

        ( EE_CHAR_LANGUAGE_CJK,           C2U( "CharLocaleAsian" ))
        ( EE_CHAR_LANGUAGE_CTL,           C2U( "CharLocaleComplex" ))
//         ( EE_CHAR_FONTHEIGHT_CJK,         C2U( "CharHeightAsian" ))
//         ( EE_CHAR_FONTHEIGHT_CTL,         C2U( "CharHeightComplex" ))
//         ( EE_CHAR_WEIGHT_CJK,             C2U( "CharWeightAsian" ))
//         ( EE_CHAR_WEIGHT_CTL,             C2U( "CharWeightComplex" ))
//         ( EE_CHAR_ITALIC_CJK,             C2U( "CharPostureAsian" ))
//         ( EE_CHAR_ITALIC_CTL,             C2U( "CharPostureComplex" ))

        );

    return aCharacterPropertyMap;
}
} // anonymous namespace

// ========================================

namespace chart
{
namespace wrapper
{

CharacterPropertyItemConverter::CharacterPropertyItemConverter(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    ::std::auto_ptr< awt::Size > pRefSize,
    const ::rtl::OUString & rRefSizePropertyName ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_pRefSize( pRefSize ),
        m_aRefSizePropertyName( rRefSizePropertyName )
{}

CharacterPropertyItemConverter::~CharacterPropertyItemConverter()
{}

const USHORT * CharacterPropertyItemConverter::GetWhichPairs() const
{
    return nCharacterPropertyWhichPairs;
}

bool CharacterPropertyItemConverter::GetItemPropertyName( USHORT nWhichId, ::rtl::OUString & rOutName ) const
{
    ::comphelper::ItemPropertyMapType & rMap( lcl_GetCharacterPropertyPropertyMap());
    ::comphelper::ItemPropertyMapType::const_iterator aIt( rMap.find( nWhichId ));

    if( aIt == rMap.end())
        return false;

    rOutName =(*aIt).second;
    return true;
}

void CharacterPropertyItemConverter::FillSpecialItem(
    USHORT nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
{
    switch( nWhichId )
    {
        case EE_CHAR_FONTINFO:
        case EE_CHAR_FONTINFO_CJK:
        case EE_CHAR_FONTINFO_CTL:
        {
            ::rtl::OUString aPostfix;
            if( nWhichId == EE_CHAR_FONTINFO_CJK )
                aPostfix = C2U( "Asian" );
            else if( nWhichId == EE_CHAR_FONTINFO_CTL )
                aPostfix = C2U( "Complex" );

            SvxFontItem aItem( nWhichId );

            aItem.PutValue( GetPropertySet()->getPropertyValue( C2U( "CharFontName" ) + aPostfix),
                            MID_FONT_FAMILY_NAME );
            aItem.PutValue( GetPropertySet()->getPropertyValue( C2U( "CharFontFamily" ) + aPostfix),
                            MID_FONT_FAMILY );
            aItem.PutValue( GetPropertySet()->getPropertyValue( C2U( "CharFontStyleName" ) + aPostfix),
                            MID_FONT_STYLE_NAME );
            aItem.PutValue( GetPropertySet()->getPropertyValue( C2U( "CharFontCharSet" ) + aPostfix),
                            MID_FONT_CHAR_SET );
            aItem.PutValue( GetPropertySet()->getPropertyValue( C2U( "CharFontPitch" ) + aPostfix),
                            MID_FONT_PITCH );

            rOutItemSet.Put( aItem );
        }
        break;

        case EE_CHAR_UNDERLINE:
        {
            SvxUnderlineItem aItem;
            bool bModified = false;

            uno::Any aValue( GetPropertySet()->getPropertyValue( C2U( "CharUnderline" )));
            if( aValue.hasValue())
            {
                aItem.PutValue( aValue, MID_UNDERLINE );
                bModified = true;
            }

            aValue = GetPropertySet()->getPropertyValue( C2U( "CharUnderlineHasColor" ));
            if( aValue.hasValue() &&
                ( *reinterpret_cast< const sal_Bool * >( aValue.getValue()) != sal_False ))
            {
                aItem.PutValue( aValue, MID_UL_HASCOLOR );
                bModified = true;
            }

            aValue = GetPropertySet()->getPropertyValue( C2U( "CharUnderlineColor" ));
            if( aValue.hasValue())
            {
                aItem.PutValue( aValue, MID_UL_COLOR );
                bModified = true;
            }

            if( bModified )
                rOutItemSet.Put( aItem );
        }
        break;

        case EE_CHAR_STRIKEOUT:
        {
            SvxCrossedOutItem aItem;

            uno::Any aValue( GetPropertySet()->getPropertyValue( C2U( "CharStrikeout" )));
            if( aValue.hasValue())
            {
                // default member-id is MID_CROSSED_OUT (bool flag)
                aItem.PutValue( aValue, MID_CROSS_OUT );
                rOutItemSet.Put( aItem );
            }
        }
        break;

        case EE_CHAR_ITALIC:
        case EE_CHAR_ITALIC_CJK:
        case EE_CHAR_ITALIC_CTL:
        {
            ::rtl::OUString aPostfix;
            if( nWhichId == EE_CHAR_ITALIC_CJK )
                aPostfix = C2U( "Asian" );
            else if( nWhichId == EE_CHAR_ITALIC_CTL )
                aPostfix = C2U( "Complex" );

            SvxPostureItem aItem( ITALIC_NONE, nWhichId );

            uno::Any aValue( GetPropertySet()->getPropertyValue( C2U( "CharPosture" ) + aPostfix));
            if( aValue.hasValue())
            {
                aItem.PutValue( aValue, MID_POSTURE );
                rOutItemSet.Put( aItem );
            }
        }
        break;

        case EE_CHAR_WEIGHT:
        case EE_CHAR_WEIGHT_CJK:
        case EE_CHAR_WEIGHT_CTL:
        {
            ::rtl::OUString aPostfix;
            if( nWhichId == EE_CHAR_WEIGHT_CJK )
                aPostfix = C2U( "Asian" );
            else if( nWhichId == EE_CHAR_WEIGHT_CTL )
                aPostfix = C2U( "Complex" );

            SvxWeightItem aItem( WEIGHT_NORMAL, nWhichId );

            uno::Any aValue( GetPropertySet()->getPropertyValue( C2U( "CharWeight" ) + aPostfix));
            if( aValue.hasValue())
            {
                aItem.PutValue( aValue, MID_WEIGHT );
                rOutItemSet.Put( aItem );
            }
        }
        break;

        case EE_CHAR_FONTHEIGHT:
        case EE_CHAR_FONTHEIGHT_CJK:
        case EE_CHAR_FONTHEIGHT_CTL:
        {
            ::rtl::OUString aPostfix;
            if( nWhichId == EE_CHAR_FONTHEIGHT_CJK )
                aPostfix = C2U( "Asian" );
            else if( nWhichId == EE_CHAR_FONTHEIGHT_CTL )
                aPostfix = C2U( "Complex" );

            SvxFontHeightItem aItem( 240, 100, nWhichId );

            try
            {
                uno::Any aValue( GetPropertySet()->getPropertyValue( C2U( "CharHeight" ) + aPostfix ));
                float fHeight;
                if( aValue >>= fHeight )
                {
                    if( m_pRefSize.get())
                    {
                        awt::Size aOldRefSize;
                        if( GetPropertySet()->getPropertyValue( m_aRefSizePropertyName ) >>= aOldRefSize )
                        {
                            // calculate font height in view
                            fHeight = RelativeSizeHelper::calculate(
                                fHeight, aOldRefSize, *m_pRefSize );
                            aValue <<= fHeight;
                        }
                    }

                    aItem.PutValue( aValue );
                    rOutItemSet.Put( aItem );
                }
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
        break;
    }
}

bool CharacterPropertyItemConverter::ApplySpecialItem(
    USHORT nWhichId, const SfxItemSet & rItemSet )
    throw( uno::Exception )
{
    bool bChanged = false;
    uno::Any aValue;

    switch( nWhichId )
    {
        case EE_CHAR_FONTINFO:
        case EE_CHAR_FONTINFO_CJK:
        case EE_CHAR_FONTINFO_CTL:
        {
            ::rtl::OUString aPostfix;
            if( nWhichId == EE_CHAR_FONTINFO_CJK )
                aPostfix = C2U( "Asian" );
            else if( nWhichId == EE_CHAR_FONTINFO_CTL )
                aPostfix = C2U( "Complex" );

            const SvxFontItem & rItem =
                reinterpret_cast< const SvxFontItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue, MID_FONT_FAMILY_NAME ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharFontName" ) + aPostfix ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharFontName" ) + aPostfix, aValue );
                    bChanged = true;
                }
            }
            if( rItem.QueryValue( aValue, MID_FONT_FAMILY ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharFontFamily" ) + aPostfix ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharFontFamily" ) + aPostfix, aValue );
                    bChanged = true;
                }
            }
            if( rItem.QueryValue( aValue, MID_FONT_STYLE_NAME ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharFontStyleName" ) + aPostfix ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharFontStyleName" ) + aPostfix, aValue );
                    bChanged = true;
                }
            }
            if( rItem.QueryValue( aValue, MID_FONT_CHAR_SET ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharFontCharSet" ) + aPostfix ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharFontCharSet" ) + aPostfix, aValue );
                    bChanged = true;
                }
            }
            if( rItem.QueryValue( aValue, MID_FONT_PITCH ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharFontPitch" ) + aPostfix ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharFontPitch" ) + aPostfix, aValue );
                    bChanged = true;
                }
            }
        }
        break;

        case EE_CHAR_UNDERLINE:
        {
            const SvxUnderlineItem & rItem =
                reinterpret_cast< const SvxUnderlineItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue, MID_UNDERLINE ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharUnderline" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharUnderline" ), aValue );
                    bChanged = true;
                }
            }

            if( rItem.QueryValue( aValue, MID_UL_COLOR ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharUnderlineColor" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharUnderlineColor" ), aValue );
                    bChanged = true;
                }
            }

            if( rItem.QueryValue( aValue, MID_UL_HASCOLOR ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharUnderlineHasColor" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharUnderlineHasColor" ), aValue );
                    bChanged = true;
                }
            }
        }
        break;

        case EE_CHAR_STRIKEOUT:
        {
            const SvxCrossedOutItem & rItem =
                reinterpret_cast< const SvxCrossedOutItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue, MID_CROSS_OUT ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharStrikeout" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharStrikeout" ), aValue );
                    bChanged = true;
                }
            }
        }
        break;

        case EE_CHAR_ITALIC:
        case EE_CHAR_ITALIC_CJK:
        case EE_CHAR_ITALIC_CTL:
        {
            ::rtl::OUString aPostfix;
            if( nWhichId == EE_CHAR_ITALIC_CJK )
                aPostfix = C2U( "Asian" );
            else if( nWhichId == EE_CHAR_ITALIC_CTL )
                aPostfix = C2U( "Complex" );

            const SvxPostureItem & rItem =
                reinterpret_cast< const SvxPostureItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue, MID_POSTURE ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharPosture" ) + aPostfix))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharPosture" ) + aPostfix, aValue );
                    bChanged = true;
                }
            }
        }
        break;

        case EE_CHAR_WEIGHT:
        case EE_CHAR_WEIGHT_CJK:
        case EE_CHAR_WEIGHT_CTL:
        {
            ::rtl::OUString aPostfix;
            if( nWhichId == EE_CHAR_WEIGHT_CJK )
                aPostfix = C2U( "Asian" );
            else if( nWhichId == EE_CHAR_WEIGHT_CTL )
                aPostfix = C2U( "Complex" );

            const SvxWeightItem & rItem =
                reinterpret_cast< const SvxWeightItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue, MID_WEIGHT ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharWeight" ) + aPostfix))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharWeight" ) + aPostfix, aValue );
                    bChanged = true;
                }
            }
        }
        break;

        case EE_CHAR_FONTHEIGHT:
        case EE_CHAR_FONTHEIGHT_CJK:
        case EE_CHAR_FONTHEIGHT_CTL:
        {
            ::rtl::OUString aPostfix;
            if( nWhichId == EE_CHAR_FONTHEIGHT_CJK )
                aPostfix = C2U( "Asian" );
            else if( nWhichId == EE_CHAR_FONTHEIGHT_CTL )
                aPostfix = C2U( "Complex" );

            const SvxFontHeightItem & rItem =
                reinterpret_cast< const SvxFontHeightItem & >(
                    rItemSet.Get( nWhichId ));

            try
            {
                if( rItem.QueryValue( aValue ) &&
                    aValue != GetPropertySet()->getPropertyValue( C2U( "CharHeight" ) + aPostfix ))
                {
                    if( m_pRefSize.get())
                    {
                        GetPropertySet()->setPropertyValue( m_aRefSizePropertyName,
                                                            uno::makeAny( *m_pRefSize ));
                    }

                    GetPropertySet()->setPropertyValue( C2U( "CharHeight" ) + aPostfix, aValue );
                    bChanged = true;
                }
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
        break;
    }

    return bChanged;
}

} //  namespace wrapper
} //  namespace chart
