/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "CharacterPropertyItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"
#include "RelativeSizeHelper.hxx"
#include <editeng/memberids.hrc>
#include <editeng/eeitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svl/stritem.hxx>

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/chart2/XFormattedString.hpp>

using namespace ::com::sun::star;

namespace
{
::comphelper::ItemPropertyMapType & lcl_GetCharacterPropertyPropertyMap()
{
    static ::comphelper::ItemPropertyMapType aCharacterPropertyMap(
        ::comphelper::MakeItemPropertyMap
        IPM_MAP_ENTRY( EE_CHAR_COLOR, "CharColor", 0 )
        IPM_MAP_ENTRY( EE_CHAR_LANGUAGE, "CharLocale", MID_LANG_LOCALE )
        IPM_MAP_ENTRY( EE_CHAR_LANGUAGE_CJK, "CharLocaleAsian", MID_LANG_LOCALE )
        IPM_MAP_ENTRY( EE_CHAR_LANGUAGE_CTL, "CharLocaleComplex", MID_LANG_LOCALE )
//         IPM_MAP_ENTRY( EE_CHAR_FONTHEIGHT, "CharHeight", 0 )
//         IPM_MAP_ENTRY( EE_CHAR_ITALIC, "CharPosture", 0 )
//         IPM_MAP_ENTRY( EE_CHAR_WEIGHT, "CharWeight", 0 )

        IPM_MAP_ENTRY( EE_CHAR_STRIKEOUT, "CharStrikeout", MID_CROSS_OUT )
        IPM_MAP_ENTRY( EE_CHAR_WLM, "CharWordMode", 0 )
        IPM_MAP_ENTRY( EE_CHAR_SHADOW, "CharShadowed", 0 )
        IPM_MAP_ENTRY( EE_CHAR_RELIEF, "CharRelief", 0 )
        IPM_MAP_ENTRY( EE_CHAR_OUTLINE, "CharContoured", 0 )
        IPM_MAP_ENTRY( EE_CHAR_EMPHASISMARK, "CharEmphasis", 0 )

        IPM_MAP_ENTRY( EE_PARA_WRITINGDIR, "WritingMode", 0 )

//         IPM_MAP_ENTRY( EE_CHAR_FONTHEIGHT_CJK, "CharHeightAsian", 0 )
//         IPM_MAP_ENTRY( EE_CHAR_FONTHEIGHT_CTL, "CharHeightComplex", 0 )
//         IPM_MAP_ENTRY( EE_CHAR_WEIGHT_CJK, "CharWeightAsian", 0 )
//         IPM_MAP_ENTRY( EE_CHAR_WEIGHT_CTL, "CharWeightComplex", 0 )
//         IPM_MAP_ENTRY( EE_CHAR_ITALIC_CJK, "CharPostureAsian", 0 )
//         IPM_MAP_ENTRY( EE_CHAR_ITALIC_CTL, "CharPostureComplex", 0 )
        IPM_MAP_ENTRY( EE_PARA_ASIANCJKSPACING, "ParaIsCharacterDistance", 0 )
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
    const uno::Reference< beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool ) :
        ItemConverter( rPropertySet, rItemPool )
{}

CharacterPropertyItemConverter::CharacterPropertyItemConverter(
    const uno::Reference< beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    ::std::auto_ptr< awt::Size > pRefSize,
    const ::rtl::OUString & rRefSizePropertyName,
    const uno::Reference< beans::XPropertySet > & rRefSizePropSet ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_pRefSize( pRefSize ),
        m_aRefSizePropertyName( rRefSizePropertyName ),
        m_xRefSizePropSet( rRefSizePropSet.is() ? rRefSizePropSet : rPropertySet )
{}

CharacterPropertyItemConverter::~CharacterPropertyItemConverter()
{}

const sal_uInt16 * CharacterPropertyItemConverter::GetWhichPairs() const
{
    return nCharacterPropertyWhichPairs;
}

bool CharacterPropertyItemConverter::GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const
{
    ::comphelper::ItemPropertyMapType & rMap( lcl_GetCharacterPropertyPropertyMap());
    ::comphelper::ItemPropertyMapType::const_iterator aIt( rMap.find( nWhichId ));

    if( aIt == rMap.end())
        return false;

    rOutProperty =(*aIt).second;
    return true;
}

void CharacterPropertyItemConverter::FillSpecialItem(
    sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
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
            SvxUnderlineItem aItem(UNDERLINE_NONE, EE_CHAR_UNDERLINE);
            bool bModified = false;

            uno::Any aValue( GetPropertySet()->getPropertyValue( C2U( "CharUnderline" )));
            if( aValue.hasValue())
            {
                aItem.PutValue( aValue, MID_TL_STYLE );
                bModified = true;
            }

            aValue = GetPropertySet()->getPropertyValue( C2U( "CharUnderlineHasColor" ));
            if( aValue.hasValue() &&
                ( *reinterpret_cast< const sal_Bool * >( aValue.getValue()) != sal_False ))
            {
                aItem.PutValue( aValue, MID_TL_HASCOLOR );
                bModified = true;
            }

            aValue = GetPropertySet()->getPropertyValue( C2U( "CharUnderlineColor" ));
            if( aValue.hasValue())
            {
                aItem.PutValue( aValue, MID_TL_COLOR );
                bModified = true;
            }

            if( bModified )
                rOutItemSet.Put( aItem );
        }
        break;

        case EE_CHAR_OVERLINE:
        {
            SvxOverlineItem aItem( UNDERLINE_NONE, EE_CHAR_OVERLINE );
            bool bModified = false;

            uno::Any aValue( GetPropertySet()->getPropertyValue( C2U( "CharOverline" ) ) );
            if ( aValue.hasValue() )
            {
                aItem.PutValue( aValue, MID_TL_STYLE );
                bModified = true;
            }

            aValue = GetPropertySet()->getPropertyValue( C2U( "CharOverlineHasColor" ) );
            if ( aValue.hasValue() &&
                 ( *reinterpret_cast< const sal_Bool* >( aValue.getValue() ) != sal_False ) )
            {
                aItem.PutValue( aValue, MID_TL_HASCOLOR );
                bModified = true;
            }

            aValue = GetPropertySet()->getPropertyValue( C2U( "CharOverlineColor" ) );
            if ( aValue.hasValue() )
            {
                aItem.PutValue( aValue, MID_TL_COLOR );
                bModified = true;
            }

            if ( bModified )
            {
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
                        if( GetRefSizePropertySet()->getPropertyValue( m_aRefSizePropertyName ) >>= aOldRefSize )
                        {
                            // calculate font height in view
                            fHeight = static_cast< float >(
                                RelativeSizeHelper::calculate( fHeight, aOldRefSize, *m_pRefSize ));
                            aValue <<= fHeight;
                        }
                    }

                    aItem.PutValue( aValue, MID_FONTHEIGHT );
                    rOutItemSet.Put( aItem );
                }
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
        break;

        case SID_CHAR_DLG_PREVIEW_STRING:
        {
            uno::Reference< chart2::XFormattedString > xFormattedString( GetPropertySet(), uno::UNO_QUERY );
            if( xFormattedString.is() )
            {
                ::rtl::OUString aString = xFormattedString->getString();
                rOutItemSet.Put( SfxStringItem( nWhichId, aString ) );
            }
            else
                rOutItemSet.Put( SfxStringItem( nWhichId, C2U( "" ) ) );
        }
        break;

        case EE_PARA_FORBIDDENRULES:
        case EE_PARA_HANGINGPUNCTUATION:
            rOutItemSet.DisableItem( nWhichId );
            break;
    }
}

bool CharacterPropertyItemConverter::ApplySpecialItem(
    sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
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
                static_cast< const SvxFontItem & >(
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
               static_cast< const SvxUnderlineItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue, MID_TL_STYLE ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharUnderline" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharUnderline" ), aValue );
                    bChanged = true;
                }
            }

            if( rItem.QueryValue( aValue, MID_TL_COLOR ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharUnderlineColor" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharUnderlineColor" ), aValue );
                    bChanged = true;
                }
            }

            if( rItem.QueryValue( aValue, MID_TL_HASCOLOR ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharUnderlineHasColor" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharUnderlineHasColor" ), aValue );
                    bChanged = true;
                }
            }
        }
        break;

        case EE_CHAR_OVERLINE:
        {
            const SvxOverlineItem& rItem = static_cast< const SvxOverlineItem & >( rItemSet.Get( nWhichId ) );

            if ( rItem.QueryValue( aValue, MID_TL_STYLE ) )
            {
                if ( aValue != GetPropertySet()->getPropertyValue( C2U( "CharOverline" ) ) )
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharOverline" ), aValue );
                    bChanged = true;
                }
            }

            if ( rItem.QueryValue( aValue, MID_TL_COLOR ) )
            {
                if ( aValue != GetPropertySet()->getPropertyValue( C2U( "CharOverlineColor" ) ) )
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharOverlineColor" ), aValue );
                    bChanged = true;
                }
            }

            if ( rItem.QueryValue( aValue, MID_TL_HASCOLOR ) )
            {
                if ( aValue != GetPropertySet()->getPropertyValue( C2U( "CharOverlineHasColor" ) ) )
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharOverlineHasColor" ), aValue );
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
                static_cast< const SvxPostureItem & >(
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
                static_cast< const SvxWeightItem & >(
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
                static_cast< const SvxFontHeightItem & >(
                    rItemSet.Get( nWhichId ));

            try
            {
                if( rItem.QueryValue( aValue, MID_FONTHEIGHT ) )
                {
                    bool bSetValue = false;
                    if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharHeight" ) + aPostfix ))
                        bSetValue = true;
                    else
                    {
                        if( m_pRefSize.get() )
                        {
                            awt::Size aNewRefSize = *m_pRefSize;
                            awt::Size aOldRefSize;
                            if( GetRefSizePropertySet()->getPropertyValue( m_aRefSizePropertyName ) >>= aOldRefSize )
                            {
                                if( aNewRefSize.Width != aOldRefSize.Width
                                    || aNewRefSize.Height != aOldRefSize.Height )
                                    bSetValue = true;
                            }
                        }
                    }
                    if( bSetValue )
                    {
                        // set new reference size only if there was a reference size before (auto-scaling on)
                        if( m_pRefSize.get() &&
                            GetRefSizePropertySet()->getPropertyValue( m_aRefSizePropertyName ).hasValue())
                        {
                            GetRefSizePropertySet()->setPropertyValue( m_aRefSizePropertyName,
                                                                    uno::makeAny( *m_pRefSize ));
                        }

                        GetPropertySet()->setPropertyValue( C2U( "CharHeight" ) + aPostfix, aValue );
                        bChanged = true;
                    }
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

uno::Reference< beans::XPropertySet >
    CharacterPropertyItemConverter::GetRefSizePropertySet() const
{
    return m_xRefSizePropSet;
}

} //  namespace wrapper
} //  namespace chart
