/*************************************************************************
 *
 *  $RCSfile: xmlehelp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:31:44 $
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
#include <limits.h>
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _BIGINT_HXX //autogen wg. BigInt
#include <tools/bigint.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _XMLOFF_XMLEHELP_HXX
#include "xmlehelp.hxx"
#endif

#ifndef _XMLOFF_XMKYWD_HXX
#include "xmlkywd.hxx"
#endif

using namespace ::rtl;

void SvXMLExportHelper::AddLength( long nValue, MapUnit eValueUnit,
                                           OUStringBuffer& rOut,
                                   MapUnit eOutUnit )
{
    // the sign is processed seperatly
    if( nValue < 0 )
    {
        nValue = -nValue;
        rOut.append( sal_Unicode('-') );
    }

    // The new length is (nVal * nMul)/(nDiv*nFac*10)
    long nMul = 1000;
    long nDiv = 1;
    long nFac = 100;
    const sal_Char *pUnit = NULL;
    switch( eValueUnit )
    {
    case MAP_TWIP:
        switch( eOutUnit )
        {
        case MAP_100TH_MM:
        case MAP_10TH_MM:
            DBG_ASSERT( MAP_INCH == eOutUnit,
                        "output unit not supported for twip values" );
        case MAP_MM:
            // 0.01mm = 0.57twip (exactly)
            nMul = 25400;   // 25.4 * 1000
            nDiv = 1440;    // 72 * 20;
            nFac = 100;
            pUnit = sXML_unit_mm;
            break;

        case MAP_CM:
            // 0.001cm = 0.57twip (exactly)
            nMul = 25400;   // 2.54 * 10000
            nDiv = 1440;    // 72 * 20;
            nFac = 1000;
            pUnit = sXML_unit_cm;
            break;

        case MAP_POINT:
            // 0.01pt = 0.2twip (exactly)
            nMul = 1000;
            nDiv = 20;
            nFac = 100;
            pUnit = sXML_unit_pt;
            break;

        case MAP_INCH:
        default:
            DBG_ASSERT( MAP_INCH == eOutUnit,
                        "output unit not supported for twip values" );
            // 0.0001in = 0.144twip (exactly)
            nMul = 100000;
            nDiv = 1440;    // 72 * 20;
            nFac = 10000;
            pUnit = sXML_unit_inch;
            break;
        }
        break;

    case MAP_POINT:
        // 1pt = 1pt (exactly)
        DBG_ASSERT( MAP_POINT == eOutUnit,
                    "output unit not supported for pt values" );
        nMul = 10;
        nDiv = 1;
        nFac = 1;
        pUnit = sXML_unit_pt;
        break;

    case MAP_100TH_MM:
        switch( eOutUnit )
        {
        case MAP_100TH_MM:
        case MAP_10TH_MM:
            DBG_ASSERT( MAP_INCH == eOutUnit,
                        "output unit not supported for 1/100mm values" );
        case MAP_MM:
            // 0.01mm = 1 mm/100 (exactly)
            nMul = 10;
            nDiv = 1;
            nFac = 100;
            pUnit = sXML_unit_mm;
            break;

        case MAP_CM:
            // 0.001mm = 1 mm/100 (exactly)
            nMul = 10;
            nDiv = 1;   // 72 * 20;
            nFac = 1000;
            pUnit = sXML_unit_cm;
            break;

        case MAP_POINT:
            // 0.01pt = 0.35 mm/100 (exactly)
            nMul = 72000;
            nDiv = 2540;
            nFac = 100;
            pUnit = sXML_unit_pt;
            break;

        case MAP_INCH:
        default:
            DBG_ASSERT( MAP_INCH == eOutUnit,
                        "output unit not supported for 1/100mm values" );
            // 0.0001in = 0.254 mm/100 (exactly)
            nMul = 100000;
            nDiv = 2540;
            nFac = 10000;
            pUnit = sXML_unit_inch;
            break;
        }
        break;
    }


    long nLongVal;
    BOOL bOutLongVal = TRUE;
    if( nValue > LONG_MAX / nMul )
    {
        // A big int is required for calculation
        BigInt nBigVal( nValue );
        BigInt nBigFac( nFac );
        nBigVal *= nMul;
        nBigVal /= nDiv;
        nBigVal += 5;
        nBigVal /= 10;

        if( nBigVal.IsLong() )
        {
            // To convert the value into a string a long is sufficient
            nLongVal = (long)nBigVal;
        }
        else
        {
            BigInt nBigFac( nFac );
            BigInt nBig10( 10 );
            rOut.append( (sal_Int32)(nBigVal / nBigFac) );
            if( !(nBigVal % nBigFac).IsZero() )
            {
                rOut.append( sal_Unicode('.') );
                while( nFac > 1 && !(nBigVal % nBigFac).IsZero() )
                {
                    nFac /= 10;
                    nBigFac = nFac;
                    rOut.append( (sal_Int32)((nBigVal / nBigFac) % nBig10 ) );
                }
            }
            bOutLongVal = FALSE;
        }
    }
    else
    {
        nLongVal = nValue * nMul;
        nLongVal /= nDiv;
        nLongVal += 5;
        nLongVal /= 10;
    }

    if( bOutLongVal )
    {
        rOut.append( (sal_Int32)(nLongVal / nFac) );
        if( nFac > 1 && (nLongVal % nFac) != 0 )
        {
            rOut.append( sal_Unicode('.') );
            while( nFac > 1 && (nLongVal % nFac) != 0 )
            {
                nFac /= 10;
                rOut.append( (sal_Int32)((nLongVal / nFac) % 10) );
            }
        }
    }

    if( pUnit )
        rOut.appendAscii( pUnit );
#if 0
    const sal_Char *pUnit;
    long nFac = 1;
    switch( eOutUnit )
    {
    case MAP_100TH_MM:
        nFac *= 10L;
    case MAP_10TH_MM:
        nFac *= 10L;
        eOutUnit = MAP_MM;
    case MAP_MM:
        // 0.01mm
        nFac *= 100L;
        pUnit = sXML_unit_mm;
        break;

    case MAP_CM:
#ifdef EXACT_VALUES
        // 0.001cm
        nFac *= 1000L;
#else
        // 0.01cm
        nFac *= 100L;
#endif
        pUnit = sXML_unit_cm;
        break;

    case MAP_TWIP:
    case MAP_POINT:
#ifdef EXACT_VALUES
        // 0.01pt
        nFac *= 100L;
#else
        // 0.1pt
        nFac *= 10L;
#endif
        pUnit = sXML_unit_pt;
        break;

    case MAP_1000TH_INCH:
        nFac *= 10L;
    case MAP_100TH_INCH:
        nFac *= 10L;
    case MAP_10TH_INCH:
        nFac *= 10L;
    case MAP_INCH:
    default:
        eOutUnit = MAP_INCH;
#ifdef EXACT_VALUES
        // 0.0001in
        nFac *= 10000L;
#else
        // 0.01in
        nFac *= 100L;
#endif
        pUnit = sXML_unit_inch;
        break;
    }

    if( eValueUnit != eOutUnit )
        nValue = OutputDevice::LogicToLogic( nValue, eValueUnit, eOutUnit );

    rOut.append( nValue / nFac );
    if( nFac > 1 && (nValue % nFac) != 0 )
    {
        rOut.append( sal_Unicode('.') );
        while( nFac > 1 && (nValue % nFac) != 0 )
        {
            nFac /= 10L;
            rOut.append( (nValue / nFac) % 10L );
        }
    }

    rOut.appendAscii( OUString::createFromAscii( pUnit ) );
#endif

}

void SvXMLExportHelper::AddPercentage( long nValue, OUStringBuffer& rOut )
{
    rOut.append( nValue );
    rOut.append( sal_Unicode('%' ) );
}
