/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "opbase.hxx"

using namespace formula;

namespace sc { namespace opencl {

UnhandledToken::UnhandledToken(
    formula::FormulaToken* t, const char* m, const std::string& fn, int ln ) :
    mToken(t), mMessage(m), mFile(fn), mLineNumber(ln) {}

OpenCLError::OpenCLError( cl_int err, const std::string& fn, int ln ) :
    mError(err), mFile(fn), mLineNumber(ln)
{
    SAL_INFO("sc.opencl", "OpenCLError:" << mError << ": " << strerror(mError));
}

const char* OpenCLError::strerror( cl_int i ) const
{
#define CASE(val) case val: return #val
    switch (i)
    {
        CASE(CL_SUCCESS);
        CASE(CL_DEVICE_NOT_FOUND);
        CASE(CL_DEVICE_NOT_AVAILABLE);
        CASE(CL_COMPILER_NOT_AVAILABLE);
        CASE(CL_MEM_OBJECT_ALLOCATION_FAILURE);
        CASE(CL_OUT_OF_RESOURCES);
        CASE(CL_OUT_OF_HOST_MEMORY);
        CASE(CL_PROFILING_INFO_NOT_AVAILABLE);
        CASE(CL_MEM_COPY_OVERLAP);
        CASE(CL_IMAGE_FORMAT_MISMATCH);
        CASE(CL_IMAGE_FORMAT_NOT_SUPPORTED);
        CASE(CL_BUILD_PROGRAM_FAILURE);
        CASE(CL_MAP_FAILURE);
        CASE(CL_INVALID_VALUE);
        CASE(CL_INVALID_DEVICE_TYPE);
        CASE(CL_INVALID_PLATFORM);
        CASE(CL_INVALID_DEVICE);
        CASE(CL_INVALID_CONTEXT);
        CASE(CL_INVALID_QUEUE_PROPERTIES);
        CASE(CL_INVALID_COMMAND_QUEUE);
        CASE(CL_INVALID_HOST_PTR);
        CASE(CL_INVALID_MEM_OBJECT);
        CASE(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
        CASE(CL_INVALID_IMAGE_SIZE);
        CASE(CL_INVALID_SAMPLER);
        CASE(CL_INVALID_BINARY);
        CASE(CL_INVALID_BUILD_OPTIONS);
        CASE(CL_INVALID_PROGRAM);
        CASE(CL_INVALID_PROGRAM_EXECUTABLE);
        CASE(CL_INVALID_KERNEL_NAME);
        CASE(CL_INVALID_KERNEL_DEFINITION);
        CASE(CL_INVALID_KERNEL);
        CASE(CL_INVALID_ARG_INDEX);
        CASE(CL_INVALID_ARG_VALUE);
        CASE(CL_INVALID_ARG_SIZE);
        CASE(CL_INVALID_KERNEL_ARGS);
        CASE(CL_INVALID_WORK_DIMENSION);
        CASE(CL_INVALID_WORK_GROUP_SIZE);
        CASE(CL_INVALID_WORK_ITEM_SIZE);
        CASE(CL_INVALID_GLOBAL_OFFSET);
        CASE(CL_INVALID_EVENT_WAIT_LIST);
        CASE(CL_INVALID_EVENT);
        CASE(CL_INVALID_OPERATION);
        CASE(CL_INVALID_GL_OBJECT);
        CASE(CL_INVALID_BUFFER_SIZE);
        CASE(CL_INVALID_MIP_LEVEL);
        CASE(CL_INVALID_GLOBAL_WORK_SIZE);
        default:
            return "Unknown OpenCL error code";
    }
#undef CASE
}

Unhandled::Unhandled( const std::string& fn, int ln ) :
    mFile(fn), mLineNumber(ln) {}

DynamicKernelArgument::DynamicKernelArgument( const std::string& s,
    FormulaTreeNodeRef ft ) :
    mSymName(s), mFormulaTree(ft) { }

std::string DynamicKernelArgument::GenDoubleSlidingWindowDeclRef( bool ) const
{
    return std::string("");
}

/// When Mix, it will be called
std::string DynamicKernelArgument::GenStringSlidingWindowDeclRef( bool ) const
{
    return std::string("");
}

bool DynamicKernelArgument::IsMixedArgument() const
{
    return false;
}

/// Generate use/references to the argument
void DynamicKernelArgument::GenDeclRef( std::stringstream& ss ) const
{
    ss << mSymName;
}

void DynamicKernelArgument::GenNumDeclRef( std::stringstream& ss ) const
{
    ss << ",";
}

void DynamicKernelArgument::GenStringDeclRef( std::stringstream& ss ) const
{
    ss << ",";
}

void DynamicKernelArgument::GenSlidingWindowFunction( std::stringstream& ) {}

FormulaToken* DynamicKernelArgument::GetFormulaToken() const
{
    return mFormulaTree->GetFormulaToken();
}

std::string DynamicKernelArgument::DumpOpName() const
{
    return std::string("");
}

void DynamicKernelArgument::DumpInlineFun( std::set<std::string>&, std::set<std::string>& ) const {}

const std::string& DynamicKernelArgument::GetName() const
{
    return mSymName;
}

bool DynamicKernelArgument::NeedParallelReduction() const
{
    return false;
}

VectorRef::VectorRef( const std::string& s, FormulaTreeNodeRef ft, int idx ) :
    DynamicKernelArgument(s, ft), mpClmem(NULL), mnIndex(idx)
{
    if (mnIndex)
    {
        std::stringstream ss;
        ss << mSymName << "s" << mnIndex;
        mSymName = ss.str();
    }
}

VectorRef::~VectorRef()
{
    if (mpClmem)
    {
        clReleaseMemObject(mpClmem);
    }
}

/// Generate declaration
void VectorRef::GenDecl( std::stringstream& ss ) const
{
    ss << "__global double *" << mSymName;
}

/// When declared as input to a sliding window function
void VectorRef::GenSlidingWindowDecl( std::stringstream& ss ) const
{
    VectorRef::GenDecl(ss);
}

/// When referenced in a sliding window function
std::string VectorRef::GenSlidingWindowDeclRef( bool nested ) const
{
    std::stringstream ss;
    formula::SingleVectorRefToken* pSVR =
        dynamic_cast<formula::SingleVectorRefToken*>(DynamicKernelArgument::GetFormulaToken());
    if (pSVR && !nested)
        ss << "(gid0 < " << pSVR->GetArrayLength() << "?";
    ss << mSymName << "[gid0]";
    if (pSVR && !nested)
        ss << ":NAN)";
    return ss.str();
}

void VectorRef::GenSlidingWindowFunction( std::stringstream& ) {}

size_t VectorRef::GetWindowSize() const
{
    FormulaToken* pCur = mFormulaTree->GetFormulaToken();
    assert(pCur);
    if (const formula::DoubleVectorRefToken* pCurDVR =
        dynamic_cast<const formula::DoubleVectorRefToken*>(pCur))
    {
        return pCurDVR->GetRefRowSize();
    }
    else if (dynamic_cast<const formula::SingleVectorRefToken*>(pCur))
    {
        // Prepare intermediate results (on CPU for now)
        return 1;
    }
    else
    {
        throw Unhandled();
    }
}

std::string VectorRef::DumpOpName() const
{
    return std::string("");
}

void VectorRef::DumpInlineFun( std::set<std::string>&, std::set<std::string>& ) const {}

const std::string& VectorRef::GetName() const
{
    return mSymName;
}

cl_mem VectorRef::GetCLBuffer() const
{
    return mpClmem;
}

bool VectorRef::NeedParallelReduction() const
{
    return false;
}

void Normal::GenSlidingWindowFunction(
    std::stringstream& ss, const std::string& sSymName, SubArguments& vSubArguments )
{
    ArgVector argVector;
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
        argVector.push_back(vSubArguments[i]->GenSlidingWindowDeclRef());
    }
    ss << ") {\n\t";
    ss << "double tmp = " << GetBottom() << ";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "tmp = ";
    ss << Gen(argVector);
    ss << ";\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

void CheckVariables::GenTmpVariables(
    std::stringstream& ss, SubArguments& vSubArguments )
{
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        ss << "    double tmp";
        ss << i;
        ss << ";\n";
    }
}

void CheckVariables::CheckSubArgumentIsNan( std::stringstream& ss,
    SubArguments& vSubArguments,  int argumentNum )
{
    int i = argumentNum;
    if (vSubArguments[i]->GetFormulaToken()->GetType() ==
            formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken* pTmpDVR1 =
            static_cast<const formula::SingleVectorRefToken*>(vSubArguments[i]->GetFormulaToken());
        ss << "    if(singleIndex>=";
        ss << pTmpDVR1->GetArrayLength();
        ss << " ||";
        ss << "isNan(";
        ss << vSubArguments[i]->GenSlidingWindowDeclRef(true);
        ss << "))\n";
        ss << "        tmp";
        ss << i;
        ss << "=0;\n    else \n";
        ss << "        tmp";
        ss << i;
        ss << "=";
        ss << vSubArguments[i]->GenSlidingWindowDeclRef(true);
        ss << ";\n";
    }
    if (vSubArguments[i]->GetFormulaToken()->GetType() ==
            formula::svDoubleVectorRef)
    {
        const formula::DoubleVectorRefToken* pTmpDVR2 =
            static_cast<const formula::DoubleVectorRefToken*>(vSubArguments[i]->GetFormulaToken());
        ss << "    if(doubleIndex>=";
        ss << pTmpDVR2->GetArrayLength();
        ss << " ||";
        ss << "isNan(";
        ss << vSubArguments[i]->GenSlidingWindowDeclRef(false);
        ss << "))\n";
        ss << "        tmp";
        ss << i;
        ss << "=0;\n    else \n";
        ss << "        tmp";
        ss << i;
        ss << "=";
        ss << vSubArguments[i]->GenSlidingWindowDeclRef(false);
        ss << ";\n";
    }
    if (vSubArguments[i]->GetFormulaToken()->GetType() == formula::svDouble ||
        vSubArguments[i]->GetFormulaToken()->GetOpCode() != ocPush)
    {
        ss << "    if(";
        ss << "isNan(";
        ss << vSubArguments[i]->GenSlidingWindowDeclRef();
        ss << "))\n";
        ss << "        tmp";
        ss << i;
        ss << "=0;\n    else \n";
        ss << "        tmp";
        ss << i;
        ss << "=";
        ss << vSubArguments[i]->GenSlidingWindowDeclRef();
        ss << ";\n";

    }

}

void CheckVariables::CheckSubArgumentIsNan2( std::stringstream& ss,
    SubArguments& vSubArguments,  int argumentNum, std::string p )
{
    int i = argumentNum;
    if (vSubArguments[i]->GetFormulaToken()->GetType() == formula::svDouble)
    {
        ss << "    tmp";
        ss << i;
        ss << "=";
        vSubArguments[i]->GenDeclRef(ss);
        ss << ";\n";
        return;
    }

    ss << "    tmp";
    ss << i;
    ss << "= fsum(";
    vSubArguments[i]->GenDeclRef(ss);
    if (vSubArguments[i]->GetFormulaToken()->GetType() ==
            formula::svDoubleVectorRef)
        ss << "[" << p.c_str() << "]";
    else  if (vSubArguments[i]->GetFormulaToken()->GetType() ==
            formula::svSingleVectorRef)
        ss << "[get_group_id(1)]";
    ss << ", 0);\n";
}

void CheckVariables::CheckAllSubArgumentIsNan(
    std::stringstream& ss, SubArguments& vSubArguments )
{
    ss << "    int k = gid0;\n";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        CheckSubArgumentIsNan(ss, vSubArguments, i);
    }
}

void CheckVariables::UnrollDoubleVector( std::stringstream& ss,
    std::stringstream& unrollstr, const formula::DoubleVectorRefToken* pCurDVR,
    int nCurWindowSize )
{
    int unrollSize = 16;
    if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed())
    {
        ss << "    loop = (" << nCurWindowSize << " - gid0)/";
        ss << unrollSize << ";\n";
    }
    else if (pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
    {
        ss << "    loop = (" << nCurWindowSize << " + gid0)/";
        ss << unrollSize << ";\n";

    }
    else
    {
        ss << "    loop = " << nCurWindowSize << "/" << unrollSize << ";\n";
    }

    ss << "    for ( int j = 0;j< loop; j++)\n";
    ss << "    {\n";
    ss << "        int i = ";
    if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed())
    {
        ss << "gid0 + j * " << unrollSize << ";\n";
    }
    else
    {
        ss << "j * " << unrollSize << ";\n";
    }

    if (!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
    {
        ss << "        int doubleIndex = i+gid0;\n";
    }
    else
    {
        ss << "        int doubleIndex = i;\n";
    }

    for (int j = 0; j < unrollSize; j++)
    {
        ss << unrollstr.str();
        ss << "i++;\n";
        ss << "doubleIndex++;\n";
    }
    ss << "    }\n";
    ss << "    for (int i = ";
    if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed())
    {
        ss << "gid0 + loop *" << unrollSize << "; i < ";
        ss << nCurWindowSize << "; i++)\n";
    }
    else if (pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
    {
        ss << "0 + loop *" << unrollSize << "; i < gid0+";
        ss << nCurWindowSize << "; i++)\n";
    }
    else
    {
        ss << "0 + loop *" << unrollSize << "; i < ";
        ss << nCurWindowSize << "; i++)\n";
    }
    ss << "    {\n";
    if (!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
    {
        ss << "        int doubleIndex = i+gid0;\n";
    }
    else
    {
        ss << "        int doubleIndex = i;\n";
    }
    ss << unrollstr.str();
    ss << "    }\n";
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
