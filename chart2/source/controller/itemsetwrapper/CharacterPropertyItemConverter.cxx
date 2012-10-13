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

        IPM_MAP_ENTRY( EE_CHAR_STRIKEOUT, "CharStrikeout", MID_CROSS_OUT )
        IPM_MAP_ENTRY( EE_CHAR_WLM, "CharWordMode", 0 )
        IPM_MAP_ENTRY( EE_CHAR_SHADOW, "CharShadowed", 0 )
        IPM_MAP_ENTRY( EE_CHAR_RELIEF, "CharRelief", 0 )
        IPM_MAP_ENTRY( EE_CHAR_OUTLINE, "CharContoured", 0 )
        IPM_MAP_ENTRY( EE_CHAR_EMPHASISMARK, "CharEmphasis", 0 )

        IPM_MAP_ENTRY( EE_PARA_WRITINGDIR, "WritingMode", 0 )

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

SAL_WNODEPRECATED_DECLARATIONS_PUSH
CharacterPropertyItemConverter::CharacterPropertyItemConverter(
    const uno::Reference< beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    ::std::auto_ptr< awt::Size > pRefSize,
    const OUString & rRefSizePropertyName,
    const uno::Reference< beans::XPropertySet > & rRefSizePropSet ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_pRefSize( pRefSize ),
        m_aRefSizePropertyName( rRefSizePropertyName ),
        m_xRefSizePropSet( rRefSizePropSet.is() ? rRefSizePropSet : rPropertySet )
{}
SAL_WNODEPRECATED_DECLARATIONS_POP

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
            OUString aPostfix;
            if( nWhichId == EE_CHAR_FONTINFO_CJK )
                aPostfix = "Asian";
            else if( nWhichId == EE_CHAR_FONTINFO_CTL )
                aPostfix = "Complex";

            SvxFontItem aItem( nWhichId );

            aItem.PutValue( GetPropertySet()->getPropertyValue( "CharFontName" + aPostfix),
                            MID_FONT_FAMILY_NAME );
            aItem.PutValue( GetPropertySet()->getPropertyValue( "CharFontFamily" + aPostfix),
                            MID_FONT_FAMILY );
            aItem.PutValue( GetPropertySet()->getPropertyValue( "CharFontStyleName" + aPostfix),
                            MID_FONT_STYLE_NAME );
            aItem.PutValue( GetPropertySet()->getPropertyValue( "CharFontCharSet" + aPostfix),
                            MID_FONT_CHAR_SET );
            aItem.PutValue( GetPropertySet()->getPropertyValue( "CharFontPitch" + aPostfix),
                            MID_FONT_PITCH );

            rOutItemSet.Put( aItem );
        }
        break;

        case EE_CHAR_UNDERLINE:
        {
            SvxUnderlineItem aItem(UNDERLINE_NONE, EE_CHAR_UNDERLINE);
            bool bModified = false;

            uno::Any aValue( GetPropertySet()->getPropertyValue( "CharUnderline" ));
            if( aValue.hasValue())
            {
                aItem.PutValue( aValue, MID_TL_STYLE );
                bModified = true;
            }

            aValue = GetPropertySet()->getPropertyValue( "CharUnderlineHasColor" );
            if( aValue.hasValue() &&
                ( *reinterpret_cast< const sal_Bool * >( aValue.getValue()) != sal_False ))
            {
                aItem.PutValue( aValue, MID_TL_HASCOLOR );
                bModified = true;
            }

            aValue = GetPropertySet()->getPropertyValue( "CharUnderlineColor" );
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

            uno::Any aValue( GetPropertySet()->getPropertyValue( "CharOverline" ) );
            if ( aValue.hasValue() )
            {
                aItem.PutValue( aValue, MID_TL_STYLE );
                bModified = true;
            }

            aValue = GetPropertySet()->getPropertyValue( "CharOverlineHasColor" );
            if ( aValue.hasValue() &&
                 ( *reinterpret_cast< const sal_Bool* >( aValue.getValue() ) != sal_False ) )
            {
                aItem.PutValue( aValue, MID_TL_HASCOLOR );
                bModified = true;
            }

            aValue = GetPropertySet()->getPropertyValue( "CharOverlineColor" );
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
            OUString aPostfix;
            if( nWhichId == EE_CHAR_ITALIC_CJK )
                aPostfix = "Asian";
            else if( nWhichId == EE_CHAR_ITALIC_CTL )
                aPostfix = "Complex";

            SvxPostureItem aItem( ITALIC_NONE, nWhichId );

            uno::Any aValue( GetPropertySet()->getPropertyValue( "CharPosture" + aPostfix));
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
            OUString aPostfix;
            if( nWhichId == EE_CHAR_WEIGHT_CJK )
                aPostfix = "Asian";
            else if( nWhichId == EE_CHAR_WEIGHT_CTL )
                aPostfix = "Complex";

            SvxWeightItem aItem( WEIGHT_NORMAL, nWhichId );

            uno::Any aValue( GetPropertySet()->getPropertyValue( "CharWeight" + aPostfix));
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
            OUString aPostfix;
            if( nWhichId == EE_CHAR_FONTHEIGHT_CJK )
                aPostfix = "Asian";
            else if( nWhichId == EE_CHAR_FONTHEIGHT_CTL )
                aPostfix = "Complex";

            SvxFontHeightItem aItem( 240, 100, nWhichId );

            try
            {
                uno::Any aValue( GetPropertySet()->getPropertyValue( "CharHeight" + aPostfix ));
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
            catch( const uno::Exception & ex )
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
                OUString aString = xFormattedString->getString();
                rOutItemSet.Put( SfxStringItem( nWhichId, aString ) );
            }
            else
                rOutItemSet.Put( SfxStringItem( nWhichId, OUString("") ) );
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
            OUString aPostfix;
            if( nWhichId == EE_CHAR_FONTINFO_CJK )
                aPostfix = "Asian";
            else if( nWhichId == EE_CHAR_FONTINFO_CTL )
                aPostfix = "Complex";

            const SvxFontItem & rItem =
                static_cast< const SvxFontItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue, MID_FONT_FAMILY_NAME ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( "CharFontName" + aPostfix ))
                {
                    GetPropertySet()->setPropertyValue( "CharFontName" + aPostfix, aValue );
                    bChanged = true;
                }
            }
            if( rItem.QueryValue( aValue, MID_FONT_FAMILY ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( "CharFontFamily" + aPostfix ))
                {
                    GetPropertySet()->setPropertyValue( "CharFontFamily" + aPostfix, aValue );
                    bChanged = true;
                }
            }
            if( rItem.QueryValue( aValue, MID_FONT_STYLE_NAME ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( "CharFontStyleName" + aPostfix ))
                {
                    GetPropertySet()->setPropertyValue( "CharFontStyleName" + aPostfix, aValue );
                    bChanged = true;
                }
            }
            if( rItem.QueryValue( aValue, MID_FONT_CHAR_SET ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( "CharFontCharSet" + aPostfix ))
                {
                    GetPropertySet()->setPropertyValue( "CharFontCharSet" + aPostfix, aValue );
                    bChanged = true;
                }
            }
            if( rItem.QueryValue( aValue, MID_FONT_PITCH ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( "CharFontPitch" + aPostfix ))
                {
                    GetPropertySet()->setPropertyValue( "CharFontPitch" + aPostfix, aValue );
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
                if( aValue != GetPropertySet()->getPropertyValue( "CharUnderline" ))
                {
                    GetPropertySet()->setPropertyValue( "CharUnderline" , aValue );
                    bChanged = true;
                }
            }

            if( rItem.QueryValue( aValue, MID_TL_COLOR ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( "CharUnderlineColor" ))
                {
                    GetPropertySet()->setPropertyValue( "CharUnderlineColor" , aValue );
                    bChanged = true;
                }
            }

            if( rItem.QueryValue( aValue, MID_TL_HASCOLOR ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( "CharUnderlineHasColor" ))
                {
                    GetPropertySet()->setPropertyValue( "CharUnderlineHasColor" , aValue );
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
                if ( aValue != GetPropertySet()->getPropertyValue( "CharOverline" ) )
                {
                    GetPropertySet()->setPropertyValue( "CharOverline" , aValue );
                    bChanged = true;
                }
            }

            if ( rItem.QueryValue( aValue, MID_TL_COLOR ) )
            {
                if ( aValue != GetPropertySet()->getPropertyValue( "CharOverlineColor" ) )
                {
                    GetPropertySet()->setPropertyValue( "CharOverlineColor" , aValue );
                    bChanged = true;
                }
            }

            if ( rItem.QueryValue( aValue, MID_TL_HASCOLOR ) )
            {
                if ( aValue != GetPropertySet()->getPropertyValue( "CharOverlineHasColor" ) )
                {
                    GetPropertySet()->setPropertyValue( "CharOverlineHasColor" , aValue );
                    bChanged = true;
                }
            }
        }
        break;

        case EE_CHAR_ITALIC:
        case EE_CHAR_ITALIC_CJK:
        case EE_CHAR_ITALIC_CTL:
        {
            OUString aPostfix;
            if( nWhichId == EE_CHAR_ITALIC_CJK )
                aPostfix = "Asian";
            else if( nWhichId == EE_CHAR_ITALIC_CTL )
                aPostfix = "Complex";

            const SvxPostureItem & rItem =
                static_cast< const SvxPostureItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue, MID_POSTURE ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( "CharPosture" + aPostfix))
                {
                    GetPropertySet()->setPropertyValue( "CharPosture" + aPostfix, aValue );
                    bChanged = true;
                }
            }
        }
        break;

        case EE_CHAR_WEIGHT:
        case EE_CHAR_WEIGHT_CJK:
        case EE_CHAR_WEIGHT_CTL:
        {
            OUString aPostfix;
            if( nWhichId == EE_CHAR_WEIGHT_CJK )
                aPostfix = "Asian" ;
            else if( nWhichId == EE_CHAR_WEIGHT_CTL )
                aPostfix = "Complex";

            const SvxWeightItem & rItem =
                static_cast< const SvxWeightItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue, MID_WEIGHT ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( "CharWeight" + aPostfix))
                {
                    GetPropertySet()->setPropertyValue( "CharWeight" + aPostfix, aValue );
                    bChanged = true;
                }
            }
        }
        break;

        case EE_CHAR_FONTHEIGHT:
        case EE_CHAR_FONTHEIGHT_CJK:
        case EE_CHAR_FONTHEIGHT_CTL:
        {
            OUString aPostfix;
            if( nWhichId == EE_CHAR_FONTHEIGHT_CJK )
                aPostfix = "Asian";
            else if( nWhichId == EE_CHAR_FONTHEIGHT_CTL )
                aPostfix = "Complex";

            const SvxFontHeightItem & rItem =
                static_cast< const SvxFontHeightItem & >(
                    rItemSet.Get( nWhichId ));

            try
            {
                if( rItem.QueryValue( aValue, MID_FONTHEIGHT ) )
                {
                    bool bSetValue = false;
                    if( aValue != GetPropertySet()->getPropertyValue( "CharHeight" + aPostfix ))
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

                        GetPropertySet()->setPropertyValue( "CharHeight" + aPostfix, aValue );
                        bChanged = true;
                    }
                }
            }
            catch( const uno::Exception & ex )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
