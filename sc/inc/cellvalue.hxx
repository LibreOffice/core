/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_CELLVALUE_HXX__
#define __SC_CELLVALUE_HXX__

#include "global.hxx"

class ScDocument;
class ScFormulaCell;
class EditTextObject;
class ScBaseCell;

/**
 * Store arbitrary cell value of any kind.  It only stores cell value and
 * nothing else.
 */
struct SC_DLLPUBLIC ScCellValue
{
    CellType meType;
    union {
        double mfValue;
        OUString* mpString;
        EditTextObject* mpEditText;
        ScFormulaCell* mpFormula;
    };

    ScCellValue();
    ScCellValue( double fValue );
    ScCellValue( const OUString& rString );
    ScCellValue( const EditTextObject& rEditText );
    ScCellValue( const ScFormulaCell& rFormula );
    ScCellValue( const ScCellValue& r );
    ~ScCellValue();

    void clear();

    /**
     * Take cell value from specified position in specified document.
     */
    void assign( const ScDocument& rDoc, const ScAddress& rPos );

    void assign( const ScCellValue& rOther, ScDocument& rDestDoc, int nCloneFlags = SC_CLONECELL_DEFAULT );

    /**
     * TODO: Remove this later.
     */
    void assign( const ScBaseCell& rCell );

    /**
     * Set cell value at specified position in specified document.
     */
    void commit( ScDocument& rDoc, const ScAddress& rPos ) const;

    /**
     * Set cell value at specified position in specified document. But unlike
     * commit(), this method sets the original value to the document without
     * copying.  After this call, the value gets cleared.
     */
    void release( ScDocument& rDoc, const ScAddress& rPos );

    bool hasString() const;

    bool hasNumeric() const;

    bool isEmpty() const;

    bool equalsWithoutFormat( const ScCellValue& r ) const;

    ScCellValue& operator= ( const ScCellValue& r );

    void swap( ScCellValue& r );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
