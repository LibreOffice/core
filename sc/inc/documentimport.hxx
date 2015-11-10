/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_DOCUMENTIMPORT_HXX
#define INCLUDED_SC_INC_DOCUMENTIMPORT_HXX

#include "scdllapi.h"
#include "address.hxx"

#include <rtl/ustring.hxx>

#include <boost/noncopyable.hpp>

class EditTextObject;
class ScDocument;
class ScColumn;
class ScAddress;
struct ScAttrEntry;
class ScTokenArray;
class ScFormulaCell;
class ScStyleSheet;
struct ScSetStringParam;
struct ScTabOpParam;
struct ScDocumentImportImpl;
enum class SvtScriptType;

/**
 * Accessor class to ScDocument.  Its purpose is to allow import filter to
 * fill the document model and nothing but that.  Filling the document via
 * this class does not trigger any kind of broadcasting, drawing object
 * position calculation, or anything else that requires expensive
 * computation which are unnecessary and undesirable during import.
 */
class SC_DLLPUBLIC ScDocumentImport : boost::noncopyable
{
    ScDocumentImportImpl* mpImpl;

    ScDocumentImport(); // disabled
    ScDocumentImport& operator=(const ScDocumentImport&); //disabled

public:

    struct SC_DLLPUBLIC Attrs
    {
        ScAttrEntry* mpData;
        size_t mnSize;

        bool mbLatinNumFmtOnly;

        Attrs();
    };

    ScDocumentImport(ScDocument& rDoc);
    ~ScDocumentImport();

    ScDocument& getDoc();
    const ScDocument& getDoc() const;

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

    void setOriginDate(sal_uInt16 nYear, sal_uInt16 nMonth, sal_uInt16 nDay);

    void setAutoInput(const ScAddress& rPos, const OUString& rStr,
            ScSetStringParam* pStringParam = nullptr);
    void setNumericCell(const ScAddress& rPos, double fVal);
    void setStringCell(const ScAddress& rPos, const OUString& rStr);
    void setEditCell(const ScAddress& rPos, EditTextObject* pEditText);
    void setFormulaCell(const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGrammar);
    void setFormulaCell(const ScAddress& rPos, ScTokenArray* pArray);
    void setFormulaCell(const ScAddress& rPos, ScFormulaCell* pCell);

    void setMatrixCells(
        const ScRange& rRange, const ScTokenArray& rArray, formula::FormulaGrammar::Grammar eGrammar);

    void setTableOpCells(const ScRange& rRange, const ScTabOpParam& rParam);

    /**
     * Set an array of cell attributes to specified column.  This call
     * transfers the ownership of the ScAttrEntry array from the caller to the
     * column.
     */
    void setAttrEntries( SCTAB nTab, SCCOL nCol, Attrs& rAttrs );

    void finalize();

private:
    void initColumn(ScColumn& rCol);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
