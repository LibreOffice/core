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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <tools/stream.hxx>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/table/CellVertJustify.hpp>
#include <com/sun/star/table/ShadowLocation.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/table/TableOrientation.hpp>
#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/util/SortField.hpp>
#include <com/sun/star/util/SortFieldType.hpp>
#include <com/sun/star/table/CellOrientation.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include "svx/rotmodit.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

// STATIC DATA -----------------------------------------------------------

TYPEINIT1_FACTORY(SvxRotateModeItem, SfxEnumItem, new SvxRotateModeItem(SVX_ROTATE_MODE_STANDARD, 0));


//-----------------------------------------------------------------------
//  SvxRotateModeItem - Ausrichtung bei gedrehtem Text
//-----------------------------------------------------------------------

SvxRotateModeItem::SvxRotateModeItem( SvxRotateMode eMode, sal_uInt16 _nWhich )
    : SfxEnumItem( _nWhich, (sal_uInt16)eMode )
{
}

SvxRotateModeItem::SvxRotateModeItem( const SvxRotateModeItem& rItem )
    : SfxEnumItem( rItem )
{
}

__EXPORT SvxRotateModeItem::~SvxRotateModeItem()
{
}

SfxPoolItem* __EXPORT SvxRotateModeItem::Create( SvStream& rStream, sal_uInt16 ) const
{
    sal_uInt16 nVal;
    rStream >> nVal;
    return new SvxRotateModeItem( (SvxRotateMode) nVal,Which() );
}

SfxItemPresentation __EXPORT SvxRotateModeItem::GetPresentation(
                                SfxItemPresentation ePres,
                                SfxMapUnit /*eCoreUnit*/, SfxMapUnit /*ePresUnit*/,
                                String& rText, const IntlWrapper * )  const
{
    rText.Erase();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText.AppendAscii("...");
            rText.AppendAscii(": ");
//          break; // DURCHFALLEN!!!

        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText += UniString::CreateFromInt32( GetValue() );
            break;
        default: ;//prevent warning
    }

    return ePres;
}

String __EXPORT SvxRotateModeItem::GetValueText( sal_uInt16 nVal ) const
{
    String aText;

    switch ( nVal )
    {
        case SVX_ROTATE_MODE_STANDARD:
        case SVX_ROTATE_MODE_TOP:
        case SVX_ROTATE_MODE_CENTER:
        case SVX_ROTATE_MODE_BOTTOM:
            aText.AppendAscii("...");
            break;
        default:
            DBG_ERROR("SvxRotateModeItem: falscher enum");
            break;
    }
    return aText;
}

sal_uInt16 __EXPORT SvxRotateModeItem::GetValueCount() const
{
    return 4;       // STANDARD, TOP, CENTER, BOTTOM
}

SfxPoolItem* __EXPORT SvxRotateModeItem::Clone( SfxItemPool* ) const
{
    return new SvxRotateModeItem( *this );
}

sal_uInt16 __EXPORT SvxRotateModeItem::GetVersion( sal_uInt16 /*nFileVersion*/ ) const
{
    return 0;
}

//  QueryValue/PutValue: Der ::com::sun::star::table::CellVertJustify enum wird mitbenutzt...

sal_Bool SvxRotateModeItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    table::CellVertJustify eUno = table::CellVertJustify_STANDARD;
    switch ( (SvxRotateMode)GetValue() )
    {
        case SVX_ROTATE_MODE_STANDARD: eUno = table::CellVertJustify_STANDARD; break;
        case SVX_ROTATE_MODE_TOP:      eUno = table::CellVertJustify_TOP;       break;
        case SVX_ROTATE_MODE_CENTER:   eUno = table::CellVertJustify_CENTER;    break;
        case SVX_ROTATE_MODE_BOTTOM:   eUno = table::CellVertJustify_BOTTOM;    break;
    }
    rVal <<= eUno;
    return sal_True;
}

sal_Bool SvxRotateModeItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    table::CellVertJustify eUno;
    if(!(rVal >>= eUno))
    {
        sal_Int32 nValue = 0;
        if(!(rVal >>= nValue))
            return sal_False;
        eUno = (table::CellVertJustify)nValue;
    }

    SvxRotateMode eSvx = SVX_ROTATE_MODE_STANDARD;
    switch (eUno)
    {
        case table::CellVertJustify_STANDARD: eSvx = SVX_ROTATE_MODE_STANDARD; break;
        case table::CellVertJustify_TOP:       eSvx = SVX_ROTATE_MODE_TOP;      break;
        case table::CellVertJustify_CENTER:   eSvx = SVX_ROTATE_MODE_CENTER;    break;
        case table::CellVertJustify_BOTTOM:   eSvx = SVX_ROTATE_MODE_BOTTOM;    break;
        default: ;//prevent warning
    }
    SetValue( (sal_uInt16)eSvx );
    return sal_True;
}



