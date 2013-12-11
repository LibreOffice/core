/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_OPENCL_OP_DATABASE_HXX
#define SC_OPENCL_OP_DATABASE_HXX

#include "opbase.hxx"


namespace sc { namespace opencl {

class OpDmax: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
             const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Dmax"; }
};

class OpDmin: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
             const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Dmin"; }
};

class OpDproduct: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
             const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Dproduct"; }
};

class OpDaverage: public CheckVariables
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
             const std::string sSymName, SubArguments &vSubArguments);
    virtual std::string BinFuncName(void) const { return "Daverage"; }
};




}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
