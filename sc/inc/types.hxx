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

#include <boost/intrusive_ptr.hpp>

class ScMatrix;

typedef ::boost::intrusive_ptr<ScMatrix>        ScMatrixRef;
typedef ::boost::intrusive_ptr<const ScMatrix>  ScConstMatrixRef;

class ScToken;
typedef ::boost::intrusive_ptr<ScToken> ScTokenRef;

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

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
