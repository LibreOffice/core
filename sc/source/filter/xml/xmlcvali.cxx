/*************************************************************************
 *
 *  $RCSfile: xmlcvali.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2000-11-02 14:05:44 $
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

#include "xmlcvali.hxx"
#include "xmlimprt.hxx"
#include "xmlconti.hxx"
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlkywd.hxx>

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLContentValidationsContext::ScXMLContentValidationsContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    // here are no attributes
}

ScXMLContentValidationsContext::~ScXMLContentValidationsContext()
{
}

SvXMLImportContext *ScXMLContentValidationsContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetContentValidationsElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_CONTENT_VALIDATION:
            pContext = new ScXMLContentValidationContext( GetScImport(), nPrefix, rLName, xAttrList);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLContentValidationsContext::EndElement()
{
}

ScXMLContentValidationContext::ScXMLContentValidationContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sName(),
    sHelpTitle(),
    sHelpMessage(),
    sErrorTitle(),
    sErrorMessage(),
    sErrorMessageType(),
    sCondition(),
    sBaseCellAddress(),
    bAllowEmptyCell(sal_True),
    bDisplayHelp(sal_False),
    bDisplayError(sal_False)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_CONTENT_VALIDATION_NAME:
                sName = sValue;
            break;
            case XML_TOK_CONTENT_VALIDATION_CONDITION:
                sCondition = sValue;
            break;
            case XML_TOK_CONTENT_VALIDATION_BASE_CELL_ADDRESS:
                sBaseCellAddress = sValue;
            break;
            case XML_TOK_CONTENT_VALIDATION_ALLOW_EMPTY_CELL:
                if (sValue.compareToAscii(sXML_false) == 0)
                    bAllowEmptyCell = sal_True;
            break;
        }
    }
}

ScXMLContentValidationContext::~ScXMLContentValidationContext()
{
}

SvXMLImportContext *ScXMLContentValidationContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetContentValidationElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_CONTENT_VALIDATION_ELEM_HELP_MESSAGE:
            pContext = new ScXMLHelpMessageContext( GetScImport(), nPrefix, rLName, xAttrList, this);
        break;
        case XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MESSAGE:
            pContext = new ScXMLErrorMessageContext( GetScImport(), nPrefix, rLName, xAttrList, this);
        break;
        case XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MACRO:
            pContext = new ScXMLErrorMacroContext( GetScImport(), nPrefix, rLName, xAttrList, this);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLContentValidationContext::GetAlertStyle(const rtl::OUString& sMessageType, com::sun::star::sheet::ValidationAlertStyle& aAlertStyle)
{
    if (sMessageType.compareToAscii("macro") == 0)
        aAlertStyle = sheet::ValidationAlertStyle_MACRO;
    else if (sMessageType.compareToAscii(sXML_stop) == 0)
        aAlertStyle = sheet::ValidationAlertStyle_STOP;
    else if (sMessageType.compareToAscii(sXML_warning) == 0)
        aAlertStyle = sheet::ValidationAlertStyle_WARNING;
    else if (sMessageType.compareToAscii(sXML_information) == 0)
        aAlertStyle = sheet::ValidationAlertStyle_INFO;
}

void ScXMLContentValidationContext::SetFormulas(const rtl::OUString& sFormulas, rtl::OUString& sFormula1, rtl::OUString& sFormula2) const
{
    sal_Int32 i = 0;
    sal_Bool bString = sal_False;
    sal_Int32 nBrakes = 0;
    while ((sFormulas[i] != ',' || nBrakes > 0 || bString) && i < sFormulas.getLength())
    {
        if (sFormulas[i] == '(')
            nBrakes++;
        if (sFormulas[i] == ')')
            nBrakes--;
        if (sFormulas[i] == '"')
            bString = !bString;
        i++;
    }
    if (sFormulas[i] == ',')
    {
        sFormula1 = sFormulas.copy(0, i);
        sFormula2 = sFormulas.copy(i + 1);
    }
}

void ScXMLContentValidationContext::GetCondition(const rtl::OUString& sTempCondition, rtl::OUString& sFormula1, rtl::OUString& sFormula2,
        com::sun::star::sheet::ValidationType& aValidationType,
        com::sun::star::sheet::ConditionOperator& aOperator)
{
    rtl::OUString sCondition = sTempCondition;
    if (sCondition.getLength())
    {
        // ToDo: erase all blanks in the condition, but not in formulas or strings
        rtl::OUString scell_content(RTL_CONSTASCII_USTRINGPARAM("cell_content"));
        rtl::OUString scell_content_is_date(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-date"));
        rtl::OUString scell_content_is_time(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-time"));
        rtl::OUString scell_content_is_between(RTL_CONSTASCII_USTRINGPARAM("cell_content_is_between"));
        rtl::OUString scell_content_text_length(RTL_CONSTASCII_USTRINGPARAM("cell-content-text-length"));
        rtl::OUString scell_content_is_not_between(RTL_CONSTASCII_USTRINGPARAM("cell_content_is_not_between"));
        rtl::OUString scell_content_is_whole_number(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-whole-number"));
        rtl::OUString scell_content_is_decimal_number(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-decimal-number"));
        rtl::OUString scell_content_text_length_is_between(RTL_CONSTASCII_USTRINGPARAM("cell-content-text-length-is-between"));
        rtl::OUString scell_content_text_length_is_not_between(RTL_CONSTASCII_USTRINGPARAM("cell-content-text-length-is-not-between"));
        sal_Int32 i = 0;
        sal_Bool bAnd(sal_True);
        while (sCondition[i] != '(' && i < sCondition.getLength())
            i++;
        if (sCondition[i] == '(')
        {
            if (i != scell_content_text_length.getLength() &&
                i != scell_content_text_length_is_between.getLength() &&
                i != scell_content_text_length_is_not_between.getLength())
            {
                if (i == scell_content_is_time.getLength())
                {
                    rtl::OUString sTemp = sCondition.copy(0, i);
                    if (sTemp == scell_content_is_time)
                        aValidationType = sheet::ValidationType_TIME;
                    else
                        aValidationType = sheet::ValidationType_DATE;
                }
                else if (i == scell_content_is_whole_number.getLength())
                    aValidationType = sheet::ValidationType_WHOLE;
                else if (i == scell_content_is_decimal_number.getLength())
                    aValidationType = sheet::ValidationType_DECIMAL;
                sCondition = sCondition.copy(i + 2);
                rtl::OUString sTemp = sCondition.copy(0, 5);
                if (sTemp.compareToAscii(" and ") == 0)
                    sCondition = sCondition.copy(5);
                else
                    bAnd = sal_False;
            }
            if (sCondition.getLength() && bAnd)
            {
                i = 0;
                while (sCondition[i] != '(' && i < sCondition.getLength())
                    i++;
                if (sCondition[i] == '(')
                {
                    sCondition = sCondition.copy(i + 1);
                    if (i == scell_content_is_between.getLength() ||
                        i == scell_content_text_length_is_between.getLength())
                    {
                        if (i == scell_content_text_length_is_between.getLength())
                            aValidationType = sheet::ValidationType_TEXT_LEN;
                        aOperator = sheet::ConditionOperator_BETWEEN;
                        sCondition = sCondition.copy(0, sCondition.getLength() - 1);
                        SetFormulas(sCondition, sFormula1, sFormula2);
                    }
                    else if (i == scell_content_is_not_between.getLength() ||
                        i == scell_content_text_length_is_not_between.getLength())
                    {
                        if (i == scell_content_text_length_is_not_between.getLength())
                            aValidationType = sheet::ValidationType_TEXT_LEN;
                        aOperator = sheet::ConditionOperator_NOT_BETWEEN;
                        sCondition = sCondition.copy(0, sCondition.getLength() - 1);
                        SetFormulas(sCondition, sFormula1, sFormula2);
                    }
                    else if (i == scell_content.getLength() ||
                        i == scell_content_text_length.getLength())
                    {
                        if (i == scell_content_text_length.getLength())
                            aValidationType = sheet::ValidationType_TEXT_LEN;
                        sCondition = sCondition.copy(1);
                        switch (sCondition[0])
                        {
                            case '<' :
                            {
                                if (sCondition[1] == '=')
                                {
                                    aOperator = sheet::ConditionOperator_LESS_EQUAL;
                                    sCondition = sCondition.copy(2);
                                }
                                else
                                {
                                    aOperator = sheet::ConditionOperator_LESS;
                                    sCondition = sCondition.copy(1);
                                }
                            }
                            break;
                            case '>' :
                            {
                                if (sCondition[1] == '=')
                                {
                                    aOperator = sheet::ConditionOperator_GREATER_EQUAL;
                                    sCondition = sCondition.copy(2);
                                }
                                else
                                {
                                    aOperator = sheet::ConditionOperator_GREATER;
                                    sCondition = sCondition.copy(1);
                                }
                            }
                            break;
                            case '=' :
                            {
                                aOperator = sheet::ConditionOperator_EQUAL;
                                sCondition = sCondition.copy(1);
                            }
                            break;
                            case '!' :
                            {
                                aOperator = sheet::ConditionOperator_NOT_EQUAL;
                                sCondition = sCondition.copy(1);
                            }
                            break;
                        }
                        sFormula1 = sCondition;
                    }
                }
            }
        }
    }
}

void ScXMLContentValidationContext::EndElement()
{
    ScMyImportValidation aValidation;
    aValidation.sName = sName;
    aValidation.sImputTitle = sHelpTitle;
    aValidation.sImputMessage = sHelpMessage;
    aValidation.sErrorTitle = sErrorTitle;
    aValidation.sErrorMessage = sErrorMessage;
    GetCondition(sCondition, aValidation.sFormula1, aValidation.sFormula2, aValidation.aValidationType, aValidation.aOperator);
    if (aValidation.sFormula1.getLength())
        ScXMLConverter::ParseFormula(aValidation.sFormula1);
    if (aValidation.sFormula2.getLength())
        ScXMLConverter::ParseFormula(aValidation.sFormula2);
    GetScImport().GetCellFromString(sBaseCellAddress, aValidation.aBaseCellAddress);
    GetAlertStyle(sErrorMessageType, aValidation.aAlertStyle);
    aValidation.bShowErrorMessage = bDisplayError;
    aValidation.bShowImputMessage = bDisplayHelp;
    aValidation.bIgnoreBlanks = bAllowEmptyCell;
    GetScImport().AddValidation(aValidation);
}

void ScXMLContentValidationContext::SetHelpMessage(const rtl::OUString& sTitle, const rtl::OUString& sMessage, const sal_Bool bDisplay)
{
    sHelpTitle = sTitle;
    sHelpMessage = sMessage;
    bDisplayHelp = bDisplay;
}

void ScXMLContentValidationContext::SetErrorMessage(const rtl::OUString& sTitle, const rtl::OUString& sMessage,
    const rtl::OUString& sMessageType, const sal_Bool bDisplay)
{
    sErrorTitle = sTitle;
    sErrorMessage = sMessage;
    sErrorMessageType = sMessageType;
    bDisplayError = bDisplay;
}

void ScXMLContentValidationContext::SetErrorMacro(const rtl::OUString& sName, const sal_Bool bExecute)
{
    sErrorTitle = sName;
    sErrorMessageType = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("macro"));
    bDisplayError = bExecute;
}

ScXMLHelpMessageContext::ScXMLHelpMessageContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLContentValidationContext* pTempValidationContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sTitle(),
    sMessage(),
    nParagraphCount(0),
    bDisplay(sal_False)
{
    pValidationContext = pTempValidationContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationHelpMessageAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_HELP_MESSAGE_ATTR_TITLE:
                sTitle = sValue;
            break;
            case XML_TOK_HELP_MESSAGE_ATTR_DISPLAY:
                if (sValue.compareToAscii(sXML_true) == 0)
                    bDisplay = sal_True;
            break;
        }
    }
}

ScXMLHelpMessageContext::~ScXMLHelpMessageContext()
{
}

SvXMLImportContext *ScXMLHelpMessageContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetContentValidationMessageElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_P:
        {
            if(nParagraphCount)
                sMessage += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
            nParagraphCount++;
            pContext = new ScXMLContentContext( GetScImport(), nPrefix, rLName, xAttrList, sMessage);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLHelpMessageContext::EndElement()
{
    pValidationContext->SetHelpMessage(sTitle, sMessage, bDisplay);
}

ScXMLErrorMessageContext::ScXMLErrorMessageContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLContentValidationContext* pTempValidationContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sTitle(),
    sMessage(),
    sMessageType(),
    nParagraphCount(0),
    bDisplay(sal_False)
{
    pValidationContext = pTempValidationContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationErrorMessageAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_ERROR_MESSAGE_ATTR_TITLE:
                sTitle = sValue;
            break;
            case XML_TOK_ERROR_MESSAGE_ATTR_MESSAGE_TYPE:
                sMessageType = sValue;
            break;
            case XML_TOK_ERROR_MESSAGE_ATTR_DISPLAY:
                if (sValue.compareToAscii(sXML_true) == 0)
                    bDisplay = sal_True;
            break;
        }
    }
}

ScXMLErrorMessageContext::~ScXMLErrorMessageContext()
{
}

SvXMLImportContext *ScXMLErrorMessageContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetContentValidationMessageElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_P:
        {
            if(nParagraphCount)
                sMessage += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
            nParagraphCount++;
            pContext = new ScXMLContentContext( GetScImport(), nPrefix, rLName, xAttrList, sMessage);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLErrorMessageContext::EndElement()
{
    pValidationContext->SetErrorMessage(sTitle, sMessage, sMessageType, bDisplay);
}

ScXMLErrorMacroContext::ScXMLErrorMacroContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLContentValidationContext* pTempValidationContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sName(),
    bExecute(sal_False)
{
    pValidationContext = pTempValidationContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationErrorMacroAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_ERROR_MACRO_ATTR_NAME:
                sName = sValue;
            break;
            case XML_TOK_ERROR_MACRO_ATTR_EXECUTE:
                if (sValue.compareToAscii(sXML_true) == 0)
                    bExecute = sal_True;
            break;
        }
    }
}

ScXMLErrorMacroContext::~ScXMLErrorMacroContext()
{
}

SvXMLImportContext *ScXMLErrorMacroContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLErrorMacroContext::EndElement()
{
    pValidationContext->SetErrorMacro(sName, bExecute);
}
