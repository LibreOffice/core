/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "dbdata.hxx"
#include "dbdataformatting.hxx"

OUString ScDBData::GetCellStyle( const ScAddress& rPos, bool bRowStripe )
{
    //first see if the DB Range has any DB Formatting at all
    if( !HasFormatting() )
        return OUString("Default");
    //Check if the bRowStripe specified applies?
    if ( bRowStripe == true && !(mpTableFormatData->GetBandedRows()) )
        return OUString("Default");//Asked banded rows, but not banded
    if ( bRowStripe == false && !(mpTableFormatData->GetBandedColumns()) )
        return OUString("Default");//asked banded cols, but not banded here
    //Calculating which style sheet is applicable to the give cell begins now
    ScDBDataFormatting aDBFormatting;
    GetTableFormatting( aDBFormatting );
    sal_Int32 i;
    sal_Int32 nFirstSize;
    sal_Int32 nSecondSize;
    OUString aFirstStyle;
    OUString aSecondStyle;
    sal_Int32 nStyleIndex;
    if ( bRowStripe )
    {
        //Fill the row stripe sequence
        nFirstSize = aDBFormatting.GetFirstRowStripeSize();
        aFirstStyle = aDBFormatting.GetFirstRowStripeStyle();
        nSecondSize = aDBFormatting.GetSecondRowStripeSize();
        aSecondStyle = aDBFormatting.GetSecondRowStripeStyle();
    }
    else
    {
        //Fill the col stripe sequence
        nFirstSize = aDBFormatting.GetFirstColStripeSize();
        aFirstStyle = aDBFormatting.GetFirstColStripeStyle();
        nSecondSize = aDBFormatting.GetSecondColStripeSize();
        aSecondStyle = aDBFormatting.GetSecondColStripeStyle();
    }
    if ( bRowStripe )
    {
        nStyleIndex = ( rPos.Row() + 1 ) % ( nFirstSize + nSecondSize );
    }
    else
    {
        nStyleIndex = ( rPos.Col() + 1 ) % ( nFirstSize + nSecondSize );
    }
    if ( nStyleIndex == 0 )
    {
        sal_Int32 nEntryNo = ( ( nFirstSize + nSecondSize ) );
        if ( nEntryNo <= nFirstSize )
            return aFirstStyle;
        else
            return aSecondStyle;
    }
    else
    {
        sal_Int32 nEntryNo = ( nStyleIndex );
        if ( nEntryNo <= nFirstSize )
            return aFirstStyle;
        else
            return aSecondStyle;
    }
}

