/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#include <tools/ref.hxx>
#include "svx/relfld.hxx"

// -----------------------------------------------------------------------

SvxRelativeField::SvxRelativeField( Window* pParent, WinBits nWinSize ) :
    MetricField( pParent, nWinSize )
{
    bNegativeEnabled = sal_False;
    bRelativeMode = sal_False;
    bRelative     = sal_False;

    SetDecimalDigits( 2 );
    SetMin( 0 );
    SetMax( 9999 );
}

// -----------------------------------------------------------------------

SvxRelativeField::SvxRelativeField( Window* pParent, const ResId& rResId ) :
    MetricField( pParent, rResId )
{
    bNegativeEnabled = sal_False;
    bRelativeMode = sal_False;
    bRelative     = sal_False;

    SetDecimalDigits( 2 );
    SetMin( 0 );
    SetMax( 9999 );
}

// -----------------------------------------------------------------------

void SvxRelativeField::Modify()
{
    MetricField::Modify();

    if ( bRelativeMode )
    {
        String  aStr = GetText();
        sal_Bool    bNewMode = bRelative;

        if ( bRelative )
        {
            const sal_Unicode* pStr = aStr.GetBuffer();

            while ( *pStr )
            {
                if( ( ( *pStr < sal_Unicode( '0' ) ) || ( *pStr > sal_Unicode( '9' ) ) ) &&
                    ( *pStr != sal_Unicode( '%' ) ) )
                {
                    bNewMode = sal_False;
                    break;
                }
                pStr++;
            }
        }
        else
        {
            xub_StrLen nPos = aStr.Search( sal_Unicode( '%' ) );

            if ( nPos != STRING_NOTFOUND )
                bNewMode = sal_True;
        }

        if ( bNewMode != bRelative )
            SetRelative( bNewMode );

        MetricField::Modify();
    }
}

// -----------------------------------------------------------------------

void SvxRelativeField::EnableRelativeMode( sal_uInt16 nMin,
                                           sal_uInt16 nMax, sal_uInt16 nStep )
{
    bRelativeMode = sal_True;
    nRelMin       = nMin;
    nRelMax       = nMax;
    nRelStep      = nStep;
    SetUnit( FUNIT_CM );
}

// -----------------------------------------------------------------------

void SvxRelativeField::SetRelative( sal_Bool bNewRelative )
{
    Selection aSelection = GetSelection();
    String aStr = GetText();

    if ( bNewRelative )
    {
        bRelative = sal_True;
        SetDecimalDigits( 0 );
        SetMin( nRelMin );
        SetMax( nRelMax );
        SetUnit( FUNIT_PERCENT );
    }
    else
    {
        bRelative = sal_False;
        SetDecimalDigits( 2 );
        SetMin( bNegativeEnabled ? -9999 : 0 );
        SetMax( 9999 );
        SetUnit( FUNIT_CM );
    }

    SetText( aStr );
    SetSelection( aSelection );
}


