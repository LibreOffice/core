/*************************************************************************
 *
 *  $RCSfile: XMLStylesExportHelper.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-28 08:19:33 $
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

#ifndef _SC_XMLSTYLESEXPORTHELPER_HXX
#include "XMLStylesExportHelper.hxx"
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_UNONAMES_HXX
#include "unonames.hxx"
#endif
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif
#ifndef SC_XMLEXPRT_HXX
#include "xmlexprt.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETCONDITION_HPP_
#include <com/sun/star/sheet/XSheetCondition.hpp>
#endif

#include <algorithm>

using namespace com::sun::star;

ScMyValidation::ScMyValidation()
    : sName(),
    sErrorMessage(),
    sErrorTitle(),
    sImputMessage(),
    sImputTitle(),
    sFormula1(),
    sFormula2(),
    bShowErrorMessage(sal_False),
    bShowImputMessage(sal_False),
    bIgnoreBlanks(sal_False)
{
}

ScMyValidation::~ScMyValidation()
{
}

sal_Bool ScMyValidation::IsEqual(const ScMyValidation& aVal) const
{
    if (aVal.bIgnoreBlanks == bIgnoreBlanks &&
        aVal.bShowImputMessage == bShowImputMessage &&
        aVal.bShowErrorMessage == bShowErrorMessage &&
        aVal.aBaseCell.Sheet == aBaseCell.Sheet &&
        aVal.aBaseCell.Column == aBaseCell.Column &&
        aVal.aBaseCell.Row == aBaseCell.Row &&
        aVal.aAlertStyle == aAlertStyle &&
        aVal.aValidationType == aValidationType &&
        aVal.aOperator == aOperator &&
        aVal.sErrorTitle == sErrorTitle &&
        aVal.sImputTitle == sImputTitle &&
        aVal.sErrorMessage == sErrorMessage &&
        aVal.sImputMessage == sImputMessage &&
        aVal.sFormula1 == sFormula1 &&
        aVal.sFormula2 == sFormula2)
        return sal_True;
    else
        return sal_False;
}

ScMyValidationsContainer::ScMyValidationsContainer()
    : aValidationVec(),
    sEmptyString()
{
}

ScMyValidationsContainer::~ScMyValidationsContainer()
{
}

sal_Bool ScMyValidationsContainer::AddValidation(const uno::Any& aTempAny,
    const table::CellRangeAddress& aCellRange, sal_Int32& nValidationIndex)
{
    sal_Bool bAdded(sal_False);
    uno::Reference<beans::XPropertySet> xPropertySet;
    if (aTempAny >>= xPropertySet)
    {
        uno::Any aAny = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ERRMESS)));
        rtl::OUString sErrorMessage;
        aAny >>= sErrorMessage;
        aAny = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ERRTITLE)));
        rtl::OUString sErrorTitle;
        aAny >>= sErrorTitle;
        aAny = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_INPMESS)));
        rtl::OUString sImputMessage;
        aAny >>= sImputMessage;
        aAny = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_INPTITLE)));
        rtl::OUString sImputTitle;
        aAny >>= sImputTitle;
        aAny = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SHOWERR)));
        sal_Bool bShowErrorMessage;
        aAny >>= bShowErrorMessage;
        aAny = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SHOWINP)));
        sal_Bool bShowImputMessage;
        aAny >>= bShowImputMessage;
        aAny = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_TYPE)));
        sheet::ValidationType aValidationType;
        aAny >>= aValidationType;
        if (bShowErrorMessage || bShowImputMessage || aValidationType != sheet::ValidationType_ANY ||
            sErrorMessage.getLength() || sErrorTitle.getLength() || sImputMessage.getLength() || sImputTitle.getLength())
        {
            ScMyValidation aValidation;
            aValidation.sErrorMessage = sErrorMessage;
            aValidation.sErrorTitle = sErrorTitle;
            aValidation.sImputMessage = sImputMessage;
            aValidation.sImputTitle = sImputTitle;
            aValidation.bShowErrorMessage = bShowErrorMessage;
            aValidation.bShowImputMessage = bShowImputMessage;
            aValidation.aValidationType = aValidationType;
            aAny = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_IGNOREBL)));
            sal_Bool bIgnoreBlanks(sal_False);
            aAny >>= bIgnoreBlanks;
            aValidation.bIgnoreBlanks = bIgnoreBlanks;
            aAny = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ERRALSTY)));
            aAny >>= aValidation.aAlertStyle;
            uno::Reference<sheet::XSheetCondition> xCondition(xPropertySet, uno::UNO_QUERY);
            if (xCondition.is())
            {
                aValidation.sFormula1 = xCondition->getFormula1();
                aValidation.sFormula2 = xCondition->getFormula2();
                aValidation.aOperator = xCondition->getOperator();
                aValidation.aBaseCell = xCondition->getSourcePosition();
            }
            //ScMyValidationRange aValidationRange;
            sal_Bool bEqualFound(sal_False);
            sal_Int32 i = 0;
            sal_Int32 nCount = aValidationVec.size();
            while (i < nCount && !bEqualFound)
            {
                bEqualFound = aValidationVec[i].IsEqual(aValidation);
                if (!bEqualFound)
                    i++;
            }
            if (bEqualFound)
                //aValidationRange.nIndex = i;
                nValidationIndex = i;
            else
            {
                sal_Int32 nNameIndex = nCount + 1;
                rtl::OUString sCount = rtl::OUString::valueOf(nNameIndex);
                rtl::OUString sPrefix(RTL_CONSTASCII_USTRINGPARAM("val"));
                aValidation.sName += sPrefix;
                aValidation.sName += sCount;
                aValidationVec.push_back(aValidation);
                //aValidationRange.nIndex = nCount;
                nValidationIndex = nCount;
                bAdded = sal_True;
            }
        }
    }
    return bAdded;
}

rtl::OUString ScMyValidationsContainer::GetCondition(const ScMyValidation& aValidation)
{
    rtl::OUString sCondition;
    if (aValidation.aValidationType != sheet::ValidationType_ANY)
    {
        switch (aValidation.aValidationType)
        {
            //case sheet::ValidationType_CUSTOM
            case sheet::ValidationType_DATE :
                sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-date()"));
            break;
            case sheet::ValidationType_DECIMAL :
                sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-decimal-number()"));
            break;
            //case sheet::ValidationType_LIST :
            case sheet::ValidationType_TEXT_LEN :
                if (aValidation.aOperator != sheet::ConditionOperator_BETWEEN &&
                    aValidation.aOperator != sheet::ConditionOperator_NOT_BETWEEN)
                    sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content-text-length()"));
            break;
            case sheet::ValidationType_TIME :
                sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-time()"));
            break;
            case sheet::ValidationType_WHOLE :
                sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-whole-number()"));
            break;
        }
        if (aValidation.sFormula1.getLength() ||
            (aValidation.aOperator == sheet::ConditionOperator_BETWEEN &&
            aValidation.aOperator == sheet::ConditionOperator_NOT_BETWEEN &&
            aValidation.sFormula2.getLength()))
        {
            if (aValidation.aValidationType != sheet::ValidationType_TEXT_LEN)
                sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" and "));
            if (aValidation.aOperator != sheet::ConditionOperator_BETWEEN &&
                aValidation.aOperator != sheet::ConditionOperator_NOT_BETWEEN)
            {
                if (aValidation.aValidationType != sheet::ValidationType_TEXT_LEN)
                    sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content()"));
                switch (aValidation.aOperator)
                {
                    case sheet::ConditionOperator_EQUAL :
                        sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
                    break;
                    case sheet::ConditionOperator_GREATER :
                        sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">"));
                    break;
                    case sheet::ConditionOperator_GREATER_EQUAL :
                        sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">="));
                    break;
                    case sheet::ConditionOperator_LESS :
                        sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<"));
                    break;
                    case sheet::ConditionOperator_LESS_EQUAL :
                        sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<="));
                    break;
                    case sheet::ConditionOperator_NOT_EQUAL :
                        sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("!="));
                    break;
                }
                sCondition += aValidation.sFormula1;
            }
            else
            {
                if (aValidation.aValidationType == sheet::ValidationType_TEXT_LEN)
                {
                    if (aValidation.aOperator == sheet::ConditionOperator_BETWEEN)
                        sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content-text-length-is-between("));
                    else
                        sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content-text-length-is-not-between("));
                }
                else
                {
                    if (aValidation.aOperator == sheet::ConditionOperator_BETWEEN)
                        sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-between("));
                    else
                        sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-not-between("));
                }
                sCondition += aValidation.sFormula1;
                sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
                sCondition += aValidation.sFormula2;
                sCondition += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
            }
        }
        else
            if (aValidation.aValidationType == sheet::ValidationType_TEXT_LEN)
                sCondition = rtl::OUString();
    }
    return sCondition;
}

rtl::OUString ScMyValidationsContainer::GetBaseCellAddress(ScDocument* pDoc, const table::CellAddress& aCell)
{
    rtl::OUString sAddress;
    ScXMLConverter::GetStringFromAddress( sAddress, aCell, pDoc );
    return sAddress;
}

void ScMyValidationsContainer::WriteMessage(ScXMLExport& rExport,
    const rtl::OUString& sTitle, const rtl::OUString& sOUMessage,
    const sal_Bool bShowMessage, const sal_Bool bIsHelpMessage)
{
    if (sTitle.getLength())
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_title, sTitle);
    if (bShowMessage)
        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_display, sXML_true);
    else
        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_display, sXML_false);
    SvXMLElementExport* pMessage = NULL;
    if (bIsHelpMessage)
        pMessage = new SvXMLElementExport(rExport, XML_NAMESPACE_TABLE, sXML_help_message, sal_True, sal_True);
    else
        pMessage = new SvXMLElementExport(rExport, XML_NAMESPACE_TABLE, sXML_error_message, sal_True, sal_True);
    if (sOUMessage.getLength())
    {
        sal_Int32 i = 0;
        rtl::OUStringBuffer sTemp;
        String sMessage(sOUMessage);
        rtl::OUString sText (sMessage.ConvertLineEnd(LINEEND_LF));
        sal_Bool bPrevCharWasSpace(sal_True);
        while(i < sText.getLength())
        {
            if ((sText[i] == '\n'))
            {
                SvXMLElementExport aElemP(rExport, XML_NAMESPACE_TEXT, sXML_p, sal_True, sal_False);
                rExport.GetTextParagraphExport()->exportText(sTemp.makeStringAndClear(), bPrevCharWasSpace);
                //rExport.GetDocHandler()->characters(sTemp.makeStringAndClear());
            }
            else
                sTemp.append(sText[i]);
            i++;
        }
        if (sTemp.getLength())
        {
            SvXMLElementExport aElemP(rExport, XML_NAMESPACE_TEXT, sXML_p, sal_True, sal_False);
            rExport.GetTextParagraphExport()->exportText(sTemp.makeStringAndClear(), bPrevCharWasSpace);
            //rExport.GetDocHandler()->characters(sTemp.makeStringAndClear());
        }
    }
    if (pMessage)
        delete pMessage;
}

void ScMyValidationsContainer::WriteValidations(ScXMLExport& rExport)
{
    if (aValidationVec.size())
    {
        SvXMLElementExport aElemVs(rExport, XML_NAMESPACE_TABLE, sXML_content_validations, sal_True, sal_True);
        ScMyValidationVec::iterator aItr = aValidationVec.begin();
        while (aItr != aValidationVec.end())
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_name, aItr->sName);
            rtl::OUString sCondition = GetCondition(*aItr);
            if (sCondition.getLength())
            {
                rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_condition, sCondition);
                if (aItr->bIgnoreBlanks)
                    rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_allow_empty_cell, sXML_false);
                else
                    rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_allow_empty_cell, sXML_true);
            }
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_base_cell_address, GetBaseCellAddress(rExport.GetDocument(), aItr->aBaseCell));
            SvXMLElementExport aElemV(rExport, XML_NAMESPACE_TABLE, sXML_content_validation, sal_True, sal_True);
            if (aItr->bShowImputMessage || aItr->sImputMessage.getLength() || aItr->sImputTitle.getLength())
            {
                WriteMessage(rExport, aItr->sImputTitle, aItr->sImputMessage, aItr->bShowImputMessage, sal_True);
            }
            if (aItr->bShowErrorMessage || aItr->sErrorMessage.getLength() || aItr->sErrorTitle.getLength())
            {
                switch (aItr->aAlertStyle)
                {
                    case sheet::ValidationAlertStyle_INFO :
                    {
                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_message_type, sXML_information);
                        WriteMessage(rExport, aItr->sErrorTitle, aItr->sErrorMessage, aItr->bShowErrorMessage, sal_False);
                    }
                    break;
                    case sheet::ValidationAlertStyle_WARNING :
                    {
                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_message_type, sXML_warning);
                        WriteMessage(rExport, aItr->sErrorTitle, aItr->sErrorMessage, aItr->bShowErrorMessage, sal_False);
                    }
                    break;
                    case sheet::ValidationAlertStyle_STOP :
                    {
                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_message_type, sXML_stop);
                        WriteMessage(rExport, aItr->sErrorTitle, aItr->sErrorMessage, aItr->bShowErrorMessage, sal_False);
                    }
                    break;
                    case sheet::ValidationAlertStyle_MACRO :
                    {
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_name, aItr->sErrorTitle);
                        if (aItr->bShowErrorMessage)
                            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_execute, sXML_true);
                        else
                            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_execute, sXML_false);
                        SvXMLElementExport(rExport, XML_NAMESPACE_TABLE, sXML_error_macro, sal_True, sal_True);
                    }
                    break;
                }
            }
            aItr++;
        }
    }
}

const rtl::OUString& ScMyValidationsContainer::GetValidationName(const sal_Int32 nIndex)
{
    DBG_ASSERT( static_cast<sal_uInt32>(nIndex) < aValidationVec.size(), "out of range" );
    return aValidationVec[nIndex].sName;
}

//==============================================================================

ScMyRowFormatRange::ScMyRowFormatRange()
    : nStartColumn(0),
    nRepeatColumns(0),
    nRepeatRows(0),
    nIndex(-1),
    nValidationIndex(-1),
    bIsAutoStyle(sal_True)
{
}

sal_Bool ScMyRowFormatRange::operator< (const ScMyRowFormatRange& rRange)
{
    return (nStartColumn < rRange.nStartColumn);
}

ScRowFormatRanges::ScRowFormatRanges()
    : aRowFormatRanges(),
    nSize(0)
{
}

ScRowFormatRanges::~ScRowFormatRanges()
{
}

void ScRowFormatRanges::Clear()
{
    aRowFormatRanges.clear();
    nSize = 0;
}

void ScRowFormatRanges::AddRange(const ScMyRowFormatRange& aFormatRange)
{
    aRowFormatRanges.push_back(aFormatRange);
    nSize++;
}

sal_Bool ScRowFormatRanges::GetNext(ScMyRowFormatRange& aFormatRange)
{
    ScMyRowFormatRangesList::iterator aItr = aRowFormatRanges.begin();
    if (aItr != aRowFormatRanges.end())
    {
        aFormatRange = (*aItr);
        aRowFormatRanges.erase(aItr);
        nSize--;
        return sal_True;
    }
    return sal_False;
}

sal_Int32 ScRowFormatRanges::GetMaxRows()
{
    ScMyRowFormatRangesList::iterator aItr = aRowFormatRanges.begin();
    sal_Int32 nMaxRows = MAXROW + 1;
    if (aItr != aRowFormatRanges.end())
        while (aItr != aRowFormatRanges.end())
        {
            if ((*aItr).nRepeatRows < nMaxRows)
                nMaxRows = (*aItr).nRepeatRows;
            aItr++;
        }
    else
        nMaxRows = 0;
    return nMaxRows;
}

sal_Int32 ScRowFormatRanges::GetSize()
{
    return nSize;
}

void ScRowFormatRanges::Sort()
{
    aRowFormatRanges.sort();
}

// ============================================================================
ScMyFormatRange::ScMyFormatRange()
    : nStyleNameIndex(-1),
    nValidationIndex(-1),
    bIsAutoStyle(sal_True)
{
}

sal_Bool ScMyFormatRange::operator<(const ScMyFormatRange& rRange)
{
    if (aRangeAddress.StartRow < rRange.aRangeAddress.StartRow)
        return sal_True;
    else
        if (aRangeAddress.StartRow == rRange.aRangeAddress.StartRow)
            return (aRangeAddress.StartColumn < rRange.aRangeAddress.StartColumn);
        else
            return sal_False;
}

ScFormatRangeStyles::ScFormatRangeStyles()
    : aTables(),
    aStyleNames(),
    aAutoStyleNames()
{
}

ScFormatRangeStyles::~ScFormatRangeStyles()
{
    ScMyOUStringVec::iterator i = aStyleNames.begin();
    while (i != aStyleNames.end())
    {
        delete *i;
        i++;
    }
    i = aAutoStyleNames.begin();
    while (i != aAutoStyleNames.end())
    {
        delete *i;
        i++;
    }
    ScMyFormatRangeListVec::iterator j = aTables.begin();
    while (j != aTables.end())
    {
        delete *j;
        j++;
    }
}

void ScFormatRangeStyles::AddNewTable(const sal_Int16 nTable)
{
    sal_Int16 nSize = aTables.size() - 1;
    if (nTable > nSize)
        for (sal_Int16 i = nSize; i < nTable; i++)
        {
            ScMyFormatRangeAddresses* aRangeAddresses = new ScMyFormatRangeAddresses;
            aTables.push_back(aRangeAddresses);
        }
}

sal_Int32 ScFormatRangeStyles::AddStyleName(rtl::OUString* pString, const sal_Bool bIsAutoStyle)
{
    if (bIsAutoStyle)
    {
        aAutoStyleNames.push_back(pString);
        return aAutoStyleNames.size() - 1;
    }
    else
    {
        sal_Int32 nCount = aStyleNames.size();
        sal_Bool bFound(sal_False);
        sal_Int32 i = nCount - 1;
        while ((i >= 0) && (!bFound))
        {
            if (aStyleNames.at(i)->equals(*pString))
                bFound = sal_True;
            else
                i--;
        }
        if (bFound)
            return i;
        else
        {
            aStyleNames.push_back(pString);
            return aStyleNames.size() - 1;
        }
    }
}

sal_Int32 ScFormatRangeStyles::GetIndexOfStyleName(const rtl::OUString& rString, const rtl::OUString& rPrefix, sal_Bool& bIsAutoStyle)
{
    sal_Int32 nPrefixLength = rPrefix.getLength();
    rtl::OUString sTemp = rString.copy(nPrefixLength);
    sal_Int32 nIndex = sTemp.toInt32();
    if (aAutoStyleNames.at(nIndex - 1)->equals(rString))
    {
        bIsAutoStyle = sal_True;
        return nIndex - 1;
    }
    else
    {
        sal_Int32 i = 0;
        sal_Bool bFound(sal_False);
        while (!bFound && static_cast<sal_uInt32>(i) < aStyleNames.size())
        {
            if (aStyleNames[i]->equals(rString))
                bFound = sal_True;
            else
                i++;
        }
        if (bFound)
        {
            bIsAutoStyle = sal_False;
            return i;
        }
        else
        {
            i = 0;
            while (!bFound && static_cast<sal_uInt32>(i) < aAutoStyleNames.size())
            {
                if (aAutoStyleNames[i]->equals(rString))
                    bFound = sal_True;
                else
                    i++;
            }
            if (bFound)
            {
                bIsAutoStyle = sal_True;
                return i;
            }
            else
                return -1;
        }
    }
}

sal_Int32 ScFormatRangeStyles::GetStyleNameIndex(const sal_Int16 nTable, const sal_Int32 nColumn, const sal_Int32 nRow, sal_Bool& bIsAutoStyle, sal_Int32& nValidationIndex)
{
    DBG_ASSERT(static_cast<sal_uInt32>(nTable) < aTables.size(), "wrong table");
    ScMyFormatRangeAddresses* pFormatRanges = aTables[nTable];
    ScMyFormatRangeAddresses::iterator aItr = pFormatRanges->begin();
    while (aItr != pFormatRanges->end())
    {
        if (((*aItr).aRangeAddress.StartColumn <= nColumn) &&
            ((*aItr).aRangeAddress.EndColumn >= nColumn) &&
            ((*aItr).aRangeAddress.StartRow <= nRow) &&
            ((*aItr).aRangeAddress.EndRow >= nRow))
        {
            bIsAutoStyle = aItr->bIsAutoStyle;
            nValidationIndex = aItr->nValidationIndex;
            return (*aItr).nStyleNameIndex;
        }
        else
        {
            if ((*aItr).aRangeAddress.EndRow < nRow)
                aItr = pFormatRanges->erase(aItr);
            else
                aItr++;
        }
    }
    return -1;
}
void ScFormatRangeStyles::GetFormatRanges(const sal_Int32 nStartColumn, const sal_Int32 nEndColumn, const sal_Int32 nRow,
                    const sal_Int16 nTable, ScRowFormatRanges* pRowFormatRanges)
{
    sal_Int32 nTotalColumns = nEndColumn - nStartColumn + 1;
    DBG_ASSERT(static_cast<sal_uInt32>(nTable) < aTables.size(), "wrong table");
    ScMyFormatRangeAddresses* pFormatRanges = aTables[nTable];
    ScMyFormatRangeAddresses::iterator aItr = pFormatRanges->begin();
    sal_Int32 nColumns = 0;
    while (aItr != pFormatRanges->end() && nColumns < nTotalColumns)
    {
        if (((*aItr).aRangeAddress.StartRow <= nRow) &&
            ((*aItr).aRangeAddress.EndRow >= nRow))
        {
            if ((((*aItr).aRangeAddress.StartColumn <= nStartColumn) &&
                ((*aItr).aRangeAddress.EndColumn >= nStartColumn)) ||
                (((*aItr).aRangeAddress.StartColumn <= nEndColumn) &&
                ((*aItr).aRangeAddress.EndColumn >= nEndColumn)) ||
                (((*aItr).aRangeAddress.StartColumn >= nStartColumn) &&
                ((*aItr).aRangeAddress.EndColumn <= nEndColumn)))
            {
                ScMyRowFormatRange aRange;
                aRange.aRangeAddress = aItr->aRangeAddress;
                aRange.nIndex = aItr->nStyleNameIndex;
                aRange.nValidationIndex = aItr->nValidationIndex;
                aRange.bIsAutoStyle = aItr->bIsAutoStyle;
                if ((aItr->aRangeAddress.StartColumn < nStartColumn) &&
                    (aItr->aRangeAddress.EndColumn >= nStartColumn))
                {
                    if (aItr->aRangeAddress.EndColumn >= nEndColumn)
                        aRange.nRepeatColumns = nTotalColumns;
                    else
                        aRange.nRepeatColumns = aItr->aRangeAddress.EndColumn - nStartColumn + 1;
                    aRange.nStartColumn = nStartColumn;
                }
                else if ((aItr->aRangeAddress.StartColumn >= nStartColumn) &&
                    (aItr->aRangeAddress.EndColumn <= nEndColumn))
                {
                    aRange.nRepeatColumns = aItr->aRangeAddress.EndColumn - aItr->aRangeAddress.StartColumn + 1;
                    aRange.nStartColumn = aItr->aRangeAddress.StartColumn;
                }
                else if ((aItr->aRangeAddress.StartColumn >= nStartColumn) &&
                    (aItr->aRangeAddress.StartColumn <= nEndColumn) &&
                    (aItr->aRangeAddress.EndColumn > nEndColumn))
                {
                    aRange.nRepeatColumns = nEndColumn - aItr->aRangeAddress.StartColumn + 1;
                    aRange.nStartColumn = aItr->aRangeAddress.StartColumn;
                }
                aRange.nRepeatRows = aItr->aRangeAddress.EndRow - nRow + 1;
                pRowFormatRanges->AddRange(aRange);
                nColumns += aRange.nRepeatColumns;
            }
            aItr++;
        }
        else
            if(aItr->aRangeAddress.EndRow < nRow)
                aItr = pFormatRanges->erase(aItr);
            else
                aItr++;
    }
    pRowFormatRanges->Sort();
}

void ScFormatRangeStyles::AddRangeStyleName(const table::CellRangeAddress aCellRangeAddress, const sal_Int32 nStringIndex, const sal_Bool bIsAutoStyle, const sal_Int32 nValidationIndex)
{
    ScMyFormatRange aFormatRange;
    aFormatRange.aRangeAddress = aCellRangeAddress;
    aFormatRange.nStyleNameIndex = nStringIndex;
    aFormatRange.nValidationIndex = nValidationIndex;
    aFormatRange.bIsAutoStyle = bIsAutoStyle;
    DBG_ASSERT(static_cast<sal_uInt32>(aCellRangeAddress.Sheet) < aTables.size(), "wrong table");
    ScMyFormatRangeAddresses* pFormatRanges = aTables[aCellRangeAddress.Sheet];
    pFormatRanges->push_back(aFormatRange);
}

/*rtl::OUString* ScFormatRangeStyles::GetStyleName(const sal_Int16 nTable, const sal_Int32 nColumn, const sal_Int32 nRow)
{
    sal_Bool bIsAutoStyle;
    sal_Int32 nIndex = GetStyleNameIndex(nTable, nColumn, nRow, bIsAutoStyle);
    if (bIsAutoStyle)
        return aAutoStyleNames.at(nIndex);
    else
        return aStyleNames.at(nIndex);
}*/

rtl::OUString* ScFormatRangeStyles::GetStyleNameByIndex(const sal_Int32 nIndex, const sal_Bool bIsAutoStyle)
{
    if (bIsAutoStyle)
        return aAutoStyleNames[nIndex];
    else
        return aStyleNames[nIndex];
}

void ScFormatRangeStyles::Sort()
{
    sal_Int16 nTables = aTables.size();
    for (sal_Int16 i = 0; i < nTables; i++)
        if (!aTables[i]->empty())
            aTables[i]->sort();
}

//===========================================================================

ScColumnRowStyles::ScColumnRowStyles()
    : aTables(),
    aStyleNames()
{
}

ScColumnRowStyles::~ScColumnRowStyles()
{
    ScMyOUStringVec::iterator i = aStyleNames.begin();
    while (i != aStyleNames.end())
    {
        delete *i;
        i++;
    }
}

void ScColumnRowStyles::AddNewTable(const sal_Int16 nTable, const sal_Int32 nFields)
{
    sal_Int16 nSize = aTables.size() - 1;
    if (nTable > nSize)
        for (sal_Int32 i = nSize; i < nTable; i++)
        {
            ScMysalInt32Vec aFieldsVec(nFields + 1, -1);
            aTables.push_back(aFieldsVec);
        }
}

sal_Int32 ScColumnRowStyles::AddStyleName(rtl::OUString* pString)
{
    aStyleNames.push_back(pString);
    return aStyleNames.size() - 1;
}

sal_Int32 ScColumnRowStyles::GetIndexOfStyleName(const rtl::OUString& rString, const rtl::OUString& rPrefix)
{
    sal_Int32 nPrefixLength = rPrefix.getLength();
    rtl::OUString sTemp = rString.copy(nPrefixLength);
    sal_Int32 nIndex = sTemp.toInt32();
    if (aStyleNames.at(nIndex - 1)->equals(rString))
        return nIndex - 1;
    else
    {
        sal_Int32 i = 0;
        sal_Bool bFound(sal_False);
        while (!bFound && static_cast<sal_uInt32>(i) < aStyleNames.size())
        {
            if (aStyleNames.at(i)->equals(rString))
                bFound = sal_True;
            else
                i++;
        }
        if (bFound)
            return i;
        else
            return -1;
    }
}

sal_Int32 ScColumnRowStyles::GetStyleNameIndex(const sal_Int16 nTable, const sal_Int32 nField)
{
    DBG_ASSERT(static_cast<sal_uInt32>(nTable) < aTables.size(), "wrong table");
    if (static_cast<sal_uInt32>(nField) < aTables[nTable].size())
        return aTables[nTable][nField];
    else
        return aTables[nTable][aTables[nTable].size() - 1];
}

void ScColumnRowStyles::AddFieldStyleName(const sal_Int16 nTable, const sal_Int32 nField, const sal_Int32 nStringIndex)
{
    DBG_ASSERT(static_cast<sal_uInt32>(nTable) < aTables.size(), "wrong table");
    DBG_ASSERT(aTables[nTable].size() >= static_cast<sal_uInt32>(nField), "wrong field");
    if (aTables[nTable].size() == static_cast<sal_uInt32>(nField))
        aTables[nTable].push_back(nStringIndex);
    aTables[nTable][nField] = nStringIndex;
}

rtl::OUString* ScColumnRowStyles::GetStyleName(const sal_Int16 nTable, const sal_Int32 nField)
{
    return aStyleNames[GetStyleNameIndex(nTable, nField)];
}

rtl::OUString* ScColumnRowStyles::GetStyleNameByIndex(const sal_Int32 nIndex)
{
    return aStyleNames[nIndex];
}

