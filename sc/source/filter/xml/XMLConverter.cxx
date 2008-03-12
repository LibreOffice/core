/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLConverter.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 13:15:15 $
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
#include "precompiled_sc.hxx"




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
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace ::com::sun::star;
using namespace xmloff::token;


//___________________________________________________________________

ScDocument* ScXMLConverter::GetScDocument( uno::Reference< frame::XModel > xModel )
{
    if (xModel.is())
    {
        ScModelObj* pDocObj = ScModelObj::getImplementation( xModel );
        return pDocObj ? pDocObj->GetDocument() : NULL;
    }
    return NULL;
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
        case sheet::GeneralFunction_AUTO:       sFuncStr = GetXMLToken( XML_AUTO );         break;
        case sheet::GeneralFunction_AVERAGE:    sFuncStr = GetXMLToken( XML_AVERAGE );      break;
        case sheet::GeneralFunction_COUNT:      sFuncStr = GetXMLToken( XML_COUNT );        break;
        case sheet::GeneralFunction_COUNTNUMS:  sFuncStr = GetXMLToken( XML_COUNTNUMS );    break;
        case sheet::GeneralFunction_MAX:        sFuncStr = GetXMLToken( XML_MAX );          break;
        case sheet::GeneralFunction_MIN:        sFuncStr = GetXMLToken( XML_MIN );          break;
        case sheet::GeneralFunction_NONE:       sFuncStr = GetXMLToken( XML_NONE );         break;
        case sheet::GeneralFunction_PRODUCT:    sFuncStr = GetXMLToken( XML_PRODUCT );      break;
        case sheet::GeneralFunction_STDEV:      sFuncStr = GetXMLToken( XML_STDEV );        break;
        case sheet::GeneralFunction_STDEVP:     sFuncStr = GetXMLToken( XML_STDEVP );       break;
        case sheet::GeneralFunction_SUM:        sFuncStr = GetXMLToken( XML_SUM );          break;
        case sheet::GeneralFunction_VAR:        sFuncStr = GetXMLToken( XML_VAR );          break;
        case sheet::GeneralFunction_VARP:       sFuncStr = GetXMLToken( XML_VARP );         break;
        default:
        {
            // added to avoid warnings
        }
    }
    ScRangeStringConverter::AssignString( rString, sFuncStr, bAppendStr );
}

void ScXMLConverter::GetStringFromFunction(
        OUString& rString,
        const ScSubTotalFunc eFunction,
        sal_Bool bAppendStr )
{
    OUString sFuncStr;
    switch( eFunction )
    {
        case SUBTOTAL_FUNC_AVE:     sFuncStr = GetXMLToken( XML_AVERAGE );      break;
        case SUBTOTAL_FUNC_CNT:     sFuncStr = GetXMLToken( XML_COUNT );        break;
        case SUBTOTAL_FUNC_CNT2:    sFuncStr = GetXMLToken( XML_COUNTNUMS );    break;
        case SUBTOTAL_FUNC_MAX:     sFuncStr = GetXMLToken( XML_MAX );          break;
        case SUBTOTAL_FUNC_MIN:     sFuncStr = GetXMLToken( XML_MIN );          break;
        case SUBTOTAL_FUNC_NONE:    sFuncStr = GetXMLToken( XML_NONE );         break;
        case SUBTOTAL_FUNC_PROD:    sFuncStr = GetXMLToken( XML_PRODUCT );      break;
        case SUBTOTAL_FUNC_STD:     sFuncStr = GetXMLToken( XML_STDEV );        break;
        case SUBTOTAL_FUNC_STDP:    sFuncStr = GetXMLToken( XML_STDEVP );       break;
        case SUBTOTAL_FUNC_SUM:     sFuncStr = GetXMLToken( XML_SUM );          break;
        case SUBTOTAL_FUNC_VAR:     sFuncStr = GetXMLToken( XML_VAR );          break;
        case SUBTOTAL_FUNC_VARP:    sFuncStr = GetXMLToken( XML_VARP );         break;
    }
    ScRangeStringConverter::AssignString( rString, sFuncStr, bAppendStr );
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
        default:
        {
            // added to avoid warnings
        }
    }
    ScRangeStringConverter::AssignString( rString, sOrientStr, bAppendStr );
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
        default:
        {
            // added to avoid warnings
        }
    }
    ScRangeStringConverter::AssignString( rString, sTypeStr, bAppendStr );
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
    ScRangeStringConverter::AssignString( rString, sTypeStr, bAppendStr );
}


//___________________________________________________________________

void ScXMLConverter::ParseFormula(OUString& sFormula, const sal_Bool bIsFormula)
{
    OUStringBuffer sBuffer(sFormula.getLength());
    sal_Bool bInQuotationMarks(sal_False);
    sal_Bool bInDoubleQuotationMarks(sal_False);
    sal_Int16 nCountBraces(0);
    sal_Unicode chPrevious('=');
    for (sal_Int32 i = 0; i < sFormula.getLength(); ++i)
    {
        if (sFormula[i] == '\'' && !bInDoubleQuotationMarks &&
            chPrevious != '\\')
            bInQuotationMarks = !bInQuotationMarks;
        else if (sFormula[i] == '"' && !bInQuotationMarks)
            bInDoubleQuotationMarks = !bInDoubleQuotationMarks;
        if (bInQuotationMarks || bInDoubleQuotationMarks)
            sBuffer.append(sFormula[i]);
        else if (sFormula[i] == '[')
            ++nCountBraces;
        else if (sFormula[i] == ']')
            nCountBraces--;
        else if ((sFormula[i] != '.') ||
                ((nCountBraces == 0) && bIsFormula) ||
                !((chPrevious == '[') || (chPrevious == ':') || (chPrevious == ' ') || (chPrevious == '=')))
                sBuffer.append(sFormula[i]);
        chPrevious = sFormula[i];
    }

    DBG_ASSERT(nCountBraces == 0, "there are some braces still open");
    sFormula = sBuffer.makeStringAndClear();
}


//_____________________________________________________________________

void ScXMLConverter::ConvertDateTimeToString(const DateTime& aDateTime, rtl::OUStringBuffer& sDate)
{
    util::DateTime aAPIDateTime;
    ConvertCoreToAPIDateTime(aDateTime, aAPIDateTime);
    SvXMLUnitConverter::convertDateTime(sDate, aAPIDateTime);
}

void ScXMLConverter::ConvertStringToDateTime(const rtl::OUString& sDate, DateTime& aDateTime, SvXMLUnitConverter* /* pUnitConverter */)
{
    com::sun::star::util::DateTime aAPIDateTime;
    SvXMLUnitConverter::convertDateTime(aAPIDateTime, sDate);
    ConvertAPIToCoreDateTime(aAPIDateTime, aDateTime);
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

