/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <opencl/openclwrapper.hxx>
#include <formula/vectortoken.hxx>
#include <sal/log.hxx>
#include <utility>

#include "opbase.hxx"

using namespace formula;

namespace sc::opencl {

UnhandledToken::UnhandledToken(
    const char* m, std::string  fn, int ln ) :
    mMessage(m), mFile(std::move(fn)), mLineNumber(ln) {}

OpenCLError::OpenCLError( std::string  function, cl_int error, std::string  file, int line ) :
    mFunction(std::move(function)), mError(error), mFile(std::move(file)), mLineNumber(line)
{
    // Not sure if this SAL_INFO() is useful; the place in
    // CLInterpreterContext::launchKernel() where OpenCLError is
    // caught already uses SAL_WARN() to display it.

    // SAL_INFO("sc.opencl", "OpenCL error: " << openclwrapper::errorString(mError));
}

Unhandled::Unhandled( std::string  fn, int ln ) :
    mFile(std::move(fn)), mLineNumber(ln) {}

InvalidParameterCount::InvalidParameterCount( int parameterCount, std::string file, int ln ) :
    mParameterCount(parameterCount), mFile(std::move(file)), mLineNumber(ln) {}

DynamicKernelArgument::DynamicKernelArgument( const ScCalcConfig& config, std::string s,
    FormulaTreeNodeRef  ft ) :
    mCalcConfig(config), mSymName(std::move(s)), mFormulaTree(std::move(ft)) { }

std::string DynamicKernelArgument::GenDoubleSlidingWindowDeclRef( bool ) const
{
    return std::string("");
}

/// When Mix, it will be called
std::string DynamicKernelArgument::GenStringSlidingWindowDeclRef( bool ) const
{
    return std::string("");
}

/// Generate use/references to the argument
void DynamicKernelArgument::GenDeclRef( outputstream& ss ) const
{
    ss << mSymName;
}

void DynamicKernelArgument::GenSlidingWindowFunction( outputstream& ) {}

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

VectorRef::VectorRef( const ScCalcConfig& config, const std::string& s, const FormulaTreeNodeRef& ft, int idx ) :
    DynamicKernelArgument(config, s, ft), mpClmem(nullptr), mnIndex(idx)
{
    if (mnIndex)
    {
        outputstream ss;
        ss << mSymName << "s" << mnIndex;
        mSymName = ss.str();
    }
}

VectorRef::~VectorRef()
{
    if (mpClmem)
    {
        cl_int err;
        err = clReleaseMemObject(mpClmem);
        SAL_WARN_IF(err != CL_SUCCESS, "sc.opencl", "clReleaseMemObject failed: " << openclwrapper::errorString(err));
    }
}

/// Generate declaration
void VectorRef::GenDecl( outputstream& ss ) const
{
    ss << "__global double *" << mSymName;
}

/// When declared as input to a sliding window function
void VectorRef::GenSlidingWindowDecl( outputstream& ss ) const
{
    VectorRef::GenDecl(ss);
}

/// When referenced in a sliding window function
std::string VectorRef::GenSlidingWindowDeclRef( bool nested ) const
{
    outputstream ss;
    formula::SingleVectorRefToken* pSVR =
        dynamic_cast<formula::SingleVectorRefToken*>(DynamicKernelArgument::GetFormulaToken());
    if (pSVR && !nested)
        ss << "(gid0 < " << pSVR->GetArrayLength() << "?";
    ss << mSymName << "[gid0]";
    if (pSVR && !nested)
        ss << ":NAN)";
    return ss.str();
}

void VectorRef::GenSlidingWindowFunction( outputstream& ) {}

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
        throw Unhandled(__FILE__, __LINE__);
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

void SlidingFunctionBase::GenerateArg( const char* name, int num, SubArguments& vSubArguments, outputstream& ss )
{
    CHECK_PARAMETER_COUNT_MIN( num );
    FormulaToken *token = vSubArguments[num]->GetFormulaToken();
    if( token == nullptr )
        throw Unhandled( __FILE__, __LINE__ );
    ss << "    double " << name << ";\n";
    if(token->GetOpCode() == ocPush)
    {
        if(token->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* svr =
                static_cast<const formula::SingleVectorRefToken *>(token);
            ss << "    if (gid0 >= " << svr->GetArrayLength() << " || isnan(";
            ss << vSubArguments[num]->GenSlidingWindowDeclRef() << "))\n";
            ss << "        " << name << " = 0.0;\n";
            ss << "    else\n";
            ss << "        " << name << " = ";
            ss << vSubArguments[num]->GenSlidingWindowDeclRef() << ";\n";
        }
        else if(token->GetType() == formula::svDouble)
            ss << "    " << name << " = " << token->GetDouble() << ";\n";
        else
            throw Unhandled( __FILE__, __LINE__ );
    }
    else
    {
        ss << "    " << name << " = ";
        ss << vSubArguments[num]->GenSlidingWindowDeclRef() << ";\n";
    }
}

void SlidingFunctionBase::GenerateArg( int num, SubArguments& vSubArguments, outputstream& ss )
{
    char buf[ 30 ];
    sprintf( buf, "arg%d", num );
    GenerateArg( buf, num, vSubArguments, ss );
}

void SlidingFunctionBase::GenerateFunctionDeclaration( const std::string& sSymName,
    SubArguments& vSubArguments, outputstream& ss )
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ", ";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
}

void Normal::GenSlidingWindowFunction(
    outputstream& ss, const std::string& sSymName, SubArguments& vSubArguments )
{
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n\t";
    ss << "double tmp = " << GetBottom() << ";\n\t";
    ss << "int gid0 = get_global_id(0);\n\t";
    ss << "tmp = ";
    std::vector<std::string> argVector;
    for (size_t i = 0; i < vSubArguments.size(); i++)
        argVector.push_back(vSubArguments[i]->GenSlidingWindowDeclRef());
    ss << Gen(argVector);
    ss << ";\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

void CheckVariables::GenTmpVariables(
    outputstream& ss, const SubArguments& vSubArguments )
{
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        ss << "    double tmp";
        ss << i;
        ss << ";\n";
    }
}

void CheckVariables::CheckSubArgumentIsNan( outputstream& ss,
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
        ss << "isnan(";
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
        ss << "isnan(";
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
    if (vSubArguments[i]->GetFormulaToken()->GetType() == formula::svDouble ||
        vSubArguments[i]->GetFormulaToken()->GetOpCode() != ocPush)
    {
        ss << "    if(";
        ss << "isnan(";
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

void CheckVariables::CheckSubArgumentIsNan2( outputstream& ss,
    SubArguments& vSubArguments,  int argumentNum, const std::string& p )
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
    outputstream& ss, SubArguments& vSubArguments )
{
    ss << "    int k = gid0;\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        CheckSubArgumentIsNan(ss, vSubArguments, i);
    }
}

void CheckVariables::UnrollDoubleVector( outputstream& ss,
    const outputstream& unrollstr, const formula::DoubleVectorRefToken* pCurDVR,
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
