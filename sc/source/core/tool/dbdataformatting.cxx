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

#include "dbdataformatting.hxx"
#include "rtl/ustring.hxx"

ScDBDataFormatting::ScDBDataFormatting():
    maTableStyleName        ( "Default" ),
    maFirstRowStripeStyle   ( "Default" ),
    maSecondRowStripeStyle  ( "Default" ),
    maFirstColStripeStyle   ( "Default" ),
    maSecondColStripeStyle  ( "Default" ),
    bBandedRows             ( false ),
    bBandedColumns          ( false ),
    maFirstRowStripeSize    ( 1 ),
    maSecondRowStripeSize   ( 1 ),
    maFirstColStripeSize    ( 1 ),
    maSecondColStripeSize   ( 1 )
{
}

ScDBDataFormatting::ScDBDataFormatting(const OUString& rTableStyleName, const OUString& rFirstRowStripeStyle, const OUString& rSecondRowStripeStyle, const OUString& rFirstColStripeStyle, const OUString& rSecondColStripeStyle, bool bBRows, bool bBCols) :
    maTableStyleName        ( rTableStyleName ),
    maFirstRowStripeStyle   ( rFirstRowStripeStyle),
    maSecondRowStripeStyle  ( rSecondRowStripeStyle ),
    maFirstColStripeStyle   ( rFirstColStripeStyle ),
    maSecondColStripeStyle  ( rSecondColStripeStyle ),
    bBandedRows             ( bBRows ),
    bBandedColumns          ( bBCols )
{
}

ScDBDataFormatting::ScDBDataFormatting( const ScDBDataFormatting& rTableFormatData ):
    maTableStyleName        ( rTableFormatData.maTableStyleName ),
    maFirstRowStripeStyle   ( rTableFormatData.maFirstRowStripeStyle ),
    maSecondRowStripeStyle  ( rTableFormatData.maSecondRowStripeStyle ),
    maFirstColStripeStyle   ( rTableFormatData.maFirstColStripeStyle ),
    maSecondColStripeStyle  ( rTableFormatData.maSecondColStripeStyle ),
    bBandedRows             ( rTableFormatData.bBandedRows ),
    bBandedColumns          ( rTableFormatData.bBandedColumns )
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
    bBandedRows = bBRows;
}

bool ScDBDataFormatting::GetBandedRows()
{
    return bBandedRows;
}

void ScDBDataFormatting::SetBandedColumns( bool bBCols )
{
    bBandedColumns = bBCols;
}

bool ScDBDataFormatting::GetBandedColumns()
{
    return bBandedColumns;
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
