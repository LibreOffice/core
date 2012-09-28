/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef OOX_XLS_DEFINEDNAMESBUFFER_HXX
#define OOX_XLS_DEFINEDNAMESBUFFER_HXX

#include "formulabase.hxx"
#include "rangenam.hxx"

namespace com { namespace sun { namespace star {
    namespace sheet { class XNamedRange; }
} } }

namespace oox {
namespace xls {

class BiffInputStreamPos;

// ============================================================================

// codes for built-in names
const sal_Unicode BIFF_DEFNAME_CONSOLIDATEAREA  = '\x00';
const sal_Unicode BIFF_DEFNAME_AUTOOPEN         = '\x01';   // Sheet macro executed when workbook is opened.
const sal_Unicode BIFF_DEFNAME_AUTOCLOSE        = '\x02';   // Sheet macro executed when workbook is closed.
const sal_Unicode BIFF_DEFNAME_EXTRACT          = '\x03';   // Filter output destination for advanced filter.
const sal_Unicode BIFF_DEFNAME_DATABASE         = '\x04';
const sal_Unicode BIFF_DEFNAME_CRITERIA         = '\x05';   // Filter criteria source range for advanced filter.
const sal_Unicode BIFF_DEFNAME_PRINTAREA        = '\x06';   // Print ranges.
const sal_Unicode BIFF_DEFNAME_PRINTTITLES      = '\x07';   // Rows/columns repeated on each page when printing.
const sal_Unicode BIFF_DEFNAME_RECORDER         = '\x08';
const sal_Unicode BIFF_DEFNAME_DATAFORM         = '\x09';
const sal_Unicode BIFF_DEFNAME_AUTOACTIVATE     = '\x0A';   // Sheet macro executed when workbook is activated.
const sal_Unicode BIFF_DEFNAME_AUTODEACTIVATE   = '\x0B';   // Sheet macro executed when workbook is deactivated.
const sal_Unicode BIFF_DEFNAME_SHEETTITLE       = '\x0C';
const sal_Unicode BIFF_DEFNAME_FILTERDATABASE   = '\x0D';   // Sheet range autofilter or advanced filter works on.
const sal_Unicode BIFF_DEFNAME_UNKNOWN          = '\x0E';

// ============================================================================

struct DefinedNameModel
{
    ::rtl::OUString     maName;         /// The original name.
    ::rtl::OUString     maFormula;      /// The formula string.
    sal_Int32           mnSheet;        /// Sheet index for local names.
    sal_Int32           mnFuncGroupId;  /// Function group identifier.
    bool                mbMacro;        /// True = Macro name (VBA or sheet macro).
    bool                mbFunction;     /// True = function, false = command.
    bool                mbVBName;       /// True = VBA macro, false = sheet macro.
    bool                mbHidden;       /// True = name hidden in UI.

    explicit            DefinedNameModel();
};

// ============================================================================

/** Base class for defined names and external names. */
class DefinedNameBase : public WorkbookHelper
{
public:
    explicit            DefinedNameBase( const WorkbookHelper& rHelper );

    /** Returns the original name as imported from or exported to the file. */
    inline const ::rtl::OUString& getModelName() const { return maModel.maName; }
    /** Returns the name as used in the Calc document. */
    inline const ::rtl::OUString& getCalcName() const { return maCalcName; }

    /** Returns the original name as imported from or exported to the file. */
    const ::rtl::OUString& getUpcaseModelName() const;
    /** Returns an Any with a SingleReference or ComplexReference, or an empty Any. */
    ::com::sun::star::uno::Any getReference( const ::com::sun::star::table::CellAddress& rBaseAddr ) const;

protected:
    /** Converts the OOXML formula string stored in the own model. */
    ApiTokenSequence    importOoxFormula( sal_Int16 nBaseSheet );
    /** Imports the BIFF12 formula from the passed stream. */
    ApiTokenSequence    importBiff12Formula( sal_Int16 nBaseSheet, SequenceInputStream& rStrm );
    /** Imports the BIFF formula from the passed stream. */
    ApiTokenSequence    importBiffFormula( sal_Int16 nBaseSheet, BiffInputStream& rStrm, const sal_uInt16* pnFmlaSize = 0 );

protected:
    DefinedNameModel    maModel;            /// Model data for this defined name.
    mutable ::rtl::OUString maUpModelName;  /// Model name converted to uppercase ASCII.
    ::rtl::OUString     maCalcName;         /// Final name used in the Calc document.
    ::com::sun::star::uno::Any maRefAny;    /// Single cell/range reference.
};

// ============================================================================

class DefinedName : public DefinedNameBase
{
public:
    explicit            DefinedName( const WorkbookHelper& rHelper );

    /** Sets the attributes for this defined name from the passed attribute set. */
    void                importDefinedName( const AttributeList& rAttribs );
    /** Sets the formula string from the body of the definedName element. */
    void                setFormula( const ::rtl::OUString& rFormula );
    /** Imports the defined name from a DEFINEDNAME record in the passed stream. */
    void                importDefinedName( SequenceInputStream& rStrm );

    /** Creates a defined name in the Calc document. */
    void                createNameObject( sal_Int32 nIndex );
    /** Converts the formula string or BIFF token array for this defined name. */
    void                convertFormula();
    ApiTokenSequence    getTokens();
    /** Returns true, if this defined name is global in the document. */
    inline bool         isGlobalName() const { return mnCalcSheet < 0; }
    /** Returns true, if this defined name is a special builtin name. */
    inline bool         isBuiltinName() const { return mcBuiltinId != BIFF_DEFNAME_UNKNOWN; }
    /** Returns true, if this defined name is a macro function call. */
    inline bool         isMacroFunction() const { return maModel.mbMacro && maModel.mbFunction; }
    /** Returns true, if this defined name is a reference to a VBA macro. */
    inline bool         isVBName() const { return maModel.mbMacro && maModel.mbVBName; }

    /** Returns the 0-based sheet index for local names, or -1 for global names. */
    inline sal_Int16    getLocalCalcSheet() const { return mnCalcSheet; }
    /** Returns the built-in identifier of the defined name. */
    inline sal_Unicode  getBuiltinId() const { return mcBuiltinId; }
    /** Returns the token index used in API token arrays (com.sun.star.sheet.FormulaToken). */
    inline sal_Int32    getTokenIndex() const { return mnTokenIndex; }
    /** Tries to resolve the defined name to an absolute cell range. */
    bool                getAbsoluteRange( ::com::sun::star::table::CellRangeAddress& orRange ) const;

private:
    /** Imports the OOXML or BIFF12 definition of the name. */
    void                implImportOoxFormula();
    /** Imports the BIFF definition of the name. */
    void                implImportBiffFormula();

private:
    typedef ::std::auto_ptr< StreamDataSequence >   StreamDataSeqPtr;
    typedef ::std::auto_ptr< BiffInputStreamPos >   BiffStreamPosPtr;

    ScRangeData*        mpScRangeData;       /// ScRangeData of the defined name.
    sal_Int32           mnTokenIndex;       /// Name index used in API token array.
    sal_Int16           mnCalcSheet;        /// Calc sheet index for sheet-local names.
    sal_Unicode         mcBuiltinId;        /// Identifier for built-in defined names.
    StreamDataSeqPtr    mxFormula;          /// Formula data for BIFF12 import.
    BiffStreamPosPtr    mxBiffStrm;         /// Cached BIFF stream for formula import.
    sal_uInt16          mnFmlaSize;         /// Cached BIFF formula size for formula import.
};

typedef ::boost::shared_ptr< DefinedName > DefinedNameRef;

// ============================================================================

class DefinedNamesBuffer : public WorkbookHelper
{
public:
    explicit            DefinedNamesBuffer( const WorkbookHelper& rHelper );

    /** Imports a defined name from the passed attribute set. */
    DefinedNameRef      importDefinedName( const AttributeList& rAttribs );
    /** Imports a defined name from a DEFINEDNAME record in the passed stream. */
    void                importDefinedName( SequenceInputStream& rStrm );

    /** Creates all defined names in the document. */
    void                finalizeImport();

    /** Returns a defined name by zero-based index (order of appearance). */
    DefinedNameRef      getByIndex( sal_Int32 nIndex ) const;
    /** Returns a defined name by token index (index in XDefinedNames container). */
    DefinedNameRef      getByTokenIndex( sal_Int32 nIndex ) const;
    /** Returns a defined name by its model name.
        @param nSheet  The sheet index for local names or -1 for global names.
            If no local name is found, tries to find a matching global name.
        @return  Reference to the defined name or empty reference. */
    DefinedNameRef      getByModelName( const ::rtl::OUString& rModelName, sal_Int16 nCalcSheet = -1 ) const;
    /** Returns a built-in defined name by its built-in identifier.
        @param nSheet  The sheet index of the built-in name.
        @return  Reference to the defined name or empty reference. */
    DefinedNameRef      getByBuiltinId( sal_Unicode cBuiltinId, sal_Int16 nCalcSheet ) const;

private:
    DefinedNameRef      createDefinedName();

private:
    typedef ::std::pair< sal_Int16, ::rtl::OUString >   SheetNameKey;
    typedef ::std::pair< sal_Int16, sal_Unicode >       BuiltinKey;

    typedef RefVector< DefinedName >            DefNameVector;
    typedef RefMap< SheetNameKey, DefinedName > DefNameNameMap;
    typedef RefMap< BuiltinKey, DefinedName >   DefNameBuiltinMap;
    typedef RefMap< sal_Int32, DefinedName >    DefNameTokenIdMap;

    DefNameVector       maDefNames;         /// List of all defined names in insertion order.
    DefNameNameMap      maModelNameMap;     /// Maps all defined names by sheet index and model name.
    DefNameBuiltinMap   maBuiltinMap;       /// Maps all defined names by sheet index and built-in identifier.
    DefNameTokenIdMap   maTokenIdMap;       /// Maps all defined names by API token index.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
