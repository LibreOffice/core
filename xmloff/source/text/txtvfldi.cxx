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


/** @#file
 *
 *  export of all variable related text fields (and database display field)
 */
#include <txtvfldi.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/i18nmap.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlement.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/SetVariableType.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>

#include <sax/tools/converter.hxx>

#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>


// service names
constexpr char16_t sAPI_fieldmaster_prefix[] = u"com.sun.star.text.FieldMaster.";
constexpr OUString sAPI_get_expression = u"GetExpression"_ustr;
constexpr OUString sAPI_set_expression = u"SetExpression"_ustr;
constexpr OUString sAPI_user      = u"User"_ustr;
constexpr OUString sAPI_database  = u"com.sun.star.text.TextField.Database"_ustr;

// property names
constexpr OUString sAPI_content   = u"Content"_ustr;
constexpr OUString sAPI_sub_type  = u"SubType"_ustr;
constexpr OUString sAPI_number_format = u"NumberFormat"_ustr;
constexpr OUString sAPI_is_visible = u"IsVisible"_ustr;
constexpr OUString sAPI_current_presentation = u"CurrentPresentation"_ustr;


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;
using namespace ::xmloff::token;


// XMLVarFieldImportContext: superclass for all variable related fields


XMLVarFieldImportContext::XMLVarFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    const OUString& pServiceName,
    bool bFormula, bool bFormulaDefault,
    bool bDescription, bool bHelp, bool bHint, bool bVisible,
    bool bIsDisplayFormula,
    bool bType, bool bStyle, bool bValue,
    bool bPresentation) :
        XMLTextFieldImportContext(rImport, rHlp, pServiceName),
        aValueHelper(rImport, rHlp, bType, bStyle, bValue, false),
        bDisplayFormula(false),
        bDisplayNone(false),
        bFormulaOK(false),
        bDescriptionOK(false),
        bHelpOK(false),
        bHintOK(false),
        bDisplayOK(false),
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
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_NAME):
            sName = OUString::fromUtf8(sAttrValue);
            bValid = true;      // we assume: field with name is valid!
            break;
        case XML_ELEMENT(TEXT, XML_DESCRIPTION):
            sDescription = OUString::fromUtf8(sAttrValue);
            bDescriptionOK = true;
            break;
        case XML_ELEMENT(TEXT, XML_HELP):
            sHelp = OUString::fromUtf8(sAttrValue);
            bHelpOK = true;
            break;
        case XML_ELEMENT(TEXT, XML_HINT):
            sHint = OUString::fromUtf8(sAttrValue);
            bHintOK = true;
            break;
        case XML_ELEMENT(TEXT, XML_FORMULA):
            {
                OUString sTmp;
                sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                        GetKeyByAttrValueQName(OUString::fromUtf8(sAttrValue), &sTmp);
                if( XML_NAMESPACE_OOOW == nPrefix )
                {
                    sFormula = sTmp;
                    bFormulaOK = true;
                }
                else
                    sFormula = OUString::fromUtf8(sAttrValue);
            }
            break;
        case XML_ELEMENT(TEXT, XML_DISPLAY):
            if (IsXMLToken(sAttrValue, XML_FORMULA))
            {
                bDisplayFormula = true;
                bDisplayNone = false;
                bDisplayOK = true;
            }
            else if (IsXMLToken(sAttrValue, XML_VALUE))
            {
                bDisplayFormula = false;
                bDisplayNone = false;
                bDisplayOK = true;
            }
            else if (IsXMLToken(sAttrValue, XML_NONE))
            {
                bDisplayFormula = false;
                bDisplayNone = true;
                bDisplayOK = true;
            } // else: no change
            DBG_ASSERT(!(bDisplayFormula && bDisplayNone),
                       "illegal display values");
            break;
        default:
            // delegate all others to value helper
            aValueHelper.ProcessAttribute(nAttrToken, sAttrValue);
            break;
    }
}

void XMLVarFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    // bSetName: not implemented

    if (bSetFormula)
    {
        if (!bFormulaOK && bSetFormulaDefault)
        {
            sFormula = GetContent();
            bFormulaOK = true;
        }

        if (bFormulaOK)
        {
            xPropertySet->setPropertyValue(sAPI_content, Any(sFormula));
        }
    }

    if (bSetDescription && bDescriptionOK)
    {
        xPropertySet->setPropertyValue("Hint", Any(sDescription));
    }

    if (bSetHelp && bHelpOK)
    {
        xPropertySet->setPropertyValue("Help", Any(sHelp));
    }

    if (bSetHint && bHintOK)
    {
        xPropertySet->setPropertyValue("Tooltip", Any(sHint));
    }

    if (bSetVisible && bDisplayOK)
    {
        bool bTmp = !bDisplayNone;
        xPropertySet->setPropertyValue(sAPI_is_visible, Any(bTmp));
    }

    // workaround for #no-bug#: display formula by default
    if (xPropertySet->getPropertySetInfo()->
                hasPropertyByName("IsShowFormula") &&
        !bSetDisplayFormula)
    {
        bDisplayFormula = false;
        bSetDisplayFormula = true;
    }


    if (bSetDisplayFormula)
    {
        bool bTmp = bDisplayFormula && bDisplayOK;
        xPropertySet->setPropertyValue("IsShowFormula", Any(bTmp));
    }

    // delegate to value helper
    aValueHelper.SetDefault(GetContent());
    aValueHelper.PrepareField(xPropertySet);

    // finally, set the current presentation
    if (bSetPresentation)
    {
        Any aAny;
        aAny <<= GetContent();
        xPropertySet->setPropertyValue(sAPI_current_presentation, aAny);
    }
}


// variable set fields


XMLSetVarFieldImportContext::XMLSetVarFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    const OUString& pServiceName, VarType eVarType,
    bool bFormula, bool bFormulaDefault,
    bool bDescription, bool bHelp, bool bHint, bool bVisible, bool bIsDisplayFormula,
    bool bType, bool bStyle, bool bValue, bool bPresentation) :
        XMLVarFieldImportContext(rImport, rHlp, pServiceName,
                                 bFormula, bFormulaDefault,
                                 bDescription, bHelp, bHint, bVisible, bIsDisplayFormula,
                                 bType, bStyle, bValue, bPresentation),
        eFieldType(eVarType)
{
}

void XMLSetVarFieldImportContext::endFastElement(sal_Int32 )
{
    // should we call PrepareField on the field, or rather on it's master?
    // currently: call on field (just like superclass)
    // possible alternatives: call on master
    //                        call field or master depending on variable
    //                        PrepareMaster() in addition to PrepareField()

    DBG_ASSERT(!GetServiceName().isEmpty(), "no service name for element!");

    if (bValid)
    {
        DBG_ASSERT(!GetName().isEmpty(), "variable name needed!");

        // find field master
        Reference<XPropertySet> xMaster;
        if (FindFieldMaster(xMaster))
        {
            // create field/Service
            Reference<XPropertySet> xPropSet;
            if (CreateField(xPropSet, "com.sun.star.text.TextField." + GetServiceName()))
            {
                Reference<XDependentTextField> xDepTextField(xPropSet, UNO_QUERY);
                if (xDepTextField.is())
                {
                    // attach field to field master
                    xDepTextField->attachTextFieldMaster(xMaster);

                    // attach field to document
                    Reference<XTextContent> xTextContent(xPropSet, UNO_QUERY);
                    if (xTextContent.is())
                    {
                        try {
                        // insert, set field properties and exit!
                        GetImportHelper().InsertTextContent(xTextContent);
                        PrepareField(xPropSet);
                        } catch (lang::IllegalArgumentException & /*e*/)
                        {
                            // ignore e: #i54023#
                        };
                        return;
                    }
                }
            }
        }
    }

    // above: exit on success; so for all error cases we end up here!
    // write element content
    GetImportHelper().InsertString(GetContent());
}

bool XMLSetVarFieldImportContext::FindFieldMaster(
    Reference<XPropertySet> & xMaster)
{
    // currently: delegate to XMLVariableDeclImportContext;
    // should eventually go here
    return XMLVariableDeclImportContext::FindFieldMaster(xMaster,
                                                         GetImport(),
                                                         GetImportHelper(),
                                                         GetName(),
                                                         eFieldType);
}


// sequence field


XMLSequenceFieldImportContext::XMLSequenceFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLSetVarFieldImportContext(rImport, rHlp, sAPI_set_expression,
                                    VarTypeSequence,
                                    // formula
                                    true, true,
                                    false, false, false, false,
                                    false,
                                    false, false, false, true),

        sNumFormat(OUString('1')),
        sNumFormatSync(GetXMLToken(XML_FALSE)),
        bRefNameOK(false)
{
}

void XMLSequenceFieldImportContext::ProcessAttribute(
    sal_Int32 nAttrToken, std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(STYLE, XML_NUM_FORMAT):
            sNumFormat = OUString::fromUtf8(sAttrValue);
            break;
        case XML_ELEMENT(STYLE, XML_NUM_LETTER_SYNC):
            sNumFormatSync = OUString::fromUtf8(sAttrValue);
            break;
        case XML_ELEMENT(TEXT, XML_REF_NAME):
            sRefName = OUString::fromUtf8(sAttrValue);
            bRefNameOK = true;
            break;
        default:
            // delegate to super class (name, formula)
            XMLSetVarFieldImportContext::ProcessAttribute(nAttrToken,
                                                          sAttrValue);
            break;
    } // switch
}

void XMLSequenceFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    // delegate to super class (formula)
    XMLSetVarFieldImportContext::PrepareField(xPropertySet);

    // set format
    sal_Int16 nNumType = NumberingType::ARABIC;
    GetImport().GetMM100UnitConverter().convertNumFormat( nNumType, sNumFormat, sNumFormatSync );
    xPropertySet->setPropertyValue(sAPI_number_format, Any(nNumType));

    // handle reference name
    if (bRefNameOK)
    {
        Any aAny = xPropertySet->getPropertyValue("SequenceValue");
        sal_Int16 nValue = 0;
        aAny >>= nValue;
        GetImportHelper().InsertSequenceID(sRefName, GetName(), nValue);
    }
}


// variable set field


XMLVariableSetFieldImportContext::XMLVariableSetFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLSetVarFieldImportContext(rImport, rHlp, sAPI_set_expression,
                                    VarTypeSimple,
                                    // formula, value&type, style,
                                    // display none
                                    true, true,
                                    false, false, false,
                                    true, false,
                                    true, true, true,
                                    true)
{
}

void XMLVariableSetFieldImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    // set type
    Any aAny;
    aAny <<= (IsStringValue()? SetVariableType::STRING : SetVariableType::VAR);
    xPropertySet->setPropertyValue(sAPI_sub_type, aAny);

    // the remainder is handled by super class
    XMLSetVarFieldImportContext::PrepareField(xPropertySet);
}


// variable input field


XMLVariableInputFieldImportContext::XMLVariableInputFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLSetVarFieldImportContext(rImport, rHlp, sAPI_set_expression,
                                    VarTypeSimple,
                                    // description, display none/formula,
                                    // value&type, style, formula
                                    true, true,
                                    true, true, true,
                                    true, false,
                                    true, true, true,
                                    true)
{
}

void XMLVariableInputFieldImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    // set type (input field)
    Any aAny;
    xPropertySet->setPropertyValue("Input", Any(true));

    // set type
    aAny <<= (IsStringValue()? SetVariableType::STRING : SetVariableType::VAR);
    xPropertySet->setPropertyValue(sAPI_sub_type, aAny);

    // the remainder is handled by super class
    XMLSetVarFieldImportContext::PrepareField(xPropertySet);
}


// user field


XMLUserFieldImportContext::XMLUserFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLSetVarFieldImportContext(rImport, rHlp, sAPI_user,
                                    VarTypeUserField,
                                    // display none/formula, style
                                    false, false,
                                    false, false, false, true,
                                    true,
                                    false, true, false,
                                    false)
{
}


// user input field


// bug: doesn't work (SO API lacking)
XMLUserFieldInputImportContext::XMLUserFieldInputImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLVarFieldImportContext(rImport, rHlp, "InputUser",
                                 // description, style
                                 false, false,
                                 true, false, false,
                                 false, false,
                                 false /*???*/, true, false,
                                 false)
{
}

void XMLUserFieldInputImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(sAPI_content, Any(GetName()));

    // delegate to super class
    XMLVarFieldImportContext::PrepareField(xPropertySet);
}


// variable get field


XMLVariableGetFieldImportContext::XMLVariableGetFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLVarFieldImportContext(rImport, rHlp, sAPI_get_expression,
                                 // style, display formula
                                 false, false,
                                 false, false, false,
                                 false, true,
                                 true, true, false,
                                 true)
{
}

void XMLVariableGetFieldImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    // set name
    xPropertySet->setPropertyValue(sAPI_content, Any(GetName()));

    // the remainder is handled by super class
    XMLVarFieldImportContext::PrepareField(xPropertySet);
}


// expression field


XMLExpressionFieldImportContext::XMLExpressionFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLVarFieldImportContext(rImport, rHlp, sAPI_get_expression,
                                 // formula, type, style, display formula
                                 true, true,
                                 false, false, false,
                                 false, true,
                                 true, true, false,
                                 true)
{
    bValid = true;  // always valid
}


void XMLExpressionFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(sAPI_sub_type, Any(sal_Int16(SetVariableType::FORMULA)));

    // delegate to super class
    XMLVarFieldImportContext::PrepareField(xPropertySet);
}


// text input field


XMLTextInputFieldImportContext::XMLTextInputFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLVarFieldImportContext(rImport, rHlp, "Input",
                                 // description
                                 false, false,
                                 true, true, true,
                                 false, false,
                                 false, false, false,
                                 false)
{
    bValid = true;  // always valid
}

void XMLTextInputFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    XMLVarFieldImportContext::PrepareField(xPropertySet);

    xPropertySet->setPropertyValue(sAPI_content, Any(GetContent()));
}


// table formula field


XMLTableFormulaImportContext::XMLTableFormulaImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp) :
        XMLTextFieldImportContext(rImport, rHlp, "TableFormula"),
        aValueHelper(rImport, rHlp, false, true, false, true),
        bIsShowFormula(false)
{
}

void XMLTableFormulaImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_FORMULA):
            aValueHelper.ProcessAttribute( nAttrToken, sAttrValue );
            bValid = true;  // we need a formula!
            break;

        case XML_ELEMENT(STYLE, XML_DATA_STYLE_NAME):
            aValueHelper.ProcessAttribute( nAttrToken, sAttrValue );
            break;
        case XML_ELEMENT(TEXT, XML_DISPLAY):
            if ( sAttrValue == "formula" )
                 bIsShowFormula = true;
            break;
        default:
            // unknown attribute -> ignore
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
            break;
    }
}

void XMLTableFormulaImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    // set format and formula
    aValueHelper.PrepareField( xPropertySet );

    Any aAny;

    // set 'show formula' and presentation
    xPropertySet->setPropertyValue( "IsShowFormula", Any(bIsShowFormula) );

    aAny <<= GetContent();
    xPropertySet->setPropertyValue( "CurrentPresentation", aAny );
}


// variable declarations

// Should be adapted to XMLVarField-/XMLSetVarFieldImportContext scheme!


// declaration container import (<variable/user-field/sequence-decls>)


XMLVariableDeclsImportContext::XMLVariableDeclsImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, enum VarType eVarType) :
        SvXMLImportContext(rImport),
        eVarDeclsContextType(eVarType),
        rImportHelper(rHlp)
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLVariableDeclsImportContext::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( IsTokenInNamespace(nElement, XML_NAMESPACE_TEXT) )
    {
        enum XMLTokenEnum eElementName;
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

        if( nElement == XML_ELEMENT(TEXT, eElementName) )
        {
            return new XMLVariableDeclImportContext(
                GetImport(), rImportHelper, nElement, xAttrList,
                eVarDeclsContextType);
        }
    }

    // if no context was created, use default context
    return nullptr;
}


// declaration import (<variable/user-field/sequence-decl> elements)


XMLVariableDeclImportContext::XMLVariableDeclImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_Int32 nElement,
    const Reference<xml::sax::XFastAttributeList> & xAttrList,
    enum VarType eVarType) :
        SvXMLImportContext(rImport)
{
    // bug?? which properties for userfield/userfieldmaster
    XMLValueImportHelper aValueHelper(rImport, rHlp, true, false, true, false);
    sal_Unicode cSeparationChar('.');

    sal_Int8 nNumLevel(-1);
    OUString sName;

    if (nElement != XML_ELEMENT(TEXT, XML_SEQUENCE_DECL) &&
        nElement != XML_ELEMENT(TEXT, XML_VARIABLE_DECL) &&
        nElement != XML_ELEMENT(TEXT, XML_USER_FIELD_DECL) )
        return;

    // TODO: check validity (need name!)

    // parse attributes
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(TEXT, XML_NAME):
                sName = aIter.toString();
                break;
            case XML_ELEMENT(TEXT, XML_DISPLAY_OUTLINE_LEVEL):
            {
                sal_Int32 nLevel;
                bool const bRet = ::sax::Converter::convertNumber(
                    nLevel, aIter.toView(), 0,
                    GetImport().GetTextImport()->GetChapterNumbering()->
                                                               getCount());
                if (bRet)
                {
                    nNumLevel = static_cast< sal_Int8 >( nLevel-1 ); // API numbers -1..9
                }
                break;
            }
            case XML_ELEMENT(TEXT, XML_SEPARATION_CHARACTER):
                cSeparationChar =
                    static_cast<char>(aIter.toString().toChar());
                break;

            default:
                // delegate to value helper
                aValueHelper.ProcessAttribute(aIter.getToken(), aIter.toView());
                break;
        }
    }

    Reference<XPropertySet> xFieldMaster;
    if (!FindFieldMaster(xFieldMaster, GetImport(), rHlp,
                        sName, eVarType))
        return;

    // now we have a field master: process attributes!
    Any aAny;

    switch (eVarType)
    {
    case VarTypeSequence:
        xFieldMaster->setPropertyValue("ChapterNumberingLevel", Any(nNumLevel));

        if (nNumLevel >= 0)
        {
            OUString sStr(&cSeparationChar, 1);
            xFieldMaster->setPropertyValue(
                "NumberingSeparator", Any(sStr));
        }
        break;
    case VarTypeSimple:
        {
            // set string or non-string SubType (#93192#)
            // The SubType was already set in the FindFieldMaster
            // method, but it needs to be adjusted if it's a string.
            aAny <<= aValueHelper.IsStringValue()
                ? SetVariableType::STRING : SetVariableType::VAR;
            xFieldMaster->setPropertyValue(sAPI_sub_type, aAny);
        }
        break;
    case VarTypeUserField:
    {
        bool bTmp = !aValueHelper.IsStringValue();
        xFieldMaster->setPropertyValue("IsExpression", Any(bTmp));
        aValueHelper.PrepareField(xFieldMaster);
        break;
    }
    default:
        OSL_FAIL("unknown varfield type");
    } // switch
}


bool XMLVariableDeclImportContext::FindFieldMaster(
    Reference<XPropertySet> & xMaster, SvXMLImport& rImport,
    XMLTextImportHelper& rImportHelper,
    const OUString& sVarName, enum VarType eVarType)
{
    static sal_Int32 nCollisionCount = 0;

    // rename field
    // currently: no family in use! Use 0.
    OUString sName = rImportHelper.GetRenameMap().Get(
        sal::static_int_cast< sal_uInt16 >(eVarType), sVarName);

    // get text fields supplier and field masters
    Reference<XTextFieldsSupplier> xTextFieldsSupp(rImport.GetModel(),
                                                   UNO_QUERY);
    Reference<container::XNameAccess> xFieldMasterNameAccess =
        xTextFieldsSupp->getTextFieldMasters();

    OUString sVarServiceName =
        OUString::Concat(sAPI_fieldmaster_prefix) +
        sAPI_set_expression +
        "." +
        sName;

    OUString sUserServiceName =
        OUString::Concat(sAPI_fieldmaster_prefix) +
        sAPI_user +
        "." +
        sName;

    if (xFieldMasterNameAccess->hasByName(sVarServiceName)) {
        // variable field master already in document

        Any aAny = xFieldMasterNameAccess->getByName(sVarServiceName);
        aAny >>= xMaster;

        aAny = xMaster->getPropertyValue(sAPI_sub_type);
        sal_Int16 nType = 0;
        aAny >>= nType;

        enum VarType eFMVarType =
            (SetVariableType::SEQUENCE == nType) ?
                VarTypeSequence : VarTypeSimple;

        if (eFMVarType != eVarType)
        {
            ++nCollisionCount;
            OUString sNew(sName + "_renamed_" + OUString::number(nCollisionCount));

            // FIXME! can't find if name is taken already!!!!

            rImportHelper.GetRenameMap().Add(
                sal::static_int_cast< sal_uInt16 >(eVarType), sName, sNew);

            // call FindFieldMaster recursively to create new master
            return FindFieldMaster(xMaster, rImport, rImportHelper,
                                   sNew, eVarType);
        }
    } else if (xFieldMasterNameAccess->hasByName(sUserServiceName)) {
        // user field: get field master
        Any aAny = xFieldMasterNameAccess->getByName(sUserServiceName);
        aAny >>= xMaster;

        if (VarTypeUserField != eVarType) {
            ++nCollisionCount;
            // find new name that is not taken
            OUString sNew(sName + "_renamed_" + OUString::number(nCollisionCount));

            // FIXME! can't find if name is taken already!!!!

            rImportHelper.GetRenameMap().Add(
                sal::static_int_cast< sal_uInt16 >(eVarType), sName, sNew);

            // call FindFieldMaster recursively to create new master
            return FindFieldMaster(xMaster, rImport, rImportHelper,
                                   sNew, eVarType);
        }
    } else {
        // field name not used: create field master

        // import -> model is MultiServiceFactory -> createInstance
        Reference<lang::XMultiServiceFactory>
            xFactory(rImport.GetModel(),UNO_QUERY);
        if( xFactory.is() ) {

            OUString sService = sAPI_fieldmaster_prefix
                + ((eVarType==VarTypeUserField) ?
                                 sAPI_user : sAPI_set_expression);
            Reference<XInterface> xIfc =
                xFactory->createInstance( sService );
            if (xIfc.is()) {
                xMaster.set(xIfc, UNO_QUERY);

                // set name
                xMaster->setPropertyValue("Name", Any(sName));

                if (eVarType != VarTypeUserField) {
                    // set subtype for setexp field
                    Any aAny;
                    aAny <<= ((eVarType == VarTypeSimple) ?
                              SetVariableType::VAR :
                              SetVariableType::SEQUENCE);
                    xMaster->setPropertyValue(sAPI_sub_type, aAny);
                } // else : user field: no subtype

            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    DBG_ASSERT(xMaster.is(), "no field master found!?!");
    return true;
}


// Database Display field import


XMLDatabaseDisplayImportContext::XMLDatabaseDisplayImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLDatabaseFieldImportContext(rImport, rHlp, sAPI_database, false),
        aValueHelper(rImport, rHlp, false, true, false, false),
        bColumnOK(false),
        bDisplay( true ),
        bDisplayOK( false )
{
}

void XMLDatabaseDisplayImportContext::ProcessAttribute(
    sal_Int32 nAttrToken, std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_COLUMN_NAME):
            sColumnName = OUString::fromUtf8(sAttrValue);
            bColumnOK = true;
            break;
        case XML_ELEMENT(TEXT, XML_DISPLAY):
            {
                bool bNone = IsXMLToken( sAttrValue, XML_NONE );
                bool bValue = IsXMLToken( sAttrValue, XML_VALUE );
                bDisplay = bValue;
                bDisplayOK = bNone || bValue;
            }
            break;
        case XML_ELEMENT(TEXT, XML_DATABASE_NAME):
        case XML_ELEMENT(TEXT, XML_TABLE_NAME):
        case XML_ELEMENT(TEXT, XML_TABLE_TYPE):
            // handled by super class
            XMLDatabaseFieldImportContext::ProcessAttribute(nAttrToken,
                                                            sAttrValue);
            break;
        default:
            // remainder handled by value helper
            aValueHelper.ProcessAttribute(nAttrToken, sAttrValue);
            break;
    }

    bValid = m_bTableOK && m_bDatabaseOK && bColumnOK;
}

void XMLDatabaseDisplayImportContext::endFastElement(sal_Int32 )
{
    // we have an EndElement of our own, because database fields need
    // to be attached to a field master before they can be inserted into
    // the document. Database stuff (database, table, column) all goes
    // to the field master, value & style go to the field.

    if (bValid)
    {

        // so here goes: we start with the master
        Reference<XPropertySet> xMaster;

        // create and prepare field master first
        if (CreateField(xMaster,
                        "com.sun.star.text.FieldMaster.Database"))
        {
            Any aAny;
            xMaster->setPropertyValue("DataColumnName", Any(sColumnName));

            // fieldmaster takes database, table and column name
            XMLDatabaseFieldImportContext::PrepareField(xMaster);

            // create field
            Reference<XPropertySet> xField;
            if (CreateField(xField,
                            sAPI_database))
            {
                // attach field master
                Reference<XDependentTextField> xDepField(xField, UNO_QUERY);
                if (xDepField.is())
                {
                    // attach field to field master
                    xDepField->attachTextFieldMaster(xMaster);

                    // attach field to document
                    Reference<XTextContent> xTextContent(xField, UNO_QUERY);
                    if (xTextContent.is())
                    {
                        // insert, set field properties and exit!
                        try
                        {
                            GetImportHelper().InsertTextContent(xTextContent);

                            // prepare field: format from database?
                            bool bTmp = !aValueHelper.IsFormatOK();
                            xField->setPropertyValue("DataBaseFormat", Any(bTmp));

                            // value, value-type and format done by value helper
                            aValueHelper.PrepareField(xField);

                            // visibility
                            if( bDisplayOK )
                            {
                                xField->setPropertyValue(sAPI_is_visible, Any(bDisplay));
                            }

                            // set presentation
                            aAny <<= GetContent();
                            xField->setPropertyValue(sAPI_current_presentation, aAny);

                            // success!
                            return;
                        }
                        catch (const lang::IllegalArgumentException&)
                        {
                            TOOLS_WARN_EXCEPTION("xmloff.text", "Failed to insert text content");
                        }
                    }
                }
            }
        }
    }

    // above: exit on success; so for all error cases we end up here!
    // write element content
    GetImportHelper().InsertString(GetContent());
}


// value import helper

namespace {

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

}

SvXMLEnumMapEntry<ValueType> const aValueTypeMap[] =
{
    { XML_FLOAT,        XML_VALUE_TYPE_FLOAT },
    { XML_CURRENCY,     XML_VALUE_TYPE_CURRENCY },
    { XML_PERCENTAGE,   XML_VALUE_TYPE_PERCENTAGE },
    { XML_DATE,         XML_VALUE_TYPE_DATE },
    { XML_TIME,         XML_VALUE_TYPE_TIME },
    { XML_BOOLEAN,      XML_VALUE_TYPE_BOOLEAN },
    { XML_STRING,       XML_VALUE_TYPE_STRING },
    { XML_TOKEN_INVALID, ValueType(0) }
};

XMLValueImportHelper::XMLValueImportHelper(
    SvXMLImport& rImprt,
    XMLTextImportHelper& rHlp,
    bool bType, bool bStyle, bool bValue, bool bFormula) :

        rImport(rImprt),
        rHelper(rHlp),

        fValue(0.0),
        nFormatKey(0),
        bIsDefaultLanguage(true),

        bStringType(false),
        bFormatOK(false),
        bStringValueOK(false),
        bFormulaOK(false),

        bSetType(bType),
        bSetValue(bValue),
        bSetStyle(bStyle),
        bSetFormula(bFormula)
{
}

void XMLValueImportHelper::ProcessAttribute(
    sal_Int32 nAttrToken, std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_VALUE_TYPE): // #i32362#: src680m48++ saves text:value-type
        case XML_ELEMENT(OFFICE, XML_VALUE_TYPE):
        {
            // convert enum
            ValueType eValueType = XML_VALUE_TYPE_STRING;
            bool bRet = SvXMLUnitConverter::convertEnum(
                eValueType, sAttrValue, aValueTypeMap);

            if (bRet) {
                switch (eValueType)
                {
                    case XML_VALUE_TYPE_STRING:
                        bStringType = true;
                        break;
                    case XML_VALUE_TYPE_FLOAT:
                    case XML_VALUE_TYPE_CURRENCY:
                    case XML_VALUE_TYPE_PERCENTAGE:
                    case XML_VALUE_TYPE_DATE:
                    case XML_VALUE_TYPE_TIME:
                    case XML_VALUE_TYPE_BOOLEAN:
                        bStringType = false;
                        break;

                    default:
                        OSL_FAIL("unknown value type");
                }
            }
            break;
        }

        case XML_ELEMENT(TEXT, XML_VALUE):
        case XML_ELEMENT(OFFICE, XML_VALUE):
        {
            double fTmp;
            bool const bRet = ::sax::Converter::convertDouble(fTmp,sAttrValue);
            if (bRet) {
                fValue = fTmp;
            }
            break;
        }

        case XML_ELEMENT(TEXT, XML_TIME_VALUE):
        case XML_ELEMENT(OFFICE, XML_TIME_VALUE):
        {
            double fTmp;
            bool const bRet =
                ::sax::Converter::convertDuration(fTmp, sAttrValue);
            if (bRet) {
                fValue = fTmp;
            }
            break;
        }

        case XML_ELEMENT(TEXT, XML_DATE_VALUE):
        case XML_ELEMENT(OFFICE, XML_DATE_VALUE):
        {
            double fTmp;
            bool bRet = rImport.GetMM100UnitConverter().
                convertDateTime(fTmp,sAttrValue);
            if (bRet) {
                fValue = fTmp;
            }
            break;
        }

        case XML_ELEMENT(OFFICE, XML_BOOLEAN_VALUE):
        {
            bool bTmp(false);
            bool bRet = ::sax::Converter::convertBool(bTmp, sAttrValue);
            if (bRet) {
                fValue = (bTmp ? 1.0 : 0.0);
            }
            else
            {
                double fTmp;
                bRet = ::sax::Converter::convertDouble(fTmp, sAttrValue);
                if (bRet) {
                    fValue = fTmp;
                }
            }
            break;
        }

        case XML_ELEMENT(TEXT, XML_STRING_VALUE):
        case XML_ELEMENT(OFFICE, XML_STRING_VALUE):
            sValue = OUString::fromUtf8(sAttrValue);
            bStringValueOK = true;
            break;

        case  XML_ELEMENT(TEXT, XML_FORMULA):
            {
                OUString sTmp;
                sal_uInt16 nPrefix = rImport.GetNamespaceMap().
                        GetKeyByAttrValueQName(OUString::fromUtf8(sAttrValue), &sTmp);
                if( XML_NAMESPACE_OOOW == nPrefix )
                {
                    sFormula = sTmp;
                    bFormulaOK = true;
                }
                else
                    sFormula = OUString::fromUtf8(sAttrValue);
            }
            break;

        case XML_ELEMENT(STYLE, XML_DATA_STYLE_NAME):
        {
            sal_Int32 nKey = rHelper.GetDataStyleKey(
                                          OUString::fromUtf8(sAttrValue), &bIsDefaultLanguage);
            if (-1 != nKey)
            {
                nFormatKey = nKey;
                bFormatOK = true;
            }
            break;
        }
        default:
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
    } // switch
}

void XMLValueImportHelper::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    if (bSetType)
    {
        // ??? how to set type?
    }

    if (bSetFormula)
    {
        aAny <<= !bFormulaOK ? sDefault : sFormula;
        xPropertySet->setPropertyValue(sAPI_content, aAny);
    }

    // format/style
    if (bSetStyle && bFormatOK)
    {
        xPropertySet->setPropertyValue(sAPI_number_format, Any(nFormatKey));

        if( xPropertySet->getPropertySetInfo()->
                hasPropertyByName( "IsFixedLanguage" ) )
        {
            bool bIsFixedLanguage = ! bIsDefaultLanguage;
            xPropertySet->setPropertyValue( "IsFixedLanguage", Any(bIsFixedLanguage) );
        }
    }

    // value: string or float
    if (bSetValue)
    {
        if (bStringType)
        {
            aAny <<= !bStringValueOK ? sDefault : sValue;
            xPropertySet->setPropertyValue(sAPI_content, aAny);
        }
        else
        {
            xPropertySet->setPropertyValue("Value", Any(fValue));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
