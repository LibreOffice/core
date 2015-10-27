/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "xmlcvali.hxx"
#include "xmlimprt.hxx"
#include "xmlconti.hxx"
#include "document.hxx"
#include "XMLConverter.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include <com/sun/star/sheet/TableValidationVisibility.hpp>

using namespace com::sun::star;
using namespace xmloff::token;
using namespace ::formula;

class ScXMLContentValidationContext : public SvXMLImportContext
{
    OUString      sName;
    OUString      sHelpTitle;
    OUString      sHelpMessage;
    OUString      sErrorTitle;
    OUString      sErrorMessage;
    OUString      sErrorMessageType;
    OUString      sBaseCellAddress;
    OUString      sCondition;
    sal_Int16          nShowList;
    bool           bAllowEmptyCell;
    bool           bDisplayHelp;
    bool           bDisplayError;

    SvXMLImportContextRef           xEventContext;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

    css::sheet::ValidationAlertStyle GetAlertStyle() const;
    void SetFormula( OUString& rFormula, OUString& rFormulaNmsp, FormulaGrammar::Grammar& reGrammar,
        const OUString& rCondition, const OUString& rGlobNmsp, FormulaGrammar::Grammar eGlobGrammar, bool bHasNmsp ) const;
    void GetCondition( ScMyImportValidation& rValidation ) const;

public:

    ScXMLContentValidationContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLContentValidationContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;

    void SetHelpMessage(const OUString& sTitle, const OUString& sMessage, const bool bDisplay);
    void SetErrorMessage(const OUString& sTitle, const OUString& sMessage, const OUString& sMessageType, const bool bDisplay);
    void SetErrorMacro(const bool bExecute);
};

class ScXMLHelpMessageContext : public SvXMLImportContext
{
    OUString   sTitle;
    OUStringBuffer sMessage;
    sal_Int32       nParagraphCount;
    bool        bDisplay;

    ScXMLContentValidationContext* pValidationContext;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLHelpMessageContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLContentValidationContext* pValidationContext);

    virtual ~ScXMLHelpMessageContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLErrorMessageContext : public SvXMLImportContext
{
    OUString   sTitle;
    OUStringBuffer sMessage;
    OUString   sMessageType;
    sal_Int32       nParagraphCount;
    bool        bDisplay;

    ScXMLContentValidationContext* pValidationContext;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLErrorMessageContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLContentValidationContext* pValidationContext);

    virtual ~ScXMLErrorMessageContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLErrorMacroContext : public SvXMLImportContext
{
    OUString   sName;
    bool        bExecute;

    ScXMLContentValidationContext*  pValidationContext;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLErrorMacroContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLContentValidationContext* pValidationContext);

    virtual ~ScXMLErrorMacroContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void EndElement() override;
};

ScXMLContentValidationsContext::ScXMLContentValidationsContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    // here are no attributes
}

ScXMLContentValidationsContext::~ScXMLContentValidationsContext()
{
}

SvXMLImportContext *ScXMLContentValidationsContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
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
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    nShowList(sheet::TableValidationVisibility::UNSORTED),
    bAllowEmptyCell(true),
    bDisplayHelp(false),
    bDisplayError(false)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

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
                if (IsXMLToken(sValue, XML_FALSE))
                    bAllowEmptyCell = false;
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
                else if (IsXMLToken(sValue, XML_SORT_ASCENDING))
                {
                    nShowList = sheet::TableValidationVisibility::SORTEDASCENDING;
                }
                else if (IsXMLToken(sValue, XML_SORTED_ASCENDING))
                {
                    // Read old wrong value, fdo#72548
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

SvXMLImportContext *ScXMLContentValidationContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
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

sheet::ValidationAlertStyle ScXMLContentValidationContext::GetAlertStyle() const
{
    if (IsXMLToken(sErrorMessageType, XML_MACRO))
        return sheet::ValidationAlertStyle_MACRO;
    if (IsXMLToken(sErrorMessageType, XML_STOP))
        return sheet::ValidationAlertStyle_STOP;
    if (IsXMLToken(sErrorMessageType, XML_WARNING))
        return sheet::ValidationAlertStyle_WARNING;
    if (IsXMLToken(sErrorMessageType, XML_INFORMATION))
        return sheet::ValidationAlertStyle_INFO;
    // default for unknown
    return sheet::ValidationAlertStyle_STOP;
}

void ScXMLContentValidationContext::SetFormula( OUString& rFormula, OUString& rFormulaNmsp, FormulaGrammar::Grammar& reGrammar,
    const OUString& rCondition, const OUString& rGlobNmsp, FormulaGrammar::Grammar eGlobGrammar, bool bHasNmsp ) const
{
    reGrammar = FormulaGrammar::GRAM_UNSPECIFIED;
    if( bHasNmsp )
    {
        // the entire attribute contains a namespace: internal namespace not allowed
        rFormula = rCondition;
        rFormulaNmsp = rGlobNmsp;
        reGrammar = eGlobGrammar;
    }
    else
    {
        // the attribute does not contain a namespace: try to find a namespace of an external grammar
        GetScImport().ExtractFormulaNamespaceGrammar( rFormula, rFormulaNmsp, reGrammar, rCondition, true );
        if( reGrammar != FormulaGrammar::GRAM_EXTERNAL )
            reGrammar = eGlobGrammar;
    }
}

void ScXMLContentValidationContext::GetCondition( ScMyImportValidation& rValidation ) const
{
    rValidation.aValidationType = sheet::ValidationType_ANY;    // default if no condition is given
    rValidation.aOperator = sheet::ConditionOperator_NONE;

    if( !sCondition.isEmpty() )
    {
        // extract leading namespace from condition string
        OUString aCondition, aConditionNmsp;
        FormulaGrammar::Grammar eGrammar = FormulaGrammar::GRAM_UNSPECIFIED;
        GetScImport().ExtractFormulaNamespaceGrammar( aCondition, aConditionNmsp, eGrammar, sCondition );
        bool bHasNmsp = aCondition.getLength() < sCondition.getLength();

        // parse a condition from the attribute string
        ScXMLConditionParseResult aParseResult;
        ScXMLConditionHelper::parseCondition( aParseResult, aCondition, 0 );

        /*  Check the result. A valid value in aParseResult.meToken implies
            that the other members of aParseResult are filled with valid data
            for that token. */
        bool bSecondaryPart = false;
        switch( aParseResult.meToken )
        {
            case XML_COND_TEXTLENGTH:               // condition is 'cell-content-text-length()<operator><expression>'
            case XML_COND_TEXTLENGTH_ISBETWEEN:     // condition is 'cell-content-text-length-is-between(<expression1>,<expression2>)'
            case XML_COND_TEXTLENGTH_ISNOTBETWEEN:  // condition is 'cell-content-text-length-is-not-between(<expression1>,<expression2>)'
            case XML_COND_ISINLIST:                 // condition is 'cell-content-is-in-list(<expression>)'
                rValidation.aValidationType = aParseResult.meValidation;
                rValidation.aOperator = aParseResult.meOperator;
            break;

            case XML_COND_ISWHOLENUMBER:            // condition is 'cell-content-is-whole-number() and <condition>'
            case XML_COND_ISDECIMALNUMBER:          // condition is 'cell-content-is-decimal-number() and <condition>'
            case XML_COND_ISDATE:                   // condition is 'cell-content-is-date() and <condition>'
            case XML_COND_ISTIME:                   // condition is 'cell-content-is-time() and <condition>'
                rValidation.aValidationType = aParseResult.meValidation;
                bSecondaryPart = true;
            break;

            default:;   // unacceptable or unknown condition
        }

        /*  Parse the following 'and <condition>' part of some conditions. This
            updates the members of aParseResult that will contain the operands
            and comparison operator then. */
        if( bSecondaryPart )
        {
            ScXMLConditionHelper::parseCondition( aParseResult, aCondition, aParseResult.mnEndIndex );
            if( aParseResult.meToken == XML_COND_AND )
            {
                ScXMLConditionHelper::parseCondition( aParseResult, aCondition, aParseResult.mnEndIndex );
                switch( aParseResult.meToken )
                {
                    case XML_COND_CELLCONTENT:  // condition is 'and cell-content()<operator><expression>'
                    case XML_COND_ISBETWEEN:    // condition is 'and cell-content-is-between(<expression1>,<expression2>)'
                    case XML_COND_ISNOTBETWEEN: // condition is 'and cell-content-is-not-between(<expression1>,<expression2>)'
                        rValidation.aOperator = aParseResult.meOperator;
                    break;
                    default:;   // unacceptable or unknown condition
                }
            }
        }

        // a validation type (date, integer) without a condition isn't possible
        if( rValidation.aOperator == sheet::ConditionOperator_NONE )
            rValidation.aValidationType = sheet::ValidationType_ANY;

        // parse the formulas
        if( rValidation.aValidationType != sheet::ValidationType_ANY )
        {
            SetFormula( rValidation.sFormula1, rValidation.sFormulaNmsp1, rValidation.eGrammar1,
                aParseResult.maOperand1, aConditionNmsp, eGrammar, bHasNmsp );
            SetFormula( rValidation.sFormula2, rValidation.sFormulaNmsp2, rValidation.eGrammar2,
                aParseResult.maOperand2, aConditionNmsp, eGrammar, bHasNmsp );
        }
    }
}

void ScXMLContentValidationContext::EndElement()
{
    // #i36650# event-listeners element moved up one level
    if (xEventContext.Is())
    {
        OUString sOnError("OnError");
        XMLEventsImportContext* pEvents =
            static_cast<XMLEventsImportContext*>(&xEventContext);
        uno::Sequence<beans::PropertyValue> aValues;
        pEvents->GetEventSequence( sOnError, aValues );

        sal_Int32 nLength = aValues.getLength();
        for( sal_Int32 i = 0; i < nLength; i++ )
        {
            // #i47525# must allow "MacroName" or "Script"
            if ( aValues[i].Name == "MacroName" ||
                 aValues[i].Name == "Script" )
            {
                aValues[i].Value >>= sErrorTitle;
                break;
            }
        }
    }

    ScMyImportValidation aValidation;
    aValidation.eGrammar1 = aValidation.eGrammar2 = GetScImport().GetDocument()->GetStorageGrammar();
    aValidation.sName = sName;
    aValidation.sBaseCellAddress = sBaseCellAddress;
    aValidation.sImputTitle = sHelpTitle;
    aValidation.sImputMessage = sHelpMessage;
    aValidation.sErrorTitle = sErrorTitle;
    aValidation.sErrorMessage = sErrorMessage;
    GetCondition( aValidation );
    aValidation.aAlertStyle = GetAlertStyle();
    aValidation.bShowErrorMessage = bDisplayError;
    aValidation.bShowImputMessage = bDisplayHelp;
    aValidation.bIgnoreBlanks = bAllowEmptyCell;
    aValidation.nShowList = nShowList;
    GetScImport().AddValidation(aValidation);
}

void ScXMLContentValidationContext::SetHelpMessage(const OUString& sTitle, const OUString& sMessage, const bool bDisplay)
{
    sHelpTitle = sTitle;
    sHelpMessage = sMessage;
    bDisplayHelp = bDisplay;
}

void ScXMLContentValidationContext::SetErrorMessage(const OUString& sTitle, const OUString& sMessage,
    const OUString& sMessageType, const bool bDisplay)
{
    sErrorTitle = sTitle;
    sErrorMessage = sMessage;
    sErrorMessageType = sMessageType;
    bDisplayError = bDisplay;
}

void ScXMLContentValidationContext::SetErrorMacro(const bool bExecute)
{
    sErrorMessageType = "macro";
    bDisplayError = bExecute;
}

ScXMLHelpMessageContext::ScXMLHelpMessageContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLContentValidationContext* pTempValidationContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sTitle(),
    sMessage(),
    nParagraphCount(0),
    bDisplay(false)
{
    pValidationContext = pTempValidationContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationHelpMessageAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

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

SvXMLImportContext *ScXMLHelpMessageContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetContentValidationMessageElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_P:
        {
            if(nParagraphCount)
                sMessage.append('\n');
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
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLContentValidationContext* pTempValidationContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sTitle(),
    sMessage(),
    sMessageType(),
    nParagraphCount(0),
    bDisplay(false)
{
    pValidationContext = pTempValidationContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationErrorMessageAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

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

SvXMLImportContext *ScXMLErrorMessageContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetContentValidationMessageElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_P:
        {
            if(nParagraphCount)
                sMessage.append('\n');
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
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLContentValidationContext* pTempValidationContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sName(),
    bExecute(false)
{
    pValidationContext = pTempValidationContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetContentValidationErrorMacroAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

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

SvXMLImportContext *ScXMLErrorMacroContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
