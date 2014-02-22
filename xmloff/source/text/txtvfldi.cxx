/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


/** @#file
 *
 *  export of all variable related text fields (and database display field)
 */
#include "txtvfldi.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmlnumi.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/i18nmap.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlement.hxx>
#include <com/sun/star/text/SetVariableType.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include <sax/tools/converter.hxx>

#include <rtl/ustring.hxx>

#include <tools/debug.hxx>



static const sal_Char sAPI_textfield_prefix[]   = "com.sun.star.text.TextField.";
static const sal_Char sAPI_fieldmaster_prefix[] = "com.sun.star.text.FieldMaster.";
static const sal_Char sAPI_input[]              = "Input";
static const sal_Char sAPI_input_user[]         = "InputUser";
static const sal_Char sAPI_get_expression[]     = "GetExpression";
static const sal_Char sAPI_set_expression[]     = "SetExpression";
static const sal_Char sAPI_user[]               = "User";
static const sal_Char sAPI_table_formula[]      = "TableFormula";
static const sal_Char sAPI_database[]           = "com.sun.star.text.TextField.Database";
static const sal_Char sAPI_fieldmaster_database[] = "com.sun.star.text.FieldMaster.Database";


static const sal_Char sAPI_hint[]               = "Hint";
static const sal_Char sAPI_help[]               = "Help";
static const sal_Char sAPI_tooltip[]            = "Tooltip";
static const sal_Char sAPI_content[]            = "Content";
static const sal_Char sAPI_sub_type[]           = "SubType";
static const sal_Char sAPI_is_expression[]      = "IsExpression";
static const sal_Char sAPI_is_input[]           = "Input";
static const sal_Char sAPI_is_show_formula[]    = "IsShowFormula";
static const sal_Char sAPI_number_format[]      = "NumberFormat";
static const sal_Char sAPI_name[]               = "Name";
static const sal_Char sAPI_numbering_separator[]    = "NumberingSeparator";
static const sal_Char sAPI_chapter_numbering_level[]= "ChapterNumberingLevel";
static const sal_Char sAPI_value[]              = "Value";
static const sal_Char sAPI_is_visible[]         = "IsVisible";
static const sal_Char sAPI_data_column_name[]   = "DataColumnName";
static const sal_Char sAPI_is_data_base_format[]    = "DataBaseFormat";
static const sal_Char sAPI_current_presentation[]   = "CurrentPresentation";
static const sal_Char sAPI_sequence_value[]     = "SequenceValue";
static const sal_Char sAPI_is_fixed_language[] = "IsFixedLanguage";



using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;
using namespace ::xmloff::token;



//

//

TYPEINIT1( XMLVarFieldImportContext, XMLTextFieldImportContext );

XMLVarFieldImportContext::XMLVarFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    const sal_Char* pServiceName, sal_uInt16 nPrfx,
    const OUString& rLocalName,
    sal_Bool bFormula, sal_Bool bFormulaDefault,
    sal_Bool bDescription, sal_Bool bHelp, sal_Bool bHint, sal_Bool bVisible,
    sal_Bool bIsDisplayFormula,
    sal_Bool bType, sal_Bool bStyle, sal_Bool bValue,
    sal_Bool bPresentation) :
        XMLTextFieldImportContext(rImport, rHlp, pServiceName, nPrfx, rLocalName),
        sPropertyContent(sAPI_content),
        sPropertyHint(sAPI_hint),
        sPropertyHelp(sAPI_help),
        sPropertyTooltip(sAPI_tooltip),
        sPropertyIsVisible(sAPI_is_visible),
        sPropertyIsDisplayFormula(sAPI_is_show_formula),
        sPropertyCurrentPresentation(sAPI_current_presentation),
        aValueHelper(rImport, rHlp, bType, bStyle, bValue, sal_False),
        bDisplayFormula(sal_False),
        bDisplayNone(sal_False),
        bNameOK(sal_False),
        bFormulaOK(sal_False),
        bDescriptionOK(sal_False),
        bHelpOK(sal_False),
        bHintOK(sal_False),
        bDisplayOK(sal_False),
        bSetFormula(bFormula),
        bSetFormulaDefault(bFormulaDefault),
        bSetDescription(bDescription),
        bSetHelp(bHelp),
        bSetHint(bHint),
        bSetVisible(bVisible),
        bSetDisplayFormula(bIsDisplayFormula),
        bSetPresentation(bPresentation)
{
}

void XMLVarFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
        {
        case XML_TOK_TEXTFIELD_NAME:
            sName = sAttrValue;
            bNameOK = sal_True;
            bValid = sal_True;      
            break;
        case XML_TOK_TEXTFIELD_DESCRIPTION:
            sDescription = sAttrValue;
            bDescriptionOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_HELP:
            sHelp = sAttrValue;
            bHelpOK = true;
            break;
        case XML_TOK_TEXTFIELD_HINT:
            sHint = sAttrValue;
            bHintOK = true;
            break;
        case XML_TOK_TEXTFIELD_FORMULA:
            {
                OUString sTmp;
                sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                        _GetKeyByAttrName( sAttrValue, &sTmp, sal_False );
                if( XML_NAMESPACE_OOOW == nPrefix )
                {
                    sFormula = sTmp;
                    bFormulaOK = sal_True;
                }
                else
                    sFormula = sAttrValue;
            }
            break;
        case XML_TOK_TEXTFIELD_DISPLAY:
            if (IsXMLToken(sAttrValue, XML_FORMULA))
            {
                bDisplayFormula = sal_True;
                bDisplayNone = sal_False;
                bDisplayOK = sal_True;
            }
            else if (IsXMLToken(sAttrValue, XML_VALUE))
            {
                bDisplayFormula = sal_False;
                bDisplayNone = sal_False;
                bDisplayOK = sal_True;
            }
            else if (IsXMLToken(sAttrValue, XML_NONE))
            {
                bDisplayFormula = sal_False;
                bDisplayNone = sal_True;
                bDisplayOK = sal_True;
            } 
            DBG_ASSERT(!(bDisplayFormula && bDisplayNone),
                       "illegal display values");
            break;
        default:
            
            aValueHelper.ProcessAttribute(nAttrToken, sAttrValue);
            break;
        }
}

void XMLVarFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    

    if (bSetFormula)
    {
        if (!bFormulaOK && bSetFormulaDefault)
        {
            sFormula = GetContent();
            bFormulaOK = sal_True;
        }

        if (bFormulaOK)
        {
            Any aAny;
            aAny <<= sFormula;
            xPropertySet->setPropertyValue(sPropertyContent, aAny);
        }
    }

    if (bSetDescription && bDescriptionOK)
    {
        Any aAny;
        aAny <<= sDescription;
        xPropertySet->setPropertyValue(sPropertyHint, aAny);
    }

    if (bSetHelp && bHelpOK)
    {
        Any aAny;
        aAny <<= sHelp;
        xPropertySet->setPropertyValue(sPropertyHelp, aAny);
    }

    if (bSetHint && bHintOK)
    {
        Any aAny;
        aAny <<= sHint;
        xPropertySet->setPropertyValue(sPropertyTooltip, aAny);
    }

    if (bSetVisible && bDisplayOK)
    {
        Any aAny;
        sal_Bool bTmp = ! (bDisplayNone && bDisplayOK);
        aAny.setValue( &bTmp, ::getBooleanCppuType());
        xPropertySet->setPropertyValue(sPropertyIsVisible, aAny);
    }

    
    if (xPropertySet->getPropertySetInfo()->
                hasPropertyByName(sPropertyIsDisplayFormula) &&
        !bSetDisplayFormula)
    {
        bDisplayFormula = sal_False;
        bSetDisplayFormula = sal_True;
    }


    if (bSetDisplayFormula)
    {
        Any aAny;
        sal_Bool bTmp = bDisplayFormula && bDisplayOK;
        aAny.setValue( &bTmp, ::getBooleanCppuType());
        xPropertySet->setPropertyValue(sPropertyIsDisplayFormula, aAny);
    }

    
    aValueHelper.SetDefault(GetContent());
    aValueHelper.PrepareField(xPropertySet);

    
    if (bSetPresentation)
    {
        Any aAny;
        aAny <<= GetContent();
        xPropertySet->setPropertyValue(sPropertyCurrentPresentation, aAny);
    }
}





//

//

TYPEINIT1( XMLSetVarFieldImportContext, XMLVarFieldImportContext );

XMLSetVarFieldImportContext::XMLSetVarFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    const sal_Char* pServiceName, sal_uInt16 nPrfx,
    const OUString& rLocalName, VarType eVarType,
    sal_Bool bFormula, sal_Bool bFormulaDefault,
    sal_Bool bDescription, sal_Bool bHelp, sal_Bool bHint, sal_Bool bVisible, sal_Bool bIsDisplayFormula,
    sal_Bool bType, sal_Bool bStyle, sal_Bool bValue, sal_Bool bPresentation) :
        XMLVarFieldImportContext(rImport, rHlp, pServiceName,
                                 nPrfx, rLocalName,
                                 bFormula, bFormulaDefault,
                                 bDescription, bHelp, bHint, bVisible, bIsDisplayFormula,
                                 bType, bStyle, bValue, bPresentation),
        eFieldType(eVarType)
{
}

void XMLSetVarFieldImportContext::EndElement()
{
    
    
    
    
    

    DBG_ASSERT(!GetServiceName().isEmpty(), "no service name for element!");

    if (bValid)
    {
        DBG_ASSERT(!GetName().isEmpty(), "variable name needed!");

        
        Reference<XPropertySet> xMaster;
        if (FindFieldMaster(xMaster))
        {
            
            Reference<XPropertySet> xPropSet;
            if (CreateField(xPropSet, OUString(sAPI_textfield_prefix) + GetServiceName()))
            {
                Reference<XDependentTextField> xDepTextField(xPropSet, UNO_QUERY);
                if (xDepTextField.is())
                {
                    
                    xDepTextField->attachTextFieldMaster(xMaster);

                    
                    Reference<XTextContent> xTextContent(xPropSet, UNO_QUERY);
                    if (xTextContent.is())
                    {
                        try {
                        
                        GetImportHelper().InsertTextContent(xTextContent);
                        PrepareField(xPropSet);
                        } catch (lang::IllegalArgumentException & /*e*/)
                        {
                            
                        };
                        return;
                    }
                }
            }
        }
    }

    
    
    GetImportHelper().InsertString(GetContent());
}

sal_Bool XMLSetVarFieldImportContext::FindFieldMaster(
    Reference<XPropertySet> & xMaster)
{
    
    
    return XMLVariableDeclImportContext::FindFieldMaster(xMaster,
                                                         GetImport(),
                                                         GetImportHelper(),
                                                         GetName(),
                                                         eFieldType);
}



//

//

TYPEINIT1( XMLSequenceFieldImportContext, XMLSetVarFieldImportContext );

XMLSequenceFieldImportContext::XMLSequenceFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& rLocalName) :
        XMLSetVarFieldImportContext(rImport, rHlp, sAPI_set_expression,
                                    nPrfx, rLocalName, VarTypeSequence,
                                    
                                    sal_True, sal_True,
                                    sal_False, sal_False, sal_False, sal_False,
                                    sal_False,
                                    sal_False, sal_False, sal_False, sal_True),

        sPropertyNumberFormat(sAPI_number_format),
        sPropertySequenceValue(sAPI_sequence_value),
        sNumFormat(OUString('1')),
        sNumFormatSync(GetXMLToken(XML_FALSE)),
        bRefNameOK(sal_False)
{
}

void XMLSequenceFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken, const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_NUM_FORMAT:
            sNumFormat = sAttrValue;
            break;
        case XML_TOK_TEXTFIELD_NUM_LETTER_SYNC:
            sNumFormatSync = sAttrValue;
            break;
        case XML_TOK_TEXTFIELD_REF_NAME:
            sRefName = sAttrValue;
            bRefNameOK = sal_True;
            break;
        default:
            
            XMLSetVarFieldImportContext::ProcessAttribute(nAttrToken,
                                                          sAttrValue);
            break;
    } 
}

void XMLSequenceFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    
    XMLSetVarFieldImportContext::PrepareField(xPropertySet);

    
    sal_Int16 nNumType = NumberingType::ARABIC;
    GetImport().GetMM100UnitConverter().convertNumFormat( nNumType, sNumFormat, sNumFormatSync );
    Any aAny;
    aAny <<= nNumType;
    xPropertySet->setPropertyValue(sPropertyNumberFormat, aAny);

    
    if (bRefNameOK)
    {
        aAny = xPropertySet->getPropertyValue(sPropertySequenceValue);
        sal_Int16 nValue = 0;
        aAny >>= nValue;
        GetImportHelper().InsertSequenceID(sRefName, GetName(), nValue);
    }
}



//

//

TYPEINIT1( XMLVariableSetFieldImportContext, XMLSetVarFieldImportContext );

XMLVariableSetFieldImportContext::XMLVariableSetFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& rLocalName) :
        XMLSetVarFieldImportContext(rImport, rHlp, sAPI_set_expression,
                                    nPrfx, rLocalName, VarTypeSimple,
                                    
                                    
                                    sal_True, sal_True,
                                    sal_False, sal_False, sal_False,
                                    sal_True, sal_False,
                                    sal_True, sal_True, sal_True,
                                    sal_True),
        sPropertySubType(sAPI_sub_type)
{
}

void XMLVariableSetFieldImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    
    Any aAny;
    aAny <<= (IsStringValue()? SetVariableType::STRING : SetVariableType::VAR);
    xPropertySet->setPropertyValue(sPropertySubType, aAny);

    
    XMLSetVarFieldImportContext::PrepareField(xPropertySet);
}



//

//

TYPEINIT1( XMLVariableInputFieldImportContext, XMLSetVarFieldImportContext );

XMLVariableInputFieldImportContext::XMLVariableInputFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& rLocalName) :
        XMLSetVarFieldImportContext(rImport, rHlp, sAPI_set_expression,
                                    nPrfx, rLocalName, VarTypeSimple,
                                    
                                    
                                    sal_True, sal_True,
                                    sal_True, sal_True, sal_True,
                                    sal_True, sal_False,
                                    sal_True, sal_True, sal_True,
                                    sal_True),
        sPropertySubType(sAPI_sub_type),
        sPropertyIsInput(sAPI_is_input)
{
}

void XMLVariableInputFieldImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    
    Any aAny;
    sal_Bool bTrue = sal_True;
    aAny.setValue( &bTrue, ::getBooleanCppuType() );
    xPropertySet->setPropertyValue(sPropertyIsInput, aAny);

    
    aAny <<= (IsStringValue()? SetVariableType::STRING : SetVariableType::VAR);
    xPropertySet->setPropertyValue(sPropertySubType, aAny);

    
    XMLSetVarFieldImportContext::PrepareField(xPropertySet);
}



//

//

TYPEINIT1( XMLUserFieldImportContext, XMLSetVarFieldImportContext );

XMLUserFieldImportContext::XMLUserFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& rLocalName) :
        XMLSetVarFieldImportContext(rImport, rHlp, sAPI_user, nPrfx,
                                    rLocalName, VarTypeUserField,
                                    
                                    sal_False, sal_False,
                                    sal_False, sal_False, sal_False, sal_True,
                                    sal_True,
                                    sal_False, sal_True, sal_False,
                                    sal_False)
{
}



//

//

TYPEINIT1( XMLUserFieldInputImportContext, XMLVarFieldImportContext );


XMLUserFieldInputImportContext::XMLUserFieldInputImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& rLocalName) :
        XMLVarFieldImportContext(rImport, rHlp, sAPI_input_user,
                                 nPrfx, rLocalName,
                                 
                                 sal_False, sal_False,
                                 sal_True, sal_False, sal_False,
                                 sal_False, sal_False,
                                 sal_False /*???*/, sal_True, sal_False,
                                 sal_False)
{
}

void XMLUserFieldInputImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;
    aAny <<= GetName();
    xPropertySet->setPropertyValue(sPropertyContent, aAny);

    
    XMLVarFieldImportContext::PrepareField(xPropertySet);
}


//

//

TYPEINIT1( XMLVariableGetFieldImportContext, XMLVarFieldImportContext );

XMLVariableGetFieldImportContext::XMLVariableGetFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& rLocalName) :
        XMLVarFieldImportContext(rImport, rHlp, sAPI_get_expression,
                                 nPrfx, rLocalName,
                                 
                                 sal_False, sal_False,
                                 sal_False, sal_False, sal_False,
                                 sal_False, sal_True,
                                 sal_True, sal_True, sal_False,
                                 sal_True),
        sPropertySubType(sAPI_sub_type)
{
}

void XMLVariableGetFieldImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    
      Any aAny;
    aAny <<= GetName();
    xPropertySet->setPropertyValue(sPropertyContent, aAny);

    
    XMLVarFieldImportContext::PrepareField(xPropertySet);
}



//

//

TYPEINIT1( XMLExpressionFieldImportContext, XMLVarFieldImportContext );

XMLExpressionFieldImportContext::XMLExpressionFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& rLocalName) :
        XMLVarFieldImportContext(rImport, rHlp, sAPI_get_expression,
                                 nPrfx, rLocalName,
                                 
                                 sal_True, sal_True,
                                 sal_False, sal_False, sal_False,
                                 sal_False, sal_True,
                                 sal_True, sal_True, sal_False,
                                 sal_True),
        sPropertySubType(sAPI_sub_type)
{
    bValid = sal_True;  
}


void XMLExpressionFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    sal_Int16 nSubType = SetVariableType::FORMULA;
    Any aAny;
    aAny <<= nSubType;
    xPropertySet->setPropertyValue(sPropertySubType, aAny);

    
    XMLVarFieldImportContext::PrepareField(xPropertySet);
}



//

//

TYPEINIT1( XMLTextInputFieldImportContext, XMLVarFieldImportContext );

XMLTextInputFieldImportContext::XMLTextInputFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLVarFieldImportContext(rImport, rHlp, sAPI_input,
                                 nPrfx, sLocalName,
                                 
                                 sal_False, sal_False,
                                 sal_True, sal_True, sal_True,
                                 sal_False, sal_False,
                                 sal_False, sal_False, sal_False,
                                 sal_False),
        sPropertyContent(sAPI_content)
{
    bValid = sal_True;  
}

void XMLTextInputFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    XMLVarFieldImportContext::PrepareField(xPropertySet);

    Any aAny;
    aAny <<= GetContent();
    xPropertySet->setPropertyValue(sPropertyContent, aAny);
}


//

//

TYPEINIT1( XMLTableFormulaImportContext, XMLTextFieldImportContext );

XMLTableFormulaImportContext::XMLTableFormulaImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx,
    const OUString& rLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_table_formula,
                                  nPrfx, rLocalName),
        sPropertyIsShowFormula("IsShowFormula"),
        sPropertyCurrentPresentation(
            "CurrentPresentation"),
        aValueHelper(rImport, rHlp, sal_False, sal_True, sal_False, sal_True),
        sFormula(),
        bIsShowFormula(sal_False)
{
}

XMLTableFormulaImportContext::~XMLTableFormulaImportContext()
{
}

void XMLTableFormulaImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_FORMULA:
            aValueHelper.ProcessAttribute( nAttrToken, sAttrValue );
            bValid = sal_True;  
            break;

        case XML_TOK_TEXTFIELD_DATA_STYLE_NAME:
            aValueHelper.ProcessAttribute( nAttrToken, sAttrValue );
            break;
        case XML_TOK_TEXTFIELD_DISPLAY:
            if ( sAttrValue == "formula" )
                 bIsShowFormula = sal_True;
            break;
        default:
            
            break;
    }
}

void XMLTableFormulaImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    
    aValueHelper.PrepareField( xPropertySet );

    Any aAny;

    
    aAny.setValue( &bIsShowFormula, ::getBooleanCppuType() );
    xPropertySet->setPropertyValue( sPropertyIsShowFormula, aAny );

    aAny <<= GetContent();
    xPropertySet->setPropertyValue( sPropertyCurrentPresentation, aAny );
}



//

//

//



//

//

TYPEINIT1( XMLVariableDeclsImportContext, SvXMLImportContext );

XMLVariableDeclsImportContext::XMLVariableDeclsImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& rLocalName, enum VarType eVarType) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        eVarDeclsContextType(eVarType),
        rImportHelper(rHlp)
{
}

SvXMLImportContext* XMLVariableDeclsImportContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName,
    const Reference<xml::sax::XAttributeList> & xAttrList )
{
    enum XMLTokenEnum eElementName;
    SvXMLImportContext* pImportContext = NULL;

    if( XML_NAMESPACE_TEXT == nPrefix )
    {
        switch (eVarDeclsContextType)
        {
            case VarTypeSequence:
                eElementName = XML_SEQUENCE_DECL;
                break;
            case VarTypeSimple:
                eElementName = XML_VARIABLE_DECL;
                break;
            case VarTypeUserField:
                eElementName = XML_USER_FIELD_DECL;
                break;
            default:
                OSL_FAIL("unknown field type!");
                eElementName = XML_SEQUENCE_DECL;
                break;
        }

        if( IsXMLToken( rLocalName, eElementName ) )
        {
            pImportContext = new XMLVariableDeclImportContext(
                GetImport(), rImportHelper, nPrefix, rLocalName, xAttrList,
                eVarDeclsContextType);
        }
    }

    
    if (NULL == pImportContext) {
        pImportContext = SvXMLImportContext::CreateChildContext(nPrefix,
                                                                rLocalName,
                                                                xAttrList);
    }

    return pImportContext;
}



//

//

TYPEINIT1( XMLVariableDeclImportContext, SvXMLImportContext );

XMLVariableDeclImportContext::XMLVariableDeclImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& rLocalName,
    const Reference<xml::sax::XAttributeList> & xAttrList,
    enum VarType eVarType) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        
        sPropertyName(sAPI_name),
        sPropertySubType(sAPI_sub_type),
        sPropertyNumberingLevel(sAPI_chapter_numbering_level),
        sPropertyNumberingSeparator(sAPI_numbering_separator),
        sPropertyIsExpression(sAPI_is_expression),
        aValueHelper(rImport, rHlp, sal_True, sal_False, sal_True, sal_False),
        nNumLevel(-1), cSeparationChar('.')
{
    if ( (XML_NAMESPACE_TEXT == nPrfx) &&
         ( ( IsXMLToken( rLocalName, XML_SEQUENCE_DECL )) ||
           ( IsXMLToken( rLocalName, XML_VARIABLE_DECL)) ||
           ( IsXMLToken( rLocalName, XML_USER_FIELD_DECL))    )) {

        

        
        sal_Int16 nLength = xAttrList->getLength();
        for(sal_Int16 i=0; i<nLength; i++) {

            OUString sLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );

            sal_uInt16 nToken = rHlp.
                GetTextFieldAttrTokenMap().Get(nPrefix, sLocalName);

            switch (nToken)
            {
                case XML_TOK_TEXTFIELD_NAME:
                    sName = xAttrList->getValueByIndex(i);
                    break;
                case XML_TOK_TEXTFIELD_NUMBERING_LEVEL:
                {
                    sal_Int32 nLevel;
                    bool const bRet = ::sax::Converter::convertNumber(
                        nLevel, xAttrList->getValueByIndex(i), 0,
                        GetImport().GetTextImport()->GetChapterNumbering()->
                                                                   getCount());
                    if (bRet)
                    {
                        nNumLevel = static_cast< sal_Int8 >( nLevel-1 ); 
                    }
                    break;
                }
                case XML_TOK_TEXTFIELD_NUMBERING_SEPARATOR:
                    cSeparationChar =
                        (sal_Char)xAttrList->getValueByIndex(i).toChar();
                    break;

                default:
                    
                    aValueHelper.ProcessAttribute(nToken,
                                                  xAttrList->getValueByIndex(i));
                    break;
            }
        }

        Reference<XPropertySet> xFieldMaster;
        if (FindFieldMaster(xFieldMaster, GetImport(), rHlp,
                            sName, eVarType))
        {
            
            Any aAny;

            switch (eVarType)
            {
            case VarTypeSequence:
                aAny <<= nNumLevel;
                xFieldMaster->setPropertyValue(sPropertyNumberingLevel, aAny);

                if (nNumLevel >= 0)
                {
                    OUString sStr(&cSeparationChar, 1);
                    aAny <<= sStr;
                    xFieldMaster->setPropertyValue(
                        sPropertyNumberingSeparator, aAny);
                }
                break;
            case VarTypeSimple:
                {
                    
                    
                    
                    aAny <<= aValueHelper.IsStringValue()
                        ? SetVariableType::STRING : SetVariableType::VAR;
                    xFieldMaster->setPropertyValue(sPropertySubType, aAny);
                }
                break;
            case VarTypeUserField:
            {
                sal_Bool bTmp = !aValueHelper.IsStringValue();
                aAny.setValue(&bTmp, ::getBooleanCppuType());
                xFieldMaster->setPropertyValue(sPropertyIsExpression, aAny);
                aValueHelper.PrepareField(xFieldMaster);
                break;
            }
            default:
                OSL_FAIL("unknown varfield type");
            } 
        } 
    } 
}



sal_Bool XMLVariableDeclImportContext::FindFieldMaster(
    Reference<XPropertySet> & xMaster, SvXMLImport& rImport,
    XMLTextImportHelper& rImportHelper,
    const OUString& sVarName, enum VarType eVarType)
{
    static sal_Int32 nCollisionCount = 0;

    
    
    OUString sName = rImportHelper.GetRenameMap().Get(
        sal::static_int_cast< sal_uInt16 >(eVarType), sVarName);

    
    Reference<XTextFieldsSupplier> xTextFieldsSupp(rImport.GetModel(),
                                                   UNO_QUERY);
    Reference<container::XNameAccess> xFieldMasterNameAccess(
        xTextFieldsSupp->getTextFieldMasters(), UNO_QUERY);

    OUStringBuffer sBuffer;
    sBuffer.appendAscii(sAPI_fieldmaster_prefix);
    sBuffer.appendAscii(sAPI_set_expression);
    sBuffer.appendAscii(".");
    sBuffer.append(sName);
    OUString sVarServiceName = sBuffer.makeStringAndClear();

    sBuffer.appendAscii(sAPI_fieldmaster_prefix);
    sBuffer.appendAscii(sAPI_user);
    sBuffer.appendAscii(".");
    sBuffer.append(sName);
    OUString sUserServiceName = sBuffer.makeStringAndClear();

    if (xFieldMasterNameAccess->hasByName(sVarServiceName)) {
        

        Any aAny = xFieldMasterNameAccess->getByName(sVarServiceName);
        aAny >>= xMaster;

        aAny = xMaster->getPropertyValue(
            
            OUString(sAPI_sub_type)
            );
        sal_Int16 nType = 0;
        aAny >>= nType;

        enum VarType eFMVarType =
            (SetVariableType::SEQUENCE == nType) ?
                VarTypeSequence : VarTypeSimple;

        if (eFMVarType != eVarType)
        {
            OUString sNew;

            

            nCollisionCount++;
            OUStringBuffer aBuf;
            aBuf.append(sName);
            aBuf.appendAscii("_renamed_");
            aBuf.append(nCollisionCount);
            sNew = aBuf.makeStringAndClear();

            rImportHelper.GetRenameMap().Add(
                sal::static_int_cast< sal_uInt16 >(eVarType), sName, sNew);

            
            return FindFieldMaster(xMaster, rImport, rImportHelper,
                                   sNew, eVarType);
        }
    } else if (xFieldMasterNameAccess->hasByName(sUserServiceName)) {
        
        Any aAny = xFieldMasterNameAccess->getByName(sUserServiceName);
        aAny >>= xMaster;

        if (VarTypeUserField != eVarType) {
            
            OUString sNew;

            

            nCollisionCount++;
            OUStringBuffer aBuf;
            aBuf.append(sName);
            aBuf.appendAscii("_renamed_");
            aBuf.append(nCollisionCount);
            sNew = aBuf.makeStringAndClear();

            rImportHelper.GetRenameMap().Add(
                sal::static_int_cast< sal_uInt16 >(eVarType), sName, sNew);

            
            return FindFieldMaster(xMaster, rImport, rImportHelper,
                                   sNew, eVarType);
        }
    } else {
        

        
        Reference<lang::XMultiServiceFactory>
            xFactory(rImport.GetModel(),UNO_QUERY);
        if( xFactory.is() ) {

            OUStringBuffer sService;
            sService.appendAscii(sAPI_fieldmaster_prefix);
            sService.appendAscii((eVarType==VarTypeUserField) ?
                                 sAPI_user : sAPI_set_expression);
            Reference<XInterface> xIfc =
                xFactory->createInstance( sService.makeStringAndClear() );
            if (xIfc.is()) {
                Reference<XPropertySet> xTmp( xIfc, UNO_QUERY );
                xMaster = xTmp;

                
                Any aAny;
                aAny <<= sName;
                xMaster->setPropertyValue(
                    
                    OUString(sAPI_name)
                    , aAny);

                if (eVarType != VarTypeUserField) {
                    

                    aAny <<= ((eVarType == VarTypeSimple) ?
                              SetVariableType::VAR :
                              SetVariableType::SEQUENCE);
                    xMaster->setPropertyValue(
                        
                        OUString(sAPI_sub_type)
                        , aAny);
                } 

            } else {
                return sal_False;
            }
        } else {
            return sal_False;
        }
    }

    DBG_ASSERT(xMaster.is(), "no field master found!?!");
    return sal_True;
}


//

//


TYPEINIT1( XMLDatabaseDisplayImportContext, XMLDatabaseFieldImportContext );

XMLDatabaseDisplayImportContext::XMLDatabaseDisplayImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& rLocalName) :
        XMLDatabaseFieldImportContext(rImport, rHlp, sAPI_database,
                                      nPrfx, rLocalName, false),
        sPropertyColumnName(sAPI_data_column_name),
        sPropertyDatabaseFormat(sAPI_is_data_base_format),
        sPropertyCurrentPresentation(sAPI_current_presentation),
        sPropertyIsVisible(sAPI_is_visible),
        aValueHelper(rImport, rHlp, sal_False, sal_True, sal_False, sal_False),
        bColumnOK(sal_False),
        bDisplay( sal_True ),
        bDisplayOK( sal_False )
{
}

void XMLDatabaseDisplayImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken, const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_COLUMN_NAME:
            sColumnName = sAttrValue;
            bColumnOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_DISPLAY:
            {
                sal_Bool bNone = IsXMLToken( sAttrValue, XML_NONE );
                sal_Bool bValue = IsXMLToken( sAttrValue, XML_VALUE );
                bDisplay = bValue;
                bDisplayOK = bNone || bValue;
            }
            break;
        case XML_TOK_TEXTFIELD_DATABASE_NAME:
        case XML_TOK_TEXTFIELD_TABLE_NAME:
        case XML_TOK_TEXTFIELD_TABLE_TYPE:
            
            XMLDatabaseFieldImportContext::ProcessAttribute(nAttrToken,
                                                            sAttrValue);
            break;
        default:
            
            aValueHelper.ProcessAttribute(nAttrToken, sAttrValue);
            break;
    }

    bValid = bTableOK && bDatabaseOK && bColumnOK;
}

void XMLDatabaseDisplayImportContext::EndElement()
{
    
    
    
    

    if (bValid)
    {

        
        Reference<XPropertySet> xMaster;

        
        if (CreateField(xMaster,
                        OUString(
                            sAPI_fieldmaster_database)))
        {
            Any aAny;
            aAny <<= sColumnName;
            xMaster->setPropertyValue(sPropertyColumnName, aAny);

            
            XMLDatabaseFieldImportContext::PrepareField(xMaster);

            
            Reference<XPropertySet> xField;
            if (CreateField(xField,
                            OUString(
                                sAPI_database)))
            {
                
                Reference<XDependentTextField> xDepField(xField, UNO_QUERY);
                if (xDepField.is())
                {
                    
                    xDepField->attachTextFieldMaster(xMaster);

                    
                    Reference<XTextContent> xTextContent(xField, UNO_QUERY);
                    if (xTextContent.is())
                    {
                        
                        GetImportHelper().InsertTextContent(xTextContent);

                        
                        sal_Bool bTmp = !aValueHelper.IsFormatOK();
                        aAny.setValue( &bTmp, ::getBooleanCppuType() );
                        xField->setPropertyValue(sPropertyDatabaseFormat,aAny);

                        
                        aValueHelper.PrepareField(xField);

                        
                        if( bDisplayOK )
                        {
                            aAny.setValue( &bDisplay, ::getBooleanCppuType() );
                            xField->setPropertyValue(sPropertyIsVisible, aAny);
                        }

                        
                        aAny <<= GetContent();
                        xField->setPropertyValue(sPropertyCurrentPresentation,
                                                    aAny);

                        
                        return;
                    }
                }
            }
        }
    }

    
    
    GetImportHelper().InsertString(GetContent());
}


//

//

enum ValueType
{
    XML_VALUE_TYPE_STRING,
    XML_VALUE_TYPE_FLOAT,
    XML_VALUE_TYPE_CURRENCY,
    XML_VALUE_TYPE_PERCENTAGE,
    XML_VALUE_TYPE_DATE,
    XML_VALUE_TYPE_TIME,
    XML_VALUE_TYPE_BOOLEAN
};

static SvXMLEnumMapEntry const aValueTypeMap[] =
{
    { XML_FLOAT,        XML_VALUE_TYPE_FLOAT },
    { XML_CURRENCY,     XML_VALUE_TYPE_CURRENCY },
    { XML_PERCENTAGE,   XML_VALUE_TYPE_PERCENTAGE },
    { XML_DATE,         XML_VALUE_TYPE_DATE },
    { XML_TIME,         XML_VALUE_TYPE_TIME },
    { XML_BOOLEAN,      XML_VALUE_TYPE_BOOLEAN },
    { XML_STRING,       XML_VALUE_TYPE_STRING },
    { XML_TOKEN_INVALID, 0 }
};

XMLValueImportHelper::XMLValueImportHelper(
    SvXMLImport& rImprt,
    XMLTextImportHelper& rHlp,
    sal_Bool bType, sal_Bool bStyle, sal_Bool bValue, sal_Bool bFormula) :
        sPropertyContent(sAPI_content),
        sPropertyValue(sAPI_value),
        sPropertyNumberFormat(sAPI_number_format),
        sPropertyIsFixedLanguage(sAPI_is_fixed_language),

        rImport(rImprt),
        rHelper(rHlp),

        fValue(0.0),
        nFormatKey(0),
        bIsDefaultLanguage(sal_True),

        bStringType(sal_False),
        bFormatOK(sal_False),
        bTypeOK(sal_False),
        bStringValueOK(sal_False),
        bFloatValueOK(sal_False),
        bFormulaOK(sal_False),

        bSetType(bType),
        bSetValue(bValue),
        bSetStyle(bStyle),
        bSetFormula(bFormula),

        bStringDefault(sal_True),
        bFormulaDefault(sal_True)
{
}

XMLValueImportHelper::~XMLValueImportHelper()
{
}

void XMLValueImportHelper::ProcessAttribute(
    sal_uInt16 nAttrToken, const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_VALUE_TYPE:
        {
            
            sal_uInt16 nTmp = 0;
            sal_Bool bRet = SvXMLUnitConverter::convertEnum(
                nTmp, sAttrValue, aValueTypeMap);

            if (bRet) {
                ValueType eValueType = (ValueType)nTmp;

                bTypeOK = sal_True;

                switch (eValueType)
                {
                    case XML_VALUE_TYPE_STRING:
                        bStringType = sal_True;
                        break;
                    case XML_VALUE_TYPE_FLOAT:
                    case XML_VALUE_TYPE_CURRENCY:
                    case XML_VALUE_TYPE_PERCENTAGE:
                    case XML_VALUE_TYPE_DATE:
                    case XML_VALUE_TYPE_TIME:
                    case XML_VALUE_TYPE_BOOLEAN:
                        bStringType = sal_False;
                        break;

                    default:
                        OSL_FAIL("unknown value type");
                        bTypeOK = sal_False;
                }
            }
            break;
        }

        case XML_TOK_TEXTFIELD_VALUE:
        {
            double fTmp;
            bool const bRet = ::sax::Converter::convertDouble(fTmp,sAttrValue);
            if (bRet) {
                bFloatValueOK = sal_True;
                fValue = fTmp;
            }
            break;
        }

        case XML_TOK_TEXTFIELD_TIME_VALUE:
        {
            double fTmp;
            bool const bRet =
                ::sax::Converter::convertDuration(fTmp, sAttrValue);
            if (bRet) {
                bFloatValueOK = sal_True;
                fValue = fTmp;
            }
            break;
        }

        case XML_TOK_TEXTFIELD_DATE_VALUE:
        {
            double fTmp;
            sal_Bool bRet = rImport.GetMM100UnitConverter().
                convertDateTime(fTmp,sAttrValue);
            if (bRet) {
                bFloatValueOK = sal_True;
                fValue = fTmp;
            }
            break;
        }

        case XML_TOK_TEXTFIELD_BOOL_VALUE:
        {
            bool bTmp(false);
            bool bRet = ::sax::Converter::convertBool(bTmp, sAttrValue);
            if (bRet) {
                bFloatValueOK = sal_True;
                fValue = (bTmp ? 1.0 : 0.0);
            }
            else
            {
                double fTmp;
                bRet = ::sax::Converter::convertDouble(fTmp, sAttrValue);
                if (bRet) {
                    bFloatValueOK = sal_True;
                    fValue = fTmp;
                }
            }
            break;
        }

        case XML_TOK_TEXTFIELD_STRING_VALUE:
            sValue = sAttrValue;
            bStringValueOK = sal_True;
            break;

        case XML_TOK_TEXTFIELD_FORMULA:
            {
                OUString sTmp;
                sal_uInt16 nPrefix = rImport.GetNamespaceMap().
                        _GetKeyByAttrName( sAttrValue, &sTmp, sal_False );
                if( XML_NAMESPACE_OOOW == nPrefix )
                {
                    sFormula = sTmp;
                    bFormulaOK = sal_True;
                }
                else
                    sFormula = sAttrValue;
            }
            break;

        case XML_TOK_TEXTFIELD_DATA_STYLE_NAME:
        {
            sal_Int32 nKey = rHelper.GetDataStyleKey(
                                          sAttrValue, &bIsDefaultLanguage);
            if (-1 != nKey)
            {
                nFormatKey = nKey;
                bFormatOK = sal_True;
            }
            break;
        }
    } 
}

void XMLValueImportHelper::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    if (bSetType)
    {
        
    }

    if (bSetFormula)
    {
        aAny <<= (!bFormulaOK && bFormulaDefault) ? sDefault : sFormula;
        xPropertySet->setPropertyValue(sPropertyContent, aAny);
    }

    
    if (bSetStyle && bFormatOK)
    {
        aAny <<= nFormatKey;
        xPropertySet->setPropertyValue(sPropertyNumberFormat, aAny);

        if( xPropertySet->getPropertySetInfo()->
                hasPropertyByName( sPropertyIsFixedLanguage ) )
        {
            sal_Bool bIsFixedLanguage = ! bIsDefaultLanguage;
            aAny.setValue( &bIsFixedLanguage, ::getBooleanCppuType() );
            xPropertySet->setPropertyValue( sPropertyIsFixedLanguage, aAny );
        }
    }

    
    if (bSetValue)
    {
        if (bStringType)
        {
            aAny <<= (!bStringValueOK && bStringDefault) ? sDefault : sValue;
            xPropertySet->setPropertyValue(sPropertyContent, aAny);
        }
        else
        {
            aAny <<= fValue;
            xPropertySet->setPropertyValue(sPropertyValue, aAny);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
