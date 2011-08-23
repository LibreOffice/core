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

#include "oox/helper/containerhelper.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/xls/formulabase.hxx"

namespace com { namespace sun { namespace star {
    namespace sheet { class XNamedRange; }
} } }

namespace oox {
namespace xls {

class FormulaContext;
class BiffInputStreamPos;

// ============================================================================

// codes for built-in names
const sal_Unicode OOX_DEFNAME_CONSOLIDATEAREA   = '\x00';
const sal_Unicode OOX_DEFNAME_AUTOOPEN          = '\x01';
const sal_Unicode OOX_DEFNAME_AUTOCLOSE         = '\x02';
const sal_Unicode OOX_DEFNAME_EXTRACT           = '\x03';
const sal_Unicode OOX_DEFNAME_DATABASE          = '\x04';
const sal_Unicode OOX_DEFNAME_CRITERIA          = '\x05';
const sal_Unicode OOX_DEFNAME_PRINTAREA         = '\x06';
const sal_Unicode OOX_DEFNAME_PRINTTITLES       = '\x07';
const sal_Unicode OOX_DEFNAME_RECORDER          = '\x08';
const sal_Unicode OOX_DEFNAME_DATAFORM          = '\x09';
const sal_Unicode OOX_DEFNAME_AUTOACTIVATE      = '\x0A';
const sal_Unicode OOX_DEFNAME_AUTODEACTIVATE    = '\x0B';
const sal_Unicode OOX_DEFNAME_SHEETTITLE        = '\x0C';
const sal_Unicode OOX_DEFNAME_FILTERDATABASE    = '\x0D';
const sal_Unicode OOX_DEFNAME_UNKNOWN           = '\x0E';

// ============================================================================

struct DefinedNameModel
{
    ::rtl::OUString     maName;         /// The original name.
    ::rtl::OUString     maFormula;      /// The formula string.
    sal_Int32           mnSheet;        /// Sheet index for local names.
    sal_Int32           mnFuncGroupId;  /// Function group identifier.
    bool                mbMacro;        /// True = Macro name (VBasic or sheet macro).
    bool                mbFunction;     /// True = function, false = command.
    bool                mbVBName;       /// True = VBasic macro, false = sheet macro.
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
    ::com::sun::star::uno::Any getReference( const ::com::sun::star::table::CellAddress& rBaseAddress ) const;

protected:
    /** Imports the OOX formula string, using the passed formula context. */
    void                importOoxFormula( FormulaContext& rContext, sal_Int16 nBaseSheet );
    /** Imports the OOBIN formula, using the passed formula context. */
    void                importOobFormula( FormulaContext& rContext, sal_Int16 nBaseSheet, RecordInputStream& rStrm );
    /** Imports the BIFF formula, using the passed formula context. */
    void                importBiffFormula( FormulaContext& rContext, sal_Int16 nBaseSheet, BiffInputStream& rStrm, const sal_uInt16* pnFmlaSize = 0 );

    /** Tries to convert the passed token sequence to a SingleReference or ComplexReference. */
    void                extractReference( const ApiTokenSequence& rTokens );

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
    void                importDefinedName( RecordInputStream& rStrm );
    /** Imports the defined name from a DEFINEDNAME record in the passed BIFF stream. */
    void                importDefinedName( BiffInputStream& rStrm, sal_Int16 nCalcSheet );

    /** Creates a defined name in the Calc document. */
    void                createNameObject();
    /** Converts the formula string or BIFF token array for this defined name. */
    void                convertFormula();

    /** Returns true, if this defined name is a special builtin name. */
    inline bool         isBuiltinName() const { return mcBuiltinId != OOX_DEFNAME_UNKNOWN; }
    /** Returns true, if this defined name is a macro function call. */
    inline bool         isMacroFunction() const { return maModel.mbMacro && maModel.mbFunction; }
    /** Returns true, if this defined name is global in the document. */
    inline bool         isGlobalName() const { return mnCalcSheet < 0; }

    /** Returns the token index used in API token arrays (com.sun.star.sheet.FormulaToken). */
    inline sal_Int32    getTokenIndex() const { return mnTokenIndex; }
    /** Returns the 0-based sheet index for local names, or -1 for global names. */
    inline sal_Int16    getLocalCalcSheet() const { return mnCalcSheet; }
    /** Tries to resolve the defined name to an absolute cell range. */
    bool                getAbsoluteRange( ::com::sun::star::table::CellRangeAddress& orRange ) const;

private:
    /** Imports the OOX or OOBIN formula, using the passed formula context. */
    void                implImportOoxFormula( FormulaContext& rContext );
    /** Imports the BIFF formula, using the passed formula context. */
    void                implImportBiffFormula( FormulaContext& rContext );

private:
    typedef ::std::auto_ptr< StreamDataSequence >   StreamDataSeqPtr;
    typedef ::std::auto_ptr< BiffInputStreamPos >   BiffStreamPosPtr;

    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XNamedRange >
                        mxNamedRange;       /// XNamedRange interface of the defined name.
    sal_Int32           mnTokenIndex;       /// Name index used in API token array.
    sal_Int16           mnCalcSheet;        /// Calc sheet index for sheet-local names.
    sal_Unicode         mcBuiltinId;        /// Identifier for built-in defined names.
    StreamDataSeqPtr    mxFormula;          /// Formula data for OOBIN import.
    BiffStreamPosPtr    mxBiffStrm;         /// Cached BIFF stream for formula import.
    sal_uInt16          mnFmlaSize;         /// Cached BIFF formula size for formula import.
};

typedef ::boost::shared_ptr< DefinedName > DefinedNameRef;

// ============================================================================

class DefinedNamesBuffer : public WorkbookHelper
{
public:
    explicit            DefinedNamesBuffer( const WorkbookHelper& rHelper );

    /** Sets the sheet index for local names (BIFF2-BIFF4 only). */
    void                setLocalCalcSheet( sal_Int16 nCalcSheet );

    /** Imports a defined name from the passed attribute set. */
    DefinedNameRef      importDefinedName( const AttributeList& rAttribs );
    /** Imports a defined name from a DEFINEDNAME record in the passed stream. */
    void                importDefinedName( RecordInputStream& rStrm );
    /** Imports a defined name from a DEFINEDNAME record in the passed BIFF stream. */
    void                importDefinedName( BiffInputStream& rStrm );

    /** Creates all defined names in the document. */
    void                finalizeImport();

    /** Returns a defined name by zero-based index (order of appearence). */
    DefinedNameRef      getByIndex( sal_Int32 nIndex ) const;
    /** Returns a defined name by token index (index in XDefinedNames container). */
    DefinedNameRef      getByTokenIndex( sal_Int32 nIndex ) const;
    /** Returns a defined name by its model name.
        @param nSheet  The sheet index for local names or -1 for global names.
            If no local name is found, tries to find a matching global name.
        @return  Reference to the defined name or empty reference. */
    DefinedNameRef      getByModelName( const ::rtl::OUString& rModelName, sal_Int16 nCalcSheet = -1 ) const;

private:
    DefinedNameRef      createDefinedName();

private:
    typedef RefVector< DefinedName >            DefNameVector;
    typedef RefMap< sal_Int32, DefinedName >    DefNameMap;

    DefNameVector       maDefNames;         /// List of all defined names in insertion order.
    DefNameMap          maDefNameMap;       /// Maps all defined names by API token index.
    sal_Int16           mnCalcSheet;        /// Current sheet index for BIFF2-BIFF4 names (always sheet-local).
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

