/*************************************************************************
 *
 *  $RCSfile: CharacterPropertyItemConverter.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-17 14:30:13 $
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
#include "CharacterPropertyItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"

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
        ( EE_CHAR_FONTHEIGHT,             C2U( "CharHeight" ))
//         ( EE_CHAR_ITALIC,                 C2U( "CharPosture" ))
//         ( EE_CHAR_WEIGHT,                 C2U( "CharWeight" ))
//         ( EE_CHAR_STRIKEOUT,              C2U( "CharStrikeout" ))
        ( EE_CHAR_WLM,                    C2U( "CharWordMode" ))
        ( EE_CHAR_SHADOW,                 C2U( "CharShadowed" ))
        ( EE_CHAR_RELIEF,                 C2U( "CharRelief" ))
        ( EE_CHAR_OUTLINE,                C2U( "CharContoured" ))
        ( EE_CHAR_EMPHASISMARK,           C2U( "CharEmphasis" ))
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
    SfxItemPool& rItemPool ) :
        ItemConverter( rPropertySet, rItemPool )
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
        {
            SvxFontItem aItem;

            aItem.PutValue( GetPropertySet()->getPropertyValue( C2U( "CharFontName" )),
                            MID_FONT_FAMILY_NAME );
            aItem.PutValue( GetPropertySet()->getPropertyValue( C2U( "CharFontFamily" )),
                            MID_FONT_FAMILY );
            aItem.PutValue( GetPropertySet()->getPropertyValue( C2U( "CharFontStyleName" )),
                            MID_FONT_STYLE_NAME );
            aItem.PutValue( GetPropertySet()->getPropertyValue( C2U( "CharFontCharSet" )),
                            MID_FONT_CHAR_SET );
            aItem.PutValue( GetPropertySet()->getPropertyValue( C2U( "CharFontPitch" )),
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
        {
            SvxPostureItem aItem;

            uno::Any aValue( GetPropertySet()->getPropertyValue( C2U( "CharPosture" )));
            if( aValue.hasValue())
            {
                aItem.PutValue( aValue, MID_POSTURE );
                rOutItemSet.Put( aItem );
            }
        }
        break;

        case EE_CHAR_WEIGHT:
        {
            SvxWeightItem aItem;

            uno::Any aValue( GetPropertySet()->getPropertyValue( C2U( "CharWeight" )));
            if( aValue.hasValue())
            {
                aItem.PutValue( aValue, MID_WEIGHT );
                rOutItemSet.Put( aItem );
            }
        }
        break;
    }
}

bool CharacterPropertyItemConverter::ApplySpecialItem(
    USHORT nWhichId, const SfxItemSet & rItemSet ) const
    throw( uno::Exception )
{
    bool bChanged = false;
    uno::Any aValue;

    switch( nWhichId )
    {
        case EE_CHAR_FONTINFO:
        {
            const SvxFontItem & rItem =
                reinterpret_cast< const SvxFontItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue, MID_FONT_FAMILY_NAME ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharFontName" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharFontName" ), aValue );
                    bChanged = true;
                }
            }
            if( rItem.QueryValue( aValue, MID_FONT_FAMILY ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharFontFamily" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharFontFamily" ), aValue );
                    bChanged = true;
                }
            }
            if( rItem.QueryValue( aValue, MID_FONT_STYLE_NAME ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharFontStyleName" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharFontStyleName" ), aValue );
                    bChanged = true;
                }
            }
            if( rItem.QueryValue( aValue, MID_FONT_CHAR_SET ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharFontCharSet" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharFontCharSet" ), aValue );
                    bChanged = true;
                }
            }
            if( rItem.QueryValue( aValue, MID_FONT_PITCH ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharFontPitch" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharFontPitch" ), aValue );
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
        {
            const SvxPostureItem & rItem =
                reinterpret_cast< const SvxPostureItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue, MID_POSTURE ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharPosture" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharPosture" ), aValue );
                    bChanged = true;
                }
            }
        }
        break;

        case EE_CHAR_WEIGHT:
        {
            const SvxWeightItem & rItem =
                reinterpret_cast< const SvxWeightItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue, MID_WEIGHT ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( C2U( "CharWeight" ) ))
                {
                    GetPropertySet()->setPropertyValue( C2U( "CharWeight" ), aValue );
                    bChanged = true;
                }
            }
        }
        break;
    }

    return bChanged;
}

} //  namespace wrapper
} //  namespace chart
