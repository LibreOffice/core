/*************************************************************************
 *
 *  $RCSfile: XMLConverter.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: dr $ $Date: 2000-11-10 16:56:12 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop


//___________________________________________________________________

#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif
#ifndef SC_RANGEUTL_HXX
#include "rangeutl.hxx"
#endif
#ifndef SC_DOCUNO_HXX
#include "docuno.hxx"
#endif
#ifndef SC_CONVUNO_HXX
#include "convuno.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;


//___________________________________________________________________

void ScXMLConverter::AssignString(
        OUString& rString,
        const OUString& rNewStr,
        sal_Bool bAppendStr )
{
    if( bAppendStr )
    {
        if( rNewStr.getLength() )
        {
            if( rString.getLength() )
                rString += OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_WS ) );
            rString += rNewStr;
        }
    }
    else
        rString = rNewStr;
}

sal_Int32 ScXMLConverter::IndexOf(
        const OUString& rString,
        sal_Unicode cSearchChar,
        sal_Int32 nOffset,
        sal_Unicode cQuote )
{
    sal_Int32       nLength     = rString.getLength();
    sal_Int32       nIndex      = nOffset;
    sal_Bool        bQuoted     = sal_False;
    sal_Bool        bExitLoop   = sal_False;

    while( !bExitLoop && (nIndex < nLength) )
    {
        sal_Unicode cCode = rString[ nIndex ];
        bExitLoop = (cCode == cSearchChar) && !bQuoted;
        bQuoted = (bQuoted != (cCode == cQuote));
        if( !bExitLoop )
            nIndex++;
    }
    return (nIndex < nLength) ? nIndex : -1;
}

sal_Int32 ScXMLConverter::IndexOfDifferent(
        const OUString& rString,
        sal_Unicode cSearchChar,
        sal_Int32 nOffset )
{
    sal_Int32       nLength     = rString.getLength();
    sal_Int32       nIndex      = nOffset;
    sal_Bool        bExitLoop   = sal_False;

    while( !bExitLoop && (nIndex < nLength) )
    {
        bExitLoop = (rString[ nIndex ] != cSearchChar);
        if( !bExitLoop )
            nIndex++;
    }
    return (nIndex < nLength) ? nIndex : -1;
}


//___________________________________________________________________

sal_Int32 ScXMLConverter::GetTokenByOffset(
        OUString& rToken,
        const OUString& rString,
        sal_Int32 nOffset,
        sal_Unicode cQuote )
{
    sal_Int32 nLength = rString.getLength();
    if( nOffset >= nLength )
    {
        rToken = OUString();
        return -1;
    }

    sal_Int32 nTokenEnd = IndexOf( rString, ' ', nOffset );
    if( nTokenEnd < 0 )
        nTokenEnd = nLength;
    rToken = rString.copy( nOffset, nTokenEnd - nOffset );

    sal_Int32 nNextBegin = IndexOfDifferent( rString, ' ', nTokenEnd );
    return (nNextBegin < 0) ? nLength : nNextBegin;
}

sal_Int32 ScXMLConverter::GetTokenCount( const OUString& rString )
{
    OUString    sToken;
    sal_Int32   nCount = 0;
    sal_Int32   nOffset = 0;
    while( nOffset >= 0 )
    {
        nOffset = GetTokenByOffset( sToken, rString, nOffset );
        if( nOffset >= 0 )
            nCount++;
    }
    return nCount;
}

void ScXMLConverter::AppendString( OUString& rString, const OUString& rNewStr )
{
    AssignString( rString, rNewStr, sal_True );
}

ScDocument* ScXMLConverter::GetScDocument( uno::Reference< frame::XModel > xModel )
{
    ScModelObj* pDocObj = ScModelObj::getImplementation( xModel );
    return pDocObj ? pDocObj->GetDocument() : NULL;
}


//___________________________________________________________________

sal_Int32 ScXMLConverter::GetAddressFromString(
        ScAddress& rAddress,
        const OUString& rAddressStr,
        const ScDocument* pDocument,
        sal_Int32 nOffset )
{
    OUString sToken;
    nOffset = GetTokenByOffset( sToken, rAddressStr, nOffset );
    if( nOffset >= 0 )
        rAddress.Parse( sToken, (ScDocument*) pDocument );
    return nOffset;
}

sal_Int32 ScXMLConverter::GetRangeFromString(
        ScRange& rRange,
        const OUString& rRangeStr,
        const ScDocument* pDocument,
        sal_Int32 nOffset )
{
    OUString sToken;
    nOffset = GetTokenByOffset( sToken, rRangeStr, nOffset );
    if( nOffset >= 0 )
    {
        sal_Int32 nIndex = IndexOf( sToken, sXML__colon[0], 0 );
        if( nIndex < 0 )
        {
            rRange.aStart.Parse( sToken, (ScDocument*) pDocument );
            rRange.aEnd = rRange.aStart;
        }
        else
        {
            rRange.aStart.Parse( sToken.copy( 0, nIndex ), (ScDocument*) pDocument );
            rRange.aEnd.Parse( sToken.copy( nIndex + 1 ), (ScDocument*) pDocument );
        }
    }
    return nOffset;
}

void ScXMLConverter::GetRangeListFromString(
        ScRangeList& rRangeList,
        const OUString& rRangeListStr,
        const ScDocument* pDocument )
{
    DBG_ASSERT( rRangeListStr.getLength(), "ScXMLConverter::GetRangeListFromString - empty string!" );
    sal_Int32 nOffset = 0;
    while( nOffset >= 0 )
    {
        ScRange* pRange = new ScRange;
        nOffset = GetRangeFromString( *pRange, rRangeListStr, pDocument, nOffset );
        if( nOffset >= 0 )
            rRangeList.Insert( pRange, LIST_APPEND );
    }
}


//___________________________________________________________________

sal_Int32 ScXMLConverter::GetAreaFromString(
        ScArea& rArea,
        const OUString& rRangeStr,
        const ScDocument* pDocument,
        sal_Int32 nOffset )
{
    ScRange aScRange;
    nOffset = GetRangeFromString( aScRange, rRangeStr, pDocument, nOffset );
    if( nOffset >= 0 )
    {
        rArea.nTab = aScRange.aStart.Tab();
        rArea.nColStart = aScRange.aStart.Col();
        rArea.nRowStart = aScRange.aStart.Row();
        rArea.nColEnd = aScRange.aEnd.Col();
        rArea.nRowEnd = aScRange.aEnd.Row();
    }
    return nOffset;
}


//___________________________________________________________________

sal_Int32 ScXMLConverter::GetAddressFromString(
        table::CellAddress& rAddress,
        const OUString& rAddressStr,
        const ScDocument* pDocument,
        sal_Int32 nOffset )
{
    ScAddress aScAddress;
    nOffset = GetAddressFromString( aScAddress, rAddressStr, pDocument, nOffset );
    if( nOffset >= 0 )
        ScUnoConversion::FillApiAddress( rAddress, aScAddress );
    return nOffset;
}

sal_Int32 ScXMLConverter::GetRangeFromString(
        table::CellRangeAddress& rRange,
        const OUString& rRangeStr,
        const ScDocument* pDocument,
        sal_Int32 nOffset )
{
    ScRange aScRange;
    nOffset = GetRangeFromString( aScRange, rRangeStr, pDocument, nOffset );
    if( nOffset >= 0 )
        ScUnoConversion::FillApiRange( rRange, aScRange );
    return nOffset;
}

void ScXMLConverter::GetRangeListFromString(
        uno::Sequence< table::CellRangeAddress >& rRangeSeq,
        const OUString& rRangeListStr,
        const ScDocument* pDocument )
{
    DBG_ASSERT( rRangeListStr.getLength(), "ScXMLConverter::GetRangeListFromString - empty string!" );
    table::CellRangeAddress aRange;
    sal_Int32 nOffset = 0;
    while( nOffset >= 0 )
    {
        nOffset = GetRangeFromString( aRange, rRangeListStr, pDocument, nOffset );
        if( nOffset >= 0 )
        {
            rRangeSeq.realloc( rRangeSeq.getLength() + 1 );
            rRangeSeq[ rRangeSeq.getLength() - 1 ] = aRange;
        }
    }
}


//___________________________________________________________________

void ScXMLConverter::GetStringFromAddress(
        OUString& rString,
        const ScAddress& rAddress,
        const ScDocument* pDocument,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    String sAddress;
    rAddress.Format( sAddress, nFormatFlags, (ScDocument*) pDocument );
    AssignString( rString, sAddress, bAppendStr );
}

void ScXMLConverter::GetStringFromRange(
        OUString& rString,
        const ScRange& rRange,
        const ScDocument* pDocument,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    ScAddress aStartAddress( rRange.aStart );
    ScAddress aEndAddress( rRange.aEnd );
    String sStartAddress;
    String sEndAddress;
    aStartAddress.Format( sStartAddress, nFormatFlags, (ScDocument*) pDocument );
    aEndAddress.Format( sEndAddress, nFormatFlags, (ScDocument*) pDocument );
    OUString sOUStartAddress( sStartAddress );
    sOUStartAddress += OUString( RTL_CONSTASCII_USTRINGPARAM( sXML__colon ) );
    sOUStartAddress += OUString( sEndAddress );
    AssignString( rString, sOUStartAddress, bAppendStr );
}

void ScXMLConverter::GetStringFromRangeList(
        OUString& rString,
        const ScRangeList* pRangeList,
        const ScDocument* pDocument,
        sal_uInt16 nFormatFlags )
{
    OUString sRangeListStr;
    if( pRangeList )
    {
        sal_Int32 nCount = pRangeList->Count();
        for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
        {
            const ScRange* pRange = pRangeList->GetObject( nIndex );
            if( pRange )
                GetStringFromRange( sRangeListStr, *pRange, pDocument, sal_True, nFormatFlags );
        }
    }
    rString = sRangeListStr;
}


//___________________________________________________________________

void ScXMLConverter::GetStringFromArea(
        OUString& rString,
        const ScArea& rArea,
        const ScDocument* pDocument,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    ScRange aRange( rArea.nColStart, rArea.nRowStart, rArea.nTab, rArea.nColEnd, rArea.nRowEnd, rArea.nTab );
    GetStringFromRange( rString, aRange, pDocument, bAppendStr, nFormatFlags );
}


//___________________________________________________________________

void ScXMLConverter::GetStringFromAddress(
        OUString& rString,
        const table::CellAddress& rAddress,
        const ScDocument* pDocument,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    ScAddress aScAddress( rAddress.Column, rAddress.Row, rAddress.Sheet );
    GetStringFromAddress( rString, aScAddress, pDocument, bAppendStr, nFormatFlags );
}

void ScXMLConverter::GetStringFromRange(
        OUString& rString,
        const table::CellRangeAddress& rRange,
        const ScDocument* pDocument,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    ScRange aScRange( rRange.StartColumn, rRange.StartRow, rRange.Sheet,
        rRange.EndColumn, rRange.EndRow, rRange.Sheet );
    GetStringFromRange( rString, aScRange, pDocument, bAppendStr, nFormatFlags );
}

void ScXMLConverter::GetStringFromRangeList(
        OUString& rString,
        const uno::Sequence< table::CellRangeAddress >& rRangeSeq,
        const ScDocument* pDocument,
        sal_uInt16 nFormatFlags )
{
    OUString sRangeListStr;
    sal_Int32 nCount = rRangeSeq.getLength();
    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const table::CellRangeAddress& rRange = rRangeSeq[ nIndex ];
        GetStringFromRange( sRangeListStr, rRange, pDocument, sal_True, nFormatFlags );
    }
    rString = sRangeListStr;
}


//___________________________________________________________________

sheet::GeneralFunction ScXMLConverter::GetFunctionFromString( const OUString& sFunction )
{
    if( sFunction.compareToAscii( sXML_sum ) == 0 )
        return sheet::GeneralFunction_SUM;
    if( sFunction.compareToAscii( sXML_auto ) == 0 )
        return sheet::GeneralFunction_AUTO;
    if( sFunction.compareToAscii( sXML_count ) == 0 )
        return sheet::GeneralFunction_COUNT;
    if( sFunction.compareToAscii( sXML_countnums ) == 0 )
        return sheet::GeneralFunction_COUNTNUMS;
    if( sFunction.compareToAscii( sXML_product ) == 0 )
        return sheet::GeneralFunction_PRODUCT;
    if( sFunction.compareToAscii( sXML_average ) == 0 )
        return sheet::GeneralFunction_AVERAGE;
    if( sFunction.compareToAscii( sXML_max ) == 0 )
        return sheet::GeneralFunction_MAX;
    if( sFunction.compareToAscii( sXML_min ) == 0 )
        return sheet::GeneralFunction_MIN;
    if( sFunction.compareToAscii( sXML_stdev ) == 0 )
        return sheet::GeneralFunction_STDEV;
    if( sFunction.compareToAscii( sXML_stdevp ) == 0 )
        return sheet::GeneralFunction_STDEVP;
    if( sFunction.compareToAscii( sXML_var ) == 0 )
        return sheet::GeneralFunction_VAR;
    if( sFunction.compareToAscii( sXML_varp ) == 0 )
        return sheet::GeneralFunction_VARP;
    return sheet::GeneralFunction_NONE;
}

ScSubTotalFunc ScXMLConverter::GetSubTotalFuncFromString( const OUString& sFunction )
{
    if( sFunction.compareToAscii( sXML_sum ) == 0 )
        return SUBTOTAL_FUNC_SUM;
    if( sFunction.compareToAscii( sXML_count ) == 0 )
        return SUBTOTAL_FUNC_CNT;
    if( sFunction.compareToAscii( sXML_countnums ) == 0 )
        return SUBTOTAL_FUNC_CNT2;
    if( sFunction.compareToAscii( sXML_product ) == 0 )
        return SUBTOTAL_FUNC_PROD;
    if( sFunction.compareToAscii( sXML_average ) == 0 )
        return SUBTOTAL_FUNC_AVE;
    if( sFunction.compareToAscii( sXML_max ) == 0 )
        return SUBTOTAL_FUNC_MAX;
    if( sFunction.compareToAscii( sXML_min ) == 0 )
        return SUBTOTAL_FUNC_MIN;
    if( sFunction.compareToAscii( sXML_stdev ) == 0 )
        return SUBTOTAL_FUNC_STD;
    if( sFunction.compareToAscii( sXML_stdevp ) == 0 )
        return SUBTOTAL_FUNC_STDP;
    if( sFunction.compareToAscii( sXML_var ) == 0 )
        return SUBTOTAL_FUNC_VAR;
    if( sFunction.compareToAscii( sXML_varp ) == 0 )
        return SUBTOTAL_FUNC_VARP;
    return SUBTOTAL_FUNC_NONE;
}


//___________________________________________________________________

void ScXMLConverter::GetStringFromFunction(
        OUString& rString,
        const sheet::GeneralFunction eFunction,
        sal_Bool bAppendStr )
{
    OUString sFuncStr;
    switch( eFunction )
    {
        case sheet::GeneralFunction_AUTO:       sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_auto ) );        break;
        case sheet::GeneralFunction_AVERAGE:    sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_average ) );     break;
        case sheet::GeneralFunction_COUNT:      sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_count ) );       break;
        case sheet::GeneralFunction_COUNTNUMS:  sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_countnums ) );   break;
        case sheet::GeneralFunction_MAX:        sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_max ) );         break;
        case sheet::GeneralFunction_MIN:        sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_min ) );         break;
        case sheet::GeneralFunction_NONE:       sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_none ) );        break;
        case sheet::GeneralFunction_PRODUCT:    sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_product ) );     break;
        case sheet::GeneralFunction_STDEV:      sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_stdev ) );       break;
        case sheet::GeneralFunction_STDEVP:     sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_stdevp ) );      break;
        case sheet::GeneralFunction_SUM:        sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_sum ) );         break;
        case sheet::GeneralFunction_VAR:        sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_var ) );         break;
        case sheet::GeneralFunction_VARP:       sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_varp ) );        break;
    }
    AssignString( rString, sFuncStr, bAppendStr );
}

void ScXMLConverter::GetStringFromFunction(
        OUString& rString,
        const ScSubTotalFunc eFunction,
        sal_Bool bAppendStr )
{
    OUString sFuncStr;
    switch( eFunction )
    {
        case SUBTOTAL_FUNC_AVE:     sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_average ) );     break;
        case SUBTOTAL_FUNC_CNT:     sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_count ) );       break;
        case SUBTOTAL_FUNC_CNT2:    sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_countnums ) );   break;
        case SUBTOTAL_FUNC_MAX:     sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_max ) );         break;
        case SUBTOTAL_FUNC_MIN:     sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_min ) );         break;
        case SUBTOTAL_FUNC_NONE:    sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_none ) );        break;
        case SUBTOTAL_FUNC_PROD:    sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_product ) );     break;
        case SUBTOTAL_FUNC_STD:     sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_stdev ) );       break;
        case SUBTOTAL_FUNC_STDP:    sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_stdevp ) );      break;
        case SUBTOTAL_FUNC_SUM:     sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_sum ) );         break;
        case SUBTOTAL_FUNC_VAR:     sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_var ) );         break;
        case SUBTOTAL_FUNC_VARP:    sFuncStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_varp ) );        break;
    }
    AssignString( rString, sFuncStr, bAppendStr );
}


//___________________________________________________________________

sheet::DataPilotFieldOrientation ScXMLConverter::GetOrientationFromString(
    const OUString& rString )
{
    if( rString.compareToAscii( sXML_column ) == 0 )
        return sheet::DataPilotFieldOrientation_COLUMN;
    if( rString.compareToAscii( sXML_row ) == 0 )
        return sheet::DataPilotFieldOrientation_ROW;
    if( rString.compareToAscii( sXML_page ) == 0 )
        return sheet::DataPilotFieldOrientation_PAGE;
    if( rString.compareToAscii( sXML_data ) == 0 )
        return sheet::DataPilotFieldOrientation_DATA;
    return sheet::DataPilotFieldOrientation_HIDDEN;
}


//___________________________________________________________________

void ScXMLConverter::GetStringFromOrientation(
    OUString& rString,
    const sheet::DataPilotFieldOrientation eOrientation,
    sal_Bool bAppendStr )
{
    OUString sOrientStr;
    switch( eOrientation )
    {
        case sheet::DataPilotFieldOrientation_HIDDEN:
            sOrientStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_hidden ) );
        break;
        case sheet::DataPilotFieldOrientation_COLUMN:
            sOrientStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_column ) );
        break;
        case sheet::DataPilotFieldOrientation_ROW:
            sOrientStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_row ) );
        break;
        case sheet::DataPilotFieldOrientation_PAGE:
            sOrientStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_page ) );
        break;
        case sheet::DataPilotFieldOrientation_DATA:
            sOrientStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_data ) );
        break;
    }
    AssignString( rString, sOrientStr, bAppendStr );
}


//___________________________________________________________________

ScDetectiveObjType ScXMLConverter::GetDetObjTypeFromString( const OUString& rString )
{
    if( rString.compareToAscii( sXML_from_same_table ) == 0 )
        return SC_DETOBJ_ARROW;
    if( rString.compareToAscii( sXML_from_another_table ) == 0 )
        return SC_DETOBJ_FROMOTHERTAB;
    if( rString.compareToAscii( sXML_to_another_table ) == 0 )
        return SC_DETOBJ_TOOTHERTAB;
    return SC_DETOBJ_NONE;
}

ScDetOpType ScXMLConverter::GetDetOpTypeFromString( const OUString& rString )
{
    if( rString.compareToAscii( sXML_trace_dependents ) == 0 )
        return SCDETOP_ADDSUCC;
    if( rString.compareToAscii( sXML_trace_precedents ) == 0 )
        return SCDETOP_ADDPRED;
    if( rString.compareToAscii( sXML_remove_dependents ) == 0 )
        return SCDETOP_DELSUCC;
    if( rString.compareToAscii( sXML_remove_precedents ) == 0 )
        return SCDETOP_DELPRED;
    return SCDETOP_ADDERROR;
}


//___________________________________________________________________

void ScXMLConverter::GetStringFromDetObjType(
        OUString& rString,
        const ScDetectiveObjType eObjType,
        sal_Bool bAppendStr )
{
    OUString sTypeStr;
    switch( eObjType )
    {
        case SC_DETOBJ_ARROW:
            sTypeStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_from_same_table ) );
        break;
        case SC_DETOBJ_FROMOTHERTAB:
            sTypeStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_from_another_table ) );
        break;
        case SC_DETOBJ_TOOTHERTAB:
            sTypeStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_to_another_table ) );
        break;
    }
    AssignString( rString, sTypeStr, bAppendStr );
}

void ScXMLConverter::GetStringFromDetOpType(
        OUString& rString,
        const ScDetOpType eOpType,
        sal_Bool bAppendStr )
{
    OUString sTypeStr;
    switch( eOpType )
    {
        case SCDETOP_ADDSUCC:
            sTypeStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_trace_dependents ) );
        break;
        case SCDETOP_ADDPRED:
            sTypeStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_trace_precedents ) );
        break;
        case SCDETOP_ADDERROR:
            sTypeStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_trace_errors ) );
        break;
        case SCDETOP_DELSUCC:
            sTypeStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_remove_dependents ) );
        break;
        case SCDETOP_DELPRED:
            sTypeStr = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_remove_precedents ) );
        break;
    }
    AssignString( rString, sTypeStr, bAppendStr );
}


//___________________________________________________________________

void ScXMLConverter::ParseFormula(OUString& sFormula, const sal_Bool bIsFormula)
{
    OUStringBuffer sBuffer(sFormula.getLength());
    sal_Int16 nCountQuotationMarks = 0;
    sal_Int16 nCountBraces = 0;
    sal_Unicode chPrevious = '=';
    for (sal_Int32 i = 0; i < sFormula.getLength(); i++)
    {
        if (sFormula[i] == '"')
            if (nCountQuotationMarks == 0)
                nCountQuotationMarks++;
            else
                nCountQuotationMarks--;
        if (sFormula[i] != '[' && sFormula[i] != ']')
        {
            if (sFormula[i] != '.' || (nCountBraces == 0 && bIsFormula) ||
                !(chPrevious == '[' || chPrevious == ':' || chPrevious == ' ' || chPrevious == '='))
            {
                sBuffer.append(sFormula[i]);
            }
        }
        else
            if (nCountQuotationMarks == 0)
                if (sFormula[i] == '[')
                    nCountBraces++;
                else
                    nCountBraces--;
            else
                sBuffer.append(sFormula[i]);
        chPrevious = sFormula[i];
    }
    sFormula = sBuffer.makeStringAndClear();
}
