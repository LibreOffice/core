/*************************************************************************
 *
 *  $RCSfile: itemtype.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:21 $
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

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#pragma hdrstop

#include "svxids.hrc"

#define ITEMID_COLOR_TABLE  SID_COLOR_TABLE

#include "xtable.hxx"
#include "drawitem.hxx"
#include "itemtype.hxx"

// -----------------------------------------------------------------------

XubString GetMetricText( long nVal, SfxMapUnit eSrcUnit, SfxMapUnit eDestUnit )
{
    sal_Bool bNeg = sal_False;
    long nRet = 0;
    XubString sRet;

    if ( nVal < 0 )
    {
        bNeg = sal_True;
        nVal *= -1;
    }

    switch ( eDestUnit )
    {
        case SFX_MAPUNIT_100TH_MM:
        case SFX_MAPUNIT_10TH_MM:
        case SFX_MAPUNIT_MM:
        case SFX_MAPUNIT_CM:
        {
            nRet = (long)OutputDevice::LogicToLogic(
                nVal, (MapUnit)eSrcUnit, (MapUnit)SFX_MAPUNIT_100TH_MM );

            switch ( eDestUnit )
            {
                case SFX_MAPUNIT_100TH_MM:  nRet *= 1000; break;
                case SFX_MAPUNIT_10TH_MM:   nRet *= 100; break;
                case SFX_MAPUNIT_MM:        nRet *= 10; break;
            }
            break;
        }

        case SFX_MAPUNIT_1000TH_INCH:
        case SFX_MAPUNIT_100TH_INCH:
        case SFX_MAPUNIT_10TH_INCH:
        case SFX_MAPUNIT_INCH:
        {
            nRet = (long)OutputDevice::LogicToLogic(
                nVal, (MapUnit)eSrcUnit, (MapUnit)SFX_MAPUNIT_1000TH_INCH );

            switch ( eDestUnit )
            {
                case SFX_MAPUNIT_1000TH_INCH:   nRet *= 1000; break;
                case SFX_MAPUNIT_100TH_INCH:    nRet *= 100; break;
                case SFX_MAPUNIT_10TH_INCH:     nRet *= 10; break;
            }
            break;
        }

        case SFX_MAPUNIT_POINT:
        case SFX_MAPUNIT_TWIP:
        case SFX_MAPUNIT_PIXEL:
            return String::CreateFromInt32( (long)OutputDevice::LogicToLogic(
                        nVal, (MapUnit)eSrcUnit, (MapUnit)eDestUnit ));

        default:
            DBG_ERROR( "not supported mapunit" );
            return sRet;
    }

    if ( SFX_MAPUNIT_CM == eDestUnit || SFX_MAPUNIT_INCH == eDestUnit )
    {
        long nMod = nRet % 10;

        if ( nMod > 4 )
            nRet += 10 - nMod;
        else if ( nMod > 0 )
            nRet -= nMod;
    }

    if ( bNeg )
        sRet += sal_Unicode('-');

    long nDiff = 1000;
    for( int nDigits = 4; nDigits; --nDigits, nDiff /= 10 )
    {
        if ( nRet < nDiff )
            sRet += sal_Unicode('0');
        else
            sRet += String::CreateFromInt32( nRet / nDiff );
        nRet %= nDiff;
        if( 4 == nDigits )
        {
            sRet += sal_Unicode(',');
            if( !nRet )
            {
                sRet += sal_Unicode('0');
                break;
            }
        }
        else if( !nRet )
            break;
    }
    return sRet;
}

// -----------------------------------------------------------------------

XubString GetSvxString( sal_uInt16 nId )
{
    return SVX_RESSTR( nId );
}

#ifndef SVX_LIGHT

// -----------------------------------------------------------------------

XubString GetColorString( const Color& rCol )
{
    const SfxPoolItem* pItem = NULL;
    XColorTable* pCol = NULL;
    SfxObjectShell* pSh = SfxObjectShell::Current();

    if ( pSh && ( pItem = pSh->GetItem( SID_COLOR_TABLE ) ) )
            pCol = ( (SvxColorTableItem*)pItem )->GetColorTable();

    XubString sStr;

    FASTBOOL bFound = sal_False;
    ColorData nColData =
        RGB_COLORDATA( rCol.GetRed(), rCol.GetGreen(), rCol.GetBlue() );
    sal_uInt16 nColor = 0, nColCount = 16;

    static ColorData aColAry[] = {
        COL_BLACK, COL_BLUE, COL_GREEN, COL_CYAN,
        COL_RED, COL_MAGENTA, COL_BROWN, COL_GRAY,
        COL_LIGHTGRAY, COL_LIGHTBLUE, COL_LIGHTGREEN, COL_LIGHTCYAN,
        COL_LIGHTRED, COL_LIGHTMAGENTA, COL_YELLOW, COL_WHITE };

    while ( !bFound && nColor < nColCount )
    {
        if ( aColAry[nColor] == nColData )
            bFound = sal_True;
        else
            nColor++;
    }

    if ( nColor < nColCount )
        sStr = SVX_RESSTR( RID_SVXITEMS_COLOR_BEGIN + nColor + 1 );

    if ( !sStr.Len() )
    {
        sStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "RGB" ));
        sStr += sal_Unicode('(');
        sStr += String::CreateFromInt32( rCol.GetRed() );
        sStr += cpDelim;
        sStr += String::CreateFromInt32( rCol.GetGreen() );
        sStr += cpDelim;
        sStr += String::CreateFromInt32( rCol.GetBlue() );
        sStr += sal_Unicode(')');
    }
    return sStr;
}

#endif

// -----------------------------------------------------------------------

sal_uInt16 GetMetricId( SfxMapUnit eUnit )
{
    sal_uInt16 nId = RID_SVXITEMS_METRIC_MM;

    switch ( eUnit )
    {
        case SFX_MAPUNIT_100TH_MM:
        case SFX_MAPUNIT_10TH_MM:
        case SFX_MAPUNIT_MM:
            nId = RID_SVXITEMS_METRIC_MM;
            break;

        case SFX_MAPUNIT_CM:
            nId = RID_SVXITEMS_METRIC_CM;
            break;

        case SFX_MAPUNIT_1000TH_INCH:
        case SFX_MAPUNIT_100TH_INCH:
        case SFX_MAPUNIT_10TH_INCH:
        case SFX_MAPUNIT_INCH:
            nId = RID_SVXITEMS_METRIC_INCH;
            break;

        case SFX_MAPUNIT_POINT:
            nId = RID_SVXITEMS_METRIC_POINT;
            break;

        case SFX_MAPUNIT_TWIP:
            nId = RID_SVXITEMS_METRIC_TWIP;
            break;

        case SFX_MAPUNIT_PIXEL:
            nId = RID_SVXITEMS_METRIC_PIXEL;
            break;

        default:
            DBG_ERROR( "not supported mapunit" );
    }
    return nId;
}


