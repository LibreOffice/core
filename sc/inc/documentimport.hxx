/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "scdllapi.h"
#include "address.hxx"
#include "attarray.hxx"

#include <rtl/ustring.hxx>

#include <memory>
#include <vector>

class EditTextObject;
class ScDocument;
class ScColumn;
class ScPatternAttr;
class ScTokenArray;
class ScFormulaCell;
class ScStyleSheet;
struct ScSetStringParam;
struct ScTabOpParam;
struct ScDocumentImportImpl;
enum class SvtScriptType : sal_uInt8;

/**
 * Accessor class to ScDocument.  Its purpose is to allow import filter to
 * fill the document model and nothing but that.  Filling the document via
 * this class does not trigger any kind of broadcasting, drawing object
 * position calculation, or anything else that requires expensive
 * computation which are unnecessary and undesirable during import.
 */
class SC_DLLPUBLIC ScDocumentImport
{
    std::unique_ptr<ScDocumentImportImpl> mpImpl;

public:

    struct SC_DLLPUBLIC Attrs
    {
        std::vector<ScAttrEntry> mvData;

        bool mbLatinNumFmtOnly;

        Attrs();
        ~Attrs();
        Attrs& operator=( Attrs const & ) = delete; // MSVC2015 workaround
        Attrs( Attrs const & ) = delete; // MSVC2015 workaround
        bool operator==(const Attrs& other) const
        {
            return mvData == other.mvData && mbLatinNumFmtOnly == other.mbLatinNumFmtOnly;
        }
        Attrs& operator=( Attrs&& attrs ) = default;
    };

    ScDocumentImport() = delete;
    ScDocumentImport(ScDocument& rDoc);
    ScDocumentImport(const ScDocumentImport&) = delete;
    const ScDocumentImport& operator=(const ScDocumentImport&) = delete;
    ~ScDocumentImport();

    ScDocument& getDoc();
    const ScDocument& getDoc() const;

    /**
     * Initialize the storage for all sheets after all the sheet instances
     * have been created in the document.
     */
    void initForSheets();

    void setDefaultNumericScript(SvtScriptType nScript);

    /**
     * Apply specified cell style to an entire sheet.
     */
    void setCellStyleToSheet(SCTAB nTab, const ScStyleSheet& rStyle);

    /**
     * @param rName sheet name.
     *
     * @return 0-based sheet index, or -1 in case no sheet is found by
     *         specified name.
     */
    SCTAB getSheetIndex(const OUString& rName) const;
    SCTAB getSheetCount() const;
    bool appendSheet(const OUString& rName);
    void setSheetName(SCTAB nTab, const OUString& rName);

    void setOriginDate(sal_uInt16 nYear, sal_uInt16 nMonth, sal_uInt16 nDay);

    void setAutoInput(const ScAddress& rPos, const OUString& rStr,
            const ScSetStringParam* pStringParam = nullptr);
    void setNumericCell(const ScAddress& rPos, double fVal);
    void setStringCell(const ScAddress& rPos, const OUString& rStr);
    void setEditCell(const ScAddress& rPos, std::unique_ptr<EditTextObject> pEditText);

    void setFormulaCell(
        const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGrammar,
        const double* pResult = nullptr );

    void setFormulaCell(
        const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGrammar,
        const OUString& rResult );

    void setFormulaCell(const ScAddress& rPos, std::unique_ptr<ScTokenArray> pArray);
    void setFormulaCell(const ScAddress& rPos, ScFormulaCell* pCell);

    void setMatrixCells(
        const ScRange& rRange, const ScTokenArray& rArray, formula::FormulaGrammar::Grammar eGrammar);

    void setTableOpCells(const ScRange& rRange, const ScTabOpParam& rParam);

    void fillDownCells(const ScAddress& rPos, SCROW nFillSize);

    /**
     * Set an array of cell attributes to specified range of columns.  This call
     * transfers the ownership of the ScAttrEntry array from the caller to the
     * column.
     */
    void setAttrEntries( SCTAB nTab, SCCOL nColStart, SCCOL nColEnd, Attrs&& rAttrs );

    void setRowsVisible(SCTAB nTab, SCROW nRowStart, SCROW nRowEnd, bool bVisible);

    void setMergedCells(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);

    void invalidateBlockPositionSet(SCTAB nTab);

    void finalize();

    /** Broadcast all formula cells that are marked with
        FormulaTokenArray::IsRecalcModeMustAfterImport() for a subsequent
        ScDocument::CalcFormulaTree().
     */
    void broadcastRecalcAfterImport();

    /** small cache for hot call during import */
    bool isLatinScript(sal_uInt32 nFormat);
    bool isLatinScript(const ScPatternAttr&);

private:
    void initColumn(ScColumn& rCol);
    static void broadcastRecalcAfterImportColumn(ScColumn& rCol);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
