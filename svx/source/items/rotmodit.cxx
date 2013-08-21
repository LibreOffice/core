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
                                OUString& rText, const IntlWrapper * )  const
{
    rText = OUString();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText += "...: ";
//          break; // DURCHFALLEN!!!

        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText += OUString( GetValue() );
            break;
        default: ;//prevent warning
    }

    return ePres;
}

OUString SvxRotateModeItem::GetValueText( sal_uInt16 nVal ) const
{
    OUString aText;

    switch ( nVal )
    {
        case SVX_ROTATE_MODE_STANDARD:
        case SVX_ROTATE_MODE_TOP:
        case SVX_ROTATE_MODE_CENTER:
        case SVX_ROTATE_MODE_BOTTOM:
            aText = OUString("...");
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
