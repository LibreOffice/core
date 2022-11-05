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
#include <rtl/string.hxx>
#include <sal/log.hxx>
#include <utility>
#include <unordered_map>

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

// Strings and OpenCL:
// * Strings are non-trivial types and so passing them to OpenCL and handling them there
// would be rather complex. However, in practice most string operations are checking
// string equality, so only such string usage is supported (other cases will be
// handled by Calc core when they get rejected for OpenCL).
// * Strings from Calc core come from svl::SharedString, which already ensures that
// equal strings have equal rtl_uString.
// * Strings are passed to opencl as integer IDs, each uniquely identifying a different
// string.
// * OpenCL code generally handles all values as doubles, so merely converting rtl_uString*
// to double could lead to loss of precision (double can store 52bits of precision).
// This could lead to two strings possibly being considered equal by mistake (unlikely,
// but not impossible). Therefore all rtl_uString* are mapped to internal integer IDs.
// * Functions that can handle strings properly should override OpBase::takeString()
// to return true. They should
// * Empty string Id is 0. Empty cell Id is NAN.
// * Since strings are marshalled as doubles too, it is important to check whether a value
// is a real double or a string. Use e.g. GenerateArgType to generate also 'xxx_is_string'
// variable, there is cell_equal() function to compare two cells.

static std::unordered_map<const rtl_uString*, int>* stringIdsMap;

int DynamicKernelArgument::GetStringId( const rtl_uString* string )
{
    assert( string != nullptr );
    if( string->length == 0 )
        return 0;
    if( stringIdsMap == nullptr )
        stringIdsMap = new std::unordered_map<const rtl_uString*, int>;
    std::unordered_map<const rtl_uString*, int>::iterator it = stringIdsMap->find( string );
    if( it != stringIdsMap->end())
        return it->second;
    int newId = stringIdsMap->size() + 1;
    stringIdsMap->insert( std::pair( string, newId ));
    return newId;
}

void DynamicKernelArgument::ClearStringIds()
{
    delete stringIdsMap;
    stringIdsMap = nullptr;
}

VectorRef::VectorRef( const ScCalcConfig& config, const std::string& s, const FormulaTreeNodeRef& ft, int idx ) :
    DynamicKernelArgument(config, s, ft), mpClmem(nullptr), mnIndex(idx), forceStringsToZero( false )
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

VectorRefStringsToZero::VectorRefStringsToZero( const ScCalcConfig& config, const std::string& s,
    const FormulaTreeNodeRef& ft, int index )
    : VectorRef( config, s, ft, index )
{
    forceStringsToZero = true;
}

void SlidingFunctionBase::GenerateArg( const char* name, int arg, SubArguments& vSubArguments,
    outputstream& ss, EmptyArgType empty, GenerateArgTypeType generateType )
{
    assert( arg < int( vSubArguments.size()));
    FormulaToken *token = vSubArguments[arg]->GetFormulaToken();
    if( token == nullptr )
        throw Unhandled( __FILE__, __LINE__ );
    if(token->GetOpCode() == ocPush)
    {
        if(token->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* svr =
                static_cast<const formula::SingleVectorRefToken *>(token);
            ss << "    double " << name << " = NAN;\n";
            if( generateType == GenerateArgType )
                ss << "    bool " << name << "_is_string = false;\n";
            ss << "    if (gid0 < " << svr->GetArrayLength() << ")\n";
            if( generateType == GenerateArgType )
                ss << "    {\n";
            ss << "        " << name << " = ";
            ss << vSubArguments[arg]->GenSlidingWindowDeclRef( true ) << ";\n";
            if( generateType == GenerateArgType )
            {
                ss << "        " << name << "_is_string = ";
                ss << vSubArguments[arg]->GenIsString( true ) << ";\n";
                ss << "    }\n";
            }
            switch( empty )
            {
                case EmptyIsZero:
                    ss << "    if( isnan( " << name << " ))\n";
                    ss << "        " << name << " = 0;\n";
                    break;
                case EmptyIsNan:
                    break;
                case SkipEmpty:
                    abort();
                    break;
            }
        }
        else if(token->GetType() == formula::svDouble)
        {
            ss << "    double " << name << " = " << token->GetDouble() << ";\n";
            if( generateType == GenerateArgType )
                ss << "    bool " << name << "_is_string = "
                    << vSubArguments[arg]->GenIsString() << ";\n";
        }
        else if(token->GetType() == formula::svString)
        {
            if( forceStringsToZero())
                assert( dynamic_cast<DynamicKernelStringToZeroArgument*>(vSubArguments[arg].get()));
            else if( !takeString())
                throw Unhandled( __FILE__, __LINE__ );
            ss << "    double " << name << " = 0.0;\n";
            if( generateType == GenerateArgType )
                ss << "    bool " << name << "_is_string = "
                    << vSubArguments[arg]->GenIsString() << ";\n";
        }
        else
            throw Unhandled( __FILE__, __LINE__ );
    }
    else
    {
        ss << "    double " << name << " = ";
        ss << vSubArguments[arg]->GenSlidingWindowDeclRef() << ";\n";
        if( generateType == GenerateArgType )
            ss << "    bool " << name << "_is_string = "
                << vSubArguments[arg]->GenIsString() << ";\n";
    }
}

void SlidingFunctionBase::GenerateArg( int arg, SubArguments& vSubArguments, outputstream& ss,
    EmptyArgType empty, GenerateArgTypeType generateType )
{
    OString buf = "arg" + OString::number(arg);
    GenerateArg( buf.getStr(), arg, vSubArguments, ss, empty, generateType );
}

void SlidingFunctionBase::GenerateArgWithDefault( const char* name, int arg, double def,
    SubArguments& vSubArguments, outputstream& ss, EmptyArgType empty )
{
    if( arg < int(vSubArguments.size()))
        GenerateArg( name, arg, vSubArguments, ss, empty );
    else
        ss << "    double " << name << " = " << def << ";\n";
}

void SlidingFunctionBase::GenerateRangeArgs( int firstArg, int lastArg, SubArguments& vSubArguments,
    outputstream& ss, EmptyArgType empty, const char* code )
{
    assert( firstArg >= 0 );
    assert( firstArg <= lastArg );
    assert( lastArg < int( vSubArguments.size()));
    for( int i = firstArg;
         i <= lastArg;
         ++i )
    {
        FormulaToken *token = vSubArguments[i]->GetFormulaToken();
        if( token == nullptr )
            throw Unhandled( __FILE__, __LINE__ );
        if(token->GetOpCode() == ocPush)
        {
            if (token->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(token);
                GenerateDoubleVectorLoopHeader( ss, pDVR, nullptr );
                ss << "        double arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ";\n";
                switch( empty )
                {
                    case EmptyIsZero:
                        ss << "        if( isnan( arg ))\n";
                        ss << "            arg = 0;\n";
                        break;
                    case EmptyIsNan:
                        break;
                    case SkipEmpty:
                        ss << "        if( isnan( arg ))\n";
                        ss << "            continue;\n";
                        break;
                }
                ss << code;
                ss << "    }\n";
            }
            else if (token->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken*>(token);
                ss << "    if (gid0 < " << pSVR->GetArrayLength() << ")\n";
                ss << "    {\n";
                ss << "        double arg = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
                switch( empty )
                {
                    case EmptyIsZero:
                        ss << "        if( isnan( arg ))\n";
                        ss << "            arg = 0;\n";
                        ss << code;
                        break;
                    case EmptyIsNan:
                        ss << code;
                        break;
                    case SkipEmpty:
                        ss << "        if( !isnan( arg ))\n";
                        ss << "        {\n";
                        ss << code;
                        ss << "        }\n";
                        break;
                }
                ss << "    }\n";
            }
            else if(token->GetType() == formula::svDouble)
            {
                ss << "    {\n";
                ss << "        double arg = " << token->GetDouble() << ";\n";
                ss << code;
                ss << "    }\n";
            }
            else if(token->GetType() == formula::svString)
            {
                assert( dynamic_cast<DynamicKernelStringToZeroArgument*>(vSubArguments[i].get()));
                ss << "    {\n";
                ss << "        double arg = 0.0;\n";
                ss << code;
                ss << "    }\n";
            }
            else
                throw Unhandled( __FILE__, __LINE__ );
        }
        else
        {
            ss << "    {\n";
            ss << "        double arg = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef() << ";\n";
            ss << code;
            ss << "    }\n";
        }
    }
}

void SlidingFunctionBase::GenerateRangeArgs( SubArguments& vSubArguments,
    outputstream& ss, EmptyArgType empty, const char* code )
{
    GenerateRangeArgs( 0, vSubArguments.size() - 1, vSubArguments, ss, empty, code );
}

void SlidingFunctionBase::GenerateRangeArg( int arg, SubArguments& vSubArguments,
    outputstream& ss, EmptyArgType empty, const char* code )
{
    GenerateRangeArgs( arg, arg, vSubArguments, ss, empty, code );
}

void SlidingFunctionBase::GenerateRangeArgPair( int arg1, int arg2, SubArguments& vSubArguments,
    outputstream& ss, EmptyArgType empty, const char* code, const char* firstElementDiff )
{
    assert( arg1 >= 0 && arg1 < int (vSubArguments.size()));
    assert( arg2 >= 0 && arg2 < int (vSubArguments.size()));
    assert( arg1 != arg2 );
    FormulaToken *token1 = vSubArguments[arg1]->GetFormulaToken();
    if( token1 == nullptr )
        throw Unhandled( __FILE__, __LINE__ );
    FormulaToken *token2 = vSubArguments[arg2]->GetFormulaToken();
    if( token2 == nullptr )
        throw Unhandled( __FILE__, __LINE__ );
    if(token1->GetType() != formula::svDoubleVectorRef
        || token2->GetType() != formula::svDoubleVectorRef)
    {
        throw Unhandled( __FILE__, __LINE__ );
    }
    const formula::DoubleVectorRefToken* pDVR1 =
        static_cast<const formula::DoubleVectorRefToken *>(token1);
    const formula::DoubleVectorRefToken* pDVR2 =
        static_cast<const formula::DoubleVectorRefToken *>(token2);

    size_t nCurWindowSize1  = pDVR1->GetRefRowSize();
    size_t nCurWindowSize2 = pDVR2->GetRefRowSize();

    if(nCurWindowSize1 != nCurWindowSize2)
        throw Unhandled( __FILE__, __LINE__ );
    if(pDVR1->IsStartFixed() != pDVR2->IsStartFixed()
        || pDVR1->IsEndFixed() != pDVR2->IsEndFixed())
    {
        throw Unhandled( __FILE__, __LINE__ );
    }

    // If either of the ranges ends with empty cells, it will not include those last
    // nan values (its GetArrayLength() will be less than its GetRefRowSize().
    // If we skip empty cells, just iterate until both ranges have elements, but if
    // we need to iterate even over empty cells, so use the longer one.
    // FIXME: If both ranges end with empty cells, this does not actually iterate
    // over all empty cells.
    const formula::DoubleVectorRefToken* loopDVR;
    bool checkBounds;
    if( empty == SkipEmpty )
    {
        loopDVR = pDVR1->GetArrayLength() < pDVR2->GetArrayLength() ? pDVR1 : pDVR2;
        checkBounds = false;
    }
    else
    {
        loopDVR = pDVR1->GetArrayLength() > pDVR2->GetArrayLength() ? pDVR1 : pDVR2;
        checkBounds = true;
    }
    GenerateDoubleVectorLoopHeader( ss, loopDVR, firstElementDiff );
    ss << "        double arg1 = ";
    ss << vSubArguments[arg1]->GenSlidingWindowDeclRef(!checkBounds) << ";\n";
    ss << "        double arg2 = ";
    ss << vSubArguments[arg2]->GenSlidingWindowDeclRef(!checkBounds) << ";\n";
    switch( empty )
    {
        case EmptyIsZero:
            ss << "        if( isnan( arg1 ))\n";
            ss << "            arg1 = 0;\n";
            ss << "        if( isnan( arg2 ))\n";
            ss << "            arg2 = 0;\n";
            break;
        case EmptyIsNan:
            break;
        case SkipEmpty:
            ss << "        if( isnan( arg1 ) || isnan( arg2 ))\n";
            ss << "            continue;\n";
            break;
    }
    ss << code;
    ss << "    }\n";
}

void SlidingFunctionBase::GenerateRangeArgElement( const char* name, int arg, const char* element,
    SubArguments& vSubArguments, outputstream& ss, EmptyArgType empty )
{
    assert( arg >= 0 && arg < int (vSubArguments.size()));
    FormulaToken *token = vSubArguments[arg]->GetFormulaToken();
    if( token == nullptr )
        throw Unhandled( __FILE__, __LINE__ );
    if(token->GetType() != formula::svDoubleVectorRef)
        throw Unhandled( __FILE__, __LINE__ );
    const formula::DoubleVectorRefToken* pDVR =
        static_cast<const formula::DoubleVectorRefToken *>(token);
    ss << "    double " << name << " = NAN;\n";
    ss << "    {\n";
    // GenSlidingWindowDeclRef() may refer to 'i' variable.
    ss << "        int i = 0;\n";
    ss << "        if( ";
    if( !pDVR->IsStartFixed())
        ss << "gid0 + ";
    ss << element << " < " << pDVR->GetArrayLength() << " )\n";
    ss << "            " << name << " = " << vSubArguments[arg]->GenSlidingWindowDeclRef(true) << ";\n";
    ss << "    }\n";
    switch( empty )
    {
        case EmptyIsZero:
            ss << "        if( isnan( " << name << " ))\n";
            ss << "            " << name << " = 0;\n";
            break;
        case EmptyIsNan:
            break;
        case SkipEmpty:
            abort();
            break;
    }
}

void SlidingFunctionBase::GenerateDoubleVectorLoopHeader( outputstream& ss,
    const formula::DoubleVectorRefToken* pDVR, const char* firstElementDiff )
{
    size_t nCurWindowSize = pDVR->GetRefRowSize();
    std::string startDiff;
    if( firstElementDiff )
        startDiff = std::string( " + " ) + firstElementDiff;
    ss << "    for (int i = ";
    if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
    {
        ss << "gid0" << startDiff << "; i < " << pDVR->GetArrayLength();
        ss << " && i < " << nCurWindowSize  << "; i++)\n";
        ss << "    {\n";
    }
    else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
    {
        ss << "0" << startDiff << "; i < " << pDVR->GetArrayLength();
        ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
        ss << "    {\n";
    }
    else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
    {
        ss << "0" << startDiff << "; i + gid0 < " << pDVR->GetArrayLength();
        ss << " &&  i < " << nCurWindowSize << "; i++)\n";
        ss << "    {\n";
    }
    else
    {
        ss << "0" << startDiff << "; i < " << pDVR->GetArrayLength() << "; i++)\n";
        ss << "    {\n";
    }
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

void Reduction::GenSlidingWindowFunction( outputstream& ss,
    const std::string& sSymName, SubArguments& vSubArguments )
{
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "double tmp = " << GetBottom() << ";\n";
    ss << "int gid0 = get_global_id(0);\n";
    if (isAverage() || isMinOrMax())
        ss << "int nCount = 0;\n";
    ss << "double tmpBottom;\n";
    unsigned i = vSubArguments.size();
    while (i--)
    {
        if (NumericRange* NR = dynamic_cast<NumericRange*>(vSubArguments[i].get()))
        {
            bool needBody;
            NR->GenReductionLoopHeader(ss, needBody);
            if (!needBody)
                continue;
        }
        else if (NumericRangeStringsToZero* NRS = dynamic_cast<NumericRangeStringsToZero*>(vSubArguments[i].get()))
        {
            bool needBody;
            NRS->GenReductionLoopHeader(ss, needBody);
            if (!needBody)
                continue;
        }
        else if (ParallelNumericRange* PNR = dynamic_cast<ParallelNumericRange*>(vSubArguments[i].get()))
        {
            //did not handle yet
            bool bNeedBody = false;
            PNR->GenReductionLoopHeader(ss, mnResultSize, bNeedBody);
            if (!bNeedBody)
                continue;
        }
        else if (StringRange* SR = dynamic_cast<StringRange*>(vSubArguments[i].get()))
        {
            //did not handle yet
            bool needBody;
            SR->GenReductionLoopHeader(ss, needBody);
            if (!needBody)
                continue;
        }
        else
        {
            FormulaToken* pCur = vSubArguments[i]->GetFormulaToken();
            if( pCur == nullptr || pCur->GetType() == formula::svDoubleVectorRef )
            {
                throw Unhandled(__FILE__, __LINE__);
            }
            ss << "{\n";
        }
        if (ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            bool bNanHandled = HandleNaNArgument(ss, i, vSubArguments);

            ss << "    tmpBottom = " << GetBottom() << ";\n";

            if (!bNanHandled)
            {
                ss << "    if (isnan(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << "))\n";
                if (ZeroReturnZero())
                    ss << "        return 0;\n";
                else
                {
                    ss << "        tmp = ";
                    ss << Gen2("tmpBottom", "tmp") << ";\n";
                }
                ss << "    else\n";
            }
            ss << "        tmp = ";
            ss << Gen2(vSubArguments[i]->GenSlidingWindowDeclRef(), "tmp");
            ss << ";\n";
        }
        else
        {
            ss << "    tmp = ";
            ss << Gen2(vSubArguments[i]->GenSlidingWindowDeclRef(), "tmp");
            ss << ";\n";
        }
        ss << "}\n";
    }
    if (isAverage())
        ss <<
            "if (nCount==0)\n"
            "    return CreateDoubleError(DivisionByZero);\n";
    else if (isMinOrMax())
        ss <<
            "if (nCount==0)\n"
            "    return 0;\n";
    ss << "return tmp";
    if (isAverage())
        ss << "/(double)nCount";
    ss << ";\n}";
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
