/*************************************************************************
 *
 *  $RCSfile: xmlimprt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:16 $
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

#ifndef SC_XMLIMPRT_HXX
#define SC_XMLIMPRT_HXX

#ifndef _RSCSFX_HXX
#include <rsc/rscsfx.hxx>
#endif
#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _XMLOFF_XMLTKMAP_HXX
#include <xmloff/xmltkmap.hxx>
#endif
#ifndef _XMLOFF_XMLASTPLP_HXX
#include <xmloff/xmlaustp.hxx>
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX
#include <xmloff/xmlstyle.hxx>
#endif
#include <com/sun/star/frame/XModel.hpp>
#include <tools/time.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <vector>
#include "xmlsubti.hxx"
#include "global.hxx"
#ifndef _XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif

#ifdef _USE_NAMESPACE
using namespace rtl;
#endif

enum ScXMLDocTokens
{
    XML_TOK_DOC_STYLES,
    XML_TOK_DOC_AUTOSTYLES,
    XML_TOK_DOC_USESTYLES,
    XML_TOK_DOC_META,
    XML_TOK_DOC_BODY,
    XML_TOK_OFFICE_END=XML_TOK_UNKNOWN
};

enum ScXMLStylesTokens
{
    XML_TOK_STYLES_STYLE
};

enum ScXMLStylesAttrTokens
{
    XML_TOK_STYLES_STYLE_NAME,
    XML_TOK_STYLES_STYLE_FAMILY,
    XML_TOK_STYLES_STYLE_PARENT_STYLE_NAME
};

enum ScXMLStyleTokens
{
    XML_TOK_STYLE_PROPERTIES
};

enum ScXMLBodyTokens
{
    XML_TOK_BODY_TABLE,
    XML_TOK_BODY_NAMED_EXPRESSIONS,
    XML_TOK_BODY_DATABASE_RANGES,
    XML_TOK_BODY_DATABASE_RANGE,
    XML_TOK_BODY_DATA_PILOT_TABLES
};

enum ScXMLTableTokens
{
    XML_TOK_TABLE_HEADER_COLS,
    XML_TOK_TABLE_COLS,
    XML_TOK_TABLE_COL,
    XML_TOK_TABLE_HEADER_ROWS,
    XML_TOK_TABLE_ROWS,
    XML_TOK_TABLE_ROW
};

enum ScXMLNamedExpressionsTokens
{
    XML_TOK_NAMED_EXPRESSIONS_NAMED_RANGE,
    XML_TOK_NAMED_EXPRESSIONS_NAMED_EXPRESSION
};

enum ScXMLNamedRangeAttrTokens
{
    XML_TOK_NAMED_RANGE_ATTR_NAME,
    XML_TOK_NAMED_RANGE_ATTR_CELL_RANGE_ADDRESS,
    XML_TOK_NAMED_RANGE_ATTR_BASE_CELL_ADDRESS,
    XML_TOK_NAMED_RANGE_ATTR_RANGE_USABLE_AS
};

enum ScXMLNamedExpressionAttrTokens
{
    XML_TOK_NAMED_EXPRESSION_ATTR_NAME,
    XML_TOK_NAMED_EXPRESSION_ATTR_BASE_CELL_ADDRESS,
    XML_TOK_NAMED_EXPRESSION_ATTR_EXPRESSION
};

enum ScXMLDatabaseRangesTokens
{
    XML_TOK_DATABASE_RANGE
};

enum ScXMLDatabaseRangeTokens
{
    XML_TOK_DATABASE_RANGE_SOURCE_SQL,
    XML_TOK_DATABASE_RANGE_SOURCE_TABLE,
    XML_TOK_DATABASE_RANGE_SOURCE_QUERY,
    XML_TOK_FILTER,
    XML_TOK_SORT,
    XML_TOK_DATABASE_RANGE_SUBTOTAL_RULES
};

enum ScXMLDatabaseRangeAttrTokens
{
    XML_TOK_DATABASE_RANGE_ATTR_NAME,
    XML_TOK_DATABASE_RANGE_ATTR_IS_SELECTION,
    XML_TOK_DATABASE_RANGE_ATTR_ON_UPDATE_KEEP_STYLES,
    XML_TOK_DATABASE_RANGE_ATTR_ON_UPDATE_KEEP_SIZE,
    XML_TOK_DATABASE_RANGE_ATTR_HAS_PERSISTENT_DATA,
    XML_TOK_DATABASE_RANGE_ATTR_ORIENTATION,
    XML_TOK_DATABASE_RANGE_ATTR_CONTAINS_HEADER,
    XML_TOK_DATABASE_RANGE_ATTR_DISPLAY_FILTER_BUTTONS,
    XML_TOK_DATABASE_RANGE_ATTR_TARGET_RANGE_ADDRESS
};

enum ScXMLDatabaseRangeSourceSQLAttrTokens
{
    XML_TOK_SOURCE_SQL_ATTR_DATABASE_NAME,
    XML_TOK_SOURCE_SQL_ATTR_SQL_STATEMENT,
    XML_TOK_SOURCE_SQL_ATTR_PARSE_SQL_STATEMENT
};

enum ScXMLDatabaseRangeSourceTableAttrTokens
{
    XML_TOK_SOURCE_TABLE_ATTR_DATABASE_NAME,
    XML_TOK_SOURCE_TABLE_ATTR_TABLE_NAME
};

enum ScXMLDatabaseRangeSourceQueryAttrTokens
{
    XML_TOK_SOURCE_QUERY_ATTR_DATABASE_NAME,
    XML_TOK_SOURCE_QUERY_ATTR_QUERY_NAME
};

enum ScXMLFilterTokens
{
    XML_TOK_FILTER_AND,
    XML_TOK_FILTER_OR,
    XML_TOK_FILTER_CONDITION
};

enum ScXMLFilterAttrTokens
{
    XML_TOK_FILTER_ATTR_TARGET_RANGE_ADDRESS,
    XML_TOK_FILTER_ATTR_CONDITION_SOURCE_RANGE_ADDRESS,
    XML_TOK_FILTER_ATTR_CONDITION_SOURCE,
    XML_TOK_FILTER_ATTR_DISPLAY_DUPLICATES
};

enum ScXMLFilterConditionAttrTokens
{
    XML_TOK_CONDITION_ATTR_FIELD_NUMBER,
    XML_TOK_CONDITION_ATTR_CASE_SENSITIVE,
    XML_TOK_CONDITION_ATTR_DATA_TYPE,
    XML_TOK_CONDITION_ATTR_VALUE,
    XML_TOK_CONDITION_ATTR_OPERATOR
};

enum ScXMLSortTokens
{
    XML_TOK_SORT_SORT_BY
};

enum ScXMLSortAttrTokens
{
    XML_TOK_SORT_ATTR_BIND_STYLES_TO_CONTENT,
    XML_TOK_SORT_ATTR_TARGET_RANGE_ADDRESS,
    XML_TOK_SORT_ATTR_CASE_SENSITIVE
};

enum ScXMLSortSortByAttrTokens
{
    XML_TOK_SORT_BY_ATTR_FIELD_NUMBER,
    XML_TOK_SORT_BY_ATTR_DATA_TYPE,
    XML_TOK_SORT_BY_ATTR_ORDER
};

enum ScXMLDatabaseRangeSubTotalRulesTokens
{
    XML_TOK_SUBTOTAL_RULES_SORT_GROUPS,
    XML_TOK_SUBTOTAL_RULES_SUBTOTAL_RULE
};

enum ScXMLDatabaseRangeSubTotalRulesAttrTokens
{
    XML_TOK_SUBTOTAL_RULES_ATTR_BIND_STYLES_TO_CONTENT,
    XML_TOK_SUBTOTAL_RULES_ATTR_CASE_SENSITIVE,
    XML_TOK_SUBTOTAL_RULES_ATTR_PAGE_BREAKS_ON_GROUP_CHANGE
};

enum ScXMLSubTotalRulesSortGroupsAttrTokens
{
    XML_TOK_SORT_GROUPS_ATTR_DATA_TYPE,
    XML_TOK_SORT_GROUPS_ATTR_ORDER
};

enum ScXMLSubTotalRulesSubTotalRuleTokens
{
    XML_TOK_SUBTOTAL_RULE_SUBTOTAL_FIELD
};

enum ScXMLSubTotalRulesSubTotalRuleAttrTokens
{
    XML_TOK_SUBTOTAL_RULE_ATTR_GROUP_BY_FIELD_NUMBER
};

enum ScXMLSubTotalRuleSubTotalField
{
    XML_TOK_SUBTOTAL_FIELD_ATTR_FIELD_NUMBER,
    XML_TOK_SUBTOTAL_FIELD_ATTR_FUNCTION
};

enum ScXMLDataPilotTablesElemTokens
{
    XML_TOK_DATA_PILOT_TABLE
};

enum ScXMLDataPilotTableAttrTokens
{
    XML_TOK_DATA_PILOT_TABLE_ATTR_NAME,
    XML_TOK_DATA_PILOT_TABLE_ATTR_APPLICATION_DATA,
    XML_TOK_DATA_PILOT_TABLE_ATTR_GRAND_TOTAL,
    XML_TOK_DATA_PILOT_TABLE_ATTR_IGNORE_EMPTY_ROWS,
    XML_TOK_DATA_PILOT_TABLE_ATTR_IDENTIFY_CATEGORIES,
    XML_TOK_DATA_PILOT_TABLE_ATTR_TARGET_RANGE_ADDRESS
};

enum ScXMLDataPilotTableElemTokens
{
    XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_SQL,
    XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_TABLE,
    XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_QUERY,
    XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_SERVICE,
    XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_CELL_RANGE,
    XML_TOK_DATA_PILOT_TABLE_ELEM_DATA_PILOT_FIELD
};

enum ScXMLDataPilotTableSourceServiceAttrTokens
{
    XML_TOK_SOURCE_SERVICE_ATTR_NAME,
    XML_TOK_SOURCE_SERVICE_ATTR_SOURCE_NAME,
    XML_TOK_SOURCE_SERVICE_ATTR_OBJECT_NAME,
    XML_TOK_SOURCE_SERVICE_ATTR_USERNAME,
    XML_TOK_SOURCE_SERVICE_ATTR_PASSWORD
};

enum ScXMLDataPilotTableSourceCellRangeElemTokens
{
    XML_TOK_SOURCE_CELL_RANGE_ELEM_FILTER
};

enum ScXMLDataPilotTableSourceCellRangeAttrTokens
{
    XML_TOK_SOURCE_CELL_RANGE_ATTR_CELL_RANGE_ADDRESS
};

enum ScXMLDataPilotFieldAttrTokens
{
    XML_TOK_DATA_PILOT_FIELD_ATTR_SOURCE_FIELD_NAME,
    XML_TOK_DATA_PILOT_FIELD_ATTR_IS_DATA_LAYOUT_FIELD,
    XML_TOK_DATA_PILOT_FIELD_ATTR_FUNCTION,
    XML_TOK_DATA_PILOT_FIELD_ATTR_ORIENTATION,
    XML_TOK_DATA_PILOT_FIELD_ATTR_USED_HIERARCHY
};

enum ScXMLDataPilotFieldElemTokens
{
    XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_LEVEL
};

enum ScXMLDataPilotLevelAttrTokens
{
    XML_TOK_DATA_PILOT_LEVEL_ATTR_DISPLAY_EMPTY
};

enum ScXMLDataPilotLevelElemTokens
{
    XML_TOK_DATA_PILOT_LEVEL_ELEM_DATA_PILOT_SUBTOTALS,
    XML_TOK_DATA_PILOT_LEVEL_ELEM_DATA_PILOT_MEMBERS
};

enum ScXMLDataPilotSubTotalsElemTokens
{
    XML_TOK_DATA_PILOT_SUBTOTALS_ELEM_DATA_PILOT_SUBTOTAL
};

enum ScXMLDataPilotSubTotalAttrTokens
{
    XML_TOK_DATA_PILOT_SUBTOTAL_ATTR_FUNCTION
};

enum ScXMLDataPilotMembersElemTokens
{
    XML_TOK_DATA_PILOT_MEMBERS_ELEM_DATA_PILOT_MEMBER
};

enum ScXMLDataPilotMemberAttrTokens
{
    XML_TOK_DATA_PILOT_MEMBER_ATTR_NAME,
    XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY,
    XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY_DETAILS
};

enum ScXMLTableRowTokens
{
    XML_TOK_TABLE_ROW_CELL,
    XML_TOK_TABLE_ROW_COVERED_CELL
};

enum ScXMLTableRowCellTokens
{
    XML_TOK_TABLE_ROW_CELL_P,
    XML_TOK_TABLE_ROW_CELL_SUBTABLE,
    XML_TOK_TABLE_ROW_CELL_ANNOTATION
};

enum ScXMLTableAttrTokens
{
    XML_TOK_TABLE_NAME,
    XML_TOK_TABLE_STYLE_NAME,
    XML_TOK_TABLE_PROTECTION
};

enum ScXMLTableColAttrTokens
{
    XML_TOK_TABLE_COL_ATTR_STYLE_NAME,
    XML_TOK_TABLE_COL_ATTR_REPEATED,
    XML_TOK_TABLE_COL_ATTR_VISIBILITY
};

enum ScXMLTableRowAttrTokens
{
    XML_TOK_TABLE_ROW_ATTR_STYLE_NAME,
    XML_TOK_TABLE_ROW_ATTR_VISIBILITY,
    XML_TOK_TABLE_ROW_ATTR_REPEATED
//  XML_TOK_TABLE_ROW_ATTR_USE_OPTIMAL_HEIGHT
};

enum ScXMLTableRowCellAttrTokens
{
    XML_TOK_TABLE_ROW_CELL_ATTR_STYLE_NAME,
    XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_ROWS,
    XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_COLS,
    XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_COLS,
    XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_ROWS,
    XML_TOK_TABLE_ROW_CELL_ATTR_REPEATED,
    XML_TOK_TABLE_ROW_CELL_ATTR_VALUE_TYPE,
    XML_TOK_TABLE_ROW_CELL_ATTR_VALUE,
    XML_TOK_TABLE_ROW_CELL_ATTR_DATE_VALUE,
    XML_TOK_TABLE_ROW_CELL_ATTR_TIME_VALUE,
    XML_TOK_TABLE_ROW_CELL_ATTR_STRING_VALUE,
    XML_TOK_TABLE_ROW_CELL_ATTR_BOOLEAN_VALUE,
    XML_TOK_TABLE_ROW_CELL_ATTR_FORMULA,
    XML_TOK_TABLE_ROW_CELL_ATTR_CURRENCY
};

enum ScXMLAnnotationAttrTokens
{
    XML_TOK_TABLE_ANNOTATION_ATTR_AUTHOR,
    XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE,
    XML_TOK_TABLE_ANNOTATION_ATTR_DISPLAY
};

class SvI18NMap;
class SvXMLTokenMap;
//class SvXMLImportItemMapper;
class SvXMLStyleContext;
class SfxItemSet;
class SvXMLNumFmtHelper;
//class ScDocument;

struct tScMyCellRange
{
    sal_Int16 Sheet;
    sal_Int32 StartColumn, EndColumn;
    sal_Int32 StartRow, EndRow;
};

struct ScMyNamedExpression
{
    sal_Bool        bIsExpression;
    rtl::OUString   sName;
    rtl::OUString   sContent;
    rtl::OUString   sBaseCellAddress;
    rtl::OUString   sRangeType;
};

typedef std::list<const ScMyNamedExpression*> ScMyNamedExpressions;

class ScXMLImport: public SvXMLImport
{
//  ScDocument&             rDoc;

//  SvXMLAutoStylePoolP     *pScAutoStylePool;
    XMLScPropHdlFactory     *pScPropHdlFactory;
    XMLCellStylesPropertySetMapper      *pCellStylesPropertySetMapper;
    XMLPageStylesPropertySetMapper      *pPageStylesPropertySetMapper;
    XMLColumnStylesPropertySetMapper    *pColumnStylesPropertySetMapper;
    XMLRowStylesPropertySetMapper       *pRowStylesPropertySetMapper;
    XMLTableStylesPropertySetMapper     *pTableStylesPropertySetMapper;
    SvXMLImportContextRef       xStyles;
    SvXMLImportContextRef       xAutoStyles;

//  SvXMLImportItemMapper   *pParaItemMapper;// paragraph item import
//  SvI18NMap               *pI18NMap;          // name mapping for I18N
    SvXMLTokenMap           *pDocElemTokenMap;
    SvXMLTokenMap           *pStylesElemTokenMap;
    SvXMLTokenMap           *pStylesAttrTokenMap;
    SvXMLTokenMap           *pStyleElemTokenMap;
    SvXMLTokenMap           *pBodyElemTokenMap;
    SvXMLTokenMap           *pTableElemTokenMap;
    SvXMLTokenMap           *pNamedExpressionsElemTokenMap;
    SvXMLTokenMap           *pNamedRangeAttrTokenMap;
    SvXMLTokenMap           *pNamedExpressionAttrTokenMap;
    SvXMLTokenMap           *pDatabaseRangesElemTokenMap;
    SvXMLTokenMap           *pDatabaseRangeElemTokenMap;
    SvXMLTokenMap           *pDatabaseRangeAttrTokenMap;
    SvXMLTokenMap           *pDatabaseRangeSourceSQLAttrTokenMap;
    SvXMLTokenMap           *pDatabaseRangeSourceTableAttrTokenMap;
    SvXMLTokenMap           *pDatabaseRangeSourceQueryAttrTokenMap;
    SvXMLTokenMap           *pFilterElemTokenMap;
    SvXMLTokenMap           *pFilterAttrTokenMap;
    SvXMLTokenMap           *pFilterConditionAttrTokenMap;
    SvXMLTokenMap           *pSortElemTokenMap;
    SvXMLTokenMap           *pSortAttrTokenMap;
    SvXMLTokenMap           *pSortSortByAttrTokenMap;
    SvXMLTokenMap           *pDatabaseRangeSubTotalRulesElemTokenMap;
    SvXMLTokenMap           *pDatabaseRangeSubTotalRulesAttrTokenMap;
    SvXMLTokenMap           *pSubTotalRulesSortGroupsAttrTokenMap;
    SvXMLTokenMap           *pSubTotalRulesSubTotalRuleElemTokenMap;
    SvXMLTokenMap           *pSubTotalRulesSubTotalRuleAttrTokenMap;
    SvXMLTokenMap           *pSubTotalRuleSubTotalFieldAttrTokenMap;
    SvXMLTokenMap           *pDataPilotTablesElemTokenMap;
    SvXMLTokenMap           *pDataPilotTableAttrTokenMap;
    SvXMLTokenMap           *pDataPilotTableElemTokenMap;
    SvXMLTokenMap           *pDataPilotTableSourceServiceAttrTokenMap;
    SvXMLTokenMap           *pDataPilotTableSourceCellRangeElemTokenMap;
    SvXMLTokenMap           *pDataPilotTableSourceCellRangeAttrTokenMap;
    SvXMLTokenMap           *pDataPilotFieldAttrTokenMap;
    SvXMLTokenMap           *pDataPilotFieldElemTokenMap;
    SvXMLTokenMap           *pDataPilotLevelAttrTokenMap;
    SvXMLTokenMap           *pDataPilotLevelElemTokenMap;
    SvXMLTokenMap           *pDataPilotSubTotalsElemTokenMap;
    SvXMLTokenMap           *pDataPilotSubTotalAttrTokenMap;
    SvXMLTokenMap           *pDataPilotMembersElemTokenMap;
    SvXMLTokenMap           *pDataPilotMemberAttrTokenMap;
    SvXMLTokenMap           *pTableRowElemTokenMap;
    SvXMLTokenMap           *pTableRowCellElemTokenMap;
    SvXMLTokenMap           *pTableAttrTokenMap;
    SvXMLTokenMap           *pTableColAttrTokenMap;
    SvXMLTokenMap           *pTableRowAttrTokenMap;
    SvXMLTokenMap           *pTableRowCellAttrTokenMap;
    SvXMLTokenMap           *pTableAnnotationAttrTokenMap;

    sal_uInt16              nStyleFamilyMask;// Mask of styles to load
    sal_Bool                bLoadDoc : 1;   // Load doc or styles only

    ScMyTables              aTables;

    ScMyNamedExpressions    aMyNamedExpressions;

protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext(USHORT nPrefix,
                                      const ::rtl::OUString& rLocalName,
                                      const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

public:
    rtl::OUString sSC_float;
    rtl::OUString sSC_time;
    rtl::OUString sSC_date;
    rtl::OUString sSC_percentage;
    rtl::OUString sSC_currency;
    rtl::OUString sSC_string;
    rtl::OUString sSC_boolean;

    ScXMLImport(    com::sun::star::uno::Reference <com::sun::star::frame::XModel> xTempModel, sal_Bool bLoadDoc,
                 sal_uInt16 nStyleFamMask );
    ~ScXMLImport();

    // namespace office
    SvXMLImportContext *CreateMetaContext(
                                    const NAMESPACE_RTL(OUString)& rLocalName );
    SvXMLImportContext *CreateStylesContext(const NAMESPACE_RTL(OUString)& rLocalName,
                                     const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList, sal_Bool bAutoStyles );
//  SvXMLImportContext *CreateUseStylesContext(const NAMESPACE_RTL(OUString)& rLocalName ,
//                                  const ::com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList);
    SvXMLImportContext *CreateBodyContext(
                                    const NAMESPACE_RTL(OUString)& rLocalName );

//  ScDocument& GetDoc() { return rDoc; }
//  const ScDocument& GetDoc() const { return rDoc; }

    ScMyTables& GetTables() { return aTables; }

    sal_uInt16 GetStyleFamilyMask() const { return nStyleFamilyMask; }
    sal_Bool IsStylesOnlyMode() const { return !bLoadDoc; }

//  SvI18NMap& GetI18NMap() { return *pI18NMap; }

//  inline const SvXMLImportItemMapper& GetParaItemMapper() const;
//  SvXMLImportContext *CreateParaItemImportContext( USHORT nPrefix,
//                                const NAMESPACE_RTL(OUString)& rLocalName,
//                                const ::com::sun::star::uno::Reference<
//                                  ::com::sun::star::xml::sax::XAttributeList& xAttrList,
//                                SfxItemSet& rItemSet );

    XMLCellStylesPropertySetMapper* GetCellStylesPropertySetMapper() const { return pCellStylesPropertySetMapper; }
    XMLPageStylesPropertySetMapper* GetPageStylesPropertySetMapper() const { return pPageStylesPropertySetMapper; }
    XMLColumnStylesPropertySetMapper* GetColumnStylesPropertySetMapper() const { return pColumnStylesPropertySetMapper; }
    XMLRowStylesPropertySetMapper* GetRowStylesPropertySetMapper() const { return pRowStylesPropertySetMapper; }
    XMLTableStylesPropertySetMapper* GetTableStylesPropertySetMapper() const { return pTableStylesPropertySetMapper; }
    SvXMLImportContextRef           GetAutoStyles() const { return xAutoStyles; }
    SvXMLImportContextRef           GetStyles() const { return xStyles; }

    const SvXMLTokenMap& GetDocElemTokenMap();
    const SvXMLTokenMap& GetStylesElemTokenMap();
    const SvXMLTokenMap& GetStylesAttrTokenMap();
    const SvXMLTokenMap& GetStyleElemTokenMap();
    const SvXMLTokenMap& GetBodyElemTokenMap();
    const SvXMLTokenMap& GetTableElemTokenMap();
    const SvXMLTokenMap& GetNamedExpressionsElemTokenMap();
    const SvXMLTokenMap& GetNamedRangeAttrTokenMap();
    const SvXMLTokenMap& GetNamedExpressionAttrTokenMap();
    const SvXMLTokenMap& GetDatabaseRangesElemTokenMap();
    const SvXMLTokenMap& GetDatabaseRangeElemTokenMap();
    const SvXMLTokenMap& GetDatabaseRangeAttrTokenMap();
    const SvXMLTokenMap& GetDatabaseRangeSourceSQLAttrTokenMap();
    const SvXMLTokenMap& GetDatabaseRangeSourceTableAttrTokenMap();
    const SvXMLTokenMap& GetDatabaseRangeSourceQueryAttrTokenMap();
    const SvXMLTokenMap& GetFilterElemTokenMap();
    const SvXMLTokenMap& GetFilterAttrTokenMap();
    const SvXMLTokenMap& GetFilterConditionAttrTokenMap();
    const SvXMLTokenMap& GetSortElemTokenMap();
    const SvXMLTokenMap& GetSortAttrTokenMap();
    const SvXMLTokenMap& GetSortSortByAttrTokenMap();
    const SvXMLTokenMap& GetDatabaseRangeSubTotalRulesElemTokenMap();
    const SvXMLTokenMap& GetDatabaseRangeSubTotalRulesAttrTokenMap();
    const SvXMLTokenMap& GetSubTotalRulesSortGroupsAttrTokenMap();
    const SvXMLTokenMap& GetSubTotalRulesSubTotalRuleElemTokenMap();
    const SvXMLTokenMap& GetSubTotalRulesSubTotalRuleAttrTokenMap();
    const SvXMLTokenMap& GetSubTotalRuleSubTotalFieldAttrTokenMap();
    const SvXMLTokenMap& GetDataPilotTablesElemTokenMap();
    const SvXMLTokenMap& GetDataPilotTableAttrTokenMap();
    const SvXMLTokenMap& GetDataPilotTableElemTokenMap();
    const SvXMLTokenMap& GetDataPilotTableSourceServiceAttrTokenMap();
    const SvXMLTokenMap& GetDataPilotTableSourceCellRangeElemTokenMap();
    const SvXMLTokenMap& GetDataPilotTableSourceCellRangeAttrTokenMap();
    const SvXMLTokenMap& GetDataPilotFieldAttrTokenMap();
    const SvXMLTokenMap& GetDataPilotFieldElemTokenMap();
    const SvXMLTokenMap& GetDataPilotLevelAttrTokenMap();
    const SvXMLTokenMap& GetDataPilotLevelElemTokenMap();
    const SvXMLTokenMap& GetDataPilotSubTotalsElemTokenMap();
    const SvXMLTokenMap& GetDataPilotSubTotalAttrTokenMap();
    const SvXMLTokenMap& GetDataPilotMembersElemTokenMap();
    const SvXMLTokenMap& GetDataPilotMemberAttrTokenMap();
    const SvXMLTokenMap& GetTableRowElemTokenMap();
    const SvXMLTokenMap& GetTableRowCellElemTokenMap();
    const SvXMLTokenMap& GetTableAttrTokenMap();
    const SvXMLTokenMap& GetTableColAttrTokenMap();
    const SvXMLTokenMap& GetTableRowAttrTokenMap();
    const SvXMLTokenMap& GetTableRowCellAttrTokenMap();
    const SvXMLTokenMap& GetTableAnnotationAttrTokenMap();
//  const SvXMLTokenMap& GetTextPElemTokenMap();
//  const SvXMLTokenMap& GetTextPAttrTokenMap();
//  const SvXMLTokenMap& GetStyleStylesElemTokenMap();
//  const SvXMLTokenMap& GetTextListBlockAttrTokenMap();
//  const SvXMLTokenMap& GetTextListBlockElemTokenMap();

    void    AddNamedExpression(const ScMyNamedExpression* pMyNamedExpression) { aMyNamedExpressions.insert(aMyNamedExpressions.end(), pMyNamedExpression); }
    ScMyNamedExpressions* GetNamedExpressions() { return &aMyNamedExpressions; }
};

#endif

