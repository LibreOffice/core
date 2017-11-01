/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_TYPES_HXX
#define INCLUDED_SC_INC_TYPES_HXX

#include <sal/types.h>

#include <boost/intrusive_ptr.hpp>
#include <o3tl/typed_flags_set.hxx>

class ScMatrix;

// The typedefs
typedef sal_Int32 SCROW;
typedef sal_Int16 SCCOL;
typedef sal_Int16 SCTAB;
typedef sal_Int32 SCCOLROW;     ///< a type capable of holding either SCCOL or SCROW

typedef ::boost::intrusive_ptr<ScMatrix>        ScMatrixRef;
typedef ::boost::intrusive_ptr<const ScMatrix>  ScConstMatrixRef;

namespace formula { class FormulaToken; }
typedef ::boost::intrusive_ptr<formula::FormulaToken> ScTokenRef;

enum class ScMatValType : sal_uInt8 {
    Value        = 0x00,
    Boolean      = 0x01,
    String       = 0x02,
    Empty        = String | 0x04, // STRING plus flag
    EmptyPath    = Empty | 0x08,  // EMPTY plus flag
    NonvalueMask = EmptyPath      // mask of all non-value bits
};
namespace o3tl{
    template<> struct typed_flags<ScMatValType> : o3tl::is_typed_flags<ScMatValType, 0x0f> {};
}

struct ScFormulaCellGroup;
typedef ::boost::intrusive_ptr<ScFormulaCellGroup> ScFormulaCellGroupRef;

/**
 * When vectorization is enabled, we could potentially mass-calculate a
 * series of formula token arrays in adjacent formula cells in one step,
 * provided that they all contain identical set of tokens.
 */
enum ScFormulaVectorState
{
    FormulaVectorDisabled,
    FormulaVectorDisabledNotInSubSet,
    FormulaVectorDisabledNotInSoftwareSubset,
    FormulaVectorDisabledByOpCode,
    FormulaVectorDisabledByStackVariable,

    FormulaVectorEnabled,
    FormulaVectorCheckReference,
    FormulaVectorEnabledForThreading,
    FormulaVectorUnknown
};

namespace sc {

enum class MatrixEdge{
    Nothing = 0,
    Inside  = 1,
    Bottom  = 2,
    Left    = 4,
    Top     = 8,
    Right   = 16,
    Open    = 32
}; // typed_flags, template outside of sc namespace

enum GroupCalcState
{
    GroupCalcDisabled = 0,
    GroupCalcEnabled,
    GroupCalcRunning,
};

struct RangeMatrix
{
    ScMatrixRef mpMat;
    sal_Int32 mnCol1;
    sal_Int32 mnRow1;
    sal_Int32 mnTab1;
    sal_Int32 mnCol2;
    sal_Int32 mnRow2;
    sal_Int32 mnTab2;

    RangeMatrix();

    bool isRangeValid() const;
};

struct MultiDataCellState
{
    enum StateType { Invalid = 0, Empty, HasOneCell, HasMultipleCells };

    StateType meState;

    SCCOL mnCol1; //< first non-empty column
    SCROW mnRow1; //< first non-empty row

    MultiDataCellState();
    MultiDataCellState( StateType eState );
};

enum AreaOverlapType
{
    AreaInside,
    AreaPartialOverlap,
    AreaInsideOrOverlap,
    OneRowInsideArea,
    OneColumnInsideArea
};

enum class ListenerGroupType
{
    Single,
    Group,
    Both
};

enum StartListeningType
{
    ConvertToGroupListening,
    SingleCellListening,
    NoListening
};

}

namespace o3tl{
    template<> struct typed_flags<sc::MatrixEdge> : o3tl::is_typed_flags<sc::MatrixEdge, 63> {};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
