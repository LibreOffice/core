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

#include <com/sun/star/table/CellVertJustify2.hpp>

#include <svx/dialmgr.hxx>
#include <svx/rotmodit.hxx>
#include <rotationstrings.hrc>

using namespace ::com::sun::star;


SfxPoolItem* SvxRotateModeItem::CreateDefault() { return new  SvxRotateModeItem(SVX_ROTATE_MODE_STANDARD, 0);}


//  SvxRotateModeItem - orientation with turned text


SvxRotateModeItem::SvxRotateModeItem( SvxRotateMode eMode, sal_uInt16 _nWhich )
    : SfxEnumItem( _nWhich, eMode )
{
}

SvxRotateModeItem::SvxRotateModeItem( const SvxRotateModeItem& rItem )
    : SfxEnumItem( rItem )
{
}

SvxRotateModeItem::~SvxRotateModeItem()
{
}

OUString SvxRotateModeItem::GetValueText(SvxRotateMode nVal)
{
    assert(nVal <= SVX_ROTATE_MODE_BOTTOM && "enum overflow!");
    return SvxResId(RID_SVXITEMS_ROTATE_MODE[static_cast<size_t>(nVal)]);
}

bool SvxRotateModeItem::GetPresentation(
                                SfxItemPresentation ePres,
                                MapUnit /*eCoreUnit*/, MapUnit /*ePresUnit*/,
                                OUString& rText, const IntlWrapper& )  const
{
    rText.clear();

    switch ( ePres )
    {
        case SfxItemPresentation::Complete:
            rText += GetValueText(GetValue());
            return true;

        case SfxItemPresentation::Nameless:
            rText += OUString::number( GetValue() );
            return true;
        default: ;//prevent warning
    }

    return false;
}

sal_uInt16 SvxRotateModeItem::GetValueCount() const
{
    return 4;       // STANDARD, TOP, CENTER, BOTTOM
}

SvxRotateModeItem* SvxRotateModeItem::Clone( SfxItemPool* ) const
{
    return new SvxRotateModeItem( *this );
}

bool SvxRotateModeItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    sal_Int32 nUno = table::CellVertJustify2::STANDARD;
    switch ( GetValue() )
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
    SetValue( eSvx );
    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
