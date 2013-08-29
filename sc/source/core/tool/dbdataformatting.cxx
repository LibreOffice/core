/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dbdataformatting.hxx"
#include "rtl/ustring.hxx"

ScDBDataFormatting::ScDBDataFormatting():
    maTableStyleName        ( "Default" ),
    maFirstRowStripeStyle   ( "Default" ),
    maSecondRowStripeStyle  ( "Default" ),
    maFirstColStripeStyle   ( "Default" ),
    maSecondColStripeStyle  ( "Default" ),
    nFirstRowStripeSize    ( 1 ),
    nSecondRowStripeSize   ( 1 ),
    nFirstColStripeSize    ( 1 ),
    nSecondColStripeSize   ( 1 ),
    mbBandedRows            ( false ),
    mbBandedColumns         ( false )
{
}

ScDBDataFormatting::ScDBDataFormatting(const OUString& rTableStyleName, const OUString& rFirstRowStripeStyle, const OUString& rSecondRowStripeStyle, const OUString& rFirstColStripeStyle, const OUString& rSecondColStripeStyle, bool bBRows, bool bBCols) :
    maTableStyleName        ( rTableStyleName ),
    maFirstRowStripeStyle   ( rFirstRowStripeStyle),
    maSecondRowStripeStyle  ( rSecondRowStripeStyle ),
    maFirstColStripeStyle   ( rFirstColStripeStyle ),
    maSecondColStripeStyle  ( rSecondColStripeStyle ),
    nFirstRowStripeSize     ( 1 ),
    nSecondRowStripeSize    ( 1 ),
    nFirstColStripeSize     ( 1 ),
    nSecondColStripeSize    ( 1 ),
    mbBandedRows            ( bBRows ),
    mbBandedColumns         ( bBCols )
{
}

ScDBDataFormatting::ScDBDataFormatting( const ScDBDataFormatting& rTableFormatData ):
    maTableStyleName        ( rTableFormatData.maTableStyleName ),
    maFirstRowStripeStyle   ( rTableFormatData.maFirstRowStripeStyle ),
    maSecondRowStripeStyle  ( rTableFormatData.maSecondRowStripeStyle ),
    maFirstColStripeStyle   ( rTableFormatData.maFirstColStripeStyle ),
    maSecondColStripeStyle  ( rTableFormatData.maSecondColStripeStyle ),
    nFirstRowStripeSize     ( rTableFormatData.nFirstRowStripeSize ),
    nSecondRowStripeSize    ( rTableFormatData.nSecondRowStripeSize ),
    nFirstColStripeSize     ( rTableFormatData.nFirstColStripeSize ),
    nSecondColStripeSize    ( rTableFormatData.nSecondColStripeSize ),
    mbBandedRows            ( rTableFormatData.mbBandedRows ),
    mbBandedColumns         ( rTableFormatData.mbBandedColumns )
{
}

void ScDBDataFormatting::SetTableStyleName( const OUString& rTableStyleName )
{
    maTableStyleName = rTableStyleName;
}

const OUString& ScDBDataFormatting::GetTableStyleName()
{
    return maTableStyleName;
}

void ScDBDataFormatting::SetBandedRows( bool bBRows )
{
    mbBandedRows = bBRows;
}

bool ScDBDataFormatting::GetBandedRows()
{
    return mbBandedRows;
}

void ScDBDataFormatting::SetBandedColumns( bool bBCols )
{
    mbBandedColumns = bBCols;
}

bool ScDBDataFormatting::GetBandedColumns()
{
    return mbBandedColumns;
}

const OUString& ScDBDataFormatting::GetFirstRowStripeStyle() const
{
    return maFirstRowStripeStyle;
}

const OUString& ScDBDataFormatting::GetSecondRowStripeStyle() const
{
    return maSecondRowStripeStyle;
}

const OUString& ScDBDataFormatting::GetFirstColStripeStyle() const
{
    return maFirstColStripeStyle;
}

const OUString& ScDBDataFormatting::GetSecondColStripeStyle() const
{
    return maSecondColStripeStyle;
}

void ScDBDataFormatting::SetFirstRowStripeStyle( const OUString& aStyleName )
{
    maFirstRowStripeStyle = aStyleName;
}

void ScDBDataFormatting::SetSecondRowStripeStyle( const OUString& aStyleName )
{
    maSecondRowStripeStyle = aStyleName;
}

void ScDBDataFormatting::SetFirstColStripeStyle( const OUString& aStyleName )
{
    maFirstColStripeStyle = aStyleName;
}

void ScDBDataFormatting::SetSecondColStripeStyle( const OUString& aStyleName )
{
    maSecondColStripeStyle = aStyleName;
}

void ScDBDataFormatting::SetFirstRowStripeSize( sal_Int32 nSize)
{
    nFirstRowStripeSize = nSize;
}
void ScDBDataFormatting::SetSecondRowStripeSize( sal_Int32 nSize )
{
    nSecondRowStripeSize = nSize;
}
void ScDBDataFormatting::SetFirstColStripeSize( sal_Int32 nSize )
{
    nFirstColStripeSize = nSize;
}
void ScDBDataFormatting::SetSecondColStripeSize( sal_Int32 nSize )
{
    nSecondColStripeSize = nSize;
}

sal_Int32 ScDBDataFormatting::GetFirstRowStripeSize()
{
    return nFirstRowStripeSize;
}

sal_Int32 ScDBDataFormatting::GetSecondRowStripeSize()
{
    return nSecondRowStripeSize;
}

sal_Int32 ScDBDataFormatting::GetFirstColStripeSize()
{
    return nFirstColStripeSize;
}

sal_Int32 ScDBDataFormatting::GetSecondColStripeSize()
{
    return nSecondColStripeSize;
}
