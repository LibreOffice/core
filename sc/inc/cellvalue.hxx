/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_CELLVALUE_HXX
#define SC_CELLVALUE_HXX

#include "global.hxx"
#include "mtvelements.hxx"

class ScDocument;
class ScFormulaCell;
class EditTextObject;
class ScColumn;
struct ScRefCellValue;

/**
 * Store arbitrary cell value of any kind.  It only stores cell value and
 * nothing else.  It creates a copy of the original cell value, and manages
 * the life cycle of the copied value.
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
    ScCellValue( const ScRefCellValue& rCell );
    ScCellValue( double fValue );
    ScCellValue( const OUString& rString );
    ScCellValue( const EditTextObject& rEditText );
    ScCellValue( const ScFormulaCell& rFormula );
    ScCellValue( const ScCellValue& r );
    ~ScCellValue();

    void clear();

    void set( double fValue );
    void set( const OUString& rStr );
    void set( const EditTextObject& rEditText );
    void set( const ScFormulaCell& rFormula );
    void set( ScFormulaCell* pFormula );

    /**
     * Take cell value from specified position in specified document.
     */
    void assign( const ScDocument& rDoc, const ScAddress& rPos );

    void assign( const ScCellValue& rOther, ScDocument& rDestDoc, int nCloneFlags = SC_CLONECELL_DEFAULT );

    /**
     * Set cell value at specified position in specified document.
     */
    void commit( ScDocument& rDoc, const ScAddress& rPos ) const;

    void commit( ScColumn& rColumn, SCROW nRow ) const;

    /**
     * Set cell value at specified position in specified document. But unlike
     * commit(), this method sets the original value to the document without
     * copying.  After this call, the value gets cleared.
     */
    void release( ScDocument& rDoc, const ScAddress& rPos );

    void release( ScColumn& rColumn, SCROW nRow );

    bool hasString() const;

    bool hasNumeric() const;

    bool isEmpty() const;

    bool equalsWithoutFormat( const ScCellValue& r ) const;

    ScCellValue& operator= ( const ScCellValue& r );
    ScCellValue& operator= ( const ScRefCellValue& r );

    void swap( ScCellValue& r );
};

/**
 * This is very similar to ScCellValue, except that it references the
 * original value instead of copying it.  As such, don't hold an instance of
 * this class any longer than necessary, and absolutely not after the
 * original cell has been destroyed.
 */
struct SC_DLLPUBLIC ScRefCellValue
{
    CellType meType;
    union {
        double mfValue;
        const OUString* mpString;
        const EditTextObject* mpEditText;
        ScFormulaCell* mpFormula;
    };

    ScRefCellValue();
    ScRefCellValue( double fValue );
    ScRefCellValue( const OUString* pString );
    ScRefCellValue( const EditTextObject* pEditText );
    ScRefCellValue( ScFormulaCell* pFormula );
    ScRefCellValue( const ScRefCellValue& r );
    ~ScRefCellValue();

    void clear();

    /**
     * Take cell value from specified position in specified document.
     */
    void assign( ScDocument& rDoc, const ScAddress& rPos );

    void assign( const sc::CellStoreType::const_iterator& itPos, size_t nOffset );

    /**
     * Set cell value at specified position in specified document.
     */
    void commit( ScDocument& rDoc, const ScAddress& rPos ) const;

    void commit( ScColumn& rColumn, SCROW nRow ) const;

    bool hasString() const;

    bool hasNumeric() const;

    double getValue();

    /** Retrieve string value.

        @param  pDoc
                Needed to resolve EditCells' field contents, obtain a
                ScFieldEditEngine from that document. May be NULL if there is
                no ScDocument in the calling context but then the document
                specific fields can not be resolved. See
                ScEditUtil::GetString().
     */
    OUString getString( const ScDocument* pDoc );

    bool isEmpty() const;

    bool hasEmptyValue();

    bool equalsWithoutFormat( const ScRefCellValue& r ) const;

    ScRefCellValue& operator= ( const ScRefCellValue& r );

    void swap( ScRefCellValue& r );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
