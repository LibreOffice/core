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

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLIMPRT_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLIMPRT_HXX

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlprmap.hxx>
#include "xmlsubti.hxx"
#include <formula/grammar.hxx>
#include <dociter.hxx>

#include <com/sun/star/sheet/ValidationAlertStyle.hpp>
#include <com/sun/star/sheet/ValidationType.hpp>
#include <com/sun/star/sheet/ConditionOperator.hpp>

#include <memory>
#include <map>
#include <vector>
#include <list>

namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace sheet { class XSheetCellRangeContainer; } } } }
namespace com { namespace sun { namespace star { namespace table { struct CellRangeAddress; } } } }
namespace com { namespace sun { namespace star { namespace util { class XNumberFormatTypes; } } } }
namespace com { namespace sun { namespace star { namespace util { class XNumberFormats; } } } }
namespace sax_fastparser { class FastAttributeList; }

class ScCompiler;
class ErrCode;
class ScMyStyleNumberFormats;
class XMLNumberFormatAttributesExportHelper;
class ScEditEngineDefaulter;
class ScDocumentImport;
class ScMyImpDetectiveOpArray;

namespace sc {

struct ImportPostProcessData;
struct PivotTableSources;

}

enum ScXMLDocTokens
{
    XML_TOK_DOC_FONTDECLS,
    XML_TOK_DOC_STYLES,
    XML_TOK_DOC_AUTOSTYLES,
    XML_TOK_DOC_MASTERSTYLES,
    XML_TOK_DOC_META,
    XML_TOK_DOC_SCRIPTS,
    XML_TOK_DOC_BODY,
    XML_TOK_DOC_SETTINGS
};

enum ScXMLContentValidationElemTokens
{
    XML_TOK_CONTENT_VALIDATION_ELEM_HELP_MESSAGE,
    XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MESSAGE,
    XML_TOK_CONTENT_VALIDATION_ELEM_ERROR_MACRO,
    XML_TOK_CONTENT_VALIDATION_ELEM_EVENT_LISTENERS
};

enum ScXMLContentValidationMessageElemTokens
{
    XML_TOK_P
};

enum ScXMLTableTokens
{
    XML_TOK_TABLE_NAMED_EXPRESSIONS,
    XML_TOK_TABLE_COL_GROUP,
    XML_TOK_TABLE_HEADER_COLS,
    XML_TOK_TABLE_COLS,
    XML_TOK_TABLE_COL,
    XML_TOK_TABLE_ROW_GROUP,
    XML_TOK_TABLE_HEADER_ROWS,
    XML_TOK_TABLE_PROTECTION,
    XML_TOK_TABLE_PROTECTION_EXT,
    XML_TOK_TABLE_ROWS,
    XML_TOK_TABLE_ROW,
    XML_TOK_TABLE_SOURCE,
    XML_TOK_TABLE_SCENARIO,
    XML_TOK_TABLE_SHAPES,
    XML_TOK_TABLE_FORMS,
    XML_TOK_TABLE_EVENT_LISTENERS,
    XML_TOK_TABLE_EVENT_LISTENERS_EXT,
    XML_TOK_TABLE_CONDFORMATS
};

enum ScXMLTableRowsTokens
{
    XML_TOK_TABLE_ROWS_ROW_GROUP,
    XML_TOK_TABLE_ROWS_HEADER_ROWS,
    XML_TOK_TABLE_ROWS_ROWS,
    XML_TOK_TABLE_ROWS_ROW
};

enum ScXMLTableRowTokens
{
    XML_TOK_TABLE_ROW_CELL,
    XML_TOK_TABLE_ROW_COVERED_CELL
};

enum ScXMLTableRowAttrTokens
{
    XML_TOK_TABLE_ROW_ATTR_STYLE_NAME,
    XML_TOK_TABLE_ROW_ATTR_VISIBILITY,
    XML_TOK_TABLE_ROW_ATTR_REPEATED,
    XML_TOK_TABLE_ROW_ATTR_DEFAULT_CELL_STYLE_NAME
//  XML_TOK_TABLE_ROW_ATTR_USE_OPTIMAL_HEIGHT
};

enum ScXMLTableRowCellTokens
{
    XML_TOK_TABLE_ROW_CELL_P,
    XML_TOK_TABLE_ROW_CELL_TABLE,
    XML_TOK_TABLE_ROW_CELL_ANNOTATION,
    XML_TOK_TABLE_ROW_CELL_DETECTIVE,
    XML_TOK_TABLE_ROW_CELL_CELL_RANGE_SOURCE
};

enum ScXMLTableRowCellAttrTokens
{
    XML_TOK_TABLE_ROW_CELL_ATTR_STYLE_NAME,
    XML_TOK_TABLE_ROW_CELL_ATTR_CONTENT_VALIDATION_NAME,
    XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_ROWS,
    XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_COLS,
    XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_COLS,
    XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_ROWS,
    XML_TOK_TABLE_ROW_CELL_ATTR_REPEATED,
    XML_TOK_TABLE_ROW_CELL_ATTR_VALUE_TYPE,
    XML_TOK_TABLE_ROW_CELL_ATTR_NEW_VALUE_TYPE,
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
    XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE_STRING,
    XML_TOK_TABLE_ANNOTATION_ATTR_DISPLAY,
    XML_TOK_TABLE_ANNOTATION_ATTR_X,
    XML_TOK_TABLE_ANNOTATION_ATTR_Y
};

class SvXMLTokenMap;
class XMLShapeImportHelper;
class ScXMLChangeTrackingImportHelper;
class SolarMutexGuard;

struct ScMyNamedExpression
{
    OUString      sName;
    OUString      sContent;
    OUString      sContentNmsp;
    OUString      sBaseCellAddress;
    OUString      sRangeType;
    formula::FormulaGrammar::Grammar eGrammar;
    bool               bIsExpression;
};

typedef ::std::list<std::unique_ptr<ScMyNamedExpression>> ScMyNamedExpressions;

struct ScMyLabelRange
{
    OUString const   sLabelRangeStr;
    OUString const   sDataRangeStr;
    bool const            bColumnOrientation;
};

typedef std::list< std::unique_ptr<const ScMyLabelRange> > ScMyLabelRanges;

struct ScMyImportValidation
{
    OUString                                   sName;
    OUString                                   sImputTitle;
    OUString                                   sImputMessage;
    OUString                                   sErrorTitle;
    OUString                                   sErrorMessage;
    OUString                                   sFormula1;
    OUString                                   sFormula2;
    OUString                                   sFormulaNmsp1;
    OUString                                   sFormulaNmsp2;
    OUString                                   sBaseCellAddress;   // string is used directly
    css::sheet::ValidationAlertStyle           aAlertStyle;
    css::sheet::ValidationType                 aValidationType;
    css::sheet::ConditionOperator              aOperator;
    formula::FormulaGrammar::Grammar           eGrammar1;
    formula::FormulaGrammar::Grammar           eGrammar2;
    sal_Int16                                  nShowList;
    bool                                       bShowErrorMessage;
    bool                                       bShowImputMessage;
    bool                                       bIgnoreBlanks;
};

typedef std::vector<ScMyImportValidation>           ScMyImportValidations;
class ScMyStylesImportHelper;
class ScXMLEditAttributeMap;

class ScXMLImport: public SvXMLImport
{
    ScXMLImport(const ScXMLImport&) = delete;
    const ScXMLImport& operator=(const ScXMLImport&) = delete;

    typedef ::std::map<SCTAB, std::unique_ptr<ScMyNamedExpressions>> SheetNamedExpMap;

    ScDocument*             pDoc;
    std::unique_ptr<ScDocumentImport> mpDocImport;
    std::unique_ptr<ScCompiler> mpComp; // For error-checking of cached string cell values.
    std::unique_ptr<ScEditEngineDefaulter> mpEditEngine;
    std::unique_ptr<sc::PivotTableSources> mpPivotSources;

    mutable std::unique_ptr<ScXMLEditAttributeMap> mpEditAttrMap;
    std::unique_ptr<ScXMLChangeTrackingImportHelper>    pChangeTrackingImportHelper;
    std::unique_ptr<ScMyStylesImportHelper>        pStylesImportHelper;

    rtl::Reference < XMLPropertyHandlerFactory >  xScPropHdlFactory;
    rtl::Reference < XMLPropertySetMapper >       xCellStylesPropertySetMapper;
    rtl::Reference < XMLPropertySetMapper >       xColumnStylesPropertySetMapper;
    rtl::Reference < XMLPropertySetMapper >       xRowStylesPropertySetMapper;
    rtl::Reference < XMLPropertySetMapper >       xTableStylesPropertySetMapper;

    std::unique_ptr<SvXMLTokenMap>           pDocElemTokenMap;
    std::unique_ptr<SvXMLTokenMap>           pContentValidationElemTokenMap;
    std::unique_ptr<SvXMLTokenMap>           pContentValidationMessageElemTokenMap;
    std::unique_ptr<SvXMLTokenMap>           pTableElemTokenMap;
    std::unique_ptr<SvXMLTokenMap>           pTableRowsElemTokenMap;
    std::unique_ptr<SvXMLTokenMap>           pTableRowElemTokenMap;
    std::unique_ptr<SvXMLTokenMap>           pTableRowAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap>           pTableRowCellElemTokenMap;
    std::unique_ptr<SvXMLTokenMap>           pTableRowCellAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap>           pTableAnnotationAttrTokenMap;

    sc::ImportPostProcessData* mpPostProcessData; /// Lift cycle managed elsewhere, no need to delete.

    ScMyTables              aTables;

    std::vector<ScDocRowHeightUpdater::TabRanges> maRecalcRowRanges;

    std::unique_ptr<ScMyNamedExpressions>   m_pMyNamedExpressions;
    SheetNamedExpMap m_SheetNamedExpressions;

    std::unique_ptr<ScMyLabelRanges>            pMyLabelRanges;
    std::unique_ptr<ScMyImportValidations>  pValidations;
    std::unique_ptr<ScMyImpDetectiveOpArray>    pDetectiveOpArray;
    std::unique_ptr<SolarMutexGuard>        pSolarMutexGuard;

    std::unique_ptr<XMLNumberFormatAttributesExportHelper> pNumberFormatAttributesExportHelper;
    std::unique_ptr<ScMyStyleNumberFormats> pStyleNumberFormats;
    css::uno::Reference <css::util::XNumberFormats> xNumberFormats;
    css::uno::Reference <css::util::XNumberFormatTypes> xNumberFormatTypes;

    css::uno::Reference <css::sheet::XSheetCellRangeContainer> xSheetCellRanges;

    OUString           sPrevStyleName;
    OUString           sPrevCurrency;
    sal_uInt32              nSolarMutexLocked;
    sal_Int32               nProgressCount;
    sal_Int16               nPrevCellType;
    bool                    bLoadDoc;   // Load doc or styles only
    bool                    bNullDateSetted;
    bool                    bSelfImportingXMLSet;
    bool mbLockSolarMutex;
    bool mbImportStyles;
    bool mbHasNewCondFormatData;

protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateFastContext( sal_Int32 nElement,
        const ::css::uno::Reference< ::css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual XMLShapeImportHelper* CreateShapeImport() override;

public:
    ScXMLImport(
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        OUString const & implementationName, SvXMLImportFlags nImportFlag);

    virtual ~ScXMLImport() throw() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence<css::uno::Any>& aArguments ) override;

    // namespace office
    // NB: in contrast to other CreateFooContexts, this particular one handles
    //     the root element (i.e. office:document-meta)
    SvXMLImportContext *CreateMetaContext(
                                    const sal_Int32 nElement );
    SvXMLImportContext *CreateFontDeclsContext(const sal_uInt16 nPrefix, const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList);
    SvXMLImportContext *CreateScriptContext(
                                    const OUString& rLocalName );
    SvXMLImportContext *CreateStylesContext(const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                     bool bAutoStyles );

    SvXMLImportContext *CreateBodyContext(
                                    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList );

    virtual void SetStatistics( const css::uno::Sequence< css::beans::NamedValue> & i_rStats) override;

    ScDocumentImport& GetDoc();

    ScDocument*          GetDocument()           { return pDoc; }
    const ScDocument*    GetDocument() const     { return pDoc; }

    ScMyTables& GetTables() { return aTables; }

    std::vector<ScDocRowHeightUpdater::TabRanges>& GetRecalcRowRanges() { return maRecalcRowRanges; }

    bool IsStylesOnlyMode() const { return !bLoadDoc; }

    static sal_Int16 GetCellType(const char* rStrValue, const sal_Int32 nStrLength);

    const rtl::Reference < XMLPropertySetMapper >& GetCellStylesPropertySetMapper() const { return xCellStylesPropertySetMapper; }
    const rtl::Reference < XMLPropertySetMapper >& GetColumnStylesPropertySetMapper() const { return xColumnStylesPropertySetMapper; }
    const rtl::Reference < XMLPropertySetMapper >& GetRowStylesPropertySetMapper() const { return xRowStylesPropertySetMapper; }
    const rtl::Reference < XMLPropertySetMapper >& GetTableStylesPropertySetMapper() const { return xTableStylesPropertySetMapper; }

    const SvXMLTokenMap& GetDocElemTokenMap();
    const SvXMLTokenMap& GetContentValidationElemTokenMap();
    const SvXMLTokenMap& GetContentValidationMessageElemTokenMap();
    const SvXMLTokenMap& GetTableElemTokenMap();
    const SvXMLTokenMap& GetTableRowsElemTokenMap();
    const SvXMLTokenMap& GetTableRowElemTokenMap();
    const SvXMLTokenMap& GetTableRowAttrTokenMap();
    const SvXMLTokenMap& GetTableRowCellElemTokenMap();
    const SvXMLTokenMap& GetTableRowCellAttrTokenMap();
    const SvXMLTokenMap& GetTableAnnotationAttrTokenMap();

    void SetPostProcessData( sc::ImportPostProcessData* p );
    sc::ImportPostProcessData* GetPostProcessData() { return mpPostProcessData;}

    sc::PivotTableSources& GetPivotTableSources();

    void AddNamedExpression(ScMyNamedExpression* pMyNamedExpression)
    {
        if (!m_pMyNamedExpressions)
            m_pMyNamedExpressions.reset(new ScMyNamedExpressions);
        m_pMyNamedExpressions->push_back(std::unique_ptr<ScMyNamedExpression>(pMyNamedExpression));
    }

    void AddNamedExpression(SCTAB nTab, ScMyNamedExpression* pNamedExp);

    void AddLabelRange(std::unique_ptr<const ScMyLabelRange> pMyLabelRange) {
        if (!pMyLabelRanges)
            pMyLabelRanges.reset(new ScMyLabelRanges);
        pMyLabelRanges->push_back(std::move(pMyLabelRange)); }

    void AddValidation(const ScMyImportValidation& rValidation) {
        if (!pValidations)
            pValidations.reset(new ScMyImportValidations);
        pValidations->push_back(rValidation); }
    bool GetValidation(const OUString& sName, ScMyImportValidation& aValidation);

    ScMyImpDetectiveOpArray* GetDetectiveOpArray();

    ScXMLChangeTrackingImportHelper* GetChangeTrackingImportHelper();
    void InsertStyles();

    void SetChangeTrackingViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& rChangeProps);
    virtual void SetViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& aViewProps) override;
    virtual void SetConfigurationSettings(const css::uno::Sequence<css::beans::PropertyValue>& aConfigProps) override;

    ScMyStylesImportHelper* GetStylesImportHelper() { return pStylesImportHelper.get(); }
    sal_Int32 SetCurrencySymbol(const sal_Int32 nKey, const OUString& rCurrency);
    bool IsCurrencySymbol(const sal_Int32 nNumberFormat, const OUString& sCurrencySymbol, const OUString& sBankSymbol);
    void SetType(const css::uno::Reference <css::beans::XPropertySet>& rProperties,
        sal_Int32& rNumberFormat,
        const sal_Int16 nCellType,
        const OUString& rCurrency);

    void ProgressBarIncrement();

    void SetNewCondFormatData() { mbHasNewCondFormatData = true; }
    bool HasNewCondFormatData() { return mbHasNewCondFormatData; }

private:
    void AddStyleRange(const css::table::CellRangeAddress& rCellRange);
    void SetStyleToRanges();

    void ExamineDefaultStyle();
public:
    void SetStyleToRange(const ScRange& rRange, const OUString* pStyleName,
        const sal_Int16 nCellType, const OUString* pCurrency);
    bool SetNullDateOnUnitConverter();
    XMLNumberFormatAttributesExportHelper* GetNumberFormatAttributesExportHelper();
    ScMyStyleNumberFormats* GetStyleNumberFormats();

    void SetStylesToRangesFinished();

    // XImporter
    virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;

    virtual void SAL_CALL startDocument() override;
    virtual void SAL_CALL endDocument() override;

    virtual void DisposingModel() override;

    /**
     * Use this class to manage solar mutex locking instead of calling
     * LockSolarMutex() and UnlockSolarMutex() directly.
     */
    class MutexGuard
    {
    public:
        explicit MutexGuard(ScXMLImport& rImport);
        ~MutexGuard();
    private:
        ScXMLImport& mrImport;
    };
    void LockSolarMutex();
    void UnlockSolarMutex();

    sal_Int32 GetByteOffset();

    void SetRangeOverflowType(ErrCode nType);

    static sal_Int32 GetRangeType(const OUString& sRangeType);
    void SetNamedRanges();
    void SetSheetNamedRanges();
    void SetLabelRanges();
    void SetStringRefSyntaxIfMissing();

    /** Extracts the formula string, the formula grammar namespace URL, and a
        grammar enum value from the passed formula attribute value.

        @param rFormula
            (out-parameter) Returns the plain formula string with the leading
            equality sign if existing.

        @param rFormulaNmsp
            (out-parameter) Returns the URL of the formula grammar namespace if
            the attribute value contains the prefix of an unknown namespace.

        @param reGrammar
            (out-parameter) Returns the exact formula grammar if the formula
            is in a supported ODF format (e.g. FormulaGrammar::GRAM_PODF for
            ODF 1.0/1.1 formulas, or FormulaGrammar::GRAM_ODFF for ODF 1.2
            formulas a.k.a. OpenFormula). Returns the default storage grammar,
            if the attribute value does not contain a namespace prefix. Returns
            the special value FormulaGrammar::GRAM_EXTERNAL, if an unknown
            namespace could be extracted from the formula which will be
            contained in the parameter rFormulaNmsp then.

        @param rAttrValue
            The value of the processed formula attribute.

        @param bRestrictToExternalNmsp
            If set to true, only namespaces of external formula grammars will
            be recognized. Internal namespace prefixes (e.g. 'oooc:' or 'of:'
            will be considered to be part of the formula, e.g. an expression
            with range operator.
     */
    void ExtractFormulaNamespaceGrammar(
            OUString& rFormula,
            OUString& rFormulaNmsp,
            ::formula::FormulaGrammar::Grammar& reGrammar,
            const OUString& rAttrValue,
            bool bRestrictToExternalNmsp = false ) const;

    FormulaError GetFormulaErrorConstant( const OUString& rStr ) const;

    ScEditEngineDefaulter* GetEditEngine();
    const ScXMLEditAttributeMap& GetEditAttributeMap() const;
    virtual void NotifyEmbeddedFontRead() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
