/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: condformatbuffer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef OOX_XLS_CONDFORMATBUFFER_HXX
#define OOX_XLS_CONDFORMATBUFFER_HXX

#include "oox/helper/containerhelper.hxx"
#include "oox/xls/formulaparser.hxx"
#include "oox/xls/worksheethelper.hxx"

namespace com { namespace sun { namespace star {
    namespace sheet { class XSheetConditionalEntries; }
} } }

namespace oox {
namespace xls {

// ============================================================================

/** Cointains OOX data for a single rule in a conditional formatting. */
struct OoxCondFormatRuleData
{
    typedef ::std::vector< TokensFormulaContext > ContextVector;

    ContextVector       maFormulas;         /// Formulas for rule conditions.
    ::rtl::OUString     maText;             /// Text for 'contains' rules.
    sal_Int32           mnPriority;         /// Priority of this rule.
    sal_Int32           mnType;             /// Type of the rule.
    sal_Int32           mnOperator;         /// In cell-is rules: Comparison operator.
    sal_Int32           mnTimePeriod;       /// In time-period rules: Type of time period.
    sal_Int32           mnRank;             /// In top-10 rules: True = bottom, false = top.
    sal_Int32           mnStdDev;           /// In average rules: Number of std deviations.
    sal_Int32           mnDxfId;            /// Differential formatting identifier.
    bool                mbStopIfTrue;       /// True = stop evaluating rules, if this rule is true.
    bool                mbBottom;           /// In top-10 rules: True = bottom, false = top.
    bool                mbPercent;          /// In top-10 rules: True = percent, false = rank.
    bool                mbAboveAverage;     /// In average rules: True = above average, false = below.
    bool                mbEqualAverage;     /// In average rules: True = include average, false = exclude.

    explicit            OoxCondFormatRuleData();

    /** Sets the passed OOBIN or BIFF operator for condition type cellIs. */
    void                setBinOperator( sal_Int32 nOperator );

    /** Sets the passed OOBIN text comparison type and operator. */
    void                setOobTextType( sal_Int32 nOperator );
};

// ============================================================================

class CondFormat;

/** Represents a single rule in a conditional formatting. */
class CondFormatRule : public WorksheetHelper
{
public:
    explicit            CondFormatRule( const CondFormat& rCondFormat );

    /** Imports rule settings from the cfRule element. */
    void                importCfRule( const AttributeList& rAttribs );
    /** Appends a new condition formula string. */
    void                appendFormula( const ::rtl::OUString& rFormula );

    /** Imports rule settings from a CFRULE record. */
    void                importCfRule( RecordInputStream& rStrm );

    /** Imports rule settings from a CFRULE record. */
    void                importCfRule( BiffInputStream& rStrm, sal_Int32 nPriority );

    /** Creates a conditional formatting rule in the Calc document. */
    void                finalizeImport(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetConditionalEntries >& rxEntries );

    /** Returns the priority of this rule. */
    inline sal_Int32    getPriority() const { return maOoxData.mnPriority; }

private:
    const CondFormat&   mrCondFormat;
    OoxCondFormatRuleData maOoxData;
};

typedef ::boost::shared_ptr< CondFormatRule > CondFormatRuleRef;

// ============================================================================

/** Cointains OOX data for a conditional formatting. */
struct OoxCondFormatData
{
    ApiCellRangeList    maRanges;           /// Cell ranges for this conditional format.
    bool                mbPivot;            /// Conditional formatting belongs to pivot table.

    explicit            OoxCondFormatData();
};

// ============================================================================

/** Represents a conditional formatting object with a list of affected cell ranges. */
class CondFormat : public WorksheetHelper
{
public:
    explicit            CondFormat( const WorksheetHelper& rHelper );

    /** Imports settings from the conditionalFormatting element. */
    void                importConditionalFormatting( const AttributeList& rAttribs );
    /** Imports a conditional formatting rule from the cfRule element. */
    CondFormatRuleRef   importCfRule( const AttributeList& rAttribs );

    /** Imports settings from the CONDFORMATTING record. */
    void                importCondFormatting( RecordInputStream& rStrm );
    /** Imports a conditional formatting rule from the CFRULE record. */
    void                importCfRule( RecordInputStream& rStrm );

    /** Imports settings from the CFHEADER record. */
    void                importCfHeader( BiffInputStream& rStrm );

    /** Creates the conditional formatting in the Calc document. */
    void                finalizeImport();

    /** Returns the cell ranges this conditional formatting belongs to. */
    inline const ApiCellRangeList& getRanges() const { return maOoxData.maRanges; }

private:
    CondFormatRuleRef   createRule();
    void                insertRule( CondFormatRuleRef xRule );

private:
    typedef RefMap< sal_Int32, CondFormatRule > CondFormatRuleMap;

    OoxCondFormatData   maOoxData;          /// Data of this conditional formatting.
    CondFormatRuleMap   maRules;            /// Maps formatting rules by priority.
};

typedef ::boost::shared_ptr< CondFormat > CondFormatRef;

// ============================================================================

class CondFormatBuffer : public WorksheetHelper
{
public:
    explicit            CondFormatBuffer( const WorksheetHelper& rHelper );

    /** Imports settings from the conditionalFormatting element. */
    CondFormatRef       importConditionalFormatting( const AttributeList& rAttribs );
    /** Imports settings from the CONDFORMATTING record. */
    CondFormatRef       importCondFormatting( RecordInputStream& rStrm );
    /** Imports settings from the CFHEADER record. */
    void                importCfHeader( BiffInputStream& rStrm );

    /** Creates all conditional formattings in the Calc document. */
    void                finalizeImport();

private:
    CondFormatRef       createCondFormat();

private:
    typedef RefVector< CondFormat > CondFormatVec;
    CondFormatVec       maCondFormats;      /// All conditional formattings in a sheet.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

