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
 *  Import of all text fields except those from txtvfldi.cxx
 *  (variable related text fields and database display fields)
 */
#include "txtfldi.hxx"
#include "txtvfldi.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumi.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmlnmspe.hxx>
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
#include <osl/diagnose.h>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;


// SO API string constants


// service prefix and service names
const sal_Char sAPI_textfield_prefix[]  = "com.sun.star.text.TextField.";
const sal_Char sAPI_fieldmaster_prefix[] = "com.sun.star.text.FieldMaster.";
const sal_Char sAPI_presentation_prefix[] = "com.sun.star.presentation.TextField.";

const sal_Char sAPI_extended_user[]             = "ExtendedUser";
const sal_Char sAPI_user_data_type[]            = "UserDataType";
const sal_Char sAPI_jump_edit[]                 = "JumpEdit";
const sal_Char sAPI_date_time[]                 = "DateTime";
const sal_Char sAPI_page_number[]               = "PageNumber";
const sal_Char sAPI_database_next[]             = "DatabaseNextSet";
const sal_Char sAPI_database_select[]           = "DatabaseNumberOfSet";
const sal_Char sAPI_database_number[]           = "DatabaseSetNumber";
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
const sal_Char sAPI_pagename[]                  = "PageName";
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
const sal_Char sAPI_author[]            = "Author";
const sal_Char sAPI_initials[]          = "Initials";
const sal_Char sAPI_full_name[]         = "FullName";
const sal_Char sAPI_place_holder_type[] = "PlaceHolderType";
const sal_Char sAPI_place_holder[]      = "PlaceHolder";
const sal_Char sAPI_hint[]              = "Hint";
const sal_Char sAPI_name[]              = "Name";
const sal_Char sAPI_sub_type[]          = "SubType";
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
const sal_Char sAPI_url_content[]       = "URLContent";
const sal_Char sAPI_script_type[]       = "ScriptType";
const sal_Char sAPI_is_hidden[]         = "IsHidden";
const sal_Char sAPI_is_condition_true[] = "IsConditionTrue";
const sal_Char sAPI_data_command_type[] = "DataCommandType";
const sal_Char sAPI_is_fixed_language[] = "IsFixedLanguage";
const sal_Char sAPI_is_visible[]        = "IsVisible";
const sal_Char sAPI_TextRange[]         = "TextRange";

const sal_Char sAPI_true[] = "TRUE";


XMLTextFieldImportContext::XMLTextFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    const sal_Char* pService,
    sal_uInt16 nPrefix, const OUString& rElementName)
:   SvXMLImportContext( rImport, nPrefix, rElementName )
,   rTextImportHelper(rHlp)
,   sServicePrefix(sAPI_textfield_prefix)
,   bValid(false)
{
    DBG_ASSERT(nullptr != pService, "Need service name!");
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

bool XMLTextFieldImportContext::CreateField(
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
            return false;   // can't create instance
        }
    } else {
        return false;   // can't get MultiServiceFactory
    }

    return true;
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
    XMLTextFieldImportContext* pContext = nullptr;

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
                                                          nToken, true,
                                                          false );
            break;
        case XML_TOK_TEXT_DOCUMENT_CREATION_AUTHOR:
        case XML_TOK_TEXT_DOCUMENT_PRINT_AUTHOR:
        case XML_TOK_TEXT_DOCUMENT_SAVE_AUTHOR:
            pContext = new XMLSimpleDocInfoImportContext( rImport, rHlp,
                                                          nPrefix, rName,
                                                          nToken, false,
                                                          true );
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

        case XML_TOK_TEXT_PAGE_NAME:
            pContext = new XMLPageNameFieldImportContext( rImport, rHlp,
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
        case XML_TOK_TEXT_DROP_DOWN:
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
            pContext = nullptr;
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


// XMLSenderFieldImportContext


XMLSenderFieldImportContext::XMLSenderFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName,
    sal_uInt16 nToken)
    : XMLTextFieldImportContext(rImport, rHlp, sAPI_extended_user,nPrfx, sLocalName)
    , nSubType(0)
    , sPropertyFixed(sAPI_is_fixed)
    , sPropertyFieldSubType(sAPI_user_data_type)
    , sPropertyContent(sAPI_content)
    , bFixed(true)
    , nElementToken(nToken)
{
}

void XMLSenderFieldImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    bValid = true;
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
        bValid = false;
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

        // set bFixed if successful
        if (bRet) {
            bFixed = bVal;
        }
    }
}

void XMLSenderFieldImportContext::PrepareField(
    const Reference<XPropertySet> & rPropSet)
{
    // set members
    rPropSet->setPropertyValue(sPropertyFieldSubType, Any(nSubType));

    // set fixed
    rPropSet->setPropertyValue(sPropertyFixed, Any(bFixed));

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
            rPropSet->setPropertyValue(sPropertyContent, Any(GetContent()));
        }
    }
}


// XMLAuthorFieldImportContext


XMLAuthorFieldImportContext::XMLAuthorFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName,
    sal_uInt16 nToken)
:   XMLSenderFieldImportContext(rImport, rHlp, nPrfx, sLocalName, nToken)
,   bAuthorFullName(true)
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
    bValid = true;

    // process Attributes
    XMLTextFieldImportContext::StartElement(xAttrList);
}

void XMLAuthorFieldImportContext::PrepareField(
    const Reference<XPropertySet> & rPropSet)
{
    // set members
    Any aAny;
    rPropSet->setPropertyValue(sPropertyAuthorFullName, Any(bAuthorFullName));

    rPropSet->setPropertyValue(sPropertyFixed, Any(bFixed));

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


// page continuation string


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
,   sStringOK(false)
{
    bValid = true;
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
            sStringOK = true;
            break;
    }
}

void XMLPageContinuationImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    xPropertySet->setPropertyValue(sPropertySubType, Any(eSelectPage));

    aAny <<= (sStringOK ? sString : GetContent());
    xPropertySet->setPropertyValue(sPropertyUserText, aAny);

    aAny <<= style::NumberingType::CHAR_SPECIAL;
    xPropertySet->setPropertyValue(sPropertyNumberingType, aAny);
}


// page number field


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
,   sNumberFormatOK(false)
{
    bValid = true;
}

void XMLPageNumberImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_NUM_FORMAT:
            sNumberFormat = sAttrValue;
            sNumberFormatOK = true;
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

        xPropertySet->setPropertyValue(sPropertyNumberingType, Any(nNumType));
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
                SAL_WARN("xmloff.text", "unknown page number type");
        }
        xPropertySet->setPropertyValue(sPropertyOffset, Any(nPageAdjust));
    }

    if (xPropertySetInfo->hasPropertyByName(sPropertySubType))
    {
        xPropertySet->setPropertyValue(sPropertySubType, Any(eSelectPage));
    }
}


// Placeholder


XMLPlaceholderFieldImportContext::XMLPlaceholderFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName)
:   XMLTextFieldImportContext(rImport, rHlp, sAPI_jump_edit,nPrfx, sLocalName)
,   sPropertyPlaceholderType(sAPI_place_holder_type)
,   sPropertyPlaceholder(sAPI_place_holder)
,   sPropertyHint(sAPI_hint)
,   nPlaceholderType(PlaceholderType::TEXT)
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
        bValid = true;
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
            bValid = false;
        }
        break;

    default:
        ; // ignore
    }
}

void XMLPlaceholderFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet) {

    Any aAny;
    xPropertySet->setPropertyValue(sPropertyHint, Any(sDescription));

    // remove <...> around content (if present)
    OUString aContent = GetContent();
    sal_Int32 nStart = 0;
    sal_Int32 nLength = aContent.getLength();
    if (aContent.startsWith("<"))
    {
        --nLength;
        ++nStart;
    }
    if (aContent.endsWith(">"))
    {
        --nLength;
    }
    aAny <<= aContent.copy(nStart, nLength);
    xPropertySet->setPropertyValue(sPropertyPlaceholder, aAny);

    xPropertySet->setPropertyValue(sPropertyPlaceholderType, Any(nPlaceholderType));
}


// time field


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
,   nAdjust(0)
,   nFormatKey(0)
,   bTimeOK(false)
,   bFormatOK(false)
,   bFixed(false)
,   bIsDate(false)
,   bIsDefaultLanguage( true )
{
    bValid = true;  // always valid!
}

void XMLTimeFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken, const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_TIME_VALUE:
        {
            if (::sax::Converter::parseTimeOrDateTime(aDateTimeValue, nullptr,
                        sAttrValue))
            {
                bTimeOK = true;
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
                bFormatOK = true;
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
    // all properties are optional (except IsDate)
    Reference<XPropertySetInfo> xPropertySetInfo(
        rPropertySet->getPropertySetInfo());

    if (xPropertySetInfo->hasPropertyByName(sPropertyFixed))
    {
        rPropertySet->setPropertyValue(sPropertyFixed, Any(bFixed));
    }

    rPropertySet->setPropertyValue(sPropertyIsDate, Any(bIsDate));

    if (xPropertySetInfo->hasPropertyByName(sPropertyAdjust))
    {
        rPropertySet->setPropertyValue(sPropertyAdjust, Any(nAdjust));
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
                   rPropertySet->setPropertyValue(sPropertyDateTimeValue, Any(aDateTimeValue));
               }
               else if (xPropertySetInfo->hasPropertyByName(sPropertyDateTime))
               {
                   rPropertySet->setPropertyValue(sPropertyDateTime, Any(aDateTimeValue));
               }
            }
        }
    }

    if (bFormatOK &&
        xPropertySetInfo->hasPropertyByName(sPropertyNumberFormat))
    {
        rPropertySet->setPropertyValue(sPropertyNumberFormat, Any(nFormatKey));

        if( xPropertySetInfo->hasPropertyByName( sPropertyIsFixedLanguage ) )
        {
            bool bIsFixedLanguage = ! bIsDefaultLanguage;
            rPropertySet->setPropertyValue( sPropertyIsFixedLanguage, Any(bIsFixedLanguage) );
        }
    }
}


// date field


XMLDateFieldImportContext::XMLDateFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLTimeFieldImportContext(rImport, rHlp, nPrfx, sLocalName)
{
    bIsDate = true; // always a date!
}

void XMLDateFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_DATE_VALUE:
        {
            if (::sax::Converter::parseDateTime(aDateTimeValue, nullptr, sAttrValue))
            {
                bTimeOK = true;
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


// database field superclass


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
,   bCommandTypeOK(false)
,   bDisplay( true )
,   bDisplayOK( false )
,   bUseDisplay( bUseDisply )
,   bDatabaseOK(false)
,   bDatabaseNameOK(false)
,   bDatabaseURLOK(false)
,   bTableOK(false)
{
}

void XMLDatabaseFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken, const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_DATABASE_NAME:
            sDatabaseName = sAttrValue;
            bDatabaseOK = true;
            bDatabaseNameOK = true;
            break;
        case XML_TOK_TEXTFIELD_TABLE_NAME:
            sTableName = sAttrValue;
            bTableOK = true;
            break;
        case XML_TOK_TEXTFIELD_TABLE_TYPE:
            if( IsXMLToken( sAttrValue, XML_TABLE ) )
            {
                nCommandType = sdb::CommandType::TABLE;
                bCommandTypeOK = true;
            }
            else if( IsXMLToken( sAttrValue, XML_QUERY ) )
            {
                nCommandType = sdb::CommandType::QUERY;
                bCommandTypeOK = true;
            }
            else if( IsXMLToken( sAttrValue, XML_COMMAND ) )
            {
                nCommandType = sdb::CommandType::COMMAND;
                bCommandTypeOK = true;
            }
            break;
        case XML_TOK_TEXTFIELD_DISPLAY:
            if( IsXMLToken( sAttrValue, XML_NONE ) )
            {
                bDisplay = false;
                bDisplayOK = true;
            }
            else if( IsXMLToken( sAttrValue, XML_VALUE ) )
            {
                bDisplay = true;
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
                bDatabaseOK = true;
                bDatabaseURLOK = true;
            }
        }

        // we call ProcessAttribute in order to set bValid appropriately
        ProcessAttribute( XML_TOKEN_INVALID, OUString() );
    }

    return SvXMLImportContext::CreateChildContext(p_nPrefix, rLocalName,
                                                  xAttrList);
}


void XMLDatabaseFieldImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(sPropertyTableName, Any(sTableName));

    if( bDatabaseNameOK )
    {
        xPropertySet->setPropertyValue(sPropertyDataBaseName, Any(sDatabaseName));
    }
    else if( bDatabaseURLOK )
    {
        xPropertySet->setPropertyValue(sPropertyDataBaseURL, Any(sDatabaseURL));
    }

    // #99980# load/save command type for all fields; also load
    //         old documents without command type
    if( bCommandTypeOK )
    {
        xPropertySet->setPropertyValue( sPropertyDataCommandType, Any(nCommandType) );
    }

    if( bUseDisplay && bDisplayOK )
    {
        xPropertySet->setPropertyValue( sPropertyIsVisible, Any(bDisplay) );
    }
}


// database name field


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


// database next field


XMLDatabaseNextImportContext::XMLDatabaseNextImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    const sal_Char* pServiceName, sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLDatabaseFieldImportContext(rImport, rHlp, pServiceName,
                                      nPrfx, sLocalName, false),
        sPropertyCondition(sAPI_condition),
        sTrue(sAPI_true),
        sCondition(),
        bConditionOK(false)
{
}

XMLDatabaseNextImportContext::XMLDatabaseNextImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName)
: XMLDatabaseFieldImportContext(rImport, rHlp, sAPI_database_next, nPrfx, sLocalName, false)
,   sPropertyCondition(sAPI_condition)
,   sTrue(sAPI_true)
,   bConditionOK(false)
{
}

void XMLDatabaseNextImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken, const OUString& sAttrValue )
{
    if (XML_TOK_TEXTFIELD_CONDITION == nAttrToken)
    {
        OUString sTmp;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap()._GetKeyByAttrName(
                                    sAttrValue, &sTmp, false );
        if( XML_NAMESPACE_OOOW == nPrefix )
        {
            sCondition = sTmp;
            bConditionOK = true;
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


// database select field


XMLDatabaseSelectImportContext::XMLDatabaseSelectImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLDatabaseNextImportContext(rImport, rHlp, sAPI_database_select,
                                     nPrfx, sLocalName),
        sPropertySetNumber(sAPI_set_number),
        nNumber(0),
        bNumberOK(false)
{
}

void XMLDatabaseSelectImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    if (XML_TOK_TEXTFIELD_ROW_NUMBER == nAttrToken)
    {
        sal_Int32 nTmp;
        if (::sax::Converter::convertNumber( nTmp, sAttrValue
                                               /* , nMin, nMax ??? */ ))
        {
            nNumber = nTmp;
            bNumberOK = true;
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
    xPropertySet->setPropertyValue(sPropertySetNumber, Any(nNumber));

    XMLDatabaseNextImportContext::PrepareField(xPropertySet);
}


// database display row number field


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
        bValueOK(false)
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
                bValueOK = true;
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
    sal_Int16 nNumType = style::NumberingType::ARABIC;
    GetImport().GetMM100UnitConverter().convertNumFormat( nNumType,
                                                    sNumberFormat,
                                                    sNumberSync );
    xPropertySet->setPropertyValue(sPropertyNumberingType, Any(nNumType));

    if (bValueOK)
    {
        xPropertySet->setPropertyValue(sPropertySetNumber, Any(nValue));
    }

    XMLDatabaseFieldImportContext::PrepareField(xPropertySet);
}


// Simple doc info fields


XMLSimpleDocInfoImportContext::XMLSimpleDocInfoImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName, sal_uInt16 nToken,
    bool bContent, bool bAuthor)
:   XMLTextFieldImportContext(rImport, rHlp, MapTokenToServiceName(nToken),nPrfx, sLocalName)
,   sPropertyFixed(sAPI_is_fixed)
,   sPropertyContent(sAPI_content)
,   sPropertyAuthor(sAPI_author)
,   sPropertyCurrentPresentation(sAPI_current_presentation)
,   bFixed(false)
,   bHasAuthor(bAuthor)
,   bHasContent(bContent)
{
    bValid = true;
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
        rPropertySet->setPropertyValue(sPropertyFixed, Any(bFixed));

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
    const sal_Char* pServiceName = nullptr;

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
            pServiceName = nullptr;
            break;
    }

    return pServiceName;
}


// revision field


XMLRevisionDocInfoImportContext::XMLRevisionDocInfoImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName, sal_uInt16 nToken) :
        XMLSimpleDocInfoImportContext(rImport, rHlp, nPrfx, sLocalName,
                                      nToken, false, false),
        sPropertyRevision(sAPI_revision)
{
    bValid = true;
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
                rPropertySet->setPropertyValue(sPropertyRevision, Any(nTmp));
            }
        }
    }
}


// DocInfo fields with date/time attributes


XMLDateTimeDocInfoImportContext::XMLDateTimeDocInfoImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName, sal_uInt16 nToken)
    : XMLSimpleDocInfoImportContext(rImport, rHlp, nPrfx, sLocalName,nToken, false, false)
    , sPropertyNumberFormat(sAPI_number_format)
    , sPropertyIsDate(sAPI_is_date)
    , sPropertyIsFixedLanguage(sAPI_is_fixed_language)
    , nFormat(0)
    , bFormatOK(false)
    , bIsDate(false)
    , bHasDateTime(false)
    , bIsDefaultLanguage(true)
{
    // we allow processing of EDIT_DURATION here, because import of actual
    // is not supported anyway. If it was, we'd need an extra import class
    // because times and time durations are presented differently!

    bValid = true;
    switch (nToken)
    {
        case XML_TOK_TEXT_DOCUMENT_CREATION_DATE:
        case XML_TOK_TEXT_DOCUMENT_PRINT_DATE:
        case XML_TOK_TEXT_DOCUMENT_SAVE_DATE:
            bIsDate = true;
            bHasDateTime = true;
            break;
        case XML_TOK_TEXT_DOCUMENT_CREATION_TIME:
        case XML_TOK_TEXT_DOCUMENT_PRINT_TIME:
        case XML_TOK_TEXT_DOCUMENT_SAVE_TIME:
            bIsDate = false;
            bHasDateTime = true;
            break;
        case XML_TOK_TEXT_DOCUMENT_EDIT_DURATION:
            bIsDate = false;
            bHasDateTime = false;
            break;
        default:
            OSL_FAIL(
                "XMLDateTimeDocInfoImportContext needs date/time doc. fields");
            bValid = false;
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
                bFormatOK = true;
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

    if (bHasDateTime)
    {
        xPropertySet->setPropertyValue(sPropertyIsDate, Any(bIsDate));
    }

    if (bFormatOK)
    {
        xPropertySet->setPropertyValue(sPropertyNumberFormat, Any(nFormat));

        if( xPropertySet->getPropertySetInfo()->
                hasPropertyByName( sPropertyIsFixedLanguage ) )
        {
            bool bIsFixedLanguage = ! bIsDefaultLanguage;
            xPropertySet->setPropertyValue( sPropertyIsFixedLanguage, Any(bIsFixedLanguage) );
        }
    }

    // can't set date/time/duration value! Sorry.
}


// user defined docinfo fields


XMLUserDocInfoImportContext::XMLUserDocInfoImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName, sal_uInt16 nToken) :
        XMLSimpleDocInfoImportContext(rImport, rHlp, nPrfx,
                                      sLocalName, nToken,
                                      false, false)
    , sPropertyName(sAPI_name)
    , sPropertyNumberFormat(sAPI_number_format)
    , sPropertyIsFixedLanguage(sAPI_is_fixed_language)
    , nFormat(0)
    , bFormatOK(false)
    , bIsDefaultLanguage( true )
{
    bValid = false;
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
                bFormatOK = true;
            }
            break;
        }
        case XML_TOK_TEXTFIELD_NAME:
        {
            if (!bValid)
            {
                SetServiceName(sAPI_docinfo_custom );
                aName = sAttrValue;
                bValid = true;
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
    if ( !aName.isEmpty() )
    {
        xPropertySet->setPropertyValue(sPropertyName, Any(aName));
    }
    Reference<XPropertySetInfo> xPropertySetInfo(
        xPropertySet->getPropertySetInfo());
    if (bFormatOK &&
        xPropertySetInfo->hasPropertyByName(sPropertyNumberFormat))
    {
        xPropertySet->setPropertyValue(sPropertyNumberFormat, Any(nFormat));

        if( xPropertySetInfo->hasPropertyByName( sPropertyIsFixedLanguage ) )
        {
            bool bIsFixedLanguage = ! bIsDefaultLanguage;
            xPropertySet->setPropertyValue( sPropertyIsFixedLanguage, Any(bIsFixedLanguage) );
        }
    }

    // call superclass to handle "fixed"
    XMLSimpleDocInfoImportContext::PrepareField(xPropertySet);
}


// import hidden paragraph fields


XMLHiddenParagraphImportContext::XMLHiddenParagraphImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_hidden_paragraph,
                                  nPrfx, sLocalName),
        sPropertyCondition(sAPI_condition),
        sPropertyIsHidden(sAPI_is_hidden),
        sCondition(),
        bIsHidden(false)
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
                                    sAttrValue, &sTmp, false );
        if( XML_NAMESPACE_OOOW == nPrefix )
        {
            sCondition = sTmp;
            bValid = true;
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
    xPropertySet->setPropertyValue(sPropertyCondition, Any(sCondition));
    xPropertySet->setPropertyValue(sPropertyIsHidden, Any(bIsHidden));
}


// import conditional text (<text:conditional-text>)


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
        bConditionOK(false),
        bTrueOK(false),
        bFalseOK(false),
        bCurrentValue(false)
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
                        _GetKeyByAttrName( sAttrValue, &sTmp, false );
                if( XML_NAMESPACE_OOOW == nPrefix )
                {
                    sCondition = sTmp;
                    bConditionOK = true;
                }
                else
                    sCondition = sAttrValue;
            }
            break;
        case XML_TOK_TEXTFIELD_STRING_VALUE_IF_FALSE:
            sFalseContent = sAttrValue;
            bFalseOK = true;
            break;
        case XML_TOK_TEXTFIELD_STRING_VALUE_IF_TRUE:
            sTrueContent = sAttrValue;
            bTrueOK = true;
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
    xPropertySet->setPropertyValue(sPropertyCondition, Any(sCondition));
    xPropertySet->setPropertyValue(sPropertyFalseContent, Any(sFalseContent));
    xPropertySet->setPropertyValue(sPropertyTrueContent, Any(sTrueContent));
    xPropertySet->setPropertyValue(sPropertyIsConditionTrue, Any(bCurrentValue));
    xPropertySet->setPropertyValue(sPropertyCurrentPresentation, Any(GetContent()));
}


// hidden text


XMLHiddenTextImportContext::XMLHiddenTextImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_hidden_text,
                                  nPrfx, sLocalName),
        sPropertyCondition(sAPI_condition),
        sPropertyContent(sAPI_content),
        sPropertyIsHidden(sAPI_is_hidden),
        bConditionOK(false),
        bStringOK(false),
        bIsHidden(false)
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
                                        _GetKeyByAttrName( sAttrValue, &sTmp, false );
                if( XML_NAMESPACE_OOOW == nPrefix )
                {
                    sCondition = sTmp;
                    bConditionOK = true;
                }
                else
                    sCondition = sAttrValue;
            }
            break;
        case XML_TOK_TEXTFIELD_STRING_VALUE:
            sString = sAttrValue;
            bStringOK = true;
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
    xPropertySet->setPropertyValue(sPropertyCondition, Any(sCondition));
    xPropertySet->setPropertyValue(sPropertyContent, Any(sString));
    xPropertySet->setPropertyValue(sPropertyIsHidden, Any(bIsHidden));
}


// file name fields


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
        bFixed(false)
{
    bValid = true;
}

void XMLFileNameImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
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
            ; // unknown attribute: ignore
            break;
    }
}

void XMLFileNameImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    // properties are optional
    Reference<XPropertySetInfo> xPropertySetInfo(
        xPropertySet->getPropertySetInfo());

     if (xPropertySetInfo->hasPropertyByName(sPropertyFixed))
     {
         xPropertySet->setPropertyValue(sPropertyFixed, Any(bFixed));
     }

    if (xPropertySetInfo->hasPropertyByName(sPropertyFileFormat))
    {
        xPropertySet->setPropertyValue(sPropertyFileFormat, Any(nFormat));
    }

    if (xPropertySetInfo->hasPropertyByName(sPropertyCurrentPresentation))
    {
        xPropertySet->setPropertyValue(sPropertyCurrentPresentation, Any(GetContent()));
    }
}


// template name field


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


XMLTemplateNameImportContext::XMLTemplateNameImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_template_name,
                                  nPrfx, sLocalName),
        sPropertyFileFormat(sAPI_file_format),
        nFormat(TemplateDisplayFormat::FULL)
{
    bValid = true;
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
    xPropertySet->setPropertyValue(sPropertyFileFormat, Any(nFormat));
}


// import chapter fields


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
    bValid = true;
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
    xPropertySet->setPropertyValue(sPropertyChapterFormat, Any(nFormat));
    xPropertySet->setPropertyValue(sPropertyLevel, Any(nLevel));
}


// counting fields


XMLCountFieldImportContext::XMLCountFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName, sal_uInt16 nToken) :
        XMLTextFieldImportContext(rImport, rHlp, MapTokenToServiceName(nToken),
                                  nPrfx, sLocalName),
        sPropertyNumberingType(
            sAPI_numbering_type),
        sNumberFormat(),
        sLetterSync(),
        bNumberFormatOK(false)
{
    bValid = true;
}

void XMLCountFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_NUM_FORMAT:
            sNumberFormat = sAttrValue;
            bNumberFormatOK = true;
            break;
        case XML_TOK_TEXTFIELD_NUM_LETTER_SYNC:
            sLetterSync = sAttrValue;
            break;
    }
}

void XMLCountFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
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
        xPropertySet->setPropertyValue(sPropertyNumberingType, Any(nNumType));
    }
}

const sal_Char* XMLCountFieldImportContext::MapTokenToServiceName(
    sal_uInt16 nToken)
{
    const sal_Char* pServiceName = nullptr;

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
            pServiceName = nullptr;
            OSL_FAIL("unknown count field!");
            break;
    }

    return pServiceName;
}


// page variable import


XMLPageVarGetFieldImportContext::XMLPageVarGetFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx, const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_reference_page_get,
                                  nPrfx, sLocalName),
        sPropertyNumberingType(
            sAPI_numbering_type),
        sNumberFormat(),
        sLetterSync(),
        bNumberFormatOK(false)
{
    bValid = true;
}

void XMLPageVarGetFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_TOK_TEXTFIELD_NUM_FORMAT:
            sNumberFormat = sAttrValue;
            bNumberFormatOK = true;
            break;
        case XML_TOK_TEXTFIELD_NUM_LETTER_SYNC:
            sLetterSync = sAttrValue;
            break;
    }
}

void XMLPageVarGetFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
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
    xPropertySet->setPropertyValue(sPropertyNumberingType, Any(nNumType));

    // display old content (#96657#)
    xPropertySet->setPropertyValue( sAPI_current_presentation, Any(GetContent()) );
}


// page variable set fields


XMLPageVarSetFieldImportContext::XMLPageVarSetFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_reference_page_set,
                                  nPrfx, sLocalName),
        sPropertyOn(sAPI_on),
        sPropertyOffset(sAPI_offset),
        nAdjust(0),
        bActive(true)
{
    bValid = true;
}

void XMLPageVarSetFieldImportContext::ProcessAttribute(
    sal_uInt16 nAttrToken,
    const OUString& sAttrValue )
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
        break;
    }
    case XML_TOK_TEXTFIELD_PAGE_ADJUST:
    {
        sal_Int32 nTmp(0);
        if (::sax::Converter::convertNumber(nTmp, sAttrValue))
        {
            nAdjust = (sal_Int16)nTmp;
        }
        break;
    }
    default:
        break;
    }
}

void XMLPageVarSetFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(sPropertyOn, Any(bActive));
    xPropertySet->setPropertyValue(sPropertyOffset, Any(nAdjust));
}


// macro fields


XMLMacroFieldImportContext::XMLMacroFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_macro,
                                  nPrfx, sLocalName),
        sPropertyHint(sAPI_hint),
        sPropertyMacroName("MacroName"),
        sPropertyScriptURL("ScriptURL"),
        bDescriptionOK(false)
{
}

SvXMLImportContext* XMLMacroFieldImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = nullptr;

    if ( (nPrefix == XML_NAMESPACE_OFFICE) &&
         IsXMLToken( rLocalName, XML_EVENT_LISTENERS ) )
    {
        // create events context and remember it!
        pContext = new XMLEventsImportContext(
            GetImport(), nPrefix, rLocalName );
        xEventContext = pContext;
        bValid = true;
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
            bDescriptionOK = true;
            break;
        case XML_TOK_TEXTFIELD_NAME:
            sMacro = sAttrValue;
            bValid = true;
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
            static_cast<XMLEventsImportContext*>(&xEventContext);
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

    xPropertySet->setPropertyValue(sPropertyScriptURL, Any(sScriptURL));
    xPropertySet->setPropertyValue(sPropertyMacroName, Any(sMacroName));
    xPropertySet->setPropertyValue(sPropertyMacroLibrary, Any(sLibraryName));
}


// reference field import


XMLReferenceFieldImportContext::XMLReferenceFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_uInt16 nToken, sal_uInt16 nPrfx, const OUString& sLocalName)
:   XMLTextFieldImportContext(rImport, rHlp, sAPI_get_reference, nPrfx, sLocalName)
,   sPropertyReferenceFieldPart(sAPI_reference_field_part)
,   sPropertyReferenceFieldSource(sAPI_reference_field_source)
,   sPropertySourceName(sAPI_source_name)
,   sPropertyCurrentPresentation(sAPI_current_presentation)
,   nElementToken(nToken)
,   nSource(0)
,   nType(ReferenceFieldPart::PAGE_DESC)
,   bNameOK(false)
,   bTypeOK(false)
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
    bTypeOK = true;
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
            bTypeOK = false;
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
            bNameOK = true;
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
    xPropertySet->setPropertyValue(sPropertyReferenceFieldPart, Any(nType));

    xPropertySet->setPropertyValue(sPropertyReferenceFieldSource, Any(nSource));

    switch (nElementToken)
    {
        case XML_TOK_TEXT_REFERENCE_REF:
        case XML_TOK_TEXT_BOOKMARK_REF:
            xPropertySet->setPropertyValue(sPropertySourceName, Any(sName));
            break;

        case XML_TOK_TEXT_NOTE_REF:
            GetImportHelper().ProcessFootnoteReference(sName, xPropertySet);
            break;

        case XML_TOK_TEXT_SEQUENCE_REF:
            GetImportHelper().ProcessSequenceReference(sName, xPropertySet);
            break;
    }

    xPropertySet->setPropertyValue(sPropertyCurrentPresentation, Any(GetContent()));
}


// field declarations container


enum DdeFieldDeclAttrs
{
    XML_TOK_DDEFIELD_NAME,
    XML_TOK_DDEFIELD_APPLICATION,
    XML_TOK_DDEFIELD_TOPIC,
    XML_TOK_DDEFIELD_ITEM,
    XML_TOK_DDEFIELD_UPDATE
};

static const SvXMLTokenMapEntry aDdeDeclAttrTokenMap[] =
{
    { XML_NAMESPACE_OFFICE, XML_NAME, XML_TOK_DDEFIELD_NAME },
    { XML_NAMESPACE_OFFICE, XML_DDE_APPLICATION, XML_TOK_DDEFIELD_APPLICATION },
    { XML_NAMESPACE_OFFICE, XML_DDE_TOPIC, XML_TOK_DDEFIELD_TOPIC },
    { XML_NAMESPACE_OFFICE, XML_DDE_ITEM, XML_TOK_DDEFIELD_ITEM },
    { XML_NAMESPACE_OFFICE, XML_AUTOMATIC_UPDATE, XML_TOK_DDEFIELD_UPDATE },
    XML_TOKEN_MAP_END
};


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


// import dde field declaration


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
    bool bNameOK = false;
    bool bCommandApplicationOK = false;
    bool bCommandTopicOK = false;
    bool bCommandItemOK = false;

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
                bNameOK = true;
                break;
            case XML_TOK_DDEFIELD_APPLICATION:
                sCommandApplication = xAttrList->getValueByIndex(i);
                bCommandApplicationOK = true;
                break;
            case XML_TOK_DDEFIELD_TOPIC:
                sCommandTopic = xAttrList->getValueByIndex(i);
                bCommandTopicOK = true;
                break;
            case XML_TOK_DDEFIELD_ITEM:
                sCommandItem = xAttrList->getValueByIndex(i);
                bCommandItemOK = true;
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
        sBuf.append(sAPI_fieldmaster_prefix);
        sBuf.append(sAPI_dde);

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
                        xPropSet->setPropertyValue(sPropertyName, Any(sName));

                        xPropSet->setPropertyValue(sPropertyDDECommandType, Any(sCommandApplication));

                        xPropSet->setPropertyValue(sPropertyDDECommandFile, Any(sCommandTopic));

                        xPropSet->setPropertyValue(sPropertyDDECommandElement,
                                                   Any(sCommandItem));

                        xPropSet->setPropertyValue(sPropertyIsAutomaticUpdate,
                                                   Any(bUpdate));
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


// DDE field import


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
    const OUString& sAttrValue )
{
    if (XML_TOK_TEXTFIELD_CONNECTION_NAME == nAttrToken)
    {
        sName = sAttrValue;
        bValid = true;
    }
}

void XMLDdeFieldImportContext::EndElement()
{
    if (bValid)
    {
        // find master
        OUStringBuffer sBuf;
        sBuf.append(sAPI_fieldmaster_prefix);
        sBuf.append(sAPI_dde);
        sBuf.append('.');
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
            sBuf.append(sAPI_textfield_prefix);
            sBuf.append(sAPI_dde);
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


// sheet name fields


XMLSheetNameImportContext::XMLSheetNameImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx,
    const OUString& sLocalName) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_sheet_name,
                                  nPrfx, sLocalName)
{
    bValid = true;  // always valid!
}

void XMLSheetNameImportContext::ProcessAttribute(
    sal_uInt16,
    const OUString& )
{
    // no attributes -> nothing to be done
}

void XMLSheetNameImportContext::PrepareField(
    const Reference<XPropertySet> &)
{
    // no attributes -> nothing to be done
}

/** import page|slide name fields (<text:page-name>) */

XMLPageNameFieldImportContext::XMLPageNameFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName)      /// element name w/o prefix
: XMLTextFieldImportContext(rImport, rHlp, sAPI_pagename, nPrfx, sLocalName )
{
    bValid = true;
}

/// process attribute values
void XMLPageNameFieldImportContext::ProcessAttribute( sal_uInt16,
                                   const OUString& )
{
}

/// prepare XTextField for insertion into document
void XMLPageNameFieldImportContext::PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> &)
{
}


// URL fields (Calc, Impress, Draw)


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
        bFrameOK(false)
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
            bValid = true;
            break;
        case XML_TOK_TEXTFIELD_TARGET_FRAME:
            sFrame = sAttrValue;
            bFrameOK = true;
            break;
        default:
            // ignore
            break;
    }
}

void XMLUrlFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(sPropertyURL, Any(sURL));

    if (bFrameOK)
    {
        xPropertySet->setPropertyValue(sPropertyTargetFrame, Any(sFrame));
    }

    xPropertySet->setPropertyValue(sPropertyRepresentation, Any(GetContent()));
}


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
    bValid = true;
}

// TODO: this is the same map as is used in the text field export
static SvXMLEnumMapEntry const aBibliographyDataTypeMap[] =
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


// we'll process attributes on our own and for fit the standard
// textfield mechanism, because our attributes have zero overlap with
// all the other textfields.
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
    xPropertySet->setPropertyValue(sPropertyFields, Any(aValueSequence));
}

const sal_Char* XMLBibliographyFieldImportContext::MapBibliographyFieldName(
    const OUString& sName)
{
    const sal_Char* pName = nullptr;

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
        pName = nullptr;
    }

    return pName;
}


// Annotation Field


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
    bValid = true;

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
    SvXMLImportContext *pContext = nullptr;
    if( XML_NAMESPACE_DC == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_CREATOR ) )
            pContext = new XMLStringBufferImportContext(GetImport(), nPrefix,
                                            rLocalName, aAuthorBuffer);
        else if( IsXMLToken( rLocalName, XML_DATE ) )
            pContext = new XMLStringBufferImportContext(GetImport(), nPrefix,
                                            rLocalName, aDateBuffer);
    }
    else if( XML_NAMESPACE_TEXT == nPrefix || XML_NAMESPACE_LO_EXT == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_SENDER_INITIALS ) )
            pContext = new XMLStringBufferImportContext(GetImport(), nPrefix,
                                            rLocalName, aInitialsBuffer);
    }

    if( !pContext )
    {
        try
        {
            bool bOK = true;
            if ( !mxField.is() )
                bOK = CreateField( mxField, sServicePrefix + GetServiceName() );
            if (bOK)
            {
                Any aAny = mxField->getPropertyValue( sPropertyTextRange );
                Reference< XText > xText;
                aAny >>= xText;
                if( xText.is() )
                {
                    rtl::Reference < XMLTextImportHelper > xTxtImport = GetImport().GetTextImport();
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
        if ( m_nToken == XML_TOK_TEXT_ANNOTATION_END )
        {
            // Search for a previous annotation with the same name.
            uno::Reference< text::XTextContent > xPrevField;
            {
                Reference<XTextFieldsSupplier> xTextFieldsSupplier(GetImport().GetModel(), UNO_QUERY);
                uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
                uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
                while (xFields->hasMoreElements())
                {
                    uno::Reference<beans::XPropertySet> xCurrField(xFields->nextElement(), uno::UNO_QUERY);
                    uno::Reference<beans::XPropertySetInfo> const xInfo(
                            xCurrField->getPropertySetInfo());
                    if (xInfo->hasPropertyByName(sPropertyName))
                    {
                        OUString aFieldName;
                        xCurrField->getPropertyValue(sPropertyName) >>= aFieldName;
                        if (aFieldName == aName)
                        {
                            xPrevField.set( xCurrField, uno::UNO_QUERY );
                            break;
                        }
                    }
                }
            }
            if ( xPrevField.is() )
            {
                // So we are ending a previous annotation,
                // let's create a text range covering the old and the current position.
                uno::Reference<text::XText> xText = GetImportHelper().GetText();
                uno::Reference<text::XTextCursor> xCursor =
                    xText->createTextCursorByRange(GetImportHelper().GetCursorAsRange());
                xCursor->gotoRange(xPrevField->getAnchor(), true);
                uno::Reference<text::XTextRange> xTextRange(xCursor, uno::UNO_QUERY);

                xText->insertTextContent(xTextRange, xPrevField, !xCursor->isCollapsed());
            }
        }
        else
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
                    GetImportHelper().InsertTextContent( xTextContent );
                }
                catch (const lang::IllegalArgumentException&)
                {
                    // ignore
                }
            }
        }
    }
    else
        GetImportHelper().InsertString(GetContent());
}

void XMLAnnotationImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet )
{
    // import (possibly empty) author
    OUString sAuthor( aAuthorBuffer.makeStringAndClear() );
    xPropertySet->setPropertyValue(sPropertyAuthor, makeAny(sAuthor));

    // import (possibly empty) initials
    OUString sInitials( aInitialsBuffer.makeStringAndClear() );
    xPropertySet->setPropertyValue(sPropertyInitials, makeAny(sInitials));

    util::DateTime aDateTime;
    if (::sax::Converter::parseDateTime(aDateTime, nullptr,
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
        if (sal_Char(0x0a) == sBuffer[sBuffer.getLength()-1])
            sBuffer = sBuffer.copy(0, sBuffer.getLength()-1);
        xPropertySet->setPropertyValue(sPropertyContent, makeAny(sBuffer));
    }

    if (!aName.isEmpty())
        xPropertySet->setPropertyValue(sPropertyName, makeAny(aName));
}


// script field


XMLScriptImportContext::XMLScriptImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrfx,
    const OUString& sLocalName)
:   XMLTextFieldImportContext(rImport, rHlp, sAPI_script, nPrfx, sLocalName)
,   sPropertyScriptType(sAPI_script_type)
,   sPropertyURLContent(sAPI_url_content)
,   sPropertyContent(sAPI_content)
,   bContentOK(false)
,   bScriptTypeOK(false)
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
            bContentOK = true;
            break;

        case XML_TOK_TEXTFIELD_LANGUAGE:
            sScriptType = sAttrValue;
            bScriptTypeOK = true;
            break;

        default:
            // ignore
            break;
    }

    // always valid (even without ScriptType; cf- #96531#)
    bValid = true;
}

void XMLScriptImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    // if href attribute was present, we use it. Else we use element content
    if (! bContentOK)
    {
        sContent = GetContent();
    }
    xPropertySet->setPropertyValue(sPropertyContent, Any(sContent));

    // URL or script text? We use URL if we have an href-attribute
    xPropertySet->setPropertyValue(sPropertyURLContent, Any(bContentOK));

    xPropertySet->setPropertyValue(sPropertyScriptType, Any(sScriptType));
}


// measure field


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
                mnKind = 0; bValid = true;
            }
            else if( IsXMLToken( sAttrValue, XML_UNIT ) )
            {
                mnKind = 1; bValid = true;
            }
            else if( IsXMLToken( sAttrValue, XML_GAP ) )
            {
                mnKind = 2; bValid = true;
            }
            break;
    }
}

void XMLMeasureFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue("Kind", Any(mnKind));
}


// dropdown field


XMLDropDownFieldImportContext::XMLDropDownFieldImportContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rHlp,
        sal_uInt16 nPrfx,
        const OUString& sLocalName) :
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
    bValid = true;
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
        bool bIsSelected = false;
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
    const OUString& sAttrValue )
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

    xPropertySet->setPropertyValue( sPropertyItems, Any(aSequence) );

    if( nSelected >= 0  &&  nSelected < nLength )
    {
        xPropertySet->setPropertyValue( sPropertySelectedItem, Any(pSequence[nSelected]) );
    }

    // set name
    if( bNameOK )
    {
        xPropertySet->setPropertyValue( sPropertyName, Any(sName) );
    }
    // set help
    if( bHelpOK )
    {
        xPropertySet->setPropertyValue( sPropertyHelp, Any(sHelp) );
    }
    // set hint
    if( bHintOK )
    {
        xPropertySet->setPropertyValue( sPropertyToolTip, Any(sHint) );
    }

}

/** import header fields (<draw:header>) */

XMLHeaderFieldImportContext::XMLHeaderFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName)      /// element name w/o prefix
: XMLTextFieldImportContext(rImport, rHlp, sAPI_header, nPrfx, sLocalName )
{
    sServicePrefix = sAPI_presentation_prefix;
    bValid = true;
}

/// process attribute values
void XMLHeaderFieldImportContext::ProcessAttribute( sal_uInt16, const OUString& )
{
}

/// prepare XTextField for insertion into document
void XMLHeaderFieldImportContext::PrepareField(const Reference<XPropertySet> &)
{
}

/** import footer fields (<draw:footer>) */

XMLFooterFieldImportContext::XMLFooterFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName)      /// element name w/o prefix
: XMLTextFieldImportContext(rImport, rHlp, sAPI_footer, nPrfx, sLocalName )
{
    sServicePrefix = sAPI_presentation_prefix;
    bValid = true;
}

/// process attribute values
void XMLFooterFieldImportContext::ProcessAttribute( sal_uInt16, const OUString& )
{
}

/// prepare XTextField for insertion into document
void XMLFooterFieldImportContext::PrepareField(const Reference<XPropertySet> &)
{
}


/** import footer fields (<draw:date-and-time>) */

XMLDateTimeFieldImportContext::XMLDateTimeFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName)      /// element name w/o prefix
: XMLTextFieldImportContext(rImport, rHlp, sAPI_datetime, nPrfx, sLocalName )
{
    sServicePrefix = sAPI_presentation_prefix;
    bValid = true;
}

/// process attribute values
void XMLDateTimeFieldImportContext::ProcessAttribute( sal_uInt16,
                                   const OUString& )
{
}

/// prepare XTextField for insertion into document
void XMLDateTimeFieldImportContext::PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> &)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
