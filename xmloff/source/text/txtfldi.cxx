/*************************************************************************
 *
 *  $RCSfile: txtfldi.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: mib $ $Date: 2001-03-19 09:41:43 $
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
 *  Import of all text fields except those from txtvfldi.cxx
 *  (variable related text fields and database display fields)
 */

#ifndef _XMLOFF_TXTFLDI_HXX
#include "txtfldi.hxx"
#endif

#ifndef _XMLOFF_TXTVFLDI_HXX
#include "txtvfldi.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLNUMI_HXX
#include "xmlnumi.hxx"
#endif

#ifndef _XMLOFF_TEXTIMP_HXX_
#include "txtimp.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlement.hxx"
#endif

#ifndef _XMLOFF_XMLSTRINGBUFFERIMPORTCONTEXT_HXX
#include "XMLStringBufferImportContext.hxx"
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_USERDATAPART_HPP_
#include <com/sun/star/text/UserDataPart.hpp>
#endif

#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_PLACEHOLDERTYPE_HPP_
#include <com/sun/star/text/PlaceholderType.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_REFERENCEFIELDPART_HPP_
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_REFERENCEFIELDSOURCE_HPP_
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELD_HPP_
#include <com/sun/star/text/XTextField.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
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

#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELDSSUPPLIER_HPP_
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XDEPENDENTTEXTFIELD_HPP_
#include <com/sun/star/text/XDependentTextField.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_SETVARIABLETYPE_HPP_
#include <com/sun/star/text/SetVariableType.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_FILENAMEDISPLAYFORMAT_HPP_
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_CHAPTERFORMAT_HPP_
#include <com/sun/star/text/ChapterFormat.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_TEMPLATEDISPLAYFORMAT_HPP_
#include <com/sun/star/text/TemplateDisplayFormat.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFO_HPP_
#include <com/sun/star/document/XDocumentInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_BIBLIOGRAPHYDATATYPE_HPP_
#include <com/sun/star/text/BibliographyDataType.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_BIBLIOGRAPHYDATAFIELD_HPP_
#include <com/sun/star/text/BibliographyDataField.hpp>
#endif

#ifndef _RTL_USTRING
#include <rtl/ustring>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _TOOLS_SOLMATH_HXX
#include <tools/solmath.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::xml::sax;



//
// SO API string constants
//

// service prefix and service anems
const sal_Char sAPI_textfield_prefix[]  = "com.sun.star.text.TextField.";
const sal_Char sAPI_fieldmaster_prefix[] = "com.sun.star.text.FieldMaster.";

const sal_Char sAPI_extended_user[]             = "ExtendedUser";
const sal_Char sAPI_user_data_type[]            = "UserDataType";
const sal_Char sAPI_jump_edit[]                 = "JumpEdit";
const sal_Char sAPI_get_expression[]            = "GetExpression";
const sal_Char sAPI_set_expression[]            = "SetExpression";
const sal_Char sAPI_user[]                      = "User";
const sal_Char sAPI_date_time[]                 = "DateTime";
const sal_Char sAPI_page_number[]               = "PageNumber";
const sal_Char sAPI_database_next[]             = "DatabaseNextSet";
const sal_Char sAPI_database_select[]           = "DatabaseNumberOfSet";
const sal_Char sAPI_database_number[]           = "DatabaseSetNumber";
const sal_Char sAPI_database[]                  = "Database";
const sal_Char sAPI_database_name[]             = "DatabaseName";
const sal_Char sAPI_docinfo_change_author[]     = "DocInfo.ChangeAuthor";
const sal_Char sAPI_docinfo_change_date_time[]  = "DocInfo.ChangeDateTime";
const sal_Char sAPI_docinfo_edit_time[]         = "DocInfo.EditTime";
const sal_Char sAPI_docinfo_description[]       = "DocInfo.Description";
const sal_Char sAPI_docinfo_create_author[]     = "DocInfo.CreateAuthor";
const sal_Char sAPI_docinfo_create_date_time[]  = "DocInfo.CreateDateTime";
const sal_Char sAPI_docinfo_info0[]             = "DocInfo.Info0";
const sal_Char sAPI_docinfo_info1[]             = "DocInfo.Info1";
const sal_Char sAPI_docinfo_info2[]             = "DocInfo.Info2";
const sal_Char sAPI_docinfo_info3[]             = "DocInfo.Info3";
const sal_Char sAPI_docinfo_print_author[]      = "DocInfo.PrintAuthor";
const sal_Char sAPI_docinfo_print_date_time[]   = "DocInfo.PrintDateTime";
const sal_Char sAPI_docinfo_keywords[]          = "DocInfo.KeyWords";
const sal_Char sAPI_docinfo_subject[]           = "DocInfo.Subject";
const sal_Char sAPI_docinfo_title[]             = "DocInfo.Title";
const sal_Char sAPI_docinfo_revision[]          = "DocInfo.Revision";
const sal_Char sAPI_hidden_paragraph[]          = "HiddenParagraph";
const sal_Char sAPI_hidden_text[]               = "HiddenText";
const sal_Char sAPI_conditional_text[]          = "ConditionalText";
const sal_Char sAPI_file_name[]                 = "FileName";
const sal_Char sAPI_chapter[]                   = "Chapter";
const sal_Char sAPI_template_name[]             = "TemplateName";
const sal_Char sAPI_page_count[]                = "PageCount";
const sal_Char sAPI_paragraph_count[]           = "ParagraphCount";
const sal_Char sAPI_word_count[]                = "WordCount";
const sal_Char sAPI_character_count[]           = "CharacterCount";
const sal_Char sAPI_table_count[]               = "TableCount";
const sal_Char sAPI_graphic_object_count[]      = "GraphicObjectCount";
const sal_Char sAPI_embedded_object_count[]     = "EmbeddedObjectCount";
const sal_Char sAPI_reference_page_set[]        = "ReferencePageSet";
const sal_Char sAPI_reference_page_get[]        = "ReferencePageGet";
const sal_Char sAPI_macro[]                     = "Macro";
const sal_Char sAPI_dde[]                       = "DDE";
const sal_Char sAPI_get_reference[]             = "GetReference";
const sal_Char sAPI_sheet_name[]                = "SheetName";
const sal_Char sAPI_url[]                       = "URL";
const sal_Char sAPI_bibliography[]              = "Bibliography";
const sal_Char sAPI_annotation[]                = "Annotation";
const sal_Char sAPI_script[]                    = "Script";
const sal_Char sAPI_measure[]                   = "Measure";

// property names
const sal_Char sAPI_is_fixed[]          = "IsFixed";
const sal_Char sAPI_content[]           = "Content";
const sal_Char sAPI_value[]             = "Value";
const sal_Char sAPI_author[]            = "Author";
const sal_Char sAPI_full_name[]         = "FullName";
const sal_Char sAPI_place_holder_type[] = "PlaceHolderType";
const sal_Char sAPI_place_holder[]      = "PlaceHolder";
const sal_Char sAPI_hint[]              = "Hint";
const sal_Char sAPI_variable_name[]     = "VariableName";
const sal_Char sAPI_name[]              = "Name";
const sal_Char sAPI_sub_type[]          = "SubType";
const sal_Char sAPI_numbering_separator[] = "NumberingSeparator";
const sal_Char sAPI_chapter_numbering_level[] = "ChapterNumberingLevel";
const sal_Char sAPI_variable_subtype[]  = "VariableSubtype";
const sal_Char sAPI_formula[]           = "Formula";
const sal_Char sAPI_date_time_value[]   = "DateTimeValue";
const sal_Char sAPI_number_format[]     = "NumberFormat";
const sal_Char sAPI_user_text[]         = "UserText";
const sal_Char sAPI_numbering_type[]    = "NumberingType";
const sal_Char sAPI_offset[]            = "Offset";
const sal_Char sAPI_data_base_name[]    = "DataBaseName";
const sal_Char sAPI_data_table_name[]   = "DataTableName";
const sal_Char sAPI_condition[]         = "Condition";
const sal_Char sAPI_set_number[]        = "SetNumber";
const sal_Char sAPI_is_data_base_format[] = "DataBaseFormat";
const sal_Char sAPI_true_content[]      = "TrueContent";
const sal_Char sAPI_false_content[]     = "FalseContent";
const sal_Char sAPI_revision[]          = "Revision";
const sal_Char sAPI_file_format[]       = "FileFormat";
const sal_Char sAPI_chapter_format[]    = "ChapterFormat";
const sal_Char sAPI_level[]             = "Level";
const sal_Char sAPI_is_date[]           = "IsDate";
const sal_Char sAPI_adjust[]            = "Adjust";
const sal_Char sAPI_on[]                = "On";
const sal_Char sAPI_is_automatic_update[] = "IsAutomaticUpdate";
const sal_Char sAPI_source_name[]       = "SourceName";
const sal_Char sAPI_current_presentation[] = "CurrentPresentation";
const sal_Char sAPI_reference_field_part[] = "ReferenceFieldPart";
const sal_Char sAPI_reference_field_source[] = "ReferenceFieldSource";
const sal_Char sAPI_dde_command_type[]  = "DDECommandType";
const sal_Char sAPI_dde_command_file[]  = "DDECommandFile";
const sal_Char sAPI_dde_command_element[] = "DDECommandElement";
// sAPI_url: also used as service name
const sal_Char sAPI_target_frame[]      = "TargetFrame";
const sal_Char sAPI_representation[]    = "Representation";
const sal_Char sAPI_date[]              = "Date";
const sal_Char sAPI_url_content[]       = "URLContent";
const sal_Char sAPI_script_type[]       = "ScriptType";

const sal_Char sAPI_true[] = "TRUE";


static __FAR_DATA SvXMLTokenMapEntry aTextFieldAttrTokenMap[] =
{
    { XML_NAMESPACE_TEXT, sXML_fixed, XML_TOK_TEXTFIELD_FIXED },
    { XML_NAMESPACE_TEXT, sXML_description, XML_TOK_TEXTFIELD_DESCRIPTION },
    { XML_NAMESPACE_TEXT, sXML_placeholder_type,
                XML_TOK_TEXTFIELD_PLACEHOLDER_TYPE },
    { XML_NAMESPACE_TEXT, sXML_name, XML_TOK_TEXTFIELD_NAME },
    { XML_NAMESPACE_TEXT, sXML_formula, XML_TOK_TEXTFIELD_FORMULA },
    { XML_NAMESPACE_STYLE, sXML_num_format, XML_TOK_TEXTFIELD_NUM_FORMAT },
    { XML_NAMESPACE_STYLE, sXML_num_letter_sync,
                XML_TOK_TEXTFIELD_NUM_LETTER_SYNC },
    { XML_NAMESPACE_TEXT, sXML_display_formula,
                XML_TOK_TEXTFIELD_DISPLAY_FORMULA },
    { XML_NAMESPACE_TEXT, sXML_value_type, XML_TOK_TEXTFIELD_VALUE_TYPE },
    { XML_NAMESPACE_TEXT, sXML_value, XML_TOK_TEXTFIELD_VALUE },
    { XML_NAMESPACE_TEXT, sXML_string_value, XML_TOK_TEXTFIELD_STRING_VALUE },
    { XML_NAMESPACE_TEXT, sXML_date_value, XML_TOK_TEXTFIELD_DATE_VALUE },
    { XML_NAMESPACE_TEXT, sXML_time_value, XML_TOK_TEXTFIELD_TIME_VALUE },
    { XML_NAMESPACE_TEXT, sXML_boolean_value, XML_TOK_TEXTFIELD_BOOL_VALUE},
    { XML_NAMESPACE_TEXT, sXML_currency, XML_TOK_TEXTFIELD_CURRENCY},
    { XML_NAMESPACE_STYLE, sXML_data_style_name,
                XML_TOK_TEXTFIELD_DATA_STYLE_NAME },
    { XML_NAMESPACE_TEXT, sXML_display_outline_level,
                XML_TOK_TEXTFIELD_NUMBERING_LEVEL },
    { XML_NAMESPACE_TEXT, sXML_separation_character,
                XML_TOK_TEXTFIELD_NUMBERING_SEPARATOR },
    { XML_NAMESPACE_TEXT, sXML_display, XML_TOK_TEXTFIELD_DISPLAY },
    { XML_NAMESPACE_TEXT, sXML_time_adjust, XML_TOK_TEXTFIELD_TIME_ADJUST },
    { XML_NAMESPACE_TEXT, sXML_date_adjust, XML_TOK_TEXTFIELD_DATE_ADJUST },
    { XML_NAMESPACE_TEXT, sXML_page_adjust, XML_TOK_TEXTFIELD_PAGE_ADJUST },
    { XML_NAMESPACE_TEXT, sXML_select_page, XML_TOK_TEXTFIELD_SELECT_PAGE },
    { XML_NAMESPACE_TEXT, sXML_database_name, XML_TOK_TEXTFIELD_DATABASE_NAME},
    { XML_NAMESPACE_TEXT, sXML_table_name, XML_TOK_TEXTFIELD_TABLE_NAME },
    { XML_NAMESPACE_TEXT, sXML_column_name, XML_TOK_TEXTFIELD_COLUMN_NAME },
    { XML_NAMESPACE_TEXT, sXML_row_number, XML_TOK_TEXTFIELD_ROW_NUMBER },
    { XML_NAMESPACE_TEXT, sXML_condition, XML_TOK_TEXTFIELD_CONDITION },
    { XML_NAMESPACE_TEXT, sXML_string_value_if_true,
                XML_TOK_TEXTFIELD_STRING_VALUE_IF_TRUE },
    { XML_NAMESPACE_TEXT, sXML_string_value_if_false,
                XML_TOK_TEXTFIELD_STRING_VALUE_IF_FALSE },
    { XML_NAMESPACE_TEXT, sXML_editing_cycles, XML_TOK_TEXTFIELD_REVISION },
    { XML_NAMESPACE_TEXT, sXML_outline_level, XML_TOK_TEXTFIELD_OUTLINE_LEVEL},
    { XML_NAMESPACE_TEXT, sXML_active, XML_TOK_TEXTFIELD_ACTIVE },
    { XML_NAMESPACE_TEXT, sXML_reference_format,
                XML_TOK_TEXTFIELD_REFERENCE_FORMAT },
    { XML_NAMESPACE_TEXT, sXML_ref_name, XML_TOK_TEXTFIELD_REF_NAME },
    { XML_NAMESPACE_TEXT, sXML_connection_name,
      XML_TOK_TEXTFIELD_CONNECTION_NAME },
    { XML_NAMESPACE_XLINK, sXML_href, XML_TOK_TEXTFIELD_HREF },
    { XML_NAMESPACE_OFFICE, sXML_target_frame_name,
      XML_TOK_TEXTFIELD_TARGET_FRAME },
    { XML_NAMESPACE_OFFICE, sXML_create_date,
                XML_TOK_TEXTFIELD_OFFICE_CREATE_DATE },
    { XML_NAMESPACE_OFFICE, sXML_author, XML_TOK_TEXTFIELD_OFFICE_AUTHOR },
    { XML_NAMESPACE_TEXT, sXML_annotation, XML_TOK_TEXTFIELD_ANNOTATION },
    { XML_NAMESPACE_SCRIPT, sXML_language, XML_TOK_TEXTFIELD_LANGUAGE },
    { XML_NAMESPACE_TEXT, sXML_kind, XML_TOK_TEXTFIELD_MEASURE_KIND },

    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& XMLTextImportHelper::GetTextFieldAttrTokenMap()
{
    if (NULL == pTextFieldAttrTokenMap) {
        pTextFieldAttrTokenMap = new SvXMLTokenMap(aTextFieldAttrTokenMap);
    }

    return *pTextFieldAttrTokenMap;

}

TYPEINIT1( XMLTextFieldImportContext, SvXMLImportContext);

XMLTextFieldImportContext::XMLTextFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    const sal_Char* pService,
    sal_uInt16 nPrefix, const OUString& rElementName)
    : SvXMLImportContext( rImport, nPrefix, rElementName ),
      sContentBuffer(),
      rTextImportHelper(rHlp),
      bValid(sal_False),
      sServicePrefix(RTL_CONSTASCII_USTRINGPARAM(
          sAPI_textfield_prefix))
{
    DBG_ASSERT(NULL != pService, "Need service name!");
    sServiceName = OUString::createFromAscii(pService);
}

void XMLTextFieldImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // process attributes
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 i=0; i<nLength; i++) {

        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );

        ProcessAttribute(rTextImportHelper.GetTextFieldAttrTokenMap().
                             Get(nPrefix, sLocalName),
                         xAttrList->getValueByIndex(i) );
    }
}

XMLTextFieldImportContext::~XMLTextFieldImportContext() {
}

OUString XMLTextFieldImportContext::GetContent()
{
    if (sContent.getLength()==0)
    {
        sContent = sContentBuffer.makeStringAndClear();
    }

    return sContent;
}

void XMLTextFieldImportContext::EndElement()
{
    DBG_ASSERT(GetServiceName().getLength()>0, "no service name for element!");
    if (bValid)
    {

        // create field/Service
        Reference<XPropertySet> xPropSet;
        if (CreateField(xPropSet, sServicePrefix + GetServiceName()))
        {
            // set field properties
            PrepareField(xPropSet);

            // attach field to document
            Reference<XTextContent> xTextContent(xPropSet, UNO_QUERY);

            // workaround for #80606#
            try
            {
                rTextImportHelper.InsertTextContent(xTextContent);
            }
            catch (lang::IllegalArgumentException e)
            {
                // ignore
            }
            return;
        }
    }

    // in case of error: write element content
    rTextImportHelper.InsertString(GetContent());
}

void XMLTextFieldImportContext::Characters(const OUString& sContent)
{
    sContentBuffer.append(sContent);
}

sal_Bool XMLTextFieldImportContext::CreateField(
    Reference<XPropertySet> & xField,
    const OUString& sServiceName)
{
    // instantiate new XTextField:
    // ask import for model, model is factory, ask factory to create service

    Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),UNO_QUERY);
    if( xFactory.is() )
    {
        Reference<XInterface> xIfc = xFactory->createInstance(sServiceName);
        if( xIfc.is() )
        {
            Reference<XPropertySet> xTmp( xIfc, UNO_QUERY );

            xField = xTmp;
        } else {
            return sal_False;   // can't create instance
        }
    } else {
        return sal_False;   // can't get MultiServiceFactory
    }

    return sal_True;
}

/// create the appropriate field context from
XMLTextFieldImportContext*
XMLTextFieldImportContext::CreateTextFieldImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrefix,
    const OUString& rName,
    sal_uInt16 nToken)
{
    XMLTextFieldImportContext* pContext = NULL;

    switch (nToken)
    {
        case XML_TOK_TEXT_SENDER_FIRSTNAME:
        case XML_TOK_TEXT_SENDER_LASTNAME:
        case XML_TOK_TEXT_SENDER_INITIALS:
        case XML_TOK_TEXT_SENDER_TITLE:
        case XML_TOK_TEXT_SENDER_POSITION:
        case XML_TOK_TEXT_SENDER_EMAIL:
        case XML_TOK_TEXT_SENDER_PHONE_PRIVATE:
        case XML_TOK_TEXT_SENDER_FAX:
        case XML_TOK_TEXT_SENDER_COMPANY:
        case XML_TOK_TEXT_SENDER_PHONE_WORK:
        case XML_TOK_TEXT_SENDER_STREET:
        case XML_TOK_TEXT_SENDER_CITY:
        case XML_TOK_TEXT_SENDER_POSTAL_CODE:
        case XML_TOK_TEXT_SENDER_COUNTRY:
        case XML_TOK_TEXT_SENDER_STATE_OR_PROVINCE:
            pContext =
                new XMLSenderFieldImportContext( rImport, rHlp,
                                                 nPrefix, rName, nToken );
            break;

        case XML_TOK_TEXT_AUTHOR_NAME:
        case XML_TOK_TEXT_AUTHOR_INITIALS:
            pContext =
                new XMLAuthorFieldImportContext( rImport, rHlp,
                                                 nPrefix, rName, nToken );
            break;

        case XML_TOK_TEXT_PLACEHOLDER:
            pContext =
                new XMLPlaceholderFieldImportContext( rImport, rHlp,
                                                      nPrefix, rName);
            break;
        case XML_TOK_TEXT_SEQUENCE:
            pContext =
                new XMLSequenceFieldImportContext( rImport, rHlp,
                                                   nPrefix, rName );
            break;
        case XML_TOK_TEXT_TEXT_INPUT:
            pContext =
                new XMLTextInputFieldImportContext( rImport, rHlp,
                                                    nPrefix, rName );
            break;
        case XML_TOK_TEXT_EXPRESSION:
            pContext =
                new XMLExpressionFieldImportContext( rImport, rHlp,
                                                     nPrefix, rName );
            break;
        case XML_TOK_TEXT_VARIABLE_SET:
            pContext =
                new XMLVariableSetFieldImportContext( rImport, rHlp,
                                                      nPrefix, rName );
            break;
        case XML_TOK_TEXT_VARIABLE_INPUT:
            pContext =
                new XMLVariableInputFieldImportContext( rImport, rHlp,
                                                        nPrefix, rName );
            break;
        case XML_TOK_TEXT_VARIABLE_GET:
            pContext =
                new XMLVariableGetFieldImportContext( rImport, rHlp,
                                                      nPrefix, rName );
            break;
        case XML_TOK_TEXT_USER_FIELD_GET:
            pContext = new XMLUserFieldImportContext( rImport, rHlp,
                                                      nPrefix, rName );
            break;
        case XML_TOK_TEXT_USER_FIELD_INPUT:
            pContext = new XMLUserFieldInputImportContext( rImport, rHlp,
                                                           nPrefix, rName );
            break;
        case XML_TOK_TEXT_TIME:
            pContext = new XMLTimeFieldImportContext( rImport, rHlp,
                                                      nPrefix, rName );
            break;
        case XML_TOK_TEXT_PAGE_CONTINUATION_STRING:
            pContext = new XMLPageContinuationImportContext( rImport, rHlp,
                                                             nPrefix, rName );
            break;

        case XML_TOK_TEXT_PAGE_NUMBER:
            pContext = new XMLPageNumberImportContext( rImport, rHlp,
                                                       nPrefix, rName );
            break;

        case XML_TOK_TEXT_DATE:
            pContext = new XMLDateFieldImportContext( rImport, rHlp,
                                                      nPrefix, rName );
            break;

        case XML_TOK_TEXT_DATABASE_NAME:
            pContext = new XMLDatabaseNameImportContext( rImport, rHlp,
                                                         nPrefix, rName );
            break;
        case XML_TOK_TEXT_DATABASE_NEXT:
            pContext = new XMLDatabaseNextImportContext( rImport, rHlp,
                                                         nPrefix, rName );
            break;
        case XML_TOK_TEXT_DATABASE_SELECT:
            pContext = new XMLDatabaseSelectImportContext( rImport, rHlp,
                                                           nPrefix, rName );
            break;
        case XML_TOK_TEXT_DATABASE_ROW_NUMBER:
            pContext = new XMLDatabaseNumberImportContext( rImport, rHlp,
                                                           nPrefix, rName );
            break;
        case XML_TOK_TEXT_DATABASE_DISPLAY:
            pContext = new XMLDatabaseDisplayImportContext( rImport, rHlp,
                                                            nPrefix, rName );
            break;
        case XML_TOK_TEXT_CONDITIONAL_TEXT:
            pContext = new XMLConditionalTextImportContext( rImport, rHlp,
                                                            nPrefix, rName );
            break;
        case XML_TOK_TEXT_HIDDEN_TEXT:
            pContext = new XMLHiddenTextImportContext( rImport, rHlp,
                                                       nPrefix, rName );
            break;
        case XML_TOK_TEXT_HIDDEN_PARAGRAPH:
            pContext = new XMLHiddenParagraphImportContext( rImport, rHlp,
                                                            nPrefix, rName );
            break;

        case XML_TOK_TEXT_DOCUMENT_INFORMATION_0:   // info fields only for
        case XML_TOK_TEXT_DOCUMENT_INFORMATION_1:   // compatibilty with older
        case XML_TOK_TEXT_DOCUMENT_INFORMATION_2:   // versions
        case XML_TOK_TEXT_DOCUMENT_INFORMATION_3:
        case XML_TOK_TEXT_DOCUMENT_DESCRIPTION:
        case XML_TOK_TEXT_DOCUMENT_TITLE:
        case XML_TOK_TEXT_DOCUMENT_SUBJECT:
        case XML_TOK_TEXT_DOCUMENT_KEYWORDS:
            pContext = new XMLSimpleDocInfoImportContext( rImport, rHlp,
                                                          nPrefix, rName,
                                                          nToken, sal_True,
                                                          sal_False );
            break;
        case XML_TOK_TEXT_DOCUMENT_CREATION_AUTHOR:
        case XML_TOK_TEXT_DOCUMENT_PRINT_AUTHOR:
        case XML_TOK_TEXT_DOCUMENT_SAVE_AUTHOR:
            pContext = new XMLSimpleDocInfoImportContext( rImport, rHlp,
                                                          nPrefix, rName,
                                                          nToken, sal_False,
                                                          sal_True );
            break;

        case XML_TOK_TEXT_DOCUMENT_CREATION_DATE:
        case XML_TOK_TEXT_DOCUMENT_CREATION_TIME:
        case XML_TOK_TEXT_DOCUMENT_PRINT_DATE:
        case XML_TOK_TEXT_DOCUMENT_PRINT_TIME:
        case XML_TOK_TEXT_DOCUMENT_SAVE_DATE:
        case XML_TOK_TEXT_DOCUMENT_SAVE_TIME:
        case XML_TOK_TEXT_DOCUMENT_EDIT_DURATION:
            pContext = new XMLDateTimeDocInfoImportContext( rImport, rHlp,
                                                            nPrefix, rName,
                                                            nToken );
            break;

        case XML_TOK_TEXT_DOCUMENT_REVISION:
            pContext = new XMLRevisionDocInfoImportContext( rImport, rHlp,
                                                            nPrefix, rName,
                                                            nToken );
            break;

        case XML_TOK_TEXT_DOCUMENT_USER_DEFINED:
            pContext = new XMLUserDocInfoImportContext( rImport, rHlp,
                                                        nPrefix, rName,
                                                        nToken );
            break;

        case XML_TOK_TEXT_FILENAME:
            pContext = new XMLFileNameImportContext( rImport, rHlp,
                                                     nPrefix, rName );
            break;

        case XML_TOK_TEXT_CHAPTER:
            pContext = new XMLChapterImportContext( rImport, rHlp,
                                                    nPrefix, rName );
            break;

        case XML_TOK_TEXT_TEMPLATENAME:
            pContext = new XMLTemplateNameImportContext( rImport, rHlp,
                                                         nPrefix, rName );
            break;

        case XML_TOK_TEXT_WORD_COUNT:
        case XML_TOK_TEXT_PARAGRAPH_COUNT:
        case XML_TOK_TEXT_TABLE_COUNT:
        case XML_TOK_TEXT_CHARACTER_COUNT:
        case XML_TOK_TEXT_IMAGE_COUNT:
        case XML_TOK_TEXT_OBJECT_COUNT:
        case XML_TOK_TEXT_PAGE_COUNT:
            pContext = new XMLCountFieldImportContext( rImport, rHlp,
                                                       nPrefix, rName, nToken);
            break;

        case XML_TOK_TEXT_GET_PAGE_VAR:
            pContext = new XMLPageVarGetFieldImportContext( rImport, rHlp,
                                                            nPrefix, rName );
            break;

        case XML_TOK_TEXT_SET_PAGE_VAR:
            pContext = new XMLPageVarSetFieldImportContext( rImport, rHlp,
                                                            nPrefix, rName );
            break;

        case XML_TOK_TEXT_MACRO:
            pContext = new XMLMacroFieldImportContext( rImport, rHlp,
                                                       nPrefix, rName );
            break;

        case XML_TOK_TEXT_DDE:
            pContext = new XMLDdeFieldImportContext( rImport, rHlp,
                                                     nPrefix, rName );
            break;

        case XML_TOK_TEXT_REFERENCE_REF:
        case XML_TOK_TEXT_BOOKMARK_REF:
        case XML_TOK_TEXT_FOOTNOTE_REF:
        case XML_TOK_TEXT_ENDNOTE_REF:
        case XML_TOK_TEXT_SEQUENCE_REF:
            pContext = new XMLReferenceFieldImportContext( rImport, rHlp,
                                                           nToken,
                                                           nPrefix, rName );
            break;

        case XML_TOK_TEXT_SHEET_NAME:
            pContext = new XMLSheetNameImportContext( rImport, rHlp,
                                                      nPrefix, rName );
            break;

        case XML_TOK_TEXT_BIBLIOGRAPHY_MARK:
            pContext = new XMLBibliographyFieldImportContext( rImport, rHlp,
                                                              nPrefix, rName );
            break;

        case XML_TOK_TEXT_ANNOTATION:
            pContext = new XMLAnnotationImportContext( rImport, rHlp,
                                                       nPrefix, rName);
            break;

        case XML_TOK_TEXT_SCRIPT:
            pContext = new XMLScriptImportContext( rImport, rHlp,
                                                   nPrefix, rName);
            break;

        case XML_TOK_TEXT_MEASURE:
            pContext = new XMLMeasureFieldImportContext( rImport, rHlp,
                                                         nPrefix, rName );
            break;

        default:
            // ignore! May not even be a textfield.
            // (Reminder: This method is called inside default:-branch)
            pContext = NULL;
            break;
    }

    return pContext;
}



//
// XMLSenderFieldImportContext
//

TYPEINIT1( XMLSenderFieldImportContext, XMLTextFieldImportContext);

XMLSenderFieldImportContext::XMLSenderFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName,
    sal_uInt16 nToken) :
    XMLTextFieldImportContext(rImport, rHlp, sAPI_extended_user,
                              nPrfx, sLocalName),
    bFixed(sal_True),
    nElementToken(nToken),
    sEmpty(),
    sPropertyFieldSubType(RTL_CONSTASCII_USTRINGPARAM(sAPI_user_data_type)),
    sPropertyFixed(RTL_CONSTASCII_USTRINGPARAM(sAPI_is_fixed)),
    sPropertyContent(RTL_CONSTASCII_USTRINGPARAM(sAPI_content))
{
}

void XMLSenderFieldImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    bValid = sal_True;
    switch (nElementToken) {
    case XML_TOK_TEXT_SENDER_FIRSTNAME:
        nSubType = UserDataPart::FIRSTNAME;
        break;
    case XML_TOK_TEXT_SENDER_LASTNAME:
        nSubType = UserDataPart::NAME;
        break;
    case XML_TOK_TEXT_SENDER_INITIALS:
        nSubType = UserDataPart::SHORTCUT;
        break;
    case XML_TOK_TEXT_SENDER_TITLE:
        nSubType = UserDataPart::TITLE;
        break;
    case XML_TOK_TEXT_SENDER_POSITION:
        nSubType = UserDataPart::POSITION;
        break;
    case XML_TOK_TEXT_SENDER_EMAIL:
        nSubType = UserDataPart::EMAIL;
        break;
    case XML_TOK_TEXT_SENDER_PHONE_PRIVATE:
        nSubType = UserDataPart::PHONE_PRIVATE;
        break;
    case XML_TOK_TEXT_SENDER_FAX:
        nSubType = UserDataPart::FAX;
        break;
    case XML_TOK_TEXT_SENDER_COMPANY:
        nSubType = UserDataPart::COMPANY;
        break;
    case XML_TOK_TEXT_SENDER_PHONE_WORK:
        nSubType = UserDataPart::PHONE_COMPANY;
        break;
    case XML_TOK_TEXT_SENDER_STREET:
        nSubType = UserDataPart::STREET;
        break;
    case XML_TOK_TEXT_SENDER_CITY:
        nSubType = UserDataPart::CITY;
        break;
    case XML_TOK_TEXT_SENDER_POSTAL_CODE:
        nSubType = UserDataPart::ZIP;
        break;
    case XML_TOK_TEXT_SENDER_COUNTRY:
        nSubType = UserDataPart::COUNTRY;
        break;
    case XML_TOK_TEXT_SENDER_STATE_OR_PROVINCE:
        nSubType = UserDataPart::STATE;
        break;
    default:
        bValid = sal_False;
        break;
    }

    // process Attributes
    XMLTextFieldImportContext::StartElement(xAttrList);
}

void XMLSenderFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue)
{
    if (XML_TOK_TEXTFIELD_FIXED == nAttrToken) {

        // set bVal
        sal_Bool bVal;
        sal_Bool bRet = GetImport().GetMM100UnitConverter().
            convertBool(bVal, sAttrValue);

        // set bFixed if successfull
        if (bRet) {
            bFixed = bVal;
        }
    }
}

void XMLSenderFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropSet)
{
    // set members
    Any aAny;
    aAny <<= nSubType;
    xPropSet->setPropertyValue(sPropertyFieldSubType, aAny);

    aAny.setValue( &bFixed, ::getBooleanCppuType() );
    xPropSet->setPropertyValue(sPropertyFixed, aAny);

    if (bFixed)
    {
        aAny <<= GetContent();
        xPropSet->setPropertyValue(sPropertyContent, aAny);
    }
}



//
// XMLAuthorFieldImportContext
//

TYPEINIT1( XMLAuthorFieldImportContext, XMLSenderFieldImportContext);

XMLAuthorFieldImportContext::XMLAuthorFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName,
    sal_uInt16 nToken) :
        XMLSenderFieldImportContext(rImport, rHlp, nPrfx, sLocalName, nToken),
        bAuthorFullName(sal_True),
        sPropertyAuthorFullName(RTL_CONSTASCII_USTRINGPARAM(sAPI_full_name)),
        sPropertyContent(RTL_CONSTASCII_USTRINGPARAM(sAPI_content)),
        sPropertyFixed(RTL_CONSTASCII_USTRINGPARAM(sAPI_is_fixed)),
        sServiceAuthor(RTL_CONSTASCII_USTRINGPARAM(sAPI_author))
{
    // overwrite service name from XMLSenderFieldImportContext
    SetServiceName(sServiceAuthor);
}

void XMLAuthorFieldImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList) {

    bAuthorFullName = (XML_TOK_TEXT_AUTHOR_INITIALS != nElementToken);
    bValid = sal_True;

    // process Attributes
    XMLTextFieldImportContext::StartElement(xAttrList);
}

void XMLAuthorFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropSet)
{
    // set members
    Any aAny;
    aAny.setValue( &bAuthorFullName, ::getBooleanCppuType() );
    xPropSet->setPropertyValue(sPropertyAuthorFullName, aAny);

    aAny.setValue( &bFixed, ::getBooleanCppuType() );
    xPropSet->setPropertyValue(sPropertyFixed, aAny);

    if (bFixed)
    {
        aAny <<= GetContent();
        xPropSet->setPropertyValue(sPropertyContent, aAny);
    }
}


//
// page continuation string
//

TYPEINIT1( XMLPageContinuationImportContext, XMLTextFieldImportContext );

static SvXMLEnumMapEntry __READONLY_DATA lcl_aSelectPageAttrMap[] =
{
    { sXML_previous,    PageNumberType_PREV },
    { sXML_current,     PageNumberType_CURRENT },
    { sXML_next,        PageNumberType_NEXT },
    { 0,                0 },
};

XMLPageContinuationImportContext::XMLPageContinuationImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_page_number,
                                  nPrfx, sLocalName),
        sString(),
        sStringOK(sal_False),
        eSelectPage(PageNumberType_CURRENT),
        sPropertySubType(RTL_CONSTASCII_USTRINGPARAM(sAPI_sub_type)),
        sPropertyUserText(RTL_CONSTASCII_USTRINGPARAM(sAPI_user_text)),
        sPropertyNumberingType(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_numbering_type))
{
    bValid = sal_True;
}

void XMLPageContinuationImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken, const OUString& sAttrValue )
{
    switch(nAttrToken)
    {
        case XML_TOK_TEXTFIELD_SELECT_PAGE:
        {
            sal_uInt16 nTmp;
            if (SvXMLUnitConverter::convertEnum(nTmp, sAttrValue,
                                                lcl_aSelectPageAttrMap)
                && (PageNumberType_CURRENT != nTmp) )
            {
                eSelectPage = (PageNumberType)nTmp;
            }
            break;
        }
        case XML_TOK_TEXTFIELD_STRING_VALUE:
            sString = sAttrValue;
            sStringOK = sal_True;
            break;
    }
}

void XMLPageContinuationImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    aAny <<= eSelectPage;
    xPropertySet->setPropertyValue(sPropertySubType, aAny);

    aAny <<= (sStringOK ? sString : GetContent());
    xPropertySet->setPropertyValue(sPropertyUserText, aAny);

    aAny <<= style::NumberingType::CHAR_SPECIAL;
    xPropertySet->setPropertyValue(sPropertyNumberingType, aAny);
}



//
// page number field
//

TYPEINIT1( XMLPageNumberImportContext, XMLTextFieldImportContext );

XMLPageNumberImportContext::XMLPageNumberImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_page_number,
                                  nPrfx, sLocalName),
        sPropertySubType(RTL_CONSTASCII_USTRINGPARAM(sAPI_sub_type)),
        sPropertyNumberingType(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_numbering_type)),
        sPropertyOffset(RTL_CONSTASCII_USTRINGPARAM(sAPI_offset)),
        sNumberFormat(),
        sNumberSync(RTL_CONSTASCII_USTRINGPARAM(sXML_false)),
        nPageAdjust(0),
        eSelectPage(PageNumberType_CURRENT),
        sNumberFormatOK(sal_False)
{
    bValid = sal_True;
}

void XMLPageNumberImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_NUM_FORMAT:
            sNumberFormat = sAttrValue;
            sNumberFormatOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_NUM_LETTER_SYNC:
            sNumberSync = sAttrValue;
            break;
        case XML_TOK_TEXTFIELD_SELECT_PAGE:
        {
            sal_uInt16 nTmp;
            if (SvXMLUnitConverter::convertEnum(nTmp, sAttrValue,
                                                lcl_aSelectPageAttrMap))
            {
                eSelectPage = (PageNumberType)nTmp;
            }
            break;
        }
        case XML_TOK_TEXTFIELD_PAGE_ADJUST:
        {
            sal_Int32 nTmp;
            if (SvXMLUnitConverter::convertNumber(nTmp, sAttrValue))
            {
                nPageAdjust = (sal_Int16)nTmp;
            }
            break;
        }
    }
}

void XMLPageNumberImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    // all properties are optional
    Reference<XPropertySetInfo> xPropertySetInfo(
        xPropertySet->getPropertySetInfo());

    if (xPropertySetInfo->hasPropertyByName(sPropertyNumberingType))
    {
        sal_Int16 nNumType;
        if( sNumberFormatOK )
        {
            nNumType= style::NumberingType::ARABIC;
            GetImport().GetMM100UnitConverter().convertNumFormat( nNumType,
                                                    sNumberFormat,
                                                    sNumberSync );
        }
        else
            nNumType = style::NumberingType::PAGE_DESCRIPTOR;

        aAny <<= nNumType;
        xPropertySet->setPropertyValue(sPropertyNumberingType, aAny);
    }

    if (xPropertySetInfo->hasPropertyByName(sPropertyOffset))
    {
        // adjust offset
        switch (eSelectPage)
        {
            case PageNumberType_PREV:
                nPageAdjust--;
                break;
            case PageNumberType_CURRENT:
                break;
            case PageNumberType_NEXT:
                nPageAdjust++;
                break;
            default:
                DBG_WARNING("unknown page number type");
        }
        aAny <<= nPageAdjust;
        xPropertySet->setPropertyValue(sPropertyOffset, aAny);
    }

    if (xPropertySetInfo->hasPropertyByName(sPropertySubType))
    {
        aAny <<= eSelectPage;
        xPropertySet->setPropertyValue(sPropertySubType, aAny);
    }
}



//
// Placeholder
//

TYPEINIT1( XMLPlaceholderFieldImportContext, XMLTextFieldImportContext);

XMLPlaceholderFieldImportContext::XMLPlaceholderFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_jump_edit,
                                  nPrfx, sLocalName),
        sPropertyPlaceholderType(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_place_holder_type)),
        sPropertyPlaceholder(RTL_CONSTASCII_USTRINGPARAM(sAPI_place_holder)),
        sPropertyHint(RTL_CONSTASCII_USTRINGPARAM(sAPI_hint)),
        sDescription()
{
}

/// process attribute values
void XMLPlaceholderFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken, const OUString& sAttrValue )
{
    switch (nAttrToken) {
    case XML_TOK_TEXTFIELD_DESCRIPTION:
        sDescription = sAttrValue;
        break;

    case XML_TOK_TEXTFIELD_PLACEHOLDER_TYPE:
        bValid = sal_True;
        if (0 == sAttrValue.compareToAscii(sXML_table)) {
            nPlaceholderType = PlaceholderType::TABLE;
        } else if (0 == sAttrValue.compareToAscii(sXML_text)) {
            nPlaceholderType = PlaceholderType::TEXT;
        } else if (0 == sAttrValue.compareToAscii(sXML_text_box)) {
            nPlaceholderType = PlaceholderType::TEXTFRAME;
        } else if (0 == sAttrValue.compareToAscii(sXML_image)) {
            nPlaceholderType = PlaceholderType::GRAPHIC;
        } else if (0 == sAttrValue.compareToAscii(sXML_object)) {
            nPlaceholderType = PlaceholderType::OBJECT;
        } else {
            bValid = sal_False;
        }
        break;

    default:
        ; // ignore
    }
}

void XMLPlaceholderFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet) {

    Any aAny;
    aAny <<= sDescription;
    xPropertySet->setPropertyValue(sPropertyHint, aAny);

    // remove <...> around content (if present)
    OUString sContent = GetContent();
    sal_Int32 nStart = 0;
    sal_Int32 nLength = sContent.getLength();
    if ((nLength > 0) && (sContent.getStr()[0] == '<'))
    {
        --nLength;
        ++nStart;
    }
    if ((nLength > 0) && (sContent.getStr()[sContent.getLength()-1] == '>'))
    {
        --nLength;
    }
    aAny <<= sContent.copy(nStart, nLength);
    xPropertySet->setPropertyValue(sPropertyPlaceholder, aAny);

    aAny <<= nPlaceholderType;
    xPropertySet->setPropertyValue(sPropertyPlaceholderType, aAny);
}


//
// time field
//

TYPEINIT1( XMLTimeFieldImportContext, XMLTextFieldImportContext);

XMLTimeFieldImportContext::XMLTimeFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_date_time,
                                  nPrfx, sLocalName),
        fTimeValue(0.0),
        nAdjust(0),
        bTimeOK(sal_False),
        bFixed(sal_False),
        nFormatKey(0),
        bFormatOK(sal_False),
        bIsDate(sal_False),
        sPropertyNumberFormat(RTL_CONSTASCII_USTRINGPARAM(sAPI_number_format)),
        sPropertyFixed(RTL_CONSTASCII_USTRINGPARAM(sAPI_is_fixed)),
        sPropertyDateTimeValue(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_date_time_value)),
        sPropertyDateTime(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_date_time)),
        sPropertyAdjust(RTL_CONSTASCII_USTRINGPARAM(sAPI_adjust)),
        sPropertyIsDate(RTL_CONSTASCII_USTRINGPARAM(sAPI_is_date))
{
    bValid = sal_True;  // always valid!
}

void XMLTimeFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken, const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_TIME_VALUE:
        {
            double fTmp;
            if (GetImport().GetMM100UnitConverter().
                convertDateTime(fTmp, sAttrValue))
            {
                fTimeValue = fTmp;
                bTimeOK = sal_True;
            }

            if (GetImport().GetMM100UnitConverter().
                convertDateTime(aDateTimeValue, sAttrValue ))
            {
                bTimeOK = sal_True;
            }
            break;
        }
        case XML_TOK_TEXTFIELD_FIXED:
        {
            sal_Bool bTmp;
            if (SvXMLUnitConverter::convertBool(bTmp, sAttrValue))
            {
                bFixed = bTmp;
            }
            break;
        }
        case XML_TOK_TEXTFIELD_DATA_STYLE_NAME:
        {
            sal_Int32 nKey = GetImportHelper().GetDataStyleKey(sAttrValue);
            if (-1 != nKey)
            {
                nFormatKey = nKey;
                bFormatOK = sal_True;
            }
            break;
        }
        case XML_TOK_TEXTFIELD_TIME_ADJUST:
        {
            double fTmp;

            if (SvXMLUnitConverter::convertTime(fTmp, sAttrValue))
            {
                // convert to minutes
                nAdjust = (sal_Int16)SolarMath::ApproxFloor(fTmp * 60 * 24);
            }
            break;
        }
    }
}

void XMLTimeFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    // all properties are optional (except IsDate)
    Reference<XPropertySetInfo> xPropertySetInfo(
        xPropertySet->getPropertySetInfo());

    if (xPropertySetInfo->hasPropertyByName(sPropertyFixed))
    {
        aAny.setValue( &bFixed, ::getBooleanCppuType() );
        xPropertySet->setPropertyValue(sPropertyFixed, aAny);
    }

    aAny.setValue( &bIsDate, ::getBooleanCppuType() );
    xPropertySet->setPropertyValue(sPropertyIsDate, aAny);

    if (xPropertySetInfo->hasPropertyByName(sPropertyAdjust))
    {
        aAny <<= nAdjust;
        xPropertySet->setPropertyValue(sPropertyAdjust, aAny);
    }

    if (bFixed && bTimeOK &&
        xPropertySetInfo->hasPropertyByName(sPropertyDateTimeValue))
    {
        aAny <<= aDateTimeValue;
        xPropertySet->setPropertyValue(sPropertyDateTimeValue, aAny);
    }

    if (bFixed && bTimeOK &&
        xPropertySetInfo->hasPropertyByName(sPropertyDateTime))
    {
        aAny <<= aDateTimeValue;
        xPropertySet->setPropertyValue(sPropertyDateTime, aAny);
    }

    if (bFormatOK &&
        xPropertySetInfo->hasPropertyByName(sPropertyNumberFormat))
    {
        aAny <<= nFormatKey;
        xPropertySet->setPropertyValue(sPropertyNumberFormat, aAny);
    }
}



//
// date field
//

TYPEINIT1( XMLDateFieldImportContext, XMLTimeFieldImportContext );

XMLDateFieldImportContext::XMLDateFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLTimeFieldImportContext(rImport, rHlp, nPrfx, sLocalName)
{
    bIsDate = sal_True; // always a date!
}

void XMLDateFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const ::rtl::OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_DATE_VALUE:
        {
            double fTmp;

            if (GetImport().GetMM100UnitConverter().
                convertDateTime(fTmp, sAttrValue))
            {
                fTimeValue = SolarMath::ApproxFloor(fTmp);
                bTimeOK = sal_True;
            }

            if (GetImport().GetMM100UnitConverter().
                convertDateTime(aDateTimeValue, sAttrValue ))
            {
                bTimeOK = sal_True;
            }
            break;
        }
        case XML_TOK_TEXTFIELD_DATE_ADJUST:
            // delegate to superclass, pretending it was a time-adjust attr.
            XMLTimeFieldImportContext::ProcessAttribute(
                XML_TOK_TEXTFIELD_TIME_ADJUST,
                sAttrValue);
            break;
        case XML_TOK_TEXTFIELD_TIME_VALUE:
        case XML_TOK_TEXTFIELD_TIME_ADJUST:
            ; // ignore time-adjust and time-value attributes
            break;
        default:
            // all others: delegate to super-class
            XMLTimeFieldImportContext::ProcessAttribute(nAttrToken,
                                                        sAttrValue);
            break;
    }
}




//
// database field superclass
//

TYPEINIT1( XMLDatabaseFieldImportContext, XMLTextFieldImportContext );

XMLDatabaseFieldImportContext::XMLDatabaseFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    const sal_Char* pServiceName, sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, pServiceName,
                                  nPrfx, sLocalName),
        sPropertyDatabaseName(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_data_base_name)),
        sPropertyTableName(RTL_CONSTASCII_USTRINGPARAM(sAPI_data_table_name)),
        sDatabaseName(),
        sTableName(),
        bDatabaseOK(sal_False),
        bTableOK(sal_False)
{
}

void XMLDatabaseFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken, const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_DATABASE_NAME:
            sDatabaseName = sAttrValue;
            bDatabaseOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_TABLE_NAME:
            sTableName = sAttrValue;
            bTableOK = sal_True;
            break;
    }
}

void XMLDatabaseFieldImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    aAny <<= sDatabaseName;
    xPropertySet->setPropertyValue(sPropertyDatabaseName, aAny);

    aAny <<= sTableName;
    xPropertySet->setPropertyValue(sPropertyTableName, aAny);
}



//
// database name field
//

TYPEINIT1( XMLDatabaseNameImportContext, XMLDatabaseFieldImportContext );

XMLDatabaseNameImportContext::XMLDatabaseNameImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLDatabaseFieldImportContext(rImport, rHlp, sAPI_database_name,
                                      nPrfx, sLocalName)
{
}

void XMLDatabaseNameImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken, const OUString& sAttrValue )
{
    // delegate to superclass and check for success
    XMLDatabaseFieldImportContext::ProcessAttribute(nAttrToken, sAttrValue);
    bValid = bDatabaseOK && bTableOK;
}



//
// database next field
//

TYPEINIT1( XMLDatabaseNextImportContext, XMLDatabaseFieldImportContext );

XMLDatabaseNextImportContext::XMLDatabaseNextImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    const sal_Char* pServiceName, sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLDatabaseFieldImportContext(rImport, rHlp, pServiceName,
                                      nPrfx, sLocalName),
        sPropertyCondition(RTL_CONSTASCII_USTRINGPARAM(sAPI_condition)),
        sTrue(RTL_CONSTASCII_USTRINGPARAM(sAPI_true)),
        sCondition(),
        bConditionOK(sal_False)
{
}

XMLDatabaseNextImportContext::XMLDatabaseNextImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLDatabaseFieldImportContext(rImport, rHlp, sAPI_database_next,
                                      nPrfx, sLocalName),
        sPropertyCondition(RTL_CONSTASCII_USTRINGPARAM(sAPI_condition)),
        sTrue(RTL_CONSTASCII_USTRINGPARAM(sAPI_true)),
        sCondition(),
        bConditionOK(sal_False)
{
}

void XMLDatabaseNextImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken, const OUString& sAttrValue )
{
    if (XML_TOK_TEXTFIELD_CONDITION == nAttrToken)
    {
        sCondition = sAttrValue;
        bConditionOK = sal_True;
    }
    else
    {
        XMLDatabaseFieldImportContext::ProcessAttribute(nAttrToken,
                                                        sAttrValue);
    }

    bValid = bDatabaseOK && bTableOK;
}

void XMLDatabaseNextImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    aAny <<= bConditionOK ? sCondition : sTrue;
    xPropertySet->setPropertyValue(sPropertyCondition, aAny);

    XMLDatabaseFieldImportContext::PrepareField(xPropertySet);
}



//
// database select field
//

TYPEINIT1( XMLDatabaseSelectImportContext, XMLDatabaseNextImportContext );

XMLDatabaseSelectImportContext::XMLDatabaseSelectImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const ::rtl::OUString& sLocalName) :
        XMLDatabaseNextImportContext(rImport, rHlp, sAPI_database_select,
                                     nPrfx, sLocalName),
        sPropertySetNumber(RTL_CONSTASCII_USTRINGPARAM(sAPI_set_number)),
        nNumber(0),
        bNumberOK(sal_False)
{
}

void XMLDatabaseSelectImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const ::rtl::OUString& sAttrValue )
{
    if (XML_TOK_TEXTFIELD_ROW_NUMBER == nAttrToken)
    {
        sal_Int32 nTmp;
        if (SvXMLUnitConverter::convertNumber( nTmp, sAttrValue
                                               /* , nMin, nMax ??? */ ))
        {
            nNumber = nTmp;
            bNumberOK = sal_True;
        }
    }
    else
    {
        XMLDatabaseNextImportContext::ProcessAttribute(nAttrToken, sAttrValue);
    }

    bValid = bTableOK && bDatabaseOK && bNumberOK;
}

void XMLDatabaseSelectImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    aAny <<= nNumber;
    xPropertySet->setPropertyValue(sPropertySetNumber, aAny);

    XMLDatabaseNextImportContext::PrepareField(xPropertySet);
}



//
// database display row number field
//

TYPEINIT1( XMLDatabaseNumberImportContext, XMLDatabaseFieldImportContext );

XMLDatabaseNumberImportContext::XMLDatabaseNumberImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLDatabaseFieldImportContext(rImport, rHlp, sAPI_database_number,
                                      nPrfx, sLocalName),
        sPropertyNumberingType(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_numbering_type)),
        sPropertySetNumber(RTL_CONSTASCII_USTRINGPARAM(sAPI_set_number)),
        sNumberFormat('1'),
        sNumberSync(RTL_CONSTASCII_USTRINGPARAM(sXML_false)),
        nValue(0),
        bValueOK(sal_False)
{
}

void XMLDatabaseNumberImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_NUM_FORMAT:
            sNumberFormat = sAttrValue;
            break;
        case XML_TOK_TEXTFIELD_NUM_LETTER_SYNC:
            sNumberSync = sAttrValue;
            break;
        case XML_TOK_TEXTFIELD_VALUE:
        {
            sal_Int32 nTmp;
            if (SvXMLUnitConverter::convertNumber( nTmp, sAttrValue ))
            {
                nValue = nTmp;
                bValueOK = sal_True;
            }
            break;
        }
        default:
            XMLDatabaseFieldImportContext::ProcessAttribute(nAttrToken,
                                                            sAttrValue);
            break;
    }

    bValid = bTableOK && bDatabaseOK;
}

void XMLDatabaseNumberImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    sal_Int16 nNumType = style::NumberingType::ARABIC;
    GetImport().GetMM100UnitConverter().convertNumFormat( nNumType,
                                                    sNumberFormat,
                                                    sNumberSync );
    aAny <<= nNumType;
    xPropertySet->setPropertyValue(sPropertyNumberingType, aAny);

    if (bValueOK)
    {
        aAny <<= nValue;
        xPropertySet->setPropertyValue(sPropertySetNumber, aAny);
    }

    XMLDatabaseFieldImportContext::PrepareField(xPropertySet);
}



//
// Simple doc info fields
//

TYPEINIT1( XMLSimpleDocInfoImportContext, XMLTextFieldImportContext );

XMLSimpleDocInfoImportContext::XMLSimpleDocInfoImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName, sal_uInt16 nToken,
    sal_Bool bContent, sal_Bool bAuthor) :
        XMLTextFieldImportContext(rImport, rHlp, MapTokenToServiceName(nToken),
                                  nPrfx, sLocalName),
        bFixed(sal_False),
        bHasAuthor(bAuthor),
        bHasContent(bContent),
        sPropertyFixed(RTL_CONSTASCII_USTRINGPARAM(sAPI_is_fixed)),
        sPropertyContent(RTL_CONSTASCII_USTRINGPARAM(sAPI_content)),
        sPropertyAuthor(RTL_CONSTASCII_USTRINGPARAM(sAPI_author)),
        sPropertyCurrentPresentation(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_current_presentation))
{
    bValid = sal_True;
}

void XMLSimpleDocInfoImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    if (XML_TOK_TEXTFIELD_FIXED == nAttrToken)
    {
        sal_Bool bTmp;
        if (SvXMLUnitConverter::convertBool(bTmp, sAttrValue))
        {
            bFixed = bTmp;
        }
    }
}

void XMLSimpleDocInfoImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;
    aAny.setValue(&bFixed, ::getBooleanCppuType() );
    xPropertySet->setPropertyValue(sPropertyFixed, aAny);

    // set Content (if not fixed) AND CurrentPresentation
    aAny <<= GetContent();

    if (bFixed && bHasAuthor)
    {
        xPropertySet->setPropertyValue(sPropertyAuthor, aAny);
    }

    if (bFixed && bHasContent)
    {
        xPropertySet->setPropertyValue(sPropertyContent, aAny);
    }

    xPropertySet->setPropertyValue(sPropertyCurrentPresentation, aAny);
}

const sal_Char* XMLSimpleDocInfoImportContext::MapTokenToServiceName(
    sal_uInt16 nToken)
{
    const sal_Char* pServiceName = NULL;

    switch(nToken)
    {
        case XML_TOK_TEXT_DOCUMENT_CREATION_AUTHOR:
            pServiceName = sAPI_docinfo_create_author;
            break;
        case XML_TOK_TEXT_DOCUMENT_CREATION_DATE:
            pServiceName = sAPI_docinfo_create_date_time;
            break;
        case XML_TOK_TEXT_DOCUMENT_CREATION_TIME:
            pServiceName = sAPI_docinfo_create_date_time;
            break;
        case XML_TOK_TEXT_DOCUMENT_DESCRIPTION:
            pServiceName = sAPI_docinfo_description;
            break;
        case XML_TOK_TEXT_DOCUMENT_EDIT_DURATION:
            pServiceName = sAPI_docinfo_edit_time;
            break;
        case XML_TOK_TEXT_DOCUMENT_INFORMATION_0:
            pServiceName = sAPI_docinfo_info0;
            break;
        case XML_TOK_TEXT_DOCUMENT_INFORMATION_1:
            pServiceName = sAPI_docinfo_info1;
            break;
        case XML_TOK_TEXT_DOCUMENT_INFORMATION_2:
            pServiceName = sAPI_docinfo_info2;
            break;
        case XML_TOK_TEXT_DOCUMENT_INFORMATION_3:
            pServiceName = sAPI_docinfo_info3;
            break;
        case XML_TOK_TEXT_DOCUMENT_PRINT_AUTHOR:
            pServiceName = sAPI_docinfo_print_author;
            break;
        case XML_TOK_TEXT_DOCUMENT_PRINT_DATE:
            pServiceName = sAPI_docinfo_print_date_time;
            break;
        case XML_TOK_TEXT_DOCUMENT_PRINT_TIME:
            pServiceName = sAPI_docinfo_print_date_time;
            break;
        case XML_TOK_TEXT_DOCUMENT_KEYWORDS:
            pServiceName = sAPI_docinfo_keywords;
            break;
        case XML_TOK_TEXT_DOCUMENT_SUBJECT:
            pServiceName = sAPI_docinfo_subject;
            break;
        case XML_TOK_TEXT_DOCUMENT_REVISION:
            pServiceName = sAPI_docinfo_revision;
            break;
        case XML_TOK_TEXT_DOCUMENT_SAVE_AUTHOR:
            pServiceName = sAPI_docinfo_change_author;
            break;
        case XML_TOK_TEXT_DOCUMENT_SAVE_DATE:
            pServiceName = sAPI_docinfo_change_date_time;
            break;
        case XML_TOK_TEXT_DOCUMENT_SAVE_TIME:
            pServiceName = sAPI_docinfo_change_date_time;
            break;
        case XML_TOK_TEXT_DOCUMENT_TITLE:
            pServiceName = sAPI_docinfo_title;
            break;
        case XML_TOK_TEXT_DOCUMENT_USER_DEFINED:
            // hack: service name not used in XMLUserDocInfoImportContext
            pServiceName = sAPI_docinfo_info0;
            break;

        default:
            DBG_ERROR("no docinfo field token");
            pServiceName = NULL;
            break;
    }

    return pServiceName;
}


//
// revision field
//

TYPEINIT1( XMLRevisionDocInfoImportContext, XMLSimpleDocInfoImportContext );

XMLRevisionDocInfoImportContext::XMLRevisionDocInfoImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName, sal_uInt16 nToken) :
        XMLSimpleDocInfoImportContext(rImport, rHlp, nPrfx, sLocalName,
                                      nToken, sal_False, sal_False),
        sPropertyRevision(RTL_CONSTASCII_USTRINGPARAM(sAPI_revision))
{
    bValid = sal_True;
}

void XMLRevisionDocInfoImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    XMLSimpleDocInfoImportContext::PrepareField(xPropertySet);

    // set revision number if fixed
    if (bFixed)
    {
        sal_Int32 nTmp;

        if (SvXMLUnitConverter::convertNumber(nTmp, GetContent()))
        {
            Any aAny;
            aAny <<= nTmp;
            xPropertySet->setPropertyValue(sPropertyRevision, aAny);
        }
    }
}



//
// DocInfo fields with date/time attributes
//

TYPEINIT1( XMLDateTimeDocInfoImportContext, XMLSimpleDocInfoImportContext );

XMLDateTimeDocInfoImportContext::XMLDateTimeDocInfoImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName, sal_uInt16 nToken) :
        XMLSimpleDocInfoImportContext(rImport, rHlp, nPrfx, sLocalName,
                                      nToken, sal_False, sal_False),
        sPropertyNumberFormat(RTL_CONSTASCII_USTRINGPARAM(sAPI_number_format)),
        sPropertyIsDate(RTL_CONSTASCII_USTRINGPARAM(sAPI_is_date)),
        nFormat(0),
        bFormatOK(sal_False)
{
    // we allow processing of EDIT_DURATION here, because import of actual
    // is not supported anyway. If it was, we'd need an extra import class
    // because times and time durations are presented differently!

    bValid = sal_True;
    switch (nToken)
    {
        case XML_TOK_TEXT_DOCUMENT_CREATION_DATE:
        case XML_TOK_TEXT_DOCUMENT_PRINT_DATE:
        case XML_TOK_TEXT_DOCUMENT_SAVE_DATE:
            bIsDate = sal_True;
            bHasDateTime = sal_True;
            break;
        case XML_TOK_TEXT_DOCUMENT_CREATION_TIME:
        case XML_TOK_TEXT_DOCUMENT_PRINT_TIME:
        case XML_TOK_TEXT_DOCUMENT_SAVE_TIME:
            bIsDate = sal_False;
            bHasDateTime = sal_True;
            break;
        case XML_TOK_TEXT_DOCUMENT_EDIT_DURATION:
            bIsDate = sal_False;
            bHasDateTime = sal_False;
            break;
        default:
            DBG_ERROR(
                "XMLDateTimeDocInfoImportContext needs date/time doc. fields");
            bValid = sal_False;
            break;
    }
}

void XMLDateTimeDocInfoImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_DATA_STYLE_NAME:
        {
            sal_Int32 nKey = GetImportHelper().GetDataStyleKey(sAttrValue);
            if (-1 != nKey)
            {
                nFormat = nKey;
                bFormatOK = sal_True;
            }
            break;
        }
        case XML_TOK_TEXTFIELD_FIXED:
            XMLSimpleDocInfoImportContext::ProcessAttribute(nAttrToken,
                                                            sAttrValue);
            break;
        default:
            // ignore -> we can't set date/time value anyway!
            break;
    }
}

void XMLDateTimeDocInfoImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    // process fixed and presentation
    XMLSimpleDocInfoImportContext::PrepareField(xPropertySet);

    Any aAny;

    if (bHasDateTime)
    {
        aAny.setValue( &bIsDate, ::getBooleanCppuType());
        xPropertySet->setPropertyValue(sPropertyIsDate, aAny);
    }

    if (bFormatOK)
    {
        aAny <<= nFormat;
        xPropertySet->setPropertyValue(sPropertyNumberFormat, aAny);
    }

    // can't set date/time/duration value! Sorry.
}


//
// user defined docinfo fields
//

TYPEINIT1( XMLUserDocInfoImportContext, XMLSimpleDocInfoImportContext );

XMLUserDocInfoImportContext::XMLUserDocInfoImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName, sal_uInt16 nToken) :
        XMLSimpleDocInfoImportContext(rImport, rHlp, nPrfx,
                                      sLocalName, nToken,
                                      sal_False, sal_False)
{
}

void XMLUserDocInfoImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    static const sal_Char* aUserDocInfoServiceNames[] =
    {
        sAPI_docinfo_info0,
        sAPI_docinfo_info1,
        sAPI_docinfo_info2,
        sAPI_docinfo_info3
    };

    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_NAME:
        {
            // iterate over user field names, until match is found
            Reference<XDocumentInfoSupplier> xDocInfoSupp(
                GetImport().GetModel(), UNO_QUERY);
            Reference<XDocumentInfo> xDocInfo =
                xDocInfoSupp->getDocumentInfo();
            sal_Int16 nCount = xDocInfo->getUserFieldCount();
            DBG_ASSERT((nCount*sizeof(sal_Char*) <=
                        sizeof(aUserDocInfoServiceNames)), "unknown service");
            for(sal_Int16 i = 0; i<nCount; i++)
            {
                if (0 == sAttrValue.compareTo(xDocInfo->getUserFieldName(i)))
                {
                    SetServiceName(OUString::createFromAscii(
                        aUserDocInfoServiceNames[i]));
                    bValid = sal_True;
                    break; // for loop; no need to check remainder
                }
            }
            break;
        }

        default:
            XMLSimpleDocInfoImportContext::ProcessAttribute(nAttrToken,
                                                            sAttrValue);
            break;
    }
}



//
// import hidden paragraph fields
//

TYPEINIT1( XMLHiddenParagraphImportContext, XMLTextFieldImportContext );

XMLHiddenParagraphImportContext::XMLHiddenParagraphImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_hidden_paragraph,
                                  nPrfx, sLocalName),
        sPropertyCondition(RTL_CONSTASCII_USTRINGPARAM(sAPI_condition)),
        sCondition()
{
}

void XMLHiddenParagraphImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    if (XML_TOK_TEXTFIELD_CONDITION == nAttrToken)
    {
        sCondition = sAttrValue;
        bValid = sal_True;
    }
}

void XMLHiddenParagraphImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;
    aAny <<= sCondition;
    xPropertySet->setPropertyValue(sPropertyCondition, aAny);
}



//
// import conditional text (<text:conditional-text>)
//

TYPEINIT1( XMLConditionalTextImportContext, XMLTextFieldImportContext );

XMLConditionalTextImportContext::XMLConditionalTextImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_conditional_text,
                                  nPrfx, sLocalName),
        sPropertyCondition(RTL_CONSTASCII_USTRINGPARAM(sAPI_condition)),
        sPropertyTrueContent(RTL_CONSTASCII_USTRINGPARAM(sAPI_true_content)),
        sPropertyFalseContent(RTL_CONSTASCII_USTRINGPARAM(sAPI_false_content)),
        sCondition(),
        sTrueContent(),
        sFalseContent(),
        bConditionOK(sal_False),
        bTrueOK(sal_False),
        bFalseOK(sal_False)
{
}

void XMLConditionalTextImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_CONDITION:
            sCondition = sAttrValue;
            bConditionOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_STRING_VALUE_IF_FALSE:
            sFalseContent = sAttrValue;
            bFalseOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_STRING_VALUE_IF_TRUE:
            sTrueContent = sAttrValue;
            bTrueOK = sal_True;
            break;
    }

    bValid = bConditionOK && bFalseOK && bTrueOK;
}

void XMLConditionalTextImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    aAny <<= sCondition;
    xPropertySet->setPropertyValue(sPropertyCondition, aAny);

    aAny <<= sFalseContent;
    xPropertySet->setPropertyValue(sPropertyFalseContent, aAny);

    aAny <<= sTrueContent;
    xPropertySet->setPropertyValue(sPropertyTrueContent, aAny);
}



//
// hidden text
//

TYPEINIT1( XMLHiddenTextImportContext, XMLTextFieldImportContext);

XMLHiddenTextImportContext::XMLHiddenTextImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_hidden_text,
                                  nPrfx, sLocalName),
        sPropertyCondition(RTL_CONSTASCII_USTRINGPARAM(sAPI_condition)),
        sPropertyContent(RTL_CONSTASCII_USTRINGPARAM(sAPI_content)),
        sCondition(),
        sString(),
        bConditionOK(sal_False),
        bStringOK(sal_False)
{
}

void XMLHiddenTextImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_CONDITION:
            sCondition = sAttrValue;
            bConditionOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_STRING_VALUE:
            sString = sAttrValue;
            bStringOK = sal_True;
            break;
    }

    bValid = bConditionOK && bStringOK;
}

void XMLHiddenTextImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    aAny <<= sCondition;
    xPropertySet->setPropertyValue(sPropertyCondition, aAny);

    aAny <<= sString;
    xPropertySet->setPropertyValue(sPropertyContent, aAny);
}



//
// file name fields
//

TYPEINIT1( XMLFileNameImportContext, XMLTextFieldImportContext );

static const SvXMLEnumMapEntry aFilenameDisplayMap[] =
{
    { sXML_path,                FilenameDisplayFormat::PATH },
    { sXML_name,                FilenameDisplayFormat::NAME },
    { sXML_name_and_extension,  FilenameDisplayFormat::NAME_AND_EXT },
    { sXML_full,                FilenameDisplayFormat::FULL },
    { 0,                        0 }
};

XMLFileNameImportContext::XMLFileNameImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_file_name,
                                  nPrfx, sLocalName),
        sPropertyFixed(RTL_CONSTASCII_USTRINGPARAM(sAPI_is_fixed)),
        sPropertyFileFormat(RTL_CONSTASCII_USTRINGPARAM(sAPI_file_format)),
        sPropertyCurrentPresentation(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_current_presentation)),
        nFormat(FilenameDisplayFormat::FULL),
        bFixed(sal_False)
{
    bValid = sal_True;
}

void XMLFileNameImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const ::rtl::OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_FIXED:
        {
            sal_Bool bTmp;
            if (SvXMLUnitConverter::convertBool(bTmp, sAttrValue))
            {
                bFixed = bTmp;
            }
            break;
        }
        case XML_TOK_TEXTFIELD_DISPLAY:
        {
            sal_uInt16 nTmp;
            if (SvXMLUnitConverter::convertEnum(nTmp, sAttrValue,
                                                aFilenameDisplayMap))
            {
                nFormat = (sal_uInt16)nTmp;
            }
            break;
        }
        default:
            ; // unkown attribute: ignore
            break;
    }
}

void XMLFileNameImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    // properties are optional
    Reference<XPropertySetInfo> xPropertySetInfo(
        xPropertySet->getPropertySetInfo());

//  if (xPropertySetInfo->hasPropertyByName(sPropertyFixed))
//  {
//      aAny <<= bFixed;
//      xPropertySet->setPropertyValue(sPropertyFixed, aAny);
//  }

    if (xPropertySetInfo->hasPropertyByName(sPropertyFileFormat))
    {
        aAny <<= nFormat;
        xPropertySet->setPropertyValue(sPropertyFileFormat, aAny);
    }

    if (xPropertySetInfo->hasPropertyByName(sPropertyCurrentPresentation))
    {
        aAny <<= GetContent();
        xPropertySet->setPropertyValue(sPropertyCurrentPresentation, aAny);
    }
}


//
// template name field
//

static const SvXMLEnumMapEntry aTemplateDisplayMap[] =
{
    { sXML_full,                TemplateDisplayFormat::FULL },
    { sXML_path,                TemplateDisplayFormat::PATH },
    { sXML_name,                TemplateDisplayFormat::NAME },
    { sXML_name_and_extension,  TemplateDisplayFormat::NAME_AND_EXT },
    { sXML_area,                TemplateDisplayFormat::AREA },
    { sXML_title,               TemplateDisplayFormat::TITLE },
    { 0,                        0 }
};

TYPEINIT1( XMLTemplateNameImportContext, XMLTextFieldImportContext );

XMLTemplateNameImportContext::XMLTemplateNameImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_template_name,
                                  nPrfx, sLocalName),
        sPropertyFileFormat(RTL_CONSTASCII_USTRINGPARAM(sAPI_file_format)),
        nFormat(TemplateDisplayFormat::FULL)
{
    bValid = sal_True;
}

void XMLTemplateNameImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_DISPLAY:
        {
            sal_uInt16 nTmp;
            if (SvXMLUnitConverter::convertEnum(nTmp, sAttrValue,
                                                aTemplateDisplayMap))
            {
                nFormat = (sal_uInt16)nTmp;
            }
            break;
        }
        default:
            ; // unknown attribute: ignore
            break;
    }
}

void XMLTemplateNameImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    aAny <<= nFormat;
    xPropertySet->setPropertyValue(sPropertyFileFormat, aAny);
}


//
// import chapter fields
//

TYPEINIT1( XMLChapterImportContext, XMLTextFieldImportContext );

static const SvXMLEnumMapEntry aChapterDisplayMap[] =
{
    { sXML_name,                    ChapterFormat::NAME },
    { sXML_number,                  ChapterFormat::NUMBER },
    { sXML_number_and_name,         ChapterFormat::NAME_NUMBER },
    { sXML_plain_number_and_name,   ChapterFormat::NO_PREFIX_SUFFIX },
    { sXML_plain_number,            ChapterFormat::DIGIT },
    { 0,                            0 }
};

XMLChapterImportContext::XMLChapterImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_chapter,
                                  nPrfx, sLocalName),
        sPropertyChapterFormat(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_chapter_format)),
        sPropertyLevel(RTL_CONSTASCII_USTRINGPARAM(sAPI_level)),
        nFormat(ChapterFormat::NAME_NUMBER),
        nLevel(0)
{
    bValid = sal_True;
}

void XMLChapterImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_DISPLAY:
        {
            sal_uInt16 nTmp;
            if (SvXMLUnitConverter::convertEnum(nTmp, sAttrValue,
                                                aChapterDisplayMap))
            {
                nFormat = (sal_Int16)nTmp;
            }
            break;
        }
        case XML_TOK_TEXTFIELD_OUTLINE_LEVEL:
        {
            sal_Int32 nTmp;
            if (SvXMLUnitConverter::convertNumber(
                nTmp, sAttrValue, 1,
                GetImport().GetTextImport()->GetChapterNumbering()->getCount()
                ))
            {
                // API numbers 0..9, we number 1..10
                nLevel = (sal_Int8)nTmp;
                nLevel--;
            }
            break;
        }
        default:
            ; // unknown attribute: ignore
            break;
    }
}

void XMLChapterImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    aAny <<= nFormat;
    xPropertySet->setPropertyValue(sPropertyChapterFormat, aAny);

    aAny <<= nLevel;
    xPropertySet->setPropertyValue(sPropertyLevel, aAny);
}


//
// counting fields
//

TYPEINIT1( XMLCountFieldImportContext, XMLTextFieldImportContext );

XMLCountFieldImportContext::XMLCountFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName, sal_uInt16 nToken) :
        XMLTextFieldImportContext(rImport, rHlp, MapTokenToServiceName(nToken),
                                  nPrfx, sLocalName),
        sPropertyNumberingType(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_numbering_type)),
        sNumberFormat(),
        sLetterSync(),
        bNumberFormatOK(sal_False)
{
    bValid = sal_True;
}

void XMLCountFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_NUM_FORMAT:
            sNumberFormat = sAttrValue;
            bNumberFormatOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_NUM_LETTER_SYNC:
            sLetterSync = sAttrValue;
            break;
    }
}

void XMLCountFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    // properties optional
    // (only page count, but do for all to save common implementation)

    if (xPropertySet->getPropertySetInfo()->
        hasPropertyByName(sPropertyNumberingType))
    {
        sal_Int16 nNumType;
        if( bNumberFormatOK )
        {
            nNumType= style::NumberingType::ARABIC;
            GetImport().GetMM100UnitConverter().convertNumFormat( nNumType,
                                                    sNumberFormat,
                                                    sLetterSync );
        }
        else
            nNumType = style::NumberingType::PAGE_DESCRIPTOR;
        aAny <<= nNumType;
        xPropertySet->setPropertyValue(sPropertyNumberingType, aAny);
    }
}

const sal_Char* XMLCountFieldImportContext::MapTokenToServiceName(
    sal_uInt16 nToken)
{
    const sal_Char* pServiceName = NULL;

    switch (nToken)
    {
        case XML_TOK_TEXT_WORD_COUNT:
            pServiceName = sAPI_word_count;
            break;
        case XML_TOK_TEXT_PARAGRAPH_COUNT:
            pServiceName = sAPI_paragraph_count;
            break;
        case XML_TOK_TEXT_TABLE_COUNT:
            pServiceName = sAPI_table_count;
            break;
        case XML_TOK_TEXT_CHARACTER_COUNT:
            pServiceName = sAPI_character_count;
            break;
        case XML_TOK_TEXT_IMAGE_COUNT:
            pServiceName = sAPI_graphic_object_count;
            break;
        case XML_TOK_TEXT_OBJECT_COUNT:
            pServiceName = sAPI_embedded_object_count;
            break;
        case XML_TOK_TEXT_PAGE_COUNT:
            pServiceName = sAPI_page_count;
            break;
        default:
            pServiceName = NULL;
            DBG_ERROR("unknown count field!");
            break;
    }

    return pServiceName;
}



//
// page variable import
//

TYPEINIT1( XMLPageVarGetFieldImportContext, XMLTextFieldImportContext );

XMLPageVarGetFieldImportContext::XMLPageVarGetFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_reference_page_get,
                                  nPrfx, sLocalName),
        sPropertyNumberingType(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_numbering_type)),
        sNumberFormat(),
        sLetterSync(),
        bNumberFormatOK(sal_False)
{
    bValid = sal_True;
}

void XMLPageVarGetFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_NUM_FORMAT:
            sNumberFormat = sAttrValue;
            bNumberFormatOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_NUM_LETTER_SYNC:
            sLetterSync = sAttrValue;
            break;
    }
}

void XMLPageVarGetFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    sal_Int16 nNumType;
    if( bNumberFormatOK )
    {
        nNumType= style::NumberingType::ARABIC;
        GetImport().GetMM100UnitConverter().convertNumFormat( nNumType,
                                                    sNumberFormat,
                                                    sLetterSync );
    }
    else
        nNumType = style::NumberingType::PAGE_DESCRIPTOR;
    aAny <<= nNumType;
    xPropertySet->setPropertyValue(sPropertyNumberingType, aAny);
}



//
// page variable set fields
//

TYPEINIT1(XMLPageVarSetFieldImportContext, XMLTextFieldImportContext);

XMLPageVarSetFieldImportContext::XMLPageVarSetFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_reference_page_set,
                                  nPrfx, sLocalName),
        sPropertyOn(RTL_CONSTASCII_USTRINGPARAM(sAPI_on)),
        sPropertyOffset(RTL_CONSTASCII_USTRINGPARAM(sAPI_offset)),
        nAdjust(0),
        bActive(sal_True)
{
    bValid = sal_True;
}

void XMLPageVarSetFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const ::rtl::OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_ACTIVE:
        {
            sal_Bool bTmp;
            if (SvXMLUnitConverter::convertBool(bTmp, sAttrValue))
            {
                bActive = bTmp;
            }
        }
        case XML_TOK_TEXTFIELD_PAGE_ADJUST:
        {
            sal_Int32 nTmp;
            if (SvXMLUnitConverter::convertNumber(nTmp, sAttrValue))
            {
                nAdjust = (sal_Int16)nTmp;
            }
        }
    }
}

void XMLPageVarSetFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    aAny.setValue(&bActive, ::getBooleanCppuType());
    xPropertySet->setPropertyValue(sPropertyOn, aAny);

    aAny <<= nAdjust;
    xPropertySet->setPropertyValue(sPropertyOffset, aAny);
}



//
// macro fields
//

TYPEINIT1( XMLMacroFieldImportContext, XMLTextFieldImportContext );

XMLMacroFieldImportContext::XMLMacroFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_macro,
                                  nPrfx, sLocalName),
        sPropertyHint(RTL_CONSTASCII_USTRINGPARAM(sAPI_hint)),
        sPropertyMacro(RTL_CONSTASCII_USTRINGPARAM(sAPI_macro)),
        sMacro(),
        sDescription(),
        bDescriptionOK(sal_False)
{
}

void XMLMacroFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_DESCRIPTION:
            sDescription = sAttrValue;
            bDescriptionOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_NAME:
            sMacro = sAttrValue;
            bValid = sal_True;
            break;
    }
}

void XMLMacroFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    aAny <<= sMacro;
    xPropertySet->setPropertyValue(sPropertyMacro, aAny);

    aAny <<= (bDescriptionOK ? sDescription : GetContent());
    xPropertySet->setPropertyValue(sPropertyHint, aAny);
}



//
// reference field import
//

TYPEINIT1( XMLReferenceFieldImportContext, XMLTextFieldImportContext );

XMLReferenceFieldImportContext::XMLReferenceFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nToken, sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_get_reference,
                                  nPrfx, sLocalName),
        nType(ReferenceFieldPart::PAGE_DESC),
        sName(),
        bNameOK(sal_False),
        bTypeOK(sal_False),
        bSeqNumberOK(sal_False),
        nElementToken(nToken),
        sPropertyReferenceFieldPart(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_reference_field_part)),
        sPropertyReferenceFieldSource(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_reference_field_source)),
        sPropertySourceName(RTL_CONSTASCII_USTRINGPARAM(sAPI_source_name)),
        sPropertyCurrentPresentation(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_current_presentation))
{
}

static SvXMLEnumMapEntry __READONLY_DATA lcl_aReferenceTypeTokenMap[] =
{
    { sXML_page,        ReferenceFieldPart::PAGE},
    { sXML_chapter,     ReferenceFieldPart::CHAPTER },
    { sXML_text,        ReferenceFieldPart::TEXT },
    { sXML_direction,   ReferenceFieldPart::UP_DOWN },
    { sXML_category_and_value, ReferenceFieldPart::CATEGORY_AND_NUMBER },
    { sXML_caption,     ReferenceFieldPart::ONLY_CAPTION },
    { sXML_value,       ReferenceFieldPart::ONLY_SEQUENCE_NUMBER },
    { 0,                0 }
};

void XMLReferenceFieldImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    bTypeOK = sal_True;
    switch (nElementToken)
    {
        case XML_TOK_TEXT_REFERENCE_REF:
            nSource = ReferenceFieldSource::REFERENCE_MARK;
            break;
        case XML_TOK_TEXT_BOOKMARK_REF:
            nSource = ReferenceFieldSource::BOOKMARK;
            break;
        case XML_TOK_TEXT_FOOTNOTE_REF:
            nSource = ReferenceFieldSource::FOOTNOTE;
            break;
        case XML_TOK_TEXT_ENDNOTE_REF:
            nSource = ReferenceFieldSource::ENDNOTE;
            break;
        case XML_TOK_TEXT_SEQUENCE_REF:
            nSource = ReferenceFieldSource::SEQUENCE_FIELD;
            break;
        default:
            bTypeOK = sal_False;
            DBG_ERROR("unknown reference field");
            break;
    }

    XMLTextFieldImportContext::StartElement(xAttrList);
}


void XMLReferenceFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_REF_NAME:
            sName = sAttrValue;
            bNameOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_REFERENCE_FORMAT:
        {
            sal_uInt16 nToken;
            if (SvXMLUnitConverter::convertEnum(nToken, sAttrValue,
                                                lcl_aReferenceTypeTokenMap))
            {
                nType = nToken;
            }

            // check for sequence-only-attributes
            if ( (XML_TOK_TEXT_SEQUENCE_REF != nElementToken) &&
                 ( (nType == ReferenceFieldPart::CATEGORY_AND_NUMBER) ||
                   (nType == ReferenceFieldPart::ONLY_CAPTION) ||
                   (nType == ReferenceFieldPart::ONLY_SEQUENCE_NUMBER) ) )
            {
                nType = ReferenceFieldPart::PAGE_DESC;
            }

            break;
        }
    }

    // bValid: we need proper element type and name
    bValid = bTypeOK && bNameOK;
}

void XMLReferenceFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    aAny <<= nType;
    xPropertySet->setPropertyValue(sPropertyReferenceFieldPart, aAny);

    aAny <<= nSource;
    xPropertySet->setPropertyValue(sPropertyReferenceFieldSource, aAny);

    switch (nElementToken)
    {
        case XML_TOK_TEXT_REFERENCE_REF:
        case XML_TOK_TEXT_BOOKMARK_REF:
            aAny <<= sName;
            xPropertySet->setPropertyValue(sPropertySourceName, aAny);
            break;

        case XML_TOK_TEXT_FOOTNOTE_REF:
        case XML_TOK_TEXT_ENDNOTE_REF:
            GetImportHelper().ProcessFootnoteReference(sName, xPropertySet);
            break;

        case XML_TOK_TEXT_SEQUENCE_REF:
            GetImportHelper().ProcessSequenceReference(sName, xPropertySet);
            break;
    }

    aAny <<= GetContent();
    xPropertySet->setPropertyValue(sPropertyCurrentPresentation, aAny);
}



//
// field declarations container
//

enum DdeFieldDeclAttrs
{
    XML_TOK_DDEFIELD_NAME,
    XML_TOK_DDEFIELD_APPLICATION,
    XML_TOK_DDEFIELD_TOPIC,
    XML_TOK_DDEFIELD_ITEM,
    XML_TOK_DDEFIELD_UPDATE
};

static __FAR_DATA SvXMLTokenMapEntry aDdeDeclAttrTokenMap[] =
{
    { XML_NAMESPACE_TEXT, sXML_name, XML_TOK_DDEFIELD_NAME },
    { XML_NAMESPACE_OFFICE, sXML_dde_application, XML_TOK_DDEFIELD_APPLICATION },
    { XML_NAMESPACE_OFFICE, sXML_dde_topic, XML_TOK_DDEFIELD_TOPIC },
    { XML_NAMESPACE_OFFICE, sXML_dde_item, XML_TOK_DDEFIELD_ITEM },
    { XML_NAMESPACE_OFFICE, sXML_automatic_update, XML_TOK_DDEFIELD_UPDATE },
    XML_TOKEN_MAP_END
};

TYPEINIT1( XMLDdeFieldDeclsImportContext, SvXMLImportContext );

XMLDdeFieldDeclsImportContext::XMLDdeFieldDeclsImportContext(
    SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& sLocalName) :
        SvXMLImportContext(rImport, nPrfx, sLocalName),
        aTokenMap(aDdeDeclAttrTokenMap)
{
}

SvXMLImportContext * XMLDdeFieldDeclsImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    if ( (XML_NAMESPACE_TEXT == nPrefix) &&
         (0 == rLocalName.compareToAscii(sXML_dde_connection_decl)) )
    {
        return new XMLDdeFieldDeclImportContext(GetImport(), nPrefix,
                                                rLocalName, aTokenMap);
    }
    else
    {
        return SvXMLImportContext::CreateChildContext(nPrefix,
                                                      rLocalName,
                                                      xAttrList);
    }
}



//
// import dde field declaration
//

TYPEINIT1( XMLDdeFieldDeclImportContext, SvXMLImportContext );

XMLDdeFieldDeclImportContext::XMLDdeFieldDeclImportContext(
    SvXMLImport& rImport, sal_uInt16 nPrfx,
    const OUString& sLocalName, const SvXMLTokenMap& rMap) :
        SvXMLImportContext(rImport, nPrfx, sLocalName),
        rTokenMap(rMap),
        sPropertyName(RTL_CONSTASCII_USTRINGPARAM(sAPI_name)),
        sPropertyDDECommandType(RTL_CONSTASCII_USTRINGPARAM(sAPI_dde_command_type)),
        sPropertyDDECommandFile(RTL_CONSTASCII_USTRINGPARAM(sAPI_dde_command_file)),
        sPropertyDDECommandElement(RTL_CONSTASCII_USTRINGPARAM(sAPI_dde_command_element)),
        sPropertyIsAutomaticUpdate(
            RTL_CONSTASCII_USTRINGPARAM(sAPI_is_automatic_update))
{
    DBG_ASSERT(XML_NAMESPACE_TEXT == nPrfx, "wrong prefix");
    DBG_ASSERT(0 == sLocalName.compareToAscii(sXML_dde_connection_decl),
               "wrong name");
}

void XMLDdeFieldDeclImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    OUString sName;
    OUString sCommandApplication;
    OUString sCommandTopic;
    OUString sCommandItem;

    sal_Bool bUpdate = sal_False;
    sal_Bool bNameOK = sal_False;
    sal_Bool bCommandApplicationOK = sal_False;
    sal_Bool bCommandTopicOK = sal_False;
    sal_Bool bCommandItemOK = sal_False;

    // process attributes
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 i=0; i<nLength; i++)
    {

        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );

        switch (rTokenMap.Get(nPrefix, sLocalName))
        {
            case XML_TOK_DDEFIELD_NAME:
                sName = xAttrList->getValueByIndex(i);
                bNameOK = sal_True;
                break;
            case XML_TOK_DDEFIELD_APPLICATION:
                sCommandApplication = xAttrList->getValueByIndex(i);
                bCommandApplicationOK = sal_True;
                break;
            case XML_TOK_DDEFIELD_TOPIC:
                sCommandTopic = xAttrList->getValueByIndex(i);
                bCommandTopicOK = sal_True;
                break;
            case XML_TOK_DDEFIELD_ITEM:
                sCommandItem = xAttrList->getValueByIndex(i);
                bCommandItemOK = sal_True;
                break;
            case XML_TOK_DDEFIELD_UPDATE:
            {
                sal_Bool bTmp;
                if ( SvXMLUnitConverter::convertBool(
                    bTmp, xAttrList->getValueByIndex(i)) )
                {
                    bUpdate = bTmp;
                }
                break;
            }
        }
    }

    // valid data?
    if (bNameOK && bCommandApplicationOK && bCommandTopicOK && bCommandItemOK)
    {
        // make service name
        OUStringBuffer sBuf;
        sBuf.appendAscii(sAPI_fieldmaster_prefix);
        sBuf.appendAscii(sAPI_dde);

        // create DDE TextFieldMaster
        Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),
                                                 UNO_QUERY);
        if( xFactory.is() )
        {
            Reference<XInterface> xIfc =
                xFactory->createInstance(sBuf.makeStringAndClear());
            if( xIfc.is() )
            {
                Reference<XPropertySet> xPropSet( xIfc, UNO_QUERY );
                if (xPropSet.is() &&
                    xPropSet->getPropertySetInfo()->hasPropertyByName(
                        sPropertyDDECommandType))
                {
                    Any aAny;

                    aAny <<= sName;
                    xPropSet->setPropertyValue(sPropertyName, aAny);

                    aAny <<= sCommandApplication;
                    xPropSet->setPropertyValue(sPropertyDDECommandType, aAny);

                    aAny <<= sCommandTopic;
                    xPropSet->setPropertyValue(sPropertyDDECommandFile, aAny);

                    aAny <<= sCommandItem;
                    xPropSet->setPropertyValue(sPropertyDDECommandElement,
                                               aAny);

                    aAny.setValue(&bUpdate, ::getBooleanCppuType());
                    xPropSet->setPropertyValue(sPropertyIsAutomaticUpdate,
                                               aAny);
                }
                // else: ignore (can't get XPropertySet, or DDE
                //               properties are not supported)
            }
            // else: ignore
        }
        // else: ignore
    }
    // else: ignore
}



//
// DDE field import
//

TYPEINIT1( XMLDdeFieldImportContext, XMLTextFieldImportContext );

XMLDdeFieldImportContext::XMLDdeFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_dde,
                                  nPrfx, sLocalName),
        sName()
{
}

void XMLDdeFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const ::rtl::OUString& sAttrValue )
{
    if (XML_TOK_TEXTFIELD_CONNECTION_NAME == nAttrToken)
    {
        sName = sAttrValue;
        bValid = sal_True;
    }
}

void XMLDdeFieldImportContext::EndElement()
{
    if (bValid)
    {
        // find master
        OUStringBuffer sBuf;
        sBuf.appendAscii(sAPI_fieldmaster_prefix);
        sBuf.appendAscii(sAPI_dde);
        sBuf.append(sal_Unicode('.'));
        sBuf.append(sName);
        OUString sMasterName = sBuf.makeStringAndClear();

        Reference<XTextFieldsSupplier> xTextFieldsSupp(GetImport().GetModel(),
                                                       UNO_QUERY);
        Reference<container::XNameAccess> xFieldMasterNameAccess(
            xTextFieldsSupp->getTextFieldMasters(), UNO_QUERY);

        if (xFieldMasterNameAccess->hasByName(sMasterName))
        {
            Reference<XPropertySet> xMaster;
            Any aAny = xFieldMasterNameAccess->getByName(sMasterName);
            aAny >>= xMaster;

            // master exists: create text field and attach
            Reference<XPropertySet> xField;
            sBuf.appendAscii(sAPI_textfield_prefix);
            sBuf.appendAscii(sAPI_dde);
            if (CreateField(xField, sBuf.makeStringAndClear()))
            {
                Reference<XDependentTextField> xDepTextField(xField,UNO_QUERY);
                xDepTextField->attachTextFieldMaster(xMaster);

                // attach field to document
                Reference<XTextContent> xTextContent(xField, UNO_QUERY);
                if (xTextContent.is())
                {
                    GetImportHelper().InsertTextContent(xTextContent);

                    // we're lucky. nothing else to prepare.
                }
                // else: fail, because text content could not be created
            }
            // else: fail, because field could not be created
        }
        // else: fail, because no master was found (faulty document?!)
    }
    // not valid: ignore
}

void XMLDdeFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    // empty, since not needed.
}


//
// sheet name fields
//

TYPEINIT1(XMLSheetNameImportContext, XMLTextFieldImportContext);

XMLSheetNameImportContext::XMLSheetNameImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_sheet_name,
                                  nPrfx, sLocalName)
{
    bValid = sal_True;  // always valid!
}

void XMLSheetNameImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const ::rtl::OUString& sAttrValue )
{
    // no attributes -> nothing to be done
}

void XMLSheetNameImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    // no attributes -> nothing to be done
}


//
// URL fields (Calc, Impress, Draw)
//

TYPEINIT1(XMLUrlFieldImportContext, XMLTextFieldImportContext);

XMLUrlFieldImportContext::XMLUrlFieldImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_url,
                                  nPrfx, sLocalName),
        sPropertyURL(RTL_CONSTASCII_USTRINGPARAM(sAPI_url)),
        sPropertyTargetFrame(RTL_CONSTASCII_USTRINGPARAM(sAPI_target_frame)),
        sPropertyRepresentation(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_representation)),
        bFrameOK(sal_False)
{
}

void XMLUrlFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_HREF:
            sURL = sAttrValue;
            bValid = sal_True;
            break;
        case XML_TOK_TEXTFIELD_TARGET_FRAME:
            sFrame = sAttrValue;
            bFrameOK = sal_True;
            break;
        default:
            // ignore
            break;
    }
}

void XMLUrlFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    aAny <<= sURL;
    xPropertySet->setPropertyValue(sPropertyURL, aAny);

    if (bFrameOK)
    {
        aAny <<= sFrame;
        xPropertySet->setPropertyValue(sPropertyTargetFrame, aAny);
    }

    aAny <<= GetContent();
    xPropertySet->setPropertyValue(sPropertyRepresentation, aAny);
}


TYPEINIT1(XMLBibliographyFieldImportContext, XMLTextFieldImportContext);


XMLBibliographyFieldImportContext::XMLBibliographyFieldImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_bibliography,
                                  nPrfx, sLocalName),
        sPropertyFields(RTL_CONSTASCII_USTRINGPARAM("Fields")),
        aValues()
{
    bValid = sal_True;
}

// TODO: this is the same map as is used in XMLSectionExport; we need only one copy.
SvXMLEnumMapEntry __READONLY_DATA aBibliographyDataFieldMap[] =
{
    { sXML_address,             BibliographyDataField::ADDRESS },
    { sXML_annote,              BibliographyDataField::ANNOTE },
    { sXML_author,              BibliographyDataField::AUTHOR },
    { sXML_bibiliographic_type, BibliographyDataField::BIBILIOGRAPHIC_TYPE },
    { sXML_booktitle,           BibliographyDataField::BOOKTITLE },
    { sXML_chapter,             BibliographyDataField::CHAPTER },
    { sXML_custom1,             BibliographyDataField::CUSTOM1 },
    { sXML_custom2,             BibliographyDataField::CUSTOM2 },
    { sXML_custom3,             BibliographyDataField::CUSTOM3 },
    { sXML_custom4,             BibliographyDataField::CUSTOM4 },
    { sXML_custom5,             BibliographyDataField::CUSTOM5 },
    { sXML_edition,             BibliographyDataField::EDITION },
    { sXML_editor,              BibliographyDataField::EDITOR },
    { sXML_howpublished,        BibliographyDataField::HOWPUBLISHED },
    { sXML_identifier,          BibliographyDataField::IDENTIFIER },
    { sXML_institution,         BibliographyDataField::INSTITUTION },
    { sXML_isbn,                BibliographyDataField::ISBN },
    { sXML_journal,             BibliographyDataField::JOURNAL },
    { sXML_month,               BibliographyDataField::MONTH },
    { sXML_note,                BibliographyDataField::NOTE },
    { sXML_number,              BibliographyDataField::NUMBER },
    { sXML_organizations,       BibliographyDataField::ORGANIZATIONS },
    { sXML_pages,               BibliographyDataField::PAGES },
    { sXML_publisher,           BibliographyDataField::PUBLISHER },
    { sXML_report_type,         BibliographyDataField::REPORT_TYPE },
    { sXML_school,              BibliographyDataField::SCHOOL },
    { sXML_series,              BibliographyDataField::SERIES },
    { sXML_title,               BibliographyDataField::TITLE },
    { sXML_url,                 BibliographyDataField::URL },
    { sXML_volume,              BibliographyDataField::VOLUME },
    { sXML_year,                BibliographyDataField::YEAR },
    { NULL, NULL }
};

// we'll process attributes on our own and forfit the standard
// tecfield mechanism, because our attributes have zero overlp with
// all the oher textfields.
void XMLBibliographyFieldImportContext::StartElement(
        const Reference<XAttributeList> & xAttrList)
{
    // iterate over attributes
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 i=0; i<nLength; i++) {

        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );

        if (nPrefix == XML_NAMESPACE_TEXT)
        {
            PropertyValue aValue;
            aValue.Name = OUString::createFromAscii(
                MapBibliographyFieldName(sLocalName));
            Any aAny;

            // special treatment for bibliographic type
            if (sLocalName.equalsAsciiL(sXML_bibiliographic_type,
                                        sizeof(sXML_bibiliographic_type)-1))
            {
                sal_uInt16 nTmp;
                if (SvXMLUnitConverter::convertEnum(nTmp, sLocalName,
                                                    aBibliographyDataFieldMap))
                {
                    aAny <<= (sal_Int16)nTmp;
                    aValue.Value = aAny;

                    aValues.push_back(aValue);
                }
            }
            else
            {
                aAny <<= xAttrList->getValueByIndex(i);
                aValue.Value = aAny;

                aValues.push_back(aValue);
            }
        }
        // else: unknown namespace -> ignore
    }
}

void XMLBibliographyFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    // attributes are handled in StartElement
    DBG_ERROR("This should not have happened.");
}


void XMLBibliographyFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    // convert vector into sequence
    sal_Int32 nCount = aValues.size();
    Sequence<PropertyValue> aValueSequence(nCount);
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        aValueSequence[i] = aValues[i];
    }

    // set sequence
    Any aAny;
    aAny <<= aValueSequence;
    xPropertySet->setPropertyValue(sPropertyFields, aAny);
}

const sal_Char* XMLBibliographyFieldImportContext::MapBibliographyFieldName(
    OUString sName)
{
    sal_Char* pName = NULL;

    if (sName.equalsAsciiL(sXML_identifier, sizeof(sXML_identifier)-1))
    {
        pName = "Identifier";
    }
    else if (sName.equalsAsciiL(sXML_bibiliographic_type,
                                sizeof(sXML_bibiliographic_type)-1))
    {
        pName = "BibiliographicType";
    }
    else if (sName.equalsAsciiL(sXML_address, sizeof(sXML_address)-1))
    {
        pName = "Address";
    }
    else if (sName.equalsAsciiL(sXML_annote, sizeof(sXML_annote)-1))
    {
        pName = "Annote";
    }
    else if (sName.equalsAsciiL(sXML_author, sizeof(sXML_author)-1))
    {
        pName = "Author";
    }
    else if (sName.equalsAsciiL(sXML_booktitle, sizeof(sXML_booktitle)-1))
    {
        pName = "Booktitle";
    }
    else if (sName.equalsAsciiL(sXML_chapter, sizeof(sXML_chapter)-1))
    {
        pName = "Chapter";
    }
    else if (sName.equalsAsciiL(sXML_edition, sizeof(sXML_edition)-1))
    {
        pName = "Edition";
    }
    else if (sName.equalsAsciiL(sXML_editor, sizeof(sXML_editor)-1))
    {
        pName = "Editor";
    }
    else if (sName.equalsAsciiL(sXML_howpublished,sizeof(sXML_howpublished)-1))
    {
        pName = "Howpublished";
    }
    else if (sName.equalsAsciiL(sXML_institution, sizeof(sXML_institution)-1))
    {
        pName = "Institution";
    }
    else if (sName.equalsAsciiL(sXML_journal, sizeof(sXML_journal)-1))
    {
        pName = "Journal";
    }
    else if (sName.equalsAsciiL(sXML_month, sizeof(sXML_month)-1))
    {
        pName = "Month";
    }
    else if (sName.equalsAsciiL(sXML_note, sizeof(sXML_note)-1))
    {
        pName = "Note";
    }
    else if (sName.equalsAsciiL(sXML_number, sizeof(sXML_number)-1))
    {
        pName = "Number";
    }
    else if (sName.equalsAsciiL(sXML_organizations,
                                sizeof(sXML_organizations)-1))
    {
        pName = "Organizations";
    }
    else if (sName.equalsAsciiL(sXML_pages, sizeof(sXML_pages)-1))
    {
        pName = "Pages";
    }
    else if (sName.equalsAsciiL(sXML_publisher, sizeof(sXML_publisher)-1))
    {
        pName = "Publisher";
    }
    else if (sName.equalsAsciiL(sXML_school, sizeof(sXML_school)-1))
    {
        pName = "School";
    }
    else if (sName.equalsAsciiL(sXML_series, sizeof(sXML_series)-1))
    {
        pName = "Series";
    }
    else if (sName.equalsAsciiL(sXML_title, sizeof(sXML_title)-1))
    {
        pName = "Title";
    }
    else if (sName.equalsAsciiL(sXML_report_type, sizeof(sXML_report_type)-1))
    {
        pName = "Report_Type";
    }
    else if (sName.equalsAsciiL(sXML_volume, sizeof(sXML_volume)-1))
    {
        pName = "Volume";
    }
    else if (sName.equalsAsciiL(sXML_year, sizeof(sXML_year)-1))
    {
        pName = "Year";
    }
    else if (sName.equalsAsciiL(sXML_url, sizeof(sXML_url)-1))
    {
        pName = "URL";
    }
    else if (sName.equalsAsciiL(sXML_custom1, sizeof(sXML_custom1)-1))
    {
        pName = "Custom1";
    }
    else if (sName.equalsAsciiL(sXML_custom2, sizeof(sXML_custom2)-1))
    {
        pName = "Custom2";
    }
    else if (sName.equalsAsciiL(sXML_custom3, sizeof(sXML_custom3)-1))
    {
        pName = "Custom3";
    }
    else if (sName.equalsAsciiL(sXML_custom4, sizeof(sXML_custom4)-1))
    {
        pName = "Custom4";
    }
    else if (sName.equalsAsciiL(sXML_custom5, sizeof(sXML_custom5)-1))
    {
        pName = "Custom5";
    }
    else if (sName.equalsAsciiL(sXML_isbn, sizeof(sXML_isbn)-1))
    {
        pName = "ISBN";
    }
    else
    {
        DBG_ERROR("Unknown bibliography info data");
        pName = NULL;
    }

    return pName;
}


//
// Annotation Field
//

TYPEINIT1(XMLAnnotationImportContext, XMLTextFieldImportContext);

XMLAnnotationImportContext::XMLAnnotationImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_annotation,
                                  nPrfx, sLocalName),
        sPropertyAuthor(RTL_CONSTASCII_USTRINGPARAM(sAPI_author)),
        sPropertyContent(RTL_CONSTASCII_USTRINGPARAM(sAPI_content)),
        sPropertyDate(RTL_CONSTASCII_USTRINGPARAM(sAPI_date)),
        bDateOK(sal_False)
{
    bValid = sal_True;
}

void XMLAnnotationImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_OFFICE_CREATE_DATE:
        {
            DateTime aDateTime;
            if (SvXMLUnitConverter::convertDateTime(aDateTime, sAttrValue))
            {
                aDate.Year = aDateTime.Year;
                aDate.Month = aDateTime.Month;
                aDate.Day = aDateTime.Day;
                bDateOK = sal_True;
            }
            break;
        }

        case XML_TOK_TEXTFIELD_OFFICE_AUTHOR:
            sAuthor = sAttrValue;
            break;

        default:
            // ignore
            break;
    }
}

SvXMLImportContext* XMLAnnotationImportContext::CreateChildContext(
    USHORT nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList >& xAttrList )
{
    return new XMLStringBufferImportContext(GetImport(), nPrefix,
                                            rLocalName, aTextBuffer);
}

void XMLAnnotationImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    // import (possibly empty) author
    aAny <<= sAuthor;
    xPropertySet->setPropertyValue(sPropertyAuthor, aAny);

    if (bDateOK)
    {
        aAny <<= aDate;
        xPropertySet->setPropertyValue(sPropertyDate, aAny);
    }

    // delete last paragraph mark (if necessary)
    OUString sBuffer = aTextBuffer.makeStringAndClear();
    if (sal_Char(0x0a) == sBuffer.getStr()[sBuffer.getLength()-1])
    {
        sBuffer = sBuffer.copy(0, sBuffer.getLength()-1);
    }
    aAny <<= sBuffer;
    xPropertySet->setPropertyValue(sPropertyContent, aAny);
}



//
// script field
//

TYPEINIT1(XMLScriptImportContext, XMLTextFieldImportContext);

XMLScriptImportContext::XMLScriptImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_script,
                                  nPrfx, sLocalName),
        sPropertyContent(RTL_CONSTASCII_USTRINGPARAM(sAPI_content)),
        sPropertyScriptType(RTL_CONSTASCII_USTRINGPARAM(sAPI_script_type)),
        sPropertyURLContent(RTL_CONSTASCII_USTRINGPARAM(sAPI_url_content)),
        bContentOK(sal_False),
        bUrlContent(sal_False),
        bScriptTypeOK(sal_False)
{
}

void XMLScriptImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_HREF:
            sContent = sAttrValue;
            bContentOK = sal_True;
            break;

        case XML_TOK_TEXTFIELD_LANGUAGE:
            sScriptType = sAttrValue;
            bScriptTypeOK = sal_True;
            break;

        default:
            // ignore
            break;
    }

    // validity depends only on script type
    bValid = bScriptTypeOK;
}

void XMLScriptImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    // if href attribute was present, we use it. Else we use element content
    if (! bContentOK)
    {
        sContent = GetContent();
    }
    aAny <<= sContent;
    xPropertySet->setPropertyValue(sPropertyContent, aAny);

    // URL or script text? We use URL if we have an href-attribute
    aAny.setValue(&bContentOK, ::getBooleanCppuType());
    xPropertySet->setPropertyValue(sPropertyURLContent, aAny);

    aAny <<= sScriptType;
    xPropertySet->setPropertyValue(sPropertyScriptType, aAny);
}

//
// measure field
//

TYPEINIT1(XMLMeasureFieldImportContext, XMLTextFieldImportContext);

XMLMeasureFieldImportContext::XMLMeasureFieldImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_measure,
                                  nPrfx, sLocalName),
        mnKind( 0 )
{
}

void XMLMeasureFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_MEASURE_KIND:
            if( sAttrValue.equalsAsciiL(sXML_value, sizeof(sXML_value)-1))
            {
                mnKind = 0; bValid = sal_True;
            }
            else if( sAttrValue.equalsAsciiL(sXML_unit, sizeof(sXML_unit)-1))
            {
                mnKind = 1; bValid = sal_True;
            }
            else if( sAttrValue.equalsAsciiL(sXML_gap, sizeof(sXML_gap)-1))
            {
                mnKind = 2; bValid = sal_True;
            }
            break;
    }
}

void XMLMeasureFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;
    aAny <<= mnKind;
    xPropertySet->setPropertyValue(OUString::createFromAscii("Kind"), aAny);
}
