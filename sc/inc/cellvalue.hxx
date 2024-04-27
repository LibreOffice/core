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
#include <svl/sharedstring.hxx>
#include <variant>

class ScDocument;
class ScFormulaCell;
class EditTextObject;
class ScColumn;
struct ScRefCellValue;

namespace sc {
struct ColumnBlockPosition;
}

/**
 * Store arbitrary cell value of any kind.  It only stores cell value and
 * nothing else.  It creates a copy of the original cell value, and manages
 * the life cycle of the copied value.
 */
struct ScCellValue
{
private:
    /// std::monostate is there to indicate CellType::NONE
    std::variant<std::monostate, double, svl::SharedString, EditTextObject*, ScFormulaCell*> maData;

    void reset_to_empty();
public:

    SC_DLLPUBLIC ScCellValue();
    ScCellValue( const ScRefCellValue& rCell );
    ScCellValue( double fValue );
    ScCellValue( const svl::SharedString& rString );
    ScCellValue( std::unique_ptr<EditTextObject> );
    ScCellValue( const ScCellValue& r );
    ScCellValue(ScCellValue&& r) noexcept;
    SC_DLLPUBLIC ~ScCellValue();

    SC_DLLPUBLIC void clear() noexcept;

    SC_DLLPUBLIC void set( double fValue );
    SC_DLLPUBLIC void set( const svl::SharedString& rStr );
    void set( const EditTextObject& rEditText );
    SC_DLLPUBLIC void set( std::unique_ptr<EditTextObject> );
    SC_DLLPUBLIC void set( ScFormulaCell* pFormula );

    SC_DLLPUBLIC CellType getType() const;
    double getDouble() const { return std::get<double>(maData); }
    ScFormulaCell* getFormula() const { return std::get<ScFormulaCell*>(maData); }
    const svl::SharedString* getSharedString() const { return &std::get<svl::SharedString>(maData); }
    EditTextObject* getEditText() const { return std::get<EditTextObject*>(maData); }
    EditTextObject* releaseEditText() { auto p = getEditText(); maData = static_cast<EditTextObject*>(nullptr); return p; }
    ScFormulaCell* releaseFormula() { auto p = getFormula(); maData = static_cast<ScFormulaCell*>(nullptr); return p; }

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

    SC_DLLPUBLIC bool isEmpty() const;

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
struct SAL_DLLPUBLIC_RTTI ScRefCellValue
{
private:
    CellType meType;
    union {
        double mfValue;
        const svl::SharedString* mpString;
        const EditTextObject* mpEditText;
        ScFormulaCell* mpFormula;
    };
public:

    SC_DLLPUBLIC ScRefCellValue();
    SC_DLLPUBLIC ScRefCellValue( double fValue );
    ScRefCellValue( const svl::SharedString* pString );
    ScRefCellValue( const EditTextObject* pEditText );
    ScRefCellValue( ScFormulaCell* pFormula );

    /**
     * Take cell value from specified position in specified document.
     */
    SC_DLLPUBLIC ScRefCellValue( ScDocument& rDoc, const ScAddress& rPos );
    SC_DLLPUBLIC ScRefCellValue( ScDocument& rDoc, const ScAddress& rPos, sc::ColumnBlockPosition& rBlockPos );

    void clear();

    CellType getType() const { return meType; }
    double getDouble() const { assert(meType == CELLTYPE_VALUE); return mfValue; }
    const svl::SharedString* getSharedString() const { assert(meType == CELLTYPE_STRING); return mpString; }
    const EditTextObject* getEditText() const { assert(meType == CELLTYPE_EDIT); return mpEditText; }
    ScFormulaCell* getFormula() const { assert(meType == CELLTYPE_FORMULA); return mpFormula; }

    /**
     * Take cell value from specified position in specified document.
     */
    SC_DLLPUBLIC void assign( ScDocument& rDoc, const ScAddress& rPos );
    void assign( ScDocument& rDoc, const ScAddress& rPos, sc::ColumnBlockPosition& rBlockPos );

    /**
     * Set cell value at specified position in specified document.
     */
    void commit( ScDocument& rDoc, const ScAddress& rPos ) const;

    bool hasString() const;

    SC_DLLPUBLIC bool hasNumeric() const;

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
    SC_DLLPUBLIC OUString getString( const ScDocument* pDoc ) const;

    /**
     * Retrieve a string value without modifying the states of any objects in
     * the referenced document store.
     *
     * This method is thread-safe.
     */
    OUString getRawString( const ScDocument& rDoc ) const;

    SC_DLLPUBLIC bool isEmpty() const;

    bool hasEmptyValue();

    bool equalsWithoutFormat( const ScRefCellValue& r ) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
