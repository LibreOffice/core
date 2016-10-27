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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_DEFNAMESBUFFER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_DEFNAMESBUFFER_HXX

#include "formulabase.hxx"
#include "rangenam.hxx"

#include <memory>

class ScTokenArray;

namespace com { namespace sun { namespace star {
    namespace sheet { class XNamedRange; }
} } }

namespace oox {
namespace xls {

class BiffInputStreamPos;

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

struct DefinedNameModel
{
    OUString     maName;         /// The original name.
    OUString     maFormula;      /// The formula string.
    sal_Int32           mnSheet;        /// Sheet index for local names.
    sal_Int32           mnFuncGroupId;  /// Function group identifier.
    bool                mbMacro;        /// True = Macro name (VBA or sheet macro).
    bool                mbFunction;     /// True = function, false = command.
    bool                mbVBName;       /// True = VBA macro, false = sheet macro.
    bool                mbHidden;       /// True = name hidden in UI.

    explicit            DefinedNameModel();
};

/** Base class for defined names and external names. */
class DefinedNameBase : public WorkbookHelper
{
public:
    explicit            DefinedNameBase( const WorkbookHelper& rHelper );

    /** Returns the original name as imported from or exported to the file. */
    inline const OUString& getModelName() const { return maModel.maName; }
    /** Returns the name as used in the Calc document. */
    inline const OUString& getCalcName() const { return maCalcName; }

    /** Returns the original name as imported from or exported to the file. */
    const OUString& getUpcaseModelName() const;
    /** Returns an Any with a SingleReference or ComplexReference, or an empty Any. */
    css::uno::Any getReference( const css::table::CellAddress& rBaseAddr ) const;

protected:
    DefinedNameModel    maModel;        /// Model data for this defined name.
    mutable OUString    maUpModelName;  /// Model name converted to uppercase ASCII.
    OUString            maCalcName;     /// Final name used in the Calc document.
    css::uno::Any       maRefAny;       /// Single cell/range reference.
};

class DefinedName : public DefinedNameBase
{
public:
    explicit            DefinedName( const WorkbookHelper& rHelper );

    /** Sets the attributes for this defined name from the passed attribute set. */
    void                importDefinedName( const AttributeList& rAttribs );
    /** Sets the formula string from the body of the definedName element. */
    void                setFormula( const OUString& rFormula );
    /** Imports the defined name from a DEFINEDNAME record in the passed stream. */
    void                importDefinedName( SequenceInputStream& rStrm );

    /** Creates a defined name in the Calc document. */
    void                createNameObject( sal_Int32 nIndex );
    /** Converts the formula string or BIFF token array for this defined name. */
    void                convertFormula( const css::uno::Sequence<css::sheet::ExternalLinkInfo>& rExternalLinks );
    std::unique_ptr<ScTokenArray> getScTokens( const css::uno::Sequence<css::sheet::ExternalLinkInfo>& rExternalLinks );
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
    bool                getAbsoluteRange( css::table::CellRangeAddress& orRange ) const;

private:
    typedef ::std::unique_ptr< StreamDataSequence >   StreamDataSeqPtr;
    typedef ::std::unique_ptr< BiffInputStreamPos >   BiffStreamPosPtr;

    ScRangeData*        mpScRangeData;       /// ScRangeData of the defined name.
    sal_Int32           mnTokenIndex;       /// Name index used in API token array.
    sal_Int16           mnCalcSheet;        /// Calc sheet index for sheet-local names.
    sal_Unicode         mcBuiltinId;        /// Identifier for built-in defined names.
    StreamDataSeqPtr    mxFormula;          /// Formula data for BIFF12 import.
    BiffStreamPosPtr    mxBiffStrm;         /// Cached BIFF stream for formula import.
};

typedef std::shared_ptr< DefinedName > DefinedNameRef;

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
    DefinedNameRef      getByModelName( const OUString& rModelName, sal_Int16 nCalcSheet = -1 ) const;
    /** Returns a built-in defined name by its built-in identifier.
        @param nSheet  The sheet index of the built-in name.
        @return  Reference to the defined name or empty reference. */
    DefinedNameRef      getByBuiltinId( sal_Unicode cBuiltinId, sal_Int16 nCalcSheet ) const;

private:
    DefinedNameRef      createDefinedName();

private:
    typedef ::std::pair< sal_Int16, OUString >   SheetNameKey;
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

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
