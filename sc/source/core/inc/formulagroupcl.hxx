/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_CORE_INC_FORMULAGROUPCL_HXX
#define INCLUDED_SC_SOURCE_CORE_INC_FORMULAGROUPCL_HXX

#include "formulagroup.hxx"

namespace sc { namespace opencl {

class FormulaGroupInterpreterOpenCL : public FormulaGroupInterpreter
{
public:
    FormulaGroupInterpreterOpenCL();
    virtual ~FormulaGroupInterpreterOpenCL();

    virtual ScMatrixRef inverseMatrix( const ScMatrix& rMat ) override;
    virtual bool interpret( ScDocument& rDoc, const ScAddress& rTopPos,
        ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode ) override;
};

}} // namespace sc::opencl

#endif // INCLUDED_SC_SOURCE_CORE_INC_FORMULAGROUPCL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
