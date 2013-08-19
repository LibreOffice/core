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
    //Calculating which style sheet is applicable to the give cell begins now
    //Create a whole row stripe map
    std::vector< OUString > aRowStripeSeq;
    //Create a whole column stripe map
    std::vector< OUString > aColStripeSeq;
    //The above two vectors represent a whole row/col stripe by
    //using consecutive entries of stylenames into the vector
    //just as it would look like in a rendered single stripe.
    ScDBDataFormatting aDBFormatting;
    GetTableFormatting( aDBFormatting );
    sal_Int32 i;
    sal_Int32 nFirstSize;
    sal_Int32 nSecondSize;
    OUString aFirstStyle;
    OUString aSecondStyle;
    if ( bRowStripe )
    {
        //Fill the row stripe sequence
        nFirstSize = aDBFormatting.GetFirstRowStripeSize();
        aFirstStyle = aDBFormatting.GetFirstRowStripeStyle();
        nSecondSize = aDBFormatting.GetSecondRowStripeSize();
        aSecondStyle = aDBFormatting.GetSecondRowStripeStyle();
        for ( i = 1; i <= nFirstSize; ++i )
        {
            //Add the first row stripe style to the RowStripeSeq vector nFirstSize
            //many times.
            aRowStripeSeq.push_back( aFirstStyle );
        }
        for ( i = 1; i<= nSecondSize; ++i )
        {
            //Similarly
            aRowStripeSeq.push_back( aSecondStyle );
        }
    }
    //Fill the column stripe sequence
    else
    {
        nFirstSize = aDBFormatting.GetFirstColStripeSize();
        aFirstStyle = aDBFormatting.GetFirstColStripeStyle();
        nSecondSize = aDBFormatting.GetSecondColStripeSize();
        aSecondStyle = aDBFormatting.GetSecondColStripeStyle();
        for ( i = 1; i<=nFirstSize; ++i )
        {
            aColStripeSeq.push_back( aFirstStyle );
        }
        for ( i = 1; i<=nSecondSize; ++i )
        {
            aColStripeSeq.push_back( aSecondStyle );
        }
    }
    //This approach of calculating the stripe sequence will be bad
    //if stripe sizes are huge, they generally aren't.
    //Now the math.
    //We used 1-based addressing for this instead of the 0-based used in Calc
    if ( bRowStripe )
    {
        sal_Int32 nStyleIndex = ( rPos.Row() + 1 ) % ( aRowStripeSeq.size() );
        if ( nStyleIndex == 0 )
        {
            //Return the last entry in the vector
            return aRowStripeSeq[ ( aRowStripeSeq.size() - 1 ) ];
        }
        else
        {
            //Return the nStyleIndex'th entry
            return aRowStripeSeq[ ( nStyleIndex - 1) ];
        }
    }
    else
    {
        sal_Int32 nStyleIndex = ( rPos.Row() + 1 ) % ( aRowStripeSeq.size() );
        if ( nStyleIndex == 0 )
        {
            //Return the last entry in the vector
            return aColStripeSeq[ ( aColStripeSeq.size() - 1 ) ];
        }
        else
        {
            //Return the nStyleIndex'th entry
            return aColStripeSeq[ ( nStyleIndex - 1 ) ];
        }
    }
}

