/*************************************************************************
 *
 *  $RCSfile: XMLExportDataPilot.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sab $ $Date: 2000-11-10 17:17:59 $
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

// INCLUDE ---------------------------------------------------------------

#ifndef _SC_XMLEXPORTDATAPILOT_HXX
#include "XMLExportDataPilot.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef SC_XMLEXPRT_HXX
#include "xmlexprt.hxx"
#endif
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif
#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_DPOBJECT_HXX
#include "dpobject.hxx"
#endif
#ifndef SC_DOCITER_HXX
#include "dociter.hxx"
#endif
#ifndef SC_SCATTR_HXX
#include "attrib.hxx"
#endif
#ifndef SC_SCPATATR_HXX
#include "patattr.hxx"
#endif
#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef SC_DPSAVE_HXX
#include "dpsave.hxx"
#endif
#ifndef SC_DPSHTTAB_HXX
#include "dpshttab.hxx"
#endif
#ifndef SC_DPSDBTAB_HXX
#include "dpsdbtab.hxx"
#endif

#ifndef _COM_SUN_STAR_SHEET_DATAIMPORTMODE_HPP_
#include <com/sun/star/sheet/DataImportMode.hpp>
#endif

using namespace com::sun::star;

ScXMLExportDataPilot::ScXMLExportDataPilot(ScXMLExport& rTempExport)
    : rExport(rTempExport),
    pDoc( NULL )
{
}

ScXMLExportDataPilot::~ScXMLExportDataPilot()
{
}

rtl::OUString ScXMLExportDataPilot::getDPOperatorXML(const ScQueryOp aFilterOperator, const sal_Bool bUseRegularExpressions,
    const sal_Bool bIsString, const double dVal) const
{
    if (bUseRegularExpressions)
    {
        switch (aFilterOperator)
        {
            case SC_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_match));
                break;
            case SC_NOT_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_nomatch));
                break;
        }
    }
    else
    {
        switch (aFilterOperator)
        {
            case SC_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
                break;
            case SC_NOT_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("!="));
                break;
            case SC_BOTPERC :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_bottom_percent));
                break;
            case SC_BOTVAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_bottom_values));
                break;
            case SC_GREATER :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">"));
                break;
            case SC_GREATER_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">="));
                break;
            case SC_LESS :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<"));
                break;
            case SC_LESS_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<="));
                break;
            case SC_TOPPERC :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_top_percent));
                break;
            case SC_TOPVAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_top_values));
                break;
            default:
            {
                if (bIsString)
                {
                    if (dVal == SC_EMPTYFIELDS)
                        return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_empty));
                    else if (dVal == SC_NONEMPTYFIELDS)
                        return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_noempty));
                }
            }
        }
    }
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
}

void ScXMLExportDataPilot::WriteDPCondition(const ScQueryEntry& aQueryEntry, sal_Bool bIsCaseSensitive, sal_Bool bUseRegularExpressions)
{
    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_field_number, rtl::OUString::valueOf(aQueryEntry.nField));
    if (bIsCaseSensitive)
        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_case_sensitive, sXML_true);
    if (aQueryEntry.bQueryByString)
    {
        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_data_type, sXML_number);
        rtl::OUStringBuffer sBuffer;
        rExport.GetMM100UnitConverter().convertNumber(sBuffer, aQueryEntry.nVal);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_value, sBuffer.makeStringAndClear());
    }
    else
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_value, rtl::OUString(*aQueryEntry.pStr));
    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_operator, getDPOperatorXML(aQueryEntry.eOp, bUseRegularExpressions,
        aQueryEntry.bQueryByString, aQueryEntry.nVal));
    SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, sXML_filter_condition, sal_True, sal_True);
}

void ScXMLExportDataPilot::WriteDPFilter(const ScQueryParam& aQueryParam)
{
    sal_Int16 nQueryEntryCount = aQueryParam.GetEntryCount();
    if (nQueryEntryCount > 0)
    {
        sal_Bool bAnd(sal_False);
        sal_Bool bOr(sal_False);
        sal_Bool bHasEntries(sal_True);
        sal_Int16 nEntries(0);
        for (sal_Int32 j = 1; (j < nQueryEntryCount) && bHasEntries; j++)
        {
            ScQueryEntry aEntry = aQueryParam.GetEntry(j);
            if (aEntry.bDoQuery)
            {
                nEntries++;
                if (aEntry.eConnect == SC_AND)
                    bAnd = sal_True;
                else
                    bOr = sal_True;
            }
            else
                bHasEntries = sal_False;
        }
        nQueryEntryCount = nEntries;
        if (nQueryEntryCount)
        {
            if (!aQueryParam.bInplace)
            {
                ScAddress aTargetAddress(aQueryParam.nDestCol, aQueryParam.nDestRow, aQueryParam.nDestTab);
                rtl::OUString sAddress;
                ScXMLConverter::GetStringFromAddress( sAddress, aTargetAddress, pDoc );
                rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_target_range_address, sAddress);
            }
            if(!((aQueryParam.nCol1 == aQueryParam.nCol2) && (aQueryParam.nRow1 == aQueryParam.nRow2) && (aQueryParam.nCol1 == aQueryParam.nRow1)
                && (aQueryParam.nCol1 == 0) && (aQueryParam.nTab == USHRT_MAX)))
            {
                ScRange aConditionRange(aQueryParam.nCol1, aQueryParam.nRow1, aQueryParam.nTab,
                    aQueryParam.nCol2, aQueryParam.nRow2, aQueryParam.nTab);
                rtl::OUString sConditionRange;
                ScXMLConverter::GetStringFromRange( sConditionRange, aConditionRange, pDoc );
                rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_condition_source_range_address, sConditionRange);
            }
            if (!aQueryParam.bDuplicate)
                rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_display_duplicates, sXML_false);
            SvXMLElementExport aElemDPF(rExport, XML_NAMESPACE_TABLE, sXML_filter, sal_True, sal_True);
            rExport.CheckAttrList();
            if (bOr && !bAnd)
            {
                SvXMLElementExport aElemOr(rExport, XML_NAMESPACE_TABLE, sXML_filter_or, sal_True, sal_True);
                for (j = 0; j < nQueryEntryCount; j++)
                {
                    WriteDPCondition(aQueryParam.GetEntry(j), aQueryParam.bCaseSens, aQueryParam.bRegExp);
                }
            }
            else if (bAnd && !bOr)
            {
                SvXMLElementExport aElemAnd(rExport, XML_NAMESPACE_TABLE, sXML_filter_and, sal_True, sal_True);
                for (j = 0; j < nQueryEntryCount; j++)
                {
                    WriteDPCondition(aQueryParam.GetEntry(j), aQueryParam.bCaseSens, aQueryParam.bRegExp);
                }
            }
            else if (nQueryEntryCount  == 1)
            {
                    WriteDPCondition(aQueryParam.GetEntry(0), aQueryParam.bCaseSens, aQueryParam.bRegExp);
            }
            else
            {
                SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, sXML_filter_or, sal_True, sal_True);
                ScQueryEntry aPrevFilterField = aQueryParam.GetEntry(0);
                ScQueryConnect aConnection = aQueryParam.GetEntry(1).eConnect;
                sal_Bool bOpenAndElement;
                rtl::OUString aName = rExport.GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_filter_and)));
                if (aConnection == SC_AND)
                {
                    rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
                    rExport.GetDocHandler()->startElement( aName, rExport.GetXAttrList());
                    rExport.ClearAttrList();
                    bOpenAndElement = sal_True;
                }
                else
                    bOpenAndElement = sal_False;
                for (j = 1; j < nQueryEntryCount; j++)
                {
                    if (aConnection != aQueryParam.GetEntry(j).eConnect)
                    {
                        aConnection = aQueryParam.GetEntry(j).eConnect;
                        if (aQueryParam.GetEntry(j).eConnect == SC_AND)
                        {
                            rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
                            rExport.GetDocHandler()->startElement( aName, rExport.GetXAttrList());
                            rExport.ClearAttrList();
                            bOpenAndElement = sal_True;
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                            aPrevFilterField = aQueryParam.GetEntry(j);
                            if (j == nQueryEntryCount - 1)
                            {
                                WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                                rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
                                rExport.GetDocHandler()->endElement(aName);
                                bOpenAndElement = sal_False;
                            }
                        }
                        else
                        {
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                            aPrevFilterField = aQueryParam.GetEntry(j);
                            if (bOpenAndElement)
                            {
                                rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
                                rExport.GetDocHandler()->endElement(aName);
                                bOpenAndElement = sal_False;
                            }
                            if (j == nQueryEntryCount - 1)
                            {
                                WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                            }
                        }
                    }
                    else
                    {
                        WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                        aPrevFilterField = aQueryParam.GetEntry(j);
                        if (j == nQueryEntryCount - 1)
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                    }
                }
            }
        }
    }
}

void ScXMLExportDataPilot::WriteDataPilots(const uno::Reference <sheet::XSpreadsheetDocument>& xSpreadDoc)
{
    pDoc = rExport.GetDocument();
    if (pDoc)
    {
        ScDPCollection* pDPs = pDoc->GetDPCollection();
        if (pDPs)
        {
            sal_Int16 nDPCount = pDPs->GetCount();
            if (nDPCount > 0)
            {
                SvXMLElementExport aElemDPs(rExport, XML_NAMESPACE_TABLE, sXML_data_pilot_tables, sal_True, sal_True);
                rExport.CheckAttrList();
                for (sal_Int16 i = 0; i < nDPCount; i++)
                {
                    ScDPSaveData* pDPSave = (*pDPs)[i]->GetSaveData();
                    if (pDPSave)
                    {
                        ScRange aOutRange = (*pDPs)[i]->GetOutRange();
                        rtl::OUString sTargetRangeAddress;
                        ScXMLConverter::GetStringFromRange( sTargetRangeAddress, aOutRange, pDoc );
                        ScDocAttrIterator aAttrItr(pDoc, aOutRange.aStart.Tab(),
                            aOutRange.aStart.Col(), aOutRange.aStart.Row(),
                            aOutRange.aEnd.Col(), aOutRange.aEnd.Row());
                        sal_uInt16 nCol, nRow1, nRow2;
                        rtl::OUString sOUButtonList;
                        const ScPatternAttr* pAttr = aAttrItr.GetNext(nCol, nRow1, nRow2);
                        while (pAttr)
                        {
                            ScMergeFlagAttr& rItem = (ScMergeFlagAttr&)pAttr->GetItem(ATTR_MERGE_FLAG);
                            if (rItem.HasButton())
                            {
                                for (sal_Int32 nButtonRow = nRow1; nButtonRow <= nRow2; nButtonRow++)
                                {
                                    ScAddress aButtonAddr(nCol, nButtonRow, aOutRange.aStart.Tab());
                                    ScXMLConverter::GetStringFromAddress(
                                        sOUButtonList, aButtonAddr, pDoc, sal_True );
                                }
                            }
                            pAttr = aAttrItr.GetNext(nCol, nRow1, nRow2);
                        }
                        rtl::OUString sName((*pDPs)[i]->GetName());
                        rtl::OUString sApplicationData((*pDPs)[i]->GetTag());
                        sal_Bool bRowGrand = pDPSave->GetRowGrand();
                        sal_Bool bColumnGrand = pDPSave->GetColumnGrand();
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_name, sName);
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_application_data, sApplicationData);
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_target_range_address, sTargetRangeAddress);
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_buttons, sOUButtonList);
                        if (!(bRowGrand && bColumnGrand))
                        {
                            if (bRowGrand)
                                rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_grand_total, sXML_row);
                            else if (bColumnGrand)
                                rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_grand_total, sXML_column);
                            else
                                rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_grand_total, sXML_none);
                        }
                        if (pDPSave->GetIgnoreEmptyRows())
                            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_ignore_empty_rows, sXML_true);
                        if (pDPSave->GetRepeatIfEmpty())
                            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_identify_categories, sXML_true);
                        SvXMLElementExport aElemDP(rExport, XML_NAMESPACE_TABLE, sXML_data_pilot_table, sal_True, sal_True);
                        rExport.CheckAttrList();
                        if ((*pDPs)[i]->IsSheetData())
                        {
                            const ScSheetSourceDesc* pSheetSource = (*pDPs)[i]->GetSheetDesc();
                            rtl::OUString sCellRangeAddress;
                            ScXMLConverter::GetStringFromRange( sCellRangeAddress, pSheetSource->aSourceRange, pDoc );
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_cell_range_address, sCellRangeAddress);
                            SvXMLElementExport aElemSCR(rExport, XML_NAMESPACE_TABLE, sXML_source_cell_range, sal_True, sal_True);
                            rExport.CheckAttrList();
                            WriteDPFilter(pSheetSource->aQueryParam);
                        }
                        else if ((*pDPs)[i]->IsImportData())
                        {
                            const ScImportSourceDesc* pImpSource = (*pDPs)[i]->GetImportSourceDesc();
                            switch (pImpSource->nType)
                            {
                                case sheet::DataImportMode_NONE : break;
                                case sheet::DataImportMode_QUERY :
                                {
                                    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_database_name, rtl::OUString(pImpSource->aDBName));
                                    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_query_name, rtl::OUString(pImpSource->aObject));
                                    SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, sXML_database_source_query, sal_True, sal_True);
                                    rExport.CheckAttrList();
                                }
                                break;
                                case sheet::DataImportMode_TABLE :
                                {
                                    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_database_name, rtl::OUString(pImpSource->aDBName));
                                    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_table_name, rtl::OUString(pImpSource->aObject));
                                    SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, sXML_database_source_table, sal_True, sal_True);
                                    rExport.CheckAttrList();
                                }
                                break;
                                case sheet::DataImportMode_SQL :
                                {
                                    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_database_name, rtl::OUString(pImpSource->aDBName));
                                    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_sql_statement, rtl::OUString(pImpSource->aObject));
                                    if (!pImpSource->bNative)
                                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_parse_sql_statement, sXML_true);
                                    SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, sXML_database_source_sql, sal_True, sal_True);
                                    rExport.CheckAttrList();
                                }
                                break;
                            }
                        }
                        else if ((*pDPs)[i]->IsServiceData())
                        {
                            const ScDPServiceDesc* pServSource = (*pDPs)[i]->GetDPServiceDesc();
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_name, rtl::OUString(pServSource->aServiceName));
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_source_name, rtl::OUString(pServSource->aParSource));
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_object_name, rtl::OUString(pServSource->aParName));
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_username, rtl::OUString(pServSource->aParUser));
                            // How to write the Passwort? We must know, whether the passwort shoulb be written encrypted and how or not
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_passwort, rtl::OUString(pServSource->aParPass));
                            SvXMLElementExport aElemSD(rExport, XML_NAMESPACE_TABLE, sXML_source_service, sal_True, sal_True);
                            rExport.CheckAttrList();
                        }
                        List aDimensions = pDPSave->GetDimensions();
                        sal_Int32 nDimCount = aDimensions.Count();
                        for (sal_Int32 nDim = 0; nDim < nDimCount; nDim++)
                        {
                            ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimensions.GetObject(nDim);
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_source_field_name, rtl::OUString(pDim->GetName()));
                            if (pDim->IsDataLayout())
                                rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_is_data_layout_field, sXML_true);
                            rtl::OUString sValueStr;
                            ScXMLConverter::GetStringFromOrientation( sValueStr,
                                (sheet::DataPilotFieldOrientation) pDim->GetOrientation() );
                            if( sValueStr.getLength() )
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_orientation, sValueStr );
                            if (pDim->GetUsedHierarchy() != 1)
                            {
                                rtl::OUStringBuffer sBuffer;
                                SvXMLUnitConverter::convertNumber(sBuffer, pDim->GetUsedHierarchy());
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_used_hierarchy, sBuffer.makeStringAndClear());
                            }
                            ScXMLConverter::GetStringFromFunction( sValueStr,
                                (sheet::GeneralFunction) pDim->GetFunction() );
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_function, sValueStr);
                            SvXMLElementExport aElemDPF(rExport, XML_NAMESPACE_TABLE, sXML_data_pilot_field, sal_True, sal_True);
                            rExport.CheckAttrList();
                            {
                                rtl::OUStringBuffer sBuffer;
                                SvXMLUnitConverter::convertBool(sBuffer, pDim->GetShowEmpty());
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_display_empty, sBuffer.makeStringAndClear());
                                SvXMLElementExport aElemDPL(rExport, XML_NAMESPACE_TABLE, sXML_data_pilot_level, sal_True, sal_True);
                                rExport.CheckAttrList();
                                sal_Int32 nSubTotalCount = pDim->GetSubTotalsCount();
                                if (nSubTotalCount > 0)
                                {
                                    SvXMLElementExport aElemSTs(rExport, XML_NAMESPACE_TABLE, sXML_data_pilot_subtotals, sal_True, sal_True);
                                    rExport.CheckAttrList();
                                    for (sal_Int32 nSubTotal = 0; nSubTotal < nSubTotalCount; nSubTotal++)
                                    {
                                        rtl::OUString sFunction;
                                        ScXMLConverter::GetStringFromFunction( sFunction, (sheet::GeneralFunction)pDim->GetSubTotalFunc(nSubTotal) );
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_function, sFunction);
                                        SvXMLElementExport aElemST(rExport, XML_NAMESPACE_TABLE, sXML_data_pilot_subtotal, sal_True, sal_True);
                                    }
                                }
                                List aMembers = pDim->GetMembers();
                                sal_Int32 nMemberCount = aMembers.Count();
                                if (nMemberCount > 0)
                                {
                                    SvXMLElementExport aElemDPMs(rExport, XML_NAMESPACE_TABLE, sXML_data_pilot_members, sal_True, sal_True);
                                    rExport.CheckAttrList();
                                    for (sal_Int32 nMember = 0; nMember < nMemberCount; nMember++)
                                    {
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_name, rtl::OUString(((ScDPSaveMember*)aMembers.GetObject(nMember))->GetName()));
                                        rtl::OUStringBuffer sBuffer;
                                        SvXMLUnitConverter::convertBool(sBuffer, ((ScDPSaveMember*)aMembers.GetObject(nMember))->GetIsVisible());
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_display, sBuffer.makeStringAndClear());
                                        SvXMLUnitConverter::convertBool(sBuffer, ((ScDPSaveMember*)aMembers.GetObject(nMember))->GetShowDetails());
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_display_details, sBuffer.makeStringAndClear());
                                        SvXMLElementExport aElemDPM(rExport, XML_NAMESPACE_TABLE, sXML_data_pilot_member, sal_True, sal_True);
                                        rExport.CheckAttrList();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
