/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "opbase.hxx"
#include "utils.hxx"

namespace sc::opencl {

/// Implements OpAnd, OpOr, OpXor.
class OpLogicalBinaryOperator : public Normal
{
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual bool canHandleMultiVector() const override { return true; }
    /// The C operator implementing the function.
    virtual const char* openclOperator() const = 0;
    /// Default value when chaining the operator.
    virtual const char* defaultOpenclValue() const = 0;
};

class OpAnd: public OpLogicalBinaryOperator
{
public:
    virtual std::string BinFuncName() const override { return "And"; }
    virtual const char* openclOperator() const override { return "&&"; };
    virtual const char* defaultOpenclValue() const override { return "true"; }
};

class OpOr: public OpLogicalBinaryOperator
{
public:
    virtual std::string BinFuncName() const override { return "Or"; }
    virtual const char* openclOperator() const override { return "||"; };
    virtual const char* defaultOpenclValue() const override { return "false"; }
};

class OpNot: public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Not"; }
};

class OpXor: public OpLogicalBinaryOperator
{
public:
    virtual std::string BinFuncName() const override { return "Xor"; }
    virtual const char* openclOperator() const override { return "^"; };
    virtual const char* defaultOpenclValue() const override { return "false"; }
};

class OpIf:public Normal
{
public:
    virtual void GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "IF"; }
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
