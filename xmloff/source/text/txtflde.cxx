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
 *  export of all text fields
 */
#include "txtflde.hxx"
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlnumfe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnume.hxx>
#include "xmloff/numehelp.hxx"

#include <xmloff/families.hxx>
#include <xmloff/XMLEventExport.hxx>
#include "XMLTextCharStyleNamesElementExport.hxx"
#include <xmloff/nmspmap.hxx>
#include <sax/tools/converter.hxx>

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/UserDataPart.hpp>
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>

#include <com/sun/star/text/SetVariableType.hpp>
#include <com/sun/star/text/PlaceholderType.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/text/ChapterFormat.hpp>
#include <com/sun/star/text/TemplateDisplayFormat.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/text/BibliographyDataType.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/rdf/XMetadatable.hpp>
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>
#include <rtl/math.hxx>

#include <vector>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::container;
using namespace ::xmloff::token;


static sal_Char const FIELD_SERVICE_SENDER[] = "ExtendedUser";
static sal_Char const FIELD_SERVICE_AUTHOR[] = "Author";
static sal_Char const FIELD_SERVICE_JUMPEDIT[] = "JumpEdit";
static sal_Char const FIELD_SERVICE_GETEXP[] = "GetExpression";
static sal_Char const FIELD_SERVICE_SETEXP[] = "SetExpression";
static sal_Char const FIELD_SERVICE_USER[] = "User";
static sal_Char const FIELD_SERVICE_INPUT[] = "Input";
static sal_Char const FIELD_SERVICE_USERINPUT[] = "InputUser";
static sal_Char const FIELD_SERVICE_DATETIME[] = "DateTime";
static sal_Char const FIELD_SERVICE_PAGENUMBER[] = "PageNumber";
static sal_Char const FIELD_SERVICE_DB_NEXT[] = "DatabaseNextSet";
static sal_Char const FIELD_SERVICE_DB_SELECT[] = "DatabaseNumberOfSet";
static sal_Char const FIELD_SERVICE_DB_NUMBER[] = "DatabaseSetNumber";
static sal_Char const FIELD_SERVICE_DB_DISPLAY[] = "Database";
static sal_Char const FIELD_SERVICE_DB_NAME[] = "DatabaseName";
static sal_Char const FIELD_SERVICE_CONDITIONAL_TEXT[] = "ConditionalText";
static sal_Char const FIELD_SERVICE_HIDDEN_TEXT[] = "HiddenText";
static sal_Char const FIELD_SERVICE_HIDDEN_PARAGRAPH[] = "HiddenParagraph";
static sal_Char const FIELD_SERVICE_DOC_INFO_CHANGE_AUTHOR[] = "DocInfo.ChangeAuthor";
static sal_Char const FIELD_SERVICE_DOC_INFO_CHANGE_AUTHOR2[] = "docinfo.ChangeAuthor";
static sal_Char const FIELD_SERVICE_DOC_INFO_CHANGE_DATE_TIME[] = "DocInfo.ChangeDateTime";
static sal_Char const FIELD_SERVICE_DOC_INFO_CHANGE_DATE_TIME2[] = "docinfo.ChangeDateTime";
static sal_Char const FIELD_SERVICE_DOC_INFO_EDIT_TIME[] = "DocInfo.EditTime";
static sal_Char const FIELD_SERVICE_DOC_INFO_EDIT_TIME2[] = "docinfo.EditTime";
static sal_Char const FIELD_SERVICE_DOC_INFO_DESCRIPTION[] = "DocInfo.Description";
static sal_Char const FIELD_SERVICE_DOC_INFO_DESCRIPTION2[] = "docinfo.Description";
static sal_Char const FIELD_SERVICE_DOC_INFO_CREATE_AUTHOR[] = "DocInfo.CreateAuthor";
static sal_Char const FIELD_SERVICE_DOC_INFO_CREATE_AUTHOR2[] = "docinfo.CreateAuthor";
static sal_Char const FIELD_SERVICE_DOC_INFO_CREATE_DATE_TIME[] = "DocInfo.CreateDateTime";
static sal_Char const FIELD_SERVICE_DOC_INFO_CREATE_DATE_TIME2[] = "docinfo.CreateDateTime";
static sal_Char const FIELD_SERVICE_DOC_INFO_CUSTOM[] = "DocInfo.Custom";
static sal_Char const FIELD_SERVICE_DOC_INFO_CUSTOM2[] = "docinfo.Custom";
static sal_Char const FIELD_SERVICE_DOC_INFO_PRINT_AUTHOR[] = "DocInfo.PrintAuthor";
static sal_Char const FIELD_SERVICE_DOC_INFO_PRINT_AUTHOR2[] = "docinfo.PrintAuthor";
static sal_Char const FIELD_SERVICE_DOC_INFO_PRINT_DATE_TIME[] = "DocInfo.PrintDateTime";
static sal_Char const FIELD_SERVICE_DOC_INFO_PRINT_DATE_TIME2[] = "docinfo.PrintDateTime";
static sal_Char const FIELD_SERVICE_DOC_INFO_KEY_WORDS[] = "DocInfo.KeyWords";
static sal_Char const FIELD_SERVICE_DOC_INFO_KEY_WORDS2[] = "docinfo.KeyWords";
static sal_Char const FIELD_SERVICE_DOC_INFO_SUBJECT[] = "DocInfo.Subject";
static sal_Char const FIELD_SERVICE_DOC_INFO_SUBJECT2[] = "docinfo.Subject";
static sal_Char const FIELD_SERVICE_DOC_INFO_TITLE[] = "DocInfo.Title";
static sal_Char const FIELD_SERVICE_DOC_INFO_TITLE2[] = "docinfo.Title";
static sal_Char const FIELD_SERVICE_DOC_INFO_REVISION[] = "DocInfo.Revision";
static sal_Char const FIELD_SERVICE_DOC_INFO_REVISION2[] = "docinfo.Revision";
static sal_Char const FIELD_SERVICE_FILE_NAME[] = "FileName";
static sal_Char const FIELD_SERVICE_CHAPTER[] = "Chapter";
static sal_Char const FIELD_SERVICE_TEMPLATE_NAME[] = "TemplateName";
static sal_Char const FIELD_SERVICE_PAGE_COUNT[] = "PageCount";
static sal_Char const FIELD_SERVICE_PARAGRAPH_COUNT[] = "ParagraphCount";
static sal_Char const FIELD_SERVICE_WORD_COUNT[] = "WordCount";
static sal_Char const FIELD_SERVICE_CHARACTER_COUNT[] = "CharacterCount";
static sal_Char const FIELD_SERVICE_TABLE_COUNT[] = "TableCount";
static sal_Char const FIELD_SERVICE_GRAPHIC_COUNT[] = "GraphicObjectCount";
static sal_Char const FIELD_SERVICE_OBJECT_COUNT[] = "EmbeddedObjectCount";
static sal_Char const FIELD_SERVICE_REFERENCE_PAGE_SET[] = "ReferencePageSet";
static sal_Char const FIELD_SERVICE_REFERENCE_PAGE_GET[] = "ReferencePageGet";
static sal_Char const FIELD_SERVICE_SHEET_NAME[] = "SheetName";
static sal_Char const FIELD_SERVICE_MACRO[] = "Macro";
static sal_Char const FIELD_SERVICE_GET_REFERENCE[] = "GetReference";
static sal_Char const FIELD_SERVICE_DDE[] = "DDE";
static sal_Char const FIELD_SERVICE_URL[] = "URL";
static sal_Char const FIELD_SERVICE_BIBLIOGRAPHY[] = "Bibliography";
static sal_Char const FIELD_SERVICE_SCRIPT[] = "Script";
static sal_Char const FIELD_SERVICE_ANNOTATION[] = "Annotation";
static sal_Char const FIELD_SERVICE_COMBINED_CHARACTERS[] = "CombinedCharacters";
static sal_Char const FIELD_SERVICE_META[] = "MetadataField";
static sal_Char const FIELD_SERVICE_MEASURE[] = "Measure";
static sal_Char const FIELD_SERVICE_TABLE_FORMULA[] = "TableFormula";
static sal_Char const FIELD_SERVICE_DROP_DOWN[] = "DropDown";

SvXMLEnumStringMapEntry const aFieldServiceNameMapping[] =
{
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_SENDER, FIELD_ID_SENDER ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_AUTHOR, FIELD_ID_AUTHOR ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_JUMPEDIT, FIELD_ID_PLACEHOLDER ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_GETEXP, FIELD_ID_VARIABLE_GET ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_SETEXP, FIELD_ID_VARIABLE_SET ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_USER, FIELD_ID_USER_GET ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_INPUT, FIELD_ID_TEXT_INPUT ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_USERINPUT, FIELD_ID_USER_INPUT ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DATETIME, FIELD_ID_TIME ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_PAGENUMBER, FIELD_ID_PAGENUMBER ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_REFERENCE_PAGE_SET, FIELD_ID_REFPAGE_SET ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_REFERENCE_PAGE_GET, FIELD_ID_REFPAGE_GET ),

    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DB_NEXT, FIELD_ID_DATABASE_NEXT ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DB_SELECT, FIELD_ID_DATABASE_SELECT ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DB_NUMBER, FIELD_ID_DATABASE_NUMBER ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DB_DISPLAY, FIELD_ID_DATABASE_DISPLAY ),
    // workaround for #no-bug#: Database/DataBase
    ENUM_STRING_MAP_ENTRY( "DataBase", FIELD_ID_DATABASE_DISPLAY ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DB_NAME, FIELD_ID_DATABASE_NAME ),

    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_CREATE_AUTHOR, FIELD_ID_DOCINFO_CREATION_AUTHOR ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_CREATE_AUTHOR2, FIELD_ID_DOCINFO_CREATION_AUTHOR ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_CREATE_DATE_TIME, FIELD_ID_DOCINFO_CREATION_TIME),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_CREATE_DATE_TIME2, FIELD_ID_DOCINFO_CREATION_TIME),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_CHANGE_AUTHOR, FIELD_ID_DOCINFO_SAVE_AUTHOR ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_CHANGE_AUTHOR2, FIELD_ID_DOCINFO_SAVE_AUTHOR ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_CHANGE_DATE_TIME, FIELD_ID_DOCINFO_SAVE_TIME ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_CHANGE_DATE_TIME2, FIELD_ID_DOCINFO_SAVE_TIME ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_EDIT_TIME, FIELD_ID_DOCINFO_EDIT_DURATION ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_EDIT_TIME2, FIELD_ID_DOCINFO_EDIT_DURATION ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_DESCRIPTION, FIELD_ID_DOCINFO_DESCRIPTION ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_DESCRIPTION2, FIELD_ID_DOCINFO_DESCRIPTION ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_CUSTOM, FIELD_ID_DOCINFO_CUSTOM ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_CUSTOM2, FIELD_ID_DOCINFO_CUSTOM ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_PRINT_AUTHOR, FIELD_ID_DOCINFO_PRINT_AUTHOR ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_PRINT_AUTHOR2, FIELD_ID_DOCINFO_PRINT_AUTHOR ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_PRINT_DATE_TIME, FIELD_ID_DOCINFO_PRINT_TIME ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_PRINT_DATE_TIME2, FIELD_ID_DOCINFO_PRINT_TIME ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_KEY_WORDS, FIELD_ID_DOCINFO_KEYWORDS ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_KEY_WORDS2, FIELD_ID_DOCINFO_KEYWORDS ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_SUBJECT, FIELD_ID_DOCINFO_SUBJECT ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_SUBJECT2, FIELD_ID_DOCINFO_SUBJECT ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_TITLE, FIELD_ID_DOCINFO_TITLE ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_TITLE2, FIELD_ID_DOCINFO_TITLE ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_REVISION, FIELD_ID_DOCINFO_REVISION ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DOC_INFO_REVISION2, FIELD_ID_DOCINFO_REVISION ),

    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_CONDITIONAL_TEXT, FIELD_ID_CONDITIONAL_TEXT ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_HIDDEN_TEXT, FIELD_ID_HIDDEN_TEXT ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_HIDDEN_PARAGRAPH, FIELD_ID_HIDDEN_PARAGRAPH ),

    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_FILE_NAME, FIELD_ID_FILE_NAME ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_CHAPTER, FIELD_ID_CHAPTER ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_TEMPLATE_NAME, FIELD_ID_TEMPLATE_NAME ),

    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_PAGE_COUNT, FIELD_ID_COUNT_PAGES ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_PARAGRAPH_COUNT, FIELD_ID_COUNT_PARAGRAPHS ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_WORD_COUNT, FIELD_ID_COUNT_WORDS ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_CHARACTER_COUNT, FIELD_ID_COUNT_CHARACTERS ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_TABLE_COUNT, FIELD_ID_COUNT_TABLES ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_GRAPHIC_COUNT, FIELD_ID_COUNT_GRAPHICS ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_OBJECT_COUNT, FIELD_ID_COUNT_OBJECTS ),

    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_MACRO, FIELD_ID_MACRO ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_GET_REFERENCE, FIELD_ID_REF_REFERENCE ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DDE, FIELD_ID_DDE ),

    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_BIBLIOGRAPHY, FIELD_ID_BIBLIOGRAPHY ),

    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_SCRIPT, FIELD_ID_SCRIPT ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_ANNOTATION, FIELD_ID_ANNOTATION ),

    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_COMBINED_CHARACTERS, FIELD_ID_COMBINED_CHARACTERS ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_META, FIELD_ID_META ),

    // non-writer fields
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_SHEET_NAME, FIELD_ID_SHEET_NAME ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_URL, FIELD_ID_URL ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_MEASURE, FIELD_ID_MEASURE ),

    // deprecated fields
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_TABLE_FORMULA, FIELD_ID_TABLE_FORMULA ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DROP_DOWN, FIELD_ID_DROP_DOWN ),

    ENUM_STRING_MAP_END()
};



// property accessor helper functions
inline sal_Bool GetBoolProperty(const OUString&,
                                      const Reference<XPropertySet> &);
inline sal_Bool GetOptionalBoolProperty(const OUString&,
                                              const Reference<XPropertySet> &,
                                              const Reference<XPropertySetInfo> &,
                                              sal_Bool bDefault);
inline double GetDoubleProperty(const OUString&,
                                      const Reference<XPropertySet> &);
inline OUString const GetStringProperty(const OUString&,
                                        const Reference<XPropertySet> &);
inline sal_Int32 GetIntProperty(const OUString&,
                                      const Reference<XPropertySet> &);
inline sal_Int16 GetInt16Property(const OUString&,
                                        const Reference<XPropertySet> &);
inline sal_Int8 GetInt8Property(const OUString&,
                                      const Reference<XPropertySet> &);
inline DateTime const GetDateTimeProperty( const OUString& sPropName,
                                           const Reference<XPropertySet> & xPropSet);
inline Date const GetDateProperty( const OUString& sPropName,
                                   const Reference<XPropertySet> & xPropSet);
inline Sequence<OUString> const GetStringSequenceProperty(
                                   const OUString& sPropName,
                                   const Reference<XPropertySet> & xPropSet);



XMLTextFieldExport::XMLTextFieldExport( SvXMLExport& rExp,
                                        XMLPropertyState* pCombinedCharState)
    : rExport(rExp),
      pUsedMasters(NULL),
      sServicePrefix(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.textfield.")),
      sFieldMasterPrefix(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.FieldMaster.")),
      sPresentationServicePrefix(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.TextField.")),

    sPropertyAdjust(RTL_CONSTASCII_USTRINGPARAM("Adjust")),
    sPropertyAuthor(RTL_CONSTASCII_USTRINGPARAM("Author")),
    sPropertyChapterFormat(RTL_CONSTASCII_USTRINGPARAM("ChapterFormat")),
    sPropertyChapterNumberingLevel(RTL_CONSTASCII_USTRINGPARAM("ChapterNumberingLevel")),
    sPropertyCharStyleNames(RTL_CONSTASCII_USTRINGPARAM("CharStyleNames")),
    sPropertyCondition(RTL_CONSTASCII_USTRINGPARAM("Condition")),
    sPropertyContent(RTL_CONSTASCII_USTRINGPARAM("Content")),
    sPropertyDataBaseName(RTL_CONSTASCII_USTRINGPARAM("DataBaseName")),
    sPropertyDataBaseURL(RTL_CONSTASCII_USTRINGPARAM("DataBaseURL")),
    sPropertyDataColumnName(RTL_CONSTASCII_USTRINGPARAM("DataColumnName")),
    sPropertyDataCommandType(RTL_CONSTASCII_USTRINGPARAM("DataCommandType")),
    sPropertyDataTableName(RTL_CONSTASCII_USTRINGPARAM("DataTableName")),
    sPropertyDate(RTL_CONSTASCII_USTRINGPARAM("Date")),
    sPropertyDateTime(RTL_CONSTASCII_USTRINGPARAM("DateTime")),
    sPropertyDateTimeValue(RTL_CONSTASCII_USTRINGPARAM("DateTimeValue")),
    sPropertyDDECommandElement(RTL_CONSTASCII_USTRINGPARAM("DDECommandElement")),
    sPropertyDDECommandFile(RTL_CONSTASCII_USTRINGPARAM("DDECommandFile")),
    sPropertyDDECommandType(RTL_CONSTASCII_USTRINGPARAM("DDECommandType")),
    sPropertyDependentTextFields(RTL_CONSTASCII_USTRINGPARAM("DependentTextFields")),
    sPropertyFalseContent(RTL_CONSTASCII_USTRINGPARAM("FalseContent")),
    sPropertyFields(RTL_CONSTASCII_USTRINGPARAM("Fields")),
    sPropertyFieldSubType(RTL_CONSTASCII_USTRINGPARAM("UserDataType")),
    sPropertyFileFormat(RTL_CONSTASCII_USTRINGPARAM("FileFormat")),
    sPropertyFullName(RTL_CONSTASCII_USTRINGPARAM("FullName")),
    sPropertyHint(RTL_CONSTASCII_USTRINGPARAM("Hint")),
    sPropertyInitials("Initials"),
    sPropertyInstanceName(RTL_CONSTASCII_USTRINGPARAM("InstanceName")),
    sPropertyIsAutomaticUpdate(RTL_CONSTASCII_USTRINGPARAM("IsAutomaticUpdate")),
    sPropertyIsConditionTrue(RTL_CONSTASCII_USTRINGPARAM("IsConditionTrue")),
    sPropertyIsDataBaseFormat(RTL_CONSTASCII_USTRINGPARAM("DataBaseFormat")),
    sPropertyIsDate(RTL_CONSTASCII_USTRINGPARAM("IsDate")),
    sPropertyIsExpression(RTL_CONSTASCII_USTRINGPARAM("IsExpression")),
    sPropertyIsFixed(RTL_CONSTASCII_USTRINGPARAM("IsFixed")),
    sPropertyIsFixedLanguage(RTL_CONSTASCII_USTRINGPARAM("IsFixedLanguage")),
    sPropertyIsHidden(RTL_CONSTASCII_USTRINGPARAM("IsHidden")),
    sPropertyIsInput(RTL_CONSTASCII_USTRINGPARAM("Input")),
    sPropertyIsShowFormula(RTL_CONSTASCII_USTRINGPARAM("IsShowFormula")),
    sPropertyIsVisible(RTL_CONSTASCII_USTRINGPARAM("IsVisible")),
    sPropertyItems(RTL_CONSTASCII_USTRINGPARAM("Items")),
    sPropertyLevel(RTL_CONSTASCII_USTRINGPARAM("Level")),
    sPropertyMacro(RTL_CONSTASCII_USTRINGPARAM("Macro")),
    sPropertyMeasureKind(RTL_CONSTASCII_USTRINGPARAM("Kind")),
    sPropertyName(RTL_CONSTASCII_USTRINGPARAM("Name")),
    sPropertyNumberFormat(RTL_CONSTASCII_USTRINGPARAM("NumberFormat")),
    sPropertyNumberingSeparator(RTL_CONSTASCII_USTRINGPARAM("NumberingSeparator")),
    sPropertyNumberingType(RTL_CONSTASCII_USTRINGPARAM("NumberingType")),
    sPropertyOffset(RTL_CONSTASCII_USTRINGPARAM("Offset")),
    sPropertyOn(RTL_CONSTASCII_USTRINGPARAM("On")),
    sPropertyPlaceholder(RTL_CONSTASCII_USTRINGPARAM("PlaceHolder")),
    sPropertyPlaceholderType(RTL_CONSTASCII_USTRINGPARAM("PlaceHolderType")),
    sPropertyReferenceFieldPart(RTL_CONSTASCII_USTRINGPARAM("ReferenceFieldPart")),
    sPropertyReferenceFieldSource(RTL_CONSTASCII_USTRINGPARAM("ReferenceFieldSource")),
    sPropertyReferenceFieldType(RTL_CONSTASCII_USTRINGPARAM("ReferenceFieldType")),
    sPropertyRevision(RTL_CONSTASCII_USTRINGPARAM("Revision")),
    sPropertyScriptType(RTL_CONSTASCII_USTRINGPARAM("ScriptType")),
    sPropertySelectedItem(RTL_CONSTASCII_USTRINGPARAM("SelectedItem")),
    sPropertySequenceNumber(RTL_CONSTASCII_USTRINGPARAM("SequenceNumber")),
    sPropertySequenceValue(RTL_CONSTASCII_USTRINGPARAM("SequenceValue")),
    sPropertySetNumber(RTL_CONSTASCII_USTRINGPARAM("SetNumber")),
    sPropertySourceName(RTL_CONSTASCII_USTRINGPARAM("SourceName")),
    sPropertySubType(RTL_CONSTASCII_USTRINGPARAM("SubType")),
    sPropertyTargetFrame(RTL_CONSTASCII_USTRINGPARAM("TargetFrame")),
    sPropertyTrueContent(RTL_CONSTASCII_USTRINGPARAM("TrueContent")),
    sPropertyURL(RTL_CONSTASCII_USTRINGPARAM("URL")),
    sPropertyURLContent(RTL_CONSTASCII_USTRINGPARAM("URLContent")),
    sPropertyUserText(RTL_CONSTASCII_USTRINGPARAM("UserText")),
    sPropertyValue(RTL_CONSTASCII_USTRINGPARAM("Value")),
    sPropertyVariableName(RTL_CONSTASCII_USTRINGPARAM("VariableName")),
    sPropertyVariableSubType(RTL_CONSTASCII_USTRINGPARAM("VariableSubtype")),
      sPropertyHelp(RTL_CONSTASCII_USTRINGPARAM("Help")),
      sPropertyTooltip(RTL_CONSTASCII_USTRINGPARAM("Tooltip")),
      sPropertyTextRange(RTL_CONSTASCII_USTRINGPARAM("TextRange")),
      pCombinedCharactersPropertyState(pCombinedCharState)
{
    SetExportOnlyUsedFieldDeclarations();
}

XMLTextFieldExport::~XMLTextFieldExport()
{
    delete pCombinedCharactersPropertyState;
    delete pUsedMasters;
}

/// get the field ID (as in FieldIDEnum) from XTextField
enum FieldIdEnum XMLTextFieldExport::GetFieldID(
    const Reference<XTextField> & rTextField,
    const Reference<XPropertySet> & xPropSet)
{
    // get service names for rTextField (via XServiceInfo service)
    Reference<XServiceInfo> xService(rTextField, UNO_QUERY);
    const Sequence<OUString> aServices = xService->getSupportedServiceNames();
    const OUString* pNames = aServices.getConstArray();
    sal_Int32 nCount = aServices.getLength();

    OUString sFieldName;    // service name postfix of current field

    // search for TextField service name
    while( nCount-- )
    {
        if (pNames->matchIgnoreAsciiCase(sServicePrefix))
        {
            // TextField found => postfix is field type!
            sFieldName = pNames->copy(sServicePrefix.getLength());
            break;
        }

        ++pNames;
    }

    // if this is not a normal text field, check if its a presentation text field
    if( sFieldName.isEmpty() )
    {
        const OUString* pNames2 = aServices.getConstArray();
        sal_Int32 nCount2 = aServices.getLength();
        // search for TextField service name
        while( nCount2-- )
        {
            if( 0 == pNames2->compareTo(sPresentationServicePrefix, sPresentationServicePrefix.getLength()))
            {
                // TextField found => postfix is field type!
                sFieldName = pNames2->copy(sPresentationServicePrefix.getLength());
                break;
            }

            ++pNames2;
        }

        if( !sFieldName.isEmpty() )
        {
            if( sFieldName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "Header" ) ) == 0 )
            {
                return FIELD_ID_DRAW_HEADER;
            }
            else if( sFieldName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "Footer" ) ) == 0 )
            {
                return FIELD_ID_DRAW_FOOTER;
            }
            else if( sFieldName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "DateTime" ) ) == 0 )
            {
                return FIELD_ID_DRAW_DATE_TIME;
            }
        }
    }

    // map postfix of service name to field ID
    DBG_ASSERT(!sFieldName.isEmpty(), "no TextField service found!");
    return MapFieldName(sFieldName, xPropSet);
}

enum FieldIdEnum XMLTextFieldExport::MapFieldName(
    const OUString& sFieldName,             // field (master) name
    const Reference<XPropertySet> & xPropSet)   // for subtype
{
    // we'll proceed in 2 steps:
    // a) map service name to preliminary FIELD_ID
    // b) map those prelim. FIELD_IDs that correspond to several field types
    //    (in our (XML) world) to final FIELD IDs


    // a) find prelim. FIELD_ID via aFieldServiceMapping

    // check for non-empty service name
    DBG_ASSERT(!sFieldName.isEmpty(), "no valid service name!");
    enum FieldIdEnum nToken = FIELD_ID_UNKNOWN;
    if (!sFieldName.isEmpty())
    {
        // map name to prelim. ID
        sal_uInt16 nTmp;
        sal_Bool bRet = GetExport().GetMM100UnitConverter().convertEnum(
            nTmp, sFieldName, aFieldServiceNameMapping);

        // check return
        DBG_ASSERT(bRet, "Unknown field service name encountered!");
        if (! bRet)
        {
            nToken = FIELD_ID_UNKNOWN;
        }
        else
        {
            nToken = (enum FieldIdEnum)nTmp;
        }
    } else {
        // invalid service name
        nToken = FIELD_ID_UNKNOWN;
    }

    // b) map prelim. to final FIELD_IDs
    switch (nToken) {
        case FIELD_ID_VARIABLE_SET:
            if (GetBoolProperty(sPropertyIsInput, xPropSet))
            {
                nToken = FIELD_ID_VARIABLE_INPUT;
            }
            else
            {
                switch (GetIntProperty(sPropertySubType, xPropSet))
                {
                    case SetVariableType::STRING:   // text field
                    case SetVariableType::VAR:      // num field
                        nToken = FIELD_ID_VARIABLE_SET;
                        break;
                    case SetVariableType::SEQUENCE:
                        nToken = FIELD_ID_SEQUENCE;
                        break;
                    case SetVariableType::FORMULA:
                    default:
                        nToken = FIELD_ID_UNKNOWN;
                        break;
                }
            }
            break;

        case FIELD_ID_VARIABLE_GET:
            switch (GetIntProperty(sPropertySubType, xPropSet))
            {
                case SetVariableType::STRING:   // text field
                case SetVariableType::VAR:      // num field
                    nToken = FIELD_ID_VARIABLE_GET;
                    break;
                case SetVariableType::FORMULA:
                    nToken = FIELD_ID_EXPRESSION;
                    break;
                case SetVariableType::SEQUENCE:
                default:
                    nToken = FIELD_ID_UNKNOWN;
                    break;
            }
            break;

        case FIELD_ID_TIME:
            if (GetBoolProperty(sPropertyIsDate, xPropSet))
            {
                nToken = FIELD_ID_DATE;
            }
            break;

        case FIELD_ID_PAGENUMBER:
            // NumberingType not available in non-Writer apps
            if (xPropSet->getPropertySetInfo()->
                hasPropertyByName(sPropertyNumberingType))
            {
                if (NumberingType::CHAR_SPECIAL == GetIntProperty(
                                            sPropertyNumberingType, xPropSet))
                {
                    nToken = FIELD_ID_PAGESTRING;
                }
            }
            break;

        case FIELD_ID_DOCINFO_CREATION_TIME:
             if (GetBoolProperty(sPropertyIsDate, xPropSet))
            {
                nToken = FIELD_ID_DOCINFO_CREATION_DATE;
            }
            break;

        case FIELD_ID_DOCINFO_PRINT_TIME:
             if (GetBoolProperty(sPropertyIsDate, xPropSet))
            {
                nToken = FIELD_ID_DOCINFO_PRINT_DATE;
            }
            break;

        case FIELD_ID_DOCINFO_SAVE_TIME:
             if (GetBoolProperty(sPropertyIsDate, xPropSet))
            {
                nToken = FIELD_ID_DOCINFO_SAVE_DATE;
            }
            break;

        case FIELD_ID_REF_REFERENCE:
            switch (GetInt16Property(sPropertyReferenceFieldSource, xPropSet))
            {
                case ReferenceFieldSource::REFERENCE_MARK:
                    nToken = FIELD_ID_REF_REFERENCE;
                    break;
                case ReferenceFieldSource::SEQUENCE_FIELD:
                    nToken = FIELD_ID_REF_SEQUENCE;
                    break;
                case ReferenceFieldSource::BOOKMARK:
                    nToken = FIELD_ID_REF_BOOKMARK;
                    break;
                case ReferenceFieldSource::FOOTNOTE:
                    nToken = FIELD_ID_REF_FOOTNOTE;
                    break;
                case ReferenceFieldSource::ENDNOTE:
                    nToken = FIELD_ID_REF_ENDNOTE;
                    break;
                default:
                    nToken = FIELD_ID_UNKNOWN;
                    break;
            }
            break;

        case FIELD_ID_COMBINED_CHARACTERS:
        case FIELD_ID_SCRIPT:
        case FIELD_ID_ANNOTATION:
        case FIELD_ID_BIBLIOGRAPHY:
        case FIELD_ID_DDE:
        case FIELD_ID_MACRO:
        case FIELD_ID_REFPAGE_SET:
        case FIELD_ID_REFPAGE_GET:
        case FIELD_ID_COUNT_PAGES:
        case FIELD_ID_COUNT_PARAGRAPHS:
        case FIELD_ID_COUNT_WORDS:
        case FIELD_ID_COUNT_CHARACTERS:
        case FIELD_ID_COUNT_TABLES:
        case FIELD_ID_COUNT_GRAPHICS:
        case FIELD_ID_COUNT_OBJECTS:
        case FIELD_ID_CONDITIONAL_TEXT:
        case FIELD_ID_HIDDEN_TEXT:
        case FIELD_ID_HIDDEN_PARAGRAPH:
        case FIELD_ID_DOCINFO_CREATION_AUTHOR:
        case FIELD_ID_DOCINFO_DESCRIPTION:
        case FIELD_ID_DOCINFO_CUSTOM:
        case FIELD_ID_DOCINFO_PRINT_AUTHOR:
        case FIELD_ID_DOCINFO_TITLE:
        case FIELD_ID_DOCINFO_SUBJECT:
        case FIELD_ID_DOCINFO_KEYWORDS:
        case FIELD_ID_DOCINFO_REVISION:
        case FIELD_ID_DOCINFO_EDIT_DURATION:
        case FIELD_ID_DOCINFO_SAVE_AUTHOR:
        case FIELD_ID_TEXT_INPUT:
        case FIELD_ID_USER_INPUT:
        case FIELD_ID_AUTHOR:
        case FIELD_ID_SENDER:
        case FIELD_ID_PLACEHOLDER:
        case FIELD_ID_USER_GET:
        case FIELD_ID_DATABASE_NEXT:
        case FIELD_ID_DATABASE_SELECT:
        case FIELD_ID_DATABASE_DISPLAY:
        case FIELD_ID_DATABASE_NAME:
        case FIELD_ID_DATABASE_NUMBER:
        case FIELD_ID_TEMPLATE_NAME:
        case FIELD_ID_CHAPTER:
        case FIELD_ID_FILE_NAME:
        case FIELD_ID_META:
        case FIELD_ID_SHEET_NAME:
        case FIELD_ID_MEASURE:
        case FIELD_ID_URL:
        case FIELD_ID_TABLE_FORMULA:
        case FIELD_ID_DROP_DOWN:
            ; // these field IDs are final
            break;

        default:
            nToken = FIELD_ID_UNKNOWN;
    }

    // ... and return final FIELD_ID
    return nToken;
}

// is string or numeric field?
sal_Bool XMLTextFieldExport::IsStringField(
    FieldIdEnum nFieldType,
    const Reference<XPropertySet> & xPropSet)
{
    switch (nFieldType) {

    case FIELD_ID_VARIABLE_GET:
    case FIELD_ID_VARIABLE_SET:
    case FIELD_ID_VARIABLE_INPUT:
    {
        // depends on field sub type
        return ( GetIntProperty(sPropertySubType, xPropSet) ==
                 SetVariableType::STRING                    );
    }

    case FIELD_ID_USER_GET:
    case FIELD_ID_USER_INPUT:
    {
        Reference<XTextField> xTextField(xPropSet, UNO_QUERY);
        DBG_ASSERT(xTextField.is(), "field is no XTextField!");
        sal_Bool bRet = GetBoolProperty(sPropertyIsExpression,
                                        GetMasterPropertySet(xTextField));
        return !bRet;
    }

    case FIELD_ID_META:
        return 0 > GetIntProperty(sPropertyNumberFormat, xPropSet);

    case FIELD_ID_DATABASE_DISPLAY:
        // TODO: depends on... ???
        // workaround #no-bug#: no data type
        return 5100 == GetIntProperty(sPropertyNumberFormat, xPropSet);

    case FIELD_ID_TABLE_FORMULA:
        // legacy field: always a number field (because it always has
        // a number format)
        return sal_False;

    case FIELD_ID_COUNT_PAGES:
    case FIELD_ID_COUNT_PARAGRAPHS:
    case FIELD_ID_COUNT_WORDS:
    case FIELD_ID_COUNT_CHARACTERS:
    case FIELD_ID_COUNT_TABLES:
    case FIELD_ID_COUNT_GRAPHICS:
    case FIELD_ID_COUNT_OBJECTS:
    case FIELD_ID_DOCINFO_SAVE_TIME:
    case FIELD_ID_DOCINFO_SAVE_DATE:
    case FIELD_ID_DOCINFO_CREATION_DATE:
    case FIELD_ID_DOCINFO_CREATION_TIME:
    case FIELD_ID_DOCINFO_PRINT_TIME:
    case FIELD_ID_DOCINFO_PRINT_DATE:
    case FIELD_ID_DOCINFO_EDIT_DURATION:
    case FIELD_ID_DOCINFO_REVISION:
    case FIELD_ID_DATABASE_NUMBER:
    case FIELD_ID_EXPRESSION:
    case FIELD_ID_SEQUENCE:
    case FIELD_ID_DATE:
    case FIELD_ID_TIME:
    case FIELD_ID_PAGENUMBER:
    case FIELD_ID_REFPAGE_SET:
    case FIELD_ID_REFPAGE_GET:
    case FIELD_ID_DOCINFO_CUSTOM:
        // always number
        return sal_False;

    case FIELD_ID_COMBINED_CHARACTERS:
    case FIELD_ID_BIBLIOGRAPHY:
    case FIELD_ID_DDE:
    case FIELD_ID_REF_REFERENCE:
    case FIELD_ID_REF_SEQUENCE:
    case FIELD_ID_REF_BOOKMARK:
    case FIELD_ID_REF_FOOTNOTE:
    case FIELD_ID_REF_ENDNOTE:
    case FIELD_ID_MACRO:
    case FIELD_ID_TEMPLATE_NAME:
    case FIELD_ID_CHAPTER:
    case FIELD_ID_FILE_NAME:
    case FIELD_ID_CONDITIONAL_TEXT:
    case FIELD_ID_HIDDEN_TEXT:
    case FIELD_ID_HIDDEN_PARAGRAPH:
    case FIELD_ID_DOCINFO_CREATION_AUTHOR:
    case FIELD_ID_DOCINFO_DESCRIPTION:
    case FIELD_ID_DOCINFO_PRINT_AUTHOR:
    case FIELD_ID_DOCINFO_TITLE:
    case FIELD_ID_DOCINFO_SUBJECT:
    case FIELD_ID_DOCINFO_KEYWORDS:
    case FIELD_ID_DOCINFO_SAVE_AUTHOR:
    case FIELD_ID_DATABASE_NAME:
    case FIELD_ID_TEXT_INPUT:
    case FIELD_ID_SENDER:
    case FIELD_ID_AUTHOR:
    case FIELD_ID_PAGESTRING:
    case FIELD_ID_SHEET_NAME:
    case FIELD_ID_MEASURE:
    case FIELD_ID_URL:
    case FIELD_ID_DROP_DOWN:
        // always string:
        return sal_True;

    case FIELD_ID_SCRIPT:
    case FIELD_ID_ANNOTATION:
       case FIELD_ID_DATABASE_NEXT:
    case FIELD_ID_DATABASE_SELECT:
    case FIELD_ID_VARIABLE_DECL:
    case FIELD_ID_USER_DECL:
    case FIELD_ID_SEQUENCE_DECL:
    case FIELD_ID_PLACEHOLDER:
    case FIELD_ID_UNKNOWN:
    case FIELD_ID_DRAW_HEADER:
    case FIELD_ID_DRAW_FOOTER:
    case FIELD_ID_DRAW_DATE_TIME:
    default:
        OSL_FAIL("unkown field type/field has no content");
        return sal_True; // invalid info; string in case of doubt
    }
}

/// export the styles needed by the given field. Called on first pass
/// through document
void XMLTextFieldExport::ExportFieldAutoStyle(
    const Reference<XTextField> & rTextField, const sal_Bool bProgress,
    const sal_Bool bRecursive )
{
    // get property set
    Reference<XPropertySet> xPropSet(rTextField, UNO_QUERY);

    // add field master to list of used field masters (if desired)
    if (NULL != pUsedMasters)
    {
        Reference<XDependentTextField> xDepField(rTextField, UNO_QUERY);
        if (xDepField.is())
        {
            Reference<XText> xOurText = rTextField->getAnchor()->getText();

            map<Reference<XText>, set<OUString> >::iterator aMapIter =
                pUsedMasters->find(xOurText);

            // insert a list for our XText (if necessary)
            if (aMapIter == pUsedMasters->end())
            {
                set<OUString> aSet;
                (*pUsedMasters)[xOurText] = aSet;
                aMapIter = pUsedMasters->find(xOurText);
            }

            // insert this text field master
            OUString sFieldMasterName = GetStringProperty(
                sPropertyInstanceName, xDepField->getTextFieldMaster());
            if (!sFieldMasterName.isEmpty())
                aMapIter->second.insert( sFieldMasterName );
        }
        // else: no dependent field -> no master -> ignore
    }

    // get Field ID
    FieldIdEnum nToken = GetFieldID(rTextField, xPropSet);

    // export the character style for all fields
    // with one exception: combined character fields export their own
    //                     text style below
    Reference <XPropertySet> xRangePropSet(rTextField->getAnchor(), UNO_QUERY);
    if (FIELD_ID_COMBINED_CHARACTERS != nToken)
    {
        GetExport().GetTextParagraphExport()->Add(
            XML_STYLE_FAMILY_TEXT_TEXT, xRangePropSet);
    }

    // process special styles for each field (e.g. data styles)
    switch (nToken) {

    case FIELD_ID_DATABASE_DISPLAY:
    {
        sal_Int32 nFormat = GetIntProperty(sPropertyNumberFormat, xPropSet);
        // workaround: #no-bug#; see IsStringField(...)
        if ( (5100 != nFormat) &&
             !GetBoolProperty(sPropertyIsDataBaseFormat, xPropSet) )
        {
                GetExport().addDataStyle(nFormat);
        }
        break;
    }

    case FIELD_ID_DATE:
    case FIELD_ID_TIME:
        {
            // date and time fields are always number fields, but the
            // NumberFormat property is optional (e.g. Calc doesn't
            // support it)
            Reference<XPropertySetInfo> xPropSetInfo(
                xPropSet->getPropertySetInfo() );
            if ( xPropSetInfo->hasPropertyByName( sPropertyNumberFormat ) )
            {
                sal_Int32 nFormat =
                    GetIntProperty(sPropertyNumberFormat, xPropSet);

                // nFormat may be -1 for numeric fields that display their
                //  variable name. (Maybe this should be a field type, then?)
                if (nFormat != -1)
                {
                    if( ! GetOptionalBoolProperty(
                            sPropertyIsFixedLanguage,
                            xPropSet, xPropSetInfo, sal_False ) )
                    {
                        nFormat =
                            GetExport().dataStyleForceSystemLanguage(nFormat);
                    }

                    GetExport().addDataStyle( nFormat,
                                              nToken == FIELD_ID_TIME );
                }
            }
        }
        break;

    case FIELD_ID_META:
        // recurse into content (does not export element, so can be done first)
        if (bRecursive)
        {
            ExportMetaField(xPropSet, true, bProgress);
        }
        // fall-through: for the meta-field itself!
    case FIELD_ID_DOCINFO_PRINT_TIME:
    case FIELD_ID_DOCINFO_PRINT_DATE:
    case FIELD_ID_DOCINFO_CREATION_DATE:
    case FIELD_ID_DOCINFO_CREATION_TIME:
    case FIELD_ID_DOCINFO_SAVE_TIME:
    case FIELD_ID_DOCINFO_SAVE_DATE:
    case FIELD_ID_DOCINFO_EDIT_DURATION:
    case FIELD_ID_VARIABLE_SET:
    case FIELD_ID_VARIABLE_GET:
    case FIELD_ID_VARIABLE_INPUT:
    case FIELD_ID_USER_GET:
    case FIELD_ID_EXPRESSION:
    case FIELD_ID_TABLE_FORMULA:
    case FIELD_ID_DOCINFO_CUSTOM:
        // register number format, if this is a numeric field
        if (! IsStringField(nToken, xPropSet)) {

            sal_Int32 nFormat =
                GetIntProperty(sPropertyNumberFormat, xPropSet);

            // nFormat may be -1 for numeric fields that display their
            //  variable name. (Maybe this should be a field type, then?)
            if (nFormat != -1)
            {
                // handle formats for fixed language fields
                // for all these fields (except table formula)
                if( ( nToken != FIELD_ID_TABLE_FORMULA ) &&
                    ! GetOptionalBoolProperty(
                          sPropertyIsFixedLanguage,
                          xPropSet, xPropSet->getPropertySetInfo(),
                          sal_False ) )
                {
                    nFormat =
                        GetExport().dataStyleForceSystemLanguage(nFormat);
                }

                GetExport().addDataStyle(nFormat);
            }
        }
        break;

    case FIELD_ID_COMBINED_CHARACTERS:
    {
        // export text style with the addition of the combined characters
        DBG_ASSERT(NULL != pCombinedCharactersPropertyState,
                   "need proper PropertyState for combined characters");
        const XMLPropertyState *aStates[] = { pCombinedCharactersPropertyState, 0 };
        GetExport().GetTextParagraphExport()->Add(
            XML_STYLE_FAMILY_TEXT_TEXT, xRangePropSet,
            aStates);
        break;
    }

    case FIELD_ID_SCRIPT:
    case FIELD_ID_ANNOTATION:
    case FIELD_ID_BIBLIOGRAPHY:
    case FIELD_ID_DDE:
    case FIELD_ID_REF_REFERENCE:
    case FIELD_ID_REF_SEQUENCE:
    case FIELD_ID_REF_BOOKMARK:
    case FIELD_ID_REF_FOOTNOTE:
    case FIELD_ID_REF_ENDNOTE:
    case FIELD_ID_MACRO:
    case FIELD_ID_REFPAGE_SET:
    case FIELD_ID_REFPAGE_GET:
    case FIELD_ID_COUNT_PAGES:
    case FIELD_ID_COUNT_PARAGRAPHS:
    case FIELD_ID_COUNT_WORDS:
    case FIELD_ID_COUNT_CHARACTERS:
    case FIELD_ID_COUNT_TABLES:
    case FIELD_ID_COUNT_GRAPHICS:
    case FIELD_ID_COUNT_OBJECTS:
    case FIELD_ID_CONDITIONAL_TEXT:
    case FIELD_ID_HIDDEN_TEXT:
    case FIELD_ID_HIDDEN_PARAGRAPH:
    case FIELD_ID_DOCINFO_CREATION_AUTHOR:
    case FIELD_ID_DOCINFO_DESCRIPTION:
    case FIELD_ID_DOCINFO_PRINT_AUTHOR:
    case FIELD_ID_DOCINFO_TITLE:
    case FIELD_ID_DOCINFO_SUBJECT:
    case FIELD_ID_DOCINFO_KEYWORDS:
    case FIELD_ID_DOCINFO_REVISION:
    case FIELD_ID_DOCINFO_SAVE_AUTHOR:
    case FIELD_ID_SEQUENCE:
    case FIELD_ID_PAGENUMBER:
    case FIELD_ID_PAGESTRING:
    case FIELD_ID_AUTHOR:
    case FIELD_ID_SENDER:
    case FIELD_ID_PLACEHOLDER:
    case FIELD_ID_USER_INPUT:
    case FIELD_ID_TEXT_INPUT:
    case FIELD_ID_DATABASE_NEXT:
    case FIELD_ID_DATABASE_SELECT:
    case FIELD_ID_DATABASE_NAME:
    case FIELD_ID_DATABASE_NUMBER:
    case FIELD_ID_TEMPLATE_NAME:
    case FIELD_ID_CHAPTER:
    case FIELD_ID_FILE_NAME:
    case FIELD_ID_SHEET_NAME:
    case FIELD_ID_MEASURE:
    case FIELD_ID_URL:
    case FIELD_ID_DROP_DOWN:
    case FIELD_ID_DRAW_DATE_TIME:
    case FIELD_ID_DRAW_FOOTER:
    case FIELD_ID_DRAW_HEADER:
        ; // no formats for these fields!
        break;

    case FIELD_ID_UNKNOWN:
    default:
        OSL_FAIL("unkown field type!");
        // ignore -> no format for unkowns
        break;
    }
}

/// export the given field to XML. Called on second pass through document
void XMLTextFieldExport::ExportField(
    const Reference<XTextField> & rTextField, sal_Bool bProgress )
{
    // get property set
    Reference<XPropertySet> xPropSet(rTextField, UNO_QUERY);

    // get property set of range (for the attributes)
    Reference <XPropertySet> xRangePropSet(rTextField->getAnchor(), UNO_QUERY);

    // get Field ID
    enum FieldIdEnum nToken = GetFieldID(rTextField, xPropSet);

    // special treatment for combined characters field, because it is
    // exported as a style
    const XMLPropertyState* aStates[] = { pCombinedCharactersPropertyState, 0 };
    const XMLPropertyState **pStates =
                FIELD_ID_COMBINED_CHARACTERS == nToken
                    ? aStates
                    : 0;

    // find out whether we need to set the style or hyperlink
    sal_Bool bHasHyperlink;
    sal_Bool bIsUICharStyle;
    sal_Bool bHasAutoStyle;
    OUString sStyle = GetExport().GetTextParagraphExport()->
        FindTextStyleAndHyperlink( xRangePropSet, bHasHyperlink, bIsUICharStyle,
                                   bHasAutoStyle, pStates );
    sal_Bool bHasStyle = !sStyle.isEmpty();

    // export hyperlink (if we have one)
    Reference < XPropertySetInfo > xRangePropSetInfo;
    if( bHasHyperlink )
    {
        Reference<XPropertyState> xRangePropState( xRangePropSet, UNO_QUERY );
        xRangePropSetInfo = xRangePropSet->getPropertySetInfo();
        bHasHyperlink =
            GetExport().GetTextParagraphExport()->addHyperlinkAttributes(
                xRangePropSet, xRangePropState,
                xRangePropSetInfo );
    }
    SvXMLElementExport aHyperlink( GetExport(), bHasHyperlink,
                                   XML_NAMESPACE_TEXT, XML_A,
                                   sal_False, sal_False );

    if( bHasHyperlink )
    {
        // export events (if supported)
        OUString sHyperLinkEvents(RTL_CONSTASCII_USTRINGPARAM(
            "HyperLinkEvents"));
        if (xRangePropSetInfo->hasPropertyByName(sHyperLinkEvents))
        {
            Any aAny = xRangePropSet->getPropertyValue(sHyperLinkEvents);
            Reference<XNameReplace> xName;
            aAny >>= xName;
            GetExport().GetEventExport().Export(xName, sal_False);
        }
    }

    {
        XMLTextCharStyleNamesElementExport aCharStylesExport(
            GetExport(), bIsUICharStyle &&
                         GetExport().GetTextParagraphExport()
                             ->GetCharStyleNamesPropInfoCache().hasProperty(
                                        xRangePropSet, xRangePropSetInfo ), bHasAutoStyle,
            xRangePropSet, sPropertyCharStyleNames );

        // export span with style (if necessary)
        // (except for combined characters field)
        if( bHasStyle )
        {
            // export <text:span> element
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                            GetExport().EncodeStyleName( sStyle ) );
        }
        SvXMLElementExport aSpan( GetExport(), bHasStyle,
                                  XML_NAMESPACE_TEXT, XML_SPAN,
                                  sal_False, sal_False);

        // finally, export the field itself
        ExportFieldHelper( rTextField, xPropSet, xRangePropSet, nToken,
            bProgress );
    }
}

/// export the given field to XML. Called on second pass through document
void XMLTextFieldExport::ExportFieldHelper(
    const Reference<XTextField> & rTextField,
    const Reference<XPropertySet> & rPropSet,
    const Reference<XPropertySet> &,
    enum FieldIdEnum nToken,
    sal_Bool bProgress )
{
    // get property set info (because some attributes are not support
    // in all implementations)
    Reference<XPropertySetInfo> xPropSetInfo(rPropSet->getPropertySetInfo());

    OUString sPresentation = rTextField->getPresentation(sal_False);

    // process each field type
    switch (nToken) {
    case FIELD_ID_AUTHOR:
        // author field: fixed, field (sub-)type
        ProcessBoolean(XML_FIXED,
                       GetBoolProperty(sPropertyIsFixed, rPropSet), sal_True);
        ExportElement(MapAuthorFieldName(rPropSet), sPresentation);
        break;

    case FIELD_ID_SENDER:
        // sender field: fixed, field (sub-)type
        ProcessBoolean(XML_FIXED,
                       GetBoolProperty(sPropertyIsFixed, rPropSet), sal_True);
        ExportElement(MapSenderFieldName(rPropSet), sPresentation);
        break;

    case FIELD_ID_PLACEHOLDER:
        // placeholder field: type, name, description
        ProcessString(XML_PLACEHOLDER_TYPE,
                      MapPlaceholderType(
                        GetInt16Property(sPropertyPlaceholderType, rPropSet)));
        ProcessString(XML_DESCRIPTION,
                      GetStringProperty(sPropertyHint,rPropSet), sal_True);
        ExportElement(XML_PLACEHOLDER, sPresentation);
        break;

    case FIELD_ID_VARIABLE_SET:
    {
        // variable set field: name, visible, format&value
        ProcessString(XML_NAME,
                      GetStringProperty(sPropertyVariableName, rPropSet));
        ProcessDisplay(GetBoolProperty(sPropertyIsVisible, rPropSet),
                       sal_False);
        ProcessString(XML_FORMULA, XML_NAMESPACE_OOOW,
                      GetStringProperty(sPropertyContent, rPropSet),
                      sPresentation);
        ProcessValueAndType(IsStringField(nToken, rPropSet),
                            GetIntProperty(sPropertyNumberFormat, rPropSet),
                            GetStringProperty(sPropertyContent, rPropSet),
                            sPresentation,
                            GetDoubleProperty(sPropertyValue, rPropSet),
                            sal_True, sal_True, sal_True,
                            ! GetOptionalBoolProperty(
                                 sPropertyIsFixedLanguage,
                                 rPropSet, xPropSetInfo, sal_False ) );
        ExportElement(XML_VARIABLE_SET, sPresentation);
        break;
    }
    case FIELD_ID_VARIABLE_GET:
    {
        // variable get field: name, format&value
        ProcessString(XML_NAME,
                      GetStringProperty(sPropertyContent, rPropSet));
        sal_Bool bCmd = GetBoolProperty(sPropertyIsShowFormula, rPropSet);
        ProcessDisplay(sal_True, bCmd);
        // #i81766# for older versions export of the value-type
        sal_Bool bExportValueType = !bCmd && ( GetExport().getExportFlags() & EXPORT_SAVEBACKWARDCOMPATIBLE );
        // show style, unless name will be shown
        ProcessValueAndType(IsStringField(nToken, rPropSet),
                            GetIntProperty(sPropertyNumberFormat, rPropSet),
                            sEmpty, sEmpty, 0.0, // values not used
                            sal_False,
                            bExportValueType,
                            !bCmd,
                            ! GetOptionalBoolProperty(
                                 sPropertyIsFixedLanguage,
                                 rPropSet, xPropSetInfo, sal_False ) );
        ExportElement(XML_VARIABLE_GET, sPresentation);
        break;
    }
    case FIELD_ID_VARIABLE_INPUT:
        // variable input field: name, description, format&value
        ProcessString(XML_NAME,
                      GetStringProperty(sPropertyVariableName, rPropSet));
        ProcessString(XML_DESCRIPTION,
                      GetStringProperty(sPropertyHint , rPropSet));
        ProcessDisplay(GetBoolProperty(sPropertyIsVisible, rPropSet),
                       sal_False);
        ProcessString(XML_FORMULA, XML_NAMESPACE_OOOW,
                      GetStringProperty(sPropertyContent, rPropSet),
                      sPresentation);
        ProcessValueAndType(IsStringField(nToken, rPropSet),
                            GetIntProperty(sPropertyNumberFormat, rPropSet),
                            GetStringProperty(sPropertyContent, rPropSet),
                            sPresentation,
                            GetDoubleProperty(sPropertyValue, rPropSet),
                            sal_True, sal_True, sal_True,
                            ! GetOptionalBoolProperty(
                                 sPropertyIsFixedLanguage,
                                 rPropSet, xPropSetInfo, sal_False ) );
        ExportElement(XML_VARIABLE_INPUT, sPresentation);
        break;

    case FIELD_ID_USER_GET:
        // user field: name, hidden, style
    {
        sal_Bool bCmd = GetBoolProperty(sPropertyIsShowFormula, rPropSet);
        ProcessDisplay(GetBoolProperty(sPropertyIsVisible, rPropSet),
                       bCmd);
        ProcessValueAndType(IsStringField(nToken, rPropSet),
                            GetIntProperty(sPropertyNumberFormat, rPropSet),
                            sEmpty, sEmpty, 0.0, // values not used
                            sal_False, sal_False, !bCmd,
                            ! GetOptionalBoolProperty(
                                 sPropertyIsFixedLanguage,
                                 rPropSet, xPropSetInfo, sal_False ) );

        // name from FieldMaster
        ProcessString(XML_NAME,
                      GetStringProperty(sPropertyName,
                                        GetMasterPropertySet(rTextField)));
        ExportElement(XML_USER_FIELD_GET, sPresentation);
        break;
    }

    case FIELD_ID_USER_INPUT:
        // user input field: name (from FieldMaster), description
//      ProcessString(XML_NAME,
//                    GetStringProperty(sPropertyName,
//                                      GetMasterPropertySet(rTextField)));
        ProcessString(XML_NAME,
                      GetStringProperty(sPropertyContent, rPropSet));
        ProcessString(XML_DESCRIPTION,
                      GetStringProperty(sPropertyHint, rPropSet));
        ExportElement(XML_USER_FIELD_INPUT, sPresentation);
        break;

    case FIELD_ID_SEQUENCE:
    {
        // sequence field: name, formula, seq-format
        OUString sName = GetStringProperty(sPropertyVariableName, rPropSet);
        // TODO: use reference name only if actually beeing referenced.
        ProcessString(XML_REF_NAME,
                      MakeSequenceRefName(
                          GetInt16Property(sPropertySequenceValue, rPropSet),
                          sName));
        ProcessString(XML_NAME, sName);
        ProcessString(XML_FORMULA,  XML_NAMESPACE_OOOW,
                      GetStringProperty(sPropertyContent, rPropSet),
                      sPresentation);
        ProcessNumberingType(GetInt16Property(sPropertyNumberingType,
                                              rPropSet));
        ExportElement(XML_SEQUENCE, sPresentation);
        break;
    }

    case FIELD_ID_EXPRESSION:
    {
        // formula field: formula, format&value
        sal_Bool bCmd = GetBoolProperty(sPropertyIsShowFormula, rPropSet);
        ProcessString(XML_FORMULA,  XML_NAMESPACE_OOOW,
                      GetStringProperty(sPropertyContent, rPropSet),
                      sPresentation);
        ProcessDisplay(sal_True, bCmd);
        ProcessValueAndType(IsStringField(nToken, rPropSet),
                            GetIntProperty(sPropertyNumberFormat, rPropSet),
                            GetStringProperty(sPropertyContent, rPropSet),
                            sPresentation,
                            GetDoubleProperty(sPropertyValue, rPropSet),
                            !bCmd, !bCmd, !bCmd,
                            ! GetOptionalBoolProperty(
                                 sPropertyIsFixedLanguage,
                                 rPropSet, xPropSetInfo, sal_False ) );
        ExportElement(XML_EXPRESSION, sPresentation);
        break;
    }

    case FIELD_ID_TEXT_INPUT:
        // text input field: description and string-value
        ProcessString(XML_DESCRIPTION,
                      GetStringProperty(sPropertyHint, rPropSet));
        ProcessString(XML_HELP,
                      GetStringProperty(sPropertyHelp, rPropSet), true);
        ProcessString(XML_HINT,
                      GetStringProperty(sPropertyTooltip, rPropSet), true);
        ExportElement(XML_TEXT_INPUT, sPresentation);
        break;

    case FIELD_ID_TIME:
        // all properties (except IsDate) are optional!
        if (xPropSetInfo->hasPropertyByName(sPropertyNumberFormat))
        {
            ProcessValueAndType(sal_False,
                                GetIntProperty(sPropertyNumberFormat,rPropSet),
                                sEmpty, sEmpty, 0.0, // not used
                                sal_False, sal_False, sal_True,
                                ! GetOptionalBoolProperty(
                                    sPropertyIsFixedLanguage,
                                    rPropSet, xPropSetInfo, sal_False ),
                                sal_True);
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyDateTimeValue))
        {
            // no value -> current time
            ProcessDateTime(XML_TIME_VALUE,
                            GetDateTimeProperty(sPropertyDateTimeValue,
                                                rPropSet),
                            sal_False );
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyDateTime))
        {
            // no value -> current time
            ProcessDateTime(XML_TIME_VALUE,
                            GetDateTimeProperty(sPropertyDateTime,rPropSet),
                            sal_False );
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyIsFixed))
        {
            ProcessBoolean(XML_FIXED,
                           GetBoolProperty(sPropertyIsFixed, rPropSet),
                           sal_False);
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyAdjust))
        {
            // adjust value given as integer in minutes
            ProcessDateTime(XML_TIME_ADJUST,
                            GetIntProperty(sPropertyAdjust, rPropSet),
                            sal_False, sal_True, sal_True);
        }
        ExportElement(XML_TIME, sPresentation);
        break;

    case FIELD_ID_DATE:
        // all properties (except IsDate) are optional!
        if (xPropSetInfo->hasPropertyByName(sPropertyNumberFormat))
        {
            ProcessValueAndType(sal_False,
                                GetIntProperty(sPropertyNumberFormat,rPropSet),
                                sEmpty, sEmpty, 0.0, // not used
                                sal_False, sal_False, sal_True,
                                ! GetOptionalBoolProperty(
                                    sPropertyIsFixedLanguage,
                                    rPropSet, xPropSetInfo, sal_False ) );
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyDateTimeValue))
        {
            // no value -> current date
            ProcessDateTime(XML_DATE_VALUE,
                            GetDateTimeProperty(sPropertyDateTimeValue,
                                                rPropSet),
                            // #96457#: date fields should also save time
                            sal_False);
        }
        // TODO: remove double-handling after SRC614
        else if (xPropSetInfo->hasPropertyByName(sPropertyDateTime))
        {
            ProcessDateTime(XML_DATE_VALUE,
                            GetDateTimeProperty(sPropertyDateTime,rPropSet),
                            // #96457#: date fields should also save time
                            sal_False);
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyIsFixed))
        {
            ProcessBoolean(XML_FIXED,
                           GetBoolProperty(sPropertyIsFixed, rPropSet),
                           sal_False);
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyAdjust))
        {
            // adjust value given as number of days
            ProcessDateTime(XML_DATE_ADJUST,
                            GetIntProperty(sPropertyAdjust, rPropSet),
                            sal_True, sal_True, sal_True);
        }
        ExportElement(XML_DATE, sPresentation);
        break;

    case FIELD_ID_PAGENUMBER:
        // all properties are optional
        if (xPropSetInfo->hasPropertyByName(sPropertyNumberingType))
        {
            ProcessNumberingType(GetInt16Property(sPropertyNumberingType,
                                                  rPropSet));
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyOffset))
        {
            sal_Int32 nAdjust = GetIntProperty(sPropertyOffset, rPropSet);

            if (xPropSetInfo->hasPropertyByName(sPropertySubType))
            {
                // property SubType used in MapPageNumebrName
                ProcessString(XML_SELECT_PAGE,
                              MapPageNumberName(rPropSet, nAdjust));
            }
            ProcessIntegerDef(XML_PAGE_ADJUST, nAdjust, 0);
        }
        ExportElement(XML_PAGE_NUMBER, sPresentation);
        break;

    case FIELD_ID_PAGESTRING:
    {
        ProcessString(XML_STRING_VALUE,
                      GetStringProperty(sPropertyUserText, rPropSet),
                      sPresentation);
        sal_Int32 nDummy = 0; // MapPageNumberName need int
        ProcessString(XML_SELECT_PAGE, MapPageNumberName(rPropSet, nDummy));
        if( 0 == ( GetExport().getExportFlags() & EXPORT_SAVEBACKWARDCOMPATIBLE ) )
            ExportElement(XML_PAGE_CONTINUATION, sPresentation);
        else
            ExportElement(XML_PAGE_CONTINUATION_STRING, sPresentation);
        break;
    }

    case FIELD_ID_DATABASE_NAME:
        ProcessString(XML_TABLE_NAME,
                      GetStringProperty(sPropertyDataTableName, rPropSet));
        ProcessCommandType(GetIntProperty(sPropertyDataCommandType, rPropSet));
        ProcessDisplay(GetBoolProperty(sPropertyIsVisible, rPropSet),
                       sal_False);
        ExportDataBaseElement(XML_DATABASE_NAME, sPresentation,
                              rPropSet, xPropSetInfo);
        break;

    case FIELD_ID_DATABASE_NUMBER:
        ProcessString(XML_TABLE_NAME,
                      GetStringProperty(sPropertyDataTableName, rPropSet));
        ProcessCommandType(GetIntProperty(sPropertyDataCommandType, rPropSet));
        ProcessNumberingType(
            GetInt16Property(sPropertyNumberingType,rPropSet));
        ProcessInteger(XML_VALUE,
                       GetIntProperty(sPropertySetNumber, rPropSet));
        ProcessDisplay(GetBoolProperty(sPropertyIsVisible, rPropSet),
                       sal_False);
        ExportDataBaseElement(XML_DATABASE_ROW_NUMBER, sPresentation,
                              rPropSet, xPropSetInfo);
        break;

    case FIELD_ID_DATABASE_NEXT:
        ProcessString(XML_TABLE_NAME,
                      GetStringProperty(sPropertyDataTableName, rPropSet));
        ProcessCommandType(GetIntProperty(sPropertyDataCommandType, rPropSet));
        ProcessString(XML_CONDITION, XML_NAMESPACE_OOOW,
                      GetStringProperty(sPropertyCondition, rPropSet));
        DBG_ASSERT(sPresentation.equals(sEmpty),
                   "Unexpected presentation for database next field");
        ExportDataBaseElement(XML_DATABASE_NEXT, OUString(),
                              rPropSet, xPropSetInfo);
        break;

    case FIELD_ID_DATABASE_SELECT:
        ProcessString(XML_TABLE_NAME,
                      GetStringProperty(sPropertyDataTableName, rPropSet));
        ProcessCommandType(GetIntProperty(sPropertyDataCommandType, rPropSet));
        ProcessString(XML_CONDITION, XML_NAMESPACE_OOOW,
                      GetStringProperty(sPropertyCondition, rPropSet));
        ProcessInteger(XML_ROW_NUMBER,
                       GetIntProperty(sPropertySetNumber, rPropSet));
        DBG_ASSERT(sPresentation.equals(sEmpty),
                   "Unexpected presentation for database select field");
        ExportDataBaseElement(XML_DATABASE_ROW_SELECT, OUString(),
                              rPropSet, xPropSetInfo);
        break;

    case FIELD_ID_DATABASE_DISPLAY:
    {
        // get database, table and column name from field master
        const Reference<XPropertySet> & xMaster = GetMasterPropertySet(rTextField);
        ProcessString(XML_TABLE_NAME,
                      GetStringProperty(sPropertyDataTableName, xMaster));
        ProcessCommandType(GetIntProperty(sPropertyDataCommandType, xMaster));
        ProcessString(XML_COLUMN_NAME,
                      GetStringProperty(sPropertyDataColumnName, xMaster));
        // export number format if available (happens only for numbers!)
        if (!GetBoolProperty(sPropertyIsDataBaseFormat, rPropSet))
        {
            ProcessValueAndType(sal_False,  // doesn't happen for text
                                GetIntProperty(sPropertyNumberFormat,rPropSet),
                                sEmpty, sEmpty, 0.0, // not used
                                sal_False, sal_False, sal_True, sal_False);
        }
        ProcessDisplay(GetBoolProperty(sPropertyIsVisible, rPropSet),
                       sal_False);
        ExportDataBaseElement(XML_DATABASE_DISPLAY, sPresentation,
                              xMaster, xMaster->getPropertySetInfo());
        break;
    }

    case FIELD_ID_DOCINFO_REVISION:
        ProcessBoolean(XML_FIXED,
                       GetBoolProperty(sPropertyIsFixed, rPropSet), sal_False);
        ExportElement(MapDocInfoFieldName(nToken), sPresentation);
        break;

    case FIELD_ID_DOCINFO_EDIT_DURATION:
    case FIELD_ID_DOCINFO_SAVE_TIME:
    case FIELD_ID_DOCINFO_CREATION_TIME:
    case FIELD_ID_DOCINFO_PRINT_TIME:
    case FIELD_ID_DOCINFO_SAVE_DATE:
    case FIELD_ID_DOCINFO_CREATION_DATE:
    case FIELD_ID_DOCINFO_PRINT_DATE:
        ProcessValueAndType(sal_False,
                            GetIntProperty(sPropertyNumberFormat, rPropSet),
                            sEmpty, sEmpty, 0.0,
                            sal_False, sal_False, sal_True,
                            ! GetOptionalBoolProperty(
                                    sPropertyIsFixedLanguage,
                                    rPropSet, xPropSetInfo, sal_False ) );

        // todo: export date/time value, but values not available -> core bug
        ProcessBoolean(XML_FIXED,
                       GetBoolProperty(sPropertyIsFixed, rPropSet), sal_False);
        ExportElement(MapDocInfoFieldName(nToken), sPresentation);
        break;

    case FIELD_ID_DOCINFO_CREATION_AUTHOR:
    case FIELD_ID_DOCINFO_DESCRIPTION:
    case FIELD_ID_DOCINFO_PRINT_AUTHOR:
    case FIELD_ID_DOCINFO_TITLE:
    case FIELD_ID_DOCINFO_SUBJECT:
    case FIELD_ID_DOCINFO_KEYWORDS:
    case FIELD_ID_DOCINFO_SAVE_AUTHOR:
        if (xPropSetInfo->hasPropertyByName(sPropertyIsFixed))
        {
            ProcessBoolean(XML_FIXED,
                           GetBoolProperty(sPropertyIsFixed, rPropSet), sal_False);
        }
        ExportElement(MapDocInfoFieldName(nToken), sPresentation);
        break;

    case FIELD_ID_DOCINFO_CUSTOM:
    {
        ProcessValueAndType(sal_False,  // doesn't happen for text
                                GetIntProperty(sPropertyNumberFormat,rPropSet),
                                sEmpty, sEmpty, 0.0, // not used
                                sal_False, sal_False, sal_True,
                                ! GetOptionalBoolProperty(
                                    sPropertyIsFixedLanguage,
                                    rPropSet, xPropSetInfo, sal_False ));
        uno::Any aAny = rPropSet->getPropertyValue( sPropertyName );
        ::rtl::OUString sName;
        aAny >>= sName;
        ProcessString(XML_NAME, sName);
        ProcessBoolean(XML_FIXED, GetBoolProperty(sPropertyIsFixed, rPropSet), sal_False);
        ExportElement(XML_USER_DEFINED, sPresentation);
        break;
    }

    case FIELD_ID_COUNT_PAGES:
    case FIELD_ID_COUNT_PARAGRAPHS:
    case FIELD_ID_COUNT_WORDS:
    case FIELD_ID_COUNT_CHARACTERS:
    case FIELD_ID_COUNT_TABLES:
    case FIELD_ID_COUNT_GRAPHICS:
    case FIELD_ID_COUNT_OBJECTS:
        // all properties optional (applies to pages only, but I'll do
        // it for all for sake of common implementation)
        if (xPropSetInfo->hasPropertyByName(sPropertyNumberingType))
        {
            ProcessNumberingType(GetInt16Property(sPropertyNumberingType,
                                                  rPropSet));
        }
        ExportElement(MapCountFieldName(nToken), sPresentation);
        break;

    case FIELD_ID_CONDITIONAL_TEXT:
        ProcessString(XML_CONDITION, XML_NAMESPACE_OOOW,
                      GetStringProperty(sPropertyCondition, rPropSet));
        ProcessString(XML_STRING_VALUE_IF_TRUE,
                      GetStringProperty(sPropertyTrueContent, rPropSet));
        ProcessString(XML_STRING_VALUE_IF_FALSE,
                      GetStringProperty(sPropertyFalseContent, rPropSet));
        ProcessBoolean(XML_CURRENT_VALUE,
                       GetBoolProperty(sPropertyIsConditionTrue, rPropSet),
                       sal_False);
        ExportElement(XML_CONDITIONAL_TEXT, sPresentation);
        break;

    case FIELD_ID_HIDDEN_TEXT:
        ProcessString(XML_CONDITION, XML_NAMESPACE_OOOW,
                      GetStringProperty(sPropertyCondition, rPropSet));
        ProcessString(XML_STRING_VALUE,
                      GetStringProperty(sPropertyContent, rPropSet));
        ProcessBoolean(XML_IS_HIDDEN,
                       GetBoolProperty(sPropertyIsHidden, rPropSet),
                       sal_False);
        ExportElement(XML_HIDDEN_TEXT, sPresentation);
        break;

    case FIELD_ID_HIDDEN_PARAGRAPH:
        ProcessString(XML_CONDITION, XML_NAMESPACE_OOOW,
                      GetStringProperty(sPropertyCondition, rPropSet));
        ProcessBoolean(XML_IS_HIDDEN,
                       GetBoolProperty(sPropertyIsHidden, rPropSet),
                       sal_False);
        DBG_ASSERT(sPresentation.equals(sEmpty),
                   "Unexpected presentation for hidden paragraph field");
        ExportElement(XML_HIDDEN_PARAGRAPH);
        break;

    case FIELD_ID_TEMPLATE_NAME:
        ProcessString(XML_DISPLAY,
                      MapTemplateDisplayFormat(
                          GetInt16Property(sPropertyFileFormat, rPropSet)));
        ExportElement(XML_TEMPLATE_NAME, sPresentation);
        break;

    case FIELD_ID_CHAPTER:
        ProcessString(XML_DISPLAY,
                      MapChapterDisplayFormat(
                          GetInt16Property(sPropertyChapterFormat, rPropSet)));
        // API numbers 0..9, we number 1..10
        ProcessInteger(XML_OUTLINE_LEVEL,
                       GetInt8Property(sPropertyLevel, rPropSet) + 1);
        ExportElement(XML_CHAPTER, sPresentation);
        break;

    case FIELD_ID_FILE_NAME:
        // all properties are optional
        if (xPropSetInfo->hasPropertyByName(sPropertyFileFormat))
        {
            ProcessString(XML_DISPLAY,
                          MapFilenameDisplayFormat(
                             GetInt16Property(sPropertyFileFormat, rPropSet)));
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyIsFixed))
        {
            ProcessBoolean(XML_FIXED,
                           GetBoolProperty(sPropertyIsFixed, rPropSet),
                           sal_False);
        }
        ExportElement(XML_FILE_NAME, sPresentation);
        break;

    case FIELD_ID_REFPAGE_SET:
        ProcessBoolean(XML_ACTIVE,
                       GetBoolProperty(sPropertyOn, rPropSet), sal_True);
        ProcessIntegerDef(XML_PAGE_ADJUST,
                       GetInt16Property(sPropertyOffset, rPropSet), 0);
        DBG_ASSERT(sPresentation.equals(sEmpty),
                   "Unexpected presentation page variable field");
        ExportElement(XML_PAGE_VARIABLE_SET);
        break;

    case FIELD_ID_REFPAGE_GET:
        ProcessNumberingType(
            GetInt16Property(sPropertyNumberingType, rPropSet));
        ExportElement(XML_PAGE_VARIABLE_GET, sPresentation);
        break;

    case FIELD_ID_MACRO:
        ExportMacro( rPropSet, sPresentation );
        break;

    case FIELD_ID_REF_SEQUENCE:
        // reference to sequence: format, name, find value (and element)
        // was: if (nSeqNumber != -1) ...
        ProcessString(XML_REFERENCE_FORMAT,
                      MapReferenceType(GetInt16Property(
                          sPropertyReferenceFieldPart, rPropSet)),
                      XML_TEMPLATE);
        ProcessString(XML_REF_NAME,
                      MakeSequenceRefName(
                          GetInt16Property(sPropertySequenceNumber, rPropSet),
                          GetStringProperty(sPropertySourceName, rPropSet) ) );
        ExportElement(
            MapReferenceSource(
                GetInt16Property(sPropertyReferenceFieldSource, rPropSet)),
            sPresentation);
        break;

    case FIELD_ID_REF_REFERENCE:
    case FIELD_ID_REF_BOOKMARK:
        // reference to bookmarks, references: format, name (and element)
        ProcessString(XML_REFERENCE_FORMAT,
                      MapReferenceType(GetInt16Property(
                          sPropertyReferenceFieldPart, rPropSet)),
                      XML_TEMPLATE);
        ProcessString(XML_REF_NAME,
                      GetStringProperty(sPropertySourceName, rPropSet));
        ExportElement(
            MapReferenceSource(GetInt16Property(
                sPropertyReferenceFieldSource, rPropSet)),
            sPresentation);
        break;

    case FIELD_ID_REF_FOOTNOTE:
    case FIELD_ID_REF_ENDNOTE:
        // reference to end-/footnote: format, generate name, (and element)
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_NOTE_CLASS,
            FIELD_ID_REF_ENDNOTE==nToken ? XML_ENDNOTE : XML_FOOTNOTE );
        ProcessString(XML_REFERENCE_FORMAT,
                      MapReferenceType(GetInt16Property(
                          sPropertyReferenceFieldPart, rPropSet)),
                      XML_TEMPLATE);
        ProcessString(XML_REF_NAME,
                      MakeFootnoteRefName(GetInt16Property(
                          sPropertySequenceNumber, rPropSet)));
        ExportElement(
            MapReferenceSource(GetInt16Property(
                sPropertyReferenceFieldSource, rPropSet)),
            sPresentation);
        break;

    case FIELD_ID_DDE:
        // name from field master
         ProcessString(XML_CONNECTION_NAME,

                       GetStringProperty(sPropertyName,
                                         GetMasterPropertySet(rTextField)));
        ExportElement(XML_DDE_CONNECTION, sPresentation);
        break;

    case FIELD_ID_SHEET_NAME:
        // name of spreadsheet (Calc only)
        ExportElement(XML_SHEET_NAME, sPresentation);
        break;

    case FIELD_ID_URL:
    {
        // this field is a special case because it gets mapped onto a
        // hyperlink, rather than one of the regular text field.
        ProcessString(XML_HREF, GetExport().GetRelativeReference(GetStringProperty(sPropertyURL, rPropSet)),
                      sal_False, XML_NAMESPACE_XLINK);
        ProcessString(XML_TARGET_FRAME_NAME,
                      GetStringProperty(sPropertyTargetFrame,rPropSet),
                      sal_True, XML_NAMESPACE_OFFICE);
        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        SvXMLElementExport aUrlField(rExport, XML_NAMESPACE_TEXT, XML_A,
                                     sal_False, sal_False);
        GetExport().Characters(sPresentation);
        break;
    }

    case FIELD_ID_BIBLIOGRAPHY:
    {
        ProcessBibliographyData(rPropSet);
        ExportElement(XML_BIBLIOGRAPHY_MARK, sPresentation);
        break;
    }

    case FIELD_ID_SCRIPT:
        ProcessString(XML_LANGUAGE,
                      GetStringProperty(sPropertyScriptType, rPropSet),
                      sal_True, XML_NAMESPACE_SCRIPT);
        DBG_ASSERT(sPresentation.equals(sEmpty),
                   "Unexpected presentation for script field");
        if (GetBoolProperty(sPropertyURLContent, rPropSet))
        {
            ProcessString(XML_HREF,
                          GetExport().GetRelativeReference(GetStringProperty(sPropertyContent, rPropSet)),
                          sal_False, XML_NAMESPACE_XLINK);
            ExportElement(XML_SCRIPT);
        }
        else
        {
            ExportElement(XML_SCRIPT,
                          GetStringProperty(sPropertyContent, rPropSet));
        }
        break;

    case FIELD_ID_ANNOTATION:
    {
        // check for empty presentation (just in case)
        DBG_ASSERT(sPresentation.equals(sEmpty),
                   "Unexpected presentation for annotation field");

        // annotation element + content
        OUString aName;
        rPropSet->getPropertyValue(sPropertyName) >>= aName;
        if (!aName.isEmpty())
            GetExport().AddAttribute(XML_NAMESPACE_OFFICE, XML_NAME, aName);
        SvXMLElementExport aElem(GetExport(), XML_NAMESPACE_OFFICE,
                                 XML_ANNOTATION, sal_False, sal_True);

        // author
        OUString aAuthor( GetStringProperty(sPropertyAuthor, rPropSet) );
        if( !aAuthor.isEmpty() )
        {
            SvXMLElementExport aCreatorElem( GetExport(), XML_NAMESPACE_DC,
                                              XML_CREATOR, sal_True,
                                              sal_False );
            GetExport().Characters(aAuthor);
        }

        // date time
        DateTime aDate( GetDateTimeProperty(sPropertyDateTimeValue, rPropSet) );
        {
            OUStringBuffer aBuffer;
            ::sax::Converter::convertDateTime(aBuffer, aDate, true);
            SvXMLElementExport aDateElem( GetExport(), XML_NAMESPACE_DC,
                                              XML_DATE, sal_True,
                                              sal_False );
            GetExport().Characters(aBuffer.makeStringAndClear());
        }

        // initials
        OUString aInitials( GetStringProperty(sPropertyInitials, rPropSet) );
        if( !aInitials.isEmpty() )
        {
            SvXMLElementExport aCreatorElem( GetExport(), XML_NAMESPACE_TEXT,
                                              XML_SENDER_INITIALS, sal_True,
                                              sal_False );
            GetExport().Characters(aInitials);
        }

        com::sun::star::uno::Reference < com::sun::star::text::XText > xText;
        try
        {
            com::sun::star::uno::Any aRet = rPropSet->getPropertyValue(sPropertyTextRange);
            aRet >>= xText;
        }
        catch ( com::sun::star::uno::Exception& )
        {}

        if ( xText.is() )
            GetExport().GetTextParagraphExport()->exportText( xText );
        else
            ProcessParagraphSequence(GetStringProperty(sPropertyContent,rPropSet));
        break;
    }

    case FIELD_ID_COMBINED_CHARACTERS:
    {
        // The style with the combined characters attribute has
        // already been handled in the ExportField method. So all that
        // is left to do now is to export the characters.
        GetExport().Characters(sPresentation);
        break;
    }

    case FIELD_ID_META:
    {
        ExportMetaField(rPropSet, false, bProgress);
        break;
    }

    case FIELD_ID_MEASURE:
    {
        ProcessString(XML_KIND, MapMeasureKind(GetInt16Property(sPropertyMeasureKind, rPropSet)));
        ExportElement( XML_MEASURE, sPresentation );
        break;
    }

    case FIELD_ID_TABLE_FORMULA:
        ProcessString( XML_FORMULA,  XML_NAMESPACE_OOOW,
                       GetStringProperty(sPropertyContent, rPropSet) );
        ProcessDisplay( sal_True,
                        GetBoolProperty(sPropertyIsShowFormula, rPropSet),
                        sal_True );
        ProcessValueAndType( sal_False,
                             GetIntProperty(sPropertyNumberFormat, rPropSet),
                             sEmpty, sEmpty, 0.0f,
                             sal_False, sal_False, sal_True,
                             sal_False, sal_False );
        ExportElement( XML_TABLE_FORMULA, sPresentation );
        break;

    case FIELD_ID_DROP_DOWN:
    {
        ProcessString(XML_NAME, GetStringProperty(sPropertyName, rPropSet));
        ProcessString(XML_HELP,
                      GetStringProperty(sPropertyHelp, rPropSet), true);
        ProcessString(XML_HINT,
                      GetStringProperty(sPropertyTooltip, rPropSet), true);
        SvXMLElementExport aElem( GetExport(),
                                  XML_NAMESPACE_TEXT, XML_DROPDOWN,
                                  sal_False, sal_False );
        ProcessStringSequence
            (GetStringSequenceProperty( sPropertyItems, rPropSet ),
             GetStringProperty( sPropertySelectedItem, rPropSet ) );

        GetExport().Characters( sPresentation );
    }
    break;

    case FIELD_ID_DRAW_HEADER:
    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_PRESENTATION, XML_HEADER, sal_False, sal_False );
    }
    break;

    case FIELD_ID_DRAW_FOOTER:
    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_PRESENTATION, XML_FOOTER, sal_False, sal_False );
    }
    break;

    case FIELD_ID_DRAW_DATE_TIME:
    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_PRESENTATION, XML_DATE_TIME, sal_False, sal_False );
    }
    break;

    case FIELD_ID_UNKNOWN:
    default:
        OSL_FAIL("unkown field type encountered!");
        // always export content
        GetExport().Characters(sPresentation);
    }
}


/// export field declarations / field masters
void XMLTextFieldExport::ExportFieldDeclarations()
{
    Reference<XText> xEmptyText;
    ExportFieldDeclarations(xEmptyText);
}

/// export field declarations / field masters
void XMLTextFieldExport::ExportFieldDeclarations(
    const Reference<XText> & rText )
{
    // store lists for decl elements
    vector<OUString>                    aVarName;
    vector<OUString>                    aUserName;
    vector<OUString>                    aSeqName;
    vector<OUString>                    aDdeName;

    // get text fields supplier and field master name access
    Reference<XTextFieldsSupplier> xTextFieldsSupp(GetExport().GetModel(),
                                                   UNO_QUERY);
    if( !xTextFieldsSupp.is() )
        return;

    Reference<container::XNameAccess> xFieldMasterNameAccess(
        xTextFieldsSupp->getTextFieldMasters(), UNO_QUERY);

    // where to get the text field masters from?
    // a) we get a specific XText: then use pUsedMasters
    // b) the XText is empty: then export all text fields
    Sequence<OUString> aFieldMasters;
    if (rText.is())
    {
        // export only used masters
        DBG_ASSERT(NULL != pUsedMasters,
                   "field masters must be recorded in order to be "
                   "written out separatly" );
        if (NULL != pUsedMasters)
        {
            map<Reference<XText>, set<OUString> > ::iterator aMapIter =
                pUsedMasters->find(rText);
            if (aMapIter != pUsedMasters->end())
            {
                // found the set of used field masters
                set<OUString> & rOurMasters = aMapIter->second;

                // copy set to sequence
                aFieldMasters.realloc( rOurMasters.size() );
                sal_Int32 i = 0;
                for( set<OUString>::iterator aSetIter = rOurMasters.begin();
                     aSetIter != rOurMasters.end();
                     ++aSetIter, ++i )
                {
                    aFieldMasters[i] = *aSetIter;
                }

                pUsedMasters->erase(rText);
            }
            // else: XText not found -> ignore
        }
        // else: no field masters have been recorded -> ignore
    }
    else
    {
        // no XText: export all!
        aFieldMasters = xFieldMasterNameAccess->getElementNames();
    }

    for(sal_Int32 i=0; i<aFieldMasters.getLength(); i++) {

        // get field master name
        OUString sFieldMaster = aFieldMasters[i];

        // workaround for #no-bug#
        static const sal_Char sDB[] =
            "com.sun.star.text.FieldMaster.DataBase.";
        if ( sFieldMaster.matchIgnoreAsciiCaseAsciiL( sDB, sizeof(sDB)-1) )
        {
            continue;
        }


        OUString sFieldMasterType;
        OUString sVarName;
        ExplodeFieldMasterName(sFieldMaster, sFieldMasterType, sVarName);

        // get XPropertySet of this field master
        Reference<XPropertySet> xPropSet;
        Any aAny = xFieldMasterNameAccess->getByName(sFieldMaster);
        aAny >>= xPropSet;

        // save interesting field masters
        if (0 == sFieldMasterType.compareToAscii(FIELD_SERVICE_SETEXP))
        {
            sal_Int32 nType = GetIntProperty(sPropertySubType, xPropSet);

            // sequence or variable?
            if ( SetVariableType::SEQUENCE == nType )
            {
                aSeqName.push_back( sFieldMaster );
            }
            else
            {
                aVarName.push_back( sFieldMaster );
            }
        }
        else if (0 == sFieldMasterType.compareToAscii(FIELD_SERVICE_USER))
        {
            aUserName.push_back( sFieldMaster );
        }
        else if (0 == sFieldMasterType.compareToAscii(FIELD_SERVICE_DDE))
        {
            aDdeName.push_back( sFieldMaster );
        }
        else
        {
            ; // ignore
        }
    }

    // now process fields:

    // variable field masters:
    if ( !aVarName.empty() )
    {
        SvXMLElementExport aElem( GetExport(),
                                  XML_NAMESPACE_TEXT,
                                  XML_VARIABLE_DECLS,
                                  sal_True, sal_True );

        for (vector<OUString>::iterator aVarIter = aVarName.begin();
             aVarIter != aVarName.end();
             ++aVarIter) {

            OUString sName = *aVarIter;

            // get field master property set
            Reference<XPropertySet> xPropSet;
            Any aAny = xFieldMasterNameAccess->getByName(sName);
            aAny >>= xPropSet;

            // field name and type
            OUString sFieldMasterType;
            OUString sVarName;
            ExplodeFieldMasterName(sName, sFieldMasterType, sVarName);

            // determine string/numeric field
            sal_Bool bIsString = ( GetIntProperty(sPropertySubType, xPropSet)
                                   == SetVariableType::STRING );

            // get dependent field property set
            Reference<XPropertySet> xFieldPropSet;
            if (GetDependentFieldPropertySet(xPropSet, xFieldPropSet))
            {
                // process value and type.
                ProcessValueAndType(
                    bIsString,
                    GetIntProperty(sPropertyNumberFormat, xFieldPropSet),
                    sEmpty, sEmpty, 0.0,
                    sal_False, sal_True, sal_False, sal_False);
            }
            else
            {
                // If no dependent field is found, only string and
                // float types can be supported

                // number format: 0 is default number format for 1st
                // language. should be: getDefaultNumberFormat(Locale)
                // from NumberFormats
                ProcessValueAndType(
                    bIsString,
                    0, sEmpty, sEmpty, 0.0,
                    sal_False, sal_True, sal_False, sal_False);
            }

            ProcessString(XML_NAME, sVarName);
            ExportElement(XML_VARIABLE_DECL, sal_True);
        }
    }
    // else: no declarations element

    // sequence field masters:
    if ( !aSeqName.empty() )
    {
        SvXMLElementExport aElem( GetExport(),
                                  XML_NAMESPACE_TEXT,
                                  XML_SEQUENCE_DECLS,
                                  sal_True, sal_True );

        for (vector<OUString>::iterator aSeqIter = aSeqName.begin();
             aSeqIter != aSeqName.end();
             ++aSeqIter) {

            OUString sName = *aSeqIter;

            // get field master property set
            Reference<XPropertySet> xPropSet;
            Any aAny = xFieldMasterNameAccess->getByName(sName);
            aAny >>= xPropSet;

            // field name and type
            OUString sFieldMasterType;
            OUString sVarName;
            ExplodeFieldMasterName(sName, sFieldMasterType, sVarName);

            // outline level
            sal_Int32 nLevel = 1 + GetIntProperty(
                sPropertyChapterNumberingLevel, xPropSet);
            DBG_ASSERT(nLevel >= 0, "illegal outline level");
            DBG_ASSERT(nLevel < 127, "possible illegal outline level");
            ProcessInteger(XML_DISPLAY_OUTLINE_LEVEL, nLevel);

            // separation character
            if (nLevel > 0) {
                ProcessString(XML_SEPARATION_CHARACTER, GetStringProperty(
                    sPropertyNumberingSeparator, xPropSet));
            }
            ProcessString(XML_NAME, sVarName);
            ExportElement(XML_SEQUENCE_DECL, sal_True);
        }
    }
    // else: no declarations element

    // user field field masters:
    if ( !aUserName.empty() )
    {
        SvXMLElementExport aElem( GetExport(),
                                  XML_NAMESPACE_TEXT,
                                  XML_USER_FIELD_DECLS,
                                  sal_True, sal_True );

        for (vector<OUString>::iterator aUserIter = aUserName.begin();
             aUserIter != aUserName.end();
             ++aUserIter) {

            OUString sName = *aUserIter;

            // get field master property set
            Reference<XPropertySet> xPropSet;
            Any aAny = xFieldMasterNameAccess->getByName(sName);
            aAny >>= xPropSet;

            // field name and type
            OUString sFieldMasterType;
            OUString sVarName;
            ExplodeFieldMasterName(sName, sFieldMasterType, sVarName);

            if (GetBoolProperty(sPropertyIsExpression, xPropSet))
            {
                // expression:
                ProcessValueAndType(
                    sal_False,
                    0, sEmpty, sEmpty,
                    GetDoubleProperty(sPropertyValue, xPropSet),
                    sal_True,
                    sal_True,
                    sal_False,
                    sal_False);
            }
            else
            {
                // string: write regardless of default
                ProcessString(XML_VALUE_TYPE, XML_STRING, sal_False,
                              XML_NAMESPACE_OFFICE);
                ProcessString(XML_STRING_VALUE,
                              GetStringProperty(sPropertyContent, xPropSet),
                              sal_False, XML_NAMESPACE_OFFICE );
            }
            ProcessString(XML_NAME, sVarName);
            ExportElement(XML_USER_FIELD_DECL, sal_True);
        }
    }
    // else: no declarations element

    // DDE field field masters:
    if ( !aDdeName.empty() )
    {
        SvXMLElementExport aElem( GetExport(),
                                  XML_NAMESPACE_TEXT,
                                  XML_DDE_CONNECTION_DECLS,
                                  sal_True, sal_True );

        for (vector<OUString>::iterator aDdeIter = aDdeName.begin();
             aDdeIter != aDdeName.end();
             ++aDdeIter)
        {
            OUString sName = *aDdeIter;

            // get field master property set
            Reference<XPropertySet> xPropSet;
            Any aAny = xFieldMasterNameAccess->getByName(sName);
            aAny >>= xPropSet;

            // check if this connection is being used by a field
            Reference<XPropertySet> xDummy;
            if (GetDependentFieldPropertySet(xPropSet, xDummy))
            {

                ProcessString(XML_NAME,
                              GetStringProperty(sPropertyName, xPropSet),
                              sal_False, XML_NAMESPACE_OFFICE);

                // export elements; can't use ProcessString because
                // elements are in office namespace
                ProcessString(XML_DDE_APPLICATION,
                              GetStringProperty(sPropertyDDECommandType,
                                                xPropSet),
                              sal_False, XML_NAMESPACE_OFFICE);
                ProcessString(XML_DDE_TOPIC,
                              GetStringProperty(sPropertyDDECommandFile,
                                                xPropSet),
                              sal_False, XML_NAMESPACE_OFFICE);
                ProcessString(XML_DDE_ITEM,
                              GetStringProperty(sPropertyDDECommandElement,
                                                xPropSet),
                              sal_False, XML_NAMESPACE_OFFICE);
                sal_Bool bIsAutomaticUpdate = GetBoolProperty(
                    sPropertyIsAutomaticUpdate, xPropSet);
                if (bIsAutomaticUpdate)
                {
                    GetExport().AddAttribute(XML_NAMESPACE_OFFICE,
                                             XML_AUTOMATIC_UPDATE,
                                             XML_TRUE);
                }

                ExportElement(XML_DDE_CONNECTION_DECL, sal_True);
            }
            // else: no dependent field -> no export of field declaration
        }
    }
    // else: no declarations element
}

void XMLTextFieldExport::SetExportOnlyUsedFieldDeclarations(
    sal_Bool bExportOnlyUsed)
{
    delete pUsedMasters;
    pUsedMasters = NULL;

    // create used masters set (if none is used)
    if (bExportOnlyUsed)
        pUsedMasters = new map<Reference<XText>, set<OUString> > ;
}

void XMLTextFieldExport::ExportElement(enum XMLTokenEnum eElementName,
                                       sal_Bool bAddSpace)
{
    // can't call ExportElement(eElementName, const OUString&) with empty
    // string because xmlprinter only uses empty tags if no content
    // (not even empty content) was written.

    DBG_ASSERT(XML_TOKEN_INVALID != eElementName, "invalid element name!");
    if (XML_TOKEN_INVALID != eElementName)
    {
        // Element
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                  eElementName, bAddSpace, bAddSpace );
    } // else: ignore
}

void XMLTextFieldExport::ExportElement(enum XMLTokenEnum eElementName,
                                       const OUString& sContent,
                                       sal_Bool bAddSpace)
{
    DBG_ASSERT(eElementName != XML_TOKEN_INVALID, "invalid element name!");
    if (eElementName != XML_TOKEN_INVALID)
    {
        // Element
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                  eElementName, bAddSpace, bAddSpace );
        // export content
        GetExport().Characters(sContent);
    } else {
        // always export content
        GetExport().Characters(sContent);
    }
}

void XMLTextFieldExport::ExportMacro(
    const Reference<XPropertySet> & rPropSet,
    const OUString& rContent )
{
    // some strings we'll need
    OUString sEventType( RTL_CONSTASCII_USTRINGPARAM( "EventType" ));
    OUString sStarBasic( RTL_CONSTASCII_USTRINGPARAM( "StarBasic" ));
    OUString sScript( RTL_CONSTASCII_USTRINGPARAM( "Script" ));
    OUString sLibrary( RTL_CONSTASCII_USTRINGPARAM( "Library" ));
    OUString sMacroName( RTL_CONSTASCII_USTRINGPARAM( "MacroName" ));
    OUString sOnClick( RTL_CONSTASCII_USTRINGPARAM( "OnClick" ));
    OUString sPropertyMacroLibrary( RTL_CONSTASCII_USTRINGPARAM( "MacroLibrary" ));
    OUString sPropertyMacroName( RTL_CONSTASCII_USTRINGPARAM( "MacroName" ));
    OUString sPropertyScriptURL( RTL_CONSTASCII_USTRINGPARAM( "ScriptURL" ));


    // the description attribute
    ProcessString(XML_DESCRIPTION,
                  GetStringProperty(sPropertyHint, rPropSet),
                  rContent);

    // the element
    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                              XML_EXECUTE_MACRO, sal_False, sal_False );

    // the <office:events>-macro:

    // 1) build sequence of PropertyValues
    Sequence<PropertyValue> aSeq;
    OUString sName;
    rPropSet->getPropertyValue( sPropertyScriptURL ) >>= sName;

    // if the ScriptURL property is not empty then this is a Scripting
    // Framework URL, otherwise treat it as a Basic Macro
    if (!sName.isEmpty())
    {
        aSeq = Sequence<PropertyValue> (2);
        PropertyValue* pArr = aSeq.getArray();
        pArr[0].Name = sEventType;
        pArr[0].Value <<= sScript;
        pArr[1].Name = sScript;
        pArr[1].Value = rPropSet->getPropertyValue( sPropertyScriptURL );
    }
    else
    {
        aSeq = Sequence<PropertyValue> (3);
        PropertyValue* pArr = aSeq.getArray();
        pArr[0].Name = sEventType;
        pArr[0].Value <<= sStarBasic;
        pArr[1].Name = sLibrary;
        pArr[1].Value = rPropSet->getPropertyValue( sPropertyMacroLibrary );
        pArr[2].Name = sMacroName;
        pArr[2].Value = rPropSet->getPropertyValue( sPropertyMacroName );
    }

    // 2) export the sequence
    GetExport().GetEventExport().ExportSingleEvent( aSeq, sOnClick, sal_False );

    // and finally, the field presentation
    GetExport().Characters(rContent);
}

void XMLTextFieldExport::ExportMetaField(
    const Reference<XPropertySet> & i_xMeta,
    bool i_bAutoStyles, sal_Bool i_bProgress )
{
    bool doExport(!i_bAutoStyles); // do not export element if autostyles
    // check version >= 1.2
    switch (GetExport().getDefaultVersion()) {
        case SvtSaveOptions::ODFVER_011: // fall thru
        case SvtSaveOptions::ODFVER_010: doExport = false; break;
        default: break;
    }

    const Reference < XEnumerationAccess > xEA( i_xMeta, UNO_QUERY_THROW );
    const Reference < XEnumeration > xTextEnum( xEA->createEnumeration() );

    if (doExport)
    {
        const Reference<rdf::XMetadatable> xMeta( i_xMeta, UNO_QUERY_THROW );

        // style:data-style-name
        ProcessValueAndType(sal_False,
            GetIntProperty(sPropertyNumberFormat, i_xMeta),
            sEmpty, sEmpty, 0.0, sal_False, sal_False, sal_True,
            sal_False, sal_False /*, sal_False*/ );

        // text:meta-field without xml:id is invalid
        xMeta->ensureMetadataReference();

        // xml:id for RDF metadata
        GetExport().AddAttributeXmlId(xMeta);
    }

    SvXMLElementExport aElem( GetExport(), doExport,
        XML_NAMESPACE_TEXT, XML_META_FIELD, sal_False, sal_False );

    // recurse to export content
    GetExport().GetTextParagraphExport()->
        exportTextRangeEnumeration( xTextEnum, i_bAutoStyles, i_bProgress );
}

/// export all data-style related attributes
void XMLTextFieldExport::ProcessValueAndType(
    sal_Bool bIsString,     /// do we process a string or a number?
    sal_Int32 nFormatKey,   /// format key for NumberFormatter; inv. if string
    const OUString& sContent,   /// string content; possibly invalid
    const OUString& sDefault,   /// default string
    double fValue,          /// float content; possibly invalid
    sal_Bool bExportValue,  /// export value attribute?
    sal_Bool bExportValueType,  /// export value-type attribute?
    sal_Bool bExportStyle,  /// export style-sttribute?
    sal_Bool bForceSystemLanguage, /// export language attributes?
    sal_Bool bTimeStyle)    // exporting a time style?
{
    // String or number?
    if (bIsString)
    {

        // string: attributes value-type=string, string-value=...

        if (bExportValue || bExportValueType)
        {
            XMLNumberFormatAttributesExportHelper::SetNumberFormatAttributes(
                GetExport(), sContent, sDefault, bExportValue);
        }

    }
    else
    {

        // number: value-type=..., value...=..., data-style-name=...

        DBG_ASSERT(bExportValueType || !bExportValue, "value w/o value type not supported!");

        // take care of illegal formats
        // (shouldn't happen, but does if document is corrupted)
        if (-1 != nFormatKey)
        {
            if (bExportValue || bExportValueType)
            {
                XMLNumberFormatAttributesExportHelper::
                    SetNumberFormatAttributes(
                        GetExport(), nFormatKey, fValue, bExportValue);
            }

            if (bExportStyle)
            {
                // don't export language (if desired)
                if( bForceSystemLanguage )
                    nFormatKey =
                        GetExport().dataStyleForceSystemLanguage( nFormatKey );

                OUString sDataStyleName =
                    GetExport().getDataStyleName(nFormatKey, bTimeStyle);
                if( !sDataStyleName.isEmpty() )
                {
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                              XML_DATA_STYLE_NAME,
                                                sDataStyleName );
                } // else: ignore (no valid number format)
            }  // else: ignore (no number format)
        }
    }
}



/// process display related properties
void XMLTextFieldExport::ProcessDisplay(sal_Bool bIsVisible,
                                        sal_Bool bIsCommand,
                                        sal_Bool bValueDefault)
{
    enum XMLTokenEnum eValue;

    if (bIsVisible)
    {
        eValue = bIsCommand ? XML_FORMULA : XML_VALUE;
    }
    else
    {
        eValue = XML_NONE;
    }

    // omit attribute if default
    if (!bValueDefault || (eValue != XML_VALUE))
    {
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_DISPLAY, eValue);
    }
}



/// export boolean property
void XMLTextFieldExport::ProcessBoolean(enum XMLTokenEnum eName,
                                        sal_Bool bBool, sal_Bool bDefault)
{
    DBG_ASSERT( eName != XML_TOKEN_INVALID, "invalid element token");
    if ( XML_TOKEN_INVALID == eName )
        return;

    // write attribute (if different than default)
    // negate to force 0/1 values (and make sal_Bool comparable)
    if ((!bBool) != (!bDefault)) {
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, eName,
                                 (bBool ? XML_TRUE : XML_FALSE) );
    }
}




/// export string attribute
void XMLTextFieldExport::ProcessString(enum XMLTokenEnum eName,
                                       const OUString& sValue,
                                       sal_Bool bOmitEmpty,
                                       sal_uInt16 nPrefix)
{
    DBG_ASSERT( eName != XML_TOKEN_INVALID, "invalid element token");
    if ( XML_TOKEN_INVALID == eName )
        return;

    // check for empty string, if applicable
    if ( bOmitEmpty && sValue.isEmpty() )
        return;

    // write attribute
    GetExport().AddAttribute(nPrefix, eName, sValue);
}

void XMLTextFieldExport::ProcessString(enum XMLTokenEnum eName,
                                       sal_uInt16 nValuePrefix,
                                       const OUString& sValue,
                                       sal_Bool bOmitEmpty,
                                       sal_uInt16 nPrefix)
{
    OUString sQValue =
        GetExport().GetNamespaceMap().GetQNameByKey( nValuePrefix, sValue, sal_False );
    ProcessString( eName, sQValue, bOmitEmpty, nPrefix );
}

/// export a string attribute
void XMLTextFieldExport::ProcessString(enum XMLTokenEnum eName,
                                       const ::rtl::OUString& sValue,
                                       const ::rtl::OUString& sDefault,
                                       sal_uInt16 nPrefix)
{
    if (sValue != sDefault)
    {
        ProcessString(eName, sValue, sal_False, nPrefix);
    }
}

/// export a string attribute
void XMLTextFieldExport::ProcessString(enum XMLTokenEnum eName,
                                       sal_uInt16 nValuePrefix,
                                       const ::rtl::OUString& sValue,
                                       const ::rtl::OUString& sDefault,
                                       sal_uInt16 nPrefix)
{
    if (sValue != sDefault)
    {
        ProcessString(eName, nValuePrefix, sValue, sal_False, nPrefix);
    }
}


/// export string attribute
void XMLTextFieldExport::ProcessString(
    enum XMLTokenEnum eName,
    enum XMLTokenEnum eValue,
    sal_Bool bOmitEmpty,
    sal_uInt16 nPrefix)
{
    DBG_ASSERT( eName != XML_TOKEN_INVALID, "invalid element token" );
    DBG_ASSERT( bOmitEmpty || (eValue != XML_TOKEN_INVALID),
                "invalid value token" );
    if ( XML_TOKEN_INVALID == eName )
        return;

    // check for empty string, if applicable
    if (bOmitEmpty && (eValue == XML_TOKEN_INVALID))
        return;

    GetExport().AddAttribute(nPrefix, eName, eValue);
}

/// export a string attribute
void XMLTextFieldExport::ProcessString(
    enum XMLTokenEnum eName,
    enum XMLTokenEnum eValue,
    enum XMLTokenEnum eDefault,
    sal_uInt16 nPrefix)
{
    if ( eValue != eDefault )
        ProcessString( eName, eValue, sal_False, nPrefix);
}


/// export a string as a sequence of paragraphs
void XMLTextFieldExport::ProcessParagraphSequence(
    const ::rtl::OUString& sParagraphSequence)
{
    // iterate over all string-pieces separated by return (0x0a) and
    // put each inside a paragraph element.
    SvXMLTokenEnumerator aEnumerator(sParagraphSequence, sal_Char(0x0a));
    OUString aSubString;
    while (aEnumerator.getNextToken(aSubString))
    {
        SvXMLElementExport aParagraph(
            GetExport(), XML_NAMESPACE_TEXT, XML_P, sal_True, sal_False);
        GetExport().Characters(aSubString);
    }
}

// export an integer attribute
void XMLTextFieldExport::ProcessInteger(enum XMLTokenEnum eName,
                                        sal_Int32 nNum)
{
    DBG_ASSERT( eName != XML_TOKEN_INVALID, "invalid element token");
    if ( XML_TOKEN_INVALID == eName )
        return;

    GetExport().AddAttribute(XML_NAMESPACE_TEXT, eName,
                             OUString::valueOf(nNum));
}

/// export an integer attribute, omit if default
void XMLTextFieldExport::ProcessIntegerDef(enum XMLTokenEnum eName,
                                        sal_Int32 nNum, sal_Int32 nDefault)
{
    if (nNum != nDefault)
        ProcessInteger(eName, nNum);
}



/// export a numbering type
void XMLTextFieldExport::ProcessNumberingType(sal_Int16 nNumberingType)
{
    // process only if real format (not: like page descriptor)
    if (NumberingType::PAGE_DESCRIPTOR != nNumberingType)
    {
        OUStringBuffer sTmp( 10 );
        // number type: num format
        GetExport().GetMM100UnitConverter().convertNumFormat( sTmp,
                                                              nNumberingType );
        GetExport().AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_FORMAT,
                                      sTmp.makeStringAndClear() );
        // and letter sync, if applicable
        GetExport().GetMM100UnitConverter().convertNumLetterSync( sTmp,
                                                              nNumberingType );

        if (sTmp.getLength())
        {
            GetExport().AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_LETTER_SYNC,
                                     sTmp.makeStringAndClear() );
        }
    }
    // else: like page descriptor => ignore
}


/// export a date, time, or duration
void XMLTextFieldExport::ProcessDateTime(enum XMLTokenEnum eName,
                                         double dValue,
                                         sal_Bool bIsDate,
                                         sal_Bool bIsDuration,
                                         sal_Bool bOmitDurationIfZero,
                                         sal_uInt16 nPrefix)
{
    // truncate for date granularity
    if (bIsDate)
    {
        dValue = ::rtl::math::approxFloor(dValue);
    }

    OUStringBuffer aBuffer;
    if (bIsDuration)
    {
        // date/time durationM handle bOmitDurationIfZero
        if (!bOmitDurationIfZero || !::rtl::math::approxEqual(dValue, 0.0))
        {
            ::sax::Converter::convertDuration(aBuffer, dValue);
        }
    }
    else
    {
        // date/time value
        rExport.GetMM100UnitConverter().convertDateTime(aBuffer, dValue);
    }

    // output attribute
    ProcessString(eName, aBuffer.makeStringAndClear(), sal_True, nPrefix);
}

/// export a date or time
void XMLTextFieldExport::ProcessDateTime(enum XMLTokenEnum eName,
                                         const DateTime& rTime,
                                         sal_Bool bIsDate,
                                         sal_uInt16 nPrefix)
{
    OUStringBuffer aBuffer;

    DateTime aDateTime(rTime);

    // truncate dates
    if(bIsDate)
    {
        aDateTime.HundredthSeconds = 0;
        aDateTime.Seconds = 0;
        aDateTime.Minutes = 0;
        aDateTime.Hours = 0;
    }

    // date/time value
    ::sax::Converter::convertDateTime(aBuffer, aDateTime);

    // output attribute
    ProcessString(eName, aBuffer.makeStringAndClear(), sal_True, nPrefix);
}

/// export a date, time, or duration
void XMLTextFieldExport::ProcessDateTime(enum XMLTokenEnum eName,
                                         sal_Int32 nMinutes,
                                         sal_Bool bIsDate,
                                         sal_Bool bIsDuration,
                                         sal_Bool bOmitDurationIfZero,
                                         sal_uInt16 nPrefix)
{
    // handle bOmitDurationIfZero here, because we can precisely compare ints
    if (!(bIsDuration && bOmitDurationIfZero && (nMinutes==0)))
    {
        ProcessDateTime(eName, (double)nMinutes / (double)(24*60),
                        bIsDate, bIsDuration, bOmitDurationIfZero, nPrefix);
    }
}


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



void XMLTextFieldExport::ProcessBibliographyData(
    const Reference<XPropertySet>& rPropSet)
{
    // get the values
    Any aAny = rPropSet->getPropertyValue(sPropertyFields);
    Sequence<PropertyValue> aValues;
    aAny >>= aValues;

    // one attribute per value (unless empty)
    sal_Int32 nLength = aValues.getLength();
    for (sal_Int32 i = 0; i < nLength; i++)
    {
        if (aValues[i].Name.equalsAsciiL("BibiliographicType",
                                         sizeof("BibiliographicType")-1))
        {
            sal_Int16 nTypeId = 0;
            aValues[i].Value >>= nTypeId;
            OUStringBuffer sBuf;

            if (SvXMLUnitConverter::convertEnum(sBuf, nTypeId,
                                                aBibliographyDataTypeMap))
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     XML_BIBLIOGRAPHY_TYPE,
                                     sBuf.makeStringAndClear());
            }
            // else: ignore this argument
        }
        else
        {
            OUString sStr;
            aValues[i].Value >>= sStr;

            if (!sStr.isEmpty())
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     MapBibliographyFieldName(aValues[i].Name),
                                     sStr);
            }
        }
    }
}

/// export CommandTypeAttribute
void XMLTextFieldExport::ProcessCommandType(
    sal_Int32 nCommandType)
{
    enum XMLTokenEnum eToken = XML_TOKEN_INVALID;
    switch( nCommandType )
    {
        case sdb::CommandType::TABLE:   eToken = XML_TABLE; break;
        case sdb::CommandType::QUERY:   eToken = XML_QUERY; break;
        case sdb::CommandType::COMMAND: eToken = XML_COMMAND; break;
    }

    if( eToken != XML_TOKEN_INVALID )
        rExport.AddAttribute( XML_NAMESPACE_TEXT, XML_TABLE_TYPE, eToken );
}


void XMLTextFieldExport::ProcessStringSequence(
    const Sequence<OUString>& rSequence,
    const OUString sSelected )
{
    // find selected element
    sal_Int32 nSelected = -1;
    sal_Int32 nLength = rSequence.getLength();
    const OUString* pSequence = rSequence.getConstArray();
    for( sal_Int32 i = 0; i < nLength; i++ )
    {
        if( pSequence[i] == sSelected )
            nSelected = i;
    }

    // delegate to ProcessStringSequence(OUString,sal_Int32)
    ProcessStringSequence( rSequence, nSelected );
}

void XMLTextFieldExport::ProcessStringSequence(
    const Sequence<OUString>& rSequence,
    sal_Int32 nSelected )
{
    sal_Int32 nLength = rSequence.getLength();
    const OUString* pSequence = rSequence.getConstArray();
    for( sal_Int32 i = 0; i < nLength; i++ )
    {
        if( i == nSelected )
            rExport.AddAttribute( XML_NAMESPACE_TEXT,
                                  XML_CURRENT_SELECTED, XML_TRUE );
        rExport.AddAttribute( XML_NAMESPACE_TEXT, XML_VALUE, pSequence[i] );
        SvXMLElementExport aElement( rExport, XML_NAMESPACE_TEXT, XML_LABEL,
                                     sal_False, sal_False );
    }
}

void XMLTextFieldExport::ExportDataBaseElement(
    enum XMLTokenEnum eElementName,
    const OUString& sPresentation,
    const Reference<XPropertySet>& rPropertySet,
    const Reference<XPropertySetInfo>& rPropertySetInfo )
{
    DBG_ASSERT( eElementName != XML_TOKEN_INVALID, "need token" );
    DBG_ASSERT( rPropertySet.is(), "need property set" );
    DBG_ASSERT( rPropertySetInfo.is(), "need property set info" );

    // get database properties
    OUString sDataBaseName;
    OUString sDataBaseURL;
    OUString sStr;
    if( ( rPropertySet->getPropertyValue( sPropertyDataBaseName ) >>= sStr )
        && !sStr.isEmpty() )
    {
        sDataBaseName = sStr;
    }
    else if( rPropertySetInfo->hasPropertyByName( sPropertyDataBaseURL ) &&
             (rPropertySet->getPropertyValue( sPropertyDataBaseURL ) >>= sStr) &&
             !sStr.isEmpty() )
    {
        sDataBaseURL = sStr;
    }

    // add database name property (if present)
    if( !sDataBaseName.isEmpty() )
        rExport.AddAttribute( XML_NAMESPACE_TEXT, XML_DATABASE_NAME,
                              sDataBaseName );
    SvXMLElementExport aDataBaseElement( GetExport(),
                                         XML_NAMESPACE_TEXT, eElementName,
                                         sal_False, sal_False );

    // write URL as children
    if( !sDataBaseURL.isEmpty() )
    {
        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sDataBaseURL );
        SvXMLElementExport aDataSourceElement(
            GetExport(), XML_NAMESPACE_FORM, XML_CONNECTION_RESOURCE,
            sal_False, sal_False );
    }

    // write presentation
    rExport.Characters( sPresentation );
}



// explode a field master name into field type and field name
sal_Bool XMLTextFieldExport::ExplodeFieldMasterName(
    const OUString& sMasterName, OUString& sFieldType, OUString& sVarName)
{
    sal_Int32 nLength = sFieldMasterPrefix.getLength();
    sal_Int32 nSeparator = sMasterName.indexOf('.', nLength);
    sal_Bool bReturn = sal_True;

#ifdef DBG_UTIL
    // check for service name
    bReturn &= (0 == sFieldMasterPrefix.compareTo(sMasterName, nLength));
#endif

    // '.' found?
    if (nSeparator <= nLength) {
        nSeparator = sMasterName.getLength();
        DBG_WARNING("no field var name!");
        bReturn = sal_False;
    }
    else
    {
        sFieldType = sMasterName.copy(nLength, nSeparator-nLength);
        sVarName = sMasterName.copy(nSeparator+1);
    }

    return bReturn;
}


// for XDependentTextFields, get PropertySet of FieldMaster
Reference<XPropertySet> XMLTextFieldExport::GetMasterPropertySet(
    const Reference<XTextField> & rTextField)
{
    // name, value => get Property set of TextFieldMaster
    Reference<XDependentTextField> xDep(rTextField, UNO_QUERY);
    return xDep->getTextFieldMaster();
}

// get PropertySet of (any; the first) dependent field
sal_Bool XMLTextFieldExport::GetDependentFieldPropertySet(
    const Reference<XPropertySet> & xMaster,
    Reference<XPropertySet> & xField)
{
    Any aAny;
    Sequence<Reference<XDependentTextField> > aFields;
    aAny = xMaster->getPropertyValue(sPropertyDependentTextFields);
    aAny >>= aFields;

    // any fields?
    if (aFields.getLength() > 0)
    {
        // get first one and return
        Reference<XDependentTextField> xTField = aFields[0];
        xField = Reference<XPropertySet>(xTField, UNO_QUERY);
        DBG_ASSERT(xField.is(),
                  "Surprisinlgy, this TextField refuses to be a PropertySet!");
        return sal_True;
    }
    else
    {
        return sal_False;
    }
}


/// map placeholder type
enum XMLTokenEnum XMLTextFieldExport::MapPlaceholderType(sal_uInt16 nType)
{
    enum XMLTokenEnum eType = XML_TEXT;

    switch (nType)
    {
        case PlaceholderType::TEXT:
            eType = XML_TEXT;
            break;

        case PlaceholderType::TABLE:
            eType = XML_TABLE;
            break;

        case PlaceholderType::TEXTFRAME:
            eType = XML_TEXT_BOX;
            break;

        case PlaceholderType::GRAPHIC:
            eType = XML_IMAGE;
            break;

        case PlaceholderType::OBJECT:
            eType = XML_OBJECT;
            break;

        default:
            // unkown placeholder: XML_TEXT
            OSL_FAIL("unkown placeholder type");
    }

    return eType;
}


/// element name for author fields
enum XMLTokenEnum XMLTextFieldExport::MapAuthorFieldName(
    const Reference<XPropertySet> & xPropSet)
{
    // Initalen oder voller Name?
    return GetBoolProperty(sPropertyFullName, xPropSet)
        ? XML_AUTHOR_NAME : XML_AUTHOR_INITIALS;
}

enum XMLTokenEnum XMLTextFieldExport::MapPageNumberName(
    const Reference<XPropertySet> & xPropSet,
    sal_Int32& nOffset)
{
    enum XMLTokenEnum eName = XML_TOKEN_INVALID;
    PageNumberType ePage;
    Any aAny = xPropSet->getPropertyValue(sPropertySubType);
    ePage = *(PageNumberType*)aAny.getValue();

    switch (ePage)
    {
        case PageNumberType_PREV:
            eName = XML_PREVIOUS;
            nOffset += 1;
            break;
        case PageNumberType_CURRENT:
            eName = XML_CURRENT;
            break;
        case PageNumberType_NEXT:
            eName = XML_NEXT;
            nOffset -= 1;
            break;
        default:
            OSL_FAIL("unknown page number type");
            eName = XML_TOKEN_INVALID;
            break;
    }

    return eName;
}

/// map TemplateDisplayFormat to XML
enum XMLTokenEnum XMLTextFieldExport::MapTemplateDisplayFormat(sal_Int16 nFormat)
{
    enum XMLTokenEnum eName = XML_TOKEN_INVALID;

    switch (nFormat)
    {
        case TemplateDisplayFormat::FULL:
            eName = XML_FULL;
            break;
        case TemplateDisplayFormat::PATH:
            eName = XML_PATH;
            break;
        case TemplateDisplayFormat::NAME:
            eName = XML_NAME;
            break;
        case TemplateDisplayFormat::NAME_AND_EXT:
            eName = XML_NAME_AND_EXTENSION;
            break;
        case TemplateDisplayFormat::AREA:
            eName = XML_AREA;
            break;
        case TemplateDisplayFormat::TITLE:
            eName = XML_TITLE;
            break;
        default:
            OSL_FAIL("unknown template display format");
            eName = XML_TOKEN_INVALID;
            break;
    }

    return eName;
}

/// map count/statistics field token to XML name
enum XMLTokenEnum XMLTextFieldExport::MapCountFieldName(FieldIdEnum nToken)
{
    enum XMLTokenEnum eElement = XML_TOKEN_INVALID;

    switch (nToken)
    {
        case FIELD_ID_COUNT_PAGES:
            eElement = XML_PAGE_COUNT;
            break;
        case FIELD_ID_COUNT_PARAGRAPHS:
            eElement = XML_PARAGRAPH_COUNT;
            break;
        case FIELD_ID_COUNT_WORDS:
            eElement = XML_WORD_COUNT;
            break;
        case FIELD_ID_COUNT_CHARACTERS:
            eElement = XML_CHARACTER_COUNT;
            break;
        case FIELD_ID_COUNT_TABLES:
            eElement = XML_TABLE_COUNT;
            break;
        case FIELD_ID_COUNT_GRAPHICS:
            eElement = XML_IMAGE_COUNT;
            break;
        case FIELD_ID_COUNT_OBJECTS:
            eElement = XML_OBJECT_COUNT;
            break;
        default:
            OSL_FAIL("no count field token");
            eElement = XML_TOKEN_INVALID;
            break;
    }

    return eElement;
}

/// map ChapterDisplayFormat to XML string
enum XMLTokenEnum XMLTextFieldExport::MapChapterDisplayFormat(sal_Int16 nFormat)
{
    enum XMLTokenEnum eName = XML_TOKEN_INVALID;

    switch (nFormat)
    {
        case ChapterFormat::NAME:
            eName = XML_NAME;
            break;
        case ChapterFormat::NUMBER:
            eName = XML_NUMBER;
            break;
        case ChapterFormat::NAME_NUMBER:
            eName = XML_NUMBER_AND_NAME;
            break;
        case ChapterFormat::NO_PREFIX_SUFFIX:
            eName = XML_PLAIN_NUMBER_AND_NAME;
            break;
        case ChapterFormat::DIGIT:
            eName = XML_PLAIN_NUMBER;
            break;
        default:
            OSL_FAIL("unkown chapter display format");
            eName = XML_TOKEN_INVALID;
            break;
    }

    return eName;
}


/// map FilenameDisplayFormat to XML attribute names
enum XMLTokenEnum XMLTextFieldExport::MapFilenameDisplayFormat(sal_Int16 nFormat)
{
    enum XMLTokenEnum eName = XML_TOKEN_INVALID;

    switch (nFormat)
    {
        case FilenameDisplayFormat::FULL:
            eName = XML_FULL;
            break;
        case FilenameDisplayFormat::PATH:
            eName = XML_PATH;
            break;
        case FilenameDisplayFormat::NAME:
            eName = XML_NAME;
            break;
        case FilenameDisplayFormat::NAME_AND_EXT:
            eName = XML_NAME_AND_EXTENSION;
            break;
        default:
            OSL_FAIL("unknown filename display format");
    }

    return eName;
}


/// map ReferenceFieldPart to XML string
enum XMLTokenEnum XMLTextFieldExport::MapReferenceType(sal_Int16 nType)
{
    enum XMLTokenEnum eElement = XML_TOKEN_INVALID;

    switch (nType)
    {
        case ReferenceFieldPart::PAGE:
            eElement = XML_PAGE;
            break;
        case ReferenceFieldPart::CHAPTER:
            eElement = XML_CHAPTER;
            break;
        case ReferenceFieldPart::TEXT:
            eElement = XML_TEXT;
            break;
        case ReferenceFieldPart::UP_DOWN:
            eElement = XML_DIRECTION;
            break;
        case ReferenceFieldPart::CATEGORY_AND_NUMBER:
            eElement = XML_CATEGORY_AND_VALUE;
            break;
        case ReferenceFieldPart::ONLY_CAPTION:
            eElement = XML_CAPTION;
            break;
        case ReferenceFieldPart::ONLY_SEQUENCE_NUMBER:
            eElement = XML_VALUE;
            break;
        case ReferenceFieldPart::PAGE_DESC:
            // small hack: this value never gets written, because
            // XML_TEMPLATE is default
            eElement = XML_TEMPLATE;
            break;
        // Core implementation for direct cross-references (#i81002#)
        case ReferenceFieldPart::NUMBER:
            eElement = XML_NUMBER;
            break;
        case ReferenceFieldPart::NUMBER_NO_CONTEXT:
            eElement = XML_NUMBER_NO_SUPERIOR;
            break;
        case ReferenceFieldPart::NUMBER_FULL_CONTEXT:
            eElement = XML_NUMBER_ALL_SUPERIOR;
            break;
        default:
            OSL_FAIL("unknown reference type");
            eElement = XML_TEMPLATE;
            break;
    }

    return eElement;
}

/// map ReferenceFieldPart to XML string
enum XMLTokenEnum XMLTextFieldExport::MapReferenceSource(sal_Int16 nType)
{
    enum XMLTokenEnum eElement = XML_TOKEN_INVALID;

    switch (nType)
    {
        case ReferenceFieldSource::REFERENCE_MARK:
            eElement = XML_REFERENCE_REF;
            break;
        case ReferenceFieldSource::SEQUENCE_FIELD:
            eElement = XML_SEQUENCE_REF;
            break;
        case ReferenceFieldSource::BOOKMARK:
            eElement = XML_BOOKMARK_REF;
            break;
        case ReferenceFieldSource::FOOTNOTE:
        case ReferenceFieldSource::ENDNOTE:
            eElement = XML_NOTE_REF;
            break;
        default:
            OSL_FAIL("unkown reference source");
            break;
    }

    return eElement;
}


/// element name for sender fields
enum XMLTokenEnum XMLTextFieldExport::MapSenderFieldName(
    const Reference<XPropertySet> & xPropSet)
{
    enum XMLTokenEnum eName = XML_TOKEN_INVALID;

    // sub-field type
    switch (GetInt16Property(sPropertyFieldSubType, xPropSet))
    {
        case UserDataPart::COMPANY :
            eName = XML_SENDER_COMPANY;
            break;
        case UserDataPart::FIRSTNAME :
            eName = XML_SENDER_FIRSTNAME;
            break;
        case UserDataPart::NAME :
            eName = XML_SENDER_LASTNAME;
            break;
        case UserDataPart::SHORTCUT :
            eName = XML_SENDER_INITIALS;
            break;
        case UserDataPart::STREET :
            eName = XML_SENDER_STREET;
            break;
        case UserDataPart::COUNTRY :
            eName = XML_SENDER_COUNTRY;
            break;
        case UserDataPart::ZIP :
            eName = XML_SENDER_POSTAL_CODE;
            break;
        case UserDataPart::CITY :
            eName = XML_SENDER_CITY;
            break;
        case UserDataPart::TITLE :
            eName = XML_SENDER_TITLE;
            break;
        case UserDataPart::POSITION :
            eName = XML_SENDER_POSITION;
            break;
        case UserDataPart::PHONE_PRIVATE :
            eName = XML_SENDER_PHONE_PRIVATE;
            break;
        case UserDataPart::PHONE_COMPANY :
            eName = XML_SENDER_PHONE_WORK;
            break;
        case UserDataPart::FAX :
            eName = XML_SENDER_FAX;
            break;
        case UserDataPart::EMAIL :
            eName = XML_SENDER_EMAIL;
            break;
        case UserDataPart::STATE :
            eName = XML_SENDER_STATE_OR_PROVINCE;
            break;
        default:
            DBG_WARNING("unknown sender type");
            eName = XML_TOKEN_INVALID;
            break;
    }

    return eName;
}

enum XMLTokenEnum XMLTextFieldExport::MapDocInfoFieldName(
    enum FieldIdEnum nToken)
{
    enum XMLTokenEnum eElement = XML_TOKEN_INVALID;

    switch (nToken)
    {
        case FIELD_ID_DOCINFO_CREATION_AUTHOR:
            eElement = XML_INITIAL_CREATOR;
            break;
        case FIELD_ID_DOCINFO_CREATION_DATE:
            eElement = XML_CREATION_DATE;
            break;
        case FIELD_ID_DOCINFO_CREATION_TIME:
            eElement = XML_CREATION_TIME;
            break;
        case FIELD_ID_DOCINFO_DESCRIPTION:
            eElement = XML_DESCRIPTION;
            break;
        case FIELD_ID_DOCINFO_PRINT_TIME:
            eElement = XML_PRINT_TIME;
            break;
        case FIELD_ID_DOCINFO_PRINT_DATE:
            eElement = XML_PRINT_DATE;
            break;
        case FIELD_ID_DOCINFO_PRINT_AUTHOR:
            eElement = XML_PRINTED_BY;
            break;
        case FIELD_ID_DOCINFO_TITLE:
            eElement = XML_TITLE;
            break;
        case FIELD_ID_DOCINFO_SUBJECT:
            eElement = XML_SUBJECT;
            break;
        case FIELD_ID_DOCINFO_KEYWORDS:
            eElement = XML_KEYWORDS;
            break;
        case FIELD_ID_DOCINFO_REVISION:
            eElement = XML_EDITING_CYCLES;
            break;
        case FIELD_ID_DOCINFO_EDIT_DURATION:
            eElement = XML_EDITING_DURATION;
            break;
        case FIELD_ID_DOCINFO_SAVE_TIME:
            eElement = XML_MODIFICATION_TIME;
            break;
        case FIELD_ID_DOCINFO_SAVE_DATE:
            eElement = XML_MODIFICATION_DATE;
            break;
        case FIELD_ID_DOCINFO_SAVE_AUTHOR:
            eElement = XML_CREATOR;
            break;
        default:
            DBG_WARNING("unknown docinfo field type!");
            eElement = XML_TOKEN_INVALID;
            break;
    }

    return eElement;
}

enum XMLTokenEnum XMLTextFieldExport::MapBibliographyFieldName(OUString sName)
{
    enum XMLTokenEnum eName = XML_TOKEN_INVALID;

    if (sName.equalsAsciiL("Identifier", sizeof("Identifier")-1))
    {
        eName = XML_IDENTIFIER;
    }
    else if (sName.equalsAsciiL("BibiliographicType",
                                sizeof("BibiliographicType")-1))
    {
        eName = XML_BIBLIOGRAPHY_TYPE;
    }
    else if (sName.equalsAsciiL("Address", sizeof("Address")-1))
    {
        eName = XML_ADDRESS;
    }
    else if (sName.equalsAsciiL("Annote", sizeof("Annote")-1))
    {
        eName = XML_ANNOTE;
    }
    else if (sName.equalsAsciiL("Author", sizeof("Author")-1))
    {
        eName = XML_AUTHOR;
    }
    else if (sName.equalsAsciiL("Booktitle", sizeof("Booktitle")-1))
    {
        eName = XML_BOOKTITLE;
    }
    else if (sName.equalsAsciiL("Chapter", sizeof("Chapter")-1))
    {
        eName = XML_CHAPTER;
    }
    else if (sName.equalsAsciiL("Edition", sizeof("Edition")-1))
    {
        eName = XML_EDITION;
    }
    else if (sName.equalsAsciiL("Editor", sizeof("Editor")-1))
    {
        eName = XML_EDITOR;
    }
    else if (sName.equalsAsciiL("Howpublished", sizeof("Howpublished")-1))
    {
        eName = XML_HOWPUBLISHED;
    }
    else if (sName.equalsAsciiL("Institution", sizeof("Institution")-1))
    {
        eName = XML_INSTITUTION;
    }
    else if (sName.equalsAsciiL("Journal", sizeof("Journal")-1))
    {
        eName = XML_JOURNAL;
    }
    else if (sName.equalsAsciiL("Month", sizeof("Month")-1))
    {
        eName = XML_MONTH;
    }
    else if (sName.equalsAsciiL("Note", sizeof("Note")-1))
    {
        eName = XML_NOTE;
    }
    else if (sName.equalsAsciiL("Number", sizeof("Number")-1))
    {
        eName = XML_NUMBER;
    }
    else if (sName.equalsAsciiL("Organizations", sizeof("Organizations")-1))
    {
        eName = XML_ORGANIZATIONS;
    }
    else if (sName.equalsAsciiL("Pages", sizeof("Pages")-1))
    {
        eName = XML_PAGES;
    }
    else if (sName.equalsAsciiL("Publisher", sizeof("Publisher")-1))
    {
        eName = XML_PUBLISHER;
    }
    else if (sName.equalsAsciiL("School", sizeof("School")-1))
    {
        eName = XML_SCHOOL;
    }
    else if (sName.equalsAsciiL("Series", sizeof("Series")-1))
    {
        eName = XML_SERIES;
    }
    else if (sName.equalsAsciiL("Title", sizeof("Title")-1))
    {
        eName = XML_TITLE;
    }
    else if (sName.equalsAsciiL("Report_Type", sizeof("Report_Type")-1))
    {
        eName = XML_REPORT_TYPE;
    }
    else if (sName.equalsAsciiL("Volume", sizeof("Volume")-1))
    {
        eName = XML_VOLUME;
    }
    else if (sName.equalsAsciiL("Year", sizeof("Year")-1))
    {
        eName = XML_YEAR;
    }
    else if (sName.equalsAsciiL("URL", sizeof("URL")-1))
    {
        eName = XML_URL;
    }
    else if (sName.equalsAsciiL("Custom1", sizeof("Custom1")-1))
    {
        eName = XML_CUSTOM1;
    }
    else if (sName.equalsAsciiL("Custom2", sizeof("Custom2")-1))
    {
        eName = XML_CUSTOM2;
    }
    else if (sName.equalsAsciiL("Custom3", sizeof("Custom3")-1))
    {
        eName = XML_CUSTOM3;
    }
    else if (sName.equalsAsciiL("Custom4", sizeof("Custom4")-1))
    {
        eName = XML_CUSTOM4;
    }
    else if (sName.equalsAsciiL("Custom5", sizeof("Custom5")-1))
    {
        eName = XML_CUSTOM5;
    }
    else if (sName.equalsAsciiL("ISBN", sizeof("ISBN")-1))
    {
        eName = XML_ISBN;
    }
    else
    {
        OSL_FAIL("Unknown bibliography info data");
        eName = XML_TOKEN_INVALID;
    }

    return eName;
}

enum XMLTokenEnum XMLTextFieldExport::MapMeasureKind(sal_Int16 nKind)
{
    switch( nKind )
    {
    case 0:
        return XML_VALUE;
    case 1:
        return XML_UNIT;
    }
    return XML_GAP;
}

OUString XMLTextFieldExport::MakeFootnoteRefName(
    sal_Int16 nSeqNo)
{
    // generate foot-/endnote ID
    OUStringBuffer aBuf;
    aBuf.appendAscii("ftn");
    aBuf.append((sal_Int32)nSeqNo);
    return aBuf.makeStringAndClear();
}

OUString XMLTextFieldExport::MakeSequenceRefName(
    sal_Int16 nSeqNo,
    const OUString& rSeqName)
{
    // generate foot-/endnote ID
    OUStringBuffer aBuf;
    aBuf.appendAscii("ref");
    aBuf.append(rSeqName);
    aBuf.append((sal_Int32)nSeqNo);
    return aBuf.makeStringAndClear();
}

//
// Property accessor helper functions
//

// to be relegated (does that word exist?) to a more appropriate place
//


inline sal_Bool GetBoolProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    sal_Bool bBool = *(sal_Bool *)aAny.getValue();
    return bBool;
}

inline sal_Bool GetOptionalBoolProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet,
    const Reference<XPropertySetInfo> & xPropSetInfo,
    sal_Bool bDefault)
{
    return xPropSetInfo->hasPropertyByName( sPropName )
        ? GetBoolProperty( sPropName, xPropSet ) : bDefault;
}

inline double GetDoubleProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    double fDouble = 0.0;
    aAny >>= fDouble;
    return fDouble;
}

inline OUString const GetStringProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    OUString sString;
    aAny >>= sString;
    return sString;
}

inline sal_Int32 GetIntProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    sal_Int32 nInt = 0;
    aAny >>= nInt;
    return nInt;
}

inline sal_Int16 GetInt16Property(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    sal_Int16 nInt = 0;
    aAny >>= nInt;
    return nInt;
}

inline sal_Int8 GetInt8Property(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    sal_Int8 nInt = 0;
    aAny >>= nInt;
    return nInt;
}

inline DateTime const GetDateTimeProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    DateTime aTime;
    aAny >>= aTime;
    return aTime;
}

inline Date const GetDateProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    Date aDate;
    aAny >>= aDate;
    return aDate;
}

inline Sequence<OUString> const GetStringSequenceProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    Sequence<OUString> aSequence;
    aAny >>= aSequence;
    return aSequence;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
