/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/builder.hxx>
#include "LimitBox.hxx"
#include "dbu_qry.hrc"
#include "moduledbu.hxx"

#define ALL_STRING ModuleRes(STR_QUERY_LIMIT_ALL).toString()
#define ALL_INT -1

namespace global{

/// Default values
sal_Int64 aDefLimitAry[] =
{
    5,
    10,
    20,
    50
};

}

namespace dbaui
{


LimitBox::LimitBox( Window* pParent, WinBits nStyle )
    : NumericBox( pParent, nStyle )
{
    SetShowTrailingZeros( sal_False );
    SetDecimalDigits( 0 );
    SetMin( -1 );
    SetMax( SAL_MAX_INT64 );
    LoadDefaultLimits();

    Size aSize(
        GetSizePixel().Width(),
        CalcWindowSizePixel(GetEntryCount() + 1) );
    SetSizePixel(aSize);
}

LimitBox::~LimitBox()
{
}

OUString LimitBox::CreateFieldText( sal_Int64 nValue ) const
{
    if( nValue == ALL_INT )
        return ALL_STRING;
    else
        return NumericBox::CreateFieldText( nValue );
}

void LimitBox::Reformat()
{

    if( GetText() == ALL_STRING )
    {
        SetValue( ALL_INT );
    }
    ///Reformat only when text is not All
    else
    {
        ///Not allow user to type in -1
        if( GetText() == "-1" )
        {
            Undo();
        }
        else
            NumericBox::Reformat();
    }
}

void LimitBox::ReformatAll()
{
    ///First entry is All, which do not need numeric reformat
    if ( GetEntryCount() > 0 )
    {
        RemoveEntryAt( 0 );
        NumericBox::ReformatAll();
        InsertValue( ALL_INT, 0);
    }
    else
    {
        NumericBox::ReformatAll();
    }
}

Size LimitBox::GetOptimalSize() const
{
    return CalcSize(10,1);
}

///Initialize entries
void LimitBox::LoadDefaultLimits()
{
    InsertValue( ALL_INT );

    const unsigned nSize =
        sizeof(global::aDefLimitAry)/sizeof(global::aDefLimitAry[0]);
    for( unsigned nIndex = 0; nIndex< nSize; ++nIndex)
    {
        InsertValue( global::aDefLimitAry[nIndex] );
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeLimitBox( Window *pParent, VclBuilder::stringmap & )
{
    LimitBox* pBox = new LimitBox( pParent, WB_DROPDOWN | WB_VSCROLL );
    return pBox;
}


} ///dbaui namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
