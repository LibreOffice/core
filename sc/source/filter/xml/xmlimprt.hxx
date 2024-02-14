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

#pragma once

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlprmap.hxx>
#include "xmlsubti.hxx"
#include <formula/grammar.hxx>
#include <vcl/svapp.hxx>
#include <dociter.hxx>

#include <com/sun/star/sheet/ValidationAlertStyle.hpp>
#include <com/sun/star/sheet/ValidationType.hpp>
#include <com/sun/star/sheet/ConditionOperator.hpp>

#include <memory>
#include <map>
#include <vector>
#include <list>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::sheet { class XSheetCellRangeContainer; }
namespace com::sun::star::table { struct CellRangeAddress; }
namespace com::sun::star::util { class XNumberFormatTypes; }
namespace com::sun::star::util { class XNumberFormats; }
namespace sax_fastparser { class FastAttributeList; }

class ScCompiler;
class ErrCode;
class ScMyStyleNumberFormats;
class XMLNumberFormatAttributesExportHelper;
class ScEditEngineDefaulter;
class ScDocumentImport;
class ScMyImpDetectiveOpArray;
class SdrPage;
class ScModelObj;

namespace sc {
struct ImportPostProcessData;
struct PivotTableSources;
class ScDrawObjData;
}


class SvXMLTokenMap;
class XMLShapeImportHelper;
class ScXMLChangeTrackingImportHelper;

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

typedef ::std::list<ScMyNamedExpression> ScMyNamedExpressions;

struct ScMyLabelRange
{
    OUString   sLabelRangeStr;
    OUString   sDataRangeStr;
    bool       bColumnOrientation;
};

typedef std::list<ScMyLabelRange> ScMyLabelRanges;

struct ScMyImportValidation
{
    OUString                                   sName;
    OUString                                   sInputTitle;
    OUString                                   sInputMessage;
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
    bool                                       bShowInputMessage;
    bool                                       bIgnoreBlanks;
};

typedef std::vector<ScMyImportValidation>           ScMyImportValidations;
class ScMyStylesImportHelper;
class ScXMLEditAttributeMap;
class ScCellRangesObj;

class ScXMLImport: public SvXMLImport
{
    ScXMLImport(const ScXMLImport&) = delete;
    const ScXMLImport& operator=(const ScXMLImport&) = delete;

    typedef ::std::map<SCTAB, ScMyNamedExpressions> SheetNamedExpMap;

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

    sc::ImportPostProcessData* mpPostProcessData; /// Lift cycle managed elsewhere, no need to delete.

    ScMyTables              aTables;

    std::vector<ScDocRowHeightUpdater::TabRanges> maRecalcRowRanges;

    ScMyNamedExpressions   m_aMyNamedExpressions;
    SheetNamedExpMap m_SheetNamedExpressions;

    ScMyLabelRanges            maMyLabelRanges;
    ScMyImportValidations      maValidations;
    std::unique_ptr<ScMyImpDetectiveOpArray>    pDetectiveOpArray;
    std::optional<SolarMutexGuard> moSolarMutexGuard;

    std::unique_ptr<XMLNumberFormatAttributesExportHelper> pNumberFormatAttributesExportHelper;
    std::unique_ptr<ScMyStyleNumberFormats> pStyleNumberFormats;
    css::uno::Reference <css::util::XNumberFormats> xNumberFormats;
    css::uno::Reference <css::util::XNumberFormatTypes> xNumberFormatTypes;

    rtl::Reference<ScCellRangesObj> mxSheetCellRanges; // css::sheet::XSheetCellRangeContainer

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
        OUString const & implementationName, SvXMLImportFlags nImportFlag,
        const css::uno::Sequence< OUString > & sSupportedServiceNames = {});

    virtual ~ScXMLImport() noexcept override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence<css::uno::Any>& aArguments ) override;

    // namespace office
    // NB: in contrast to other CreateFooContexts, this particular one handles
    //     the root element (i.e. office:document-meta)
    SvXMLImportContext *CreateMetaContext( sal_Int32 nElement );
    SvXMLImportContext *CreateFontDeclsContext();
    SvXMLImportContext *CreateScriptContext();
    SvXMLImportContext *CreateStylesContext( bool bAutoStyles );

    SvXMLImportContext *CreateBodyContext(
                                    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList );

    virtual void SetStatistics( const css::uno::Sequence< css::beans::NamedValue> & i_rStats) override;

    ScDocumentImport& GetDoc();

    ScDocument*          GetDocument()           { return pDoc; }
    const ScDocument*    GetDocument() const     { return pDoc; }

    ScModelObj* GetScModel() const;

    ScMyTables& GetTables() { return aTables; }

    std::vector<ScDocRowHeightUpdater::TabRanges>& GetRecalcRowRanges() { return maRecalcRowRanges; }

    bool IsStylesOnlyMode() const { return !bLoadDoc; }

    static sal_Int16 GetCellType(const char* rStrValue, const sal_Int32 nStrLength);

    const rtl::Reference < XMLPropertySetMapper >& GetCellStylesPropertySetMapper() const { return xCellStylesPropertySetMapper; }
    const rtl::Reference < XMLPropertySetMapper >& GetColumnStylesPropertySetMapper() const { return xColumnStylesPropertySetMapper; }
    const rtl::Reference < XMLPropertySetMapper >& GetRowStylesPropertySetMapper() const { return xRowStylesPropertySetMapper; }
    const rtl::Reference < XMLPropertySetMapper >& GetTableStylesPropertySetMapper() const { return xTableStylesPropertySetMapper; }

    void SetPostProcessData( sc::ImportPostProcessData* p );
    sc::ImportPostProcessData* GetPostProcessData() { return mpPostProcessData;}

    sc::PivotTableSources& GetPivotTableSources();

    void AddNamedExpression(ScMyNamedExpression aMyNamedExpression)
    {
        m_aMyNamedExpressions.push_back(std::move(aMyNamedExpression));
    }

    void AddNamedExpression(SCTAB nTab, ScMyNamedExpression aNamedExp);

    void AddLabelRange(ScMyLabelRange aMyLabelRange)
    {
        maMyLabelRanges.push_back(std::move(aMyLabelRange));
    }

    void AddValidation(const ScMyImportValidation& rValidation) { maValidations.push_back(rValidation); }
    bool GetValidation(const OUString& sName, ScMyImportValidation& aValidation);

    ScMyImpDetectiveOpArray* GetDetectiveOpArray();

    ScXMLChangeTrackingImportHelper* GetChangeTrackingImportHelper();
    void InsertStyles();

    void SetChangeTrackingViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& rChangeProps);
    virtual void SetViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& aViewProps) override;
    virtual void SetConfigurationSettings(const css::uno::Sequence<css::beans::PropertyValue>& aConfigProps) override;

    ScMyStylesImportHelper* GetStylesImportHelper() { return pStylesImportHelper.get(); }
    sal_Int32 SetCurrencySymbol(const sal_Int32 nKey, std::u16string_view rCurrency);
    bool IsCurrencySymbol(const sal_Int32 nNumberFormat, std::u16string_view sCurrencySymbol, std::u16string_view sBankSymbol);
    void SetType(const css::uno::Reference <css::beans::XPropertySet>& rProperties,
        sal_Int32& rNumberFormat,
        const sal_Int16 nCellType,
        std::u16string_view rCurrency);

    void ProgressBarIncrement();

    void SetNewCondFormatData() { mbHasNewCondFormatData = true; }
    bool HasNewCondFormatData() const { return mbHasNewCondFormatData; }

private:
    void SetStyleToRanges();

    void ExamineDefaultStyle();
public:
    void SetStyleToRanges(const ScRangeList& rRanges, const OUString* pStyleName,
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

    sal_Int64 GetByteOffset() const;

    void SetRangeOverflowType(ErrCode nType);

    static sal_Int32 GetRangeType(std::u16string_view sRangeType);
    bool GetRecalcRowHeightsMode();
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
    virtual void NotifyContainsEmbeddedFont() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
