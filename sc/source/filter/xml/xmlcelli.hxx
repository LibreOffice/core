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
#ifndef SC_XMLCELLI_HXX
#define SC_XMLCELLI_HXX

#include "XMLDetectiveContext.hxx"
#include "XMLCellRangeSourceContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>

#include "formula/grammar.hxx"
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>

class ScXMLImport;
class ScFormulaCell;
struct ScXMLAnnotationData;

class ScXMLTableRowCellContext : public SvXMLImportContext
{
    typedef std::pair<OUString, OUString> FormulaWithNamespace;

    boost::optional<FormulaWithNamespace> maFormula; /// table:formula attribute
    boost::optional<OUString> maStringValue;         /// office:string-value attribute
    boost::optional<OUString> maContentValidationName;

    std::vector<OUString> maParagraphs;

    boost::scoped_ptr< ScXMLAnnotationData > mxAnnotationData;
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
    bool        bIsFirstTextImport;
    bool        bSolarMutexLocked;
    bool        bFormulaTextResult;
    bool        mbPossibleErrorCell;
    bool        mbCheckWithCompilerForError;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }

    sal_Int16 GetCellType(const rtl::OUString& sOUValue) const;

    void DoMerge(const ScAddress& rScCellPos, const SCCOL nCols, const SCROW nRows);

    void SetContentValidation( const ScRange& rScRange );
    void SetContentValidation( const ScAddress& rScCellPos );

    void LockSolarMutex();
    void UnlockSolarMutex();

    bool HasSpecialContent() const;
    bool CellsAreRepeated() const;

    void SetFormulaCell             ( ScFormulaCell* pFCell ) const;
    void PutTextCell                ( const ScAddress& rScCurrentPos, const SCCOL nCurrentCol,
                                      const ::boost::optional< rtl::OUString >& pOUText );
    void PutValueCell               ( const ScAddress& rScCurrentPos );
    void AddTextAndValueCells       ( const ScAddress& rScCellPos,
                                      const ::boost::optional< rtl::OUString >& pOUText, ScAddress& rScCurrentPos );
    void AddNonFormulaCells         ( const ScAddress& rScCellPos );
    void PutFormulaCell             ( const ScAddress& rScCurrentPos );
    void AddFormulaCell             ( const ScAddress& rScCellPos );

    void HasSpecialCaseFormulaText();

    bool IsPossibleErrorString() const;

public:

    ScXMLTableRowCellContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                       const ::rtl::OUString& rLName,
                       const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        const bool bIsCovered, const sal_Int32 nRepeatedRows );

    virtual ~ScXMLTableRowCellContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    void PushParagraph(const OUString& rPara);

    void SetAnnotation( const ScAddress& rPosition );
    void SetDetectiveObj( const ScAddress& rPosition );
    void SetCellRangeSource( const ScAddress& rPosition );

    virtual void EndElement();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
