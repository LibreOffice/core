/*************************************************************************
 *
 *  $RCSfile: txtvfldi.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: dvo $ $Date: 2001-03-30 12:28:15 $
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

/** @#file
 *
 *  export of all variable related text fields (and database display field)
 */

#ifndef _XMLOFF_TXTVFLDI_HXX
#include "txtvfldi.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_TEXTIMP_HXX_
#include "txtimp.hxx"
#endif

#ifndef _XMLOFF_XMLNUMI_HXX
#include "xmlnumi.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_I18NMAP_HXX
#include "i18nmap.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlement.hxx"
#endif

#ifndef _COM_SUN_STAR_TEXT_SETVARIABLETYPE_HPP_
#include <com/sun/star/text/SetVariableType.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELD_HPP_
#include <com/sun/star/text/XTextField.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XDEPENDENTTEXTFIELD_HPP_
#include <com/sun/star/text/XDependentTextField.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELDSSUPPLIER_HPP_
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _RTL_USTRING
#include <rtl/ustring>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


// service names
static const sal_Char sAPI_textfield_prefix[]   = "com.sun.star.text.TextField.";
static const sal_Char sAPI_fieldmaster_prefix[] = "com.sun.star.text.FieldMaster.";
static const sal_Char sAPI_input[]              = "Input";
static const sal_Char sAPI_input_user[]         = "InputUser";
static const sal_Char sAPI_get_expression[]     = "GetExpression";
static const sal_Char sAPI_set_expression[]     = "SetExpression";
static const sal_Char sAPI_user[]               = "User";
static const sal_Char sAPI_database[]           = "com.sun.star.text.TextField.Database";
static const sal_Char sAPI_fieldmaster_database[] = "com.sun.star.text.FieldMaster.Database";

// property names
static const sal_Char sAPI_hint[]               = "Hint";
static const sal_Char sAPI_content[]            = "Content";
static const sal_Char sAPI_sub_type[]           = "SubType";
static const sal_Char sAPI_is_expression[]      = "IsExpression";
static const sal_Char sAPI_is_input[]           = "Input";
static const sal_Char sAPI_is_show_formula[]    = "IsShowFormula";
static const sal_Char sAPI_numbering_type[]     = "NumberingType";
static const sal_Char sAPI_number_format[]      = "NumberFormat";
static const sal_Char sAPI_name[]               = "Name";
static const sal_Char sAPI_numbering_separator[]    = "NumberingSeparator";
static const sal_Char sAPI_chapter_numbering_level[]= "ChapterNumberingLevel";
static const sal_Char sAPI_value[]              = "Value";
static const sal_Char sAPI_is_visible[]         = "IsVisible";
static const sal_Char sAPI_variable_subtype[]   = "VariableSubtype";
static const sal_Char sAPI_data_column_name[]   = "DataColumnName";
static const sal_Char sAPI_is_data_base_format[]    = "DataBaseFormat";
static const sal_Char sAPI_current_presentation[]   = "CurrentPresentation";
static const sal_Char sAPI_sequence_value[]     = "SequenceValue";


using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::text;
using namespace com::sun::star::style;



//
// XMLVarFieldImportContext: superclass for all variable related fields
//

TYPEINIT1( XMLVarFieldImportContext, XMLTextFieldImportContext );

XMLVarFieldImportContext::XMLVarFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    const sal_Char* pServiceName, sal_uInt16 nPrfx,
    const OUString& rLocalName,
    sal_Bool bName, sal_Bool bFormula, sal_Bool bFormulaDefault,
    sal_Bool bDescription, sal_Bool bVisible, sal_Bool bDisplayFormula,
    sal_Bool bType, sal_Bool bStyle, sal_Bool bValue,
    sal_Bool bPresentation) :
        XMLTextFieldImportContext(rImport, rHlp, pServiceName,
                                  nPrfx, rLocalName),
        sName(),
        sFormula(),
        sDescription(),
        aValueHelper(rImport, rHlp, bType, bStyle, bValue, sal_False),
        bDisplayFormula(sal_False),
        bDisplayNone(sal_False),
        bNameOK(sal_False),
        bFormulaOK(sal_False),
        bDescriptionOK(sal_False),
        bDisplayOK(sal_False),
        bSetName(bName),
        bSetFormula(bFormula),
        bSetFormulaDefault(bFormulaDefault),
        bSetDescription(bDescription),
        bSetVisible(bVisible),
        bSetDisplayFormula(bDisplayFormula),
        bSetPresentation(bPresentation),
        sPropertyContent(RTL_CONSTASCII_USTRINGPARAM(sAPI_content)),
        sPropertyHint(RTL_CONSTASCII_USTRINGPARAM(sAPI_hint)),
        sPropertyIsVisible(RTL_CONSTASCII_USTRINGPARAM(sAPI_is_visible)),
        sPropertyIsDisplayFormula(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_is_show_formula)),
        sPropertyCurrentPresentation(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_current_presentation))
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
            bValid = sal_True;      // we assume: field with name is valid!
            break;
        case XML_TOK_TEXTFIELD_DESCRIPTION:
            sDescription = sAttrValue;
            bDescriptionOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_FORMULA:
            sFormula = sAttrValue;
            bFormulaOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_DISPLAY:
            if (0 == sAttrValue.compareToAscii(sXML_formula))
            {
                bDisplayFormula = sal_True;
                bDisplayNone = sal_False;
                bDisplayOK = sal_True;
            }
            else if (0 == sAttrValue.compareToAscii(sXML_value))
            {
                bDisplayFormula = sal_False;
                bDisplayNone = sal_False;
                bDisplayOK = sal_True;
            }
            else if (0 == sAttrValue.compareToAscii(sXML_none))
            {
                bDisplayFormula = sal_False;
                bDisplayNone = sal_True;
                bDisplayOK = sal_True;
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

    if (bSetVisible && bDisplayOK)
    {
        Any aAny;
        sal_Bool bTmp = ! (bDisplayNone && bDisplayOK);
        aAny.setValue( &bTmp, ::getBooleanCppuType());
        xPropertySet->setPropertyValue(sPropertyIsVisible, aAny);
    }

    // workaround for #no-bug#: display formula by default
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

    // delegate to value helper
    aValueHelper.SetDefault(GetContent());
    aValueHelper.PrepareField(xPropertySet);

    // finally, set the curren presentation
    if (bSetPresentation)
    {
        Any aAny;
        aAny <<= GetContent();
        xPropertySet->setPropertyValue(sPropertyCurrentPresentation, aAny);
    }
}





//
// variable set fields
//

TYPEINIT1( XMLSetVarFieldImportContext, XMLVarFieldImportContext );

XMLSetVarFieldImportContext::XMLSetVarFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    const sal_Char* pServiceName, sal_uInt16 nPrfx,
    const OUString& rLocalName, VarType eVarType,
    sal_Bool bName, sal_Bool bFormula, sal_Bool bFormulaDefault,
    sal_Bool bDescription, sal_Bool bVisible, sal_Bool bDisplayFormula,
    sal_Bool bType, sal_Bool bStyle, sal_Bool bValue, sal_Bool bPresentation) :
        XMLVarFieldImportContext(rImport, rHlp, pServiceName,
                                 nPrfx, rLocalName,
                                 bName, bFormula, bFormulaDefault,
                                 bDescription, bVisible, bDisplayFormula,
                                 bType, bStyle, bValue, bPresentation),
        eFieldType(eVarType)
{
}

void XMLSetVarFieldImportContext::EndElement()
{
    // should we call PrepareField on the field, or rather on it's master?
    // currently: call on field (just like superclass)
    // possible alternatives: call on master
    //                        call field or master depending on variable
    //                        PrepareMaster() in addition to PrepareField()

    DBG_ASSERT(GetServiceName().getLength()>0, "no service name for element!");

    if (bValid)
    {
        DBG_ASSERT(GetName().getLength()>0, "variable name needed!");

        // find field master
        Reference<XPropertySet> xMaster;
        if (FindFieldMaster(xMaster))
        {
            // create field/Service
            Reference<XPropertySet> xPropSet;
            if (CreateField(xPropSet, OUString::createFromAscii(sAPI_textfield_prefix) + GetServiceName()))
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
                        // insert, set field properties and exit!
                        GetImportHelper().InsertTextContent(xTextContent);
                        PrepareField(xPropSet);
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

sal_Bool XMLSetVarFieldImportContext::FindFieldMaster(
    Reference<XPropertySet> & xMaster)
{
    // currently: delegate to XMLVariableDeclImportContext;
    // shoud eventually go here
    return XMLVariableDeclImportContext::FindFieldMaster(xMaster,
                                                         GetImport(),
                                                         GetImportHelper(),
                                                         GetName(),
                                                         eFieldType);
}



//
// sequence field
//

TYPEINIT1( XMLSequenceFieldImportContext, XMLSetVarFieldImportContext );

XMLSequenceFieldImportContext::XMLSequenceFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& rLocalName) :
        XMLSetVarFieldImportContext(rImport, rHlp, sAPI_set_expression,
                                    nPrfx, rLocalName, VarTypeSequence,
                                    // name, formula
                                    sal_True, sal_True, sal_True,
                                    sal_False, sal_False, sal_False,
                                    sal_False, sal_False, sal_False, sal_True),
        sNumFormat(sal_Unicode('1')),
        sNumFormatSync(RTL_CONSTASCII_USTRINGPARAM(sXML_false)),
        sRefName(),
        bRefNameOK(sal_False),
        sPropertyNumberFormat(RTL_CONSTASCII_USTRINGPARAM(sAPI_number_format)),
        sPropertySequenceValue(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_sequence_value))
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
    sal_Int16 nNumType = NumberingType::ARABIC;nNumType;
    GetImport().GetMM100UnitConverter().convertNumFormat( nNumType, sNumFormat, sNumFormatSync );
    Any aAny;
    aAny <<= nNumType;
    xPropertySet->setPropertyValue(sPropertyNumberFormat, aAny);

    // handle reference name
    if (bRefNameOK)
    {
        aAny = xPropertySet->getPropertyValue(sPropertySequenceValue);
        sal_Int16 nValue;
        aAny >>= nValue;
        GetImportHelper().InsertSequenceID(sRefName, GetName(), nValue);
    }
}



//
// variable set field
//

TYPEINIT1( XMLVariableSetFieldImportContext, XMLSetVarFieldImportContext );

XMLVariableSetFieldImportContext::XMLVariableSetFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& rLocalName) :
        XMLSetVarFieldImportContext(rImport, rHlp, sAPI_set_expression,
                                    nPrfx, rLocalName, VarTypeSimple,
                                    // name, formula, value&type, style,
                                    // display none
                                    sal_True, sal_True, sal_True,
                                    sal_False, sal_True, sal_False,
                                    sal_True, sal_True, sal_True,
                                    sal_True),
        sPropertySubType(RTL_CONSTASCII_USTRINGPARAM(sAPI_sub_type))
{
}

void XMLVariableSetFieldImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    // set type
    Any aAny;
    aAny <<= (IsStringValue()? SetVariableType::STRING : SetVariableType::VAR);
    xPropertySet->setPropertyValue(sPropertySubType, aAny);

    // the remainder is handled by super class
    XMLSetVarFieldImportContext::PrepareField(xPropertySet);
}



//
// variable input field
//

TYPEINIT1( XMLVariableInputFieldImportContext, XMLSetVarFieldImportContext );

XMLVariableInputFieldImportContext::XMLVariableInputFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& rLocalName) :
        XMLSetVarFieldImportContext(rImport, rHlp, sAPI_set_expression,
                                    nPrfx, rLocalName, VarTypeSimple,
                                    // name, description, display none/formula,
                                    // value&type, style, formula
                                    sal_True, sal_True, sal_True,
                                    sal_True, sal_True, sal_False,
                                    sal_True, sal_True, sal_True,
                                    sal_True),
        sPropertySubType(RTL_CONSTASCII_USTRINGPARAM(sAPI_sub_type)),
        sPropertyIsInput(RTL_CONSTASCII_USTRINGPARAM(sAPI_is_input))
{
}

void XMLVariableInputFieldImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    // set type (input field)
    Any aAny;
    sal_Bool bTrue = sal_True;
    aAny.setValue( &bTrue, ::getBooleanCppuType() );
    xPropertySet->setPropertyValue(sPropertyIsInput, aAny);

    // set type
    aAny <<= (IsStringValue()? SetVariableType::STRING : SetVariableType::VAR);
    xPropertySet->setPropertyValue(sPropertySubType, aAny);

    // the remainder is handled by super class
    XMLSetVarFieldImportContext::PrepareField(xPropertySet);
}



//
// user field
//

TYPEINIT1( XMLUserFieldImportContext, XMLSetVarFieldImportContext );

XMLUserFieldImportContext::XMLUserFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& rLocalName) :
        XMLSetVarFieldImportContext(rImport, rHlp, sAPI_user, nPrfx,
                                    rLocalName, VarTypeUserField,
                                    // name, display none/formula, style
                                    sal_True, sal_False, sal_False,
                                    sal_False, sal_True, sal_True,
                                    sal_False, sal_True, sal_False,
                                    sal_False)
{
}



//
// user input field
//

TYPEINIT1( XMLUserFieldInputImportContext, XMLVarFieldImportContext );

// bug: doesn't work (SO API lacking)
XMLUserFieldInputImportContext::XMLUserFieldInputImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& rLocalName) :
        XMLVarFieldImportContext(rImport, rHlp, sAPI_input_user,
                                 nPrfx, rLocalName,
                                 // name, description, style
                                 sal_True, sal_False, sal_False,
                                 sal_True, sal_False, sal_False,
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

    // delegate to super class
    XMLVarFieldImportContext::PrepareField(xPropertySet);
}


//
// variable get field
//

TYPEINIT1( XMLVariableGetFieldImportContext, XMLVarFieldImportContext );

XMLVariableGetFieldImportContext::XMLVariableGetFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& rLocalName) :
        XMLVarFieldImportContext(rImport, rHlp, sAPI_get_expression,
                                 nPrfx, rLocalName,
                                 // name, style, display formula
                                 sal_True, sal_False, sal_False,
                                 sal_False, sal_False, sal_True,
                                 sal_True, sal_True, sal_False,
                                 sal_True),
        sPropertySubType(RTL_CONSTASCII_USTRINGPARAM(sAPI_sub_type))
{
}

void XMLVariableGetFieldImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    // set type; IsStringValue() defaults to false
      Any aAny;
    aAny <<= (IsStringValue() ? SetVariableType::STRING : SetVariableType::VAR);
    xPropertySet->setPropertyValue(sPropertySubType, aAny);

    // set name
    aAny <<= GetName();
    xPropertySet->setPropertyValue(sPropertyContent, aAny);

    // the remainder is handled by super class
    XMLVarFieldImportContext::PrepareField(xPropertySet);
}



//
// expression field
//

TYPEINIT1( XMLExpressionFieldImportContext, XMLVarFieldImportContext );

XMLExpressionFieldImportContext::XMLExpressionFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& rLocalName) :
        XMLVarFieldImportContext(rImport, rHlp, sAPI_get_expression,
                                 nPrfx, rLocalName,
                                 // formula, type, style, display formula
                                 sal_False, sal_True, sal_True,
                                 sal_False, sal_False, sal_True,
                                 sal_True, sal_False, sal_False,
                                 sal_True),
        sPropertySubType(RTL_CONSTASCII_USTRINGPARAM(sAPI_sub_type))
{
    bValid = sal_True;  // always valid
}


void XMLExpressionFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    sal_Int16 nSubType = SetVariableType::FORMULA;
    Any aAny;
    aAny <<= nSubType;
    xPropertySet->setPropertyValue(sPropertySubType, aAny);

    // delegate to super class
    XMLVarFieldImportContext::PrepareField(xPropertySet);
}



//
// text input field
//

TYPEINIT1( XMLTextInputFieldImportContext, XMLVarFieldImportContext );

XMLTextInputFieldImportContext::XMLTextInputFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLVarFieldImportContext(rImport, rHlp, sAPI_input,
                                 nPrfx, sLocalName,
                                 // description
                                 sal_False, sal_False, sal_False,
                                 sal_True, sal_False, sal_False,
                                 sal_False, sal_False, sal_False,
                                 sal_False),
        sPropertyContent(RTL_CONSTASCII_USTRINGPARAM(sAPI_content))
{
    bValid = sal_True;  // always valid
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
// variable declarations
//
// Should be adapted to XMLVarField-/XMLSetVarFieldImportContext scheme!
//



//
// declaration containter import (<variable/user-field/sequence-decls>)
//

TYPEINIT1( XMLVariableDeclsImportContext, SvXMLImportContext );

XMLVariableDeclsImportContext::XMLVariableDeclsImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& rLocalName, enum VarType eVarType) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        rImportHelper(rHlp),
        eVarDeclsContextType(eVarType)
{
}

SvXMLImportContext* XMLVariableDeclsImportContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName,
    const Reference<xml::sax::XAttributeList> & xAttrList )
{
    sal_Char* sElementName;
    SvXMLImportContext* pImportContext = NULL;

    if (XML_NAMESPACE_TEXT == nPrefix) {
        switch (eVarDeclsContextType) {
        case VarTypeSequence:
            sElementName = sXML_sequence_decl;
            break;
        case VarTypeSimple:
            sElementName = sXML_variable_decl;
            break;
        case VarTypeUserField:
            sElementName = sXML_user_field_decl;
            break;
        default:
            DBG_ERROR("unknown field type!");
        }

        if (0 == rLocalName.compareToAscii(sElementName)) {
            pImportContext = new XMLVariableDeclImportContext(
                GetImport(), rImportHelper, nPrefix, rLocalName, xAttrList,
                eVarDeclsContextType);
        }
    }

    // if no context was created, use default context
    if (NULL == pImportContext) {
        pImportContext = SvXMLImportContext::CreateChildContext(nPrefix,
                                                                rLocalName,
                                                                xAttrList);
    }

    return pImportContext;
}



//
// declaration import (<variable/user-field/sequence-decl> elements)
//

TYPEINIT1( XMLVariableDeclImportContext, SvXMLImportContext );

XMLVariableDeclImportContext::XMLVariableDeclImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& rLocalName,
    const Reference<xml::sax::XAttributeList> & xAttrList,
    enum VarType eVarType) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        nNumLevel(-1), cSeparationChar('.'),
        // bug?? which properties for userfield/userfieldmaster
        aValueHelper(rImport, rHlp, sal_True, sal_False, sal_True, sal_False),
        sPropertyName(RTL_CONSTASCII_USTRINGPARAM(sAPI_name)),
        sPropertySubType(RTL_CONSTASCII_USTRINGPARAM(sAPI_sub_type)),
        sPropertyNumberingLevel(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_chapter_numbering_level)),
        sPropertyNumberingSeparator(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_numbering_separator)),
        sPropertyIsExpression(RTL_CONSTASCII_USTRINGPARAM(sAPI_is_expression))
{
    if ( (XML_NAMESPACE_TEXT == nPrfx) &&
         ( (0 == rLocalName.compareToAscii(sXML_sequence_decl)) ||
           (0 == rLocalName.compareToAscii(sXML_variable_decl)) ||
           (0 == rLocalName.compareToAscii(sXML_user_field_decl))    )) {

        // TODO: check validity (need name!)

        // parse attributes
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
                    sal_Bool bRet = SvXMLUnitConverter::convertNumber(
                        nLevel, xAttrList->getValueByIndex(i), 0,
                        GetImport().GetTextImport()->GetChapterNumbering()->
                                                                   getCount());
                    if (bRet)
                    {
                        nNumLevel = nLevel-1; // API numbers -1..9
                    }
                    break;
                }
                case XML_TOK_TEXTFIELD_NUMBERING_SEPARATOR:
                    cSeparationChar =
                        (sal_Char)xAttrList->getValueByIndex(i).toChar();
                    break;

                default:
                    // delegate to value helper
                    aValueHelper.ProcessAttribute(nToken,
                                                  xAttrList->getValueByIndex(i));
                    break;
            }
        }

        Reference<XPropertySet> xFieldMaster;
        if (FindFieldMaster(xFieldMaster, GetImport(), rHlp,
                            sName, eVarType))
        {
            // now we have a field master: process attributes!
            Any aAny;

            switch (eVarType)
            {
            case VarTypeSequence:
                aAny <<= nNumLevel;
                xFieldMaster->setPropertyValue(sPropertyNumberingLevel, aAny);

                if (nNumLevel >= 0)
                {
                    OUString sStr(cSeparationChar);
                    aAny <<= sStr;
                    xFieldMaster->setPropertyValue(
                        sPropertyNumberingSeparator, aAny);
                }
                break;
            case VarTypeSimple:
                break; // nothing
            case VarTypeUserField:
            {
                sal_Bool bTmp = !aValueHelper.IsStringValue();
                aAny.setValue(&bTmp, ::getBooleanCppuType());
                xFieldMaster->setPropertyValue(sPropertyIsExpression, aAny);
                aValueHelper.PrepareField(xFieldMaster);
                break;
            }
            default:
                DBG_ERROR("unkown varfield type");
            } // switch
        } // else: no field master found/constructed
    } // else: no sequence-decl
}



sal_Bool XMLVariableDeclImportContext::FindFieldMaster(
    Reference<XPropertySet> & xMaster, SvXMLImport& rImport,
    XMLTextImportHelper& rImportHelper,
    const OUString& sVarName, enum VarType eVarType)
{
    static sal_Int32 nCollisionCount = 0;

    // rename field
    // currently: no family in use! Use 0.
    OUString sName = rImportHelper.GetRenameMap().Get(eVarType, sVarName);

    // get text fields supplier and field masters
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
        // variable field master already in document

        Any aAny = xFieldMasterNameAccess->getByName(sVarServiceName);
        aAny >>= xMaster;

        aAny = xMaster->getPropertyValue(
            // sPropertySubType
            OUString::createFromAscii(sAPI_sub_type)
            );
        sal_Int16 nType;
        aAny >>= nType;

        enum VarType eFMVarType =
            (SetVariableType::SEQUENCE == nType) ?
                VarTypeSequence : VarTypeSimple;

        if (eFMVarType != eVarType)
        {
            OUString sNew;

            // FIXME! cant find if name is taken already!!!!

            nCollisionCount++;
            OUStringBuffer aBuf;
            aBuf.append(sName);
            aBuf.appendAscii("_renamed_");
            aBuf.append(nCollisionCount);
            sNew = aBuf.makeStringAndClear();

            rImportHelper.GetRenameMap().Add(eVarType, sName, sNew);

            // call FindFieldMaster recursively to create new master
            return FindFieldMaster(xMaster, rImport, rImportHelper,
                                   sNew, eVarType);
        }
    } else if (xFieldMasterNameAccess->hasByName(sUserServiceName)) {
        // user field: get field master
        Any aAny = xFieldMasterNameAccess->getByName(sUserServiceName);
        aAny >>= xMaster;

        if (VarTypeUserField != eVarType) {
            // find new name that is not taken
            OUString sNew;

            // FIXME! cant find if name is taken already!!!!

            nCollisionCount++;
            OUStringBuffer aBuf;
            aBuf.append(sName);
            aBuf.appendAscii("_renamed_");
            aBuf.append(nCollisionCount);
            sNew = aBuf.makeStringAndClear();

            rImportHelper.GetRenameMap().Add(eVarType, sName, sNew);

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

            OUStringBuffer sService;
            sService.appendAscii(sAPI_fieldmaster_prefix);
            sService.appendAscii((eVarType==VarTypeUserField) ?
                                 sAPI_user : sAPI_set_expression);
            Reference<XInterface> xIfc =
                xFactory->createInstance( sService.makeStringAndClear() );
            if (xIfc.is()) {
                Reference<XPropertySet> xTmp( xIfc, UNO_QUERY );
                xMaster = xTmp;

                // set name
                Any aAny;
                aAny <<= sName;
                xMaster->setPropertyValue(
                    // sPropertyName
                    OUString::createFromAscii(sAPI_name)
                    , aAny);

                if (eVarType != VarTypeUserField) {
                    // set subtype for setexp field

                    aAny <<= ((eVarType == VarTypeSimple) ?
                              SetVariableType::VAR :
                              SetVariableType::SEQUENCE);
                    xMaster->setPropertyValue(
                        // sPropertySubType
                        OUString::createFromAscii(sAPI_sub_type)
                        , aAny);
                } // else : user field: no subtype

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
// Database Display field import
//


TYPEINIT1( XMLDatabaseDisplayImportContext, XMLDatabaseFieldImportContext );

XMLDatabaseDisplayImportContext::XMLDatabaseDisplayImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& rLocalName) :
        XMLDatabaseFieldImportContext(rImport, rHlp, sAPI_database,
                                      nPrfx, rLocalName),
        aValueHelper(rImport, rHlp, sal_False, sal_True, sal_False, sal_False),
        sColumnName(),
        bColumnOK(sal_False),
        sPropertyColumnName(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_data_column_name)),
        sPropertyDatabaseFormat(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_is_data_base_format)),
        sPropertyCurrentPresentation(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_current_presentation))
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
        case XML_TOK_TEXTFIELD_DATABASE_NAME:
        case XML_TOK_TEXTFIELD_TABLE_NAME:
            // handled by super class
            XMLDatabaseFieldImportContext::ProcessAttribute(nAttrToken,
                                                            sAttrValue);
            break;
        default:
            // remainder handled by value helper
            aValueHelper.ProcessAttribute(nAttrToken, sAttrValue);
            break;
    }

    bValid = bTableOK && bDatabaseOK && bColumnOK;
}

void XMLDatabaseDisplayImportContext::EndElement()
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
                        OUString(RTL_CONSTASCII_USTRINGPARAM(
                            sAPI_fieldmaster_database))))
        {
            Any aAny;
            aAny <<= sColumnName;
            xMaster->setPropertyValue(sPropertyColumnName, aAny);

            // fieldmaster takes database, table and column name
            XMLDatabaseFieldImportContext::PrepareField(xMaster);

            // create field
            Reference<XPropertySet> xField;
            if (CreateField(xField,
                            OUString(RTL_CONSTASCII_USTRINGPARAM(
                                sAPI_database))))
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
                        GetImportHelper().InsertTextContent(xTextContent);

                        // prepare field: format from database?
                        Any aAny;
                        sal_Bool bTmp = !aValueHelper.IsFormatOK();
                        aAny.setValue( &bTmp, ::getBooleanCppuType() );
                        xField->setPropertyValue(sPropertyDatabaseFormat,aAny);

                        // value, value-type and format done by value helper
                        aValueHelper.PrepareField(xField);

                        // set presentation
                        aAny <<= GetContent();
                        xField->setPropertyValue(sPropertyCurrentPresentation,
                                                 aAny);

                        // success!
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


//
// value import helper
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

static SvXMLEnumMapEntry __READONLY_DATA aValueTypeMap[] =
{
    { sXML_float,       XML_VALUE_TYPE_FLOAT },
    { sXML_currency,    XML_VALUE_TYPE_CURRENCY },
    { sXML_percentage,  XML_VALUE_TYPE_PERCENTAGE },
    { sXML_date,        XML_VALUE_TYPE_DATE },
    { sXML_time,        XML_VALUE_TYPE_TIME },
    { sXML_boolean,     XML_VALUE_TYPE_BOOLEAN },
    { sXML_string,      XML_VALUE_TYPE_STRING },
    { 0,                0 },
};

XMLValueImportHelper::XMLValueImportHelper(
    SvXMLImport& rImprt,
    XMLTextImportHelper& rHlp,
    sal_Bool bType, sal_Bool bStyle, sal_Bool bValue, sal_Bool bFormula) :
        rImport(rImprt),
        rHelper(rHlp),
        bTypeOK(sal_False),
        bStringValueOK(sal_False),
        bFloatValueOK(sal_False),
        bFormatOK(sal_False),
        bFormulaOK(sal_False),
        bSetType(bType),
        bSetValue(bValue),
        bSetStyle(bStyle),
        bSetFormula(bFormula),
        bStringType(sal_False),
        bStringDefault(sal_True),
        bFormulaDefault(sal_True),
        sValue(),
        fValue(0.0),
        nFormatKey(0),
        sDefault(),
        sPropertyContent(RTL_CONSTASCII_USTRINGPARAM(sAPI_content)),
        sPropertyValue(RTL_CONSTASCII_USTRINGPARAM(sAPI_value)),
        sPropertyNumberFormat(RTL_CONSTASCII_USTRINGPARAM(sAPI_number_format))
{
}

void XMLValueImportHelper::ProcessAttribute(
    sal_uInt16 nAttrToken, const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_VALUE_TYPE:
        {
            // convert enum
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
                        DBG_ERROR("unknown value type");
                        bTypeOK = sal_False;
                }
            }
            break;
        }

        case XML_TOK_TEXTFIELD_VALUE:
        {
            double fTmp;
            sal_Bool bRet = SvXMLUnitConverter::convertDouble(fTmp,sAttrValue);
            if (bRet) {
                bFloatValueOK = sal_True;
                fValue = fTmp;
            }
            break;
        }

        case XML_TOK_TEXTFIELD_TIME_VALUE:
        {
            double fTmp;
            sal_Bool bRet = SvXMLUnitConverter::convertTime(fTmp,sAttrValue);
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
            sal_Bool bTmp;
            sal_Bool bRet = SvXMLUnitConverter::convertBool(bTmp,sAttrValue);
            if (bRet) {
                bFloatValueOK = sal_True;
                fValue = (bTmp ? 1.0 : 0.0);
            }
            break;
        }

        case XML_TOK_TEXTFIELD_STRING_VALUE:
            sValue = sAttrValue;
            bStringValueOK = sal_True;
            break;

        case XML_TOK_TEXTFIELD_FORMULA:
            sFormula = sAttrValue;
            bFormulaOK = sal_True;
            break;

        case XML_TOK_TEXTFIELD_DATA_STYLE_NAME:
        {
            sal_Int32 nKey = rHelper.GetDataStyleKey(sAttrValue);
            if (-1 != nKey)
            {
                nFormatKey = nKey;
                bFormatOK = sal_True;
            }
            break;
        }
    } // switch
}

void XMLValueImportHelper::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    if (bSetType)
    {
        // ??? how to set type?
    }

    if (bSetFormula)
    {
        Any aAny;
        aAny <<= (!bFormulaOK && bFormulaDefault) ? sDefault : sFormula;
        xPropertySet->setPropertyValue(sPropertyContent, aAny);
    }

    // format/style
    if (bSetStyle && bFormatOK)
    {
        Any aAny;
        aAny <<= nFormatKey;
        xPropertySet->setPropertyValue(sPropertyNumberFormat, aAny);
    }

    // value: string or float
    if (bSetValue)
    {
        if (bStringType)
        {
            Any aAny;
            aAny <<= (!bStringValueOK && bStringDefault) ? sDefault : sValue;
            xPropertySet->setPropertyValue(sPropertyContent, aAny);
        }
        else
        {
            Any aAny;
            aAny <<= fValue;
            xPropertySet->setPropertyValue(sPropertyValue, aAny);
        }
    }
}

