/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_CORE_OPENCL_OP_ARRAY_HXX
#define INCLUDED_SC_SOURCE_CORE_OPENCL_OP_ARRAY_HXX

#include "opbase.hxx"

namespace sc { namespace opencl {

class OpSumX2MY2: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "SumX2MY2"; }
};

class OpSumX2PY2: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "SumX2PY2"; }
};

class OpSumXMY2: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "SumXMY2"; }
};
}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
