/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <com/sun/star/table/CellVertJustify2.hpp>
#include <com/sun/star/table/ShadowLocation.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/table/TableOrientation.hpp>
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

SvxRotateModeItem::~SvxRotateModeItem()
{
}

SfxPoolItem* SvxRotateModeItem::Create( SvStream& rStream, sal_uInt16 ) const
{
    sal_uInt16 nVal;
    rStream >> nVal;
    return new SvxRotateModeItem( (SvxRotateMode) nVal,Which() );
}

SfxItemPresentation SvxRotateModeItem::GetPresentation(
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

String SvxRotateModeItem::GetValueText( sal_uInt16 nVal ) const
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
            OSL_FAIL("SvxRotateModeItem: falscher enum");
            break;
    }
    return aText;
}

sal_uInt16 SvxRotateModeItem::GetValueCount() const
{
    return 4;       // STANDARD, TOP, CENTER, BOTTOM
}

SfxPoolItem* SvxRotateModeItem::Clone( SfxItemPool* ) const
{
    return new SvxRotateModeItem( *this );
}

sal_uInt16 SvxRotateModeItem::GetVersion( sal_uInt16 /*nFileVersion*/ ) const
{
    return 0;
}

bool SvxRotateModeItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    sal_Int32 nUno = table::CellVertJustify2::STANDARD;
    switch ( (SvxRotateMode)GetValue() )
    {
        case SVX_ROTATE_MODE_STANDARD: nUno = table::CellVertJustify2::STANDARD; break;
        case SVX_ROTATE_MODE_TOP:      nUno = table::CellVertJustify2::TOP;      break;
        case SVX_ROTATE_MODE_CENTER:   nUno = table::CellVertJustify2::CENTER;   break;
        case SVX_ROTATE_MODE_BOTTOM:   nUno = table::CellVertJustify2::BOTTOM;   break;
    }
    rVal <<= nUno;
    return true;
}

bool SvxRotateModeItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    sal_Int32 nUno(0);
    if(!(rVal >>= nUno))
    {
        nUno = table::CellVertJustify2::STANDARD;
    }

    SvxRotateMode eSvx = SVX_ROTATE_MODE_STANDARD;
    switch (nUno)
    {
        case table::CellVertJustify2::STANDARD: eSvx = SVX_ROTATE_MODE_STANDARD; break;
        case table::CellVertJustify2::TOP:      eSvx = SVX_ROTATE_MODE_TOP;      break;
        case table::CellVertJustify2::CENTER:   eSvx = SVX_ROTATE_MODE_CENTER;   break;
        case table::CellVertJustify2::BOTTOM:   eSvx = SVX_ROTATE_MODE_BOTTOM;   break;
        default: ;//prevent warning
    }
    SetValue( (sal_uInt16)eSvx );
    return true;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
