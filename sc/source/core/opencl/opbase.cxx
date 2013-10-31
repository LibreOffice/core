/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "opbase.hxx"

#include "formula/vectortoken.hxx"

using namespace formula;

namespace sc { namespace opencl {

DynamicKernelArgument::DynamicKernelArgument(const std::string &s,
   FormulaTreeNodeRef ft):
    mSymName(s), mFormulaTree(ft), mpClmem(NULL) {}

/// Generate declaration
void DynamicKernelArgument::GenDecl(std::stringstream &ss) const
{
    ss << "__global double *"<<mSymName;
}

/// When declared as input to a sliding window function
void DynamicKernelArgument::GenSlidingWindowDecl(std::stringstream &ss) const
{
    DynamicKernelArgument::GenDecl(ss);
}

/// When referenced in a sliding window function
std::string DynamicKernelArgument::GenSlidingWindowDeclRef(bool) const
{
    std::stringstream ss;
    ss << mSymName << "[gid0]";
    return ss.str();
}

/// Generate use/references to the argument
void DynamicKernelArgument::GenDeclRef(std::stringstream &ss) const
{
    ss << mSymName;
}

DynamicKernelArgument::~DynamicKernelArgument()
{
    //std::cerr << "~DynamicKernelArgument: " << mSymName <<"\n";
    if (mpClmem) {
        //std::cerr << "\tFreeing cl_mem of " << mSymName <<"\n";
        cl_int ret = clReleaseMemObject(mpClmem);
        if (ret != CL_SUCCESS)
            throw OpenCLError(ret);
    }
}

FormulaToken* DynamicKernelArgument::GetFormulaToken(void) const
{
    return mFormulaTree->GetFormulaToken();
}

size_t DynamicKernelArgument::GetWindowSize(void) const
{
    FormulaToken *pCur = mFormulaTree->GetFormulaToken();
    assert(pCur);
    if (const formula::DoubleVectorRefToken* pCurDVR =
        dynamic_cast<const formula::DoubleVectorRefToken *>(pCur))
    {
        return pCurDVR->GetRefRowSize();
    }
    else if (dynamic_cast<const formula::SingleVectorRefToken *>(pCur))
    {
        // Prepare intermediate results (on CPU for now)
        return 1;
    }
    else
    {
        throw Unhandled();
    }
}

void Normal::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string sSymName, SubArguments &vSubArguments)
{
    ArgVector argVector;
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
        argVector.push_back(vSubArguments[i]->GenSlidingWindowDeclRef());
    }
    ss << ") {\n\t";
    ss << "double tmp = " << GetBottom() <<";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "tmp = ";
    ss << Gen(argVector);
    ss << ";\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
