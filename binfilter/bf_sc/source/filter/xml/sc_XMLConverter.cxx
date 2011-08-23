/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef _MSC_VER
#pragma hdrstop
#endif


//___________________________________________________________________

#include "XMLConverter.hxx"

#include "rangeutl.hxx"
#include "docuno.hxx"
#include "convuno.hxx"
#include "document.hxx"

#include <tools/datetime.hxx>

#include <bf_xmloff/xmluconv.hxx>

namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace xmloff::token;


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
                rString += GetXMLToken( XML_WS );
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
    sal_Int32		nLength		= rString.getLength();
    sal_Int32		nIndex		= nOffset;
    sal_Bool		bQuoted		= sal_False;
    sal_Bool		bExitLoop	= sal_False;

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
    sal_Int32		nLength		= rString.getLength();
    sal_Int32		nIndex		= nOffset;
    sal_Bool		bExitLoop	= sal_False;

    while( !bExitLoop && (nIndex < nLength) )
    {
        bExitLoop = (rString[ nIndex ] != cSearchChar);
        if( !bExitLoop )
            nIndex++;
    }
    return (nIndex < nLength) ? nIndex : -1;
}


//___________________________________________________________________

void ScXMLConverter::GetTokenByOffset(
        OUString& rToken,
        const OUString& rString,
        sal_Int32& nOffset,
        sal_Unicode cQuote )
{
    sal_Int32 nLength = rString.getLength();
    if( nOffset >= nLength )
    {
        rToken = OUString();
        nOffset = -1;
    }
    else
    {
        sal_Int32 nTokenEnd = IndexOf( rString, ' ', nOffset );
        if( nTokenEnd < 0 )
            nTokenEnd = nLength;
        rToken = rString.copy( nOffset, nTokenEnd - nOffset );

        sal_Int32 nNextBegin = IndexOfDifferent( rString, ' ', nTokenEnd );
        nOffset = (nNextBegin < 0) ? nLength : nNextBegin;
    }
}

sal_Int32 ScXMLConverter::GetTokenCount( const OUString& rString )
{
    OUString	sToken;
    sal_Int32	nCount = 0;
    sal_Int32	nOffset = 0;
    while( nOffset >= 0 )
    {
        GetTokenByOffset( sToken, rString, nOffset );
        if( nOffset >= 0 )
            nCount++;
    }
    return nCount;
}

ScDocument*	ScXMLConverter::GetScDocument( uno::Reference< frame::XModel > xModel )
{
    if (xModel.is())
    {
        ScModelObj* pDocObj = ScModelObj::getImplementation( xModel );
        return pDocObj ? pDocObj->GetDocument() : NULL;
    }
    return NULL;
}


//___________________________________________________________________

sal_Bool ScXMLConverter::GetAddressFromString(
        ScAddress& rAddress,
        const OUString& rAddressStr,
        const ScDocument* pDocument,
        sal_Int32& nOffset )
{
    OUString sToken;
    GetTokenByOffset( sToken, rAddressStr, nOffset );
    if( nOffset >= 0 )
        return ((rAddress.Parse( sToken, (ScDocument*) pDocument ) & SCA_VALID) == SCA_VALID);
    return sal_False;
}

sal_Bool ScXMLConverter::GetRangeFromString(
        ScRange& rRange,
        const OUString& rRangeStr,
        const ScDocument* pDocument,
        sal_Int32& nOffset )
{
    OUString sToken;
    sal_Bool bResult(sal_False);
    GetTokenByOffset( sToken, rRangeStr, nOffset );
    if( nOffset >= 0 )
    {
        sal_Int32 nIndex = IndexOf( sToken, GetXMLToken(XML__COLON)[0], 0 );
        if( nIndex < 0 )
        {
            bResult = ((rRange.aStart.Parse( sToken, (ScDocument*) pDocument ) & SCA_VALID) == SCA_VALID);
            rRange.aEnd = rRange.aStart;
        }
        else
        {
            bResult = (((rRange.aStart.Parse( sToken.copy( 0, nIndex ), (ScDocument*) pDocument ) & SCA_VALID) == SCA_VALID) &&
                        ((rRange.aEnd.Parse( sToken.copy( nIndex + 1 ), (ScDocument*) pDocument ) & SCA_VALID) == SCA_VALID));
        }
    }
    return bResult;
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
        if( GetRangeFromString( *pRange, rRangeListStr, pDocument, nOffset ) && (nOffset >= 0) )
            rRangeList.Insert( pRange, LIST_APPEND );
    }
}


//___________________________________________________________________

sal_Bool ScXMLConverter::GetAreaFromString(
        ScArea& rArea,
        const OUString& rRangeStr,
        const ScDocument* pDocument,
        sal_Int32& nOffset )
{
    ScRange aScRange;
    sal_Bool bResult(sal_False);
    if( GetRangeFromString( aScRange, rRangeStr, pDocument, nOffset ) && (nOffset >= 0) )
    {
        rArea.nTab = aScRange.aStart.Tab();
        rArea.nColStart = aScRange.aStart.Col();
        rArea.nRowStart = aScRange.aStart.Row();
        rArea.nColEnd = aScRange.aEnd.Col();
        rArea.nRowEnd = aScRange.aEnd.Row();
        bResult = sal_True;
    }
    return bResult;
}


//___________________________________________________________________

sal_Bool ScXMLConverter::GetAddressFromString(
        table::CellAddress& rAddress,
        const OUString& rAddressStr,
        const ScDocument* pDocument,
        sal_Int32& nOffset )
{
    ScAddress aScAddress;
    sal_Bool bResult(sal_False);
    if( GetAddressFromString( aScAddress, rAddressStr, pDocument, nOffset ) && (nOffset >= 0) )
    {
        ScUnoConversion::FillApiAddress( rAddress, aScAddress );
        bResult = sal_True;
    }
    return bResult;
}

sal_Bool ScXMLConverter::GetRangeFromString(
        table::CellRangeAddress& rRange,
        const OUString& rRangeStr,
        const ScDocument* pDocument,
        sal_Int32& nOffset )
{
    ScRange aScRange;
    sal_Bool bResult(sal_False);
    if( GetRangeFromString( aScRange, rRangeStr, pDocument, nOffset ) && (nOffset >= 0) )
    {
        ScUnoConversion::FillApiRange( rRange, aScRange );
        bResult = sal_True;
    }
    return bResult;
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
        if( GetRangeFromString( aRange, rRangeListStr, pDocument, nOffset ) && (nOffset >= 0) )
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
    if (pDocument && pDocument->HasTable(rAddress.Tab()))
    {
        String sAddress;
        rAddress.Format( sAddress, nFormatFlags, (ScDocument*) pDocument );
        AssignString( rString, sAddress, bAppendStr );
    }
}

void ScXMLConverter::GetStringFromRange(
        OUString& rString,
        const ScRange& rRange,
        const ScDocument* pDocument,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    if (pDocument && pDocument->HasTable(rRange.aStart.Tab()))
    {
        ScAddress aStartAddress( rRange.aStart );
        ScAddress aEndAddress( rRange.aEnd );
        String sStartAddress;
        String sEndAddress;
        aStartAddress.Format( sStartAddress, nFormatFlags, (ScDocument*) pDocument );
        aEndAddress.Format( sEndAddress, nFormatFlags, (ScDocument*) pDocument );
        OUString sOUStartAddress( sStartAddress );
        sOUStartAddress += GetXMLToken( XML__COLON );
        sOUStartAddress += OUString( sEndAddress );
        AssignString( rString, sOUStartAddress, bAppendStr );
    }
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
    ScAddress aScAddress( static_cast<USHORT>(rAddress.Column), static_cast<USHORT>(rAddress.Row), rAddress.Sheet );
    GetStringFromAddress( rString, aScAddress, pDocument, bAppendStr, nFormatFlags );
}

void ScXMLConverter::GetStringFromRange(
        OUString& rString,
        const table::CellRangeAddress& rRange,
        const ScDocument* pDocument,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    ScRange aScRange( static_cast<USHORT>(rRange.StartColumn), static_cast<USHORT>(rRange.StartRow), rRange.Sheet,
        static_cast<USHORT>(rRange.EndColumn), static_cast<USHORT>(rRange.EndRow), rRange.Sheet );
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
    if( IsXMLToken(sFunction, XML_SUM ) )
        return sheet::GeneralFunction_SUM;
    if( IsXMLToken(sFunction, XML_AUTO ) )
        return sheet::GeneralFunction_AUTO;
    if( IsXMLToken(sFunction, XML_COUNT ) )
        return sheet::GeneralFunction_COUNT;
    if( IsXMLToken(sFunction, XML_COUNTNUMS ) )
        return sheet::GeneralFunction_COUNTNUMS;
    if( IsXMLToken(sFunction, XML_PRODUCT ) )
        return sheet::GeneralFunction_PRODUCT;
    if( IsXMLToken(sFunction, XML_AVERAGE ) )
        return sheet::GeneralFunction_AVERAGE;
    if( IsXMLToken(sFunction, XML_MAX ) )
        return sheet::GeneralFunction_MAX;
    if( IsXMLToken(sFunction, XML_MIN ) )
        return sheet::GeneralFunction_MIN;
    if( IsXMLToken(sFunction, XML_STDEV ) )
        return sheet::GeneralFunction_STDEV;
    if( IsXMLToken(sFunction, XML_STDEVP ) )
        return sheet::GeneralFunction_STDEVP;
    if( IsXMLToken(sFunction, XML_VAR ) )
        return sheet::GeneralFunction_VAR;
    if( IsXMLToken(sFunction, XML_VARP ) )
        return sheet::GeneralFunction_VARP;
    return sheet::GeneralFunction_NONE;
}

ScSubTotalFunc ScXMLConverter::GetSubTotalFuncFromString( const OUString& sFunction )
{
    if( IsXMLToken(sFunction, XML_SUM ) )
        return SUBTOTAL_FUNC_SUM;
    if( IsXMLToken(sFunction, XML_COUNT ) )
        return SUBTOTAL_FUNC_CNT;
    if( IsXMLToken(sFunction, XML_COUNTNUMS ) )
        return SUBTOTAL_FUNC_CNT2;
    if( IsXMLToken(sFunction, XML_PRODUCT ) )
        return SUBTOTAL_FUNC_PROD;
    if( IsXMLToken(sFunction, XML_AVERAGE ) )
        return SUBTOTAL_FUNC_AVE;
    if( IsXMLToken(sFunction, XML_MAX ) )
        return SUBTOTAL_FUNC_MAX;
    if( IsXMLToken(sFunction, XML_MIN ) )
        return SUBTOTAL_FUNC_MIN;
    if( IsXMLToken(sFunction, XML_STDEV ) )
        return SUBTOTAL_FUNC_STD;
    if( IsXMLToken(sFunction, XML_STDEVP ) )
        return SUBTOTAL_FUNC_STDP;
    if( IsXMLToken(sFunction, XML_VAR ) )
        return SUBTOTAL_FUNC_VAR;
    if( IsXMLToken(sFunction, XML_VARP ) )
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
        case sheet::GeneralFunction_AUTO:		sFuncStr = GetXMLToken( XML_AUTO );	  		break;
        case sheet::GeneralFunction_AVERAGE:	sFuncStr = GetXMLToken( XML_AVERAGE ); 		break;
        case sheet::GeneralFunction_COUNT:		sFuncStr = GetXMLToken( XML_COUNT );		break;
        case sheet::GeneralFunction_COUNTNUMS:	sFuncStr = GetXMLToken( XML_COUNTNUMS );	break;
        case sheet::GeneralFunction_MAX:		sFuncStr = GetXMLToken( XML_MAX );			break;
        case sheet::GeneralFunction_MIN:		sFuncStr = GetXMLToken( XML_MIN );			break;
        case sheet::GeneralFunction_NONE:		sFuncStr = GetXMLToken( XML_NONE );			break;
        case sheet::GeneralFunction_PRODUCT:	sFuncStr = GetXMLToken( XML_PRODUCT );		break;
        case sheet::GeneralFunction_STDEV:		sFuncStr = GetXMLToken( XML_STDEV );		break;
        case sheet::GeneralFunction_STDEVP:		sFuncStr = GetXMLToken( XML_STDEVP );		break;
        case sheet::GeneralFunction_SUM:		sFuncStr = GetXMLToken( XML_SUM );			break;
        case sheet::GeneralFunction_VAR:		sFuncStr = GetXMLToken( XML_VAR );			break;
        case sheet::GeneralFunction_VARP:		sFuncStr = GetXMLToken( XML_VARP );			break;
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
        case SUBTOTAL_FUNC_AVE:		sFuncStr = GetXMLToken( XML_AVERAGE ); 		break;
        case SUBTOTAL_FUNC_CNT:		sFuncStr = GetXMLToken( XML_COUNT );		break;
        case SUBTOTAL_FUNC_CNT2:	sFuncStr = GetXMLToken( XML_COUNTNUMS );	break;
        case SUBTOTAL_FUNC_MAX:		sFuncStr = GetXMLToken( XML_MAX );			break;
        case SUBTOTAL_FUNC_MIN:		sFuncStr = GetXMLToken( XML_MIN );			break;
        case SUBTOTAL_FUNC_NONE:	sFuncStr = GetXMLToken( XML_NONE );			break;
        case SUBTOTAL_FUNC_PROD:	sFuncStr = GetXMLToken( XML_PRODUCT );		break;
        case SUBTOTAL_FUNC_STD:		sFuncStr = GetXMLToken( XML_STDEV );		break;
        case SUBTOTAL_FUNC_STDP:	sFuncStr = GetXMLToken( XML_STDEVP );		break;
        case SUBTOTAL_FUNC_SUM:		sFuncStr = GetXMLToken( XML_SUM );			break;
        case SUBTOTAL_FUNC_VAR:		sFuncStr = GetXMLToken( XML_VAR );			break;
        case SUBTOTAL_FUNC_VARP:	sFuncStr = GetXMLToken( XML_VARP );			break;
    }
    AssignString( rString, sFuncStr, bAppendStr );
}


//___________________________________________________________________

sheet::DataPilotFieldOrientation ScXMLConverter::GetOrientationFromString(
    const OUString& rString )
{
    if( IsXMLToken(rString, XML_COLUMN ) )
        return sheet::DataPilotFieldOrientation_COLUMN;
    if( IsXMLToken(rString, XML_ROW ) )
        return sheet::DataPilotFieldOrientation_ROW;
    if( IsXMLToken(rString, XML_PAGE ) )
        return sheet::DataPilotFieldOrientation_PAGE;
    if( IsXMLToken(rString, XML_DATA ) )
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
            sOrientStr = GetXMLToken( XML_HIDDEN );
        break;
        case sheet::DataPilotFieldOrientation_COLUMN:
            sOrientStr = GetXMLToken( XML_COLUMN );
        break;
        case sheet::DataPilotFieldOrientation_ROW:
            sOrientStr = GetXMLToken( XML_ROW );
        break;
        case sheet::DataPilotFieldOrientation_PAGE:
            sOrientStr = GetXMLToken( XML_PAGE );
        break;
        case sheet::DataPilotFieldOrientation_DATA:
            sOrientStr = GetXMLToken( XML_DATA );
        break;
    }
    AssignString( rString, sOrientStr, bAppendStr );
}


//___________________________________________________________________

ScDetectiveObjType ScXMLConverter::GetDetObjTypeFromString( const OUString& rString )
{
    if( IsXMLToken(rString, XML_FROM_SAME_TABLE ) )
        return SC_DETOBJ_ARROW;
    if( IsXMLToken(rString, XML_FROM_ANOTHER_TABLE ) )
        return SC_DETOBJ_FROMOTHERTAB;
    if( IsXMLToken(rString, XML_TO_ANOTHER_TABLE ) )
        return SC_DETOBJ_TOOTHERTAB;
    return SC_DETOBJ_NONE;
}

sal_Bool ScXMLConverter::GetDetOpTypeFromString( ScDetOpType& rDetOpType, const OUString& rString )
{
    if( IsXMLToken(rString, XML_TRACE_DEPENDENTS ) )
        rDetOpType = SCDETOP_ADDSUCC;
    else if( IsXMLToken(rString, XML_TRACE_PRECEDENTS ) )
        rDetOpType = SCDETOP_ADDPRED;
    else if( IsXMLToken(rString, XML_TRACE_ERRORS ) )
        rDetOpType = SCDETOP_ADDERROR;
    else if( IsXMLToken(rString, XML_REMOVE_DEPENDENTS ) )
        rDetOpType = SCDETOP_DELSUCC;
    else if( IsXMLToken(rString, XML_REMOVE_PRECEDENTS ) )
        rDetOpType = SCDETOP_DELPRED;
    else
        return sal_False;
    return sal_True;
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
            sTypeStr = GetXMLToken( XML_FROM_SAME_TABLE );
        break;
        case SC_DETOBJ_FROMOTHERTAB:
            sTypeStr = GetXMLToken( XML_FROM_ANOTHER_TABLE );
        break;
        case SC_DETOBJ_TOOTHERTAB:
            sTypeStr = GetXMLToken( XML_TO_ANOTHER_TABLE );
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
            sTypeStr = GetXMLToken( XML_TRACE_DEPENDENTS );
        break;
        case SCDETOP_ADDPRED:
            sTypeStr = GetXMLToken( XML_TRACE_PRECEDENTS );
        break;
        case SCDETOP_ADDERROR:
            sTypeStr = GetXMLToken( XML_TRACE_ERRORS );
        break;
        case SCDETOP_DELSUCC:
            sTypeStr = GetXMLToken( XML_REMOVE_DEPENDENTS );
        break;
        case SCDETOP_DELPRED:
            sTypeStr = GetXMLToken( XML_REMOVE_PRECEDENTS );
        break;
    }
    AssignString( rString, sTypeStr, bAppendStr );
}


//___________________________________________________________________

void ScXMLConverter::ParseFormula(OUString& sFormula, const sal_Bool bIsFormula)
{
    OUStringBuffer sBuffer(sFormula.getLength());
    sal_Bool bInQuotationMarks(sal_False);
    sal_Bool bInDoubleQuotationMarks(sal_False);
    sal_Int16 nCountBraces(0);
    sal_Unicode chPrevious('=');
    for (sal_Int32 i = 0; i < sFormula.getLength(); i++)
    {
        if (sFormula[i] == '\'' && !bInDoubleQuotationMarks &&
            chPrevious != '\\')
            bInQuotationMarks = !bInQuotationMarks;
        else if (sFormula[i] == '"' && !bInQuotationMarks)
            bInDoubleQuotationMarks = !bInDoubleQuotationMarks;
        if (bInQuotationMarks || bInDoubleQuotationMarks)
            sBuffer.append(sFormula[i]);
        else if (sFormula[i] == '[')
            nCountBraces++;
        else if (sFormula[i] == ']')
            nCountBraces--;
        else if	((sFormula[i] != '.') ||
                ((nCountBraces == 0) && bIsFormula) ||
                !((chPrevious == '[') || (chPrevious == ':') || (chPrevious == ' ') || (chPrevious == '=')))
                sBuffer.append(sFormula[i]);
        chPrevious = sFormula[i];
    }

    DBG_ASSERT(nCountBraces == 0, "there are some braces still open");
    sFormula = sBuffer.makeStringAndClear();
}


//_____________________________________________________________________

void ScXMLConverter::ConvertDateTimeToString(const DateTime& aDateTime, ::rtl::OUStringBuffer& sDate)
{
    util::DateTime aAPIDateTime;
    ConvertCoreToAPIDateTime(aDateTime, aAPIDateTime);
    SvXMLUnitConverter::convertDateTime(sDate, aAPIDateTime);
}

void ScXMLConverter::ConvertCoreToAPIDateTime(const DateTime& aDateTime, util::DateTime& rDateTime)
{
    rDateTime.Year = aDateTime.GetYear();
    rDateTime.Month = aDateTime.GetMonth();
    rDateTime.Day = aDateTime.GetDay();
    rDateTime.Hours = aDateTime.GetHour();
    rDateTime.Minutes = aDateTime.GetMin();
    rDateTime.Seconds = aDateTime.GetSec();
    rDateTime.HundredthSeconds = aDateTime.Get100Sec();
}

void ScXMLConverter::ConvertAPIToCoreDateTime(const util::DateTime& aDateTime, DateTime& rDateTime)
{
    Date aDate(aDateTime.Day, aDateTime.Month, aDateTime.Year);
    Time aTime(aDateTime.Hours, aDateTime.Minutes, aDateTime.Seconds, aDateTime.HundredthSeconds);
    DateTime aTempDateTime (aDate, aTime);
    rDateTime = aTempDateTime;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
