/*************************************************************************
 *
 *  $RCSfile: relfld.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#pragma hdrstop

#include <tools/ref.hxx>
#include "relfld.hxx"

// -----------------------------------------------------------------------

SvxRelativeField::SvxRelativeField( Window* pParent, WinBits nWinSize ) :
    MetricField( pParent, nWinSize )
{
    bRelativeMode = FALSE;
    bRelative     = FALSE;

    SetDecimalDigits( 2 );
    SetMin( 0 );
    SetMax( 9999 );
}

// -----------------------------------------------------------------------

SvxRelativeField::SvxRelativeField( Window* pParent, const ResId& rResId ) :
    MetricField( pParent, rResId )
{
    bRelativeMode = FALSE;
    bRelative     = FALSE;

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
        BOOL    bNewMode = bRelative;

        if ( bRelative )
        {
            const sal_Unicode* pStr = aStr.GetBuffer();

            while ( *pStr )
            {
                if( ( ( *pStr < sal_Unicode( '0' ) ) || ( *pStr > sal_Unicode( '9' ) ) ) &&
                    ( *pStr != sal_Unicode( '%' ) ) )
                {
                    bNewMode = FALSE;
                    break;
                }
                pStr++;
            }
        }
        else
        {
            xub_StrLen nPos = aStr.Search( sal_Unicode( '%' ) );

            if ( nPos != STRING_NOTFOUND )
                bNewMode = TRUE;
        }

        if ( bNewMode != bRelative )
            SetRelative( bNewMode );

        MetricField::Modify();
    }
}

// -----------------------------------------------------------------------

void SvxRelativeField::EnableRelativeMode( USHORT nMin,
                                           USHORT nMax, USHORT nStep )
{
    bRelativeMode = TRUE;
    nRelMin       = nMin;
    nRelMax       = nMax;
    nRelStep      = nStep;
    SetUnit( FUNIT_CM );
}

// -----------------------------------------------------------------------

void SvxRelativeField::SetRelative( BOOL bNewRelative )
{
    Selection aSelection = GetSelection();
    String aStr = GetText();

    if ( bNewRelative )
    {
        bRelative = TRUE;
        SetDecimalDigits( 0 );
        SetMin( nRelMin );
        SetMax( nRelMax );
        SetCustomUnitText( String( sal_Unicode( '%' ) ) );
        SetUnit( FUNIT_CUSTOM );
    }
    else
    {
        bRelative = FALSE;
        SetDecimalDigits( 2 );
        SetMin( 0 );
        SetMax( 9999 );
        SetUnit( FUNIT_CM );
    }

    SetText( aStr );
    SetSelection( aSelection );
}


