/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlcvali.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:02:04 $
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



// INCLUDE ---------------------------------------------------------------

#include "xmlcvali.hxx"
#include "xmlimprt.hxx"
#include "xmlconti.hxx"
#include "document.hxx"
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#include <xmloff/XMLEventsImportContext.hxx>
#endif
#include <com/sun/star/sheet/TableValidationVisibility.hpp>

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace com::sun::star;
using namespace xmloff::token;

class ScXMLContentValidationContext : public SvXMLImportContext
{
    rtl::OUString      sName;
    rtl::OUString      sHelpTitle;
    rtl::OUString      sHelpMessage;
    rtl::OUString      sErrorTitle;
    rtl::OUString      sErrorMessage;
    rtl::OUString      sErrorMessageType;
    rtl::OUString      sBaseCellAddress;
    rtl::OUString      sCondition;
    ScGrammar::Grammar eGrammar;
    sal_Int16          nShowList;
    sal_Bool           bAllowEmptyCell;
    sal_Bool           bDisplayHelp;
    sal_Bool           bDisplayError;

    SvXMLImportContextRef           xEventContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

    void GetAlertStyle(const rtl::OUString& sMessageType, com::sun::star::sheet::ValidationAlertStyle& aAlertStyle);
    void SetFormulas(const rtl::OUString& sFormulas, rtl::OUString& sFormula1, rtl::OUString& sFormula2) const;
    void GetCondition(const rtl::OUString& sCondition, rtl::OUString& sFormula1, rtl::OUString& sFormula2,
        com::sun::star::sheet::ValidationType& aValidationType,
        com::sun::star::sheet::ConditionOperator& aOperator);

public:

    ScXMLContentValidationContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLContentValidationContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

    void SetHelpMessage(const rtl::OUString& sTitle, const rtl::OUString& sMessage, const sal_Bool bDisplay);
    void SetErrorMessage(const rtl::OUString& sTitle, const rtl::OUString& sMessage, const rtl::OUString& sMessageType, const sal_Bool bDisplay);
    void SetErrorMacro(const sal_Bool bExecute);
};

class ScXMLHelpMessageContext : public SvXMLImportContext
{
    rtl::OUString   sTitle;
    rtl::OUStringBuffer sMessage;
    sal_Int32       nParagraphCount;
    sal_Bool        bDisplay;

    ScXMLContentValidationContext* pValidationContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLHelpMessageContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLContentValidationContext* pValidationContext);

    virtual ~ScXMLHelpMessageContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLErrorMessageContext : public SvXMLImportContext
{
    rtl::OUString   sTitle;
    rtl::OUStringBuffer sMessage;
    rtl::OUString   sMessageType;
    sal_Int32       nParagraphCount;
    sal_Bool        bDisplay;

    ScXMLContentValidationContext* pValidationContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLErrorMessageContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLContentValidationContext* pValidationContext);

    virtual ~ScXMLErrorMessageContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();
};

class ScXMLErrorMacroContext : public SvXMLImportContext
{
    rtl::OUString   sName;
    sal_Bool        bExecute;

    ScXMLContentValidationContext*  pValidationContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

public:

    ScXMLErrorMacroContext( ScXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLContentValidationContext* pValidationContext);

    virtual ~ScXMLErrorMacroContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

//------------------------------------------------------------------

ScXMLContentValidationsContext::ScXMLContentValidationsContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */ ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    // here are no attributes
}

ScXMLContentValidationsContext::~ScXMLContentValidationsContext()
{
}

SvXMLImportContext *ScXMLContentValidationsContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
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
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sName(),
    sHelpTitle(),
    sHelpMessage(),
    sErrorTitle(),
    sErrorMessage(),
    sErrorMessageType(),
    sBaseCellAddress(),
    sCondition(),
    nShowList(sheet::TableValidationVisibility::UNSORTED),
    bAllowEmptyCell(sal_True),
    bDisplayHelp(sal_False),
    bDisplayError(sal_False)
{
    const ScGrammar::Grammar eStorageGrammar = eGrammar = GetScImport().GetDocument()->GetStorageGrammar();
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_CONTENT_VALIDATION_NAME:
                sName = sValue;
            break;
            case XML_TOK_CONTENT_VALIDATION_CONDITION:
                {
                    sal_uInt16 nCondPrefix = GetImport().GetNamespaceMap().
                            _GetKeyByAttrName( sValue, &sCondition, sal_False );

                    if (!ScXMLImport::IsAcceptedFormulaNamespace( nCondPrefix,
                                sValue, eGrammar, eStorageGrammar))
                        sCondition = sValue;
                }
            break;
            case XML_TOK_CONTENT_VALIDATION_BASE_CELL_ADDRESS:
                sBaseCellAddress = sValue;
            break;
            case XML_TOK_CONTENT_VALIDATION_ALLOW_EMPTY_CELL:
                if (IsXMLToken(sValue, XML_FALSE))
                    bAllowEmptyCell = sal_False;
            break;
            case XML_TOK_CONTENT_VALIDATION_DISPLAY_LIST:
            {
                if (IsXMLToken(sValue, XML_NO))
                {
                    nShowList = sheet::TableValidationVisibility::INVISIBLE;
                }
                else if (IsXMLToken(sValue, XML_UNSORTED))
                {
                    nShowList = sheet::TableValidationVisibility::UNSORTED;
                }
                else if (IsXMLToken(sValue, XML_SORTED_ASCENDING))
                {
                    nShowList = sheet::TableValidationVisibility::SORTEDASCENDING;
                }
            }
            break;
        }
    }
}

ScXMLContentValidationContext::~ScXMLContentValidationContext()
{
}

SvXMLImportContext *ScXMLContentValidationContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
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
        case XML_TOK_CONTENT_VALIDATION_ELEM_EVENT_LISTENERS:
            pContext = new XMLEventsImportContext( GetImport(), nPrefix, rLName );
            xEventContext = pContext;
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLContentValidationContext::GetAlertStyle(const rtl::OUString& sMessageType, com::sun::star::sheet::ValidationAlertStyle& aAlertStyle)
{
    if (IsXMLToken(sMessageType, XML_MACRO))
        aAlertStyle = sheet::ValidationAlertStyle_MACRO;
    else if (IsXMLToken(sMessageType, XML_STOP))
        aAlertStyle = sheet::ValidationAlertStyle_STOP;
    else if (IsXMLToken(sMessageType, XML_WARNING))
        aAlertStyle = sheet::ValidationAlertStyle_WARNING;
    else if (IsXMLToken(sMessageType, XML_INFORMATION))
        aAlertStyle = sheet::ValidationAlertStyle_INFO;
    else    // don't leave uninitialized
        aAlertStyle = sheet::ValidationAlertStyle_STOP;
}

void ScXMLContentValidationContext::SetFormulas(const rtl::OUString& sFormulas, rtl::OUString& sFormula1, rtl::OUString& sFormula2) const
{
    sal_Int32 i = 0;
    sal_Bool bString = sal_False;
    sal_Int32 nBrakes = 0;
    while ((sFormulas[i] != ',' || nBrakes > 0 || bString) && i < sFormulas.getLength())
    {
        if (sFormulas[i] == '(')
            ++nBrakes;
        if (sFormulas[i] == ')')
            --nBrakes;
        if (sFormulas[i] == '"')
            bString = !bString;
        ++i;
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
    aValidationType = sheet::ValidationType_ANY;    // #b6343997# default if no condition is given
    aOperator = sheet::ConditionOperator_NONE;

    rtl::OUString sLocalCondition(sTempCondition);
    if (sLocalCondition.getLength())
    {
        // ToDo: erase all blanks in the condition, but not in formulas or strings
        rtl::OUString scell_content(RTL_CONSTASCII_USTRINGPARAM("cell_content"));
        rtl::OUString scell_content_is_date(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-date"));
        rtl::OUString scell_content_is_time(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-time"));
        rtl::OUString scell_content_is_between(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-between"));
        rtl::OUString scell_content_is_in_list(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-in-list"));
        rtl::OUString scell_content_text_length(RTL_CONSTASCII_USTRINGPARAM("cell-content-text-length"));
        rtl::OUString scell_content_is_not_between(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-not-between"));
        rtl::OUString scell_content_is_whole_number(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-whole-number"));
        rtl::OUString scell_content_is_decimal_number(RTL_CONSTASCII_USTRINGPARAM("cell-content-is-decimal-number"));
        rtl::OUString scell_content_text_length_is_between(RTL_CONSTASCII_USTRINGPARAM("cell-content-text-length-is-between"));
        rtl::OUString scell_content_text_length_is_not_between(RTL_CONSTASCII_USTRINGPARAM("cell-content-text-length-is-not-between"));
        sal_Int32 i = 0;
        sal_Bool bAnd(sal_True);
        while (sLocalCondition[i] != '(' && i < sLocalCondition.getLength())
            ++i;
        if (sLocalCondition[i] == '(')
        {
            if (i != scell_content_text_length.getLength() &&
                i != scell_content_text_length_is_between.getLength() &&
                i != scell_content_text_length_is_not_between.getLength() &&
                i != scell_content_is_in_list.getLength())
            {
                if (i == scell_content_is_time.getLength())
                {
                    rtl::OUString sTemp = sLocalCondition.copy(0, i);
                    if (sTemp == scell_content_is_time)
                        aValidationType = sheet::ValidationType_TIME;
                    else
                        aValidationType = sheet::ValidationType_DATE;
                }
                else if (i == scell_content_is_whole_number.getLength())
                    aValidationType = sheet::ValidationType_WHOLE;
                else if (i == scell_content_is_decimal_number.getLength())
                    aValidationType = sheet::ValidationType_DECIMAL;
                sLocalCondition = sLocalCondition.copy(i + 2);
                rtl::OUString sTemp = sLocalCondition.copy(0, 5);
                if (sTemp.compareToAscii(" and ") == 0)
                    sLocalCondition = sLocalCondition.copy(5);
                else
                    bAnd = sal_False;
            }
            if (sLocalCondition.getLength() && bAnd)
            {
                i = 0;
                while (sLocalCondition[i] != '(' && i < sLocalCondition.getLength())
                    ++i;
                if (sLocalCondition[i] == '(')
                {
                    rtl::OUString sTemp = sLocalCondition.copy(0, i);
                    sLocalCondition = sLocalCondition.copy(i + 1);
                    if (i == scell_content_is_between.getLength() ||
                        i == scell_content_text_length_is_between.getLength())
                    {
                        if (sTemp == scell_content_is_in_list)
                        {
                            aValidationType = sheet::ValidationType_LIST;
                            sFormula1 = sLocalCondition.copy(0, sLocalCondition.getLength() - 1);
                            aOperator = sheet::ConditionOperator_EQUAL;
                        }
                        else
                        {
                            if (i == scell_content_text_length_is_between.getLength())
                                aValidationType = sheet::ValidationType_TEXT_LEN;
                            aOperator = sheet::ConditionOperator_BETWEEN;
                            sLocalCondition = sLocalCondition.copy(0, sLocalCondition.getLength() - 1);
                            SetFormulas(sLocalCondition, sFormula1, sFormula2);
                        }
                    }
                    else if (i == scell_content_is_not_between.getLength() ||
                        i == scell_content_text_length_is_not_between.getLength())
                    {
                        if (i == scell_content_text_length_is_not_between.getLength())
                            aValidationType = sheet::ValidationType_TEXT_LEN;
                        aOperator = sheet::ConditionOperator_NOT_BETWEEN;
                        sLocalCondition = sLocalCondition.copy(0, sLocalCondition.getLength() - 1);
                        SetFormulas(sLocalCondition, sFormula1, sFormula2);
                    }
                    else if (i == scell_content.getLength() ||
                        i == scell_content_text_length.getLength())
                    {
                        if (i == scell_content_text_length.getLength())
                            aValidationType = sheet::ValidationType_TEXT_LEN;
                        sLocalCondition = sLocalCondition.copy(1);
                        switch (sLocalCondition[0])
                        {
                            case '<' :
                            {
                                if (sLocalCondition[1] == '=')
                                {
                                    aOperator = sheet::ConditionOperator_LESS_EQUAL;
                                    sLocalCondition = sLocalCondition.copy(2);
                                }
                                else
                                {
                                    aOperator = sheet::ConditionOperator_LESS;
                                    sLocalCondition = sLocalCondition.copy(1);
                                }
                            }
                            break;
                            case '>' :
                            {
                                if (sLocalCondition[1] == '=')
                                {
                                    aOperator = sheet::ConditionOperator_GREATER_EQUAL;
                                    sLocalCondition = sLocalCondition.copy(2);
                                }
                                else
                                {
                                    aOperator = sheet::ConditionOperator_GREATER;
                                    sLocalCondition = sLocalCondition.copy(1);
                                }
                            }
                            break;
                            case '=' :
                            {
                                aOperator = sheet::ConditionOperator_EQUAL;
                                sLocalCondition = sLocalCondition.copy(1);
                            }
                            break;
                            case '!' :
                            {
                                aOperator = sheet::ConditionOperator_NOT_EQUAL;
                                sLocalCondition = sLocalCondition.copy(1);
                            }
                            break;
                        }
                        sFormula1 = sLocalCondition;
                    }
                }
            }
        }
    }

    // a validation type (date, integer) without a condition isn't possible
    if ( aOperator == sheet::ConditionOperator_NONE )
        aValidationType = sheet::ValidationType_ANY;
}

void ScXMLContentValidationContext::EndElement()
{
    // #i36650# event-listeners element moved up one level
    if (xEventContext.Is())
    {
        rtl::OUString sOnError(RTL_CONSTASCII_USTRINGPARAM("OnError"));
        XMLEventsImportContext* pEvents =
            (XMLEventsImportContext*)&xEventContext;
        uno::Sequence<beans::PropertyValue> aValues;
        pEvents->GetEventSequence( sOnError, aValues );

        sal_Int32 nLength = aValues.getLength();
        for( sal_Int32 i = 0; i < nLength; i++ )
        {
            // #i47525# must allow "MacroName" or "Script"
            if ( aValues[i].Name.equalsAsciiL( "MacroName", sizeof("MacroName")-1 ) ||
                 aValues[i].Name.equalsAsciiL( "Script", sizeof("Script")-1 ) )
            {
                aValues[i].Value >>= sErrorTitle;
                break;
            }
        }
    }

    ScMyImportValidation aValidation;
    aValidation.eGrammar = eGrammar;
    aValidation.sName = sName;
    aValidation.sBaseCellAddress = sBaseCellAddress;
    aValidation.sImputTitle = sHelpTitle;
    aValidation.sImputMessage = sHelpMessage;
    aValidation.sErrorTitle = sErrorTitle;
    aValidation.sErrorMessage = sErrorMessage;
    GetCondition(sCondition, aValidation.sFormula1, aValidation.sFormula2, aValidation.aValidationType, aValidation.aOperator);
    GetAlertStyle(sErrorMessageType, aValidation.aAlertStyle);
    aValidation.bShowErrorMessage = bDisplayError;
    aValidation.bShowImputMessage = bDisplayHelp;
    aValidation.bIgnoreBlanks = bAllowEmptyCell;
    aValidation.nShowList = nShowList;
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

void ScXMLContentValidationContext::SetErrorMacro(const sal_Bool bExecute)
{
    sErrorMessageType = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("macro"));
    bDisplayError = bExecute;
}

ScXMLHelpMessageContext::ScXMLHelpMessageContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
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
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationHelpMessageAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_HELP_MESSAGE_ATTR_TITLE:
                sTitle = sValue;
            break;
            case XML_TOK_HELP_MESSAGE_ATTR_DISPLAY:
                bDisplay = IsXMLToken(sValue, XML_TRUE);
            break;
        }
    }
}

ScXMLHelpMessageContext::~ScXMLHelpMessageContext()
{
}

SvXMLImportContext *ScXMLHelpMessageContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
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
                sMessage.append(static_cast<sal_Unicode>('\n'));
            ++nParagraphCount;
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
    pValidationContext->SetHelpMessage(sTitle, sMessage.makeStringAndClear(), bDisplay);
}

ScXMLErrorMessageContext::ScXMLErrorMessageContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
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
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationErrorMessageAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_ERROR_MESSAGE_ATTR_TITLE:
                sTitle = sValue;
            break;
            case XML_TOK_ERROR_MESSAGE_ATTR_MESSAGE_TYPE:
                sMessageType = sValue;
            break;
            case XML_TOK_ERROR_MESSAGE_ATTR_DISPLAY:
                bDisplay = IsXMLToken(sValue, XML_TRUE);
            break;
        }
    }
}

ScXMLErrorMessageContext::~ScXMLErrorMessageContext()
{
}

SvXMLImportContext *ScXMLErrorMessageContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
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
                sMessage.append(static_cast<sal_Unicode>('\n'));
            ++nParagraphCount;
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
    pValidationContext->SetErrorMessage(sTitle, sMessage.makeStringAndClear(), sMessageType, bDisplay);
}

ScXMLErrorMacroContext::ScXMLErrorMacroContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLContentValidationContext* pTempValidationContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sName(),
    bExecute(sal_False)
{
    pValidationContext = pTempValidationContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationErrorMacroAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_ERROR_MACRO_ATTR_NAME:
                sName = sValue;
            break;
            case XML_TOK_ERROR_MACRO_ATTR_EXECUTE:
                bExecute = IsXMLToken(sValue, XML_TRUE);
            break;
        }
    }
}

ScXMLErrorMacroContext::~ScXMLErrorMacroContext()
{
}

SvXMLImportContext *ScXMLErrorMacroContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */ )
{
    SvXMLImportContext *pContext = NULL;

    if ((nPrefix == XML_NAMESPACE_SCRIPT) && IsXMLToken(rLName, XML_EVENTS))
    {
        pContext = new XMLEventsImportContext(GetImport(), nPrefix, rLName);
    }
    if (!pContext)
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLErrorMacroContext::EndElement()
{
    pValidationContext->SetErrorMacro( bExecute );
}
