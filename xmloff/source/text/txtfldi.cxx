/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


/** @#file
 *
 *  Import of all text fields except those from txtvfldi.cxx
 *  (variable related text fields and database display fields)
 */
#include "txtfldi.hxx"
#include "txtvfldi.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumi.hxx>
#include <xmloff/txtimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlement.hxx>
#include "XMLStringBufferImportContext.hxx"
#include <xmloff/XMLEventsImportContext.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/text/UserDataPart.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/text/PlaceholderType.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/SetVariableType.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/text/ChapterFormat.hpp>
#include <com/sun/star/text/TemplateDisplayFormat.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/text/BibliographyDataType.hpp>
#include <com/sun/star/text/BibliographyDataField.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/sdb/CommandType.hpp>

#include <sax/tools/converter.hxx>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
#include <tools/debug.hxx>


using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;


//
// SO API string constants
//

// service prefix and service anems
const sal_Char sAPI_textfield_prefix[]  = "com.sun.star.text.TextField.";
const sal_Char sAPI_fieldmaster_prefix[] = "com.sun.star.text.FieldMaster.";
const sal_Char sAPI_presentation_prefix[] = "com.sun.star.presentation.TextField.";

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
const sal_Char sAPI_docinfo_custom[]            = "DocInfo.Custom";
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
const sal_Char sAPI_drop_down[]                 = "DropDown";
const sal_Char sAPI_header[]                    = "Header";
const sal_Char sAPI_footer[]                    = "Footer";
const sal_Char sAPI_datetime[]                  = "DateTime";

// property names
const sal_Char sAPI_is_fixed[]          = "IsFixed";
const sal_Char sAPI_content[]           = "Content";
const sal_Char sAPI_value[]             = "Value";
const sal_Char sAPI_author[]            = "Author";
const sal_Char sAPI_initials[]          = "Initials";
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
const sal_Char sAPI_data_base_u_r_l[]   = "DataBaseURL";
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
const sal_Char sAPI_is_hidden[]         = "IsHidden";
const sal_Char sAPI_is_condition_true[] = "IsConditionTrue";
const sal_Char sAPI_data_command_type[] = "DataCommandType";
const sal_Char sAPI_is_fixed_language[] = "IsFixedLanguage";
const sal_Char sAPI_is_visible[]        = "IsVisible";
const sal_Char sAPI_TextRange[]         = "TextRange";

const sal_Char sAPI_true[] = "TRUE";


TYPEINIT1( XMLTextFieldImportContext, SvXMLImportContext);

XMLTextFieldImportContext::XMLTextFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    const sal_Char* pService,
    sal_uInt16 nPrefix, const OUString& rElementName)
:   SvXMLImportContext( rImport, nPrefix, rElementName )
,   sIsFixed(sAPI_is_fixed)
,   rTextImportHelper(rHlp)
,   sServicePrefix(sAPI_textfield_prefix)
,   bValid(sal_False)
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
    if (sContent.isEmpty())
    {
        sContent = sContentBuffer.makeStringAndClear();
    }

    return sContent;
}

void XMLTextFieldImportContext::EndElement()
{
    DBG_ASSERT(!GetServiceName().isEmpty(), "no service name for element!");
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
            catch (const lang::IllegalArgumentException&)
            {
                // ignore
            }
            return;
        }
    }

    // in case of error: write element content
    rTextImportHelper.InsertString(GetContent());
}

void XMLTextFieldImportContext::Characters(const OUString& rContent)
{
    sContentBuffer.append(rContent);
}

sal_Bool XMLTextFieldImportContext::CreateField(
    Reference<XPropertySet> & xField,
    const OUString& rServiceName)
{
    // instantiate new XTextField:
    // ask import for model, model is factory, ask factory to create service

    Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),UNO_QUERY);
    if( xFactory.is() )
    {
        Reference<XInterface> xIfc = xFactory->createInstance(rServiceName);
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
        case XML_TOK_TEXT_PAGE_CONTINUATION:
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
        case XML_TOK_TEXT_NOTE_REF:
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
        case XML_TOK_TEXT_ANNOTATION_END:
            pContext = new XMLAnnotationImportContext( rImport, rHlp,
                                                       nToken,
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

        case XML_TOK_TEXT_TABLE_FORMULA:
            pContext = new XMLTableFormulaImportContext( rImport, rHlp,
                                                         nPrefix, rName );
            break;
        case XML_TOK_TEXT_DROPDOWN:
            pContext = new XMLDropDownFieldImportContext( rImport, rHlp,
                                                          nPrefix, rName );
            break;
        case XML_TOK_DRAW_HEADER:
            pContext = new XMLHeaderFieldImportContext( rImport, rHlp,
                                                          nPrefix, rName );
            break;
        case XML_TOK_DRAW_FOOTER:
            pContext = new XMLFooterFieldImportContext( rImport, rHlp,
                                                          nPrefix, rName );
            break;
        case XML_TOK_DRAW_DATE_TIME:
            pContext = new XMLDateTimeFieldImportContext( rImport, rHlp,
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


void XMLTextFieldImportContext::ForceUpdate(
    const Reference<XPropertySet> & rPropertySet)
{
    // force update
    Reference<XUpdatable> xUpdate(rPropertySet, UNO_QUERY);
    if (xUpdate.is())
    {
        xUpdate->update();
    }
    else
    {
        OSL_FAIL("Expected XUpdatable support!");
    }
}



//
// XMLSenderFieldImportContext
//

TYPEINIT1( XMLSenderFieldImportContext, XMLTextFieldImportContext);

XMLSenderFieldImportContext::XMLSenderFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName,
    sal_uInt16 nToken)
:   XMLTextFieldImportContext(rImport, rHlp, sAPI_extended_user,nPrfx, sLocalName)
,   sPropertyFixed(sAPI_is_fixed)
,   sPropertyFieldSubType(sAPI_user_data_type)
,   sPropertyContent(sAPI_content)
,   bFixed(sal_True)
,   nElementToken(nToken)
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
        bool bVal(false);
        bool const bRet = ::sax::Converter::convertBool(bVal, sAttrValue);

        // set bFixed if successfull
        if (bRet) {
            bFixed = bVal;
        }
    }
}

void XMLSenderFieldImportContext::PrepareField(
    const Reference<XPropertySet> & rPropSet)
{
    // set members
    Any aAny;
    aAny <<= nSubType;
    rPropSet->setPropertyValue(sPropertyFieldSubType, aAny);

    // set fixed
    aAny.setValue( &bFixed, ::getBooleanCppuType() );
    rPropSet->setPropertyValue(sPropertyFixed, aAny);

    // set content if fixed
    if (bFixed)
    {
        // in organizer or styles-only mode: force update
        if (GetImport().GetTextImport()->IsOrganizerMode() ||
            GetImport().GetTextImport()->IsStylesOnlyMode()   )
        {
            ForceUpdate(rPropSet);
        }
        else
        {
            aAny <<= GetContent();
            rPropSet->setPropertyValue(sPropertyContent, aAny);
        }
    }
}



//
// XMLAuthorFieldImportContext
//

TYPEINIT1( XMLAuthorFieldImportContext, XMLSenderFieldImportContext);

XMLAuthorFieldImportContext::XMLAuthorFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName,
    sal_uInt16 nToken)
:   XMLSenderFieldImportContext(rImport, rHlp, nPrfx, sLocalName, nToken)
,   bAuthorFullName(sal_True)
,   sServiceAuthor(sAPI_author)
,   sPropertyAuthorFullName(sAPI_full_name)
,   sPropertyFixed(sAPI_is_fixed)
,   sPropertyContent(sAPI_content)
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
    const Reference<XPropertySet> & rPropSet)
{
    // set members
    Any aAny;
    aAny.setValue( &bAuthorFullName, ::getBooleanCppuType() );
    rPropSet->setPropertyValue(sPropertyAuthorFullName, aAny);

    aAny.setValue( &bFixed, ::getBooleanCppuType() );
    rPropSet->setPropertyValue(sPropertyFixed, aAny);

    // set content if fixed
    if (bFixed)
    {
        // organizer or styles-only mode: force update
        if (GetImport().GetTextImport()->IsOrganizerMode() ||
            GetImport().GetTextImport()->IsStylesOnlyMode()   )
        {
            ForceUpdate(rPropSet);
        }
        else
        {
            aAny <<= GetContent();
            rPropSet->setPropertyValue(sPropertyContent, aAny);
        }
    }
}


//
// page continuation string
//

TYPEINIT1( XMLPageContinuationImportContext, XMLTextFieldImportContext );

static SvXMLEnumMapEntry const lcl_aSelectPageAttrMap[] =
{
    { XML_PREVIOUS,     PageNumberType_PREV },
    { XML_CURRENT,      PageNumberType_CURRENT },
    { XML_NEXT,         PageNumberType_NEXT },
    { XML_TOKEN_INVALID, 0 },
};

XMLPageContinuationImportContext::XMLPageContinuationImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName)
:   XMLTextFieldImportContext(rImport, rHlp, sAPI_page_number, nPrfx, sLocalName)
,   sPropertySubType(sAPI_sub_type)
,   sPropertyUserText(sAPI_user_text)
,   sPropertyNumberingType(sAPI_numbering_type)
,   eSelectPage(PageNumberType_CURRENT)
,   sStringOK(sal_False)
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
    const OUString& sLocalName)
:   XMLTextFieldImportContext(rImport, rHlp, sAPI_page_number, nPrfx, sLocalName)
,   sPropertySubType(sAPI_sub_type)
,   sPropertyNumberingType(sAPI_numbering_type)
,   sPropertyOffset(sAPI_offset)
,   sNumberSync(GetXMLToken(XML_FALSE))
,   nPageAdjust(0)
,   eSelectPage(PageNumberType_CURRENT)
,   sNumberFormatOK(sal_False)
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
            if (::sax::Converter::convertNumber(nTmp, sAttrValue))
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
    sal_uInt16 nPrfx, const OUString& sLocalName)
:   XMLTextFieldImportContext(rImport, rHlp, sAPI_jump_edit,nPrfx, sLocalName)
,   sPropertyPlaceholderType(sAPI_place_holder_type)
,   sPropertyPlaceholder(sAPI_place_holder)
,   sPropertyHint(sAPI_hint)
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
        if (IsXMLToken(sAttrValue, XML_TABLE))
        {
            nPlaceholderType = PlaceholderType::TABLE;
        }
        else if (IsXMLToken(sAttrValue, XML_TEXT))
        {
            nPlaceholderType = PlaceholderType::TEXT;
        }
        else if (IsXMLToken(sAttrValue, XML_TEXT_BOX))
        {
            nPlaceholderType = PlaceholderType::TEXTFRAME;
        }
        else if (IsXMLToken(sAttrValue, XML_IMAGE))
        {
            nPlaceholderType = PlaceholderType::GRAPHIC;
        }
        else if (IsXMLToken(sAttrValue, XML_OBJECT))
        {
            nPlaceholderType = PlaceholderType::OBJECT;
        }
        else
        {
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
    OUString aContent = GetContent();
    sal_Int32 nStart = 0;
    sal_Int32 nLength = aContent.getLength();
    if ((nLength > 0) && (aContent.getStr()[0] == '<'))
    {
        --nLength;
        ++nStart;
    }
    if ((nLength > 0) && (aContent.getStr()[aContent.getLength()-1] == '>'))
    {
        --nLength;
    }
    aAny <<= aContent.copy(nStart, nLength);
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
    sal_uInt16 nPrfx, const OUString& sLocalName)
:   XMLTextFieldImportContext(rImport, rHlp, sAPI_date_time, nPrfx, sLocalName)
,   sPropertyNumberFormat(sAPI_number_format)
,   sPropertyFixed(sAPI_is_fixed)
,   sPropertyDateTimeValue(sAPI_date_time_value)
,   sPropertyDateTime(sAPI_date_time)
,   sPropertyAdjust(sAPI_adjust)
,   sPropertyIsDate(sAPI_is_date)
,   sPropertyIsFixedLanguage(sAPI_is_fixed_language)
,   fTimeValue(0.0)
,   nAdjust(0)
,   nFormatKey(0)
,   bTimeOK(sal_False)
,   bFormatOK(sal_False)
,   bFixed(sal_False)
,   bIsDate(sal_False)
,   bIsDefaultLanguage( sal_True )
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

            if (::sax::Converter::convertDateTime(aDateTimeValue, sAttrValue))
            {
                bTimeOK = sal_True;
            }
            break;
        }
        case XML_TOK_TEXTFIELD_FIXED:
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, sAttrValue))
            {
                bFixed = bTmp;
            }
            break;
        }
        case XML_TOK_TEXTFIELD_DATA_STYLE_NAME:
        {
            sal_Int32 nKey = GetImportHelper().GetDataStyleKey(
                                               sAttrValue, &bIsDefaultLanguage);
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

            if (::sax::Converter::convertDuration(fTmp, sAttrValue))
            {
                // convert to minutes
                nAdjust = (sal_Int32)::rtl::math::approxFloor(fTmp * 60 * 24);
            }
            break;
        }
    }
}

void XMLTimeFieldImportContext::PrepareField(
    const Reference<XPropertySet> & rPropertySet)
{
    Any aAny;

    // all properties are optional (except IsDate)
    Reference<XPropertySetInfo> xPropertySetInfo(
        rPropertySet->getPropertySetInfo());

    if (xPropertySetInfo->hasPropertyByName(sPropertyFixed))
    {
        aAny.setValue( &bFixed, ::getBooleanCppuType() );
        rPropertySet->setPropertyValue(sPropertyFixed, aAny);
    }

    aAny.setValue( &bIsDate, ::getBooleanCppuType() );
    rPropertySet->setPropertyValue(sPropertyIsDate, aAny);

    if (xPropertySetInfo->hasPropertyByName(sPropertyAdjust))
    {
        aAny <<= nAdjust;
        rPropertySet->setPropertyValue(sPropertyAdjust, aAny);
    }

    // set value
    if (bFixed)
    {
        // organizer or styles-only mode: force update
        if (GetImport().GetTextImport()->IsOrganizerMode() ||
            GetImport().GetTextImport()->IsStylesOnlyMode()   )
        {
            ForceUpdate(rPropertySet);
        }
        else
        {
            // normal mode: set value (if present)
            if (bTimeOK)
            {
               if (xPropertySetInfo->hasPropertyByName(sPropertyDateTimeValue))
               {
                   aAny <<= aDateTimeValue;
                   rPropertySet->setPropertyValue(sPropertyDateTimeValue,aAny);
               }
               else if (xPropertySetInfo->hasPropertyByName(sPropertyDateTime))
               {
                   aAny <<= aDateTimeValue;
                   rPropertySet->setPropertyValue(sPropertyDateTime, aAny);
               }
            }
        }
    }

    if (bFormatOK &&
        xPropertySetInfo->hasPropertyByName(sPropertyNumberFormat))
    {
        aAny <<= nFormatKey;
        rPropertySet->setPropertyValue(sPropertyNumberFormat, aAny);

        if( xPropertySetInfo->hasPropertyByName( sPropertyIsFixedLanguage ) )
        {
            sal_Bool bIsFixedLanguage = ! bIsDefaultLanguage;
            aAny.setValue( &bIsFixedLanguage, ::getBooleanCppuType() );
            rPropertySet->setPropertyValue( sPropertyIsFixedLanguage, aAny );
        }
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
                // #96457#: don't truncate in order to read date+time
                fTimeValue = fTmp;
                bTimeOK = sal_True;
            }

            if (::sax::Converter::convertDateTime(aDateTimeValue, sAttrValue))
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
    const OUString& sLocalName, bool bUseDisply)
:   XMLTextFieldImportContext(rImport, rHlp, pServiceName, nPrfx, sLocalName)
,   sPropertyDataBaseName(sAPI_data_base_name)
,   sPropertyDataBaseURL(sAPI_data_base_u_r_l)
,   sPropertyTableName(sAPI_data_table_name)
,   sPropertyDataCommandType(sAPI_data_command_type)
,   sPropertyIsVisible(sAPI_is_visible)
,   nCommandType( sdb::CommandType::TABLE )
,   bCommandTypeOK(sal_False)
,   bDisplay( sal_True )
,   bDisplayOK( false )
,   bUseDisplay( bUseDisply )
,   bDatabaseOK(sal_False)
,   bDatabaseNameOK(sal_False)
,   bDatabaseURLOK(sal_False)
,   bTableOK(sal_False)
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
            bDatabaseNameOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_TABLE_NAME:
            sTableName = sAttrValue;
            bTableOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_TABLE_TYPE:
            if( IsXMLToken( sAttrValue, XML_TABLE ) )
            {
                nCommandType = sdb::CommandType::TABLE;
                bCommandTypeOK = sal_True;
            }
            else if( IsXMLToken( sAttrValue, XML_QUERY ) )
            {
                nCommandType = sdb::CommandType::QUERY;
                bCommandTypeOK = sal_True;
            }
            else if( IsXMLToken( sAttrValue, XML_COMMAND ) )
            {
                nCommandType = sdb::CommandType::COMMAND;
                bCommandTypeOK = sal_True;
            }
            break;
        case XML_TOK_TEXTFIELD_DISPLAY:
            if( IsXMLToken( sAttrValue, XML_NONE ) )
            {
                bDisplay = sal_False;
                bDisplayOK = true;
            }
            else if( IsXMLToken( sAttrValue, XML_VALUE ) )
            {
                bDisplay = sal_True;
                bDisplayOK = true;
            }
            break;
    }
}

SvXMLImportContext* XMLDatabaseFieldImportContext::CreateChildContext(
    sal_uInt16 p_nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& xAttrList )
{
    if( ( p_nPrefix == XML_NAMESPACE_FORM ) &&
        IsXMLToken( rLocalName, XML_CONNECTION_RESOURCE ) )
    {
        // process attribute list directly
        sal_Int16 nLength = xAttrList->getLength();
        for( sal_Int16 n = 0; n < nLength; n++ )
        {
            OUString sLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                GetKeyByAttrName( xAttrList->getNameByIndex(n), &sLocalName );

            if( ( nPrefix == XML_NAMESPACE_XLINK ) &&
                IsXMLToken( sLocalName, XML_HREF ) )
            {
                sDatabaseURL = xAttrList->getValueByIndex(n);
                bDatabaseOK = sal_True;
                bDatabaseURLOK = sal_True;
            }
        }

        // we call ProcessAttribute in order to set bValid appropriatly
        ProcessAttribute( XML_TOKEN_INVALID, OUString() );
    }

    return SvXMLImportContext::CreateChildContext(p_nPrefix, rLocalName,
                                                  xAttrList);
}


void XMLDatabaseFieldImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    aAny <<= sTableName;
    xPropertySet->setPropertyValue(sPropertyTableName, aAny);

    if( bDatabaseNameOK )
    {
        aAny <<= sDatabaseName;
        xPropertySet->setPropertyValue(sPropertyDataBaseName, aAny);
    }
    else if( bDatabaseURLOK )
    {
        aAny <<= sDatabaseURL;
        xPropertySet->setPropertyValue(sPropertyDataBaseURL, aAny);
    }

    // #99980# load/save command type for all fields; also load
    //         old documents without command type
    if( bCommandTypeOK )
    {
        aAny <<= nCommandType;
        xPropertySet->setPropertyValue( sPropertyDataCommandType, aAny );
    }

    if( bUseDisplay && bDisplayOK )
    {
        aAny.setValue( &bDisplay, ::getBooleanCppuType() );
        xPropertySet->setPropertyValue( sPropertyIsVisible, aAny );
    }
}



//
// database name field
//

TYPEINIT1( XMLDatabaseNameImportContext, XMLDatabaseFieldImportContext );

XMLDatabaseNameImportContext::XMLDatabaseNameImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLDatabaseFieldImportContext(rImport, rHlp, sAPI_database_name,
                                      nPrfx, sLocalName, true)
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
                                      nPrfx, sLocalName, false),
        sPropertyCondition(sAPI_condition),
        sTrue(sAPI_true),
        sCondition(),
        bConditionOK(sal_False)
{
}

XMLDatabaseNextImportContext::XMLDatabaseNextImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName)
: XMLDatabaseFieldImportContext(rImport, rHlp, sAPI_database_next, nPrfx, sLocalName, false)
,   sPropertyCondition(sAPI_condition)
,   sTrue(sAPI_true)
,   bConditionOK(sal_False)
{
}

void XMLDatabaseNextImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken, const OUString& sAttrValue )
{
    if (XML_TOK_TEXTFIELD_CONDITION == nAttrToken)
    {
        OUString sTmp;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap()._GetKeyByAttrName(
                                    sAttrValue, &sTmp, sal_False );
        if( XML_NAMESPACE_OOOW == nPrefix )
        {
            sCondition = sTmp;
            bConditionOK = sal_True;
        }
        else
            sCondition = sAttrValue;
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
        sPropertySetNumber(sAPI_set_number),
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
        if (::sax::Converter::convertNumber( nTmp, sAttrValue
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
                                      nPrfx, sLocalName, true),
        sPropertyNumberingType(
            sAPI_numbering_type),
        sPropertySetNumber(sAPI_set_number),
        sNumberFormat("1"),
        sNumberSync(GetXMLToken(XML_FALSE)),
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
            if (::sax::Converter::convertNumber( nTmp, sAttrValue ))
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
    sal_Bool bContent, sal_Bool bAuthor)
:   XMLTextFieldImportContext(rImport, rHlp, MapTokenToServiceName(nToken),nPrfx, sLocalName)
,   sPropertyFixed(sAPI_is_fixed)
,   sPropertyContent(sAPI_content)
,   sPropertyAuthor(sAPI_author)
,   sPropertyCurrentPresentation(sAPI_current_presentation)
,   bFixed(sal_False)
,   bHasAuthor(bAuthor)
,   bHasContent(bContent)
{
    bValid = sal_True;
}

void XMLSimpleDocInfoImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    if (XML_TOK_TEXTFIELD_FIXED == nAttrToken)
    {
        bool bTmp(false);
        if (::sax::Converter::convertBool(bTmp, sAttrValue))
        {
            bFixed = bTmp;
        }
    }
}

void XMLSimpleDocInfoImportContext::PrepareField(
    const Reference<XPropertySet> & rPropertySet)
{
    //  title field in Calc has no Fixed property
    Reference<XPropertySetInfo> xPropertySetInfo(rPropertySet->getPropertySetInfo());
    if (xPropertySetInfo->hasPropertyByName(sPropertyFixed))
    {
        Any aAny;
        aAny.setValue(&bFixed, ::getBooleanCppuType() );
        rPropertySet->setPropertyValue(sPropertyFixed, aAny);

        // set Content and CurrentPresentation (if fixed)
        if (bFixed)
        {
            // in organizer-mode or styles-only-mode, only force update
            if (GetImport().GetTextImport()->IsOrganizerMode() ||
                GetImport().GetTextImport()->IsStylesOnlyMode()   )
            {
                ForceUpdate(rPropertySet);
            }
            else
            {
                // set content (author, if that's the name) and current
                // presentation
                aAny <<= GetContent();

                if (bFixed && bHasAuthor)
                {
                    rPropertySet->setPropertyValue(sPropertyAuthor, aAny);
                }

                if (bFixed && bHasContent)
                {
                    rPropertySet->setPropertyValue(sPropertyContent, aAny);
                }

                rPropertySet->setPropertyValue(sPropertyCurrentPresentation, aAny);
            }
        }
    }
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
        case XML_TOK_TEXT_DOCUMENT_USER_DEFINED:
            pServiceName = sAPI_docinfo_custom;
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

        default:
            OSL_FAIL("no docinfo field token");
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
        sPropertyRevision(sAPI_revision)
{
    bValid = sal_True;
}

void XMLRevisionDocInfoImportContext::PrepareField(
    const Reference<XPropertySet> & rPropertySet)
{
    XMLSimpleDocInfoImportContext::PrepareField(rPropertySet);

    // set revision number
    // if fixed, if not in organizer-mode, if not in styles-only-mode
    if (bFixed)
    {
        if ( GetImport().GetTextImport()->IsOrganizerMode() ||
             GetImport().GetTextImport()->IsStylesOnlyMode()   )
        {
            ForceUpdate(rPropertySet);
        }
        else
        {
            sal_Int32 nTmp;
            if (::sax::Converter::convertNumber(nTmp, GetContent()))
            {
                Any aAny;
                aAny <<= nTmp;
                rPropertySet->setPropertyValue(sPropertyRevision, aAny);
            }
        }
    }
}



//
// DocInfo fields with date/time attributes
//

TYPEINIT1( XMLDateTimeDocInfoImportContext, XMLSimpleDocInfoImportContext );

XMLDateTimeDocInfoImportContext::XMLDateTimeDocInfoImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName, sal_uInt16 nToken)
:   XMLSimpleDocInfoImportContext(rImport, rHlp, nPrfx, sLocalName,nToken, sal_False, sal_False)
,   sPropertyNumberFormat(sAPI_number_format)
,   sPropertyIsDate(sAPI_is_date)
,   sPropertyIsFixedLanguage(sAPI_is_fixed_language)
,   nFormat(0)
,   bFormatOK(sal_False)
,   bIsDefaultLanguage(sal_True)
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
            OSL_FAIL(
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
            sal_Int32 nKey = GetImportHelper().GetDataStyleKey(
                                               sAttrValue, &bIsDefaultLanguage);
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

        if( xPropertySet->getPropertySetInfo()->
                hasPropertyByName( sPropertyIsFixedLanguage ) )
        {
            sal_Bool bIsFixedLanguage = ! bIsDefaultLanguage;
            aAny.setValue( &bIsFixedLanguage, ::getBooleanCppuType() );
            xPropertySet->setPropertyValue( sPropertyIsFixedLanguage, aAny );
        }
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
    , sPropertyName(sAPI_name)
    , sPropertyNumberFormat(sAPI_number_format)
    , sPropertyIsFixedLanguage(sAPI_is_fixed_language)
    , nFormat(0)
    , bFormatOK(sal_False)
    , bIsDefaultLanguage( sal_True )
{
    bValid = sal_False;
}

void XMLUserDocInfoImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_DATA_STYLE_NAME:
        {
            sal_Int32 nKey = GetImportHelper().GetDataStyleKey(
                                               sAttrValue, &bIsDefaultLanguage);
            if (-1 != nKey)
            {
                nFormat = nKey;
                bFormatOK = sal_True;
            }
            break;
        }
        case XML_TOK_TEXTFIELD_NAME:
        {
            if (!bValid)
            {
                SetServiceName(OUString(sAPI_docinfo_custom ) );
                aName = sAttrValue;
                bValid = sal_True;
            }
            break;
        }

        default:
            XMLSimpleDocInfoImportContext::ProcessAttribute(nAttrToken,
                                                            sAttrValue);
            break;
    }
}

void XMLUserDocInfoImportContext::PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet)
{
    uno::Any aAny;
    if ( !aName.isEmpty() )
    {
        aAny <<= aName;
        xPropertySet->setPropertyValue(sPropertyName, aAny);
    }
    Reference<XPropertySetInfo> xPropertySetInfo(
        xPropertySet->getPropertySetInfo());
    if (bFormatOK &&
        xPropertySetInfo->hasPropertyByName(sPropertyNumberFormat))
    {
        aAny <<= nFormat;
        xPropertySet->setPropertyValue(sPropertyNumberFormat, aAny);

        if( xPropertySetInfo->hasPropertyByName( sPropertyIsFixedLanguage ) )
        {
            sal_Bool bIsFixedLanguage = ! bIsDefaultLanguage;
            aAny.setValue( &bIsFixedLanguage, ::getBooleanCppuType() );
            xPropertySet->setPropertyValue( sPropertyIsFixedLanguage, aAny );
        }
    }

    // call superclass to handle "fixed"
    XMLSimpleDocInfoImportContext::PrepareField(xPropertySet);
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
        sPropertyCondition(sAPI_condition),
        sPropertyIsHidden(sAPI_is_hidden),
        sCondition(),
        bIsHidden(sal_False)
{
}

void XMLHiddenParagraphImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    if (XML_TOK_TEXTFIELD_CONDITION == nAttrToken)
    {
        OUString sTmp;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap()._GetKeyByAttrName(
                                    sAttrValue, &sTmp, sal_False );
        if( XML_NAMESPACE_OOOW == nPrefix )
        {
            sCondition = sTmp;
            bValid = sal_True;
        }
        else
            sCondition = sAttrValue;
    }
    else if (XML_TOK_TEXTFIELD_IS_HIDDEN == nAttrToken)
    {
        bool bTmp(false);
        if (::sax::Converter::convertBool(bTmp, sAttrValue))
        {
            bIsHidden = bTmp;
        }
    }
}

void XMLHiddenParagraphImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;
    aAny <<= sCondition;
    xPropertySet->setPropertyValue(sPropertyCondition, aAny);

    aAny.setValue( &bIsHidden, ::getBooleanCppuType() );
    xPropertySet->setPropertyValue(sPropertyIsHidden, aAny);
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
        sPropertyCondition(sAPI_condition),
        sPropertyTrueContent(sAPI_true_content),
        sPropertyFalseContent(sAPI_false_content),
        sPropertyIsConditionTrue(sAPI_is_condition_true),
        sPropertyCurrentPresentation(sAPI_current_presentation),
        bConditionOK(sal_False),
        bTrueOK(sal_False),
        bFalseOK(sal_False),
        bCurrentValue(sal_False)
{
}

void XMLConditionalTextImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_CONDITION:
            {
                OUString sTmp;
                sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                        _GetKeyByAttrName( sAttrValue, &sTmp, sal_False );
                if( XML_NAMESPACE_OOOW == nPrefix )
                {
                    sCondition = sTmp;
                    bConditionOK = sal_True;
                }
                else
                    sCondition = sAttrValue;
            }
            break;
        case XML_TOK_TEXTFIELD_STRING_VALUE_IF_FALSE:
            sFalseContent = sAttrValue;
            bFalseOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_STRING_VALUE_IF_TRUE:
            sTrueContent = sAttrValue;
            bTrueOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_CURRENT_VALUE:
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, sAttrValue))
            {
                bCurrentValue = bTmp;
            }
            break;
        }
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

    aAny.setValue( &bCurrentValue, ::getBooleanCppuType() );
    xPropertySet->setPropertyValue(sPropertyIsConditionTrue, aAny);

    aAny <<= GetContent();
    xPropertySet->setPropertyValue(sPropertyCurrentPresentation, aAny);
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
        sPropertyCondition(sAPI_condition),
        sPropertyContent(sAPI_content),
        sPropertyIsHidden(sAPI_is_hidden),
        bConditionOK(sal_False),
        bStringOK(sal_False),
        bIsHidden(sal_False)
{
}

void XMLHiddenTextImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_CONDITION:
            {
                OUString sTmp;
                sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                                        _GetKeyByAttrName( sAttrValue, &sTmp, sal_False );
                if( XML_NAMESPACE_OOOW == nPrefix )
                {
                    sCondition = sTmp;
                    bConditionOK = sal_True;
                }
                else
                    sCondition = sAttrValue;
            }
            break;
        case XML_TOK_TEXTFIELD_STRING_VALUE:
            sString = sAttrValue;
            bStringOK = sal_True;
            break;
        case XML_TOK_TEXTFIELD_IS_HIDDEN:
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, sAttrValue))
            {
                bIsHidden = bTmp;
            }
            break;
        }
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

    aAny.setValue( &bIsHidden, ::getBooleanCppuType() );
    xPropertySet->setPropertyValue(sPropertyIsHidden, aAny);
}



//
// file name fields
//

TYPEINIT1( XMLFileNameImportContext, XMLTextFieldImportContext );

static const SvXMLEnumMapEntry aFilenameDisplayMap[] =
{
    { XML_PATH,                 FilenameDisplayFormat::PATH },
    { XML_NAME,                 FilenameDisplayFormat::NAME },
    { XML_NAME_AND_EXTENSION,   FilenameDisplayFormat::NAME_AND_EXT },
    { XML_FULL,                 FilenameDisplayFormat::FULL },
    { XML_TOKEN_INVALID, 0 }
};

XMLFileNameImportContext::XMLFileNameImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_file_name,
                                  nPrfx, sLocalName),
        sPropertyFixed(sAPI_is_fixed),
        sPropertyFileFormat(sAPI_file_format),
        sPropertyCurrentPresentation(
            sAPI_current_presentation),
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
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, sAttrValue))
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

     if (xPropertySetInfo->hasPropertyByName(sPropertyFixed))
     {
         aAny <<= bFixed;
         xPropertySet->setPropertyValue(sPropertyFixed, aAny);
     }

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
    { XML_FULL,                 TemplateDisplayFormat::FULL },
    { XML_PATH,                 TemplateDisplayFormat::PATH },
    { XML_NAME,                 TemplateDisplayFormat::NAME },
    { XML_NAME_AND_EXTENSION,   TemplateDisplayFormat::NAME_AND_EXT },
    { XML_AREA,                 TemplateDisplayFormat::AREA },
    { XML_TITLE,                TemplateDisplayFormat::TITLE },
    { XML_TOKEN_INVALID, 0 }
};

TYPEINIT1( XMLTemplateNameImportContext, XMLTextFieldImportContext );

XMLTemplateNameImportContext::XMLTemplateNameImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_template_name,
                                  nPrfx, sLocalName),
        sPropertyFileFormat(sAPI_file_format),
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
    { XML_NAME,                     ChapterFormat::NAME },
    { XML_NUMBER,                   ChapterFormat::NUMBER },
    { XML_NUMBER_AND_NAME,          ChapterFormat::NAME_NUMBER },
    { XML_PLAIN_NUMBER_AND_NAME,    ChapterFormat::NO_PREFIX_SUFFIX },
    { XML_PLAIN_NUMBER,             ChapterFormat::DIGIT },
    { XML_TOKEN_INVALID, 0 }
};

XMLChapterImportContext::XMLChapterImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_chapter,
                                  nPrfx, sLocalName),
        sPropertyChapterFormat(
            sAPI_chapter_format),
        sPropertyLevel(sAPI_level),
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
            if (::sax::Converter::convertNumber(
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
            sAPI_numbering_type),
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
            OSL_FAIL("unknown count field!");
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
            sAPI_numbering_type),
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

    // display old content (#96657#)
    aAny <<= GetContent();
    xPropertySet->setPropertyValue(
        OUString(sAPI_current_presentation),
        aAny );
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
        sPropertyOn(sAPI_on),
        sPropertyOffset(sAPI_offset),
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
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, sAttrValue))
            {
                bActive = bTmp;
            }
        }
        case XML_TOK_TEXTFIELD_PAGE_ADJUST:
        {
            sal_Int32 nTmp;
            if (::sax::Converter::convertNumber(nTmp, sAttrValue))
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
        sPropertyHint(sAPI_hint),
        sPropertyMacroName("MacroName"),
        sPropertyScriptURL("ScriptURL"),
        bDescriptionOK(sal_False)
{
}

SvXMLImportContext* XMLMacroFieldImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if ( (nPrefix == XML_NAMESPACE_OFFICE) &&
         IsXMLToken( rLocalName, XML_EVENT_LISTENERS ) )
    {
        // create events context and remember it!
        pContext = new XMLEventsImportContext(
            GetImport(), nPrefix, rLocalName );
        xEventContext = pContext;
        bValid = sal_True;
    }
    else
        pContext = SvXMLImportContext::CreateChildContext(
            nPrefix, rLocalName, xAttrList);

    return pContext;
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

    OUString sOnClick("OnClick");
    OUString sPropertyMacroLibrary("MacroLibrary");

    aAny <<= (bDescriptionOK ? sDescription : GetContent());
    xPropertySet->setPropertyValue(sPropertyHint, aAny);

    // if we have an events child element, we'll look for the OnClick
    // event if not, it may be an old (pre-638i) document. Then, we'll
    // have to look at the name attribute.
    OUString sMacroName;
    OUString sLibraryName;
    OUString sScriptURL;

    if ( xEventContext.Is() )
    {
        // get event sequence
        XMLEventsImportContext* pEvents =
            (XMLEventsImportContext*)&xEventContext;
        Sequence<PropertyValue> aValues;
        pEvents->GetEventSequence( sOnClick, aValues );

        sal_Int32 nLength = aValues.getLength();
        for( sal_Int32 i = 0; i < nLength; i++ )
        {
            if ( aValues[i].Name == "ScriptType" )
            {
                // ignore ScriptType
            }
            else if ( aValues[i].Name == "Library" )
            {
                aValues[i].Value >>= sLibraryName;
            }
            else if ( aValues[i].Name == "MacroName" )
            {
                aValues[i].Value >>= sMacroName;
            }
            if ( aValues[i].Name == "Script" )
            {
                aValues[i].Value >>= sScriptURL;
            }
        }
    }
    else
    {
        // disassemble old-style macro-name: Everything before the
        // third-last dot is the library
        sal_Int32 nPos = sMacro.getLength() + 1;    // the loop starts with nPos--
        const sal_Unicode* pBuf = sMacro.getStr();
        for( sal_Int32 i = 0; (i < 3) && (nPos > 0); i++ )
        {
            nPos--;
            while ( (pBuf[nPos] != '.') && (nPos > 0) )
                nPos--;
        }

        if (nPos > 0)
        {
            sLibraryName = sMacro.copy(0, nPos);
            sMacroName = sMacro.copy(nPos+1);
        }
        else
            sMacroName = sMacro;
    }

    aAny <<= sScriptURL;
    xPropertySet->setPropertyValue(sPropertyScriptURL, aAny);

    aAny <<= sMacroName;
    xPropertySet->setPropertyValue(sPropertyMacroName, aAny);

    aAny <<= sLibraryName;
    xPropertySet->setPropertyValue(sPropertyMacroLibrary, aAny);
}



//
// reference field import
//

TYPEINIT1( XMLReferenceFieldImportContext, XMLTextFieldImportContext );

XMLReferenceFieldImportContext::XMLReferenceFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nToken, sal_uInt16 nPrfx, const OUString& sLocalName)
:   XMLTextFieldImportContext(rImport, rHlp, sAPI_get_reference, nPrfx, sLocalName)
,   sPropertyReferenceFieldPart(sAPI_reference_field_part)
,   sPropertyReferenceFieldSource(sAPI_reference_field_source)
,   sPropertySourceName(sAPI_source_name)
,   sPropertyCurrentPresentation(sAPI_current_presentation)
,   nElementToken(nToken)
,   nType(ReferenceFieldPart::PAGE_DESC)
,   bNameOK(sal_False)
,   bTypeOK(sal_False)
{
}

static SvXMLEnumMapEntry const lcl_aReferenceTypeTokenMap[] =
{
    { XML_PAGE,         ReferenceFieldPart::PAGE},
    { XML_CHAPTER,      ReferenceFieldPart::CHAPTER },
    { XML_TEXT,         ReferenceFieldPart::TEXT },
    { XML_DIRECTION,    ReferenceFieldPart::UP_DOWN },
    { XML_CATEGORY_AND_VALUE, ReferenceFieldPart::CATEGORY_AND_NUMBER },
    { XML_CAPTION,      ReferenceFieldPart::ONLY_CAPTION },
    { XML_VALUE,        ReferenceFieldPart::ONLY_SEQUENCE_NUMBER },
    // Core implementation for direct cross-references (#i81002#)
    { XML_NUMBER,               ReferenceFieldPart::NUMBER },
    { XML_NUMBER_NO_SUPERIOR,   ReferenceFieldPart::NUMBER_NO_CONTEXT },
    { XML_NUMBER_ALL_SUPERIOR,  ReferenceFieldPart::NUMBER_FULL_CONTEXT },
    { XML_TOKEN_INVALID, 0 }
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
        case XML_TOK_TEXT_NOTE_REF:
            nSource = ReferenceFieldSource::FOOTNOTE;
            break;
        case XML_TOK_TEXT_SEQUENCE_REF:
            nSource = ReferenceFieldSource::SEQUENCE_FIELD;
            break;
        default:
            bTypeOK = sal_False;
            OSL_FAIL("unknown reference field");
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
        case XML_TOK_TEXTFIELD_NOTE_CLASS:
            if( IsXMLToken( sAttrValue, XML_ENDNOTE ) )
                nSource = ReferenceFieldSource::ENDNOTE;
            break;
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

        case XML_TOK_TEXT_NOTE_REF:
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

static SvXMLTokenMapEntry aDdeDeclAttrTokenMap[] =
{
    { XML_NAMESPACE_OFFICE, XML_NAME, XML_TOK_DDEFIELD_NAME },
    { XML_NAMESPACE_OFFICE, XML_DDE_APPLICATION, XML_TOK_DDEFIELD_APPLICATION },
    { XML_NAMESPACE_OFFICE, XML_DDE_TOPIC, XML_TOK_DDEFIELD_TOPIC },
    { XML_NAMESPACE_OFFICE, XML_DDE_ITEM, XML_TOK_DDEFIELD_ITEM },
    { XML_NAMESPACE_OFFICE, XML_AUTOMATIC_UPDATE, XML_TOK_DDEFIELD_UPDATE },
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
         (IsXMLToken(rLocalName, XML_DDE_CONNECTION_DECL)) )
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
    const OUString& sLocalName, const SvXMLTokenMap& rMap)
:   SvXMLImportContext(rImport, nPrfx, sLocalName)
,   sPropertyIsAutomaticUpdate(sAPI_is_automatic_update)
,   sPropertyName(sAPI_name)
,   sPropertyDDECommandType(sAPI_dde_command_type)
,   sPropertyDDECommandFile(sAPI_dde_command_file)
,   sPropertyDDECommandElement(sAPI_dde_command_element)
,   rTokenMap(rMap)
{
    DBG_ASSERT(XML_NAMESPACE_TEXT == nPrfx, "wrong prefix");
    DBG_ASSERT(IsXMLToken(sLocalName, XML_DDE_CONNECTION_DECL), "wrong name");
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
                bool bTmp(false);
                if (::sax::Converter::convertBool(
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
            /* #i6432# There might be multiple occurrences of one DDE
               declaration if it is used in more than one of
               header/footer/body. createInstance will throw an exception if we
               try to create the second, third, etc. instance of such a
               declaration. Thus we ignore the exception. Otherwise this will
               lead to an unloadable document. */
            try
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
            catch (const Exception&)
            {
                //ignore
            }
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
        ,sPropertyContent(sAPI_content)
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
            //apply the content to the master
            xMaster->setPropertyValue( sPropertyContent, uno::makeAny( GetContent()));
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
    const Reference<XPropertySet> &)
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
    sal_uInt16,
    const ::rtl::OUString& )
{
    // no attributes -> nothing to be done
}

void XMLSheetNameImportContext::PrepareField(
    const Reference<XPropertySet> &)
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
        sPropertyURL(sAPI_url),
        sPropertyTargetFrame(sAPI_target_frame),
        sPropertyRepresentation(
            sAPI_representation),
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
            sURL = GetImport().GetAbsoluteReference( sAttrValue );
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
        sPropertyFields("Fields"),
        aValues()
{
    bValid = sal_True;
}

// TODO: this is the same map as is used in the text field export
SvXMLEnumMapEntry const aBibliographyDataTypeMap[] =
{
    { XML_ARTICLE,          BibliographyDataType::ARTICLE },
    { XML_BOOK,             BibliographyDataType::BOOK },
    { XML_BOOKLET,          BibliographyDataType::BOOKLET },
    { XML_CONFERENCE,       BibliographyDataType::CONFERENCE },
    { XML_CUSTOM1,          BibliographyDataType::CUSTOM1 },
    { XML_CUSTOM2,          BibliographyDataType::CUSTOM2 },
    { XML_CUSTOM3,          BibliographyDataType::CUSTOM3 },
    { XML_CUSTOM4,          BibliographyDataType::CUSTOM4 },
    { XML_CUSTOM5,          BibliographyDataType::CUSTOM5 },
    { XML_EMAIL,            BibliographyDataType::EMAIL },
    { XML_INBOOK,           BibliographyDataType::INBOOK },
    { XML_INCOLLECTION,     BibliographyDataType::INCOLLECTION },
    { XML_INPROCEEDINGS,    BibliographyDataType::INPROCEEDINGS },
    { XML_JOURNAL,          BibliographyDataType::JOURNAL },
    { XML_MANUAL,           BibliographyDataType::MANUAL },
    { XML_MASTERSTHESIS,    BibliographyDataType::MASTERSTHESIS },
    { XML_MISC,             BibliographyDataType::MISC },
    { XML_PHDTHESIS,        BibliographyDataType::PHDTHESIS },
    { XML_PROCEEDINGS,      BibliographyDataType::PROCEEDINGS },
    { XML_TECHREPORT,       BibliographyDataType::TECHREPORT },
    { XML_UNPUBLISHED,      BibliographyDataType::UNPUBLISHED },
    { XML_WWW,              BibliographyDataType::WWW },
    { XML_TOKEN_INVALID, 0 }
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

            // special treatment for bibliography type
            // biblio vs bibilio: #96658#; also read old documents
            if (IsXMLToken(sLocalName, XML_BIBILIOGRAPHIC_TYPE) ||
                IsXMLToken(sLocalName, XML_BIBLIOGRAPHY_TYPE)    )
            {
                sal_uInt16 nTmp;
                if (SvXMLUnitConverter::convertEnum(
                    nTmp, xAttrList->getValueByIndex(i),
                    aBibliographyDataTypeMap))
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
    sal_uInt16,
    const OUString& )
{
    // attributes are handled in StartElement
    OSL_FAIL("This should not have happened.");
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
    const sal_Char* pName = NULL;

    if (IsXMLToken(sName, XML_IDENTIFIER))
    {
        pName = "Identifier";
    }
    else if (IsXMLToken(sName, XML_BIBILIOGRAPHIC_TYPE) ||
             IsXMLToken(sName, XML_BIBLIOGRAPHY_TYPE)     )
    {
        // biblio... vs bibilio...: #96658#: also read old documents
        pName = "BibiliographicType";
    }
    else if (IsXMLToken(sName, XML_ADDRESS))
    {
        pName = "Address";
    }
    else if (IsXMLToken(sName, XML_ANNOTE))
    {
        pName = "Annote";
    }
    else if (IsXMLToken(sName, XML_AUTHOR))
    {
        pName = "Author";
    }
    else if (IsXMLToken(sName, XML_BOOKTITLE))
    {
        pName = "Booktitle";
    }
    else if (IsXMLToken(sName, XML_CHAPTER))
    {
        pName = "Chapter";
    }
    else if (IsXMLToken(sName, XML_EDITION))
    {
        pName = "Edition";
    }
    else if (IsXMLToken(sName, XML_EDITOR))
    {
        pName = "Editor";
    }
    else if (IsXMLToken(sName, XML_HOWPUBLISHED))
    {
        pName = "Howpublished";
    }
    else if (IsXMLToken(sName, XML_INSTITUTION))
    {
        pName = "Institution";
    }
    else if (IsXMLToken(sName, XML_JOURNAL))
    {
        pName = "Journal";
    }
    else if (IsXMLToken(sName, XML_MONTH))
    {
        pName = "Month";
    }
    else if (IsXMLToken(sName, XML_NOTE))
    {
        pName = "Note";
    }
    else if (IsXMLToken(sName, XML_NUMBER))
    {
        pName = "Number";
    }
    else if (IsXMLToken(sName, XML_ORGANIZATIONS))
    {
        pName = "Organizations";
    }
    else if (IsXMLToken(sName, XML_PAGES))
    {
        pName = "Pages";
    }
    else if (IsXMLToken(sName, XML_PUBLISHER))
    {
        pName = "Publisher";
    }
    else if (IsXMLToken(sName, XML_SCHOOL))
    {
        pName = "School";
    }
    else if (IsXMLToken(sName, XML_SERIES))
    {
        pName = "Series";
    }
    else if (IsXMLToken(sName, XML_TITLE))
    {
        pName = "Title";
    }
    else if (IsXMLToken(sName, XML_REPORT_TYPE))
    {
        pName = "Report_Type";
    }
    else if (IsXMLToken(sName, XML_VOLUME))
    {
        pName = "Volume";
    }
    else if (IsXMLToken(sName, XML_YEAR))
    {
        pName = "Year";
    }
    else if (IsXMLToken(sName, XML_URL))
    {
        pName = "URL";
    }
    else if (IsXMLToken(sName, XML_CUSTOM1))
    {
        pName = "Custom1";
    }
    else if (IsXMLToken(sName, XML_CUSTOM2))
    {
        pName = "Custom2";
    }
    else if (IsXMLToken(sName, XML_CUSTOM3))
    {
        pName = "Custom3";
    }
    else if (IsXMLToken(sName, XML_CUSTOM4))
    {
        pName = "Custom4";
    }
    else if (IsXMLToken(sName, XML_CUSTOM5))
    {
        pName = "Custom5";
    }
    else if (IsXMLToken(sName, XML_ISBN))
    {
        pName = "ISBN";
    }
    else
    {
        OSL_FAIL("Unknown bibliography info data");
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
    sal_uInt16 nToken,
    sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_annotation,
                                  nPrfx, sLocalName),
        sPropertyAuthor(sAPI_author),
        sPropertyInitials(sAPI_initials),
        sPropertyContent(sAPI_content),
        // why is there no UNO_NAME_DATE_TIME, but only UNO_NAME_DATE_TIME_VALUE?
        sPropertyDate(sAPI_date_time_value),
        sPropertyTextRange(sAPI_TextRange),
        sPropertyName(sAPI_name),
        m_nToken(nToken)
{
    bValid = sal_True;

    // remember old list item and block (#91964#) and reset them
    // for the text frame
    // do this in the constructor, not in CreateChildContext (#i93392#)
    GetImport().GetTextImport()->PushListContext();
}

void XMLAnnotationImportContext::ProcessAttribute(
    sal_uInt16 nToken,
    const OUString& rValue )
{
    if (nToken == XML_TOK_TEXT_NAME)
        aName = rValue;
}

SvXMLImportContext* XMLAnnotationImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = 0;
    if( XML_NAMESPACE_DC == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_CREATOR ) )
            pContext = new XMLStringBufferImportContext(GetImport(), nPrefix,
                                            rLocalName, aAuthorBuffer);
        else if( IsXMLToken( rLocalName, XML_DATE ) )
            pContext = new XMLStringBufferImportContext(GetImport(), nPrefix,
                                            rLocalName, aDateBuffer);
    }
    else if( XML_NAMESPACE_TEXT == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_SENDER_INITIALS ) )
            pContext = new XMLStringBufferImportContext(GetImport(), nPrefix,
                                            rLocalName, aInitialsBuffer);
    }

    if( !pContext )
    {
        try
        {
            if ( !mxField.is() )
                CreateField( mxField, sServicePrefix + GetServiceName() );
            Any aAny = mxField->getPropertyValue( sPropertyTextRange );
            Reference< XText > xText;
            aAny >>= xText;
            if( xText.is() )
            {
                UniReference < XMLTextImportHelper > xTxtImport = GetImport().GetTextImport();
                if( !mxCursor.is() )
                {
                    mxOldCursor = xTxtImport->GetCursor();
                    mxCursor = xText->createTextCursor();
                }

                if( mxCursor.is() )
                {
                    xTxtImport->SetCursor( mxCursor );
                    pContext = xTxtImport->CreateTextChildContext( GetImport(), nPrefix, rLocalName, xAttrList );
                }
            }
        }
        catch (const Exception&)
        {
        }

        if( !pContext )
            pContext = new XMLStringBufferImportContext(GetImport(), nPrefix,  rLocalName, aTextBuffer);
    }

    return pContext;
}

void XMLAnnotationImportContext::EndElement()
{
    DBG_ASSERT(!GetServiceName().isEmpty(), "no service name for element!");
    if( mxCursor.is() )
    {
        // delete addition newline
        const OUString aEmpty;
        mxCursor->gotoEnd( sal_False );
        mxCursor->goLeft( 1, sal_True );
        mxCursor->setString( aEmpty );

        // reset cursor
        GetImport().GetTextImport()->ResetCursor();
    }

    if( mxOldCursor.is() )
        GetImport().GetTextImport()->SetCursor( mxOldCursor );

    // reinstall old list item #91964#
    GetImport().GetTextImport()->PopListContext();

    if ( bValid )
    {
        if ( mxField.is() || CreateField( mxField, sServicePrefix + GetServiceName() ) )
        {
            // set field properties
            PrepareField( mxField );

            // attach field to document
            Reference < XTextContent > xTextContent( mxField, UNO_QUERY );

            // workaround for #80606#
            try
            {
                if (m_nToken == XML_TOK_TEXT_ANNOTATION_END && m_xStart.is())
                {
                    // So we are ending a previous annotation, let's create a
                    // text range covering the old and the current position.
                    uno::Reference<text::XText> xText = GetImportHelper().GetText();
                    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursorByRange(m_xStart->getAnchor());
                    xCursor->gotoRange(GetImportHelper().GetCursorAsRange(), true);
                    uno::Reference<text::XTextRange> xTextRange(xCursor, uno::UNO_QUERY);
                    xText->insertTextContent(xTextRange, xTextContent, !xCursor->isCollapsed());

                    // Now we can delete the old annotation with the incorrect position.
                    uno::Reference<lang::XComponent>(m_xStart, uno::UNO_QUERY)->dispose();
                }
                else
                    GetImportHelper().InsertTextContent( xTextContent );
            }
            catch (const lang::IllegalArgumentException&)
            {
                // ignore
            }
        }
    }
    else
        GetImportHelper().InsertString(GetContent());
}

void XMLAnnotationImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    if (m_nToken == XML_TOK_TEXT_ANNOTATION_END && !aName.isEmpty())
    {
        // Search for a previous annotation with the same name.
        Reference<XTextFieldsSupplier> xTextFieldsSupplier(GetImport().GetModel(), UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
        uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
        uno::Reference<beans::XPropertySet> xPrevField;
        while (xFields->hasMoreElements())
        {
            uno::Reference<beans::XPropertySet> xCurrField(xFields->nextElement(), uno::UNO_QUERY);
            OUString aFieldName;
            xCurrField->getPropertyValue(sPropertyName) >>= aFieldName;
            if (aFieldName == aName)
            {
                xPrevField = xCurrField;
                break;
            }
        }
        if (xPrevField.is())
        {
            // Found? Then copy over the properties.
            xPropertySet->setPropertyValue(sPropertyAuthor, xPrevField->getPropertyValue(sPropertyAuthor));
            xPropertySet->setPropertyValue(sPropertyInitials, xPrevField->getPropertyValue(sPropertyInitials));
            xPropertySet->setPropertyValue(sPropertyDate, xPrevField->getPropertyValue(sPropertyDate));
            xPropertySet->setPropertyValue(sPropertyName, xPrevField->getPropertyValue(sPropertyName));
            xPropertySet->setPropertyValue(sPropertyContent, xPrevField->getPropertyValue(sPropertyContent));

            // And save a reference to it, so we can delete it later.
            m_xStart.set(xPrevField, uno::UNO_QUERY);
            return;
        }
    }

    // import (possibly empty) author
    OUString sAuthor( aAuthorBuffer.makeStringAndClear() );
    xPropertySet->setPropertyValue(sPropertyAuthor, makeAny(sAuthor));

    // import (possibly empty) initials
    OUString sInitials( aInitialsBuffer.makeStringAndClear() );
    xPropertySet->setPropertyValue(sPropertyInitials, makeAny(sInitials));

    DateTime aDateTime;
    if (::sax::Converter::convertDateTime(aDateTime,
                                            aDateBuffer.makeStringAndClear()))
    {
        /*
        Date aDate;
        aDate.Year = aDateTime.Year;
        aDate.Month = aDateTime.Month;
        aDate.Day = aDateTime.Day;
        xPropertySet->setPropertyValue(sPropertyDate, makeAny(aDate));
        */
        xPropertySet->setPropertyValue(sPropertyDate, makeAny(aDateTime));
    }

    OUString sBuffer = aTextBuffer.makeStringAndClear();
    if ( sBuffer.getLength() )
    {
        // delete last paragraph mark (if necessary)
        if (sal_Char(0x0a) == sBuffer.getStr()[sBuffer.getLength()-1])
            sBuffer = sBuffer.copy(0, sBuffer.getLength()-1);
        xPropertySet->setPropertyValue(sPropertyContent, makeAny(sBuffer));
    }

    if (!aName.isEmpty())
        xPropertySet->setPropertyValue(sPropertyName, makeAny(aName));
}



//
// script field
//

TYPEINIT1(XMLScriptImportContext, XMLTextFieldImportContext);

XMLScriptImportContext::XMLScriptImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx,
    const OUString& sLocalName)
:   XMLTextFieldImportContext(rImport, rHlp, sAPI_script, nPrfx, sLocalName)
,   sPropertyScriptType(sAPI_script_type)
,   sPropertyURLContent(sAPI_url_content)
,   sPropertyContent(sAPI_content)
,   bContentOK(sal_False)
,   bScriptTypeOK(sal_False)
{
}

void XMLScriptImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_HREF:
            sContent = GetImport().GetAbsoluteReference( sAttrValue );
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

    // always valid (even without ScriptType; cf- #96531#)
    bValid = sal_True;
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
            if( IsXMLToken( sAttrValue, XML_VALUE ) )
            {
                mnKind = 0; bValid = sal_True;
            }
            else if( IsXMLToken( sAttrValue, XML_UNIT ) )
            {
                mnKind = 1; bValid = sal_True;
            }
            else if( IsXMLToken( sAttrValue, XML_GAP ) )
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
    xPropertySet->setPropertyValue(OUString("Kind"), aAny);
}



//
// dropdown field
//


TYPEINIT1( XMLDropDownFieldImportContext, XMLTextFieldImportContext );

XMLDropDownFieldImportContext::XMLDropDownFieldImportContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rHlp,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& sLocalName) :
    XMLTextFieldImportContext( rImport, rHlp, sAPI_drop_down,
                               nPrfx, sLocalName ),
    aLabels(),
    sName(),
    nSelected( -1 ),
    bNameOK( false ),
    bHelpOK(false),
    bHintOK(false),
    sPropertyItems( "Items"  ),
    sPropertySelectedItem( "SelectedItem"  ),
    sPropertyName( "Name"  ),
    sPropertyHelp( "Help"  ),
    sPropertyToolTip( "Tooltip"  )
{
    bValid = sal_True;
}

static bool lcl_ProcessLabel( const SvXMLImport& rImport,
                       const Reference<XAttributeList>& xAttrList,
                       OUString& rLabel,
                       bool& rIsSelected )
{
    bool bValid = false;
    sal_Int16 nLength = xAttrList->getLength();
    for( sal_Int16 n = 0; n < nLength; n++ )
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(n), &sLocalName );
        OUString sValue = xAttrList->getValueByIndex(n);

        if( nPrefix == XML_NAMESPACE_TEXT )
        {
            if( IsXMLToken( sLocalName, XML_VALUE ) )
            {
                rLabel = sValue;
                bValid = true;
            }
            else if( IsXMLToken( sLocalName, XML_CURRENT_SELECTED ) )
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool( bTmp, sValue ))
                    rIsSelected = bTmp;
            }
        }
    }
    return bValid;
}

SvXMLImportContext* XMLDropDownFieldImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& xAttrList )
{
    if( nPrefix == XML_NAMESPACE_TEXT  &&
        IsXMLToken( rLocalName, XML_LABEL ) )
    {
        OUString sLabel;
        bool bIsSelected = sal_False;
        if( lcl_ProcessLabel( GetImport(), xAttrList, sLabel, bIsSelected ) )
        {
            if( bIsSelected )
                nSelected = static_cast<sal_Int32>( aLabels.size() );
            aLabels.push_back( sLabel );
        }
    }
    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

void XMLDropDownFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const ::rtl::OUString& sAttrValue )
{
    if( nAttrToken == XML_TOK_TEXTFIELD_NAME )
    {
        sName = sAttrValue;
        bNameOK = true;
    }
    else if (nAttrToken == XML_TOK_TEXTFIELD_HELP)
    {
        sHelp = sAttrValue;
        bHelpOK = true;
    }
    else if (nAttrToken == XML_TOK_TEXTFIELD_HINT)
    {
        sHint = sAttrValue;
        bHintOK = true;
    }
}

void XMLDropDownFieldImportContext::PrepareField(
    const Reference<XPropertySet>& xPropertySet)
{
    // create sequence
    sal_Int32 nLength = static_cast<sal_Int32>( aLabels.size() );
    Sequence<OUString> aSequence( nLength );
    OUString* pSequence = aSequence.getArray();
    for( sal_Int32 n = 0; n < nLength; n++ )
        pSequence[n] = aLabels[n];

    // now set values:
    Any aAny;

    aAny <<= aSequence;
    xPropertySet->setPropertyValue( sPropertyItems, aAny );

    if( nSelected >= 0  &&  nSelected < nLength )
    {
        aAny <<= pSequence[nSelected];
        xPropertySet->setPropertyValue( sPropertySelectedItem, aAny );
    }

    // set name
    if( bNameOK )
    {
        aAny <<= sName;
        xPropertySet->setPropertyValue( sPropertyName, aAny );
    }
    // set help
    if( bHelpOK )
    {
        aAny <<= sHelp;
        xPropertySet->setPropertyValue( sPropertyHelp, aAny );
    }
    // set hint
    if( bHintOK )
    {
        aAny <<= sHint;
        xPropertySet->setPropertyValue( sPropertyToolTip, aAny );
    }

}

/** import header fields (<draw:header>) */
TYPEINIT1( XMLHeaderFieldImportContext, XMLTextFieldImportContext );

XMLHeaderFieldImportContext::XMLHeaderFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const ::rtl::OUString& sLocalName)      /// element name w/o prefix
: XMLTextFieldImportContext(rImport, rHlp, sAPI_header, nPrfx, sLocalName )
{
    sServicePrefix = OUString(sAPI_presentation_prefix );
    bValid = sal_True;
}

/// process attribute values
void XMLHeaderFieldImportContext::ProcessAttribute( sal_uInt16, const ::rtl::OUString& )
{
}

/// prepare XTextField for insertion into document
void XMLHeaderFieldImportContext::PrepareField(const Reference<XPropertySet> &)
{
}

/** import footer fields (<draw:footer>) */
TYPEINIT1( XMLFooterFieldImportContext, XMLTextFieldImportContext );

XMLFooterFieldImportContext::XMLFooterFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const ::rtl::OUString& sLocalName)      /// element name w/o prefix
: XMLTextFieldImportContext(rImport, rHlp, sAPI_footer, nPrfx, sLocalName )
{
    sServicePrefix = OUString(sAPI_presentation_prefix );
    bValid = sal_True;
}

/// process attribute values
void XMLFooterFieldImportContext::ProcessAttribute( sal_uInt16, const ::rtl::OUString& )
{
}

/// prepare XTextField for insertion into document
void XMLFooterFieldImportContext::PrepareField(const Reference<XPropertySet> &)
{
}


/** import footer fields (<draw:date-and-time>) */
TYPEINIT1( XMLDateTimeFieldImportContext, XMLTextFieldImportContext );

XMLDateTimeFieldImportContext::XMLDateTimeFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const ::rtl::OUString& sLocalName)      /// element name w/o prefix
: XMLTextFieldImportContext(rImport, rHlp, sAPI_datetime, nPrfx, sLocalName )
{
    sServicePrefix = OUString(sAPI_presentation_prefix );
    bValid = sal_True;
}

/// process attribute values
void XMLDateTimeFieldImportContext::ProcessAttribute( sal_uInt16,
                                   const ::rtl::OUString& )
{
}

/// prepare XTextField for insertion into document
void XMLDateTimeFieldImportContext::PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> &)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
