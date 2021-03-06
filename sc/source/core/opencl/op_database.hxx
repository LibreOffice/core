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

namespace sc::opencl {

class OpDmax: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Dmax"; }
};

class OpDmin: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Dmin"; }
};

class OpDproduct: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Dproduct"; }
};

class OpDaverage: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Daverage"; }
};

class OpDstdev: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Dstdev"; }
};

class OpDstdevp: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Dstdevp"; }
};

class OpDsum: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Dsum"; }
};

class OpDvar: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Dvar"; }
};

class OpDvarp: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Dvarp"; }
};

class OpDcount: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Dcount"; }
};

class OpDcount2: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments) override;
    virtual std::string BinFuncName() const override { return "Dcount2"; }
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
