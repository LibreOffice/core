/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: relfld.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 08:38:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#include <tools/ref.hxx>
#include "relfld.hxx"

// -----------------------------------------------------------------------

SvxRelativeField::SvxRelativeField( Window* pParent, WinBits nWinSize ) :
    MetricField( pParent, nWinSize )
{
    bNegativeEnabled = FALSE;
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
    bNegativeEnabled = FALSE;
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
        SetMin( bNegativeEnabled ? -9999 : 0 );
        SetMax( 9999 );
        SetUnit( FUNIT_CM );
    }

    SetText( aStr );
    SetSelection( aSelection );
}


