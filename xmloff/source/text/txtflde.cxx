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
 *  export of all text fields
 */
#include <comphelper/propertyvalue.hxx>
#include <txtflde.hxx>
#include <txtfld.hxx>
#include <xmloff/XMLEventExport.hxx>
#include <xmloff/families.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/numehelp.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/maptype.hxx>

#include "XMLTextCharStyleNamesElementExport.hxx"
#include <sax/tools/converter.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/UserDataPart.hpp>
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>

#include <com/sun/star/text/SetVariableType.hpp>
#include <com/sun/star/text/PlaceholderType.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/text/ChapterFormat.hpp>
#include <com/sun/star/text/TemplateDisplayFormat.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/text/BibliographyDataType.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/rdf/XMetadatable.hpp>
#include <comphelper/sequence.hxx>
#include <o3tl/any.hxx>
#include <o3tl/safeint.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>
#include <rtl/math.hxx>
#include <sal/log.hxx>

#include <vector>


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


char const FIELD_SERVICE_SENDER[] = "ExtendedUser";
char const FIELD_SERVICE_AUTHOR[] = "Author";
char const FIELD_SERVICE_JUMPEDIT[] = "JumpEdit";
char const FIELD_SERVICE_GETEXP[] = "GetExpression";
char const FIELD_SERVICE_SETEXP[] = "SetExpression";
char const FIELD_SERVICE_USER[] = "User";
char const FIELD_SERVICE_INPUT[] = "Input";
char const FIELD_SERVICE_USERINPUT[] = "InputUser";
char const FIELD_SERVICE_DATETIME[] = "DateTime";
char const FIELD_SERVICE_PAGENUMBER[] = "PageNumber";
char const FIELD_SERVICE_DB_NEXT[] = "DatabaseNextSet";
char const FIELD_SERVICE_DB_SELECT[] = "DatabaseNumberOfSet";
char const FIELD_SERVICE_DB_NUMBER[] = "DatabaseSetNumber";
char const FIELD_SERVICE_DB_DISPLAY[] = "Database";
char const FIELD_SERVICE_DB_NAME[] = "DatabaseName";
char const FIELD_SERVICE_CONDITIONAL_TEXT[] = "ConditionalText";
char const FIELD_SERVICE_HIDDEN_TEXT[] = "HiddenText";
char const FIELD_SERVICE_HIDDEN_PARAGRAPH[] = "HiddenParagraph";
char const FIELD_SERVICE_DOC_INFO_CHANGE_AUTHOR[] = "DocInfo.ChangeAuthor";
char const FIELD_SERVICE_DOC_INFO_CHANGE_AUTHOR2[] = "docinfo.ChangeAuthor";
char const FIELD_SERVICE_DOC_INFO_CHANGE_DATE_TIME[] = "DocInfo.ChangeDateTime";
char const FIELD_SERVICE_DOC_INFO_CHANGE_DATE_TIME2[] = "docinfo.ChangeDateTime";
char const FIELD_SERVICE_DOC_INFO_EDIT_TIME[] = "DocInfo.EditTime";
char const FIELD_SERVICE_DOC_INFO_EDIT_TIME2[] = "docinfo.EditTime";
char const FIELD_SERVICE_DOC_INFO_DESCRIPTION[] = "DocInfo.Description";
char const FIELD_SERVICE_DOC_INFO_DESCRIPTION2[] = "docinfo.Description";
char const FIELD_SERVICE_DOC_INFO_CREATE_AUTHOR[] = "DocInfo.CreateAuthor";
char const FIELD_SERVICE_DOC_INFO_CREATE_AUTHOR2[] = "docinfo.CreateAuthor";
char const FIELD_SERVICE_DOC_INFO_CREATE_DATE_TIME[] = "DocInfo.CreateDateTime";
char const FIELD_SERVICE_DOC_INFO_CREATE_DATE_TIME2[] = "docinfo.CreateDateTime";
char const FIELD_SERVICE_DOC_INFO_CUSTOM[] = "DocInfo.Custom";
char const FIELD_SERVICE_DOC_INFO_CUSTOM2[] = "docinfo.Custom";
char const FIELD_SERVICE_DOC_INFO_PRINT_AUTHOR[] = "DocInfo.PrintAuthor";
char const FIELD_SERVICE_DOC_INFO_PRINT_AUTHOR2[] = "docinfo.PrintAuthor";
char const FIELD_SERVICE_DOC_INFO_PRINT_DATE_TIME[] = "DocInfo.PrintDateTime";
char const FIELD_SERVICE_DOC_INFO_PRINT_DATE_TIME2[] = "docinfo.PrintDateTime";
char const FIELD_SERVICE_DOC_INFO_KEY_WORDS[] = "DocInfo.KeyWords";
char const FIELD_SERVICE_DOC_INFO_KEY_WORDS2[] = "docinfo.KeyWords";
char const FIELD_SERVICE_DOC_INFO_SUBJECT[] = "DocInfo.Subject";
char const FIELD_SERVICE_DOC_INFO_SUBJECT2[] = "docinfo.Subject";
char const FIELD_SERVICE_DOC_INFO_TITLE[] = "DocInfo.Title";
char const FIELD_SERVICE_DOC_INFO_TITLE2[] = "docinfo.Title";
char const FIELD_SERVICE_DOC_INFO_REVISION[] = "DocInfo.Revision";
char const FIELD_SERVICE_DOC_INFO_REVISION2[] = "docinfo.Revision";
char const FIELD_SERVICE_FILE_NAME[] = "FileName";
char const FIELD_SERVICE_CHAPTER[] = "Chapter";
char const FIELD_SERVICE_TEMPLATE_NAME[] = "TemplateName";
char const FIELD_SERVICE_PAGE_COUNT[] = "PageCount";
char const FIELD_SERVICE_PARAGRAPH_COUNT[] = "ParagraphCount";
char const FIELD_SERVICE_WORD_COUNT[] = "WordCount";
char const FIELD_SERVICE_CHARACTER_COUNT[] = "CharacterCount";
char const FIELD_SERVICE_TABLE_COUNT[] = "TableCount";
char const FIELD_SERVICE_GRAPHIC_COUNT[] = "GraphicObjectCount";
char const FIELD_SERVICE_OBJECT_COUNT[] = "EmbeddedObjectCount";
char const FIELD_SERVICE_REFERENCE_PAGE_SET[] = "ReferencePageSet";
char const FIELD_SERVICE_REFERENCE_PAGE_GET[] = "ReferencePageGet";
char const FIELD_SERVICE_SHEET_NAME[] = "SheetName";
char const FIELD_SERVICE_PAGE_NAME[] = "PageName";
char const FIELD_SERVICE_MACRO[] = "Macro";
char const FIELD_SERVICE_GET_REFERENCE[] = "GetReference";
char const FIELD_SERVICE_DDE[] = "DDE";
char const FIELD_SERVICE_URL[] = "URL";
char const FIELD_SERVICE_BIBLIOGRAPHY[] = "Bibliography";
char const FIELD_SERVICE_SCRIPT[] = "Script";
char const FIELD_SERVICE_ANNOTATION[] = "Annotation";
char const FIELD_SERVICE_COMBINED_CHARACTERS[] = "CombinedCharacters";
char const FIELD_SERVICE_META[] = "MetadataField";
char const FIELD_SERVICE_MEASURE[] = "Measure";
char const FIELD_SERVICE_TABLE_FORMULA[] = "TableFormula";
char const FIELD_SERVICE_DROP_DOWN[] = "DropDown";

namespace
{
/// Walks up the parent chain of xText and returns the topmost text.
uno::Reference<text::XText> GetToplevelText(const uno::Reference<text::XText>& xText)
{
    uno::Reference<text::XText> xRet = xText;
    while (true)
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xRet, uno::UNO_QUERY);
        if (!xPropertySet.is())
            return xRet;

        if (!xPropertySet->getPropertySetInfo()->hasPropertyByName(u"ParentText"_ustr))
            return xRet;

        uno::Reference<text::XText> xParent;
        if (xPropertySet->getPropertyValue(u"ParentText"_ustr) >>= xParent)
            xRet = xParent;
        else
            return xRet;
    }
    return xRet;
}
}

SvXMLEnumStringMapEntry<FieldIdEnum> const aFieldServiceNameMapping[] =
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
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_PAGE_NAME, FIELD_ID_PAGENAME ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_URL, FIELD_ID_URL ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_MEASURE, FIELD_ID_MEASURE ),

    // deprecated fields
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_TABLE_FORMULA, FIELD_ID_TABLE_FORMULA ),
    ENUM_STRING_MAP_ENTRY( FIELD_SERVICE_DROP_DOWN, FIELD_ID_DROP_DOWN ),

    { nullptr, 0, FieldIdEnum(0) }
};


// property accessor helper functions
static bool GetBoolProperty(const OUString&,
                                      const Reference<XPropertySet> &);
static bool GetOptionalBoolProperty(const OUString&,
                                              const Reference<XPropertySet> &,
                                              const Reference<XPropertySetInfo> &,
                                              bool bDefault);
static double GetDoubleProperty(const OUString&,
                                      const Reference<XPropertySet> &);
static OUString GetStringProperty(const OUString&,
                                        const Reference<XPropertySet> &);
static sal_Int32 GetIntProperty(const OUString&,
                                      const Reference<XPropertySet> &);
static sal_Int16 GetInt16Property(const OUString&,
                                        const Reference<XPropertySet> &);
static sal_Int8 GetInt8Property(const OUString&,
                                      const Reference<XPropertySet> &);
static util::DateTime GetDateTimeProperty( const OUString& sPropName,
                                           const Reference<XPropertySet> & xPropSet);
static Sequence<OUString> GetStringSequenceProperty(
                                   const OUString& sPropName,
                                   const Reference<XPropertySet> & xPropSet);


    // service names
constexpr OUString gsServicePrefix(u"com.sun.star.text.textfield."_ustr);
constexpr OUStringLiteral gsFieldMasterPrefix(u"com.sun.star.text.FieldMaster.");
constexpr OUString gsPresentationServicePrefix(u"com.sun.star.presentation.TextField."_ustr);

    // property names
constexpr OUString gsPropertyAdjust(u"Adjust"_ustr);
constexpr OUStringLiteral gsPropertyAuthor(u"Author");
constexpr OUStringLiteral gsPropertyChapterFormat(u"ChapterFormat");
constexpr OUStringLiteral gsPropertyChapterNumberingLevel(u"ChapterNumberingLevel");
constexpr OUStringLiteral gsPropertyCharStyleNames(u"CharStyleNames");
constexpr OUString gsPropertyCondition(u"Condition"_ustr);
constexpr OUString gsPropertyContent(u"Content"_ustr);
constexpr OUStringLiteral gsPropertyDataBaseName(u"DataBaseName");
constexpr OUString gsPropertyDataBaseURL(u"DataBaseURL"_ustr);
constexpr OUStringLiteral gsPropertyDataColumnName(u"DataColumnName");
constexpr OUString gsPropertyDataCommandType(u"DataCommandType"_ustr);
constexpr OUString gsPropertyDataTableName(u"DataTableName"_ustr);
constexpr OUString gsPropertyDateTime(u"DateTime"_ustr);
constexpr OUString gsPropertyDateTimeValue(u"DateTimeValue"_ustr);
constexpr OUStringLiteral gsPropertyDDECommandElement(u"DDECommandElement");
constexpr OUStringLiteral gsPropertyDDECommandFile(u"DDECommandFile");
constexpr OUStringLiteral gsPropertyDDECommandType(u"DDECommandType");
constexpr OUStringLiteral gsPropertyDependentTextFields(u"DependentTextFields");
constexpr OUStringLiteral gsPropertyFalseContent(u"FalseContent");
constexpr OUStringLiteral gsPropertyFields(u"Fields");
constexpr OUStringLiteral gsPropertyFieldSubType(u"UserDataType");
constexpr OUString gsPropertyFileFormat(u"FileFormat"_ustr);
constexpr OUStringLiteral gsPropertyFullName(u"FullName");
constexpr OUString gsPropertyHint(u"Hint"_ustr);
constexpr OUStringLiteral gsPropertyInitials(u"Initials");
constexpr OUStringLiteral gsPropertyInstanceName(u"InstanceName");
constexpr OUStringLiteral gsPropertyIsAutomaticUpdate(u"IsAutomaticUpdate");
constexpr OUStringLiteral gsPropertyIsConditionTrue(u"IsConditionTrue");
constexpr OUString gsPropertyIsDataBaseFormat(u"DataBaseFormat"_ustr);
constexpr OUString gsPropertyIsDate(u"IsDate"_ustr);
constexpr OUString gsPropertyIsExpression(u"IsExpression"_ustr);
constexpr OUString gsPropertyIsFixed(u"IsFixed"_ustr);
constexpr OUString gsPropertyIsFixedLanguage(u"IsFixedLanguage"_ustr);
constexpr OUString gsPropertyIsHidden(u"IsHidden"_ustr);
constexpr OUStringLiteral gsPropertyIsInput(u"Input");
constexpr OUString gsPropertyIsShowFormula(u"IsShowFormula"_ustr);
constexpr OUString gsPropertyIsVisible(u"IsVisible"_ustr);
constexpr OUStringLiteral gsPropertyItems(u"Items");
constexpr OUStringLiteral gsPropertyLevel(u"Level");
constexpr OUStringLiteral gsPropertyMeasureKind(u"Kind");
constexpr OUString gsPropertyName(u"Name"_ustr);
constexpr OUStringLiteral gsPropertyParentName(u"ParentName");
constexpr OUString gsPropertyNumberFormat(u"NumberFormat"_ustr);
constexpr OUStringLiteral gsPropertyNumberingSeparator(u"NumberingSeparator");
constexpr OUString gsPropertyNumberingType(u"NumberingType"_ustr);
constexpr OUString gsPropertyOffset(u"Offset"_ustr);
constexpr OUStringLiteral gsPropertyOn(u"On");
constexpr OUStringLiteral gsPropertyPlaceholderType(u"PlaceHolderType");
constexpr OUString gsPropertyReferenceFieldFlags(u"ReferenceFieldFlags"_ustr);
constexpr OUString gsPropertyReferenceFieldPart(u"ReferenceFieldPart"_ustr);
constexpr OUString gsPropertyReferenceFieldSource(u"ReferenceFieldSource"_ustr);
constexpr OUString gsPropertyReferenceFieldLanguage(u"ReferenceFieldLanguage"_ustr);
constexpr OUStringLiteral gsPropertyScriptType(u"ScriptType");
constexpr OUStringLiteral gsPropertySelectedItem(u"SelectedItem");
constexpr OUString gsPropertySequenceNumber(u"SequenceNumber"_ustr);
constexpr OUStringLiteral gsPropertySequenceValue(u"SequenceValue");
constexpr OUString gsPropertySetNumber(u"SetNumber"_ustr);
constexpr OUString gsPropertySourceName(u"SourceName"_ustr);
constexpr OUString gsPropertySubType(u"SubType"_ustr);
constexpr OUStringLiteral gsPropertyTargetFrame(u"TargetFrame");
constexpr OUStringLiteral gsPropertyTrueContent(u"TrueContent");
constexpr OUStringLiteral gsPropertyURL(u"URL");
constexpr OUStringLiteral gsPropertyURLContent(u"URLContent");
constexpr OUStringLiteral gsPropertyUserText(u"UserText");
constexpr OUString gsPropertyValue(u"Value"_ustr);
constexpr OUString gsPropertyVariableName(u"VariableName"_ustr);
constexpr OUString gsPropertyHelp(u"Help"_ustr);
constexpr OUString gsPropertyTooltip(u"Tooltip"_ustr);
constexpr OUStringLiteral gsPropertyTextRange(u"TextRange");

XMLTextFieldExport::XMLTextFieldExport( SvXMLExport& rExp,
                                        std::unique_ptr<XMLPropertyState> pCombinedCharState)
    : rExport(rExp),
      pCombinedCharactersPropertyState(std::move(pCombinedCharState))
{
    SetExportOnlyUsedFieldDeclarations();
}

XMLTextFieldExport::~XMLTextFieldExport()
{
}

/// get the field ID (as in FieldIDEnum) from XTextField
enum FieldIdEnum XMLTextFieldExport::GetFieldID(
    const Reference<XTextField> & rTextField,
    const Reference<XPropertySet> & xPropSet)
{
    // get service names for rTextField (via XServiceInfo service)
    Reference<XServiceInfo> xService(rTextField, UNO_QUERY);
    const Sequence<OUString> aServices = xService->getSupportedServiceNames();

    OUString sFieldName;    // service name postfix of current field

    // search for TextField service name
    const OUString* pNames = std::find_if(aServices.begin(), aServices.end(),
        [](const OUString& rName) { return rName.matchIgnoreAsciiCase(gsServicePrefix); });
    if (pNames != aServices.end())
    {
        // TextField found => postfix is field type!
        sFieldName = pNames->copy(gsServicePrefix.getLength());
    }

    // if this is not a normal text field, check if it's a presentation text field
    if( sFieldName.isEmpty() )
    {
        // search for TextField service name
        pNames = std::find_if(aServices.begin(), aServices.end(),
            [](const OUString& rName) { return rName.startsWith(gsPresentationServicePrefix); });
        if (pNames != aServices.end())
        {
            // TextField found => postfix is field type!
            sFieldName = pNames->copy(gsPresentationServicePrefix.getLength());
        }

        if( !sFieldName.isEmpty() )
        {
            if( sFieldName == "Header" )
            {
                return FIELD_ID_DRAW_HEADER;
            }
            else if( sFieldName == "Footer" )
            {
                return FIELD_ID_DRAW_FOOTER;
            }
            else if( sFieldName == "DateTime" )
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
    std::u16string_view sFieldName,             // field (master) name
    const Reference<XPropertySet> & xPropSet)   // for subtype
{
    // we'll proceed in 2 steps:
    // a) map service name to preliminary FIELD_ID
    // b) map those prelim. FIELD_IDs that correspond to several field types
    //    (in our (XML) world) to final FIELD IDs


    // a) find prelim. FIELD_ID via aFieldServiceMapping

    // check for non-empty service name
    DBG_ASSERT(!sFieldName.empty(), "no valid service name!");
    enum FieldIdEnum nToken = FIELD_ID_UNKNOWN;
    if (!sFieldName.empty())
    {
        // map name to prelim. ID
        bool bRet = SvXMLUnitConverter::convertEnum(
            nToken, sFieldName, aFieldServiceNameMapping);

        // check return
        DBG_ASSERT(bRet, "Unknown field service name encountered!");
    }

    // b) map prelim. to final FIELD_IDs
    switch (nToken) {
        case FIELD_ID_VARIABLE_SET:
            if (GetBoolProperty(gsPropertyIsInput, xPropSet))
            {
                nToken = FIELD_ID_VARIABLE_INPUT;
            }
            else
            {
                switch (GetIntProperty(gsPropertySubType, xPropSet))
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
            switch (GetIntProperty(gsPropertySubType, xPropSet))
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
            if (GetBoolProperty(gsPropertyIsDate, xPropSet))
            {
                nToken = FIELD_ID_DATE;
            }
            break;

        case FIELD_ID_PAGENUMBER:
            // NumberingType not available in non-Writer apps
            if (xPropSet->getPropertySetInfo()->
                hasPropertyByName(gsPropertyNumberingType))
            {
                if (NumberingType::CHAR_SPECIAL == GetIntProperty(
                                            gsPropertyNumberingType, xPropSet))
                {
                    nToken = FIELD_ID_PAGESTRING;
                }
            }
            break;

        case FIELD_ID_DOCINFO_CREATION_TIME:
            if (GetBoolProperty(gsPropertyIsDate, xPropSet))
            {
                nToken = FIELD_ID_DOCINFO_CREATION_DATE;
            }
            break;

        case FIELD_ID_DOCINFO_PRINT_TIME:
            if (GetBoolProperty(gsPropertyIsDate, xPropSet))
            {
                nToken = FIELD_ID_DOCINFO_PRINT_DATE;
            }
            break;

        case FIELD_ID_DOCINFO_SAVE_TIME:
            if (GetBoolProperty(gsPropertyIsDate, xPropSet))
            {
                nToken = FIELD_ID_DOCINFO_SAVE_DATE;
            }
            break;

        case FIELD_ID_REF_REFERENCE:
            switch (GetInt16Property(gsPropertyReferenceFieldSource, xPropSet))
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
                case ReferenceFieldSource::STYLE:
                    nToken = FIELD_ID_REF_STYLE;
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
        case FIELD_ID_PAGENAME:
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
bool XMLTextFieldExport::IsStringField(
    FieldIdEnum nFieldType,
    const Reference<XPropertySet> & xPropSet)
{
    switch (nFieldType) {

    case FIELD_ID_VARIABLE_GET:
    case FIELD_ID_VARIABLE_SET:
    case FIELD_ID_VARIABLE_INPUT:
    {
        // depends on field sub type
        return ( GetIntProperty(gsPropertySubType, xPropSet) ==
                 SetVariableType::STRING                    );
    }

    case FIELD_ID_USER_GET:
    case FIELD_ID_USER_INPUT:
    {
        Reference<XTextField> xTextField(xPropSet, UNO_QUERY);
        DBG_ASSERT(xTextField.is(), "field is no XTextField!");
        bool bRet = GetBoolProperty(gsPropertyIsExpression,
                                        GetMasterPropertySet(xTextField));
        return !bRet;
    }

    case FIELD_ID_META:
        return 0 > GetIntProperty(gsPropertyNumberFormat, xPropSet);

    case FIELD_ID_DATABASE_DISPLAY:
        // TODO: depends on... ???
        // workaround #no-bug#: no data type
        return 5100 == GetIntProperty(gsPropertyNumberFormat, xPropSet);

    case FIELD_ID_TABLE_FORMULA:
        // legacy field: always a number field (because it always has
        // a number format)
        return false;

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
        return false;

    case FIELD_ID_COMBINED_CHARACTERS:
    case FIELD_ID_BIBLIOGRAPHY:
    case FIELD_ID_DDE:
    case FIELD_ID_REF_REFERENCE:
    case FIELD_ID_REF_SEQUENCE:
    case FIELD_ID_REF_BOOKMARK:
    case FIELD_ID_REF_FOOTNOTE:
    case FIELD_ID_REF_ENDNOTE:
    case FIELD_ID_REF_STYLE:
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
    case FIELD_ID_PAGENAME:
    case FIELD_ID_PAGESTRING:
    case FIELD_ID_SHEET_NAME:
    case FIELD_ID_MEASURE:
    case FIELD_ID_URL:
    case FIELD_ID_DROP_DOWN:
        // always string:
        return true;

    case FIELD_ID_SCRIPT:
    case FIELD_ID_ANNOTATION:
    case FIELD_ID_DATABASE_NEXT:
    case FIELD_ID_DATABASE_SELECT:
    case FIELD_ID_PLACEHOLDER:
    case FIELD_ID_UNKNOWN:
    case FIELD_ID_DRAW_HEADER:
    case FIELD_ID_DRAW_FOOTER:
    case FIELD_ID_DRAW_DATE_TIME:
    default:
        OSL_FAIL("unknown field type/field has no content");
        return true; // invalid info; string in case of doubt
    }
}

/// export the styles needed by the given field. Called on first pass
/// through document
void XMLTextFieldExport::ExportFieldAutoStyle(
    const Reference<XTextField> & rTextField, const bool bProgress,
    const bool bRecursive )
{
    // get property set
    Reference<XPropertySet> xPropSet(rTextField, UNO_QUERY);

    // add field master to list of used field masters (if desired)
    if (moUsedMasters)
    {
        Reference<XDependentTextField> xDepField(rTextField, UNO_QUERY);
        if (xDepField.is())
        {
            // The direct parent may be just the table cell, while we want the topmost parent, e.g.
            // a header text.
            Reference<XText> xOurText = GetToplevelText(rTextField->getAnchor()->getText());

            std::map<Reference<XText>, std::set<OUString> >::iterator aMapIter =
                moUsedMasters->find(xOurText);

            // insert a list for our XText (if necessary)
            if (aMapIter == moUsedMasters->end())
            {
                std::set<OUString> aSet;
                (*moUsedMasters)[xOurText] = aSet;
                aMapIter = moUsedMasters->find(xOurText);
            }

            // insert this text field master
            OUString sFieldMasterName = GetStringProperty(
                gsPropertyInstanceName, xDepField->getTextFieldMaster());
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
            XmlStyleFamily::TEXT_TEXT, xRangePropSet);
    }

    // process special styles for each field (e.g. data styles)
    switch (nToken) {

    case FIELD_ID_DATABASE_DISPLAY:
    {
        sal_Int32 nFormat = GetIntProperty(gsPropertyNumberFormat, xPropSet);
        // workaround: #no-bug#; see IsStringField(...)
        if ( (5100 != nFormat) &&
             !GetBoolProperty(gsPropertyIsDataBaseFormat, xPropSet) )
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
            if ( xPropSetInfo->hasPropertyByName( gsPropertyNumberFormat ) )
            {
                sal_Int32 nFormat =
                    GetIntProperty(gsPropertyNumberFormat, xPropSet);

                // nFormat may be -1 for numeric fields that display their
                //  variable name. (Maybe this should be a field type, then?)
                if (nFormat != -1)
                {
                    if( ! GetOptionalBoolProperty(
                            gsPropertyIsFixedLanguage,
                            xPropSet, xPropSetInfo, false ) )
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
            bool dummy_for_autostyles(true);
            ExportMetaField(xPropSet, true, bProgress, dummy_for_autostyles);
        }
        [[fallthrough]];
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
                GetIntProperty(gsPropertyNumberFormat, xPropSet);

            // nFormat may be -1 for numeric fields that display their
            //  variable name. (Maybe this should be a field type, then?)
            if (nFormat != -1)
            {
                // handle formats for fixed language fields
                // for all these fields (except table formula)
                if( ( nToken != FIELD_ID_TABLE_FORMULA ) &&
                    ! GetOptionalBoolProperty(
                          gsPropertyIsFixedLanguage,
                          xPropSet, xPropSet->getPropertySetInfo(),
                          false ) )
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
        DBG_ASSERT(nullptr != pCombinedCharactersPropertyState,
                   "need proper PropertyState for combined characters");
        std::span<XMLPropertyState> aStates( pCombinedCharactersPropertyState.get(), 1 );
        GetExport().GetTextParagraphExport()->Add(
            XmlStyleFamily::TEXT_TEXT, xRangePropSet,
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
    case FIELD_ID_REF_STYLE:
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
    case FIELD_ID_PAGENAME:
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
        OSL_FAIL("unknown field type!");
        // ignore -> no format for unknown
        break;
    }
}

/// export the given field to XML. Called on second pass through document
void XMLTextFieldExport::ExportField(
    const Reference<XTextField> & rTextField, bool bProgress,
    bool & rPrevCharIsSpace)
{
    // get property set
    Reference<XPropertySet> xPropSet(rTextField, UNO_QUERY);

    // get property set of range (for the attributes)
    Reference <XPropertySet> xRangePropSet(rTextField->getAnchor(), UNO_QUERY);

    // get Field ID
    enum FieldIdEnum nToken = GetFieldID(rTextField, xPropSet);

    // special treatment for combined characters field, because it is
    // exported as a style
    const XMLPropertyState* aStates[] = { pCombinedCharactersPropertyState.get(), nullptr };
    const XMLPropertyState **pStates =
                FIELD_ID_COMBINED_CHARACTERS == nToken
                    ? aStates
                    : nullptr;

    // find out whether we need to set the style
    bool bIsUICharStyle;
    bool bHasAutoStyle;
    OUString sStyle = GetExport().GetTextParagraphExport()->
        FindTextStyle( xRangePropSet, bIsUICharStyle, bHasAutoStyle, pStates );
    bool bHasStyle = !sStyle.isEmpty();

    {
        Reference<XPropertySetInfo> xRangePropSetInfo;
        XMLTextCharStyleNamesElementExport aCharStylesExport(
            GetExport(), bIsUICharStyle &&
                         GetExport().GetTextParagraphExport()
                             ->GetCharStyleNamesPropInfoCache().hasProperty(
                                        xRangePropSet, xRangePropSetInfo ), bHasAutoStyle,
            xRangePropSet, gsPropertyCharStyleNames );

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
                                  false, false);

        // finally, export the field itself
        ExportFieldHelper( rTextField, xPropSet, xRangePropSet, nToken,
            bProgress, rPrevCharIsSpace);
    }
}

/// export the given field to XML. Called on second pass through document
void XMLTextFieldExport::ExportFieldHelper(
    const Reference<XTextField> & rTextField,
    const Reference<XPropertySet> & rPropSet,
    const Reference<XPropertySet> &,
    enum FieldIdEnum nToken,
    bool bProgress,
    bool & rPrevCharIsSpace)
{
    // get property set info (because some attributes are not support
    // in all implementations)
    Reference<XPropertySetInfo> xPropSetInfo(rPropSet->getPropertySetInfo());

    OUString sPresentation = rTextField->getPresentation(false);

    // process each field type
    switch (nToken) {
    case FIELD_ID_AUTHOR:
        // author field: fixed, field (sub-)type
        if (xPropSetInfo->hasPropertyByName(gsPropertyIsFixed))
        {
            GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_FIXED,
                                 (GetBoolProperty(gsPropertyIsFixed, rPropSet) ? XML_TRUE : XML_FALSE) );
        }
        ExportElement(MapAuthorFieldName(rPropSet), sPresentation);
        break;

    case FIELD_ID_SENDER:
        // sender field: fixed, field (sub-)type
        ProcessBoolean(XML_FIXED,
                       GetBoolProperty(gsPropertyIsFixed, rPropSet), true);
        ExportElement(MapSenderFieldName(rPropSet), sPresentation);
        break;

    case FIELD_ID_PLACEHOLDER:
        // placeholder field: type, name, description
        ProcessString(XML_PLACEHOLDER_TYPE,
                      MapPlaceholderType(
                        GetInt16Property(gsPropertyPlaceholderType, rPropSet)));
        ProcessString(XML_DESCRIPTION,
                      GetStringProperty(gsPropertyHint,rPropSet), true);
        ExportElement(XML_PLACEHOLDER, sPresentation);
        break;

    case FIELD_ID_VARIABLE_SET:
    {
        // variable set field: name, visible, format&value
        ProcessString(XML_NAME,
                      GetStringProperty(gsPropertyVariableName, rPropSet));
        ProcessDisplay(GetBoolProperty(gsPropertyIsVisible, rPropSet),
                       false);
        ProcessString(XML_FORMULA, XML_NAMESPACE_OOOW,
                      GetStringProperty(gsPropertyContent, rPropSet),
                      sPresentation);
        ProcessValueAndType(IsStringField(nToken, rPropSet),
                            GetIntProperty(gsPropertyNumberFormat, rPropSet),
                            GetStringProperty(gsPropertyContent, rPropSet),
                            sPresentation,
                            GetDoubleProperty(gsPropertyValue, rPropSet),
                            true, true, true,
                            ! GetOptionalBoolProperty(
                                 gsPropertyIsFixedLanguage,
                                 rPropSet, xPropSetInfo, false ) );
        ExportElement(XML_VARIABLE_SET, sPresentation);
        break;
    }
    case FIELD_ID_VARIABLE_GET:
    {
        // variable get field: name, format&value
        ProcessString(XML_NAME,
                      GetStringProperty(gsPropertyContent, rPropSet));
        bool bCmd = GetBoolProperty(gsPropertyIsShowFormula, rPropSet);
        ProcessDisplay(true, bCmd);
        // show style, unless name will be shown
        ProcessValueAndType(IsStringField(nToken, rPropSet),
                            GetIntProperty(gsPropertyNumberFormat, rPropSet),
                            u""_ustr, u"", 0.0, // values not used
                            false,
                            false,
                            !bCmd,
                            ! GetOptionalBoolProperty(
                                 gsPropertyIsFixedLanguage,
                                 rPropSet, xPropSetInfo, false ) );
        ExportElement(XML_VARIABLE_GET, sPresentation);
        break;
    }
    case FIELD_ID_VARIABLE_INPUT:
        // variable input field: name, description, format&value
        ProcessString(XML_NAME,
                      GetStringProperty(gsPropertyVariableName, rPropSet));
        ProcessString(XML_DESCRIPTION,
                      GetStringProperty(gsPropertyHint , rPropSet));
        ProcessDisplay(GetBoolProperty(gsPropertyIsVisible, rPropSet),
                       false);
        ProcessString(XML_FORMULA, XML_NAMESPACE_OOOW,
                      GetStringProperty(gsPropertyContent, rPropSet),
                      sPresentation);
        ProcessValueAndType(IsStringField(nToken, rPropSet),
                            GetIntProperty(gsPropertyNumberFormat, rPropSet),
                            GetStringProperty(gsPropertyContent, rPropSet),
                            sPresentation,
                            GetDoubleProperty(gsPropertyValue, rPropSet),
                            true, true, true,
                            ! GetOptionalBoolProperty(
                                 gsPropertyIsFixedLanguage,
                                 rPropSet, xPropSetInfo, false ) );
        ExportElement(XML_VARIABLE_INPUT, sPresentation);
        break;

    case FIELD_ID_USER_GET:
        // user field: name, hidden, style
    {
        bool bCmd = GetBoolProperty(gsPropertyIsShowFormula, rPropSet);
        ProcessDisplay(GetBoolProperty(gsPropertyIsVisible, rPropSet),
                       bCmd);
        ProcessValueAndType(IsStringField(nToken, rPropSet),
                            GetIntProperty(gsPropertyNumberFormat, rPropSet),
                            u""_ustr, u"", 0.0, // values not used
                            false, false, !bCmd,
                            ! GetOptionalBoolProperty(
                                 gsPropertyIsFixedLanguage,
                                 rPropSet, xPropSetInfo, false ) );

        // name from FieldMaster
        ProcessString(XML_NAME,
                      GetStringProperty(gsPropertyName,
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
                      GetStringProperty(gsPropertyContent, rPropSet));
        ProcessString(XML_DESCRIPTION,
                      GetStringProperty(gsPropertyHint, rPropSet));
        ExportElement(XML_USER_FIELD_INPUT, sPresentation);
        break;

    case FIELD_ID_SEQUENCE:
    {
        // sequence field: name, formula, seq-format
        OUString sName = GetStringProperty(gsPropertyVariableName, rPropSet);
        // TODO: use reference name only if actually being referenced.
        ProcessString(XML_REF_NAME,
                      MakeSequenceRefName(
                          GetInt16Property(gsPropertySequenceValue, rPropSet),
                          sName));
        ProcessString(XML_NAME, sName);
        ProcessString(XML_FORMULA,  XML_NAMESPACE_OOOW,
                      GetStringProperty(gsPropertyContent, rPropSet),
                      sPresentation);
        ProcessNumberingType(GetInt16Property(gsPropertyNumberingType,
                                              rPropSet));
        ExportElement(XML_SEQUENCE, sPresentation);
        break;
    }

    case FIELD_ID_EXPRESSION:
    {
        // formula field: formula, format&value
        bool bCmd = GetBoolProperty(gsPropertyIsShowFormula, rPropSet);
        ProcessString(XML_FORMULA,  XML_NAMESPACE_OOOW,
                      GetStringProperty(gsPropertyContent, rPropSet),
                      sPresentation);
        ProcessDisplay(true, bCmd);
        ProcessValueAndType(IsStringField(nToken, rPropSet),
                            GetIntProperty(gsPropertyNumberFormat, rPropSet),
                            GetStringProperty(gsPropertyContent, rPropSet),
                            sPresentation,
                            GetDoubleProperty(gsPropertyValue, rPropSet),
                            !bCmd, !bCmd, !bCmd,
                            ! GetOptionalBoolProperty(
                                 gsPropertyIsFixedLanguage,
                                 rPropSet, xPropSetInfo, false ) );
        ExportElement(XML_EXPRESSION, sPresentation);
        break;
    }

    case FIELD_ID_TEXT_INPUT:
        // text input field: description and string-value
        ProcessString(XML_DESCRIPTION,
                      GetStringProperty(gsPropertyHint, rPropSet));
        ProcessString(XML_HELP,
                      GetStringProperty(gsPropertyHelp, rPropSet), true);
        ProcessString(XML_HINT,
                      GetStringProperty(gsPropertyTooltip, rPropSet), true);
        ExportElement(XML_TEXT_INPUT, sPresentation);
        break;

    case FIELD_ID_TIME:
        // all properties (except IsDate) are optional!
        if (xPropSetInfo->hasPropertyByName(gsPropertyNumberFormat))
        {
            ProcessValueAndType(false,
                                GetIntProperty(gsPropertyNumberFormat,rPropSet),
                                u""_ustr, u"", 0.0, // not used
                                false, false, true,
                                ! GetOptionalBoolProperty(
                                    gsPropertyIsFixedLanguage,
                                    rPropSet, xPropSetInfo, false ),
                                true);
        }
        if (xPropSetInfo->hasPropertyByName(gsPropertyDateTimeValue))
        {
            // no value -> current time
            ProcessTimeOrDateTime(XML_TIME_VALUE,
                            GetDateTimeProperty(gsPropertyDateTimeValue,
                                                rPropSet));
        }
        if (xPropSetInfo->hasPropertyByName(gsPropertyDateTime))
        {
            // no value -> current time
            ProcessTimeOrDateTime(XML_TIME_VALUE,
                            GetDateTimeProperty(gsPropertyDateTime,rPropSet));
        }
        if (xPropSetInfo->hasPropertyByName(gsPropertyIsFixed))
        {
            ProcessBoolean(XML_FIXED,
                           GetBoolProperty(gsPropertyIsFixed, rPropSet),
                           false);
        }
        if (xPropSetInfo->hasPropertyByName(gsPropertyAdjust))
        {
            // adjust value given as integer in minutes
            ProcessDateTime(XML_TIME_ADJUST,
                            GetIntProperty(gsPropertyAdjust, rPropSet),
                            false, true);
        }
        ExportElement(XML_TIME, sPresentation);
        break;

    case FIELD_ID_DATE:
        // all properties (except IsDate) are optional!
        if (xPropSetInfo->hasPropertyByName(gsPropertyNumberFormat))
        {
            ProcessValueAndType(false,
                                GetIntProperty(gsPropertyNumberFormat,rPropSet),
                                u""_ustr, u"", 0.0, // not used
                                false, false, true,
                                ! GetOptionalBoolProperty(
                                    gsPropertyIsFixedLanguage,
                                    rPropSet, xPropSetInfo, false ) );
        }
        if (xPropSetInfo->hasPropertyByName(gsPropertyDateTimeValue))
        {
            // no value -> current date
            ProcessDateTime(XML_DATE_VALUE,
                            GetDateTimeProperty(gsPropertyDateTimeValue,
                                                rPropSet));
        }
        // TODO: remove double-handling after SRC614
        else if (xPropSetInfo->hasPropertyByName(gsPropertyDateTime))
        {
            ProcessDateTime(XML_DATE_VALUE,
                            GetDateTimeProperty(gsPropertyDateTime,rPropSet));
        }
        if (xPropSetInfo->hasPropertyByName(gsPropertyIsFixed))
        {
            ProcessBoolean(XML_FIXED,
                           GetBoolProperty(gsPropertyIsFixed, rPropSet),
                           false);
        }
        if (xPropSetInfo->hasPropertyByName(gsPropertyAdjust))
        {
            // adjust value given as number of days
            ProcessDateTime(XML_DATE_ADJUST,
                            GetIntProperty(gsPropertyAdjust, rPropSet),
                            true, true);
        }
        ExportElement(XML_DATE, sPresentation);
        break;

    case FIELD_ID_PAGENUMBER:
        // all properties are optional
        if (xPropSetInfo->hasPropertyByName(gsPropertyNumberingType))
        {
            ProcessNumberingType(GetInt16Property(gsPropertyNumberingType,
                                                  rPropSet));
        }
        if (xPropSetInfo->hasPropertyByName(gsPropertyOffset))
        {
            sal_Int32 nAdjust = GetIntProperty(gsPropertyOffset, rPropSet);

            if (xPropSetInfo->hasPropertyByName(gsPropertySubType))
            {
                // property SubType used in MapPageNumberName
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
                      GetStringProperty(gsPropertyUserText, rPropSet),
                      sPresentation);
        sal_Int32 nDummy = 0; // MapPageNumberName need int
        ProcessString(XML_SELECT_PAGE, MapPageNumberName(rPropSet, nDummy));
        ExportElement(XML_PAGE_CONTINUATION, sPresentation);
        break;
    }

    case FIELD_ID_DATABASE_NAME:
        ProcessString(XML_TABLE_NAME,
                      GetStringProperty(gsPropertyDataTableName, rPropSet));
        ProcessCommandType(GetIntProperty(gsPropertyDataCommandType, rPropSet));
        ProcessDisplay(GetBoolProperty(gsPropertyIsVisible, rPropSet),
                       false);
        ExportDataBaseElement(XML_DATABASE_NAME, sPresentation,
                              rPropSet, xPropSetInfo);
        break;

    case FIELD_ID_DATABASE_NUMBER:
        ProcessString(XML_TABLE_NAME,
                      GetStringProperty(gsPropertyDataTableName, rPropSet));
        ProcessCommandType(GetIntProperty(gsPropertyDataCommandType, rPropSet));
        ProcessNumberingType(
            GetInt16Property(gsPropertyNumberingType,rPropSet));
        ProcessInteger(XML_VALUE,
                       GetIntProperty(gsPropertySetNumber, rPropSet));
        ProcessDisplay(GetBoolProperty(gsPropertyIsVisible, rPropSet),
                       false);
        ExportDataBaseElement(XML_DATABASE_ROW_NUMBER, sPresentation,
                              rPropSet, xPropSetInfo);
        break;

    case FIELD_ID_DATABASE_NEXT:
        ProcessString(XML_TABLE_NAME,
                      GetStringProperty(gsPropertyDataTableName, rPropSet));
        ProcessCommandType(GetIntProperty(gsPropertyDataCommandType, rPropSet));
        ProcessString(XML_CONDITION, XML_NAMESPACE_OOOW,
                      GetStringProperty(gsPropertyCondition, rPropSet));
        DBG_ASSERT(sPresentation.isEmpty(),
                   "Unexpected presentation for database next field");
        ExportDataBaseElement(XML_DATABASE_NEXT, OUString(),
                              rPropSet, xPropSetInfo);
        break;

    case FIELD_ID_DATABASE_SELECT:
        ProcessString(XML_TABLE_NAME,
                      GetStringProperty(gsPropertyDataTableName, rPropSet));
        ProcessCommandType(GetIntProperty(gsPropertyDataCommandType, rPropSet));
        ProcessString(XML_CONDITION, XML_NAMESPACE_OOOW,
                      GetStringProperty(gsPropertyCondition, rPropSet));
        ProcessInteger(XML_ROW_NUMBER,
                       GetIntProperty(gsPropertySetNumber, rPropSet));
        DBG_ASSERT(sPresentation.isEmpty(),
                   "Unexpected presentation for database select field");
        ExportDataBaseElement(XML_DATABASE_ROW_SELECT, OUString(),
                              rPropSet, xPropSetInfo);
        break;

    case FIELD_ID_DATABASE_DISPLAY:
    {
        // get database, table and column name from field master
        const Reference<XPropertySet> & xMaster = GetMasterPropertySet(rTextField);
        ProcessString(XML_TABLE_NAME,
                      GetStringProperty(gsPropertyDataTableName, xMaster));
        ProcessCommandType(GetIntProperty(gsPropertyDataCommandType, xMaster));
        ProcessString(XML_COLUMN_NAME,
                      GetStringProperty(gsPropertyDataColumnName, xMaster));
        // export number format if available (happens only for numbers!)
        if (!GetBoolProperty(gsPropertyIsDataBaseFormat, rPropSet))
        {
            ProcessValueAndType(false,  // doesn't happen for text
                                GetIntProperty(gsPropertyNumberFormat,rPropSet),
                                u""_ustr, u"", 0.0, // not used
                                false, false, true, false);
        }
        ProcessDisplay(GetBoolProperty(gsPropertyIsVisible, rPropSet),
                       false);
        ExportDataBaseElement(XML_DATABASE_DISPLAY, sPresentation,
                              xMaster, xMaster->getPropertySetInfo());
        break;
    }

    case FIELD_ID_DOCINFO_REVISION:
        ProcessBoolean(XML_FIXED,
                       GetBoolProperty(gsPropertyIsFixed, rPropSet), false);
        ExportElement(MapDocInfoFieldName(nToken), sPresentation);
        break;

    case FIELD_ID_DOCINFO_EDIT_DURATION:
    case FIELD_ID_DOCINFO_SAVE_TIME:
    case FIELD_ID_DOCINFO_CREATION_TIME:
    case FIELD_ID_DOCINFO_PRINT_TIME:
    case FIELD_ID_DOCINFO_SAVE_DATE:
    case FIELD_ID_DOCINFO_CREATION_DATE:
    case FIELD_ID_DOCINFO_PRINT_DATE:
        ProcessValueAndType(false,
                            GetIntProperty(gsPropertyNumberFormat, rPropSet),
                            u""_ustr, u"", 0.0,
                            false, false, true,
                            ! GetOptionalBoolProperty(
                                    gsPropertyIsFixedLanguage,
                                    rPropSet, xPropSetInfo, false ) );

        // todo: export date/time value, but values not available -> core bug
        ProcessBoolean(XML_FIXED,
                       GetBoolProperty(gsPropertyIsFixed, rPropSet), false);
        ExportElement(MapDocInfoFieldName(nToken), sPresentation);
        break;

    case FIELD_ID_DOCINFO_CREATION_AUTHOR:
    case FIELD_ID_DOCINFO_DESCRIPTION:
    case FIELD_ID_DOCINFO_PRINT_AUTHOR:
    case FIELD_ID_DOCINFO_TITLE:
    case FIELD_ID_DOCINFO_SUBJECT:
    case FIELD_ID_DOCINFO_KEYWORDS:
    case FIELD_ID_DOCINFO_SAVE_AUTHOR:
        if (xPropSetInfo->hasPropertyByName(gsPropertyIsFixed))
        {
            ProcessBoolean(XML_FIXED,
                           GetBoolProperty(gsPropertyIsFixed, rPropSet), false);
        }
        ExportElement(MapDocInfoFieldName(nToken), sPresentation);
        break;

    case FIELD_ID_DOCINFO_CUSTOM:
    {
        ProcessValueAndType(false,  // doesn't happen for text
                                GetIntProperty(gsPropertyNumberFormat,rPropSet),
                                u""_ustr, u"", 0.0, // not used
                                false, false, true,
                                ! GetOptionalBoolProperty(
                                    gsPropertyIsFixedLanguage,
                                    rPropSet, xPropSetInfo, false ));
        uno::Any aAny = rPropSet->getPropertyValue( gsPropertyName );
        OUString sName;
        aAny >>= sName;
        ProcessString(XML_NAME, sName);
        ProcessBoolean(XML_FIXED, GetBoolProperty(gsPropertyIsFixed, rPropSet), false);
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
        if (xPropSetInfo->hasPropertyByName(gsPropertyNumberingType))
        {
            ProcessNumberingType(GetInt16Property(gsPropertyNumberingType,
                                                  rPropSet));
        }
        ExportElement(MapCountFieldName(nToken), sPresentation);
        break;

    case FIELD_ID_CONDITIONAL_TEXT:
        ProcessString(XML_CONDITION, XML_NAMESPACE_OOOW,
                      GetStringProperty(gsPropertyCondition, rPropSet));
        ProcessString(XML_STRING_VALUE_IF_TRUE,
                      GetStringProperty(gsPropertyTrueContent, rPropSet));
        ProcessString(XML_STRING_VALUE_IF_FALSE,
                      GetStringProperty(gsPropertyFalseContent, rPropSet));
        ProcessBoolean(XML_CURRENT_VALUE,
                       GetBoolProperty(gsPropertyIsConditionTrue, rPropSet),
                       false);
        ExportElement(XML_CONDITIONAL_TEXT, sPresentation);
        break;

    case FIELD_ID_HIDDEN_TEXT:
        ProcessString(XML_CONDITION, XML_NAMESPACE_OOOW,
                      GetStringProperty(gsPropertyCondition, rPropSet));
        ProcessString(XML_STRING_VALUE,
                      GetStringProperty(gsPropertyContent, rPropSet));
        ProcessBoolean(XML_IS_HIDDEN,
                       GetBoolProperty(gsPropertyIsHidden, rPropSet),
                       false);
        ExportElement(XML_HIDDEN_TEXT, sPresentation);
        break;

    case FIELD_ID_HIDDEN_PARAGRAPH:
        ProcessString(XML_CONDITION, XML_NAMESPACE_OOOW,
                      GetStringProperty(gsPropertyCondition, rPropSet));
        ProcessBoolean(XML_IS_HIDDEN,
                       GetBoolProperty(gsPropertyIsHidden, rPropSet),
                       false);
        DBG_ASSERT(sPresentation.isEmpty(),
                   "Unexpected presentation for hidden paragraph field");
        ExportElement(XML_HIDDEN_PARAGRAPH);
        break;

    case FIELD_ID_TEMPLATE_NAME:
        ProcessString(XML_DISPLAY,
                      MapTemplateDisplayFormat(
                          GetInt16Property(gsPropertyFileFormat, rPropSet)));
        ExportElement(XML_TEMPLATE_NAME, sPresentation);
        break;

    case FIELD_ID_CHAPTER:
        ProcessString(XML_DISPLAY,
                      MapChapterDisplayFormat(
                          GetInt16Property(gsPropertyChapterFormat, rPropSet)));
        // API numbers 0..9, we number 1..10
        ProcessInteger(XML_OUTLINE_LEVEL,
                       GetInt8Property(gsPropertyLevel, rPropSet) + 1);
        ExportElement(XML_CHAPTER, sPresentation);
        break;

    case FIELD_ID_FILE_NAME:
        // all properties are optional
        if (xPropSetInfo->hasPropertyByName(gsPropertyFileFormat))
        {
            ProcessString(XML_DISPLAY,
                          MapFilenameDisplayFormat(
                             GetInt16Property(gsPropertyFileFormat, rPropSet)));
        }
        if (xPropSetInfo->hasPropertyByName(gsPropertyIsFixed))
        {
            ProcessBoolean(XML_FIXED,
                           GetBoolProperty(gsPropertyIsFixed, rPropSet),
                           false);
        }
        ExportElement(XML_FILE_NAME, sPresentation);
        break;

    case FIELD_ID_REFPAGE_SET:
        ProcessBoolean(XML_ACTIVE,
                       GetBoolProperty(gsPropertyOn, rPropSet), true);
        ProcessIntegerDef(XML_PAGE_ADJUST,
                       GetInt16Property(gsPropertyOffset, rPropSet), 0);
        DBG_ASSERT(sPresentation.isEmpty(),
                   "Unexpected presentation page variable field");
        ExportElement(XML_PAGE_VARIABLE_SET);
        break;

    case FIELD_ID_REFPAGE_GET:
        ProcessNumberingType(
            GetInt16Property(gsPropertyNumberingType, rPropSet));
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
                          gsPropertyReferenceFieldPart, rPropSet)),
                      XML_TEMPLATE);
        ProcessString(XML_REF_NAME,
                      MakeSequenceRefName(
                          GetInt16Property(gsPropertySequenceNumber, rPropSet),
                          GetStringProperty(gsPropertySourceName, rPropSet) ) );
        if (xPropSetInfo->hasPropertyByName(gsPropertyReferenceFieldLanguage) &&
            GetExport().getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
        {
            // export text:reference-language attribute, if not empty
            ProcessString(XML_REFERENCE_LANGUAGE,
                    GetStringProperty(gsPropertyReferenceFieldLanguage, rPropSet), true, XML_NAMESPACE_LO_EXT);
        }
        ExportElement(
            MapReferenceSource(
                GetInt16Property(gsPropertyReferenceFieldSource, rPropSet)),
            sPresentation);
        break;

    case FIELD_ID_REF_REFERENCE:
    case FIELD_ID_REF_BOOKMARK:
        // reference to bookmarks, references: format, name (and element)
        ProcessString(XML_REFERENCE_FORMAT,
                      MapReferenceType(GetInt16Property(
                          gsPropertyReferenceFieldPart, rPropSet)),
                      XML_TEMPLATE);
        ProcessString(XML_REF_NAME,
                      GetStringProperty(gsPropertySourceName, rPropSet));
        if (xPropSetInfo->hasPropertyByName(gsPropertyReferenceFieldLanguage) &&
            GetExport().getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
        {
            // export text:reference-language attribute, if not empty
            ProcessString(XML_REFERENCE_LANGUAGE,
                      GetStringProperty(gsPropertyReferenceFieldLanguage, rPropSet), true, XML_NAMESPACE_LO_EXT);
        }
        ExportElement(
            MapReferenceSource(GetInt16Property(
                gsPropertyReferenceFieldSource, rPropSet)),
            sPresentation);
        break;

    case FIELD_ID_REF_FOOTNOTE:
    case FIELD_ID_REF_ENDNOTE:
        // reference to end-/footnote: format, generate name, (and element)
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_NOTE_CLASS,
            FIELD_ID_REF_ENDNOTE==nToken ? XML_ENDNOTE : XML_FOOTNOTE );
        ProcessString(XML_REFERENCE_FORMAT,
                      MapReferenceType(GetInt16Property(
                          gsPropertyReferenceFieldPart, rPropSet)),
                      XML_TEMPLATE);
        ProcessString(XML_REF_NAME,
                      MakeFootnoteRefName(GetInt16Property(
                          gsPropertySequenceNumber, rPropSet)));
        if (xPropSetInfo->hasPropertyByName(gsPropertyReferenceFieldLanguage) &&
            GetExport().getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
        {
            // export text:reference-language attribute, if not empty
            ProcessString(XML_REFERENCE_LANGUAGE,
                      GetStringProperty(gsPropertyReferenceFieldLanguage, rPropSet), true, XML_NAMESPACE_LO_EXT);
        }
        ExportElement(
            MapReferenceSource(GetInt16Property(
                gsPropertyReferenceFieldSource, rPropSet)),
            sPresentation);
        break;

    case FIELD_ID_REF_STYLE:
    {
        ProcessString(XML_REFERENCE_FORMAT,
                      MapReferenceType(GetInt16Property(gsPropertyReferenceFieldPart, rPropSet)),
                      XML_TEMPLATE);
        ProcessString(XML_REF_NAME, GetStringProperty(gsPropertySourceName, rPropSet));

        sal_uInt16 referenceFieldFlags = GetIntProperty(gsPropertyReferenceFieldFlags, rPropSet);
        // In reality gsPropertyReferenceFieldFlags is a uInt16... but there is no GetUInt16Property

        bool fromBottom = (referenceFieldFlags & REFFLDFLAG_STYLE_FROM_BOTTOM) == REFFLDFLAG_STYLE_FROM_BOTTOM;
        bool hideNonNumerical = (referenceFieldFlags & REFFLDFLAG_STYLE_HIDE_NON_NUMERICAL) == REFFLDFLAG_STYLE_HIDE_NON_NUMERICAL;

        ProcessBoolean(XML_REFERENCE_FROM_BOTTOM, fromBottom,  false, XML_NAMESPACE_LO_EXT);
        ProcessBoolean(XML_REFERENCE_HIDE_NON_NUMERICAL, hideNonNumerical,  false, XML_NAMESPACE_LO_EXT);

        if (xPropSetInfo->hasPropertyByName(gsPropertyReferenceFieldLanguage)
            && GetExport().getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
        {
            // export text:reference-language attribute, if not empty
            ProcessString(XML_REFERENCE_LANGUAGE,
                          GetStringProperty(gsPropertyReferenceFieldLanguage, rPropSet), true,
                          XML_NAMESPACE_LO_EXT);
        }
        SvXMLElementExport aElem(
            GetExport(),
            XML_NAMESPACE_LO_EXT,
            MapReferenceSource(GetInt16Property(gsPropertyReferenceFieldSource, rPropSet)),
            false,
            false);
        GetExport().Characters(sPresentation);
        break;
    }

    case FIELD_ID_DDE:
        // name from field master
         ProcessString(XML_CONNECTION_NAME,

                       GetStringProperty(gsPropertyName,
                                         GetMasterPropertySet(rTextField)));
        ExportElement(XML_DDE_CONNECTION, sPresentation);
        break;

    case FIELD_ID_SHEET_NAME:
        // name of spreadsheet (Calc only)
        ExportElement(XML_SHEET_NAME, sPresentation);
        break;

    case FIELD_ID_PAGENAME:
    {
        if (GetExport().getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
        {
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_LO_EXT, XML_PAGE_NAME, false, false );
            GetExport().Characters( sPresentation );
        }
        break;
    }

    case FIELD_ID_URL:
    {
        // this field is a special case because it gets mapped onto a
        // hyperlink, rather than one of the regular text field.
        ProcessString(XML_HREF, GetExport().GetRelativeReference(GetStringProperty(gsPropertyURL, rPropSet)),
                      false, XML_NAMESPACE_XLINK);
        ProcessString(XML_TARGET_FRAME_NAME,
                      GetStringProperty(gsPropertyTargetFrame,rPropSet),
                      true, XML_NAMESPACE_OFFICE);
        GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        SvXMLElementExport aUrlField(rExport, XML_NAMESPACE_TEXT, XML_A,
                                     false, false);
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
                      GetStringProperty(gsPropertyScriptType, rPropSet),
                      true, XML_NAMESPACE_SCRIPT);
        DBG_ASSERT(sPresentation.isEmpty(),
                   "Unexpected presentation for script field");
        if (GetBoolProperty(gsPropertyURLContent, rPropSet))
        {
            ProcessString(XML_HREF,
                          GetExport().GetRelativeReference(GetStringProperty(gsPropertyContent, rPropSet)),
                          false, XML_NAMESPACE_XLINK);
            ExportElement(XML_SCRIPT);
        }
        else
        {
            ExportElement(XML_SCRIPT,
                          GetStringProperty(gsPropertyContent, rPropSet));
        }
        break;

    case FIELD_ID_ANNOTATION:
    {
        // check for empty presentation (just in case)
        DBG_ASSERT(sPresentation.isEmpty(),
                   "Unexpected presentation for annotation field");

        bool bRemovePersonalInfo = SvtSecurityOptions::IsOptionSet(
            SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo ) && !SvtSecurityOptions::IsOptionSet(
                SvtSecurityOptions::EOption::DocWarnKeepNoteAuthorDateInfo);

        // annotation element + content
        OUString aName;
        rPropSet->getPropertyValue(gsPropertyName) >>= aName;
        if (!aName.isEmpty())
        {
            GetExport().AddAttribute(XML_NAMESPACE_OFFICE, XML_NAME, aName);
        }

        OUString aParentName;
        rPropSet->getPropertyValue(gsPropertyParentName) >>= aParentName;
        if (!aParentName.isEmpty())
        {
            GetExport().AddAttribute(XML_NAMESPACE_LO_EXT, XML_PARENT_NAME, aParentName);
        }

        SvtSaveOptions::ODFSaneDefaultVersion eVersion = rExport.getSaneDefaultVersion();
        if (eVersion & SvtSaveOptions::ODFSVER_EXTENDED)
        {
            bool b = GetBoolProperty(u"Resolved"_ustr, rPropSet);
            OUString aResolvedText;
            OUStringBuffer aResolvedTextBuffer;
            ::sax::Converter::convertBool(aResolvedTextBuffer, b);
            aResolvedText = aResolvedTextBuffer.makeStringAndClear();

            GetExport().AddAttribute(XML_NAMESPACE_LO_EXT, XML_RESOLVED, aResolvedText);
        }
        SvXMLElementExport aElem(GetExport(), XML_NAMESPACE_OFFICE,
                                 XML_ANNOTATION, false, true);

        // author
        OUString aAuthor( GetStringProperty(gsPropertyAuthor, rPropSet) );
        if( !aAuthor.isEmpty() )
        {
            SvXMLElementExport aCreatorElem( GetExport(), XML_NAMESPACE_DC,
                                              XML_CREATOR, true,
                                              false );
            GetExport().Characters( bRemovePersonalInfo
                    ? "Author" + OUString::number( rExport.GetInfoID(aAuthor) )
                    : aAuthor );
        }

        // date time
        util::DateTime aDate( GetDateTimeProperty(gsPropertyDateTimeValue, rPropSet) );
        if ( !bRemovePersonalInfo )
        {
            OUStringBuffer aBuffer;
            ::sax::Converter::convertDateTime(aBuffer, aDate, nullptr, true);
            SvXMLElementExport aDateElem( GetExport(), XML_NAMESPACE_DC,
                                              XML_DATE, true,
                                              false );
            GetExport().Characters(aBuffer.makeStringAndClear());
        }

        if (GetExport().getSaneDefaultVersion() > SvtSaveOptions::ODFSVER_012)
        {
            // initials
            OUString aInitials( GetStringProperty(gsPropertyInitials, rPropSet) );
            if( !aInitials.isEmpty() )
            {
                // ODF 1.3 OFFICE-3776 export meta:creator-initials for ODF 1.3
                SvXMLElementExport aCreatorElem( GetExport(),
                        (SvtSaveOptions::ODFSVER_013 <= GetExport().getSaneDefaultVersion())
                            ? XML_NAMESPACE_META
                            : XML_NAMESPACE_LO_EXT,

                        (SvtSaveOptions::ODFSVER_013 <= GetExport().getSaneDefaultVersion())
                            ? XML_CREATOR_INITIALS
                            : XML_SENDER_INITIALS,
                        true, false );
                GetExport().Characters( bRemovePersonalInfo
                        ? OUString::number( rExport.GetInfoID(aInitials) )
                        : aInitials);
            }
        }

        css::uno::Reference < css::text::XText > xText;
        try
        {
            css::uno::Any aRet = rPropSet->getPropertyValue(gsPropertyTextRange);
            aRet >>= xText;
        }
        catch ( css::uno::Exception& )
        {}

        if ( xText.is() )
            GetExport().GetTextParagraphExport()->exportText( xText );
        else
            ProcessParagraphSequence(GetStringProperty(gsPropertyContent,rPropSet));
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
        ExportMetaField(rPropSet, false, bProgress, rPrevCharIsSpace);
        break;
    }

    case FIELD_ID_MEASURE:
    {
        ProcessString(XML_KIND, MapMeasureKind(GetInt16Property(gsPropertyMeasureKind, rPropSet)));
        ExportElement( XML_MEASURE, sPresentation );
        break;
    }

    case FIELD_ID_TABLE_FORMULA:
        ProcessString( XML_FORMULA,  XML_NAMESPACE_OOOW,
                       GetStringProperty(gsPropertyContent, rPropSet) );
        ProcessDisplay( true,
                        GetBoolProperty(gsPropertyIsShowFormula, rPropSet) );
        ProcessValueAndType( false,
                             GetIntProperty(gsPropertyNumberFormat, rPropSet),
                             u""_ustr, u"", 0.0f,
                             false, false, true,
                             false );
        ExportElement( XML_TABLE_FORMULA, sPresentation );
        break;

    case FIELD_ID_DROP_DOWN:
    {
        // tdf#133555 don't export in strict ODF versions that don't have it
        if (GetExport().getSaneDefaultVersion() <= SvtSaveOptions::ODFSVER_012)
        {
            break;
        }
        ProcessString(XML_NAME, GetStringProperty(gsPropertyName, rPropSet));
        ProcessString(XML_HELP,
                      GetStringProperty(gsPropertyHelp, rPropSet), true);
        ProcessString(XML_HINT,
                      GetStringProperty(gsPropertyTooltip, rPropSet), true);
        SvXMLElementExport aElem( GetExport(),
                                  XML_NAMESPACE_TEXT, XML_DROP_DOWN,
                                  false, false );
        ProcessStringSequence
            (GetStringSequenceProperty( gsPropertyItems, rPropSet ),
             GetStringProperty( gsPropertySelectedItem, rPropSet ) );

        GetExport().Characters( sPresentation );
    }
    break;

    case FIELD_ID_DRAW_HEADER:
    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_PRESENTATION, XML_HEADER, false, false );
    }
    break;

    case FIELD_ID_DRAW_FOOTER:
    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_PRESENTATION, XML_FOOTER, false, false );
    }
    break;

    case FIELD_ID_DRAW_DATE_TIME:
    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_PRESENTATION, XML_DATE_TIME, false, false );
    }
    break;


    case FIELD_ID_UNKNOWN:
    default:
        OSL_FAIL("unknown field type encountered!");
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
    std::vector<OUString>                    aVarName;
    std::vector<OUString>                    aUserName;
    std::vector<OUString>                    aSeqName;
    std::vector<OUString>                    aDdeName;

    // get text fields supplier and field master name access
    Reference<XTextFieldsSupplier> xTextFieldsSupp(GetExport().GetModel(),
                                                   UNO_QUERY);
    if( !xTextFieldsSupp.is() )
        return;

    Reference<container::XNameAccess> xFieldMasterNameAccess =
        xTextFieldsSupp->getTextFieldMasters();

    // where to get the text field masters from?
    // a) we get a specific XText: then use pUsedMasters
    // b) the XText is empty: then export all text fields
    Sequence<OUString> aFieldMasters;
    if (rText.is())
    {
        // export only used masters
        DBG_ASSERT(moUsedMasters.has_value(),
                   "field masters must be recorded in order to be "
                   "written out separately" );
        if (moUsedMasters)
        {
            std::map<Reference<XText>, std::set<OUString> > ::iterator aMapIter =
                moUsedMasters->find(rText);
            if (aMapIter != moUsedMasters->end())
            {
                // found the set of used field masters
                aFieldMasters = comphelper::containerToSequence(aMapIter->second);
                moUsedMasters->erase(rText);
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

    for(const OUString& sFieldMaster : aFieldMasters) {

        // workaround for #no-bug#
        if ( sFieldMaster.startsWithIgnoreAsciiCase(
                 "com.sun.star.text.FieldMaster.DataBase.") )
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
        if (sFieldMasterType == FIELD_SERVICE_SETEXP)
        {
            sal_Int32 nType = GetIntProperty(gsPropertySubType, xPropSet);

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
        else if (sFieldMasterType == FIELD_SERVICE_USER)
        {
            aUserName.push_back( sFieldMaster );
        }
        else if (sFieldMasterType == FIELD_SERVICE_DDE)
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
                                  true, true );

        for (const auto& sName : aVarName)
        {
            // get field master property set
            Reference<XPropertySet> xPropSet;
            Any aAny = xFieldMasterNameAccess->getByName(sName);
            aAny >>= xPropSet;

            // field name and type
            OUString sFieldMasterType;
            OUString sVarName;
            ExplodeFieldMasterName(sName, sFieldMasterType, sVarName);

            // determine string/numeric field
            bool bIsString = ( GetIntProperty(gsPropertySubType, xPropSet)
                                   == SetVariableType::STRING );

            // get dependent field property set
            Reference<XPropertySet> xFieldPropSet;
            if (GetDependentFieldPropertySet(xPropSet, xFieldPropSet))
            {
                // process value and type.
                ProcessValueAndType(
                    bIsString,
                    GetIntProperty(gsPropertyNumberFormat, xFieldPropSet),
                    u""_ustr, u"", 0.0,
                    false, true, false, false);
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
                    0, u""_ustr, u"", 0.0,
                    false, true, false, false);
            }

            ProcessString(XML_NAME, sVarName);
            ExportElement(XML_VARIABLE_DECL, true);
        }
    }
    // else: no declarations element

    // sequence field masters:
    if ( !aSeqName.empty() )
    {
        SvXMLElementExport aElem( GetExport(),
                                  XML_NAMESPACE_TEXT,
                                  XML_SEQUENCE_DECLS,
                                  true, true );

        for (const auto& sName : aSeqName)
        {
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
                gsPropertyChapterNumberingLevel, xPropSet);
            DBG_ASSERT(nLevel >= 0, "illegal outline level");
            DBG_ASSERT(nLevel < 127, "possible illegal outline level");
            ProcessInteger(XML_DISPLAY_OUTLINE_LEVEL, nLevel);

            // separation character
            if (nLevel > 0) {
                ProcessString(XML_SEPARATION_CHARACTER, GetStringProperty(
                    gsPropertyNumberingSeparator, xPropSet));
            }
            ProcessString(XML_NAME, sVarName);
            ExportElement(XML_SEQUENCE_DECL, true);
        }
    }
    // else: no declarations element

    // user field masters:
    if ( !aUserName.empty() )
    {
        SvXMLElementExport aElem( GetExport(),
                                  XML_NAMESPACE_TEXT,
                                  XML_USER_FIELD_DECLS,
                                  true, true );

        for (const auto& sName : aUserName)
        {
            // get field master property set
            Reference<XPropertySet> xPropSet;
            Any aAny = xFieldMasterNameAccess->getByName(sName);
            aAny >>= xPropSet;

            // field name and type
            OUString sFieldMasterType;
            OUString sVarName;
            ExplodeFieldMasterName(sName, sFieldMasterType, sVarName);

            if (GetBoolProperty(gsPropertyIsExpression, xPropSet))
            {
                // expression:
                ProcessValueAndType(
                    false,
                    0, u""_ustr, u"",
                    GetDoubleProperty(gsPropertyValue, xPropSet),
                    true,
                    true,
                    false,
                    false);
            }
            else
            {
                // string: write regardless of default
                ProcessString(XML_VALUE_TYPE, XML_STRING,
                              XML_NAMESPACE_OFFICE);
                ProcessString(XML_STRING_VALUE,
                              GetStringProperty(gsPropertyContent, xPropSet),
                              false, XML_NAMESPACE_OFFICE );
            }
            ProcessString(XML_NAME, sVarName);
            ExportElement(XML_USER_FIELD_DECL, true);
        }
    }
    // else: no declarations element

    // DDE field masters:
    if ( aDdeName.empty() )
        return;

    SvXMLElementExport aElem( GetExport(),
                              XML_NAMESPACE_TEXT,
                              XML_DDE_CONNECTION_DECLS,
                              true, true );

    for (const auto& sName : aDdeName)
    {
        // get field master property set
        Reference<XPropertySet> xPropSet;
        Any aAny = xFieldMasterNameAccess->getByName(sName);
        aAny >>= xPropSet;

        // check if this connection is being used by a field
        Reference<XPropertySet> xDummy;
        if (GetDependentFieldPropertySet(xPropSet, xDummy))
        {

            ProcessString(XML_NAME,
                          GetStringProperty(gsPropertyName, xPropSet),
                          false, XML_NAMESPACE_OFFICE);

            // export elements; can't use ProcessString because
            // elements are in office namespace
            ProcessString(XML_DDE_APPLICATION,
                          GetStringProperty(gsPropertyDDECommandType,
                                            xPropSet),
                          false, XML_NAMESPACE_OFFICE);
            ProcessString(XML_DDE_TOPIC,
                          GetStringProperty(gsPropertyDDECommandFile,
                                            xPropSet),
                          false, XML_NAMESPACE_OFFICE);
            ProcessString(XML_DDE_ITEM,
                          GetStringProperty(gsPropertyDDECommandElement,
                                            xPropSet),
                          false, XML_NAMESPACE_OFFICE);
            bool bIsAutomaticUpdate = GetBoolProperty(
                gsPropertyIsAutomaticUpdate, xPropSet);
            if (bIsAutomaticUpdate)
            {
                GetExport().AddAttribute(XML_NAMESPACE_OFFICE,
                                         XML_AUTOMATIC_UPDATE,
                                         XML_TRUE);
            }

            ExportElement(XML_DDE_CONNECTION_DECL, true);
        }
        // else: no dependent field -> no export of field declaration
    }
    // else: no declarations element
}

void XMLTextFieldExport::SetExportOnlyUsedFieldDeclarations(
    bool bExportOnlyUsed)
{
    moUsedMasters.reset();

    // create used masters set (if none is used)
    if (bExportOnlyUsed)
#if defined __GNUC__ && !defined __clang__ && __GNUC__ == 14
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        moUsedMasters.emplace();
#if defined __GNUC__ && !defined __clang__ && __GNUC__ == 14
#pragma GCC diagnostic pop
#endif
}

void XMLTextFieldExport::ExportElement(enum XMLTokenEnum eElementName,
                                       bool bAddSpace)
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
                                       const OUString& sContent)
{
    DBG_ASSERT(eElementName != XML_TOKEN_INVALID, "invalid element name!");
    if (eElementName != XML_TOKEN_INVALID)
    {
        // Element
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                eElementName, false, false );
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
    OUString sEventType( u"EventType"_ustr );


    // the description attribute
    ProcessString(XML_DESCRIPTION,
                  GetStringProperty(gsPropertyHint, rPropSet),
                  rContent);

    // the element
    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                              XML_EXECUTE_MACRO, false, false );

    // the <office:events>-macro:

    // 1) build sequence of PropertyValues
    Sequence<PropertyValue> aSeq;
    OUString sName;
    rPropSet->getPropertyValue(u"ScriptURL"_ustr) >>= sName;

    // if the ScriptURL property is not empty then this is a Scripting
    // Framework URL, otherwise treat it as a Basic Macro
    if (!sName.isEmpty())
    {
        OUString sScript( u"Script"_ustr );
        aSeq = Sequence<PropertyValue>
        {
            comphelper::makePropertyValue(sEventType, sScript),
            comphelper::makePropertyValue(sScript, sName)
        };
    }
    else
    {
        aSeq = Sequence<PropertyValue>
        {
            comphelper::makePropertyValue(sEventType, u"StarBasic"_ustr),
            comphelper::makePropertyValue(u"Library"_ustr, rPropSet->getPropertyValue( u"MacroLibrary"_ustr )),
            comphelper::makePropertyValue(u"MacroName"_ustr, rPropSet->getPropertyValue( u"MacroName"_ustr ))
        };
    }

    // 2) export the sequence
    GetExport().GetEventExport().ExportSingleEvent( aSeq, u"OnClick"_ustr, false );

    // and finally, the field presentation
    GetExport().Characters(rContent);
}

void XMLTextFieldExport::ExportMetaField(
    const Reference<XPropertySet> & i_xMeta,
    bool i_bAutoStyles, bool i_bProgress,
    bool & rPrevCharIsSpace)
{
    bool doExport(!i_bAutoStyles); // do not export element if autostyles
    // check version >= 1.2
    switch (GetExport().getSaneDefaultVersion()) {
        case SvtSaveOptions::ODFSVER_011: // fall through
        case SvtSaveOptions::ODFSVER_010: doExport = false; break;
        default: break;
    }

    const Reference < XEnumerationAccess > xEA( i_xMeta, UNO_QUERY_THROW );
    const Reference < XEnumeration > xTextEnum( xEA->createEnumeration() );

    if (doExport)
    {
        const Reference<rdf::XMetadatable> xMeta( i_xMeta, UNO_QUERY_THROW );

        // style:data-style-name
        ProcessValueAndType(false,
            GetIntProperty(gsPropertyNumberFormat, i_xMeta),
            u""_ustr, u"", 0.0, false, false, true,
            false  );

        // text:meta-field without xml:id is invalid
        xMeta->ensureMetadataReference();

        // xml:id for RDF metadata
        GetExport().AddAttributeXmlId(xMeta);
    }

    SvXMLElementExport aElem( GetExport(), doExport,
        XML_NAMESPACE_TEXT, XML_META_FIELD, false, false );

    // recurse to export content
    GetExport().GetTextParagraphExport()->
        exportTextRangeEnumeration(xTextEnum, i_bAutoStyles, i_bProgress, rPrevCharIsSpace);
}

/// export all data-style related attributes
void XMLTextFieldExport::ProcessValueAndType(
    bool bIsString,     /// do we process a string or a number?
    sal_Int32 nFormatKey,   /// format key for NumberFormatter; inv. if string
    const OUString& sContent,   /// string content; possibly invalid
    std::u16string_view sDefault, /// default string
    double fValue,          /// float content; possibly invalid
    bool bExportValue,  /// export value attribute?
    bool bExportValueType,  /// export value-type attribute?
    bool bExportStyle,  /// export style-attribute?
    bool bForceSystemLanguage, /// export language attributes?
    bool bTimeStyle)    // exporting a time style?
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
void XMLTextFieldExport::ProcessDisplay(bool bIsVisible,
                                        bool bIsCommand)
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
    if (eValue != XML_VALUE)
    {
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, XML_DISPLAY, eValue);
    }
}


void XMLTextFieldExport::ProcessBoolean(enum XMLTokenEnum eName,
                                        bool bBool, bool bDefault)
{
    ProcessBoolean(eName, bBool, bDefault, XML_NAMESPACE_TEXT);
}

/// export boolean property
void XMLTextFieldExport::ProcessBoolean(enum XMLTokenEnum eName,
                                        bool bBool, bool bDefault, sal_uInt16 nPrefix)
{
    SAL_WARN_IF( eName == XML_TOKEN_INVALID, "xmloff.text", "invalid element token");
    if ( XML_TOKEN_INVALID == eName )
        return;

    // write attribute (if different than default)
    // negate to force 0/1 values (and make sal_Bool comparable)
    if ((!bBool) != (!bDefault)) {
        GetExport().AddAttribute(nPrefix, eName,
                                 (bBool ? XML_TRUE : XML_FALSE) );
    }
}

/// export string attribute
void XMLTextFieldExport::ProcessString(enum XMLTokenEnum eName,
                                       const OUString& sValue,
                                       bool bOmitEmpty,
                                       sal_uInt16 nPrefix)
{
    SAL_WARN_IF( eName == XML_TOKEN_INVALID, "xmloff.text", "invalid element token");
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
                                       const OUString& sValue)
{
    OUString sQValue =
        GetExport().GetNamespaceMap().GetQNameByKey( nValuePrefix, sValue, false );
    ProcessString( eName, sQValue );
}

/// export a string attribute
void XMLTextFieldExport::ProcessString(enum XMLTokenEnum eName,
                                       const OUString& sValue,
                                       std::u16string_view sDefault)
{
    if (sValue != sDefault)
    {
        ProcessString(eName, sValue);
    }
}

/// export a string attribute
void XMLTextFieldExport::ProcessString(enum XMLTokenEnum eName,
                                       sal_uInt16 nValuePrefix,
                                       const OUString& sValue,
                                       std::u16string_view sDefault)
{
    if (sValue != sDefault)
    {
        ProcessString(eName, nValuePrefix, sValue);
    }
}


/// export string attribute
void XMLTextFieldExport::ProcessString(
    enum XMLTokenEnum eName,
    enum XMLTokenEnum eValue,
    sal_uInt16 nPrefix)
{
    SAL_WARN_IF( eName == XML_TOKEN_INVALID, "xmloff.text", "invalid element token" );
    SAL_WARN_IF( eValue == XML_TOKEN_INVALID, "xmloff.text", "invalid value token" );
    if ( XML_TOKEN_INVALID == eName )
        return;

    GetExport().AddAttribute(nPrefix, eName, eValue);
}

/// export a string attribute
void XMLTextFieldExport::ProcessString(
    enum XMLTokenEnum eName,
    enum XMLTokenEnum eValue,
    enum XMLTokenEnum eDefault)
{
    if ( eValue != eDefault )
        ProcessString( eName, eValue);
}


/// export a string as a sequence of paragraphs
void XMLTextFieldExport::ProcessParagraphSequence(
    std::u16string_view sParagraphSequence)
{
    // iterate over all string-pieces separated by return (0x0a) and
    // put each inside a paragraph element.
    SvXMLTokenEnumerator aEnumerator(sParagraphSequence, char(0x0a));
    std::u16string_view aSubString;
    while (aEnumerator.getNextToken(aSubString))
    {
        SvXMLElementExport aParagraph(
            GetExport(), XML_NAMESPACE_TEXT, XML_P, true, false);
        GetExport().Characters(OUString(aSubString));
    }
}

// export an integer attribute
void XMLTextFieldExport::ProcessInteger(enum XMLTokenEnum eName,
                                        sal_Int32 nNum)
{
    SAL_WARN_IF( eName == XML_TOKEN_INVALID, "xmloff.text", "invalid element token");
    if ( XML_TOKEN_INVALID == eName )
        return;

    GetExport().AddAttribute(XML_NAMESPACE_TEXT, eName,
                             OUString::number(nNum));
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
    if (NumberingType::PAGE_DESCRIPTOR == nNumberingType)
        return;

    OUStringBuffer sTmp( 10 );
    // number type: num format
    GetExport().GetMM100UnitConverter().convertNumFormat( sTmp,
                                                          nNumberingType );
    GetExport().AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_FORMAT,
                                  sTmp.makeStringAndClear() );
    // and letter sync, if applicable
    SvXMLUnitConverter::convertNumLetterSync( sTmp, nNumberingType );

    if (!sTmp.isEmpty())
    {
        GetExport().AddAttribute(XML_NAMESPACE_STYLE, XML_NUM_LETTER_SYNC,
                                 sTmp.makeStringAndClear() );
    }
    // else: like page descriptor => ignore
}


/// export a date, time, or duration
void XMLTextFieldExport::ProcessDateTime(enum XMLTokenEnum eName,
                                         double dValue,
                                         bool bIsDate,
                                         bool bIsDuration,
                                         bool bOmitDurationIfZero,
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
        // date/time duration handle bOmitDurationIfZero
        if (!bOmitDurationIfZero || dValue != 0.0)
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
    ProcessString(eName, aBuffer.makeStringAndClear(), true, nPrefix);
}

/// export a date or time
void XMLTextFieldExport::ProcessDateTime(enum XMLTokenEnum eName,
                                         const util::DateTime& rTime)
{
    OUStringBuffer aBuffer;

    util::DateTime aDateTime(rTime);

    // date/time value
    ::sax::Converter::convertDateTime(aBuffer, aDateTime, nullptr);

    // output attribute
    ProcessString(eName, aBuffer.makeStringAndClear(), true);
}

/// export a date, time, or duration
void XMLTextFieldExport::ProcessDateTime(enum XMLTokenEnum eName,
                                         sal_Int32 nMinutes,
                                         bool bIsDate,
                                         bool bIsDuration)
{
    // handle bOmitDurationIfZero here, because we can precisely compare ints
    if (!(bIsDuration && (nMinutes==0)))
    {
        ProcessDateTime(eName, static_cast<double>(nMinutes) / double(24*60),
                        bIsDate, bIsDuration);
    }
}

/// export a time or dateTime
void XMLTextFieldExport::ProcessTimeOrDateTime(enum XMLTokenEnum eName,
                                         const util::DateTime& rTime)
{
    OUStringBuffer aBuffer;

    // date/time value
    ::sax::Converter::convertTimeOrDateTime(aBuffer, rTime);

    // output attribute
    ProcessString(eName, aBuffer.makeStringAndClear(), true);
}


SvXMLEnumMapEntry<sal_Int16> const aBibliographyDataTypeMap[] =
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
    Any aAny = rPropSet->getPropertyValue(gsPropertyFields);
    Sequence<PropertyValue> aValues;
    aAny >>= aValues;

    // one attribute per value (unless empty)
    for (const auto& rProp : aValues)
    {
        if( rProp.Name == "BibiliographicType" )
        {
            sal_Int16 nTypeId = 0;
            rProp.Value >>= nTypeId;
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
            rProp.Value >>= sStr;

            if (!sStr.isEmpty())
            {
                XMLTokenEnum eElement = MapBibliographyFieldName(rProp.Name);
                if (eElement == XML_URL || eElement == XML_LOCAL_URL || eElement == XML_TARGET_URL)
                {
                    sStr = GetExport().GetRelativeReference(sStr);
                }
                sal_uInt16 nPrefix = XML_NAMESPACE_TEXT;
                if (eElement == XML_LOCAL_URL || eElement == XML_TARGET_TYPE
                    || eElement == XML_TARGET_URL)
                {
                    nPrefix = XML_NAMESPACE_LO_EXT;
                }
                rExport.AddAttribute(nPrefix, eElement, sStr);
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
    const OUString& sSelected )
{
    // find selected element
    sal_Int32 nSelected = comphelper::findValue(rSequence, sSelected);

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
                                     false, false );
    }
}

void XMLTextFieldExport::ExportDataBaseElement(
    enum XMLTokenEnum eElementName,
    const OUString& sPresentation,
    const Reference<XPropertySet>& rPropertySet,
    const Reference<XPropertySetInfo>& rPropertySetInfo )
{
    SAL_WARN_IF( eElementName == XML_TOKEN_INVALID, "xmloff.text", "need token" );
    SAL_WARN_IF( !rPropertySet.is(), "xmloff.text", "need property set" );
    SAL_WARN_IF( !rPropertySetInfo.is(), "xmloff.text", "need property set info" );

    // get database properties
    OUString sDataBaseName;
    OUString sDataBaseURL;
    OUString sStr;
    if( ( rPropertySet->getPropertyValue( gsPropertyDataBaseName ) >>= sStr )
        && !sStr.isEmpty() )
    {
        sDataBaseName = sStr;
    }
    else if( rPropertySetInfo->hasPropertyByName( gsPropertyDataBaseURL ) &&
             (rPropertySet->getPropertyValue( gsPropertyDataBaseURL ) >>= sStr) &&
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
                                         false, false );

    // write URL as children
    if( !sDataBaseURL.isEmpty() )
    {
        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sDataBaseURL );
        SvXMLElementExport aDataSourceElement(
            GetExport(), XML_NAMESPACE_FORM, XML_CONNECTION_RESOURCE,
            false, false );
    }

    // write presentation
    rExport.Characters( sPresentation );
}


// explode a field master name into field type and field name
void XMLTextFieldExport::ExplodeFieldMasterName(
    std::u16string_view sMasterName, OUString& sFieldType, OUString& sVarName)
{
    sal_Int32 nLength = gsFieldMasterPrefix.getLength();
    size_t nSeparator = sMasterName.find('.', nLength);

    // '.' found?
    if (nSeparator == o3tl::make_unsigned(nLength) || nSeparator == std::u16string_view::npos) {
        SAL_WARN("xmloff.text", "no field var name!");
    }
    else
    {
        sFieldType = sMasterName.substr(nLength, nSeparator-nLength);
        sVarName = sMasterName.substr(nSeparator+1);
    }
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
bool XMLTextFieldExport::GetDependentFieldPropertySet(
    const Reference<XPropertySet> & xMaster,
    Reference<XPropertySet> & xField)
{
    Any aAny;
    Sequence<Reference<XDependentTextField> > aFields;
    aAny = xMaster->getPropertyValue(gsPropertyDependentTextFields);
    aAny >>= aFields;

    // any fields?
    if (aFields.hasElements())
    {
        // get first one and return
        Reference<XDependentTextField> xTField = aFields[0];
        xField.set(xTField, UNO_QUERY);
        DBG_ASSERT(xField.is(),
                  "Surprisingly, this TextField refuses to be a PropertySet!");
        return true;
    }
    else
    {
        return false;
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
            // unknown placeholder: XML_TEXT
            OSL_FAIL("unknown placeholder type");
    }

    return eType;
}


/// element name for author fields
enum XMLTokenEnum XMLTextFieldExport::MapAuthorFieldName(
    const Reference<XPropertySet> & xPropSet)
{
    // Initials or full name?
    return GetBoolProperty(gsPropertyFullName, xPropSet)
        ? XML_AUTHOR_NAME : XML_AUTHOR_INITIALS;
}

enum XMLTokenEnum XMLTextFieldExport::MapPageNumberName(
    const Reference<XPropertySet> & xPropSet,
    sal_Int32& nOffset)
{
    enum XMLTokenEnum eName = XML_TOKEN_INVALID;
    PageNumberType ePage;
    Any aAny = xPropSet->getPropertyValue(gsPropertySubType);
    ePage = *o3tl::doAccess<PageNumberType>(aAny);

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
            OSL_FAIL("unknown chapter display format");
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
        case ReferenceFieldSource::STYLE:
            eElement = XML_STYLE_REF;
            break;
        default:
            OSL_FAIL("unknown reference source");
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
    switch (GetInt16Property(gsPropertyFieldSubType, xPropSet))
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
            SAL_WARN("xmloff.text", "unknown sender type");
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
            SAL_WARN("xmloff.text", "unknown docinfo field type!");
            eElement = XML_TOKEN_INVALID;
            break;
    }

    return eElement;
}

enum XMLTokenEnum XMLTextFieldExport::MapBibliographyFieldName(std::u16string_view sName)
{
    enum XMLTokenEnum eName = XML_TOKEN_INVALID;

    if( sName == u"Identifier" )
    {
        eName = XML_IDENTIFIER;
    }
    else if( sName == u"BibiliographicType" )
    {
        eName = XML_BIBLIOGRAPHY_TYPE;
    }
    else if( sName == u"Address" )
    {
        eName = XML_ADDRESS;
    }
    else if( sName == u"Annote" )
    {
        eName = XML_ANNOTE;
    }
    else if( sName == u"Author" )
    {
        eName = XML_AUTHOR;
    }
    else if( sName == u"Booktitle" )
    {
        eName = XML_BOOKTITLE;
    }
    else if( sName == u"Chapter" )
    {
        eName = XML_CHAPTER;
    }
    else if( sName == u"Edition" )
    {
        eName = XML_EDITION;
    }
    else if( sName == u"Editor" )
    {
        eName = XML_EDITOR;
    }
    else if( sName == u"Howpublished" )
    {
        eName = XML_HOWPUBLISHED;
    }
    else if( sName == u"Institution" )
    {
        eName = XML_INSTITUTION;
    }
    else if( sName == u"Journal" )
    {
        eName = XML_JOURNAL;
    }
    else if( sName == u"Month" )
    {
        eName = XML_MONTH;
    }
    else if( sName == u"Note" )
    {
        eName = XML_NOTE;
    }
    else if( sName == u"Number" )
    {
        eName = XML_NUMBER;
    }
    else if( sName == u"Organizations" )
    {
        eName = XML_ORGANIZATIONS;
    }
    else if( sName == u"Pages" )
    {
        eName = XML_PAGES;
    }
    else if( sName == u"Publisher" )
    {
        eName = XML_PUBLISHER;
    }
    else if( sName == u"School" )
    {
        eName = XML_SCHOOL;
    }
    else if( sName == u"Series" )
    {
        eName = XML_SERIES;
    }
    else if( sName == u"Title" )
    {
        eName = XML_TITLE;
    }
    else if( sName == u"Report_Type" )
    {
        eName = XML_REPORT_TYPE;
    }
    else if( sName == u"Volume" )
    {
        eName = XML_VOLUME;
    }
    else if( sName == u"Year" )
    {
        eName = XML_YEAR;
    }
    else if( sName == u"URL" )
    {
        eName = XML_URL;
    }
    else if( sName == u"Custom1" )
    {
        eName = XML_CUSTOM1;
    }
    else if( sName == u"Custom2" )
    {
        eName = XML_CUSTOM2;
    }
    else if( sName == u"Custom3" )
    {
        eName = XML_CUSTOM3;
    }
    else if( sName == u"Custom4" )
    {
        eName = XML_CUSTOM4;
    }
    else if( sName == u"Custom5" )
    {
        eName = XML_CUSTOM5;
    }
    else if( sName == u"ISBN" )
    {
        eName = XML_ISBN;
    }
    else if (sName == u"LocalURL")
    {
        eName = XML_LOCAL_URL;
    }
    else if (sName == u"TargetType")
    {
        eName = XML_TARGET_TYPE;
    }
    else if (sName == u"TargetURL")
    {
        eName = XML_TARGET_URL;
    }
    else
    {
        SAL_WARN("xmloff.text", "Unknown bibliography info data");
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
    return "ftn" + OUString::number(static_cast<sal_Int32>(nSeqNo));
}

OUString XMLTextFieldExport::MakeSequenceRefName(
    sal_Int16 nSeqNo,
    std::u16string_view rSeqName)
{
    // generate foot-/endnote ID
    return OUString::Concat("ref") +rSeqName + OUString::number(static_cast<sal_Int32>(nSeqNo));
}


// Property accessor helper functions


// to be relegated (does that word exist?) to a more appropriate place


bool GetBoolProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    bool bBool = *o3tl::doAccess<bool>(aAny);
    return bBool;
}

bool GetOptionalBoolProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet,
    const Reference<XPropertySetInfo> & xPropSetInfo,
    bool bDefault)
{
    return xPropSetInfo->hasPropertyByName( sPropName )
        ? GetBoolProperty( sPropName, xPropSet ) : bDefault;
}

double GetDoubleProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    double fDouble = 0.0;
    aAny >>= fDouble;
    return fDouble;
}

OUString GetStringProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    OUString sString;
    aAny >>= sString;
    return sString;
}

sal_Int32 GetIntProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    sal_Int32 nInt = 0;
    aAny >>= nInt;
    return nInt;
}

sal_Int16 GetInt16Property(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    sal_Int16 nInt = 0;
    aAny >>= nInt;
    return nInt;
}

sal_Int8 GetInt8Property(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    sal_Int8 nInt = 0;
    aAny >>= nInt;
    return nInt;
}

util::DateTime GetDateTimeProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    util::DateTime aTime;
    aAny >>= aTime;
    return aTime;
}

Sequence<OUString> GetStringSequenceProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    Sequence<OUString> aSequence;
    aAny >>= aSequence;
    return aSequence;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
