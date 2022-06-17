/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "global.hxx"

class ScDocument;
class ScFormulaCell;
class EditTextObject;
class ScColumn;
struct ScRefCellValue;

namespace sc {
struct ColumnBlockPosition;
}

namespace svl {

class SharedString;

}

/**
 * Store arbitrary cell value of any kind.  It only stores cell value and
 * nothing else.  It creates a copy of the original cell value, and manages
 * the life cycle of the copied value.
 */
struct SC_DLLPUBLIC ScCellValue
{
private:
    CellType meType;
public:
    union {
        double mfValue1;
        svl::SharedString* mpString;
        EditTextObject* mpEditText1;
        ScFormulaCell* mpFormula1;
    };

    ScCellValue();
    ScCellValue( const ScRefCellValue& rCell );
    ScCellValue( double fValue );
    ScCellValue( const svl::SharedString& rString );
    ScCellValue( std::unique_ptr<EditTextObject> );
    ScCellValue( const ScCellValue& r );
    ScCellValue(ScCellValue&& r) noexcept;
    ~ScCellValue();

    void clear() noexcept;

    void set( double fValue );
    void set( const svl::SharedString& rStr );
    void set( const EditTextObject& rEditText );
    void set( std::unique_ptr<EditTextObject> );
    void set( ScFormulaCell* pFormula );

    CellType getType() const { return meType; }
    double getDouble() const { assert(meType == CELLTYPE_VALUE); return mfValue1; }
    svl::SharedString* getSharedString() const { assert(meType == CELLTYPE_STRING); return mpString; }
    EditTextObject* getEditText() const { assert(meType == CELLTYPE_EDIT); return mpEditText1; }
    EditTextObject* releaseEditText()
    {
        assert(meType == CELLTYPE_EDIT);
        auto p = mpEditText1;
        mpEditText1 = nullptr;
        return p;
    }
    ScFormulaCell* getFormula() const { assert(meType == CELLTYPE_FORMULA); return mpFormula1; }
    ScFormulaCell* releaseFormula()
    {
        assert(meType == CELLTYPE_FORMULA);
        auto p = mpFormula1;
        mpFormula1 = nullptr;
        return p;
    }

    /**
     * Take cell value from specified position in specified document.
     */
    void assign( const ScDocument& rDoc, const ScAddress& rPos );

    void assign(const ScCellValue& rOther, ScDocument& rDestDoc, ScCloneFlags nCloneFlags = ScCloneFlags::Default);

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

    void release( ScColumn& rColumn, SCROW nRow, sc::StartListeningType eListenType = sc::SingleCellListening );

    OUString getString( const ScDocument& rDoc ) const;

    bool isEmpty() const;

    bool equalsWithoutFormat( const ScCellValue& r ) const;

    ScCellValue& operator= ( const ScCellValue& r );
    ScCellValue& operator=(ScCellValue&& r) noexcept;
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
private:
    CellType meType;
public:
    union {
        double mfValue1;
        const svl::SharedString* mpString;
        const EditTextObject* mpEditText1;
        ScFormulaCell* mpFormula1;
    };

    ScRefCellValue();
    ScRefCellValue( double fValue );
    ScRefCellValue( const svl::SharedString* pString );
    ScRefCellValue( const EditTextObject* pEditText );
    ScRefCellValue( ScFormulaCell* pFormula );

    /**
     * Take cell value from specified position in specified document.
     */
    ScRefCellValue( ScDocument& rDoc, const ScAddress& rPos );
    ScRefCellValue( ScDocument& rDoc, const ScAddress& rPos, sc::ColumnBlockPosition& rBlockPos );

    void clear();

    CellType getType() const { return meType; }
    double getDouble() const { assert(meType == CELLTYPE_VALUE); return mfValue1; }
    const svl::SharedString* getSharedString() const { assert(meType == CELLTYPE_STRING); return mpString; }
    const EditTextObject* getEditText() const { assert(meType == CELLTYPE_EDIT); return mpEditText1; }
    ScFormulaCell* getFormula() const { assert(meType == CELLTYPE_FORMULA); return mpFormula1; }

    /**
     * Take cell value from specified position in specified document.
     */
    void assign( ScDocument& rDoc, const ScAddress& rPos );
    void assign( ScDocument& rDoc, const ScAddress& rPos, sc::ColumnBlockPosition& rBlockPos );

    /**
     * Set cell value at specified position in specified document.
     */
    void commit( ScDocument& rDoc, const ScAddress& rPos ) const;

    bool hasString() const;

    bool hasNumeric() const;

    bool hasError() const;

    double getValue();

    /**
     * Retrieve a numeric value without modifying the states of any objects in
     * the referenced document store.
     */
    double getRawValue() const;

    /**
     *  Retrieve string value.
     *
     *  Note that this method is NOT thread-safe.
     *
     *  @param  pDoc
     *          Needed to resolve EditCells' field contents, obtain a
     *          ScFieldEditEngine from that document. May be NULL if there is
     *          no ScDocument in the calling context but then the document
     *          specific fields can not be resolved. See
     *          ScEditUtil::GetString().
     */
    OUString getString( const ScDocument* pDoc ) const;

    /**
     * Retrieve a string value without modifying the states of any objects in
     * the referenced document store.
     *
     * This method is thread-safe.
     */
    OUString getRawString( const ScDocument& rDoc ) const;

    bool isEmpty() const;

    bool hasEmptyValue();

    bool equalsWithoutFormat( const ScRefCellValue& r ) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
