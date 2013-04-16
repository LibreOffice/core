/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_DOCUMENTIMPORT_HXX
#define SC_DOCUMENTIMPORT_HXX

#include "scdllapi.h"
#include "address.hxx"

#include "rtl/ustring.hxx"

class ScDocument;
class ScAddress;
class ScTokenArray;
class ScBaseCell;

/**
 * Accessor class to ScDocument.  Its purpose is to allow import filter to
 * fill the document model and nothing but that.  Filling the document via
 * this class does not trigger any kind of broadcasting, drawing object
 * position calculation, or anything else that requires expensive
 * computation which are unnecessary and undesirable during import.
 */
class SC_DLLPUBLIC ScDocumentImport
{
    ScDocument& mrDoc;

    ScDocumentImport(); // disabled

public:
    ScDocumentImport(ScDocument& rDoc);
    ScDocumentImport(const ScDocumentImport& r);

    ScDocument& getDoc();
    const ScDocument& getDoc() const;

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

    void setAutoInput(const ScAddress& rPos, const OUString& rStr);
    void setNumericCell(const ScAddress& rPos, double fVal);
    void setStringCell(const ScAddress& rPos, const OUString& rStr);
    void setFormulaCell(const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGrammar);
    void setFormulaCell(const ScAddress& rPos, const ScTokenArray& rArray);

    void finalize();

private:
    void insertCell(const ScAddress& rPos, ScBaseCell* pCell);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
