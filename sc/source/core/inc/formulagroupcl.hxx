/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <formulagroup.hxx>

namespace sc::opencl {

class FormulaGroupInterpreterOpenCL final : public FormulaGroupInterpreter
{
public:
    FormulaGroupInterpreterOpenCL();
    virtual ~FormulaGroupInterpreterOpenCL() override;

    virtual ScMatrixRef inverseMatrix( const ScMatrix& rMat ) override;
    virtual bool interpret( ScDocument& rDoc, const ScAddress& rTopPos,
        ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode ) override;
};

} // namespace sc::opencl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
