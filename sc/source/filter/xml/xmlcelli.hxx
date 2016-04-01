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
#include "XMLCellRangeSourceContext.hxx"
#include "importcontext.hxx"
#include <formula/grammar.hxx>
#include <svl/itemset.hxx>
#include <editeng/editdata.hxx>
#include <editeng/flditem.hxx>

#include <boost/optional.hpp>
#include <memory>
#include <vector>

class ScXMLImport;
class ScFormulaCell;
class ScEditEngineDefaulter;
struct ScXMLAnnotationData;

class ScXMLTableRowCellContext : public ScXMLImportContext
{
    struct ParaFormat
    {
        SfxItemSet maItemSet;
        ESelection maSelection;

        explicit ParaFormat(ScEditEngineDefaulter& rEditEngine);
    };

    struct Field
    {
        SvxFieldData* mpData;
        ESelection maSelection;

        Field(const Field&) = delete;
        const Field& operator=(const Field&) = delete;

        explicit Field(SvxFieldData* pData);
        ~Field();
    };

    typedef std::vector<std::unique_ptr<ParaFormat> > ParaFormatsType;
    typedef std::vector<std::unique_ptr<Field> > FieldsType;
    typedef std::pair<OUString, OUString> FormulaWithNamespace;

    boost::optional<FormulaWithNamespace> maFormula; /// table:formula attribute
    boost::optional<OUString> maStringValue;         /// office:string-value attribute
    boost::optional<OUString> maContentValidationName;
    boost::optional<OUString> maFirstParagraph; /// unformatted first paragraph, for better performance.

    ScEditEngineDefaulter* mpEditEngine;
    OUStringBuffer maParagraph;
    sal_Int32 mnCurParagraph;

    ParaFormatsType maFormats;
    FieldsType maFields;

    std::unique_ptr< ScXMLAnnotationData > mxAnnotationData;
    ScMyImpDetectiveObjVec* pDetectiveObjVec;
    ScMyImpCellRangeSource* pCellRangeSource;
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

    void DoMerge(const ScAddress& rScCellPos, const SCCOL nCols, const SCROW nRows);

    void SetContentValidation( const ScRange& rScRange );
    void SetContentValidation( const ScAddress& rScCellPos );

    void LockSolarMutex();
    void UnlockSolarMutex();

    bool HasSpecialContent() const;
    bool CellsAreRepeated() const;

    void SetFormulaCell             ( ScFormulaCell* pFCell ) const;
    void PutTextCell                ( const ScAddress& rScCurrentPos, const SCCOL nCurrentCol,
                                      const ::boost::optional< OUString >& pOUText );
    void PutValueCell               ( const ScAddress& rScCurrentPos );
    void AddTextAndValueCell       ( const ScAddress& rScCellPos,
                                      const ::boost::optional< OUString >& pOUText, ScAddress& rScCurrentPos );
    void AddNonFormulaCell         ( const ScAddress& rScCellPos );
    void PutFormulaCell             ( const ScAddress& rScCurrentPos );
    void AddFormulaCell             ( const ScAddress& rScCellPos );

    void HasSpecialCaseFormulaText();

    bool IsPossibleErrorString() const;

    void PushParagraphField(SvxFieldData* pData, const OUString& rStyleName);

    void PushFormat(sal_Int32 nBegin, sal_Int32 nEnd, const OUString& rStyleName);

    OUString GetFirstParagraph() const;

public:

    ScXMLTableRowCellContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                       const OUString& rLName,
                       const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                       const bool bIsCovered, const sal_Int32 nRepeatedRows );

    virtual ~ScXMLTableRowCellContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    void PushParagraphSpan(const OUString& rSpan, const OUString& rStyleName);
    void PushParagraphFieldDate(const OUString& rStyleName);
    void PushParagraphFieldSheetName(const OUString& rStyleName);
    void PushParagraphFieldDocTitle(const OUString& rStyleName);
    void PushParagraphFieldURL(const OUString& rURL, const OUString& rRep, const OUString& rStyleName);
    void PushParagraphEnd();

    void SetAnnotation( const ScAddress& rPosition );
    void SetDetectiveObj( const ScAddress& rPosition );
    void SetCellRangeSource( const ScAddress& rPosition );

    virtual void EndElement() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
