/*************************************************************************
 *
 *  $RCSfile: txtflde.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: dvo $ $Date: 2001-03-09 14:10:33 $
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
 *  export of all text fields
 */

#ifndef _XMLOFF_TXTFLDE_HXX
#include "txtflde.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLNUMFE_HXX
#include "xmlnumfe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlement.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLNUME_HXX
#include "xmlnume.hxx"
#endif

#ifndef XMLOFF_NUMEHELP_HXX
#include "numehelp.hxx"
#endif

#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include "maptypes.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_USERDATAPART_HPP_
#include <com/sun/star/text/UserDataPart.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_PAGENUMBERTYPE_HPP_
#include <com/sun/star/text/PageNumberType.hpp>
#endif

#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_REFERENCEFIELDPART_HPP_
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_REFERENCEFIELDSOURCE_HPP_
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
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

#ifndef _COM_SUN_STAR_TEXT_SETVARIABLETYPE_HPP
#include <com/sun/star/text/SetVariableType.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_PLACEHOLDERTYPE_HPP_
#include <com/sun/star/text/PlaceholderType.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFO_HPP_
#include <com/sun/star/document/XDocumentInfo.hpp>
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

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_BIBLIOGRAPHYDATATYPE_HPP_
#include <com/sun/star/text/BibliographyDataType.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _TOOLS_SOLMATH_HXX
#include <tools/solmath.hxx>
#endif

#include <vector>

using namespace ::rtl;
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


static sal_Char __READONLY_DATA FIELD_SERVICE_SENDER[] = "ExtendedUser";
static sal_Char __READONLY_DATA FIELD_SERVICE_AUTHOR[] = "Author";
static sal_Char __READONLY_DATA FIELD_SERVICE_JUMPEDIT[] = "JumpEdit";
static sal_Char __READONLY_DATA FIELD_SERVICE_GETEXP[] = "GetExpression";
static sal_Char __READONLY_DATA FIELD_SERVICE_SETEXP[] = "SetExpression";
static sal_Char __READONLY_DATA FIELD_SERVICE_USER[] = "User";
static sal_Char __READONLY_DATA FIELD_SERVICE_INPUT[] = "Input";
static sal_Char __READONLY_DATA FIELD_SERVICE_USERINPUT[] = "InputUser";
static sal_Char __READONLY_DATA FIELD_SERVICE_DATETIME[] = "DateTime";
static sal_Char __READONLY_DATA FIELD_SERVICE_PAGENUMBER[] = "PageNumber";
static sal_Char __READONLY_DATA FIELD_SERVICE_DB_NEXT[] = "DatabaseNextSet";
static sal_Char __READONLY_DATA FIELD_SERVICE_DB_SELECT[] = "DatabaseNumberOfSet";
static sal_Char __READONLY_DATA FIELD_SERVICE_DB_NUMBER[] = "DatabaseSetNumber";
static sal_Char __READONLY_DATA FIELD_SERVICE_DB_DISPLAY[] = "Database";
static sal_Char __READONLY_DATA FIELD_SERVICE_DB_NAME[] = "DatabaseName";
static sal_Char __READONLY_DATA FIELD_SERVICE_CONDITIONAL_TEXT[] = "ConditionalText";
static sal_Char __READONLY_DATA FIELD_SERVICE_HIDDEN_TEXT[] = "HiddenText";
static sal_Char __READONLY_DATA FIELD_SERVICE_HIDDEN_PARAGRAPH[] = "HiddenParagraph";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_CHANGE_AUTHOR[] = "DocInfo.ChangeAuthor";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_CHANGE_DATE_TIME[] = "DocInfo.ChangeDateTime";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_EDIT_TIME[] = "DocInfo.EditTime";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_DESCRIPTION[] = "DocInfo.Description";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_CREATE_AUTHOR[] = "DocInfo.CreateAuthor";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_CREATE_DATE_TIME[] = "DocInfo.CreateDateTime";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_INFO0[] = "DocInfo.Info0";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_INFO1[] = "DocInfo.Info1";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_INFO2[] = "DocInfo.Info2";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_INFO3[] = "DocInfo.Info3";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_PRINT_AUTHOR[] = "DocInfo.PrintAuthor";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_PRINT_DATE_TIME[] = "DocInfo.PrintDateTime";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_KEY_WORDS[] = "DocInfo.KeyWords";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_SUBJECT[] = "DocInfo.Subject";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_TITLE[] = "DocInfo.Title";
static sal_Char __READONLY_DATA FIELD_SERVICE_DOC_INFO_REVISION[] = "DocInfo.Revision";
static sal_Char __READONLY_DATA FIELD_SERVICE_FILE_NAME[] = "FileName";
static sal_Char __READONLY_DATA FIELD_SERVICE_CHAPTER[] = "Chapter";
static sal_Char __READONLY_DATA FIELD_SERVICE_TEMPLATE_NAME[] = "TemplateName";
static sal_Char __READONLY_DATA FIELD_SERVICE_PAGE_COUNT[] = "PageCount";
static sal_Char __READONLY_DATA FIELD_SERVICE_PARAGRAPH_COUNT[] = "ParagraphCount";
static sal_Char __READONLY_DATA FIELD_SERVICE_WORD_COUNT[] = "WordCount";
static sal_Char __READONLY_DATA FIELD_SERVICE_CHARACTER_COUNT[] = "CharacterCount";
static sal_Char __READONLY_DATA FIELD_SERVICE_TABLE_COUNT[] = "TableCount";
static sal_Char __READONLY_DATA FIELD_SERVICE_GRAPHIC_COUNT[] = "GraphicObjectCount";
static sal_Char __READONLY_DATA FIELD_SERVICE_OBJECT_COUNT[] = "EmbeddedObjectCount";
static sal_Char __READONLY_DATA FIELD_SERVICE_REFERENCE_PAGE_SET[] = "ReferencePageSet";
static sal_Char __READONLY_DATA FIELD_SERVICE_REFERENCE_PAGE_GET[] = "ReferencePageGet";
static sal_Char __READONLY_DATA FIELD_SERVICE_SHEET_NAME[] = "SheetName";
static sal_Char __READONLY_DATA FIELD_SERVICE_MACRO[] = "Macro";
static sal_Char __READONLY_DATA FIELD_SERVICE_GET_REFERENCE[] = "GetReference";
static sal_Char __READONLY_DATA FIELD_SERVICE_DDE[] = "DDE";
static sal_Char __READONLY_DATA FIELD_SERVICE_URL[] = "URL";
static sal_Char __READONLY_DATA FIELD_SERVICE_BIBLIOGRAPHY[] = "Bibliography";
static sal_Char __READONLY_DATA FIELD_SERVICE_SCRIPT[] = "Script";
static sal_Char __READONLY_DATA FIELD_SERVICE_ANNOTATION[] = "Annotation";
static sal_Char __READONLY_DATA FIELD_SERVICE_COMBINED_CHARACTERS[] = "CombinedCharacters";
static sal_Char __READONLY_DATA FIELD_SERVICE_MEASURE[] = "Measure";


SvXMLEnumMapEntry __READONLY_DATA aFieldServiceNameMapping[] =
{
    { FIELD_SERVICE_SENDER,                 FIELD_ID_SENDER },
    { FIELD_SERVICE_AUTHOR,                 FIELD_ID_AUTHOR },
    { FIELD_SERVICE_JUMPEDIT,               FIELD_ID_PLACEHOLDER },
    { FIELD_SERVICE_GETEXP,                 FIELD_ID_VARIABLE_GET },
    { FIELD_SERVICE_SETEXP,                 FIELD_ID_VARIABLE_SET },
    { FIELD_SERVICE_USER,                   FIELD_ID_USER_GET },
    { FIELD_SERVICE_INPUT,                  FIELD_ID_TEXT_INPUT },
    { FIELD_SERVICE_USERINPUT,              FIELD_ID_USER_INPUT },
    { FIELD_SERVICE_DATETIME,               FIELD_ID_TIME },
    { FIELD_SERVICE_PAGENUMBER,             FIELD_ID_PAGENUMBER },
    { FIELD_SERVICE_REFERENCE_PAGE_SET,     FIELD_ID_REFPAGE_SET },
    { FIELD_SERVICE_REFERENCE_PAGE_GET,     FIELD_ID_REFPAGE_GET },

    { FIELD_SERVICE_DB_NEXT,                FIELD_ID_DATABASE_NEXT },
    { FIELD_SERVICE_DB_SELECT,              FIELD_ID_DATABASE_SELECT },
    { FIELD_SERVICE_DB_NUMBER,              FIELD_ID_DATABASE_NUMBER },
    { FIELD_SERVICE_DB_DISPLAY,             FIELD_ID_DATABASE_DISPLAY },
    // workaround for #no-bug#: Database/DataBase
    { "DataBase",                           FIELD_ID_DATABASE_DISPLAY },
    { FIELD_SERVICE_DB_NAME,                FIELD_ID_DATABASE_NAME },

    { FIELD_SERVICE_DOC_INFO_CREATE_AUTHOR, FIELD_ID_DOCINFO_CREATION_AUTHOR },
    { FIELD_SERVICE_DOC_INFO_CREATE_DATE_TIME, FIELD_ID_DOCINFO_CREATION_TIME},
    { FIELD_SERVICE_DOC_INFO_CHANGE_AUTHOR, FIELD_ID_DOCINFO_SAVE_AUTHOR },
    { FIELD_SERVICE_DOC_INFO_CHANGE_DATE_TIME, FIELD_ID_DOCINFO_SAVE_TIME },
    { FIELD_SERVICE_DOC_INFO_EDIT_TIME,     FIELD_ID_DOCINFO_EDIT_DURATION },
    { FIELD_SERVICE_DOC_INFO_DESCRIPTION,   FIELD_ID_DOCINFO_DESCRIPTION },
    { FIELD_SERVICE_DOC_INFO_INFO0,         FIELD_ID_DOCINFO_INFORMATION0 },
    { FIELD_SERVICE_DOC_INFO_INFO1,         FIELD_ID_DOCINFO_INFORMATION1 },
    { FIELD_SERVICE_DOC_INFO_INFO2,         FIELD_ID_DOCINFO_INFORMATION2 },
    { FIELD_SERVICE_DOC_INFO_INFO3,         FIELD_ID_DOCINFO_INFORMATION3 },
    { FIELD_SERVICE_DOC_INFO_PRINT_AUTHOR,  FIELD_ID_DOCINFO_PRINT_AUTHOR },
    { FIELD_SERVICE_DOC_INFO_PRINT_DATE_TIME, FIELD_ID_DOCINFO_PRINT_TIME },
    { FIELD_SERVICE_DOC_INFO_KEY_WORDS,     FIELD_ID_DOCINFO_KEYWORDS },
    { FIELD_SERVICE_DOC_INFO_SUBJECT,       FIELD_ID_DOCINFO_SUBJECT },
    { FIELD_SERVICE_DOC_INFO_TITLE,         FIELD_ID_DOCINFO_TITLE },
    { FIELD_SERVICE_DOC_INFO_REVISION,      FIELD_ID_DOCINFO_REVISION },

    { FIELD_SERVICE_CONDITIONAL_TEXT,       FIELD_ID_CONDITIONAL_TEXT },
    { FIELD_SERVICE_HIDDEN_TEXT,            FIELD_ID_HIDDEN_TEXT },
    { FIELD_SERVICE_HIDDEN_PARAGRAPH,       FIELD_ID_HIDDEN_PARAGRAPH },

    { FIELD_SERVICE_FILE_NAME,              FIELD_ID_FILE_NAME },
    { FIELD_SERVICE_CHAPTER,                FIELD_ID_CHAPTER },
    { FIELD_SERVICE_TEMPLATE_NAME,          FIELD_ID_TEMPLATE_NAME },

    { FIELD_SERVICE_PAGE_COUNT,             FIELD_ID_COUNT_PAGES },
    { FIELD_SERVICE_PARAGRAPH_COUNT,        FIELD_ID_COUNT_PARAGRAPHS },
    { FIELD_SERVICE_WORD_COUNT,             FIELD_ID_COUNT_WORDS },
    { FIELD_SERVICE_CHARACTER_COUNT,        FIELD_ID_COUNT_CHARACTERS },
    { FIELD_SERVICE_TABLE_COUNT,            FIELD_ID_COUNT_TABLES },
    { FIELD_SERVICE_GRAPHIC_COUNT,          FIELD_ID_COUNT_GRAPHICS },
    { FIELD_SERVICE_OBJECT_COUNT,           FIELD_ID_COUNT_OBJECTS },

    { FIELD_SERVICE_MACRO,                  FIELD_ID_MACRO },
    { FIELD_SERVICE_GET_REFERENCE,          FIELD_ID_REF_REFERENCE },
    { FIELD_SERVICE_DDE,                    FIELD_ID_DDE },

    { FIELD_SERVICE_BIBLIOGRAPHY,           FIELD_ID_BIBLIOGRAPHY },

    { FIELD_SERVICE_SCRIPT,                 FIELD_ID_SCRIPT },
    { FIELD_SERVICE_ANNOTATION,             FIELD_ID_ANNOTATION },

    { FIELD_SERVICE_COMBINED_CHARACTERS,    FIELD_ID_COMBINED_CHARACTERS },

    // non-writer fields
    { FIELD_SERVICE_SHEET_NAME,             FIELD_ID_SHEET_NAME },
    { FIELD_SERVICE_URL,                    FIELD_ID_URL },
    { FIELD_SERVICE_MEASURE,                FIELD_ID_MEASURE },

    { 0,                                    0 }
};



// property accessor helper functions
inline sal_Bool const GetBoolProperty(const OUString&,
                                      const Reference<XPropertySet> &);
inline Double const GetDoubleProperty(const OUString&,
                                      const Reference<XPropertySet> &);
inline OUString const GetStringProperty(const OUString&,
                                        const Reference<XPropertySet> &);
inline sal_Int32 const GetIntProperty(const OUString&,
                                      const Reference<XPropertySet> &);
inline sal_Int16 const GetInt16Property(const OUString&,
                                        const Reference<XPropertySet> &);
inline sal_Int8 const GetInt8Property(const OUString&,
                                      const Reference<XPropertySet> &);
inline DateTime const GetDateTimeProperty( const OUString& sPropName,
                                           const Reference<XPropertySet> & xPropSet);
inline Date const GetDateProperty( const OUString& sPropName,
                                   const Reference<XPropertySet> & xPropSet);



XMLTextFieldExport::XMLTextFieldExport( SvXMLExport& rExp,
                                        XMLPropertyState* pCombinedCharState)
    : rExport(rExp),
      pUsedMasters(NULL),
      sServicePrefix(
          RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextField.")),
      sFieldMasterPrefix(
          RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.FieldMaster.")),
      sPropertyContent(RTL_CONSTASCII_USTRINGPARAM("Content")),
      sPropertyIsFixed(RTL_CONSTASCII_USTRINGPARAM("IsFixed")),
      sPropertyAuthorFullname(RTL_CONSTASCII_USTRINGPARAM("FullName")),
      sPropertyFieldSubType(RTL_CONSTASCII_USTRINGPARAM("UserDataType")),
      sPropertyHint(RTL_CONSTASCII_USTRINGPARAM("Hint")),
      sPropertyPlaceholder(RTL_CONSTASCII_USTRINGPARAM("PlaceHolder")),
      sPropertyPlaceholderType(RTL_CONSTASCII_USTRINGPARAM("PlaceHolderType")),
      sPropertyIsVisible(RTL_CONSTASCII_USTRINGPARAM("IsVisible")),
      sPropertyIsShowFormula(RTL_CONSTASCII_USTRINGPARAM("IsShowFormula")),
      sPropertyIsInput(RTL_CONSTASCII_USTRINGPARAM("Input")),
      sPropertyIsExpression(RTL_CONSTASCII_USTRINGPARAM("IsExpression")),
      sPropertyNumberFormat(RTL_CONSTASCII_USTRINGPARAM("NumberFormat")),
      sPropertyVariableName(RTL_CONSTASCII_USTRINGPARAM("VariableName")),
      sPropertySubType(RTL_CONSTASCII_USTRINGPARAM("SubType")),
      sPropertyName(RTL_CONSTASCII_USTRINGPARAM("Name")),
      sPropertyVariableSubType(RTL_CONSTASCII_USTRINGPARAM("VariableSubtype")),
      sPropertyValue(RTL_CONSTASCII_USTRINGPARAM("Value")),
      sPropertyChapterNumberingLevel(
          RTL_CONSTASCII_USTRINGPARAM("ChapterNumberingLevel")),
      sPropertyNumberingSeparator(
          RTL_CONSTASCII_USTRINGPARAM("NumberingSeparator")),
      sPropertyNumberingType(
          RTL_CONSTASCII_USTRINGPARAM("NumberingType")),
      sPropertyDateTimeValue(RTL_CONSTASCII_USTRINGPARAM("DateTimeValue")),
      sPropertyUserText(RTL_CONSTASCII_USTRINGPARAM("UserText")),
      sPropertyOffset(RTL_CONSTASCII_USTRINGPARAM("Offset")),
      sPropertyDataBaseName(RTL_CONSTASCII_USTRINGPARAM("DataBaseName")),
      sPropertyDataTableName(RTL_CONSTASCII_USTRINGPARAM("DataTableName")),
      sPropertyCondition(RTL_CONSTASCII_USTRINGPARAM("Condition")),
      sPropertySetNumber(RTL_CONSTASCII_USTRINGPARAM("SetNumber")),
      sPropertyIsDataBaseFormat(RTL_CONSTASCII_USTRINGPARAM("DataBaseFormat")),
      sPropertyDataColumnName(RTL_CONSTASCII_USTRINGPARAM("DataColumnName")),
      sPropertyDateTime(RTL_CONSTASCII_USTRINGPARAM("DateTime")),
      sPropertyTrueContent(RTL_CONSTASCII_USTRINGPARAM("TrueContent")),
      sPropertyFalseContent(RTL_CONSTASCII_USTRINGPARAM("FalseContent")),
      sPropertyRevision(RTL_CONSTASCII_USTRINGPARAM("Revision")),
      sPropertyFileFormat(RTL_CONSTASCII_USTRINGPARAM("FileFormat")),
      sPropertyChapterFormat(RTL_CONSTASCII_USTRINGPARAM("ChapterFormat")),
      sPropertyLevel(RTL_CONSTASCII_USTRINGPARAM("Level")),
      sPropertyIsDate(RTL_CONSTASCII_USTRINGPARAM("IsDate")),
      sPropertyAdjust(RTL_CONSTASCII_USTRINGPARAM("Adjust")),
      sPropertyOn(RTL_CONSTASCII_USTRINGPARAM("On")),
      sPropertyMacro(RTL_CONSTASCII_USTRINGPARAM("Macro")),
      sPropertyReferenceFieldPart(
          RTL_CONSTASCII_USTRINGPARAM("ReferenceFieldPart")),
      sPropertyReferenceFieldType(
          RTL_CONSTASCII_USTRINGPARAM("ReferenceFieldType")),
      sPropertyReferenceFieldSource(
          RTL_CONSTASCII_USTRINGPARAM("ReferenceFieldSource")),
      sPropertySequenceNumber(RTL_CONSTASCII_USTRINGPARAM("SequenceNumber")),
      sPropertySequenceValue(RTL_CONSTASCII_USTRINGPARAM("SequenceValue")),
      sPropertySourceName(RTL_CONSTASCII_USTRINGPARAM("SourceName")),
      sPropertyDDECommandType(RTL_CONSTASCII_USTRINGPARAM("DDECommandType")),
      sPropertyDDECommandFile(RTL_CONSTASCII_USTRINGPARAM("DDECommandFile")),
      sPropertyDDECommandElement(
          RTL_CONSTASCII_USTRINGPARAM("DDECommandElement")),
      sPropertyIsAutomaticUpdate(
          RTL_CONSTASCII_USTRINGPARAM("IsAutomaticUpdate")),
      sPropertyDependentTextFields(
          RTL_CONSTASCII_USTRINGPARAM("DependentTextFields")),
      sPropertyURL(RTL_CONSTASCII_USTRINGPARAM("URL")),
      sPropertyTargetFrame(RTL_CONSTASCII_USTRINGPARAM("TargetFrame")),
      sPropertyFields(RTL_CONSTASCII_USTRINGPARAM("Fields")),
      sPropertyScriptType(RTL_CONSTASCII_USTRINGPARAM("ScriptType")),
      sPropertyURLContent(RTL_CONSTASCII_USTRINGPARAM("URLContent")),
      sPropertyAuthor(RTL_CONSTASCII_USTRINGPARAM("Author")),
      sPropertyDate(RTL_CONSTASCII_USTRINGPARAM("Date")),
      sPropertyMeasureKind(RTL_CONSTASCII_USTRINGPARAM("Kind")),
      sPropertyInstanceName(RTL_CONSTASCII_USTRINGPARAM("InstanceName")),
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
        if( 0 == pNames->compareTo(sServicePrefix, sServicePrefix.getLength()))
        {
            // TextField found => postfix is field type!
            sFieldName = pNames->copy(sServicePrefix.getLength());
            break;
        }

        ++pNames;
    }


    // map postfix of service name to field ID
    DBG_ASSERT(sFieldName.getLength()>0, "no TextField service found!");
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
    DBG_ASSERT(sFieldName.getLength()>0, "no valid service name!");
    enum FieldIdEnum nToken = FIELD_ID_UNKNOWN;
    if (sFieldName.getLength() > 0)
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
        case FIELD_ID_DOCINFO_INFORMATION0:
        case FIELD_ID_DOCINFO_INFORMATION1:
        case FIELD_ID_DOCINFO_INFORMATION2:
        case FIELD_ID_DOCINFO_INFORMATION3:
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
        case FIELD_ID_SHEET_NAME:
        case FIELD_ID_MEASURE:
        case FIELD_ID_URL:
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
    sal_uInt16 nFieldType,
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

    case FIELD_ID_DATABASE_DISPLAY:
        // TODO: depends on... ???
        // workaround #no-bug#: no data type
        return 5100 == GetIntProperty(sPropertyNumberFormat, xPropSet);
        break;

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
    case FIELD_ID_DOCINFO_INFORMATION0:
    case FIELD_ID_DOCINFO_INFORMATION1:
    case FIELD_ID_DOCINFO_INFORMATION2:
    case FIELD_ID_DOCINFO_INFORMATION3:
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
    default:
        DBG_ERROR("unkown field type/field has no content");
        return sal_True; // invalid info; string in case of doubt
    }
}

/// export the styles needed by the given field. Called on first pass
/// through document
void XMLTextFieldExport::ExportFieldAutoStyle(
    const Reference<XTextField> & rTextField)
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
                set<OUString> * pSet = new set<OUString> ;
                (*pUsedMasters)[xOurText] = *pSet;
                aMapIter = pUsedMasters->find(xOurText);
            }

            // insert this text field master
            OUString sFieldMasterName = GetStringProperty(
                sPropertyInstanceName, xDepField->getTextFieldMaster());
            aMapIter->second.insert( sFieldMasterName );
        }
        // else: no dependent field -> no master -> ignore
    }

    // get Field ID
    sal_uInt16 nToken = GetFieldID(rTextField, xPropSet);

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
        // date and time fields are always number fields, but the
        // NumberFormat property is optional (e.g. Calc doesn't
        // support it)
        if (xPropSet->getPropertySetInfo()->hasPropertyByName(
                                                    sPropertyNumberFormat))
        {
            sal_Int32 nFormat =
                GetIntProperty(sPropertyNumberFormat, xPropSet);

            // nFormat may be -1 for numeric fields that display their
            //  variable name. (Maybe this should be a field type, then?)
            if (nFormat != -1) {
                GetExport().addDataStyle(nFormat);
            }
        }
        break;

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
        // register number format, if this is a numeric field
        if (! IsStringField(nToken, xPropSet)) {

            sal_Int32 nFormat =
                GetIntProperty(sPropertyNumberFormat, xPropSet);

            // nFormat may be -1 for numeric fields that display their
            //  variable name. (Maybe this should be a field type, then?)
            if (nFormat != -1) {
                GetExport().addDataStyle(nFormat);
            }
        }
        break;

    case FIELD_ID_COMBINED_CHARACTERS:
    {
        // export text style with the addition of the combined characters
        DBG_ASSERT(NULL != pCombinedCharactersPropertyState,
                   "need proper PropertyState for combined characters");
        GetExport().GetTextParagraphExport()->Add(
            XML_STYLE_FAMILY_TEXT_TEXT, xRangePropSet,
            pCombinedCharactersPropertyState);
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
    case FIELD_ID_DOCINFO_INFORMATION0:
    case FIELD_ID_DOCINFO_INFORMATION1:
    case FIELD_ID_DOCINFO_INFORMATION2:
    case FIELD_ID_DOCINFO_INFORMATION3:
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
        ; // no formats for these fields!
        break;

    case FIELD_ID_UNKNOWN:
    default:
        DBG_ERROR("unkown field type!");
        // ignore -> no format for unkowns
        break;
    }
}

/// export the given field to XML. Called on second pass through document
void XMLTextFieldExport::ExportField(const Reference<XTextField> & rTextField )
{
    // get property set
    Reference<XPropertySet> xPropSet(rTextField, UNO_QUERY);

    // get property set of range (for the attributes)
    Reference <XPropertySet> xRangePropSet(rTextField->getAnchor(), UNO_QUERY);

    // get Field ID
    enum FieldIdEnum nToken = GetFieldID(rTextField, xPropSet);

    // export span with style (if necessary)
    // except for combined characters field)
    if (FIELD_ID_COMBINED_CHARACTERS != nToken)
    {
        OUString sStyle = GetExport().GetTextParagraphExport()->Find(
            XML_STYLE_FAMILY_TEXT_TEXT, xRangePropSet, sEmpty);
        if( sStyle.getLength() )
        {
            // export <text:span> element
            GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                      sXML_style_name, sStyle );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                      sXML_span, sal_False, sal_False);
            ExportFieldHelper( rTextField, xPropSet, xRangePropSet, nToken );
        }
        else
        {
            // no style -> no span
            ExportFieldHelper( rTextField, xPropSet, xRangePropSet, nToken );
        }
    }
    else
    {
        // combined characters: span will be handled below
        ExportFieldHelper( rTextField, xPropSet, xRangePropSet, nToken );
    }
}

/// export the given field to XML. Called on second pass through document
void XMLTextFieldExport::ExportFieldHelper(
    const Reference<XTextField> & rTextField,
    const Reference<XPropertySet> & rPropSet,
    const Reference<XPropertySet> & rRangePropSet,
    enum FieldIdEnum nToken)
{
    // get property set info (because some attributes are not support
    // in all implementations)
    Reference<XPropertySetInfo> xPropSetInfo(rPropSet->getPropertySetInfo());

    OUString sPresentation = rTextField->getPresentation(sal_False);

    // process each field type
    switch (nToken) {
    case FIELD_ID_AUTHOR:
        // author field: fixed, field (sub-)type
        ProcessBoolean(sXML_fixed,
                       GetBoolProperty(sPropertyIsFixed, rPropSet), sal_True);
        ExportElement(MapAuthorFieldName(rPropSet), sPresentation);
        break;

    case FIELD_ID_SENDER:
        // sender field: fixed, field (sub-)type
        ProcessBoolean(sXML_fixed,
                       GetBoolProperty(sPropertyIsFixed, rPropSet), sal_True);
        ExportElement(MapSenderFieldName(rPropSet), sPresentation);
        break;

    case FIELD_ID_PLACEHOLDER:
        // placeholder field: type, name, description
        ProcessString(sXML_placeholder_type,
                      MapPlaceholderType(
                        GetInt16Property(sPropertyPlaceholderType, rPropSet)));
        ProcessString(sXML_description,
                      GetStringProperty(sPropertyHint,rPropSet), sal_True);
        ExportElement(sXML_placeholder, sPresentation);
        break;

    case FIELD_ID_VARIABLE_SET:
    {
        // variable set field: name, visible, format&value
        ProcessString(sXML_name,
                      GetStringProperty(sPropertyVariableName, rPropSet));
        ProcessDisplay(GetBoolProperty(sPropertyIsVisible, rPropSet),
                       sal_False);
        ProcessString(sXML_formula,
                      GetStringProperty(sPropertyContent, rPropSet),
                      sPresentation);
        ProcessValueAndType(IsStringField(nToken, rPropSet),
                            GetIntProperty(sPropertyNumberFormat, rPropSet),
                            GetStringProperty(sPropertyContent, rPropSet),
                            sPresentation,
                            GetDoubleProperty(sPropertyValue, rPropSet),
                            sal_True, sal_True, sal_True);
        ExportElement(sXML_variable_set, sPresentation);
        break;
    }
    case FIELD_ID_VARIABLE_GET:
    {
        // variable get field: name, format&value
        ProcessString(sXML_name,
                      GetStringProperty(sPropertyContent, rPropSet));
        sal_Bool bCmd = GetBoolProperty(sPropertyIsShowFormula, rPropSet);
        ProcessDisplay(sal_True, bCmd);
        // show style, unless name will be shown
        ProcessValueAndType(IsStringField(nToken, rPropSet),
                            GetIntProperty(sPropertyNumberFormat, rPropSet),
                            sEmpty, sEmpty, 0.0, // values not used
                            sal_False, !bCmd, !bCmd);
        ExportElement(sXML_variable_get, sPresentation);
        break;
    }
    case FIELD_ID_VARIABLE_INPUT:
        // variable input field: name, description, format&value
        ProcessString(sXML_name,
                      GetStringProperty(sPropertyVariableName, rPropSet));
        ProcessString(sXML_description,
                      GetStringProperty(sPropertyHint , rPropSet));
        ProcessDisplay(GetBoolProperty(sPropertyIsVisible, rPropSet),
                       sal_False);
        ProcessString(sXML_formula,
                      GetStringProperty(sPropertyContent, rPropSet),
                      sPresentation);
        ProcessValueAndType(IsStringField(nToken, rPropSet),
                            GetIntProperty(sPropertyNumberFormat, rPropSet),
                            GetStringProperty(sPropertyContent, rPropSet),
                            sPresentation,
                            GetDoubleProperty(sPropertyValue, rPropSet),
                            sal_True, sal_True, sal_True);
        ExportElement(sXML_variable_input, sPresentation);
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
                            sal_False, sal_False, !bCmd);

        // name from FieldMaster
        ProcessString(sXML_name,
                      GetStringProperty(sPropertyName,
                                        GetMasterPropertySet(rTextField)));
        ExportElement(sXML_user_field_get, sPresentation);
        break;
    }

    case FIELD_ID_USER_INPUT:
        // user input field: name (from FieldMaster), description
//      ProcessString(sXML_name,
//                    GetStringProperty(sPropertyName,
//                                      GetMasterPropertySet(rTextField)));
        ProcessString(sXML_name,
                      GetStringProperty(sPropertyContent, rPropSet));
        ProcessString(sXML_description,
                      GetStringProperty(sPropertyHint, rPropSet));
        ExportElement(sXML_user_field_input, sPresentation);
        break;

    case FIELD_ID_SEQUENCE:
    {
        // sequence field: name, formula, seq-format
        OUString sName = GetStringProperty(sPropertyVariableName, rPropSet);
        // TODO: use reference name only if actually beeing referenced.
        ProcessString(sXML_ref_name,
                      MakeSequenceRefName(
                          GetInt16Property(sPropertySequenceValue, rPropSet),
                          sName));
        ProcessString(sXML_name, sName);
        ProcessString(sXML_formula,
                      GetStringProperty(sPropertyContent, rPropSet),
                      sPresentation);
        ProcessNumberingType(GetInt16Property(sPropertyNumberingType,
                                              rPropSet));
        ExportElement(sXML_sequence, sPresentation);
        break;
    }

    case FIELD_ID_EXPRESSION:
    {
        // formula field: formula, format&value
        sal_Bool bCmd = GetBoolProperty(sPropertyIsShowFormula, rPropSet);
        ProcessString(sXML_formula,
                      GetStringProperty(sPropertyContent, rPropSet),
                      sPresentation);
        ProcessDisplay(sal_True, bCmd);
        ProcessValueAndType(IsStringField(nToken, rPropSet),
                            GetIntProperty(sPropertyNumberFormat, rPropSet),
                            GetStringProperty(sPropertyContent, rPropSet),
                            sPresentation,
                            GetDoubleProperty(sPropertyValue, rPropSet),
                            !bCmd, !bCmd, !bCmd);
        ExportElement(sXML_expression, sPresentation);
        break;
    }

    case FIELD_ID_TEXT_INPUT:
        // text input field: description and string-value
        ProcessString(sXML_description,
                      GetStringProperty(sPropertyHint, rPropSet));
        ExportElement(sXML_text_input, sPresentation);
        break;

    case FIELD_ID_TIME:
        // all properties (except IsDate) are optional!
        if (xPropSetInfo->hasPropertyByName(sPropertyNumberFormat))
        {
            ProcessValueAndType(sal_False,
                                GetIntProperty(sPropertyNumberFormat,rPropSet),
                                sEmpty, sEmpty, 0.0, // not used
                                sal_False, sal_False, sal_True);
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyDateTimeValue))
        {
            // no value -> current time
            ProcessDateTime(sXML_time_value,
                            GetDateTimeProperty(sPropertyDateTimeValue,
                                                rPropSet),
                            sal_False );
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyDateTime))
        {
            // no value -> current time
            ProcessDateTime(sXML_time_value,
                            GetDateTimeProperty(sPropertyDateTime,rPropSet),
                            sal_False );
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyIsFixed))
        {
            ProcessBoolean(sXML_fixed,
                           GetBoolProperty(sPropertyIsFixed, rPropSet),
                           sal_False);
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyAdjust))
        {
            // adjust value given as integer in minutes
            ProcessDateTime(sXML_time_adjust,
                            GetIntProperty(sPropertyAdjust, rPropSet),
                            sal_False, sal_True, sal_True);
        }
        ExportElement(sXML_time, sPresentation);
        break;

    case FIELD_ID_DATE:
        // all properties (except IsDate) are optional!
        if (xPropSetInfo->hasPropertyByName(sPropertyNumberFormat))
        {
            ProcessValueAndType(sal_False,
                                GetIntProperty(sPropertyNumberFormat,rPropSet),
                                sEmpty, sEmpty, 0.0, // not used
                                sal_False, sal_False, sal_True);
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyDateTimeValue))
        {
            // no value -> current date
            ProcessDateTime(sXML_date_value,
                            GetDateTimeProperty(sPropertyDateTimeValue,
                                                rPropSet),
                            sal_True);
        }
        // TODO: remove double-handling after SRC614
        else if (xPropSetInfo->hasPropertyByName(sPropertyDateTime))
        {
            ProcessDateTime(sXML_date_value,
                            GetDateTimeProperty(sPropertyDateTime,rPropSet),
                            sal_True );
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyIsFixed))
        {
            ProcessBoolean(sXML_fixed,
                           GetBoolProperty(sPropertyIsFixed, rPropSet),
                           sal_False);
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyAdjust))
        {
            // adjust value given as number of days
            ProcessDateTime(sXML_date_adjust,
                            GetIntProperty(sPropertyAdjust, rPropSet),
                            sal_True, sal_True, sal_True);
        }
        ExportElement(sXML_date, sPresentation);
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
                ProcessString(sXML_select_page,
                              MapPageNumberName(rPropSet, nAdjust));
            }
            ProcessInteger(sXML_page_adjust, nAdjust, 0);
        }
        ExportElement(sXML_page_number, sPresentation);
        break;

    case FIELD_ID_PAGESTRING:
    {
        ProcessString(sXML_string_value,
                      GetStringProperty(sPropertyUserText, rPropSet),
                      sPresentation);
        sal_Int32 nDummy = 0; // MapPageNumberName need int
        ProcessString(sXML_select_page, MapPageNumberName(rPropSet, nDummy));
        ExportElement(sXML_page_continuation_string, sPresentation);
        break;
    }

    case FIELD_ID_DATABASE_NAME:
        ProcessString(sXML_database_name,
                      GetStringProperty(sPropertyDataBaseName, rPropSet));
        ProcessString(sXML_table_name,
                      GetStringProperty(sPropertyDataTableName, rPropSet));
        ExportElement(sXML_database_name, sPresentation);
        break;

    case FIELD_ID_DATABASE_NUMBER:
        ProcessString(sXML_database_name,
                      GetStringProperty(sPropertyDataBaseName, rPropSet));
        ProcessString(sXML_table_name,
                      GetStringProperty(sPropertyDataTableName, rPropSet));
        ProcessNumberingType(
            GetInt16Property(sPropertyNumberingType,rPropSet));
        ProcessInteger(sXML_value,
                       GetIntProperty(sPropertySetNumber, rPropSet));
        ExportElement(sXML_database_row_number, sPresentation);
        break;

    case FIELD_ID_DATABASE_NEXT:
        ProcessString(sXML_database_name,
                      GetStringProperty(sPropertyDataBaseName, rPropSet));
        ProcessString(sXML_table_name,
                      GetStringProperty(sPropertyDataTableName, rPropSet));
        ProcessString(sXML_condition,
                      GetStringProperty(sPropertyCondition, rPropSet));
        DBG_ASSERT(sPresentation.equals(sEmpty),
                   "Unexpected presentation for database next field");
        ExportElement(sXML_database_next);
        break;

    case FIELD_ID_DATABASE_SELECT:
        ProcessString(sXML_database_name,
                      GetStringProperty(sPropertyDataBaseName, rPropSet));
        ProcessString(sXML_table_name,
                      GetStringProperty(sPropertyDataTableName, rPropSet));
        ProcessString(sXML_condition,
                      GetStringProperty(sPropertyCondition, rPropSet));
        ProcessInteger(sXML_row_number,
                       GetIntProperty(sPropertySetNumber, rPropSet));
        DBG_ASSERT(sPresentation.equals(sEmpty),
                   "Unexpected presentation for database select field");
        ExportElement(sXML_database_select);
        break;

    case FIELD_ID_DATABASE_DISPLAY:
    {
        // get database, table and column name from field master
        Reference<XPropertySet> & xMaster = GetMasterPropertySet(rTextField);
        ProcessString(sXML_database_name,
                      GetStringProperty(sPropertyDataBaseName, xMaster));
        ProcessString(sXML_table_name,
                      GetStringProperty(sPropertyDataTableName, xMaster));
        ProcessString(sXML_column_name,
                      GetStringProperty(sPropertyDataColumnName, xMaster));
        // export number format if available (happens only for numbers!)
        if (!GetBoolProperty(sPropertyIsDataBaseFormat, rPropSet))
        {
            ProcessValueAndType(sal_False,  // doesn't happen for text
                                GetIntProperty(sPropertyNumberFormat,rPropSet),
                                sEmpty, sEmpty, 0.0, // not used
                                sal_False, sal_False, sal_True);
        }
        ExportElement(sXML_database_display, sPresentation);
        break;
    }

    case FIELD_ID_DOCINFO_REVISION:
        ProcessBoolean(sXML_fixed,
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
                            sal_False, sal_False, sal_True);
        // todo: export date/time value, but values not available -> core bug
        ProcessBoolean(sXML_fixed,
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
        ProcessBoolean(sXML_fixed,
                       GetBoolProperty(sPropertyIsFixed, rPropSet), sal_False);
        ExportElement(MapDocInfoFieldName(nToken), sPresentation);
        break;

    case FIELD_ID_DOCINFO_INFORMATION0:
    case FIELD_ID_DOCINFO_INFORMATION1:
    case FIELD_ID_DOCINFO_INFORMATION2:
    case FIELD_ID_DOCINFO_INFORMATION3:
    {
        Reference<XDocumentInfoSupplier> xDocInfoSupplier(
            GetExport().GetModel(), UNO_QUERY);
        Reference<XDocumentInfo> xDocInfo =xDocInfoSupplier->getDocumentInfo();
        Any aAny;
        ProcessString(sXML_name,
                      xDocInfo->getUserFieldName(nToken -
                                            FIELD_ID_DOCINFO_INFORMATION0));
        ProcessBoolean(sXML_fixed,
                       GetBoolProperty(sPropertyIsFixed, rPropSet), sal_False);
        ExportElement(sXML_user_defined, sPresentation);
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
        ProcessString(sXML_condition,
                      GetStringProperty(sPropertyCondition, rPropSet));
        ProcessString(sXML_string_value_if_true,
                      GetStringProperty(sPropertyTrueContent, rPropSet));
        ProcessString(sXML_string_value_if_false,
                      GetStringProperty(sPropertyFalseContent, rPropSet));
        ExportElement(sXML_conditional_text, sPresentation);
        break;

    case FIELD_ID_HIDDEN_TEXT:
        ProcessString(sXML_condition,
                      GetStringProperty(sPropertyCondition, rPropSet));
        ProcessString(sXML_string_value,
                      GetStringProperty(sPropertyContent, rPropSet));
        ExportElement(sXML_hidden_text, sPresentation);
        break;

    case FIELD_ID_HIDDEN_PARAGRAPH:
        ProcessString(sXML_condition,
                      GetStringProperty(sPropertyCondition, rPropSet));
        DBG_ASSERT(sPresentation.equals(sEmpty),
                   "Unexpected presentation for hidden paragraph field");
        ExportElement(sXML_hidden_paragraph);
        break;

    case FIELD_ID_TEMPLATE_NAME:
        ProcessString(sXML_display,
                      MapTemplateDisplayFormat(
                          GetInt16Property(sPropertyFileFormat, rPropSet)));
        ExportElement(sXML_template_name, sPresentation);
        break;

    case FIELD_ID_CHAPTER:
        ProcessString(sXML_display,
                      MapChapterDisplayFormat(
                          GetInt16Property(sPropertyChapterFormat, rPropSet)));
        // API numbers 0..9, we number 1..10
        ProcessInteger(sXML_outline_level,
                       GetInt8Property(sPropertyLevel, rPropSet) + 1);
        ExportElement(sXML_chapter, sPresentation);
        break;

    case FIELD_ID_FILE_NAME:
        // all properties are optional
        if (xPropSetInfo->hasPropertyByName(sPropertyFileFormat))
        {
            ProcessString(sXML_display,
                          MapFilenameDisplayFormat(
                             GetInt16Property(sPropertyFileFormat, rPropSet)));
        }
        if (xPropSetInfo->hasPropertyByName(sPropertyIsFixed))
        {
            ProcessBoolean(sXML_fixed,
                           GetBoolProperty(sPropertyIsFixed, rPropSet),
                           sal_False);
        }
        ExportElement(sXML_file_name, sPresentation);
        break;

    case FIELD_ID_REFPAGE_SET:
        ProcessBoolean(sXML_active,
                       GetBoolProperty(sPropertyOn, rPropSet), sal_True);
        ProcessInteger(sXML_page_adjust,
                       GetInt16Property(sPropertyOffset, rPropSet), 0);
        DBG_ASSERT(sPresentation.equals(sEmpty),
                   "Unexpected presentation page variable field");
        ExportElement(sXML_page_variable_set);
        break;

    case FIELD_ID_REFPAGE_GET:
        ProcessNumberingType(
            GetInt16Property(sPropertyNumberingType, rPropSet));
        ExportElement(sXML_page_variable_get, sPresentation);
        break;

    case FIELD_ID_MACRO:
        ProcessString(sXML_name, GetStringProperty(sPropertyMacro, rPropSet));
        ProcessString(sXML_description,
                      GetStringProperty(sPropertyHint, rPropSet),
                      sPresentation);
        ExportElement(sXML_execute_macro, sPresentation);
        break;

    case FIELD_ID_REF_SEQUENCE:
        // reference to sequence: format, name, find value (and element)
        // was: if (nSeqNumber != -1) ...
        ProcessString(sXML_reference_format,
                      MapReferenceType(GetInt16Property(
                          sPropertyReferenceFieldPart, rPropSet)),
                      sXML_template);
        ProcessString(sXML_ref_name,
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
        ProcessString(sXML_reference_format,
                      MapReferenceType(GetInt16Property(
                          sPropertyReferenceFieldPart, rPropSet)),
                      sXML_template);
        ProcessString(sXML_ref_name,
                      GetStringProperty(sPropertySourceName, rPropSet));
        ExportElement(
            MapReferenceSource(GetInt16Property(
                sPropertyReferenceFieldSource, rPropSet)),
            sPresentation);
        break;

    case FIELD_ID_REF_FOOTNOTE:
    case FIELD_ID_REF_ENDNOTE:
        // reference to end-/footnote: format, generate name, (and element)
        ProcessString(sXML_reference_format,
                      MapReferenceType(GetInt16Property(
                          sPropertyReferenceFieldPart, rPropSet)),
                      sXML_template);
        ProcessString(sXML_ref_name,
                      MakeFootnoteRefName(GetInt16Property(
                          sPropertySequenceNumber, rPropSet)));
        ExportElement(
            MapReferenceSource(GetInt16Property(
                sPropertyReferenceFieldSource, rPropSet)),
            sPresentation);
        break;

    case FIELD_ID_DDE:
        // name from field master
         ProcessString(sXML_connection_name,

                       GetStringProperty(sPropertyName,
                                         GetMasterPropertySet(rTextField)));
        ExportElement(sXML_dde_connection, sPresentation);
        break;

    case FIELD_ID_SHEET_NAME:
        // name of spreadsheet (Calc only)
        ExportElement(sXML_sheet_name, sPresentation);
        break;

    case FIELD_ID_URL:
    {
        // this field is a special case because it gets mapped onto a
        // hyperlink, rather than one of the regular text field.
        ProcessString(sXML_href, GetStringProperty(sPropertyURL, rPropSet),
                      sal_True, XML_NAMESPACE_XLINK);
        ProcessString(sXML_target_frame_name,
                      GetStringProperty(sPropertyTargetFrame,rPropSet),
                      sal_True, XML_NAMESPACE_OFFICE);
        SvXMLElementExport aUrlField(rExport, XML_NAMESPACE_TEXT, sXML_a,
                                     sal_False, sal_False);
        GetExport().GetDocHandler()->characters(sPresentation);
        break;
    }

    case FIELD_ID_BIBLIOGRAPHY:
    {
        ProcessBibliographyData(rPropSet);
        ExportElement(sXML_bibliography_mark, sPresentation);
        break;
    }

    case FIELD_ID_SCRIPT:
        ProcessString(sXML_language,
                      GetStringProperty(sPropertyScriptType, rPropSet),
                      sal_True, XML_NAMESPACE_SCRIPT);
        DBG_ASSERT(sPresentation.equals(sEmpty),
                   "Unexpected presentation for script field");
        if (GetBoolProperty(sPropertyURLContent, rPropSet))
        {
            ProcessString(sXML_href,
                          GetStringProperty(sPropertyContent, rPropSet),
                          sal_False, XML_NAMESPACE_XLINK);
            ExportElement(sXML_script);
        }
        else
        {
            ExportElement(sXML_script,
                          GetStringProperty(sPropertyContent, rPropSet));
        }
        break;

    case FIELD_ID_ANNOTATION:
    {
        // author
        ProcessString(sXML_author,
                      GetStringProperty(sPropertyAuthor, rPropSet),
                      sal_True, XML_NAMESPACE_OFFICE);

        // date time
        ProcessDate(sXML_create_date,
                    GetDateProperty(sPropertyDate, rPropSet),
                    XML_NAMESPACE_OFFICE);

        // check for empty presentation (just in case)
        DBG_ASSERT(sPresentation.equals(sEmpty),
                   "Unexpected presentation for annotation field");

        // annotation element + content
        SvXMLElementExport aElem(GetExport(), XML_NAMESPACE_OFFICE,
                                 sXML_annotation, sal_False, sal_True);
        ProcessParagraphSequence(GetStringProperty(sPropertyContent,rPropSet));
        break;
    }

    case FIELD_ID_COMBINED_CHARACTERS:
    {
        // get style name for current style + combine letters and export a span
        ProcessString(sXML_style_name,
                      GetExport().GetTextParagraphExport()->Find(
                          XML_STYLE_FAMILY_TEXT_TEXT, rRangePropSet,
                          sEmpty, pCombinedCharactersPropertyState) );
        ExportElement(sXML_span, sPresentation);
        break;
    }

    case FIELD_ID_MEASURE:
    {
        ProcessString(sXML_kind, MapMeasureKind(GetInt16Property(sPropertyMeasureKind, rPropSet)));
        ExportElement( sXML_measure, sPresentation );
        break;
    }

    case FIELD_ID_UNKNOWN:
    default:
        DBG_ERROR("unkown field type encountered!");
        // always export content
        GetExport().GetDocHandler()->characters(sPresentation);
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
                     aSetIter++, i++ )
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
        if (0 == sFieldMaster.compareTo( OUString::createFromAscii(sDB),
                                         sizeof(sDB)-1))
        {
            break;
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
    if (aVarName.size() > 0)
    {
        SvXMLElementExport aElem( GetExport(),
                                  XML_NAMESPACE_TEXT,
                                  sXML_variable_decls,
                                  sal_True, sal_True );

        for (vector<OUString>::iterator aVarIter = aVarName.begin();
             aVarIter != aVarName.end();
             aVarIter++) {

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
                    sal_False, sal_True, sal_False);
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
                    sal_False, sal_True, sal_False);
            }

            ProcessString(sXML_name, sVarName);
            ExportElement(sXML_variable_decl, sal_True);
        }
    }
    // else: no declarations element

    // sequence field masters:
    if (aSeqName.size() > 0)
    {
        SvXMLElementExport aElem( GetExport(),
                                  XML_NAMESPACE_TEXT,
                                  sXML_sequence_decls,
                                  sal_True, sal_True );

        for (vector<OUString>::iterator aSeqIter = aSeqName.begin();
             aSeqIter != aSeqName.end();
             aSeqIter++) {

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
            ProcessInteger(sXML_display_outline_level, nLevel);

            // separation character
            if (nLevel > 0) {
                ProcessString(sXML_separation_character, GetStringProperty(
                    sPropertyNumberingSeparator, xPropSet));
            }
            ProcessString(sXML_name, sVarName);
            ExportElement(sXML_sequence_decl, sal_True);
        }
    }
    // else: no declarations element

    // user field field masters:
    if (aUserName.size() > 0)
    {
        SvXMLElementExport aElem( GetExport(),
                                  XML_NAMESPACE_TEXT,
                                  sXML_user_field_decls,
                                  sal_True, sal_True );

        for (vector<OUString>::iterator aUserIter = aUserName.begin();
             aUserIter != aUserName.end();
             aUserIter++) {

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
                    sal_False);
            }
            else
            {
                // string: write regardless of default
                ProcessString(sXML_value_type, sXML_string);
                ProcessString(sXML_string_value,
                              GetStringProperty(sPropertyContent, xPropSet));
            }
            ProcessString(sXML_name, sVarName);
            ExportElement(sXML_user_field_decl, sal_True);
        }
    }
    // else: no declarations element

    // DDE field field masters:
    if (aDdeName.size() > 0)
    {
        SvXMLElementExport aElem( GetExport(),
                                  XML_NAMESPACE_TEXT,
                                  sXML_dde_connection_decls,
                                  sal_True, sal_True );

        for (vector<OUString>::iterator aDdeIter = aDdeName.begin();
             aDdeIter != aDdeName.end();
             aDdeIter++)
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

                ProcessString(sXML_name,
                              GetStringProperty(sPropertyName, xPropSet));

                // export elements; can't use ProcessString because
                // elements are in office namespace
                ProcessString(sXML_dde_application,
                              GetStringProperty(sPropertyDDECommandType,
                                                xPropSet),
                              sal_False, XML_NAMESPACE_OFFICE);
                ProcessString(sXML_dde_topic,
                              GetStringProperty(sPropertyDDECommandFile,
                                                xPropSet),
                              sal_False, XML_NAMESPACE_OFFICE);
                ProcessString(sXML_dde_item,
                              GetStringProperty(sPropertyDDECommandElement,
                                                xPropSet),
                              sal_False, XML_NAMESPACE_OFFICE);
                sal_Bool bIsAutomaticUpdate = GetBoolProperty(
                    sPropertyIsAutomaticUpdate, xPropSet);
                if (bIsAutomaticUpdate)
                {
                    GetExport().AddAttributeASCII(XML_NAMESPACE_OFFICE,
                                                  sXML_automatic_update,
                                                  sXML_true);
                }

                ExportElement(sXML_dde_connection_decl, sal_True);
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

void XMLTextFieldExport::ExportElement(const sal_Char* pElementName,
                                       sal_Bool bAddSpace)
{
    // can't call ExportElement(sal_Char*, const OUString&) with empty
    // string because xmlprinter only uses empty tags if no content
    // (not even empty content) was written.

    DBG_ASSERT(NULL != pElementName, "invalid element name!");
    if (NULL != pElementName)
    {
        // Element
        SvXMLElementExport aElem( GetExport(),
                                  XML_NAMESPACE_TEXT,
                                  pElementName,
                                  bAddSpace, bAddSpace );
    } // else: ignore
}

void XMLTextFieldExport::ExportElement(const sal_Char* pElementName,
                                       const OUString& sContent,
                                       sal_Bool bAddSpace)
{
    DBG_ASSERT(NULL != pElementName, "invalid element name!");
    if (NULL != pElementName)
    {
        // Element
        SvXMLElementExport aElem( GetExport(),
                                  XML_NAMESPACE_TEXT,
                                  pElementName,
                                  bAddSpace, bAddSpace );
        // export content
        GetExport().GetDocHandler()->characters(sContent);
    } else {
        // always export content
        GetExport().GetDocHandler()->characters(sContent);
    }
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
    sal_Bool bExportStyle)  /// export style-sttribute?
{
    // String or number?
    if (bIsString)
    {

        // string: attributes value-type=string, string-value=...

        if (bExportValue || bExportValueType)
        {
            XMLNumberFormatAttributesExportHelper::SetNumberFormatAttributes(
                GetExport(), sContent, sDefault, XML_NAMESPACE_TEXT,
                bExportValue);
        }

    }
    else
    {

        // number: value-type=..., value...=..., data-style-name=...

        DBG_ASSERT(bExportValueType || ~bExportValue,
                   "value w/o value type not supported!");

        // take care of illegal formats
        // (shouldn't happen, but does if document is corrupted)
        if (-1 != nFormatKey)
        {
            if (bExportValue || bExportValueType)
            {
                XMLNumberFormatAttributesExportHelper::
                    SetNumberFormatAttributes(
                        GetExport(), nFormatKey, fValue, XML_NAMESPACE_TEXT,
                        bExportValue);
            }

            if (bExportStyle)
            {
                GetExport().AddAttribute(XML_NAMESPACE_STYLE,
                                         sXML_data_style_name,
                                         GetExport().getDataStyleName(
                                             nFormatKey));
            }  // else: ignore (no number format)
        }
    }
}



/// process display related properties
void XMLTextFieldExport::ProcessDisplay(sal_Bool bIsVisible,
                                        sal_Bool bIsCommand,
                                        sal_Bool bValueDefault)
{
    sal_Char* pValue;

    if (bIsVisible)
    {
        pValue = bIsCommand ? sXML_formula : sXML_value;
    }
    else
    {
        pValue = sXML_none;
    }

    // omit attribute if default
    if (!bValueDefault || (pValue != sXML_value))
    {
        GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT,
                                      sXML_display,
                                      pValue);
    }
}



/// export boolean property
void XMLTextFieldExport::ProcessBoolean(const sal_Char* pXmlName,
                                        sal_Bool bBool, sal_Bool bDefault)
{
    DBG_ASSERT(NULL!=pXmlName, "invalid element name");
    if (NULL == pXmlName) {
        return;
    }

    // write attribute (if different than default)
    // negate to force 0/1 values (and make sal_Bool comparable)
    if ((!bBool) != (!bDefault)) {
        GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT,
                                      pXmlName,
                                      (bBool ? sXML_true : sXML_false) );
    }
}




/// export string attribute
void XMLTextFieldExport::ProcessString(const sal_Char* pXmlName,
                                       const OUString& sValue,
                                       sal_Bool bOmitEmpty,
                                       sal_uInt16 nPrefix)
{
    DBG_ASSERT(NULL!=pXmlName, "invalid element name");
    if (NULL == pXmlName) {
        return;
    }

    // check for empty string, if applicable
    if (bOmitEmpty && (sValue.getLength()==0)) {
        return;
    }

    // write attribute
    GetExport().AddAttribute(nPrefix, pXmlName, sValue);
}

/// export a string attribute
void XMLTextFieldExport::ProcessString(const sal_Char* pXmlName,
                                       const ::rtl::OUString& sValue,
                                       const ::rtl::OUString& sDefault,
                                       sal_uInt16 nPrefix)
{
    if (sValue != sDefault)
    {
        ProcessString(pXmlName, sValue, sal_False, nPrefix);
    }
}


/// export string attribute
void XMLTextFieldExport::ProcessString(
    const sal_Char* pXmlName,
    const sal_Char* pValue,
    sal_Bool bOmitEmpty,
    sal_uInt16 nPrefix)
{
    DBG_ASSERT(NULL != pXmlName, "invalid element name");
    DBG_ASSERT(NULL != pValue, "invalid value name");
    if ((NULL == pXmlName) || (NULL == pValue)) {
        return;
    }

    // check for empty string, if applicable
    if (bOmitEmpty && (0 == pValue[0])) {
        return;
    }

    GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT, pXmlName, pValue);
}

/// export a string attribute
void XMLTextFieldExport::ProcessString(
    const sal_Char* pXmlName,
    const sal_Char* pValue,
    const sal_Char* pDefault,
    sal_uInt16 nPrefix)
{
    // save comparisons if pointers are equals.  This will happen
    // frequently, as almost every code in here uses sXML_* constants.
    if ((pValue != pDefault) && (0 != strcmp(pValue, pDefault)))
    {
        ProcessString(pXmlName, pValue, sal_False, nPrefix);
    }
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
            GetExport(), XML_NAMESPACE_TEXT, sXML_p, sal_True, sal_False);
        GetExport().GetDocHandler()->characters(aSubString);
    }
}

// export an integer attribute
void XMLTextFieldExport::ProcessInteger(const sal_Char* pXmlName,
                                        sal_Int32 nNum)
{
    DBG_ASSERT(NULL != pXmlName, "invalid element name");

    if (NULL == pXmlName) {
        return;
    }

    GetExport().AddAttribute(XML_NAMESPACE_TEXT, pXmlName,
                             OUString::valueOf(nNum));
}

/// export an integer attribute, omit if default
void XMLTextFieldExport::ProcessInteger(const sal_Char* pXmlName,
                                        sal_Int32 nNum, sal_Int32 nDefault)
{
    if (nNum != nDefault)
    {
        ProcessInteger(pXmlName, nNum);
    }
}



/// export a numbering type
void XMLTextFieldExport::ProcessNumberingType(sal_Int16 nNumberingType)
{
    // process only if real format (not: like page descriptor)
    if (NumberingType::PAGE_DESCRIPTOR != nNumberingType)
    {
        // number type: num format
        GetExport().AddAttributeASCII(XML_NAMESPACE_STYLE,
                                      sXML_num_format,
                                      SvxXMLNumRuleExport::GetNumFormatValue(
                                          nNumberingType));
        // and letter sync, if applicable
        const sal_Char* pLetterSync =
            SvxXMLNumRuleExport::GetNumLetterSync(nNumberingType);

        if (NULL != pLetterSync)
        {
            GetExport().AddAttributeASCII(XML_NAMESPACE_STYLE,
                                          sXML_num_letter_sync,
                                          pLetterSync);
        }
    }
    // else: like page descriptor => ignore
}


/// export a date, time, or duration
void XMLTextFieldExport::ProcessDateTime(const sal_Char* sXMLName,
                                         double dValue,
                                         sal_Bool bIsDate,
                                         sal_Bool bIsDuration,
                                         sal_Bool bOmitDurationIfZero,
                                         sal_uInt16 nPrefix)
{
    // truncate for date granularity
    if (bIsDate)
    {
        dValue = SolarMath::ApproxFloor(dValue);
    }

    OUStringBuffer aBuffer;
    if (bIsDuration)
    {
        // date/time durationM handle bOmitDurationIfZero
        if (!bOmitDurationIfZero || !SolarMath::ApproxEqual(dValue, 0.0))
        {
            rExport.GetMM100UnitConverter().convertTime(aBuffer, dValue);
        }
    }
    else
    {
        // date/time value
        rExport.GetMM100UnitConverter().convertDateTime(aBuffer, dValue);
    }

    // output attribute
    ProcessString(sXMLName, aBuffer.makeStringAndClear(), sal_True, nPrefix);
}

/// export a date or time
void XMLTextFieldExport::ProcessDateTime(const sal_Char* sXMLName,
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
    rExport.GetMM100UnitConverter().convertDateTime(aBuffer, aDateTime);

    // output attribute
    ProcessString(sXMLName, aBuffer.makeStringAndClear(), sal_True, nPrefix);
}

/// export date according to ISO 8601
void XMLTextFieldExport::ProcessDate(
    const sal_Char* sXMLName,
    const ::com::sun::star::util::Date& rDate,
    sal_uInt16 nPrefix)
{
    // the easiest way: delegate to ProcessDateTime (as date)
    DateTime aDateTime;
    aDateTime.Day = rDate.Day;
    aDateTime.Month = rDate.Month;
    aDateTime.Year = rDate.Year;
    ProcessDateTime(sXMLName, aDateTime, sal_True, nPrefix);
}


/// export a date, time, or duration
void XMLTextFieldExport::ProcessDateTime(const sal_Char* sXMLName,
                                         sal_Int32 nMinutes,
                                         sal_Bool bIsDate,
                                         sal_Bool bIsDuration,
                                         sal_Bool bOmitDurationIfZero,
                                         sal_uInt16 nPrefix)
{
    // handle bOmitDurationIfZero here, because we can precisely compare ints
    if (!(bIsDuration && bOmitDurationIfZero && (nMinutes==0)))
    {
        ProcessDateTime(sXMLName, (double)nMinutes / (double)(24*60),
                        bIsDate, bIsDuration, bOmitDurationIfZero, nPrefix);
    }
}


SvXMLEnumMapEntry __READONLY_DATA aBibliographyDataTypeMap[] =
{
    { sXML_article,         BibliographyDataType::ARTICLE },
    { sXML_book,            BibliographyDataType::BOOK },
    { sXML_booklet,         BibliographyDataType::BOOKLET },
    { sXML_conference,      BibliographyDataType::CONFERENCE },
    { sXML_custom1,         BibliographyDataType::CUSTOM1 },
    { sXML_custom2,         BibliographyDataType::CUSTOM2 },
    { sXML_custom3,         BibliographyDataType::CUSTOM3 },
    { sXML_custom4,         BibliographyDataType::CUSTOM4 },
    { sXML_custom5,         BibliographyDataType::CUSTOM5 },
    { sXML_email,           BibliographyDataType::EMAIL },
    { sXML_inbook,          BibliographyDataType::INBOOK },
    { sXML_incollection,    BibliographyDataType::INCOLLECTION },
    { sXML_inproceedings,   BibliographyDataType::INPROCEEDINGS },
    { sXML_journal,         BibliographyDataType::JOURNAL },
    { sXML_manual,          BibliographyDataType::MANUAL },
    { sXML_mastersthesis,   BibliographyDataType::MASTERSTHESIS },
    { sXML_misc,            BibliographyDataType::MISC },
    { sXML_phdthesis,       BibliographyDataType::PHDTHESIS },
    { sXML_proceedings,     BibliographyDataType::PROCEEDINGS },
    { sXML_techreport,      BibliographyDataType::TECHREPORT },
    { sXML_unpublished,     BibliographyDataType::UNPUBLISHED },
    { sXML_www,             BibliographyDataType::WWW },
    { 0, 0 }
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
            sal_Int16 nTypeId;
            aValues[i].Value >>= nTypeId;
            OUStringBuffer sBuf;

            if (SvXMLUnitConverter::convertEnum(sBuf, nTypeId,
                                                aBibliographyDataTypeMap))
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     sXML_bibiliographic_type,
                                     sBuf.makeStringAndClear());
            }
            // else: ignore this argument
        }
        else
        {
            OUString sStr;
            aValues[i].Value >>= sStr;

            if (sStr.getLength() > 0)
            {
                rExport.AddAttribute(XML_NAMESPACE_TEXT,
                                     MapBibliographyFieldName(aValues[i].Name),
                                     sStr);
            }
        }
    }
}


// explode a field master name into field type and field name
sal_Bool XMLTextFieldExport::ExplodeFieldMasterName(
    const OUString& sMasterName, OUString& sFieldType, OUString& sVarName)
{
    sal_Int32 nLength = sFieldMasterPrefix.getLength();
    sal_Int32 nSeparator = sMasterName.indexOf('.', nLength);
    sal_Bool bReturn = sal_True;

#ifndef PRODUCT
    // check for service name
    bReturn &= (0 == sFieldMasterPrefix.compareTo(sMasterName, nLength));
#endif

    // '.' found?
    if (nSeparator <= nLength) {
        nSeparator = sMasterName.getLength();
        DBG_WARNING("no field var name!");
        bReturn = sal_False;
    }

    sFieldType = sMasterName.copy(nLength, nSeparator-nLength);
    sVarName = sMasterName.copy(nSeparator+1);

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
const sal_Char* XMLTextFieldExport::MapPlaceholderType(sal_uInt16 nType)
{
    sal_Char* pTypeString = sXML_text;

    switch (nType) {
    case PlaceholderType::TEXT:
        pTypeString = sXML_text;
        break;

    case PlaceholderType::TABLE:
        pTypeString = sXML_table;
        break;

    case PlaceholderType::TEXTFRAME:
        pTypeString = sXML_text_box;
        break;

    case PlaceholderType::GRAPHIC:
        pTypeString = sXML_image;
        break;

    case PlaceholderType::OBJECT:
        pTypeString = sXML_object;
        break;

    default:
        // unkown placeholder: sXML_text
        DBG_ERROR("unkown placeholder type");
    }

    return pTypeString;
}


/// element name for author fields
const sal_Char* XMLTextFieldExport::MapAuthorFieldName(
    const Reference<XPropertySet> & xPropSet)
{
    // Initalen oder voller Name?
    return GetBoolProperty(sPropertyAuthorFullname, xPropSet)
        ? sXML_author_name : sXML_author_initials;
}

const sal_Char* XMLTextFieldExport::MapPageNumberName(
    const Reference<XPropertySet> & xPropSet,
    sal_Int32& nOffset)
{
    sal_Char* pName = NULL;
    PageNumberType ePage;
    Any aAny = xPropSet->getPropertyValue(sPropertySubType);
    ePage = *(PageNumberType*)aAny.getValue();

    switch (ePage)
    {
        case PageNumberType_PREV:
            pName = sXML_previous;
            nOffset += 1;
            break;
        case PageNumberType_CURRENT:
            pName = sXML_current;
            break;
        case PageNumberType_NEXT:
            pName = sXML_next;
            nOffset -= 1;
            break;
        default:
            DBG_ERROR("unknown page number type");
            pName = NULL;
            break;
    }

    return pName;
}

/// map TemplateDisplayFormat to XML
const sal_Char* XMLTextFieldExport::MapTemplateDisplayFormat(sal_Int16 nFormat)
{
    sal_Char* pName = NULL;

    switch (nFormat)
    {
        case TemplateDisplayFormat::FULL:
            pName = sXML_full;
            break;
        case TemplateDisplayFormat::PATH:
            pName = sXML_path;
            break;
        case TemplateDisplayFormat::NAME:
            pName = sXML_name;
            break;
        case TemplateDisplayFormat::NAME_AND_EXT:
            pName = sXML_name_and_extension;
            break;
        case TemplateDisplayFormat::AREA:
            pName = sXML_area;
            break;
        case TemplateDisplayFormat::TITLE:
            pName = sXML_title;
            break;
        default:
            DBG_ERROR("unknown template display format");
            pName = NULL;
            break;
    }

    return pName;
}

/// map count/statistics field token to XML name
const sal_Char* XMLTextFieldExport::MapCountFieldName(sal_Int16 nToken)
{
    const sal_Char* pElementName = NULL;

    switch (nToken)
    {
        case FIELD_ID_COUNT_PAGES:
            pElementName = sXML_page_count;
            break;
        case FIELD_ID_COUNT_PARAGRAPHS:
            pElementName = sXML_paragraph_count;
            break;
        case FIELD_ID_COUNT_WORDS:
            pElementName = sXML_word_count;
            break;
        case FIELD_ID_COUNT_CHARACTERS:
            pElementName = sXML_character_count;
            break;
        case FIELD_ID_COUNT_TABLES:
            pElementName = sXML_table_count;
            break;
        case FIELD_ID_COUNT_GRAPHICS:
            pElementName = sXML_image_count;
            break;
        case FIELD_ID_COUNT_OBJECTS:
            pElementName = sXML_object_count;
            break;
        default:
            DBG_ERROR("no count field token");
            pElementName = NULL;
            break;
    }

    return pElementName;
}

/// map ChapterDisplayFormat to XML string
const sal_Char* XMLTextFieldExport::MapChapterDisplayFormat(sal_Int16 nFormat)
{
    const sal_Char* pName = NULL;

    switch (nFormat)
    {
        case ChapterFormat::NAME:
            pName = sXML_name;
            break;
        case ChapterFormat::NUMBER:
            pName = sXML_number;
            break;
        case ChapterFormat::NAME_NUMBER:
            pName = sXML_number_and_name;
            break;
        case ChapterFormat::NO_PREFIX_SUFFIX:
            pName = sXML_plain_number_and_name;
            break;
        case ChapterFormat::DIGIT:
            pName = sXML_plain_number;
            break;
        default:
            DBG_ERROR("unkown chapter display format");
            pName = NULL;
            break;
    }

    return pName;
}


/// map FilenameDisplayFormat to XML attribute names
const sal_Char* XMLTextFieldExport::MapFilenameDisplayFormat(sal_Int16 nFormat)
{
    sal_Char* pName = NULL;

    switch (nFormat)
    {
        case FilenameDisplayFormat::FULL:
            pName = sXML_full;
            break;
        case FilenameDisplayFormat::PATH:
            pName = sXML_path;
            break;
        case FilenameDisplayFormat::NAME:
            pName = sXML_name;
            break;
        case FilenameDisplayFormat::NAME_AND_EXT:
            pName = sXML_name_and_extension;
            break;
        default:
            DBG_ERROR("unknown filename display format");
    }

    return pName;
}


/// map ReferenceFieldPart to XML string
const sal_Char* XMLTextFieldExport::MapReferenceType(sal_Int16 nType)
{
    sal_Char* pElementName = NULL;

    switch (nType)
    {
        case ReferenceFieldPart::PAGE:
            pElementName = sXML_page;
            break;
        case ReferenceFieldPart::CHAPTER:
            pElementName = sXML_chapter;
            break;
        case ReferenceFieldPart::TEXT:
            pElementName = sXML_text;
            break;
        case ReferenceFieldPart::UP_DOWN:
            pElementName = sXML_direction;
            break;
        case ReferenceFieldPart::CATEGORY_AND_NUMBER:
            pElementName = sXML_category_and_value;
            break;
        case ReferenceFieldPart::ONLY_CAPTION:
            pElementName = sXML_caption;
            break;
        case ReferenceFieldPart::ONLY_SEQUENCE_NUMBER:
            pElementName = sXML_value;
            break;
        case ReferenceFieldPart::PAGE_DESC:
            // small hack: this value never gets written, because
            // sXML_template is default
            pElementName = sXML_template;
            break;
        default:
            DBG_ERROR("unknown reference type");
            pElementName = sXML_template;
            break;
    }

    return pElementName;
}

/// map ReferenceFieldPart to XML string
const sal_Char* XMLTextFieldExport::MapReferenceSource(sal_Int16 nType)
{
    sal_Char* pElementName = NULL;

    switch (nType)
    {
        case ReferenceFieldSource::REFERENCE_MARK:
            pElementName = sXML_reference_ref;
            break;
        case ReferenceFieldSource::SEQUENCE_FIELD:
            pElementName = sXML_sequence_ref;
            break;
        case ReferenceFieldSource::BOOKMARK:
            pElementName = sXML_bookmark_ref;
            break;
        case ReferenceFieldSource::FOOTNOTE:
            pElementName = sXML_footnote_ref;
            break;
        case ReferenceFieldSource::ENDNOTE:
            pElementName = sXML_endnote_ref;
            break;
        default:
            DBG_ERROR("unkown reference source");
            break;
    }

    return pElementName;
}


/// element name for sender fields
const sal_Char* XMLTextFieldExport::MapSenderFieldName(
    const Reference<XPropertySet> & xPropSet)
{
    sal_Char* pElementName = NULL;

    // sub-field type
    switch (GetInt16Property(sPropertyFieldSubType, xPropSet))
    {
        case UserDataPart::COMPANY :
            pElementName = sXML_sender_company;
            break;
        case UserDataPart::FIRSTNAME :
            pElementName = sXML_sender_firstname;
            break;
        case UserDataPart::NAME :
            pElementName = sXML_sender_lastname;
            break;
        case UserDataPart::SHORTCUT :
            pElementName = sXML_sender_initials;
            break;
        case UserDataPart::STREET :
            pElementName = sXML_sender_street;
            break;
        case UserDataPart::COUNTRY :
            pElementName = sXML_sender_country;
            break;
        case UserDataPart::ZIP :
            pElementName = sXML_sender_postal_code;
            break;
        case UserDataPart::CITY :
            pElementName = sXML_sender_city;
            break;
        case UserDataPart::TITLE :
            pElementName = sXML_sender_title;
            break;
        case UserDataPart::POSITION :
            pElementName = sXML_sender_position;
            break;
        case UserDataPart::PHONE_PRIVATE :
            pElementName = sXML_sender_phone_private;
            break;
        case UserDataPart::PHONE_COMPANY :
            pElementName = sXML_sender_phone_work;
            break;
        case UserDataPart::FAX :
            pElementName = sXML_sender_fax;
            break;
        case UserDataPart::EMAIL :
            pElementName = sXML_sender_email;
            break;
        case UserDataPart::STATE :
            pElementName = sXML_sender_state_or_province;
            break;
        default:
            DBG_WARNING("unknown sender type");
            pElementName = NULL;
            break;
    }

    return pElementName;
}

const sal_Char* XMLTextFieldExport::MapDocInfoFieldName(
    enum FieldIdEnum nToken)
{
    sal_Char* pElementName = NULL;

    switch (nToken)
    {
        case FIELD_ID_DOCINFO_CREATION_AUTHOR:
            pElementName = sXML_initial_creator;
            break;
        case FIELD_ID_DOCINFO_CREATION_DATE:
            pElementName = sXML_creation_date;
            break;
        case FIELD_ID_DOCINFO_CREATION_TIME:
            pElementName = sXML_creation_time;
            break;
        case FIELD_ID_DOCINFO_DESCRIPTION:
            pElementName = sXML_description;
            break;
        case FIELD_ID_DOCINFO_INFORMATION0:
            pElementName = sXML_user_info_0;
            break;
        case FIELD_ID_DOCINFO_INFORMATION1:
            pElementName = sXML_user_info_1;
            break;
        case FIELD_ID_DOCINFO_INFORMATION2:
            pElementName = sXML_user_info_2;
            break;
        case FIELD_ID_DOCINFO_INFORMATION3:
            pElementName = sXML_user_info_3;
            break;
        case FIELD_ID_DOCINFO_PRINT_TIME:
            pElementName = sXML_print_time;
            break;
        case FIELD_ID_DOCINFO_PRINT_DATE:
            pElementName = sXML_print_date;
            break;
        case FIELD_ID_DOCINFO_PRINT_AUTHOR:
            pElementName = sXML_printed_by;
            break;
        case FIELD_ID_DOCINFO_TITLE:
            pElementName = sXML_title;
            break;
        case FIELD_ID_DOCINFO_SUBJECT:
            pElementName = sXML_subject;
            break;
        case FIELD_ID_DOCINFO_KEYWORDS:
            pElementName = sXML_keywords;
            break;
        case FIELD_ID_DOCINFO_REVISION:
            pElementName = sXML_editing_cycles;
            break;
        case FIELD_ID_DOCINFO_EDIT_DURATION:
            pElementName = sXML_editing_duration;
            break;
        case FIELD_ID_DOCINFO_SAVE_TIME:
            pElementName = sXML_modification_time;
            break;
        case FIELD_ID_DOCINFO_SAVE_DATE:
            pElementName = sXML_modification_date;
            break;
        case FIELD_ID_DOCINFO_SAVE_AUTHOR:
            pElementName = sXML_creator;
            break;
        default:
            DBG_WARNING("unknown docinfo field type!");
            pElementName = NULL;
            break;
    }

    return pElementName;
}

const sal_Char* XMLTextFieldExport::MapBibliographyFieldName(OUString sName)
{
    sal_Char* pName = NULL;

    if (sName.equalsAsciiL("Identifier", sizeof("Identifier")-1))
    {
        pName = sXML_identifier;
    }
    else if (sName.equalsAsciiL("BibiliographicType",
                                sizeof("BibiliographicType")-1))
    {
        pName = sXML_bibiliographic_type;
    }
    else if (sName.equalsAsciiL("Address", sizeof("Address")-1))
    {
        pName = sXML_address;
    }
    else if (sName.equalsAsciiL("Annote", sizeof("Annote")-1))
    {
        pName = sXML_annote;
    }
    else if (sName.equalsAsciiL("Author", sizeof("Author")-1))
    {
        pName = sXML_author;
    }
    else if (sName.equalsAsciiL("Booktitle", sizeof("Booktitle")-1))
    {
        pName = sXML_booktitle;
    }
    else if (sName.equalsAsciiL("Chapter", sizeof("Chapter")-1))
    {
        pName = sXML_chapter;
    }
    else if (sName.equalsAsciiL("Edition", sizeof("Edition")-1))
    {
        pName = sXML_edition;
    }
    else if (sName.equalsAsciiL("Editor", sizeof("Editor")-1))
    {
        pName = sXML_editor;
    }
    else if (sName.equalsAsciiL("Howpublished", sizeof("Howpublished")-1))
    {
        pName = sXML_howpublished;
    }
    else if (sName.equalsAsciiL("Institution", sizeof("Institution")-1))
    {
        pName = sXML_institution;
    }
    else if (sName.equalsAsciiL("Journal", sizeof("Journal")-1))
    {
        pName = sXML_journal;
    }
    else if (sName.equalsAsciiL("Month", sizeof("Month")-1))
    {
        pName = sXML_month;
    }
    else if (sName.equalsAsciiL("Note", sizeof("Note")-1))
    {
        pName = sXML_note;
    }
    else if (sName.equalsAsciiL("Number", sizeof("Number")-1))
    {
        pName = sXML_number;
    }
    else if (sName.equalsAsciiL("Organizations", sizeof("Organizations")-1))
    {
        pName = sXML_organizations;
    }
    else if (sName.equalsAsciiL("Pages", sizeof("Pages")-1))
    {
        pName = sXML_pages;
    }
    else if (sName.equalsAsciiL("Publisher", sizeof("Publisher")-1))
    {
        pName = sXML_publisher;
    }
    else if (sName.equalsAsciiL("School", sizeof("School")-1))
    {
        pName = sXML_school;
    }
    else if (sName.equalsAsciiL("Series", sizeof("Series")-1))
    {
        pName = sXML_series;
    }
    else if (sName.equalsAsciiL("Title", sizeof("Title")-1))
    {
        pName = sXML_title;
    }
    else if (sName.equalsAsciiL("Report_Type", sizeof("Report_Type")-1))
    {
        pName = sXML_report_type;
    }
    else if (sName.equalsAsciiL("Volume", sizeof("Volume")-1))
    {
        pName = sXML_volume;
    }
    else if (sName.equalsAsciiL("Year", sizeof("Year")-1))
    {
        pName = sXML_year;
    }
    else if (sName.equalsAsciiL("URL", sizeof("URL")-1))
    {
        pName = sXML_url;
    }
    else if (sName.equalsAsciiL("Custom1", sizeof("Custom1")-1))
    {
        pName = sXML_custom1;
    }
    else if (sName.equalsAsciiL("Custom2", sizeof("Custom2")-1))
    {
        pName = sXML_custom2;
    }
    else if (sName.equalsAsciiL("Custom3", sizeof("Custom3")-1))
    {
        pName = sXML_custom3;
    }
    else if (sName.equalsAsciiL("Custom4", sizeof("Custom4")-1))
    {
        pName = sXML_custom4;
    }
    else if (sName.equalsAsciiL("Custom5", sizeof("Custom5")-1))
    {
        pName = sXML_custom5;
    }
    else if (sName.equalsAsciiL("ISBN", sizeof("ISBN")-1))
    {
        pName = sXML_isbn;
    }
    else
    {
        DBG_ERROR("Unknown bibliography info data");
        pName = NULL;
    }

    return pName;
}

const sal_Char* XMLTextFieldExport::MapMeasureKind(sal_Int16 nKind)
{
    switch( nKind )
    {
    case 0:
        return sXML_value;
    case 1:
        return sXML_unit;
    }
    return sXML_gap;
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


inline sal_Bool const GetBoolProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    sal_Bool bBool = *(sal_Bool *)aAny.getValue();
    return bBool;
}

inline Double const GetDoubleProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    Double fDouble = 0.0;
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

inline sal_Int32 const GetIntProperty(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    sal_Int32 nInt = 0;
    aAny >>= nInt;
    return nInt;
}

inline sal_Int16 const GetInt16Property(
    const OUString& sPropName,
    const Reference<XPropertySet> & xPropSet)
{
    Any aAny = xPropSet->getPropertyValue(sPropName);
    sal_Int16 nInt = 0;
    aAny >>= nInt;
    return nInt;
}

inline sal_Int8 const GetInt8Property(
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

