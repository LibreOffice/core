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
#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLCELLI_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLCELLI_HXX

#include "XMLDetectiveContext.hxx"
#include "importcontext.hxx"
#include <formula/grammar.hxx>
#include <svl/itemset.hxx>
#include <editeng/editdata.hxx>

#include <optional>
#include <memory>
#include <vector>

class ScXMLImport;
class ScFormulaCell;
class ScEditEngineDefaulter;
class SvxFieldData;
struct ScXMLAnnotationData;
struct ScMyImpCellRangeSource;

class ScXMLTableRowCellContext : public ScXMLImportContext
{
    struct ParaFormat
    {
        SfxItemSet maItemSet;
        ESelection maSelection;

        explicit ParaFormat(const ScEditEngineDefaulter& rEditEngine);
    };

    struct Field
    {
        std::unique_ptr<SvxFieldData> mpData;
        ESelection maSelection;

        Field(const Field&) = delete;
        const Field& operator=(const Field&) = delete;

        explicit Field(std::unique_ptr<SvxFieldData> pData);
        ~Field();
    };

    typedef std::vector<std::unique_ptr<ParaFormat> > ParaFormatsType;
    typedef std::vector<std::unique_ptr<Field> > FieldsType;
    typedef std::pair<OUString, OUString> FormulaWithNamespace;

    std::optional<FormulaWithNamespace> maFormula; /// table:formula attribute
    std::optional<OUString> maStringValue;         /// office:string-value attribute
    std::optional<OUString> maContentValidationName;
    std::optional<OUString> maFirstParagraph; /// unformatted first paragraph, for better performance.

    ScEditEngineDefaulter* mpEditEngine;
    OUStringBuffer maParagraph{32};
    sal_Int32 mnCurParagraph;

    ParaFormatsType maFormats;
    FieldsType maFields;

    std::unique_ptr< ScXMLAnnotationData > mxAnnotationData;
    std::unique_ptr< ScMyImpDetectiveObjVec > pDetectiveObjVec;
    std::unique_ptr< ScMyImpCellRangeSource > pCellRangeSource;
    double      fValue;
    SCROW       nMergedRows, nMatrixRows, nRepeatedRows;
    SCCOL       nMergedCols, nMatrixCols, nColsRepeated;
    ScXMLImport& rXMLImport;
    formula::FormulaGrammar::Grammar  eGrammar;
    sal_Int16   nCellType;
    bool        bIsMerged;
    bool        bIsMatrix;
    bool        bIsCovered;
    bool        bIsEmpty;
    bool        mbNewValueType;
    bool        mbErrorValue;
    bool        bSolarMutexLocked;
    bool        bFormulaTextResult;
    bool mbPossibleErrorCell;
    bool mbCheckWithCompilerForError;
    bool mbEditEngineHasText;
    bool mbHasFormatRuns;
    bool mbHasStyle;
    bool mbPossibleEmptyDisplay;

    void DoMerge(const ScAddress& rScCellPos, const SCCOL nCols, const SCROW nRows);

    void SetContentValidation( const ScRange& rScRange );
    void SetContentValidation( const ScAddress& rScCellPos );

    void LockSolarMutex();

    bool CellsAreRepeated() const;

    void SetFormulaCell             ( ScFormulaCell* pFCell ) const;
    void PutTextCell                ( const ScAddress& rScCurrentPos, const SCCOL nCurrentCol,
                                      const ::std::optional< OUString >& pOUText );
    void PutValueCell               ( const ScAddress& rScCurrentPos );
    void AddTextAndValueCell       ( const ScAddress& rScCellPos,
                                      const ::std::optional< OUString >& pOUText, ScAddress& rScCurrentPos );
    void AddNonFormulaCell         ( const ScAddress& rScCellPos );
    void PutFormulaCell             ( const ScAddress& rScCurrentPos );
    void AddFormulaCell             ( const ScAddress& rScCellPos );

    void HasSpecialCaseFormulaText();

    bool IsPossibleErrorString() const;

    void PushParagraphField(std::unique_ptr<SvxFieldData> pData, const OUString& rStyleName);

    void PushFormat(sal_Int32 nBegin, sal_Int32 nEnd, const OUString& rStyleName);

    OUString GetFirstParagraph() const;

public:

    ScXMLTableRowCellContext( ScXMLImport& rImport,
                       const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                       const bool bIsCovered, const sal_Int32 nRepeatedRows );

    virtual ~ScXMLTableRowCellContext() override;

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    void PushParagraphSpan(const OUString& rSpan, const OUString& rStyleName);
    void PushParagraphFieldDate(const OUString& rStyleName);
    void PushParagraphFieldSheetName(const OUString& rStyleName);
    void PushParagraphFieldDocTitle(const OUString& rStyleName);
    void PushParagraphFieldURL(const OUString& rURL, const OUString& rRep, const OUString& rStyleName, const OUString& rTargetFrame);
    void PushParagraphEnd();

    void SetAnnotation( const ScAddress& rPosition );
    void SetDetectiveObj( const ScAddress& rPosition );
    void SetCellRangeSource( const ScAddress& rPosition );

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
