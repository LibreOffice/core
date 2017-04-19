/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/builderfactory.hxx>
#include "LimitBox.hxx"
#include "strings.hrc"
#include "core_resource.hxx"
#include "core_resource.hxx"

#define ALL_STRING DBA_RES(STR_QUERY_LIMIT_ALL)
#define ALL_INT -1

namespace global{

/// Default values
sal_Int64 const aDefLimitAry[] =
{
    5,
    10,
    20,
    50
};

}

namespace dbaui
{


LimitBox::LimitBox( vcl::Window* pParent, WinBits nStyle )
    : NumericBox( pParent, nStyle )
{
    SetShowTrailingZeros( false );
    SetDecimalDigits( 0 );
    SetMin( -1 );
    SetMax( SAL_MAX_INT64 );
    LoadDefaultLimits();

    Size aSize(
        GetSizePixel().Width(),
        CalcWindowSizePixel(GetEntryCount() + 1) );
    SetSizePixel(aSize);
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
    return CalcBlockSize(10,1);
}

///Initialize entries
void LimitBox::LoadDefaultLimits()
{
    InsertValue( ALL_INT );

    for(long nIndex : global::aDefLimitAry)
    {
        InsertValue( nIndex );
    }
}

VCL_BUILDER_FACTORY_ARGS( LimitBox, WB_DROPDOWN | WB_VSCROLL )

} ///dbaui namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
