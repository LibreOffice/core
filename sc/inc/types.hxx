/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_TYPES_HXX
#define SC_TYPES_HXX

#include "sal/types.h"

#include <boost/intrusive_ptr.hpp>

class ScMatrix;

typedef ::boost::intrusive_ptr<ScMatrix>        ScMatrixRef;
typedef ::boost::intrusive_ptr<const ScMatrix>  ScConstMatrixRef;

class ScToken;
typedef ::boost::intrusive_ptr<ScToken> ScTokenRef;

typedef sal_uInt8 ScMatValType;
const ScMatValType SC_MATVAL_VALUE     = 0x00;
const ScMatValType SC_MATVAL_BOOLEAN   = 0x01;
const ScMatValType SC_MATVAL_STRING    = 0x02;
const ScMatValType SC_MATVAL_EMPTY     = SC_MATVAL_STRING | 0x04; // STRING plus flag
const ScMatValType SC_MATVAL_EMPTYPATH = SC_MATVAL_EMPTY | 0x08;  // EMPTY plus flag
const ScMatValType SC_MATVAL_NONVALUE  = SC_MATVAL_EMPTYPATH;     // mask of all non-value bits

struct ScFormulaCellGroup;
typedef ::boost::intrusive_ptr<ScFormulaCellGroup> ScFormulaCellGroupRef;

/**
 * When vectorization is enabled, we could potentially mass-calculate a
 * series of formula token arrays in adjacent formula cells in one step,
 * provided that they all contain identical set of tokens.
 */
enum ScFormulaVectorState
{
    FormulaVectorDisabled = 0,
    FormulaVectorEnabled,
    FormulaVectorCheckReference,
    FormulaVectorUnknown
};

namespace sc {

const sal_uInt16 MatrixEdgeNothing = 0;
const sal_uInt16 MatrixEdgeInside  = 1;
const sal_uInt16 MatrixEdgeBottom  = 2;
const sal_uInt16 MatrixEdgeLeft    = 4;
const sal_uInt16 MatrixEdgeTop     = 8;
const sal_uInt16 MatrixEdgeRight   = 16;
const sal_uInt16 MatrixEdgeOpen    = 32;

enum GroupCalcState
{
    GroupCalcEnabled,
    GroupCalcRunning,
    GroupCalcDisabled
};

struct RangeMatrix
{
    ScMatrixRef mpMat;
    sal_Int32 mnCol1;
    sal_Int32 mnRow1;
    sal_Int32 mnCol2;
    sal_Int32 mnRow2;

    RangeMatrix();

    bool isRangeValid() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
