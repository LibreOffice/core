/*************************************************************************
 *
 *  $RCSfile: weighhdl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:05 $
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

#ifndef _XMLOFF_PROPERTYHANDLER_FONTWEIGHTTYPES_HXX
#include <weighhdl.hxx>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _INC_LIMITS
#include <limits.h>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star::uno;

struct FontWeightMapper
{
    FontWeight eWeight;
    USHORT nValue;
};

FontWeightMapper const aFontWeightMap[] =
{
    { WEIGHT_DONTKNOW,              0 },
    { WEIGHT_THIN,                  100 },
    { WEIGHT_ULTRALIGHT,            150 },
    { WEIGHT_LIGHT,                 250 },
    { WEIGHT_SEMILIGHT,             350 },
    { WEIGHT_NORMAL,                400 },
    { WEIGHT_MEDIUM,                450 },
    { WEIGHT_SEMIBOLD,              600 },
    { WEIGHT_BOLD,                  700 },
    { WEIGHT_ULTRABOLD,             800 },
    { WEIGHT_BLACK,                 900 },
    { (FontWeight)USHRT_MAX,       1000 }
};

///////////////////////////////////////////////////////////////////////////////
//
// class XMLFmtBreakBeforePropHdl
//

XMLFontWeightPropHdl::~XMLFontWeightPropHdl()
{
    // Nothing to do
}

sal_Bool XMLFontWeightPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nWeight = 0;

    if( rStrImpValue.compareToAscii( sXML_weight_normal ) == 0 )
    {
        nWeight = 400;
        bRet = sal_True;
    }
    else if( rStrImpValue.compareToAscii( sXML_weight_bold ) == 0 )
    {
        nWeight = 700;
        bRet = sal_True;
    }
    else
    {
        sal_Int32 nTemp;
        if( ( bRet = rUnitConverter.convertNumber( nTemp, rStrImpValue, 100, 900 ) ) )
            nWeight = nTemp;
    }

    if( bRet )
    {
        bRet = sal_False;

        for( int i = 0; aFontWeightMap[i].eWeight != USHRT_MAX; i++ )
        {
            if( (nWeight >= aFontWeightMap[i].nValue) && (nWeight <= aFontWeightMap[i+1].nValue) )
            {
                sal_uInt16 nDiff1 = nWeight - aFontWeightMap[i].nValue;
                sal_uInt16 nDiff2 = aFontWeightMap[i+1].nValue - nWeight;

                if( nDiff1 < nDiff2 )
                    rValue <<= (float)( VCLUnoHelper::ConvertFontWeight( aFontWeightMap[i].eWeight ) );
                else
                    rValue <<= (float)( VCLUnoHelper::ConvertFontWeight( aFontWeightMap[i+1].eWeight ) );

                bRet = sal_True;
                break;
            }
        }
    }

    return bRet;
}

sal_Bool XMLFontWeightPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    FontWeight eWeight;

    float fValue;
    if( !( rValue >>= fValue ) )
    {
        sal_Int32 nValue;
        if( rValue >>= nValue )
        {
            fValue = (float)nValue;
            bRet = sal_True;
        }
    }
    else
        bRet = sal_True;

    eWeight = VCLUnoHelper::ConvertFontWeight( fValue );

    if( bRet )
    {
        sal_uInt16 nWeight = 0;

        for( int i = 0; aFontWeightMap[i].eWeight != -1; i++ )
        {
            if( aFontWeightMap[i].eWeight == eWeight )
            {
                 nWeight = aFontWeightMap[i].nValue;
                 break;
            }
        }

        OUStringBuffer aOut;

        if( 400 == nWeight )
            aOut.appendAscii( sXML_weight_normal );
        else if( 700 == nWeight )
            aOut.appendAscii( sXML_weight_bold );
        else
            rUnitConverter.convertNumber( aOut, (sal_Int32)nWeight );

        rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

