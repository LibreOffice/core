/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <formulagroup.hxx>
#include <formulagroupcl.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <tokenarray.hxx>
#include <compiler.hxx>
#include <comphelper/random.hxx>
#include <scmatrix.hxx>
#include <sal/log.hxx>

#include <opencl/openclwrapper.hxx>
#include <opencl/OpenCLZone.hxx>

#include "op_financial.hxx"
#include "op_math.hxx"
#include "op_logical.hxx"
#include "op_statistical.hxx"
#include "op_array.hxx"
#include "op_spreadsheet.hxx"
#include "op_addin.hxx"

#include <limits>

#include <com/sun/star/sheet/FormulaLanguage.hpp>

const char* const publicFunc =
 "\n"
 "#define IllegalArgument 502\n"
 "#define IllegalFPOperation 503 // #NUM!\n"
 "#define NoValue 519 // #VALUE!\n"
 "#define NoConvergence 523\n"
 "#define DivisionByZero 532 // #DIV/0!\n"
 "#define NOTAVAILABLE 0x7fff // #N/A\n"
 "\n"
 "double CreateDoubleError(ulong nErr)\n"
 "{\n"
 // At least nVidia on Linux and Intel on Windows seem to ignore the argument to nan(),
 // so using that would not propagate the type of error, work that around
 // by directly constructing the proper IEEE double NaN value
 // TODO: maybe use a better way to detect such systems?
 "    return as_double(0x7FF8000000000000+nErr);\n"
// "    return nan(nErr);\n"
 "}\n"
 "\n"
 "double fsum(double a, double b) { return isnan(a)?b:a+b; }\n"
 "double legalize(double a, double b) { return isnan(a)?b:a;}\n"
 ;

#include <utility>
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>

#include <rtl/digest.h>

#include <memory>

using namespace formula;

namespace sc::opencl {

namespace {

std::string linenumberify(const std::string& s)
{
    outputstream ss;
    int linenumber = 1;
    size_t start = 0;
    size_t newline;
    while ((newline = s.find('\n', start)) != std::string::npos)
    {
        ss << "/*" << std::setw(4) << linenumber++ << "*/ " << s.substr(start, newline-start+1);
        start = newline + 1;
    }
    if (start < s.size())
        ss << "/*" << std::setw(4) << linenumber++ << "*/ " << s.substr(start, std::string::npos);
    return ss.str();
}

bool AllStringsAreNull(const rtl_uString* const* pStringArray, size_t nLength)
{
    if (pStringArray == nullptr)
        return true;

    for (size_t i = 0; i < nLength; i++)
        if (pStringArray[i] != nullptr)
            return false;

    return true;
}

OUString LimitedString( std::u16string_view str )
{
    if( str.size() < 20 )
        return OUString::Concat("\"") + str + "\"";
    else
        return OUString::Concat("\"") + str.substr( 0, 20 ) + "\"...";
}

const int MAX_PEEK_ELEMENTS = 5;
// Returns formatted contents of the data (possibly shortened), to be used in debug output.
std::string DebugPeekData(const FormulaToken* ref, int doubleRefIndex = 0)
{
    if (ref->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken* pSVR =
            static_cast<const formula::SingleVectorRefToken*>(ref);
        outputstream buf;
        buf << "SingleRef {";
        for( size_t i = 0; i < std::min< size_t >( MAX_PEEK_ELEMENTS, pSVR->GetArrayLength()); ++i )
        {
            if( i != 0 )
                buf << ",";
            if( pSVR->GetArray().mpStringArray != nullptr
                && pSVR->GetArray().mpStringArray[ i ] != nullptr )
            {
                buf << LimitedString( OUString( pSVR->GetArray().mpStringArray[ i ] ));
            }
            else if( pSVR->GetArray().mpNumericArray != nullptr )
                buf << pSVR->GetArray().mpNumericArray[ i ];
        }
        if( pSVR->GetArrayLength() > MAX_PEEK_ELEMENTS )
            buf << ",...";
        buf << "}";
        return buf.str();
    }
    else if (ref->GetType() == formula::svDoubleVectorRef)
    {
        const formula::DoubleVectorRefToken* pDVR =
            static_cast<const formula::DoubleVectorRefToken*>(ref);
        outputstream buf;
        buf << "DoubleRef {";
        for( size_t i = 0; i < std::min< size_t >( MAX_PEEK_ELEMENTS, pDVR->GetArrayLength()); ++i )
        {
            if( i != 0 )
                buf << ",";
            if( pDVR->GetArrays()[doubleRefIndex].mpStringArray != nullptr
                && pDVR->GetArrays()[doubleRefIndex].mpStringArray[ i ] != nullptr )
            {
                buf << LimitedString( OUString( pDVR->GetArrays()[doubleRefIndex].mpStringArray[ i ] ));
            }
            else if( pDVR->GetArrays()[doubleRefIndex].mpNumericArray != nullptr )
                buf << pDVR->GetArrays()[doubleRefIndex].mpNumericArray[ i ];
        }
        if( pDVR->GetArrayLength() > MAX_PEEK_ELEMENTS )
            buf << ",...";
        buf << "}";
        return buf.str();
    }
    else if (ref->GetType() == formula::svString)
    {
        outputstream buf;
        buf << "String " << LimitedString( ref->GetString().getString());
        return buf.str();
    }
    else if (ref->GetType() == formula::svDouble)
    {
        return preciseFloat(ref->GetDouble());
    }
    else
    {
        return "?";
    }
}

// Returns formatted contents of a doubles buffer, to be used in debug output.
std::string DebugPeekDoubles(const double* data, int size)
{
    outputstream buf;
    buf << "{";
    for( int i = 0; i < std::min( MAX_PEEK_ELEMENTS, size ); ++i )
    {
        if( i != 0 )
            buf << ",";
        buf << data[ i ];
    }
    if( size > MAX_PEEK_ELEMENTS )
        buf << ",...";
    buf << "}";
    return buf.str();
}

} // anonymous namespace

/// Map the buffer used by an argument and do necessary argument setting
size_t VectorRef::Marshal( cl_kernel k, int argno, int, cl_program )
{
    OpenCLZone zone;
    FormulaToken* ref = mFormulaTree->GetFormulaToken();
    double* pHostBuffer = nullptr;
    size_t szHostBuffer = 0;
    if (ref->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken* pSVR =
            static_cast<const formula::SingleVectorRefToken*>(ref);

        SAL_INFO("sc.opencl", "SingleVectorRef len=" << pSVR->GetArrayLength() << " mpNumericArray=" << pSVR->GetArray().mpNumericArray << " (mpStringArray=" << pSVR->GetArray().mpStringArray << ")");

        if( forceStringsToZero && pSVR->GetArray().mpStringArray != nullptr )
        {
            dataBuffer.resize( pSVR->GetArrayLength());
            for( size_t i = 0; i < pSVR->GetArrayLength(); ++i )
                if( pSVR->GetArray().mpStringArray[ i ] != nullptr )
                    dataBuffer[ i ] = 0;
                else
                    dataBuffer[ i ] = pSVR->GetArray().mpNumericArray[ i ];
            pHostBuffer = dataBuffer.data();
            SAL_INFO("sc.opencl", "Forced strings to zero : " << DebugPeekDoubles( pHostBuffer, pSVR->GetArrayLength()));
        }
        else
        {
            pHostBuffer = const_cast<double*>(pSVR->GetArray().mpNumericArray);
        }
        szHostBuffer = pSVR->GetArrayLength() * sizeof(double);
    }
    else if (ref->GetType() == formula::svDoubleVectorRef)
    {
        const formula::DoubleVectorRefToken* pDVR =
            static_cast<const formula::DoubleVectorRefToken*>(ref);

        SAL_INFO("sc.opencl", "DoubleVectorRef index=" << mnIndex << " len=" << pDVR->GetArrayLength() << " mpNumericArray=" << pDVR->GetArrays()[mnIndex].mpNumericArray << " (mpStringArray=" << pDVR->GetArrays()[mnIndex].mpStringArray << ")");

        if( forceStringsToZero && pDVR->GetArrays()[mnIndex].mpStringArray != nullptr )
        {
            dataBuffer.resize( pDVR->GetArrayLength());
            for( size_t i = 0; i < pDVR->GetArrayLength(); ++i )
                if( pDVR->GetArrays()[mnIndex].mpStringArray[ i ] != nullptr )
                    dataBuffer[ i ] = 0;
                else
                    dataBuffer[ i ] = pDVR->GetArrays()[mnIndex].mpNumericArray[ i ];
            pHostBuffer = dataBuffer.data();
            SAL_INFO("sc.opencl", "Forced strings to zero : " << DebugPeekDoubles( pHostBuffer, pDVR->GetArrayLength()));
        }
        else
        {
            pHostBuffer = const_cast<double*>(pDVR->GetArrays()[mnIndex].mpNumericArray);
        }
        szHostBuffer = pDVR->GetArrayLength() * sizeof(double);
    }
    else
    {
        throw Unhandled(__FILE__, __LINE__);
    }

    openclwrapper::KernelEnv kEnv;
    openclwrapper::setKernelEnv(&kEnv);
    cl_int err;
    if (pHostBuffer)
    {
        mpClmem = clCreateBuffer(kEnv.mpkContext,
            cl_mem_flags(CL_MEM_READ_ONLY) | CL_MEM_USE_HOST_PTR,
            szHostBuffer,
            pHostBuffer, &err);
        if (CL_SUCCESS != err)
            throw OpenCLError("clCreateBuffer", err, __FILE__, __LINE__);
        SAL_INFO("sc.opencl", "Created buffer " << mpClmem << " size " << szHostBuffer << " using host buffer " << pHostBuffer);
    }
    else
    {
        if (szHostBuffer == 0)
            szHostBuffer = sizeof(double); // a dummy small value
                                           // Marshal as a buffer of NANs
        mpClmem = clCreateBuffer(kEnv.mpkContext,
            cl_mem_flags(CL_MEM_READ_ONLY) | CL_MEM_ALLOC_HOST_PTR,
            szHostBuffer, nullptr, &err);
        if (CL_SUCCESS != err)
            throw OpenCLError("clCreateBuffer", err, __FILE__, __LINE__);
        SAL_INFO("sc.opencl", "Created buffer " << mpClmem << " size " << szHostBuffer);

        double* pNanBuffer = static_cast<double*>(clEnqueueMapBuffer(
            kEnv.mpkCmdQueue, mpClmem, CL_TRUE, CL_MAP_WRITE, 0,
            szHostBuffer, 0, nullptr, nullptr, &err));
        if (CL_SUCCESS != err)
            throw OpenCLError("clEnqueueMapBuffer", err, __FILE__, __LINE__);

        for (size_t i = 0; i < szHostBuffer / sizeof(double); i++)
            pNanBuffer[i] = std::numeric_limits<double>::quiet_NaN();
        err = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue, mpClmem,
            pNanBuffer, 0, nullptr, nullptr);
        // FIXME: Is it intentional to not throw an OpenCLError even if the clEnqueueUnmapMemObject() fails?
        if (CL_SUCCESS != err)
            SAL_WARN("sc.opencl", "clEnqueueUnmapMemObject failed: " << openclwrapper::errorString(err));
    }

    SAL_INFO("sc.opencl", "Kernel " << k << " arg " << argno << ": cl_mem: " << mpClmem << " (" << DebugPeekData(ref, mnIndex) << ")");
    err = clSetKernelArg(k, argno, sizeof(cl_mem), static_cast<void*>(&mpClmem));
    if (CL_SUCCESS != err)
        throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
    return 1;
}

namespace {

class DynamicKernelPiArgument : public DynamicKernelArgument
{
public:
    DynamicKernelPiArgument( const ScCalcConfig& config, const std::string& s,
        const FormulaTreeNodeRef& ft ) :
        DynamicKernelArgument(config, s, ft) { }
    /// Generate declaration
    virtual void GenDecl( outputstream& ss ) const override
    {
        ss << "double " << mSymName;
    }
    virtual void GenDeclRef( outputstream& ss ) const override
    {
        ss << "M_PI";
    }
    virtual void GenSlidingWindowDecl( outputstream& ss ) const override
    {
        GenDecl(ss);
    }
    virtual std::string GenSlidingWindowDeclRef( bool = false ) const override
    {
        return mSymName;
    }
    virtual size_t GetWindowSize() const override
    {
        return 1;
    }
    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal( cl_kernel k, int argno, int, cl_program ) override
    {
        OpenCLZone zone;
        double tmp = 0.0;
        // Pass the scalar result back to the rest of the formula kernel
        SAL_INFO("sc.opencl", "Kernel " << k << " arg " << argno << ": double: " << preciseFloat( tmp ) << " (PI)");
        cl_int err = clSetKernelArg(k, argno, sizeof(double), static_cast<void*>(&tmp));
        if (CL_SUCCESS != err)
            throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
        return 1;
    }
};

class DynamicKernelRandomArgument : public DynamicKernelArgument
{
public:
    DynamicKernelRandomArgument( const ScCalcConfig& config, const std::string& s,
        const FormulaTreeNodeRef& ft ) :
        DynamicKernelArgument(config, s, ft) { }
    /// Generate declaration
    virtual void GenDecl( outputstream& ss ) const override
    {
        ss << "double " << mSymName;
    }
    virtual void GenDeclRef( outputstream& ss ) const override
    {
        ss << mSymName;
    }
    virtual void GenSlidingWindowDecl( outputstream& ss ) const override
    {
        ss << "int " << mSymName;
    }
    virtual std::string GenSlidingWindowDeclRef( bool = false ) const override
    {
        return mSymName + "_Random(" + mSymName + ")";
    }
    virtual void GenSlidingWindowFunction( outputstream& ss ) override
    {
        // This string is from the pi_opencl_kernel.i file as
        // generated when building the Random123 examples. Unused
        // stuff has been removed, and the actual kernel is not the
        // same as in the totally different use case of that example,
        // of course. Only the code that calculates the counter-based
        // random number and what it needs is left.
        ss << "\
\n\
#ifndef DEFINED_RANDOM123_STUFF\n\
#define DEFINED_RANDOM123_STUFF\n\
\n\
/*\n\
Copyright 2010-2011, D. E. Shaw Research.\n\
All rights reserved.\n\
\n\
Redistribution and use in source and binary forms, with or without\n\
modification, are permitted provided that the following conditions are\n\
met:\n\
\n\
* Redistributions of source code must retain the above copyright\n\
  notice, this list of conditions, and the following disclaimer.\n\
\n\
* Redistributions in binary form must reproduce the above copyright\n\
  notice, this list of conditions, and the following disclaimer in the\n\
  documentation and/or other materials provided with the distribution.\n\
\n\
* Neither the name of D. E. Shaw Research nor the names of its\n\
  contributors may be used to endorse or promote products derived from\n\
  this software without specific prior written permission.\n\
\n\
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n\
\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n\
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n\
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n\
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n\
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\n\
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n\
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n\
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n\
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n\
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\
*/\n\
\n\
typedef uint uint32_t;\n\
struct r123array2x32\n\
{\n\
  uint32_t v[2];\n\
};\n\
enum r123_enum_threefry32x2\n\
{\n\
  R_32x2_0_0 = 13,\n\
  R_32x2_1_0 = 15,\n\
  R_32x2_2_0 = 26,\n\
  R_32x2_3_0 = 6,\n\
  R_32x2_4_0 = 17,\n\
  R_32x2_5_0 = 29,\n\
  R_32x2_6_0 = 16,\n\
  R_32x2_7_0 = 24\n\
};\n\
inline uint32_t RotL_32 (uint32_t x, unsigned int N)\n\
  __attribute__ ((always_inline));\n\
inline uint32_t\n\
RotL_32 (uint32_t x, unsigned int N)\n\
{\n\
  return (x << (N & 31)) | (x >> ((32 - N) & 31));\n\
}\n\
\n\
typedef struct r123array2x32 threefry2x32_ctr_t;\n\
typedef struct r123array2x32 threefry2x32_key_t;\n\
typedef struct r123array2x32 threefry2x32_ukey_t;\n\
inline threefry2x32_key_t\n\
threefry2x32keyinit (threefry2x32_ukey_t uk)\n\
{\n\
  return uk;\n\
}\n\
\n\
inline threefry2x32_ctr_t threefry2x32_R (unsigned int Nrounds,\n\
                      threefry2x32_ctr_t in,\n\
                      threefry2x32_key_t k)\n\
  __attribute__ ((always_inline));\n\
inline threefry2x32_ctr_t\n\
threefry2x32_R (unsigned int Nrounds, threefry2x32_ctr_t in,\n\
        threefry2x32_key_t k)\n\
{\n\
  threefry2x32_ctr_t X;\n\
  uint32_t ks[2 + 1];\n\
  int i;\n\
  ks[2] = 0x1BD11BDA;\n\
  for (i = 0; i < 2; i++) {\n\
    ks[i] = k.v[i];\n\
    X.v[i] = in.v[i];\n\
    ks[2] ^= k.v[i];\n\
  }\n\
  X.v[0] += ks[0];\n\
  X.v[1] += ks[1];\n\
  if (Nrounds > 0) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_0_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 1) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_1_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 2) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_2_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 3) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_3_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 3) {\n\
    X.v[0] += ks[1];\n\
    X.v[1] += ks[2];\n\
    X.v[1] += 1;\n\
  }\n\
  if (Nrounds > 4) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_4_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 5) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_5_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 6) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_6_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 7) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_7_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 7) {\n\
    X.v[0] += ks[2];\n\
    X.v[1] += ks[0];\n\
    X.v[1] += 2;\n\
  }\n\
  if (Nrounds > 8) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_0_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 9) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_1_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 10) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_2_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 11) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_3_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 11) {\n\
    X.v[0] += ks[0];\n\
    X.v[1] += ks[1];\n\
    X.v[1] += 3;\n\
  }\n\
  if (Nrounds > 12) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_4_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 13) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_5_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 14) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_6_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 15) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_7_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 15) {\n\
    X.v[0] += ks[1];\n\
    X.v[1] += ks[2];\n\
    X.v[1] += 4;\n\
  }\n\
  if (Nrounds > 16) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_0_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 17) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_1_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 18) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_2_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 19) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_3_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 19) {\n\
    X.v[0] += ks[2];\n\
    X.v[1] += ks[0];\n\
    X.v[1] += 5;\n\
  }\n\
  if (Nrounds > 20) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_4_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 21) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_5_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 22) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_6_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 23) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_7_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 23) {\n\
    X.v[0] += ks[0];\n\
    X.v[1] += ks[1];\n\
    X.v[1] += 6;\n\
  }\n\
  if (Nrounds > 24) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_0_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 25) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_1_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 26) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_2_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 27) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_3_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 27) {\n\
    X.v[0] += ks[1];\n\
    X.v[1] += ks[2];\n\
    X.v[1] += 7;\n\
  }\n\
  if (Nrounds > 28) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_4_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 29) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_5_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 30) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_6_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 31) {\n\
    X.v[0] += X.v[1];\n\
    X.v[1] = RotL_32 (X.v[1], R_32x2_7_0);\n\
    X.v[1] ^= X.v[0];\n\
  }\n\
  if (Nrounds > 31) {\n\
    X.v[0] += ks[2];\n\
    X.v[1] += ks[0];\n\
    X.v[1] += 8;\n\
  }\n\
  return X;\n\
}\n\
\n\
enum r123_enum_threefry2x32\n\
{ threefry2x32_rounds = 20 };\n\
inline threefry2x32_ctr_t threefry2x32 (threefry2x32_ctr_t in,\n\
                    threefry2x32_key_t k)\n\
  __attribute__ ((always_inline));\n\
inline threefry2x32_ctr_t\n\
threefry2x32 (threefry2x32_ctr_t in, threefry2x32_key_t k)\n\
{\n\
  return threefry2x32_R (threefry2x32_rounds, in, k);\n\
}\n\
#endif\n\
\n\
";
        ss << "double " << mSymName << "_Random (int seed)\n\
{\n\
  unsigned tid = get_global_id(0);\n\
  threefry2x32_key_t k = { {tid, 0xdecafbad} };\n\
  threefry2x32_ctr_t c = { {seed, 0xf00dcafe} };\n\
  c = threefry2x32_R(threefry2x32_rounds, c, k);\n\
  const double factor = 1./(" << SAL_MAX_UINT32 << ".0 + 1.0);\n\
  const double halffactor = 0.5*factor;\n\
  return c.v[0] * factor + halffactor;\n\
}\n\
";
    }
    virtual size_t GetWindowSize() const override
    {
        return 1;
    }
    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal( cl_kernel k, int argno, int, cl_program ) override
    {
        OpenCLZone zone;
        cl_int seed = comphelper::rng::uniform_int_distribution(0, SAL_MAX_INT32);
        // Pass the scalar result back to the rest of the formula kernel
        SAL_INFO("sc.opencl", "Kernel " << k << " arg " << argno << ": cl_int: " << seed << "(RANDOM)");
        cl_int err = clSetKernelArg(k, argno, sizeof(cl_int), static_cast<void*>(&seed));
        if (CL_SUCCESS != err)
            throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
        return 1;
    }
};

// Arguments that are actually compile-time constant string
class ConstStringArgument : public DynamicKernelArgument
{
public:
    ConstStringArgument( const ScCalcConfig& config, const std::string& s,
        const FormulaTreeNodeRef& ft ) :
        DynamicKernelArgument(config, s, ft) { }
    /// Generate declaration
    virtual void GenDecl( outputstream& ss ) const override
    {
        ss << "double " << mSymName;
    }
    virtual void GenDeclRef( outputstream& ss ) const override
    {
        ss << GenSlidingWindowDeclRef();
    }
    virtual void GenSlidingWindowDecl( outputstream& ss ) const override
    {
        GenDecl(ss);
    }
    virtual std::string GenSlidingWindowDeclRef( bool = false ) const override
    {
        outputstream ss;
        if (GetFormulaToken()->GetType() != formula::svString)
            throw Unhandled(__FILE__, __LINE__);
        FormulaToken* Tok = GetFormulaToken();
        ss << GetStringId(Tok->GetString().getData());
        return ss.str();
    }
    virtual std::string GenIsString( bool = false ) const override
    {
        return "true";
    }
    virtual size_t GetWindowSize() const override
    {
        return 1;
    }
    virtual size_t Marshal( cl_kernel k, int argno, int, cl_program ) override
    {
        FormulaToken* ref = mFormulaTree->GetFormulaToken();
        if (ref->GetType() != formula::svString)
        {
            throw Unhandled(__FILE__, __LINE__);
        }
        cl_double stringId = GetStringId(ref->GetString().getData());

        // Pass the scalar result back to the rest of the formula kernel
        SAL_INFO("sc.opencl", "Kernel " << k << " arg " << argno
            << ": stringId: " << stringId << " (" << DebugPeekData(ref) << ")" );
        cl_int err = clSetKernelArg(k, argno, sizeof(cl_double), static_cast<void*>(&stringId));
        if (CL_SUCCESS != err)
            throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
        return 1;
    }
};

} // namespace

// Marshal a string vector reference
size_t DynamicKernelStringArgument::Marshal( cl_kernel k, int argno, int, cl_program )
{
    OpenCLZone zone;
    FormulaToken* ref = mFormulaTree->GetFormulaToken();

    openclwrapper::KernelEnv kEnv;
    openclwrapper::setKernelEnv(&kEnv);
    cl_int err;
    formula::VectorRefArray vRef;
    size_t nStrings = 0;
    if (ref->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken* pSVR =
            static_cast<const formula::SingleVectorRefToken*>(ref);
        nStrings = pSVR->GetArrayLength();
        vRef = pSVR->GetArray();
    }
    else if (ref->GetType() == formula::svDoubleVectorRef)
    {
        const formula::DoubleVectorRefToken* pDVR =
            static_cast<const formula::DoubleVectorRefToken*>(ref);
        nStrings = pDVR->GetArrayLength();
        vRef = pDVR->GetArrays()[mnIndex];
    }
    size_t szHostBuffer = nStrings * sizeof(cl_double);
    cl_double* pStringIdsBuffer = nullptr;

    if (vRef.mpStringArray != nullptr)
    {
        // Marshal strings. See GetStringId().
        mpClmem = clCreateBuffer(kEnv.mpkContext,
            cl_mem_flags(CL_MEM_READ_ONLY) | CL_MEM_ALLOC_HOST_PTR,
            szHostBuffer, nullptr, &err);
        if (CL_SUCCESS != err)
            throw OpenCLError("clCreateBuffer", err, __FILE__, __LINE__);
        SAL_INFO("sc.opencl", "Created buffer " << mpClmem << " size " << szHostBuffer);

        pStringIdsBuffer = static_cast<cl_double*>(clEnqueueMapBuffer(
            kEnv.mpkCmdQueue, mpClmem, CL_TRUE, CL_MAP_WRITE, 0,
            szHostBuffer, 0, nullptr, nullptr, &err));
        if (CL_SUCCESS != err)
            throw OpenCLError("clEnqueueMapBuffer", err, __FILE__, __LINE__);

        for (size_t i = 0; i < nStrings; i++)
        {
            if (vRef.mpStringArray[i])
                pStringIdsBuffer[i] = GetStringId(vRef.mpStringArray[i]);
            else
                rtl::math::setNan(&pStringIdsBuffer[i]);
        }
    }
    else
    {
        if (nStrings == 0)
            szHostBuffer = sizeof(cl_double); // a dummy small value
                                           // Marshal as a buffer of NANs
        mpClmem = clCreateBuffer(kEnv.mpkContext,
            cl_mem_flags(CL_MEM_READ_ONLY) | CL_MEM_ALLOC_HOST_PTR,
            szHostBuffer, nullptr, &err);
        if (CL_SUCCESS != err)
            throw OpenCLError("clCreateBuffer", err, __FILE__, __LINE__);
        SAL_INFO("sc.opencl", "Created buffer " << mpClmem << " size " << szHostBuffer);

        pStringIdsBuffer = static_cast<cl_double*>(clEnqueueMapBuffer(
            kEnv.mpkCmdQueue, mpClmem, CL_TRUE, CL_MAP_WRITE, 0,
            szHostBuffer, 0, nullptr, nullptr, &err));
        if (CL_SUCCESS != err)
            throw OpenCLError("clEnqueueMapBuffer", err, __FILE__, __LINE__);

        for (size_t i = 0; i < szHostBuffer / sizeof(cl_double); i++)
            rtl::math::setNan(&pStringIdsBuffer[i]);
    }
    err = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue, mpClmem,
        pStringIdsBuffer, 0, nullptr, nullptr);
    if (CL_SUCCESS != err)
        throw OpenCLError("clEnqueueUnmapMemObject", err, __FILE__, __LINE__);

    SAL_INFO("sc.opencl", "Kernel " << k << " arg " << argno << ": cl_mem: " << mpClmem
        << " (stringIds: " << DebugPeekDoubles(pStringIdsBuffer, nStrings) << " "
        << DebugPeekData(ref,mnIndex) << ")");
    err = clSetKernelArg(k, argno, sizeof(cl_mem), static_cast<void*>(&mpClmem));
    if (CL_SUCCESS != err)
        throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
    return 1;
}

std::string DynamicKernelStringArgument::GenIsString( bool nested ) const
{
    if( nested )
        return "!isnan(" + mSymName + "[gid0])";
    FormulaToken* ref = mFormulaTree->GetFormulaToken();
    size_t nStrings = 0;
    if (ref->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken* pSVR =
            static_cast<const formula::SingleVectorRefToken*>(ref);
        nStrings = pSVR->GetArrayLength();
    }
    else if (ref->GetType() == formula::svDoubleVectorRef)
    {
        const formula::DoubleVectorRefToken* pDVR =
            static_cast<const formula::DoubleVectorRefToken*>(ref);
        nStrings = pDVR->GetArrayLength();
    }
    else
        return "!isnan(" + mSymName + "[gid0])";
    outputstream ss;
    ss << "(gid0 < " << nStrings << "? !isnan(" << mSymName << "[gid0]):NAN)";
    return ss.str();
}

namespace {

/// A mixed string/numeric vector
class DynamicKernelMixedArgument : public VectorRef
{
public:
    DynamicKernelMixedArgument( const ScCalcConfig& config, const std::string& s,
        const FormulaTreeNodeRef& ft ) :
        VectorRef(config, s, ft), mStringArgument(config, s + "s", ft) { }
    virtual void GenSlidingWindowDecl( outputstream& ss ) const override
    {
        VectorRef::GenSlidingWindowDecl(ss);
        ss << ", ";
        mStringArgument.GenSlidingWindowDecl(ss);
    }
    virtual void GenSlidingWindowFunction( outputstream& ) override { }
    /// Generate declaration
    virtual void GenDecl( outputstream& ss ) const override
    {
        VectorRef::GenDecl(ss);
        ss << ", ";
        mStringArgument.GenDecl(ss);
    }
    virtual void GenDeclRef( outputstream& ss ) const override
    {
        VectorRef::GenDeclRef(ss);
        ss << ",";
        mStringArgument.GenDeclRef(ss);
    }
    virtual std::string GenSlidingWindowDeclRef( bool nested ) const override
    {
        outputstream ss;
        ss << "(!isnan(" << VectorRef::GenSlidingWindowDeclRef(nested);
        ss << ")?" << VectorRef::GenSlidingWindowDeclRef(nested);
        ss << ":" << mStringArgument.GenSlidingWindowDeclRef(nested);
        ss << ")";
        return ss.str();
    }
    virtual std::string GenDoubleSlidingWindowDeclRef( bool nested = false ) const override
    {
        outputstream ss;
        ss << VectorRef::GenSlidingWindowDeclRef( nested );
        return ss.str();
    }
    virtual std::string GenStringSlidingWindowDeclRef( bool nested = false ) const override
    {
        outputstream ss;
        ss << mStringArgument.GenSlidingWindowDeclRef( nested );
        return ss.str();
    }
    virtual std::string GenIsString( bool nested = false ) const override
    {
        return mStringArgument.GenIsString( nested );
    }
    virtual size_t Marshal( cl_kernel k, int argno, int vw, cl_program p ) override
    {
        int i = VectorRef::Marshal(k, argno, vw, p);
        i += mStringArgument.Marshal(k, argno + i, vw, p);
        return i;
    }

protected:
    DynamicKernelStringArgument mStringArgument;
};

}

template<class Base>
DynamicKernelSlidingArgument<Base>::DynamicKernelSlidingArgument(
    const ScCalcConfig& config, const std::string& s, const FormulaTreeNodeRef& ft,
    std::shared_ptr<SlidingFunctionBase> CodeGen, int index)
    : Base(config, s, ft, index)
    , mpCodeGen(std::move(CodeGen))
{
    FormulaToken* t = ft->GetFormulaToken();
    if (t->GetType() != formula::svDoubleVectorRef)
        throw Unhandled(__FILE__, __LINE__);
    mpDVR = static_cast<const formula::DoubleVectorRefToken*>(t);
    bIsStartFixed = mpDVR->IsStartFixed();
    bIsEndFixed = mpDVR->IsEndFixed();
}

template<class Base>
bool DynamicKernelSlidingArgument<Base>::NeedParallelReduction() const
{
    assert(dynamic_cast<OpSumIfs*>(mpCodeGen.get()));
    return GetWindowSize() > 100 &&
           ((GetStartFixed() && GetEndFixed()) ||
        (!GetStartFixed() && !GetEndFixed()));
}

template<class Base>
std::string DynamicKernelSlidingArgument<Base>::GenSlidingWindowDeclRef( bool nested ) const
{
    size_t nArrayLength = mpDVR->GetArrayLength();
    outputstream ss;
    if (!bIsStartFixed && !bIsEndFixed)
    {
        if (!nested)
            ss << "((i+gid0) <" << nArrayLength << "?";
        ss << Base::GetName() << "[i + gid0]";
        if (!nested)
            ss << ":NAN)";
    }
    else
    {
        if (!nested)
            ss << "(i <" << nArrayLength << "?";
        ss << Base::GetName() << "[i]";
        if (!nested)
            ss << ":NAN)";
    }
    return ss.str();
}

template<class Base>
size_t DynamicKernelSlidingArgument<Base>::GenReductionLoopHeader( outputstream& ss, bool& needBody )
{
    assert(mpDVR);
    size_t nCurWindowSize = mpDVR->GetRefRowSize();

    if (!mpDVR->IsStartFixed() && mpDVR->IsEndFixed())
    {
        ss << "for (int i = ";
        ss << "gid0; i < " << mpDVR->GetArrayLength();
        ss << " && i < " << nCurWindowSize  << "; i++){\n\t\t";
        needBody = true;
        return nCurWindowSize;
    }
    else if (mpDVR->IsStartFixed() && !mpDVR->IsEndFixed())
    {
        ss << "for (int i = ";
        ss << "0; i < " << mpDVR->GetArrayLength();
        ss << " && i < gid0+" << nCurWindowSize << "; i++){\n\t\t";
        needBody = true;
        return nCurWindowSize;
    }
    else if (!mpDVR->IsStartFixed() && !mpDVR->IsEndFixed())
    {
        ss << "tmpBottom = " << mpCodeGen->GetBottom() << ";\n\t";
        ss << "{int i;\n\t";
        outputstream temp1, temp2;
        int outLoopSize = UNROLLING_FACTOR;
        if (nCurWindowSize / outLoopSize != 0)
        {
            ss << "for(int outLoop=0; outLoop<" << nCurWindowSize / outLoopSize << "; outLoop++){\n\t";
            for (int count = 0; count < outLoopSize; count++)
            {
                ss << "i = outLoop*" << outLoopSize << "+" << count << ";\n\t";
                if (count == 0)
                {
                    temp1 << "if(i + gid0 < " << mpDVR->GetArrayLength();
                    temp1 << "){\n\t\t";
                    temp1 << "tmp = legalize(";
                    temp1 <<  mpCodeGen->Gen2(GenSlidingWindowDeclRef(), "tmp");
                    temp1 << ", tmp);\n\t\t\t";
                    temp1 << "}\n\t";
                }
                ss << temp1.str();
            }
            ss << "}\n\t";
        }
        // The residual of mod outLoopSize
        for (size_t count = nCurWindowSize / outLoopSize * outLoopSize; count < nCurWindowSize; count++)
        {
            ss << "i = " << count << ";\n\t";
            if (count == nCurWindowSize / outLoopSize * outLoopSize)
            {
                temp2 << "if(i + gid0 < " << mpDVR->GetArrayLength();
                temp2 << "){\n\t\t";
                temp2 << "tmp = legalize(";
                temp2 << mpCodeGen->Gen2(GenSlidingWindowDeclRef(), "tmp");
                temp2 << ", tmp);\n\t\t\t";
                temp2 << "}\n\t";
            }
            ss << temp2.str();
        }
        ss << "}\n";
        needBody = false;
        return nCurWindowSize;
    }
    // (mpDVR->IsStartFixed() && mpDVR->IsEndFixed())
    else
    {
        ss << "\n\t";
        ss << "tmpBottom = " << mpCodeGen->GetBottom() << ";\n\t";
        ss << "{int i;\n\t";
        outputstream temp1, temp2;
        int outLoopSize = UNROLLING_FACTOR;
        if (nCurWindowSize / outLoopSize != 0)
        {
            ss << "for(int outLoop=0; outLoop<" << nCurWindowSize / outLoopSize << "; outLoop++){\n\t";
            for (int count = 0; count < outLoopSize; count++)
            {
                ss << "i = outLoop*" << outLoopSize << "+" << count << ";\n\t";
                if (count == 0)
                {
                    temp1 << "if(i < " << mpDVR->GetArrayLength();
                    temp1 << "){\n\t\t";
                    temp1 << "tmp = legalize(";
                    temp1 << mpCodeGen->Gen2(GenSlidingWindowDeclRef(), "tmp");
                    temp1 << ", tmp);\n\t\t\t";
                    temp1 << "}\n\t";
                }
                ss << temp1.str();
            }
            ss << "}\n\t";
        }
        // The residual of mod outLoopSize
        for (size_t count = nCurWindowSize / outLoopSize * outLoopSize; count < nCurWindowSize; count++)
        {
            ss << "i = " << count << ";\n\t";
            if (count == nCurWindowSize / outLoopSize * outLoopSize)
            {
                temp2 << "if(i < " << mpDVR->GetArrayLength();
                temp2 << "){\n\t\t";
                temp2 << "tmp = legalize(";
                temp2 << mpCodeGen->Gen2(GenSlidingWindowDeclRef(), "tmp");
                temp2 << ", tmp);\n\t\t\t";
                temp2 << "}\n\t";
            }
            ss << temp2.str();
        }
        ss << "}\n";
        needBody = false;
        return nCurWindowSize;
    }
}

template class DynamicKernelSlidingArgument<VectorRef>;
template class DynamicKernelSlidingArgument<VectorRefStringsToZero>;
template class DynamicKernelSlidingArgument<DynamicKernelStringArgument>;

namespace {

/// A mixed string/numeric vector
class DynamicKernelMixedSlidingArgument : public VectorRef
{
public:
    DynamicKernelMixedSlidingArgument( const ScCalcConfig& config, const std::string& s,
        const FormulaTreeNodeRef& ft, const std::shared_ptr<SlidingFunctionBase>& CodeGen,
        int index ) :
        VectorRef(config, s, ft),
        mDoubleArgument(mCalcConfig, s, ft, CodeGen, index),
        mStringArgument(mCalcConfig, s + "s", ft, CodeGen, index) { }
    virtual void GenSlidingWindowDecl( outputstream& ss ) const override
    {
        mDoubleArgument.GenSlidingWindowDecl(ss);
        ss << ", ";
        mStringArgument.GenSlidingWindowDecl(ss);
    }
    virtual void GenSlidingWindowFunction( outputstream& ) override { }
    /// Generate declaration
    virtual void GenDecl( outputstream& ss ) const override
    {
        mDoubleArgument.GenDecl(ss);
        ss << ", ";
        mStringArgument.GenDecl(ss);
    }
    virtual void GenDeclRef( outputstream& ss ) const override
    {
        mDoubleArgument.GenDeclRef(ss);
        ss << ",";
        mStringArgument.GenDeclRef(ss);
    }
    virtual std::string GenSlidingWindowDeclRef( bool nested ) const override
    {
        outputstream ss;
        ss << "(!isnan(" << mDoubleArgument.GenSlidingWindowDeclRef(nested);
        ss << ")?" << mDoubleArgument.GenSlidingWindowDeclRef(nested);
        ss << ":" << mStringArgument.GenSlidingWindowDeclRef(nested);
        ss << ")";
        return ss.str();
    }
    virtual std::string GenDoubleSlidingWindowDeclRef( bool = false ) const override
    {
        outputstream ss;
        ss << mDoubleArgument.GenSlidingWindowDeclRef();
        return ss.str();
    }
    virtual std::string GenStringSlidingWindowDeclRef( bool = false ) const override
    {
        outputstream ss;
        ss << mStringArgument.GenSlidingWindowDeclRef();
        return ss.str();
    }
    virtual size_t Marshal( cl_kernel k, int argno, int vw, cl_program p ) override
    {
        int i = mDoubleArgument.Marshal(k, argno, vw, p);
        i += mStringArgument.Marshal(k, argno + i, vw, p);
        return i;
    }

protected:
    DynamicKernelSlidingArgument<VectorRef> mDoubleArgument;
    DynamicKernelSlidingArgument<DynamicKernelStringArgument> mStringArgument;
};

/// Holds the symbol table for a given dynamic kernel
class SymbolTable
{
public:
    typedef std::map<const formula::FormulaToken*, DynamicKernelArgumentRef> ArgumentMap;
    // This avoids instability caused by using pointer as the key type
    SymbolTable() : mCurId(0) { }
    template <class T>
    const DynamicKernelArgument* DeclRefArg(const ScCalcConfig& config, const FormulaTreeNodeRef&,
                                            std::shared_ptr<SlidingFunctionBase> pCodeGen, int nResultSize);
    /// Used to generate sliding window helpers
    void DumpSlidingWindowFunctions( outputstream& ss )
    {
        for (auto const& argument : mParams)
        {
            argument->GenSlidingWindowFunction(ss);
            ss << "\n";
        }
    }
    /// Memory mapping from host to device and pass buffers to the given kernel as
    /// arguments
    void Marshal( cl_kernel, int, cl_program );

private:
    unsigned int mCurId;
    ArgumentMap mSymbols;
    std::vector<DynamicKernelArgumentRef> mParams;
};

void SymbolTable::Marshal( cl_kernel k, int nVectorWidth, cl_program pProgram )
{
    int i = 1; //The first argument is reserved for results
    for (auto const& argument : mParams)
    {
        i += argument->Marshal(k, i, nVectorWidth, pProgram);
    }
}

}

template<class Base>
ParallelReductionVectorRef<Base>::ParallelReductionVectorRef(
    const ScCalcConfig& config, const std::string& s, const FormulaTreeNodeRef& ft,
    std::shared_ptr<SlidingFunctionBase> CodeGen, int index)
    : Base(config, s, ft, index)
    , mpCodeGen(std::move(CodeGen))
    , mpClmem2(nullptr)
{
    FormulaToken* t = ft->GetFormulaToken();
    if (t->GetType() != formula::svDoubleVectorRef)
        throw Unhandled(__FILE__, __LINE__);
    mpDVR = static_cast<const formula::DoubleVectorRefToken*>(t);
    bIsStartFixed = mpDVR->IsStartFixed();
    bIsEndFixed = mpDVR->IsEndFixed();
}

template<class Base>
void ParallelReductionVectorRef<Base>::GenSlidingWindowFunction( outputstream& ss )
{
    if (!dynamic_cast<OpAverage*>(mpCodeGen.get()))
    {
        std::string name = Base::GetName();
        ss << "__kernel void " << name;
        ss << "_reduction(__global double* A, "
            "__global double *result,int arrayLength,int windowSize){\n";
        ss << "    double tmp, current_result =" <<
            mpCodeGen->GetBottom();
        ss << ";\n";
        ss << "    int writePos = get_group_id(1);\n";
        ss << "    int lidx = get_local_id(0);\n";
        ss << "    __local double shm_buf[256];\n";
        if (mpDVR->IsStartFixed())
            ss << "    int offset = 0;\n";
        else // if (!mpDVR->IsStartFixed())
            ss << "    int offset = get_group_id(1);\n";
        if (mpDVR->IsStartFixed() && mpDVR->IsEndFixed())
            ss << "    int end = windowSize;\n";
        else if (!mpDVR->IsStartFixed() && !mpDVR->IsEndFixed())
            ss << "    int end = offset + windowSize;\n";
        else if (mpDVR->IsStartFixed() && !mpDVR->IsEndFixed())
            ss << "    int end = windowSize + get_group_id(1);\n";
        else if (!mpDVR->IsStartFixed() && mpDVR->IsEndFixed())
            ss << "    int end = windowSize;\n";
        ss << "    end = min(end, arrayLength);\n";

        ss << "    barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    int loop = arrayLength/512 + 1;\n";
        ss << "    for (int l=0; l<loop; l++){\n";
        ss << "    tmp = " << mpCodeGen->GetBottom() << ";\n";
        ss << "    int loopOffset = l*512;\n";
        ss << "    if((loopOffset + lidx + offset + 256) < end) {\n";
        ss << "        tmp = legalize(" << mpCodeGen->Gen2(
            "A[loopOffset + lidx + offset]", "tmp") << ", tmp);\n";
        ss << "        tmp = legalize(" << mpCodeGen->Gen2(
            "A[loopOffset + lidx + offset + 256]", "tmp") << ", tmp);\n";
        ss << "    } else if ((loopOffset + lidx + offset) < end)\n";
        ss << "        tmp = legalize(" << mpCodeGen->Gen2(
            "A[loopOffset + lidx + offset]", "tmp") << ", tmp);\n";
        ss << "    shm_buf[lidx] = tmp;\n";
        ss << "    barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    for (int i = 128; i >0; i/=2) {\n";
        ss << "        if (lidx < i)\n";
        ss << "            shm_buf[lidx] = ";
        // Special case count
        if (dynamic_cast<OpCount*>(mpCodeGen.get()))
            ss << "shm_buf[lidx] + shm_buf[lidx + i];\n";
        else
            ss << mpCodeGen->Gen2("shm_buf[lidx]", "shm_buf[lidx + i]") << ";\n";
        ss << "        barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    }\n";
        ss << "        if (lidx == 0)\n";
        ss << "            current_result =";
        if (dynamic_cast<OpCount*>(mpCodeGen.get()))
            ss << "current_result + shm_buf[0]";
        else
            ss << mpCodeGen->Gen2("current_result", "shm_buf[0]");
        ss << ";\n";
        ss << "        barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    }\n";
        ss << "    if (lidx == 0)\n";
        ss << "        result[writePos] = current_result;\n";
        ss << "}\n";
    }
    else
    {
        std::string name = Base::GetName();
        /*sum reduction*/
        ss << "__kernel void " << name << "_sum";
        ss << "_reduction(__global double* A, "
            "__global double *result,int arrayLength,int windowSize){\n";
        ss << "    double tmp, current_result =" <<
            mpCodeGen->GetBottom();
        ss << ";\n";
        ss << "    int writePos = get_group_id(1);\n";
        ss << "    int lidx = get_local_id(0);\n";
        ss << "    __local double shm_buf[256];\n";
        if (mpDVR->IsStartFixed())
            ss << "    int offset = 0;\n";
        else // if (!mpDVR->IsStartFixed())
            ss << "    int offset = get_group_id(1);\n";
        if (mpDVR->IsStartFixed() && mpDVR->IsEndFixed())
            ss << "    int end = windowSize;\n";
        else if (!mpDVR->IsStartFixed() && !mpDVR->IsEndFixed())
            ss << "    int end = offset + windowSize;\n";
        else if (mpDVR->IsStartFixed() && !mpDVR->IsEndFixed())
            ss << "    int end = windowSize + get_group_id(1);\n";
        else if (!mpDVR->IsStartFixed() && mpDVR->IsEndFixed())
            ss << "    int end = windowSize;\n";
        ss << "    end = min(end, arrayLength);\n";
        ss << "    barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    int loop = arrayLength/512 + 1;\n";
        ss << "    for (int l=0; l<loop; l++){\n";
        ss << "    tmp = " << mpCodeGen->GetBottom() << ";\n";
        ss << "    int loopOffset = l*512;\n";
        ss << "    if((loopOffset + lidx + offset + 256) < end) {\n";
        ss << "        tmp = legalize(";
        ss << "(A[loopOffset + lidx + offset]+ tmp)";
        ss << ", tmp);\n";
        ss << "        tmp = legalize((A[loopOffset + lidx + offset + 256]+ tmp)";
        ss << ", tmp);\n";
        ss << "    } else if ((loopOffset + lidx + offset) < end)\n";
        ss << "        tmp = legalize((A[loopOffset + lidx + offset] + tmp)";
        ss << ", tmp);\n";
        ss << "    shm_buf[lidx] = tmp;\n";
        ss << "    barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    for (int i = 128; i >0; i/=2) {\n";
        ss << "        if (lidx < i)\n";
        ss << "            shm_buf[lidx] = ";
        ss << "shm_buf[lidx] + shm_buf[lidx + i];\n";
        ss << "        barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    }\n";
        ss << "        if (lidx == 0)\n";
        ss << "            current_result =";
        ss << "current_result + shm_buf[0]";
        ss << ";\n";
        ss << "        barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    }\n";
        ss << "    if (lidx == 0)\n";
        ss << "        result[writePos] = current_result;\n";
        ss << "}\n";
        /*count reduction*/
        ss << "__kernel void " << name << "_count";
        ss << "_reduction(__global double* A, "
            "__global double *result,int arrayLength,int windowSize){\n";
        ss << "    double tmp, current_result =" <<
            mpCodeGen->GetBottom();
        ss << ";\n";
        ss << "    int writePos = get_group_id(1);\n";
        ss << "    int lidx = get_local_id(0);\n";
        ss << "    __local double shm_buf[256];\n";
        if (mpDVR->IsStartFixed())
            ss << "    int offset = 0;\n";
        else // if (!mpDVR->IsStartFixed())
            ss << "    int offset = get_group_id(1);\n";
        if (mpDVR->IsStartFixed() && mpDVR->IsEndFixed())
            ss << "    int end = windowSize;\n";
        else if (!mpDVR->IsStartFixed() && !mpDVR->IsEndFixed())
            ss << "    int end = offset + windowSize;\n";
        else if (mpDVR->IsStartFixed() && !mpDVR->IsEndFixed())
            ss << "    int end = windowSize + get_group_id(1);\n";
        else if (!mpDVR->IsStartFixed() && mpDVR->IsEndFixed())
            ss << "    int end = windowSize;\n";
        ss << "    end = min(end, arrayLength);\n";
        ss << "    barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    int loop = arrayLength/512 + 1;\n";
        ss << "    for (int l=0; l<loop; l++){\n";
        ss << "    tmp = " << mpCodeGen->GetBottom() << ";\n";
        ss << "    int loopOffset = l*512;\n";
        ss << "    if((loopOffset + lidx + offset + 256) < end) {\n";
        ss << "        tmp = legalize((isnan(A[loopOffset + lidx + offset])?tmp:tmp+1.0)";
        ss << ", tmp);\n";
        ss << "        tmp = legalize((isnan(A[loopOffset + lidx + offset+256])?tmp:tmp+1.0)";
        ss << ", tmp);\n";
        ss << "    } else if ((loopOffset + lidx + offset) < end)\n";
        ss << "        tmp = legalize((isnan(A[loopOffset + lidx + offset])?tmp:tmp+1.0)";
        ss << ", tmp);\n";
        ss << "    shm_buf[lidx] = tmp;\n";
        ss << "    barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    for (int i = 128; i >0; i/=2) {\n";
        ss << "        if (lidx < i)\n";
        ss << "            shm_buf[lidx] = ";
        ss << "shm_buf[lidx] + shm_buf[lidx + i];\n";
        ss << "        barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    }\n";
        ss << "        if (lidx == 0)\n";
        ss << "            current_result =";
        ss << "current_result + shm_buf[0];";
        ss << ";\n";
        ss << "        barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    }\n";
        ss << "    if (lidx == 0)\n";
        ss << "        result[writePos] = current_result;\n";
        ss << "}\n";
    }
}

template<class Base>
std::string ParallelReductionVectorRef<Base>::GenSlidingWindowDeclRef( bool ) const
{
    outputstream ss;
    if (!bIsStartFixed && !bIsEndFixed)
        ss << Base::GetName() << "[i + gid0]";
    else
        ss << Base::GetName() << "[i]";
    return ss.str();
}

template<class Base>
size_t ParallelReductionVectorRef<Base>::GenReductionLoopHeader(
    outputstream& ss, int nResultSize, bool& needBody )
{
    assert(mpDVR);
    size_t nCurWindowSize = mpDVR->GetRefRowSize();
    std::string temp = Base::GetName() + "[gid0]";
    ss << "tmp = ";
    // Special case count
    if (dynamic_cast<OpAverage*>(mpCodeGen.get()))
    {
        ss << mpCodeGen->Gen2(temp, "tmp") << ";\n";
        ss << "nCount = nCount-1;\n";
        ss << "nCount = nCount +"; /*re-assign nCount from count reduction*/
        ss << Base::GetName() << "[gid0+" << nResultSize << "]" << ";\n";
    }
    else if (dynamic_cast<OpCount*>(mpCodeGen.get()))
        ss << temp << "+ tmp";
    else
        ss << mpCodeGen->Gen2(temp, "tmp");
    ss << ";\n\t";
    needBody = false;
    return nCurWindowSize;
}

template<class Base>
size_t ParallelReductionVectorRef<Base>::Marshal( cl_kernel k, int argno, int w, cl_program mpProgram )
{
    assert(Base::mpClmem == nullptr);

    OpenCLZone zone;
    openclwrapper::KernelEnv kEnv;
    openclwrapper::setKernelEnv(&kEnv);
    cl_int err;
    size_t nInput = mpDVR->GetArrayLength();
    size_t nCurWindowSize = mpDVR->GetRefRowSize();
    // create clmem buffer
    if (mpDVR->GetArrays()[Base::mnIndex].mpNumericArray == nullptr)
        throw Unhandled(__FILE__, __LINE__);
    double* pHostBuffer = const_cast<double*>(
        mpDVR->GetArrays()[Base::mnIndex].mpNumericArray);
    size_t szHostBuffer = nInput * sizeof(double);
    Base::mpClmem = clCreateBuffer(kEnv.mpkContext,
        cl_mem_flags(CL_MEM_READ_ONLY) | CL_MEM_USE_HOST_PTR,
        szHostBuffer,
        pHostBuffer, &err);
    SAL_INFO("sc.opencl", "Created buffer " << Base::mpClmem << " size " << nInput << "*" << sizeof(double) << "=" << szHostBuffer << " using host buffer " << pHostBuffer);

    mpClmem2 = clCreateBuffer(kEnv.mpkContext,
        CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
        sizeof(double) * w, nullptr, nullptr);
    if (CL_SUCCESS != err)
        throw OpenCLError("clCreateBuffer", err, __FILE__, __LINE__);
    SAL_INFO("sc.opencl", "Created buffer " << mpClmem2 << " size " << sizeof(double) << "*" << w << "=" << (sizeof(double)*w));

    // reproduce the reduction function name
    std::string kernelName;
    if (!dynamic_cast<OpAverage*>(mpCodeGen.get()))
        kernelName = Base::GetName() + "_reduction";
    else
        kernelName = Base::GetName() + "_sum_reduction";
    cl_kernel redKernel = clCreateKernel(mpProgram, kernelName.c_str(), &err);
    if (err != CL_SUCCESS)
        throw OpenCLError("clCreateKernel", err, __FILE__, __LINE__);
    SAL_INFO("sc.opencl", "Created kernel " << redKernel << " with name " << kernelName << " in program " << mpProgram);

    // set kernel arg of reduction kernel
    // TODO(Wei Wei): use unique name for kernel
    cl_mem buf = Base::GetCLBuffer();
    SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << 0 << ": cl_mem: " << buf);
    err = clSetKernelArg(redKernel, 0, sizeof(cl_mem),
        static_cast<void*>(&buf));
    if (CL_SUCCESS != err)
        throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);

    SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << 1 << ": cl_mem: " << mpClmem2);
    err = clSetKernelArg(redKernel, 1, sizeof(cl_mem), &mpClmem2);
    if (CL_SUCCESS != err)
        throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);

    SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << 2 << ": cl_int: " << nInput);
    err = clSetKernelArg(redKernel, 2, sizeof(cl_int), static_cast<void*>(&nInput));
    if (CL_SUCCESS != err)
        throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);

    SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << 3 << ": cl_int: " << nCurWindowSize);
    err = clSetKernelArg(redKernel, 3, sizeof(cl_int), static_cast<void*>(&nCurWindowSize));
    if (CL_SUCCESS != err)
        throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);

    // set work group size and execute
    size_t global_work_size[] = { 256, static_cast<size_t>(w) };
    size_t const local_work_size[] = { 256, 1 };
    SAL_INFO("sc.opencl", "Enqueuing kernel " << redKernel);
    err = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, redKernel, 2, nullptr,
        global_work_size, local_work_size, 0, nullptr, nullptr);
    if (CL_SUCCESS != err)
        throw OpenCLError("clEnqueueNDRangeKernel", err, __FILE__, __LINE__);
    err = clFinish(kEnv.mpkCmdQueue);
    if (CL_SUCCESS != err)
        throw OpenCLError("clFinish", err, __FILE__, __LINE__);
    if (dynamic_cast<OpAverage*>(mpCodeGen.get()))
    {
        /*average need more reduction kernel for count computing*/
        std::unique_ptr<double[]> pAllBuffer(new double[2 * w]);
        double* resbuf = static_cast<double*>(clEnqueueMapBuffer(kEnv.mpkCmdQueue,
            mpClmem2,
            CL_TRUE, CL_MAP_READ, 0,
            sizeof(double) * w, 0, nullptr, nullptr,
            &err));
        if (err != CL_SUCCESS)
            throw OpenCLError("clEnqueueMapBuffer", err, __FILE__, __LINE__);

        for (int i = 0; i < w; i++)
            pAllBuffer[i] = resbuf[i];
        err = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue, mpClmem2, resbuf, 0, nullptr, nullptr);
        if (err != CL_SUCCESS)
            throw OpenCLError("clEnqueueUnmapMemObject", err, __FILE__, __LINE__);

        kernelName = Base::GetName() + "_count_reduction";
        redKernel = clCreateKernel(mpProgram, kernelName.c_str(), &err);
        if (err != CL_SUCCESS)
            throw OpenCLError("clCreateKernel", err, __FILE__, __LINE__);
        SAL_INFO("sc.opencl", "Created kernel " << redKernel << " with name " << kernelName << " in program " << mpProgram);

        // set kernel arg of reduction kernel
        buf = Base::GetCLBuffer();
        SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << 0 << ": cl_mem: " << buf);
        err = clSetKernelArg(redKernel, 0, sizeof(cl_mem),
            static_cast<void*>(&buf));
        if (CL_SUCCESS != err)
            throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);

        SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << 1 << ": cl_mem: " << mpClmem2);
        err = clSetKernelArg(redKernel, 1, sizeof(cl_mem), &mpClmem2);
        if (CL_SUCCESS != err)
            throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);

        SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << 2 << ": cl_int: " << nInput);
        err = clSetKernelArg(redKernel, 2, sizeof(cl_int), static_cast<void*>(&nInput));
        if (CL_SUCCESS != err)
            throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);

        SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << 3 << ": cl_int: " << nCurWindowSize);
        err = clSetKernelArg(redKernel, 3, sizeof(cl_int), static_cast<void*>(&nCurWindowSize));
        if (CL_SUCCESS != err)
            throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);

        // set work group size and execute
        size_t global_work_size1[] = { 256, static_cast<size_t>(w) };
        size_t const local_work_size1[] = { 256, 1 };
        SAL_INFO("sc.opencl", "Enqueuing kernel " << redKernel);
        err = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, redKernel, 2, nullptr,
            global_work_size1, local_work_size1, 0, nullptr, nullptr);
        if (CL_SUCCESS != err)
            throw OpenCLError("clEnqueueNDRangeKernel", err, __FILE__, __LINE__);
        err = clFinish(kEnv.mpkCmdQueue);
        if (CL_SUCCESS != err)
            throw OpenCLError("clFinish", err, __FILE__, __LINE__);
        resbuf = static_cast<double*>(clEnqueueMapBuffer(kEnv.mpkCmdQueue,
            mpClmem2,
            CL_TRUE, CL_MAP_READ, 0,
            sizeof(double) * w, 0, nullptr, nullptr,
            &err));
        if (err != CL_SUCCESS)
            throw OpenCLError("clEnqueueMapBuffer", err, __FILE__, __LINE__);
        for (int i = 0; i < w; i++)
            pAllBuffer[i + w] = resbuf[i];
        err = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue, mpClmem2, resbuf, 0, nullptr, nullptr);
        // FIXME: Is it intentional to not throw an OpenCLError even if the clEnqueueUnmapMemObject() fails?
        if (CL_SUCCESS != err)
            SAL_WARN("sc.opencl", "clEnqueueUnmapMemObject failed: " << openclwrapper::errorString(err));
        if (mpClmem2)
        {
            err = clReleaseMemObject(mpClmem2);
            SAL_WARN_IF(err != CL_SUCCESS, "sc.opencl", "clReleaseMemObject failed: " << openclwrapper::errorString(err));
            mpClmem2 = nullptr;
        }
        mpClmem2 = clCreateBuffer(kEnv.mpkContext,
            cl_mem_flags(CL_MEM_READ_WRITE) | CL_MEM_COPY_HOST_PTR,
            w * sizeof(double) * 2, pAllBuffer.get(), &err);
        if (CL_SUCCESS != err)
            throw OpenCLError("clCreateBuffer", err, __FILE__, __LINE__);
        SAL_INFO("sc.opencl", "Created buffer " << mpClmem2 << " size " << w << "*" << sizeof(double) << "=" << (w*sizeof(double)) << " copying host buffer " << pAllBuffer.get());
    }
    // set kernel arg
    SAL_INFO("sc.opencl", "Kernel " << k << " arg " << argno << ": cl_mem: " << mpClmem2);
    err = clSetKernelArg(k, argno, sizeof(cl_mem), &mpClmem2);
    if (CL_SUCCESS != err)
        throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
    return 1;
}

template<class Base>
ParallelReductionVectorRef<Base>::~ParallelReductionVectorRef()
{
    if (mpClmem2)
    {
        cl_int err;
        err = clReleaseMemObject(mpClmem2);
        SAL_WARN_IF(err != CL_SUCCESS, "sc.opencl", "clReleaseMemObject failed: " << openclwrapper::errorString(err));
        mpClmem2 = nullptr;
    }
}

template class ParallelReductionVectorRef<VectorRef>;

namespace {

struct SumIfsArgs
{
    explicit SumIfsArgs(cl_mem x) : mCLMem(x), mConst(0.0) { }
    explicit SumIfsArgs(double x) : mCLMem(nullptr), mConst(x) { }
    cl_mem mCLMem;
    double mConst;
};

/// Helper functions that have multiple buffers
class DynamicKernelSoPArguments : public DynamicKernelArgument
{
public:
    typedef std::vector<DynamicKernelArgumentRef> SubArgumentsType;

    DynamicKernelSoPArguments( const ScCalcConfig& config,
        const std::string& s, const FormulaTreeNodeRef& ft,
        std::shared_ptr<SlidingFunctionBase> pCodeGen, int nResultSize );

    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal( cl_kernel k, int argno, int nVectorWidth, cl_program pProgram ) override
    {
        OpenCLZone zone;
        unsigned i = 0;
        for (const auto& rxSubArgument : mvSubArguments)
        {
            i += rxSubArgument->Marshal(k, argno + i, nVectorWidth, pProgram);
        }
        if (OpSumIfs* OpSumCodeGen = dynamic_cast<OpSumIfs*>(mpCodeGen.get()))
        {
            openclwrapper::KernelEnv kEnv;
            openclwrapper::setKernelEnv(&kEnv);
            cl_int err;
            DynamicKernelArgument* Arg = mvSubArguments[0].get();
            DynamicKernelSlidingArgument<VectorRef>* slidingArgPtr =
                static_cast<DynamicKernelSlidingArgument<VectorRef>*>(Arg);
            mpClmem2 = nullptr;

            if (OpSumCodeGen->NeedReductionKernel())
            {
                size_t nInput = slidingArgPtr->GetArrayLength();
                size_t nCurWindowSize = slidingArgPtr->GetWindowSize();
                std::vector<SumIfsArgs> vclmem;

                for (const auto& rxSubArgument : mvSubArguments)
                {
                    if (VectorRef* VR = dynamic_cast<VectorRef*>(rxSubArgument.get()))
                        vclmem.emplace_back(VR->GetCLBuffer());
                    else if (DynamicKernelConstantArgument* CA = dynamic_cast<DynamicKernelConstantArgument*>(rxSubArgument.get()))
                        vclmem.emplace_back(CA->GetDouble());
                    else
                        vclmem.emplace_back(nullptr);
                }
                mpClmem2 = clCreateBuffer(kEnv.mpkContext, CL_MEM_READ_WRITE,
                    sizeof(double) * nVectorWidth, nullptr, &err);
                if (CL_SUCCESS != err)
                    throw OpenCLError("clCreateBuffer", err, __FILE__, __LINE__);
                SAL_INFO("sc.opencl", "Created buffer " << mpClmem2 << " size " << sizeof(double) << "*" << nVectorWidth << "=" << (sizeof(double)*nVectorWidth));

                std::string kernelName = mvSubArguments[0]->GetName() + "_SumIfs_reduction";
                cl_kernel redKernel = clCreateKernel(pProgram, kernelName.c_str(), &err);
                if (err != CL_SUCCESS)
                    throw OpenCLError("clCreateKernel", err, __FILE__, __LINE__);
                SAL_INFO("sc.opencl", "Created kernel " << redKernel << " with name " << kernelName << " in program " << pProgram);

                // set kernel arg of reduction kernel
                for (size_t j = 0; j < vclmem.size(); j++)
                {
                    if (vclmem[j].mCLMem)
                        SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << j << ": cl_mem: " << vclmem[j].mCLMem);
                    else
                        SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << j << ": double: " << preciseFloat( vclmem[j].mConst ));
                    err = clSetKernelArg(redKernel, j,
                        vclmem[j].mCLMem ? sizeof(cl_mem) : sizeof(double),
                        vclmem[j].mCLMem ? static_cast<void*>(&vclmem[j].mCLMem) :
                                           static_cast<void*>(&vclmem[j].mConst));
                    if (CL_SUCCESS != err)
                        throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
                }
                SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << vclmem.size() << ": cl_mem: " << mpClmem2);
                err = clSetKernelArg(redKernel, vclmem.size(), sizeof(cl_mem), static_cast<void*>(&mpClmem2));
                if (CL_SUCCESS != err)
                    throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);

                SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << (vclmem.size() + 1) << ": cl_int: " << nInput);
                err = clSetKernelArg(redKernel, vclmem.size() + 1, sizeof(cl_int), static_cast<void*>(&nInput));
                if (CL_SUCCESS != err)
                    throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);

                SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << (vclmem.size() + 2) << ": cl_int: " << nCurWindowSize);
                err = clSetKernelArg(redKernel, vclmem.size() + 2, sizeof(cl_int), static_cast<void*>(&nCurWindowSize));
                if (CL_SUCCESS != err)
                    throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
                // set work group size and execute
                size_t global_work_size[] = { 256, static_cast<size_t>(nVectorWidth) };
                size_t const local_work_size[] = { 256, 1 };
                SAL_INFO("sc.opencl", "Enqueuing kernel " << redKernel);
                err = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, redKernel, 2, nullptr,
                    global_work_size, local_work_size, 0, nullptr, nullptr);
                if (CL_SUCCESS != err)
                    throw OpenCLError("clEnqueueNDRangeKernel", err, __FILE__, __LINE__);

                err = clFinish(kEnv.mpkCmdQueue);
                if (CL_SUCCESS != err)
                    throw OpenCLError("clFinish", err, __FILE__, __LINE__);

                SAL_INFO("sc.opencl", "Releasing kernel " << redKernel);
                err = clReleaseKernel(redKernel);
                SAL_WARN_IF(err != CL_SUCCESS, "sc.opencl", "clReleaseKernel failed: " << openclwrapper::errorString(err));

                // Pass mpClmem2 to the "real" kernel
                SAL_INFO("sc.opencl", "Kernel " << k << " arg " << argno << ": cl_mem: " << mpClmem2);
                err = clSetKernelArg(k, argno, sizeof(cl_mem), static_cast<void*>(&mpClmem2));
                if (CL_SUCCESS != err)
                    throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
            }
        }
        return i;
    }

    virtual void GenSlidingWindowFunction( outputstream& ss ) override
    {
        for (DynamicKernelArgumentRef & rArg : mvSubArguments)
            rArg->GenSlidingWindowFunction(ss);
        mpCodeGen->GenSlidingWindowFunction(ss, mSymName, mvSubArguments);
    }
    virtual void GenDeclRef( outputstream& ss ) const override
    {
        for (size_t i = 0; i < mvSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            mvSubArguments[i]->GenDeclRef(ss);
        }
    }
    virtual void GenDecl( outputstream& ss ) const override
    {
        for (SubArgumentsType::const_iterator it = mvSubArguments.begin(), e = mvSubArguments.end(); it != e;
            ++it)
        {
            if (it != mvSubArguments.begin())
                ss << ", ";
            (*it)->GenDecl(ss);
        }
    }

    virtual size_t GetWindowSize() const override
    {
        size_t nCurWindowSize = 0;
        for (const auto & rSubArgument : mvSubArguments)
        {
            size_t nCurChildWindowSize = rSubArgument->GetWindowSize();
            nCurWindowSize = (nCurWindowSize < nCurChildWindowSize) ?
                nCurChildWindowSize : nCurWindowSize;
        }
        return nCurWindowSize;
    }

    /// When declared as input to a sliding window function
    virtual void GenSlidingWindowDecl( outputstream& ss ) const override
    {
        for (SubArgumentsType::const_iterator it = mvSubArguments.begin(), e = mvSubArguments.end(); it != e;
            ++it)
        {
            if (it != mvSubArguments.begin())
                ss << ", ";
            (*it)->GenSlidingWindowDecl(ss);
        }
    }
    /// Generate either a function call to each children
    /// or directly inline it if we are already inside a loop
    virtual std::string GenSlidingWindowDeclRef( bool nested = false ) const override
    {
        outputstream ss;
        if (!nested)
        {
            ss << mSymName << "_" << mpCodeGen->BinFuncName() << "(";
            for (size_t i = 0; i < mvSubArguments.size(); i++)
            {
                if (i)
                    ss << ", ";
                mvSubArguments[i]->GenDeclRef(ss);
            }
            ss << ")";
        }
        else
        {
            if (mvSubArguments.size() != 2)
                throw Unhandled(__FILE__, __LINE__);
            bool bArgument1_NeedNested =
                mvSubArguments[0]->GetFormulaToken()->GetType()
                != formula::svSingleVectorRef;
            bool bArgument2_NeedNested =
                mvSubArguments[1]->GetFormulaToken()->GetType()
                != formula::svSingleVectorRef;
            ss << "(";
            ss << mpCodeGen->
                Gen2(mvSubArguments[0]
                ->GenSlidingWindowDeclRef(bArgument1_NeedNested),
                mvSubArguments[1]
                ->GenSlidingWindowDeclRef(bArgument2_NeedNested));
            ss << ")";
        }
        return ss.str();
    }
    virtual std::string DumpOpName() const override
    {
        std::string t = "_" + mpCodeGen->BinFuncName();
        for (const auto & rSubArgument : mvSubArguments)
            t += rSubArgument->DumpOpName();
        return t;
    }
    virtual void DumpInlineFun( std::set<std::string>& decls,
        std::set<std::string>& funs ) const override
    {
        mpCodeGen->BinInlineFun(decls, funs);
        for (const auto & rSubArgument : mvSubArguments)
            rSubArgument->DumpInlineFun(decls, funs);
    }
    virtual bool IsEmpty() const override
    {
        for (const auto & rSubArgument : mvSubArguments)
            if( !rSubArgument->IsEmpty())
                return false;
        return true;
    }
    virtual ~DynamicKernelSoPArguments() override
    {
        if (mpClmem2)
        {
            cl_int err;
            err = clReleaseMemObject(mpClmem2);
            SAL_WARN_IF(err != CL_SUCCESS, "sc.opencl", "clReleaseMemObject failed: " << openclwrapper::errorString(err));
            mpClmem2 = nullptr;
        }
    }

private:
    SubArgumentsType mvSubArguments;
    std::shared_ptr<SlidingFunctionBase> mpCodeGen;
    cl_mem mpClmem2;
};

}

static DynamicKernelArgumentRef SoPHelper( const ScCalcConfig& config,
    const std::string& ts, const FormulaTreeNodeRef& ft, std::shared_ptr<SlidingFunctionBase> pCodeGen,
    int nResultSize )
{
    return std::make_shared<DynamicKernelSoPArguments>(config, ts, ft, std::move(pCodeGen), nResultSize);
}

template<class Base>
static std::shared_ptr<DynamicKernelArgument> VectorRefFactory( const ScCalcConfig& config, const std::string& s,
    const FormulaTreeNodeRef& ft,
    std::shared_ptr<SlidingFunctionBase>& pCodeGen,
    int index )
{
    //Black lists ineligible classes here ..
    // SUMIFS does not perform parallel reduction at DoubleVectorRef level
    if (dynamic_cast<OpSumIfs*>(pCodeGen.get()))
    {
        // coverity[identical_branches] - only identical if Base happens to be VectorRef
        if (index == 0) // the first argument of OpSumIfs cannot be strings anyway
            return std::make_shared<DynamicKernelSlidingArgument<VectorRef>>(config, s, ft, pCodeGen, index);
        return std::make_shared<DynamicKernelSlidingArgument<Base>>(config, s, ft, pCodeGen, index);
    }
    // AVERAGE is not supported yet
    //Average has been supported by reduction kernel
    /*else if (dynamic_cast<OpAverage*>(pCodeGen.get()))
    {
        return new DynamicKernelSlidingArgument<Base>(config, s, ft, pCodeGen, index);
    }*/
    // MUL is not supported yet
    else if (dynamic_cast<OpMul*>(pCodeGen.get()))
    {
        return std::make_shared<DynamicKernelSlidingArgument<Base>>(config, s, ft, pCodeGen, index);
    }
    // Sub is not a reduction per se
    else if (dynamic_cast<OpSub*>(pCodeGen.get()))
    {
        return std::make_shared<DynamicKernelSlidingArgument<Base>>(config, s, ft, pCodeGen, index);
    }
    // Only child class of Reduction is supported
    else if (!dynamic_cast<Reduction*>(pCodeGen.get()))
    {
        return std::make_shared<DynamicKernelSlidingArgument<Base>>(config, s, ft, pCodeGen, index);
    }

    const formula::DoubleVectorRefToken* pDVR =
        static_cast<const formula::DoubleVectorRefToken*>(
        ft->GetFormulaToken());
    // Window being too small to justify a parallel reduction
    if (pDVR->GetRefRowSize() < REDUCE_THRESHOLD)
        return std::make_shared<DynamicKernelSlidingArgument<Base>>(config, s, ft, pCodeGen, index);
    if (pDVR->IsStartFixed() == pDVR->IsEndFixed())
        return std::make_shared<ParallelReductionVectorRef<Base>>(config, s, ft, pCodeGen, index);
    else // Other cases are not supported as well
        return std::make_shared<DynamicKernelSlidingArgument<Base>>(config, s, ft, pCodeGen, index);
}

DynamicKernelSoPArguments::DynamicKernelSoPArguments(const ScCalcConfig& config,
    const std::string& s, const FormulaTreeNodeRef& ft, std::shared_ptr<SlidingFunctionBase> pCodeGen, int nResultSize ) :
    DynamicKernelArgument(config, s, ft), mpCodeGen(pCodeGen), mpClmem2(nullptr)
{
    size_t nChildren = ft->Children.size();

    for (size_t i = 0; i < nChildren; i++)
    {
        FormulaTreeNodeRef rChild = ft->Children[i];
        if (!rChild)
            throw Unhandled(__FILE__, __LINE__);
        FormulaToken* pChild = rChild->GetFormulaToken();
        if (!pChild)
            throw Unhandled(__FILE__, __LINE__);
        OpCode opc = pChild->GetOpCode();
        outputstream tmpname;
        tmpname << s << "_" << i;
        std::string ts = tmpname.str();
        switch (opc)
        {
            case ocPush:
                if (pChild->GetType() == formula::svDoubleVectorRef)
                {
                    const formula::DoubleVectorRefToken* pDVR =
                        static_cast<const formula::DoubleVectorRefToken*>(pChild);

                    // The code below will split one svDoubleVectorRef into one subargument
                    // for each column of data, and then all these subarguments will be later
                    // passed to the code generating the function. Most of the code then
                    // simply treats each subargument as one argument to the function, and thus
                    // could break in this case.
                    // As a simple solution, simply prevent this case, unless the code in question
                    // explicitly claims it will handle this situation properly.
                    if( pDVR->GetArrays().size() > 1 )
                    {
                        if( !pCodeGen->canHandleMultiVector())
                            throw UnhandledToken(("Function '" + pCodeGen->BinFuncName()
                                + "' cannot handle multi-column DoubleRef").c_str(), __FILE__, __LINE__);

                        SAL_INFO("sc.opencl", "multi-column DoubleRef");

                    }

                    // FIXME: The Right Thing to do would be to compare the accumulated kernel
                    // parameter size against the CL_DEVICE_MAX_PARAMETER_SIZE of the device, but
                    // let's just do this sanity check for now. The kernel compilation will
                    // hopefully fail anyway if the size of parameters exceeds the limit and this
                    // sanity check is just to make us bail out a bit earlier.

                    // The number 50 comes from the fact that the minimum size of
                    // CL_DEVICE_MAX_PARAMETER_SIZE is 256, which for 32-bit code probably means 64
                    // of them. Round down a bit.

                    if (pDVR->GetArrays().size() > 50)
                        throw UnhandledToken(("Kernel would have ridiculously many parameters (" + std::to_string(2 + pDVR->GetArrays().size()) + ")").c_str(), __FILE__, __LINE__);

                    for (size_t j = 0; j < pDVR->GetArrays().size(); ++j)
                    {
                        SAL_INFO("sc.opencl", "i=" << i << " j=" << j <<
                                 " mpNumericArray=" << pDVR->GetArrays()[j].mpNumericArray <<
                                 " mpStringArray=" << pDVR->GetArrays()[j].mpStringArray <<
                                 " allStringsAreNull=" << (AllStringsAreNull(pDVR->GetArrays()[j].mpStringArray, pDVR->GetArrayLength())?"YES":"NO") <<
                                 " takeNumeric=" << (pCodeGen->takeNumeric()?"YES":"NO") <<
                                 " takeString=" << (pCodeGen->takeString()?"YES":"NO"));

                        if (pDVR->GetArrays()[j].mpNumericArray &&
                            pCodeGen->takeNumeric() &&
                            pDVR->GetArrays()[j].mpStringArray &&
                            pCodeGen->takeString())
                        {
                            // Function takes numbers or strings, there are both
                            SAL_INFO("sc.opencl", "Numbers and strings");
                            mvSubArguments.push_back(
                                std::make_shared<DynamicKernelMixedSlidingArgument>(mCalcConfig,
                                        ts, ft->Children[i], mpCodeGen, j));
                        }
                        else if (pDVR->GetArrays()[j].mpNumericArray &&
                            pCodeGen->takeNumeric() &&
                                 (AllStringsAreNull(pDVR->GetArrays()[j].mpStringArray, pDVR->GetArrayLength())
                                    || mCalcConfig.meStringConversion == ScCalcConfig::StringConversion::ZERO
                                    || pCodeGen->forceStringsToZero()))
                        {
                            // Function takes numbers, and either there
                            // are no strings, or there are strings but
                            // they are to be treated as zero
                            SAL_INFO("sc.opencl", "Numbers (no strings or strings treated as zero)");
                            if(!AllStringsAreNull(pDVR->GetArrays()[j].mpStringArray, pDVR->GetArrayLength()))
                            {
                                mvSubArguments.push_back(
                                    VectorRefFactory<VectorRefStringsToZero>(mCalcConfig,
                                            ts, ft->Children[i], mpCodeGen, j));
                            }
                            else
                            {
                                mvSubArguments.push_back(
                                    VectorRefFactory<VectorRef>(mCalcConfig,
                                            ts, ft->Children[i], mpCodeGen, j));
                            }
                        }
                        else if (pDVR->GetArrays()[j].mpNumericArray == nullptr &&
                            pCodeGen->takeNumeric() &&
                            pDVR->GetArrays()[j].mpStringArray &&
                            ( mCalcConfig.meStringConversion == ScCalcConfig::StringConversion::ZERO
                                || pCodeGen->forceStringsToZero()))
                        {
                            // Function takes numbers, and there are only
                            // strings, but they are to be treated as zero
                            SAL_INFO("sc.opencl", "Only strings even if want numbers but should be treated as zero");
                            mvSubArguments.push_back(
                                VectorRefFactory<VectorRefStringsToZero>(mCalcConfig,
                                        ts, ft->Children[i], mpCodeGen, j));
                        }
                        else if (pDVR->GetArrays()[j].mpStringArray &&
                            pCodeGen->takeString())
                        {
                            // There are strings, and the function takes strings.
                            SAL_INFO("sc.opencl", "Strings only");
                            mvSubArguments.push_back(
                                VectorRefFactory
                                    <DynamicKernelStringArgument>(mCalcConfig,
                                        ts, ft->Children[i], mpCodeGen, j));
                        }
                        else if (AllStringsAreNull(pDVR->GetArrays()[j].mpStringArray, pDVR->GetArrayLength()) &&
                            pDVR->GetArrays()[j].mpNumericArray == nullptr)
                        {
                            // There are only empty cells. Push as an
                            // array of NANs
                            SAL_INFO("sc.opencl", "Only empty cells");
                            mvSubArguments.push_back(
                                VectorRefFactory<VectorRef>(mCalcConfig,
                                        ts, ft->Children[i], mpCodeGen, j));
                        }
                        else
                        {
                            SAL_INFO("sc.opencl", "Unhandled case, rejecting for OpenCL");
                            throw UnhandledToken(("Unhandled numbers/strings combination for '"
                                + pCodeGen->BinFuncName() + "'").c_str(), __FILE__, __LINE__);
                        }
                    }
                }
                else if (pChild->GetType() == formula::svSingleVectorRef)
                {
                    const formula::SingleVectorRefToken* pSVR =
                        static_cast<const formula::SingleVectorRefToken*>(pChild);

                    SAL_INFO("sc.opencl", "i=" << i <<
                             " mpNumericArray=" << pSVR->GetArray().mpNumericArray <<
                             " mpStringArray=" << pSVR->GetArray().mpStringArray <<
                             " allStringsAreNull=" << (AllStringsAreNull(pSVR->GetArray().mpStringArray, pSVR->GetArrayLength())?"YES":"NO") <<
                             " takeNumeric=" << (pCodeGen->takeNumeric()?"YES":"NO") <<
                             " takeString=" << (pCodeGen->takeString()?"YES":"NO"));

                    if (pSVR->GetArray().mpNumericArray &&
                        pCodeGen->takeNumeric() &&
                        pSVR->GetArray().mpStringArray &&
                        pCodeGen->takeString())
                    {
                        // Function takes numbers or strings, there are both
                        SAL_INFO("sc.opencl", "Numbers and strings");
                        mvSubArguments.push_back(
                            std::make_shared<DynamicKernelMixedArgument>(mCalcConfig,
                                    ts, ft->Children[i]));
                    }
                    else if (pSVR->GetArray().mpNumericArray &&
                        pCodeGen->takeNumeric() &&
                             (AllStringsAreNull(pSVR->GetArray().mpStringArray, pSVR->GetArrayLength())
                                || mCalcConfig.meStringConversion == ScCalcConfig::StringConversion::ZERO
                                || pCodeGen->forceStringsToZero()))
                    {
                        // Function takes numbers, and either there
                        // are no strings, or there are strings but
                        // they are to be treated as zero
                        SAL_INFO("sc.opencl", "Numbers (no strings or strings treated as zero)");
                        if( !AllStringsAreNull(pSVR->GetArray().mpStringArray, pSVR->GetArrayLength()))
                            mvSubArguments.push_back(
                                std::make_shared<VectorRefStringsToZero>(mCalcConfig, ts,
                                        ft->Children[i]));
                        else
                            mvSubArguments.push_back(
                                std::make_shared<VectorRef>(mCalcConfig, ts,
                                        ft->Children[i]));
                    }
                    else if (pSVR->GetArray().mpNumericArray == nullptr &&
                        pCodeGen->takeNumeric() &&
                        pSVR->GetArray().mpStringArray &&
                        (mCalcConfig.meStringConversion == ScCalcConfig::StringConversion::ZERO
                            || pCodeGen->forceStringsToZero()))
                    {
                        // Function takes numbers, and there are only
                        // strings, but they are to be treated as zero
                        SAL_INFO("sc.opencl", "Only strings even if want numbers but should be treated as zero");
                        mvSubArguments.push_back(
                            std::make_shared<VectorRefStringsToZero>(mCalcConfig, ts,
                                    ft->Children[i]));
                    }
                    else if (pSVR->GetArray().mpStringArray &&
                        pCodeGen->takeString())
                    {
                        // There are strings, and the function takes strings.
                        SAL_INFO("sc.opencl", "Strings only");
                        mvSubArguments.push_back(
                            std::make_shared<DynamicKernelStringArgument>(mCalcConfig,
                                    ts, ft->Children[i]));
                    }
                    else if (AllStringsAreNull(pSVR->GetArray().mpStringArray, pSVR->GetArrayLength()) &&
                        pSVR->GetArray().mpNumericArray == nullptr)
                    {
                        // There are only empty cells. Push as an
                        // array of NANs
                        SAL_INFO("sc.opencl", "Only empty cells");
                        mvSubArguments.push_back(
                            std::make_shared<VectorRef>(mCalcConfig, ts,
                                    ft->Children[i]));
                    }
                    else
                    {
                        SAL_INFO("sc.opencl", "Unhandled case, rejecting for OpenCL");
                        throw UnhandledToken(("Unhandled numbers/strings combination for '"
                            + pCodeGen->BinFuncName() + "'").c_str(), __FILE__, __LINE__);
                    }
                }
                else if (pChild->GetType() == formula::svDouble)
                {
                    SAL_INFO("sc.opencl", "Constant number case");
                    mvSubArguments.push_back(
                        std::make_shared<DynamicKernelConstantArgument>(mCalcConfig, ts,
                                ft->Children[i]));
                }
                else if (pChild->GetType() == formula::svString
                    && pCodeGen->takeString())
                {
                    SAL_INFO("sc.opencl", "Constant string case");
                    mvSubArguments.push_back(
                        std::make_shared<ConstStringArgument>(mCalcConfig, ts,
                                ft->Children[i]));
                }
                else if (pChild->GetType() == formula::svString
                    && !pCodeGen->takeString()
                    && pCodeGen->takeNumeric()
                    && pCodeGen->forceStringsToZero())
                {
                    SAL_INFO("sc.opencl", "Constant string case, treated as zero");
                    mvSubArguments.push_back(
                        DynamicKernelArgumentRef(new DynamicKernelStringToZeroArgument(mCalcConfig, ts,
                                ft->Children[i])));
                }
                else
                {
                    SAL_INFO("sc.opencl", "Unhandled operand, rejecting for OpenCL");
                    throw UnhandledToken(("unhandled operand " + StackVarEnumToString(pChild->GetType()) + " for ocPush").c_str(), __FILE__, __LINE__);
                }
                break;
            case ocPi:
                mvSubArguments.push_back(
                    std::make_shared<DynamicKernelPiArgument>(mCalcConfig, ts,
                            ft->Children[i]));
                break;
            case ocRandom:
                mvSubArguments.push_back(
                    std::make_shared<DynamicKernelRandomArgument>(mCalcConfig, ts,
                            ft->Children[i]));
                break;
#define CASE(opcode, createCode) \
            case opcode: \
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], createCode, nResultSize)); \
                break;
            CASE(ocAbs, std::make_shared<OpAbs>())
            CASE(ocAdd, std::make_shared<OpSum>(nResultSize))
            CASE(ocAnd, std::make_shared<OpAnd>())
            CASE(ocArcCos, std::make_shared<OpArcCos>())
            CASE(ocArcCosHyp, std::make_shared<OpArcCosHyp>())
            CASE(ocArcCot, std::make_shared<OpArcCot>())
            CASE(ocArcCotHyp, std::make_shared<OpArcCotHyp>())
            CASE(ocArcSin, std::make_shared<OpArcSin>())
            CASE(ocArcSinHyp, std::make_shared<OpArcSinHyp>())
            CASE(ocArcTan, std::make_shared<OpArcTan>())
            CASE(ocArcTan2, std::make_shared<OpArcTan2>())
            CASE(ocArcTanHyp, std::make_shared<OpArcTanH>())
            CASE(ocAveDev, std::make_shared<OpAveDev>())
            CASE(ocAverage, std::make_shared<OpAverage>(nResultSize))
            CASE(ocAverageA, std::make_shared<OpAverageA>(nResultSize))
            CASE(ocAverageIf, std::make_shared<OpAverageIf>())
            CASE(ocAverageIfs, std::make_shared<OpAverageIfs>())
            CASE(ocB, std::make_shared<OpB>())
            CASE(ocBetaDist, std::make_shared<OpBetaDist>())
            CASE(ocBetaInv, std::make_shared<OpBetainv>())
            CASE(ocBinomDist, std::make_shared<OpBinomdist>())
            CASE(ocBitAnd, std::make_shared<OpBitAnd>())
            CASE(ocBitLshift, std::make_shared<OpBitLshift>())
            CASE(ocBitOr, std::make_shared<OpBitOr>())
            CASE(ocBitRshift, std::make_shared<OpBitRshift>())
            CASE(ocBitXor, std::make_shared<OpBitXor>())
            CASE(ocCeil, std::make_shared<OpCeil>())
            CASE(ocChiDist, std::make_shared<OpChiDist>())
            CASE(ocChiInv, std::make_shared<OpChiInv>())
            CASE(ocChiSqDist, std::make_shared<OpChiSqDist>())
            CASE(ocChiSqInv, std::make_shared<OpChiSqInv>())
            CASE(ocCombin, std::make_shared<OpCombin>())
            CASE(ocCombinA, std::make_shared<OpCombinA>())
            CASE(ocConfidence, std::make_shared<OpConfidence>())
            CASE(ocCorrel, std::make_shared<OpCorrel>())
            CASE(ocCos, std::make_shared<OpCos>())
            CASE(ocCosHyp, std::make_shared<OpCosh>())
            CASE(ocCosecant, std::make_shared<OpCsc>())
            CASE(ocCosecantHyp, std::make_shared<OpCscH>())
            CASE(ocCot, std::make_shared<OpCot>())
            CASE(ocCotHyp, std::make_shared<OpCoth>())
            CASE(ocCount, std::make_shared<OpCount>(nResultSize))
            CASE(ocCount2, std::make_shared<OpCountA>(nResultSize))
            CASE(ocCountIf, std::make_shared<OpCountIf>())
            CASE(ocCountIfs, std::make_shared<OpCountIfs>())
            CASE(ocCovar, std::make_shared<OpCovar>())
            CASE(ocCritBinom, std::make_shared<OpCritBinom>())
            CASE(ocDB, std::make_shared<OpDB>())
            CASE(ocDDB, std::make_shared<OpDDB>())
            CASE(ocDeg, std::make_shared<OpDeg>())
            CASE(ocDevSq, std::make_shared<OpDevSq>())
            CASE(ocDiv, std::make_shared<OpDiv>(nResultSize))
            CASE(ocEqual, std::make_shared<OpEqual>())
            CASE(ocEven, std::make_shared<OpEven>())
            CASE(ocExp, std::make_shared<OpExp>())
            CASE(ocExpDist, std::make_shared<OpExponDist>())
            CASE(ocFDist, std::make_shared<OpFdist>())
            CASE(ocFInv, std::make_shared<OpFInv>())
            CASE(ocFTest, std::make_shared<OpFTest>())
            CASE(ocFV, std::make_shared<OpFV>())
            CASE(ocFact, std::make_shared<OpFact>())
            CASE(ocFisher, std::make_shared<OpFisher>())
            CASE(ocFisherInv, std::make_shared<OpFisherInv>())
            CASE(ocFloor, std::make_shared<OpFloor>())
            CASE(ocForecast, std::make_shared<OpForecast>())
            CASE(ocGamma, std::make_shared<OpGamma>())
            CASE(ocGammaDist, std::make_shared<OpGammaDist>())
            CASE(ocGammaInv, std::make_shared<OpGammaInv>())
            CASE(ocGammaLn, std::make_shared<OpGammaLn>())
            CASE(ocGauss, std::make_shared<OpGauss>())
            CASE(ocGeoMean, std::make_shared<OpGeoMean>())
            CASE(ocGreater, std::make_shared<OpGreater>())
            CASE(ocGreaterEqual, std::make_shared<OpGreaterEqual>())
            CASE(ocHarMean, std::make_shared<OpHarMean>())
            CASE(ocHypGeomDist, std::make_shared<OpHypGeomDist>())
            CASE(ocIRR, std::make_shared<OpIRR>())
            CASE(ocISPMT, std::make_shared<OpISPMT>())
            CASE(ocIf, std::make_shared<OpIf>())
            CASE(ocInt, std::make_shared<OpInt>())
            CASE(ocIntercept, std::make_shared<OpIntercept>())
            CASE(ocIpmt, std::make_shared<OpIPMT>())
            CASE(ocIsEven, std::make_shared<OpIsEven>())
            CASE(ocIsOdd, std::make_shared<OpIsOdd>())
            CASE(ocKurt, std::make_shared<OpKurt>())
            CASE(ocLess, std::make_shared<OpLess>())
            CASE(ocLessEqual, std::make_shared<OpLessEqual>())
            CASE(ocLn, std::make_shared<OpLn>())
            CASE(ocLog, std::make_shared<OpLog>())
            CASE(ocLog10, std::make_shared<OpLog10>())
            CASE(ocLogInv, std::make_shared<OpLogInv>())
            CASE(ocLogNormDist, std::make_shared<OpLogNormDist>())
            CASE(ocMIRR, std::make_shared<OpMIRR>())
            CASE(ocMax, std::make_shared<OpMax>(nResultSize))
            CASE(ocMaxA, std::make_shared<OpMaxA>(nResultSize))
            CASE(ocMin, std::make_shared<OpMin>(nResultSize))
            CASE(ocMinA, std::make_shared<OpMinA>(nResultSize))
            CASE(ocMod, std::make_shared<OpMod>())
            CASE(ocMul, std::make_shared<OpMul>(nResultSize))
            CASE(ocNPV, std::make_shared<OpNPV>())
            CASE(ocNegBinomVert , std::make_shared<OpNegbinomdist>())
            CASE(ocNegSub, std::make_shared<OpNegSub>())
            CASE(ocNormDist, std::make_shared<OpNormdist>())
            CASE(ocNormInv, std::make_shared<OpNorminv>())
            CASE(ocNot, std::make_shared<OpNot>())
            CASE(ocNotEqual, std::make_shared<OpNotEqual>())
            CASE(ocNper, std::make_shared<OpNper>())
            CASE(ocOdd, std::make_shared<OpOdd>())
            CASE(ocOr, std::make_shared<OpOr>())
            CASE(ocPDuration, std::make_shared<OpPDuration>())
            CASE(ocPMT, std::make_shared<OpPMT>())
            CASE(ocPV, std::make_shared<OpPV>())
            CASE(ocPearson, std::make_shared<OpPearson>())
            CASE(ocPermut, std::make_shared<OpPermut>())
            CASE(ocPermutationA, std::make_shared<OpPermutationA>())
            CASE(ocPhi, std::make_shared<OpPhi>())
            CASE(ocPoissonDist, std::make_shared<OpPoisson>())
            CASE(ocPow, std::make_shared<OpPower>())
            CASE(ocPower, std::make_shared<OpPower>())
            CASE(ocPpmt, std::make_shared<OpPPMT>())
            CASE(ocProduct, std::make_shared<OpProduct>())
            CASE(ocRRI, std::make_shared<OpRRI>())
            CASE(ocRSQ, std::make_shared<OpRsq>())
            CASE(ocRad, std::make_shared<OpRadians>())
            CASE(ocRate, std::make_shared<OpRate>())
            CASE(ocRound, std::make_shared<OpRound>())
            CASE(ocRoundDown, std::make_shared<OpRoundDown>())
            CASE(ocRoundUp, std::make_shared<OpRoundUp>())
            CASE(ocSLN, std::make_shared<OpSLN>())
            CASE(ocSNormInv, std::make_shared<OpNormsinv>())
            CASE(ocSTEYX, std::make_shared<OpSTEYX>())
            CASE(ocSYD, std::make_shared<OpSYD>())
            CASE(ocSecant, std::make_shared<OpSec>())
            CASE(ocSecantHyp, std::make_shared<OpSecH>())
            CASE(ocSin, std::make_shared<OpSin>())
            CASE(ocSinHyp, std::make_shared<OpSinh>())
            CASE(ocSkew, std::make_shared<OpSkew>())
            CASE(ocSkewp, std::make_shared<OpSkewp>())
            CASE(ocSlope, std::make_shared<OpSlope>())
            CASE(ocSqrt, std::make_shared<OpSqrt>())
            CASE(ocStDev, std::make_shared<OpStDev>())
            CASE(ocStDevA, std::make_shared<OpStDevA>())
            CASE(ocStDevP, std::make_shared<OpStDevP>())
            CASE(ocStDevPA, std::make_shared<OpStDevPA>())
            CASE(ocStandard, std::make_shared<OpStandard>())
            CASE(ocStdNormDist, std::make_shared<OpNormsdist>())
            CASE(ocSub, std::make_shared<OpSub>(nResultSize))
            CASE(ocSum, std::make_shared<OpSum>(nResultSize))
            CASE(ocSumIf, std::make_shared<OpSumIf>())
            CASE(ocSumIfs, std::make_shared<OpSumIfs>())
            CASE(ocSumProduct, std::make_shared<OpSumProduct>())
            CASE(ocSumSQ, std::make_shared<OpSumSQ>())
            CASE(ocSumX2DY2, std::make_shared<OpSumX2PY2>())
            CASE(ocSumX2MY2, std::make_shared<OpSumX2MY2>())
            CASE(ocSumXMY2, std::make_shared<OpSumXMY2>())
            CASE(ocTDist, std::make_shared<OpTDist>())
            CASE(ocTInv, std::make_shared<OpTInv>())
            CASE(ocTTest, std::make_shared<OpTTest>())
            CASE(ocTan, std::make_shared<OpTan>())
            CASE(ocTanHyp, std::make_shared<OpTanH>())
            CASE(ocTrunc, std::make_shared<OpTrunc>())
            CASE(ocVBD, std::make_shared<OpVDB>())
            CASE(ocVLookup, std::make_shared<OpVLookup>())
            CASE(ocVar, std::make_shared<OpVar>())
            CASE(ocVarA, std::make_shared<OpVarA>())
            CASE(ocVarP, std::make_shared<OpVarP>())
            CASE(ocVarPA, std::make_shared<OpVarPA>())
            CASE(ocWeibull, std::make_shared<OpWeibull>())
            CASE(ocXor, std::make_shared<OpXor>())
            CASE(ocZTest, std::make_shared<OpZTest>())
#undef CASE
            case ocExternal:
#define EXTCASE( name, createCode ) \
                else if (pChild->GetExternal() == name) \
                { \
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], createCode, nResultSize)); \
                }

                if(false) // start else-if chain
                    ;
                EXTCASE("com.sun.star.sheet.addin.Analysis.getAccrint", std::make_shared<OpAccrint>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getAccrintm", std::make_shared<OpAccrintm>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getAmordegrc", std::make_shared<OpAmordegrc>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getAmorlinc", std::make_shared<OpAmorlinc>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getBesselj", std::make_shared<OpBesselj>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getCoupdaybs", std::make_shared<OpCoupdaybs>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getCoupdays", std::make_shared<OpCoupdays>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getCoupdaysnc", std::make_shared<OpCoupdaysnc>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getCoupncd", std::make_shared<OpCoupncd>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getCoupnum", std::make_shared<OpCoupnum>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getCouppcd", std::make_shared<OpCouppcd>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getCumipmt", std::make_shared<OpCumipmt>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getCumprinc", std::make_shared<OpCumprinc>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getDisc", std::make_shared<OpDISC>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getDollarde", std::make_shared<OpDollarde>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getDollarfr", std::make_shared<OpDollarfr>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getDuration", std::make_shared<OpDuration_ADD>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getEffect", std::make_shared<OpEffective>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getFvschedule", std::make_shared<OpFvschedule>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getGestep", std::make_shared<OpGestep>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getIntrate", std::make_shared<OpINTRATE>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getIseven", std::make_shared<OpIsEven>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getIsodd", std::make_shared<OpIsOdd>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getMduration", std::make_shared<OpMDuration>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getMround", std::make_shared<OpMROUND>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getNominal", std::make_shared<OpNominal>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getOddlprice", std::make_shared<OpOddlprice>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getOddlyield", std::make_shared<OpOddlyield>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getPrice", std::make_shared<OpPrice>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getPricedisc", std::make_shared<OpPriceDisc>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getPricemat", std::make_shared<OpPriceMat>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getQuotient", std::make_shared<OpQuotient>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getReceived", std::make_shared<OpReceived>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getSeriessum", std::make_shared<OpSeriesSum>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getSqrtpi", std::make_shared<OpSqrtPi>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getTbilleq", std::make_shared<OpTbilleq>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getTbillprice", std::make_shared<OpTbillprice>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getTbillyield", std::make_shared<OpTbillyield>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getXirr", std::make_shared<OpXirr>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getXnpv", std::make_shared<OpXNPV>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getYield", std::make_shared<OpYield>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getYielddisc", std::make_shared<OpYielddisc>())
                EXTCASE("com.sun.star.sheet.addin.Analysis.getYieldmat", std::make_shared<OpYieldmat>())
                else
                    throw UnhandledToken(OUString("unhandled external " + pChild->GetExternal()).toUtf8().getStr(), __FILE__, __LINE__);
                break;
#undef EXTCASE

            default:
                throw UnhandledToken(OUString("unhandled opcode "
                    + formula::FormulaCompiler().GetOpCodeMap(com::sun::star::sheet::FormulaLanguage::ENGLISH)->getSymbol(opc)
                    + "(" + OUString::number(opc) + ")").toUtf8().getStr(), __FILE__, __LINE__);
        }
    }
}

namespace {

class DynamicKernel : public CompiledFormula
{
public:
    DynamicKernel( ScCalcConfig  config, FormulaTreeNodeRef r, int nResultSize );
    virtual ~DynamicKernel() override;

    static std::shared_ptr<DynamicKernel> create( const ScCalcConfig& config, const ScTokenArray& rCode, int nResultSize );

    /// OpenCL code generation
    void CodeGen();

    /// Produce kernel hash
    std::string const & GetMD5();

    /// Create program, build, and create kernel
    /// TODO cache results based on kernel body hash
    /// TODO: abstract OpenCL part out into OpenCL wrapper.
    void CreateKernel();

    /// Prepare buffers, marshal them to GPU, and launch the kernel
    /// TODO: abstract OpenCL part out into OpenCL wrapper.
    void Launch( size_t nr );

    cl_mem GetResultBuffer() const { return mpResClmem; }

private:
    ScCalcConfig mCalcConfig;
    FormulaTreeNodeRef mpRoot;
    SymbolTable mSyms;
    std::string mKernelSignature, mKernelHash;
    std::string mFullProgramSrc;
    cl_program mpProgram;
    cl_kernel mpKernel;
    cl_mem mpResClmem; // Results
    std::set<std::string> inlineDecl;
    std::set<std::string> inlineFun;

    int mnResultSize;
};

}

DynamicKernel::DynamicKernel( ScCalcConfig config, FormulaTreeNodeRef x, int nResultSize ) :
    mCalcConfig(std::move(config)),
    mpRoot(std::move(x)),
    mpProgram(nullptr),
    mpKernel(nullptr),
    mpResClmem(nullptr),
    mnResultSize(nResultSize) {}

DynamicKernel::~DynamicKernel()
{
    cl_int err;
    if (mpResClmem)
    {
        err = clReleaseMemObject(mpResClmem);
        SAL_WARN_IF(err != CL_SUCCESS, "sc.opencl", "clReleaseMemObject failed: " << openclwrapper::errorString(err));
    }
    if (mpKernel)
    {
        SAL_INFO("sc.opencl", "Releasing kernel " << mpKernel);
        err = clReleaseKernel(mpKernel);
        SAL_WARN_IF(err != CL_SUCCESS, "sc.opencl", "clReleaseKernel failed: " << openclwrapper::errorString(err));
    }
    // mpProgram is not going to be released here -- it's cached.
}

void DynamicKernel::CodeGen()
{
    // Traverse the tree of expression and declare symbols used
    const DynamicKernelArgument* DK = mSyms.DeclRefArg<DynamicKernelSoPArguments>(mCalcConfig, mpRoot, std::make_shared<OpNop>(mnResultSize), mnResultSize);

    outputstream decl;
    if (openclwrapper::gpuEnv.mnKhrFp64Flag)
    {
        decl << "#if __OPENCL_VERSION__ < 120\n";
        decl << "#pragma OPENCL EXTENSION cl_khr_fp64: enable\n";
        decl << "#endif\n";
    }
    else if (openclwrapper::gpuEnv.mnAmdFp64Flag)
    {
        decl << "#pragma OPENCL EXTENSION cl_amd_fp64: enable\n";
    }
    // preambles
    decl << publicFunc;
    DK->DumpInlineFun(inlineDecl, inlineFun);
    for (const auto& rItem : inlineDecl)
    {
        decl << rItem;
    }

    for (const auto& rItem : inlineFun)
    {
        decl << rItem;
    }
    mSyms.DumpSlidingWindowFunctions(decl);
    mKernelSignature = DK->DumpOpName();
    decl << "__kernel void DynamicKernel" << mKernelSignature;
    decl << "(__global double *result";
    if( !DK->IsEmpty())
    {
        decl << ", ";
        DK->GenSlidingWindowDecl(decl);
    }
    decl << ") {\n\tint gid0 = get_global_id(0);\n\tresult[gid0] = " <<
        DK->GenSlidingWindowDeclRef() << ";\n}\n";
    mFullProgramSrc = decl.str();
    SAL_INFO(
        "sc.opencl.source",
        (mKernelSignature[0] == '_'
         ? mKernelSignature.substr(1, std::string::npos) : mKernelSignature)
        << " program to be compiled:\n" << linenumberify(mFullProgramSrc));
}

std::string const & DynamicKernel::GetMD5()
{
    if (mKernelHash.empty())
    {
        outputstream md5s;
        // Compute MD5SUM of kernel body to obtain the name
        sal_uInt8 result[RTL_DIGEST_LENGTH_MD5];
        rtl_digest_MD5(
            mFullProgramSrc.c_str(),
            mFullProgramSrc.length(), result,
            RTL_DIGEST_LENGTH_MD5);
        for (sal_uInt8 i : result)
        {
            md5s << std::hex << static_cast<int>(i);
        }
        mKernelHash = md5s.str();
    }
    return mKernelHash;
}

/// Build code
void DynamicKernel::CreateKernel()
{
    if (mpKernel)
        // already created.
        return;

    cl_int err;
    std::string kname = "DynamicKernel" + mKernelSignature;
    // Compile kernel here!!!

    OpenCLZone zone;
    openclwrapper::KernelEnv kEnv;
    openclwrapper::setKernelEnv(&kEnv);
    const char* src = mFullProgramSrc.c_str();
    static std::string lastOneKernelHash;
    static std::string lastSecondKernelHash;
    static cl_program lastOneProgram = nullptr;
    static cl_program lastSecondProgram = nullptr;
    std::string KernelHash = mKernelSignature + GetMD5();
    if (lastOneKernelHash == KernelHash && lastOneProgram)
    {
        mpProgram = lastOneProgram;
    }
    else if (lastSecondKernelHash == KernelHash && lastSecondProgram)
    {
        mpProgram = lastSecondProgram;
    }
    else
    {   // doesn't match the last compiled formula.

        if (lastSecondProgram)
        {
            SAL_INFO("sc.opencl", "Releasing program " << lastSecondProgram);
            err = clReleaseProgram(lastSecondProgram);
            SAL_WARN_IF(err != CL_SUCCESS, "sc.opencl", "clReleaseProgram failed: " << openclwrapper::errorString(err));
            lastSecondProgram = nullptr;
        }
        if (openclwrapper::buildProgramFromBinary("",
                &openclwrapper::gpuEnv, KernelHash.c_str(), 0))
        {
            mpProgram = openclwrapper::gpuEnv.mpArryPrograms[0];
            openclwrapper::gpuEnv.mpArryPrograms[0] = nullptr;
        }
        else
        {
            mpProgram = clCreateProgramWithSource(kEnv.mpkContext, 1,
                &src, nullptr, &err);
            if (err != CL_SUCCESS)
                throw OpenCLError("clCreateProgramWithSource", err, __FILE__, __LINE__);
            SAL_INFO("sc.opencl", "Created program " << mpProgram);

            err = clBuildProgram(mpProgram, 1,
                &openclwrapper::gpuEnv.mpDevID, "", nullptr, nullptr);
            if (err != CL_SUCCESS)
            {
#if OSL_DEBUG_LEVEL > 0
                if (err == CL_BUILD_PROGRAM_FAILURE)
                {
                    cl_build_status stat;
                    cl_int e = clGetProgramBuildInfo(
                        mpProgram, openclwrapper::gpuEnv.mpDevID,
                        CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status),
                        &stat, nullptr);
                    SAL_WARN_IF(
                        e != CL_SUCCESS, "sc.opencl",
                        "after CL_BUILD_PROGRAM_FAILURE,"
                        " clGetProgramBuildInfo(CL_PROGRAM_BUILD_STATUS)"
                        " fails with " << openclwrapper::errorString(e));
                    if (e == CL_SUCCESS)
                    {
                        size_t n;
                        e = clGetProgramBuildInfo(
                            mpProgram, openclwrapper::gpuEnv.mpDevID,
                            CL_PROGRAM_BUILD_LOG, 0, nullptr, &n);
                        SAL_WARN_IF(
                            e != CL_SUCCESS || n == 0, "sc.opencl",
                            "after CL_BUILD_PROGRAM_FAILURE,"
                            " clGetProgramBuildInfo(CL_PROGRAM_BUILD_LOG)"
                            " fails with " << openclwrapper::errorString(e) << ", n=" << n);
                        if (e == CL_SUCCESS && n != 0)
                        {
                            std::vector<char> log(n);
                            e = clGetProgramBuildInfo(
                                mpProgram, openclwrapper::gpuEnv.mpDevID,
                                CL_PROGRAM_BUILD_LOG, n, log.data(), nullptr);
                            SAL_WARN_IF(
                                e != CL_SUCCESS || n == 0, "sc.opencl",
                                "after CL_BUILD_PROGRAM_FAILURE,"
                                " clGetProgramBuildInfo("
                                "CL_PROGRAM_BUILD_LOG) fails with " << openclwrapper::errorString(e));
                            if (e == CL_SUCCESS)
                                SAL_WARN(
                                    "sc.opencl",
                                    "CL_BUILD_PROGRAM_FAILURE, status " << stat
                                    << ", log \"" << log.data() << "\"");
                        }
                    }
                }
#endif
#ifdef DBG_UTIL
                SAL_WARN("sc.opencl", "Program failed to build, aborting.");
                abort(); // make sure errors such as typos don't accidentally go unnoticed
#else
                throw OpenCLError("clBuildProgram", err, __FILE__, __LINE__);
#endif
            }
            SAL_INFO("sc.opencl", "Built program " << mpProgram);

            // Generate binary out of compiled kernel.
            openclwrapper::generatBinFromKernelSource(mpProgram,
                (mKernelSignature + GetMD5()).c_str());
        }
        lastSecondKernelHash = lastOneKernelHash;
        lastSecondProgram = lastOneProgram;
        lastOneKernelHash = std::move(KernelHash);
        lastOneProgram = mpProgram;
    }
    mpKernel = clCreateKernel(mpProgram, kname.c_str(), &err);
    if (err != CL_SUCCESS)
        throw OpenCLError("clCreateKernel", err, __FILE__, __LINE__);
    SAL_INFO("sc.opencl", "Created kernel " << mpKernel << " with name " << kname << " in program " << mpProgram);
}

void DynamicKernel::Launch( size_t nr )
{
    OpenCLZone zone;
    openclwrapper::KernelEnv kEnv;
    openclwrapper::setKernelEnv(&kEnv);
    cl_int err;
    // The results
    mpResClmem = clCreateBuffer(kEnv.mpkContext,
        cl_mem_flags(CL_MEM_READ_WRITE) | CL_MEM_ALLOC_HOST_PTR,
        nr * sizeof(double), nullptr, &err);
    if (CL_SUCCESS != err)
        throw OpenCLError("clCreateBuffer", err, __FILE__, __LINE__);
    SAL_INFO("sc.opencl", "Created buffer " << mpResClmem << " size " << nr << "*" << sizeof(double) << "=" << (nr*sizeof(double)));

    SAL_INFO("sc.opencl", "Kernel " << mpKernel << " arg " << 0 << ": cl_mem: " << mpResClmem << " (result)");
    err = clSetKernelArg(mpKernel, 0, sizeof(cl_mem), static_cast<void*>(&mpResClmem));
    if (CL_SUCCESS != err)
        throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
    // The rest of buffers
    mSyms.Marshal(mpKernel, nr, mpProgram);
    size_t global_work_size[] = { nr };
    SAL_INFO("sc.opencl", "Enqueuing kernel " << mpKernel);
    err = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, mpKernel, 1, nullptr,
        global_work_size, nullptr, 0, nullptr, nullptr);
    if (CL_SUCCESS != err)
        throw OpenCLError("clEnqueueNDRangeKernel", err, __FILE__, __LINE__);
    err = clFlush(kEnv.mpkCmdQueue);
    if (CL_SUCCESS != err)
        throw OpenCLError("clFlush", err, __FILE__, __LINE__);
}

// Symbol lookup. If there is no such symbol created, allocate one
// kernel with argument with unique name and return so.
// The template argument T must be a subclass of DynamicKernelArgument
template <typename T>
const DynamicKernelArgument* SymbolTable::DeclRefArg(const ScCalcConfig& config,
                                                     const FormulaTreeNodeRef& t,
                                                     std::shared_ptr<SlidingFunctionBase> pCodeGen, int nResultSize)
{
    FormulaToken* ref = t->GetFormulaToken();
    ArgumentMap::iterator it = mSymbols.find(ref);
    if (it == mSymbols.end())
    {
        // Allocate new symbols
        outputstream ss;
        ss << "tmp" << mCurId++;
        DynamicKernelArgumentRef new_arg = std::make_shared<T>(config, ss.str(), t, std::move(pCodeGen), nResultSize);
        mSymbols[ref] = new_arg;
        mParams.push_back(new_arg);
        return new_arg.get();
    }
    else
    {
        return it->second.get();
    }
}

FormulaGroupInterpreterOpenCL::FormulaGroupInterpreterOpenCL() {}

FormulaGroupInterpreterOpenCL::~FormulaGroupInterpreterOpenCL() {}

ScMatrixRef FormulaGroupInterpreterOpenCL::inverseMatrix( const ScMatrix& )
{
    return nullptr;
}

std::shared_ptr<DynamicKernel> DynamicKernel::create( const ScCalcConfig& rConfig, const ScTokenArray& rCode, int nResultSize )
{
    // Constructing "AST"
    FormulaTokenIterator aCode(rCode);
    std::vector<FormulaToken*> aTokenVector;
    std::map<FormulaToken*, FormulaTreeNodeRef> aHashMap;
    FormulaToken*  pCur;
    while ((pCur = const_cast<FormulaToken*>(aCode.Next())) != nullptr)
    {
        OpCode eOp = pCur->GetOpCode();
        if (eOp != ocPush)
        {
            FormulaTreeNodeRef pCurNode = std::make_shared<FormulaTreeNode>(pCur);
            sal_uInt8 nParamCount =  pCur->GetParamCount();
            for (sal_uInt8 i = 0; i < nParamCount; i++)
            {
                if( aTokenVector.empty())
                    return nullptr;
                FormulaToken* pTempFormula = aTokenVector.back();
                aTokenVector.pop_back();
                if (pTempFormula->GetOpCode() != ocPush)
                {
                    if (aHashMap.find(pTempFormula) == aHashMap.end())
                        return nullptr;
                    pCurNode->Children.push_back(aHashMap[pTempFormula]);
                }
                else
                {
                    FormulaTreeNodeRef pChildTreeNode =
                        std::make_shared<FormulaTreeNode>(pTempFormula);
                    pCurNode->Children.push_back(std::move(pChildTreeNode));
                }
            }
            std::reverse(pCurNode->Children.begin(), pCurNode->Children.end());
            aHashMap[pCur] = std::move(pCurNode);
        }
        aTokenVector.push_back(pCur);
    }

    FormulaTreeNodeRef Root = std::make_shared<FormulaTreeNode>(nullptr);
    Root->Children.push_back(aHashMap[aTokenVector.back()]);

    auto pDynamicKernel = std::make_shared<DynamicKernel>(rConfig, Root, nResultSize);

    // OpenCL source code generation and kernel compilation
    try
    {
        pDynamicKernel->CodeGen();
        pDynamicKernel->CreateKernel();
    }
    catch (const UnhandledToken& ut)
    {
        SAL_INFO("sc.opencl", "Dynamic formula compiler: UnhandledToken: " << ut.mMessage << " at " << ut.mFile << ":" << ut.mLineNumber);
        return nullptr;
    }
    catch (const InvalidParameterCount& ipc)
    {
        SAL_INFO("sc.opencl", "Dynamic formula compiler: InvalidParameterCount " << ipc.mParameterCount
            << " at " << ipc.mFile << ":" << ipc.mLineNumber);
        return nullptr;
    }
    catch (const OpenCLError& oce)
    {
        // I think OpenCLError exceptions are actually exceptional (unexpected), so do use SAL_WARN
        // here.
        SAL_WARN("sc.opencl", "Dynamic formula compiler: OpenCLError from " << oce.mFunction << ": " << openclwrapper::errorString(oce.mError) << " at " << oce.mFile << ":" << oce.mLineNumber);

        // OpenCLError used to go to the catch-all below, and not delete pDynamicKernel. Was that
        // intentional, should we not do it here then either?
        openclwrapper::kernelFailures++;
        return nullptr;
    }
    catch (const Unhandled& uh)
    {
        SAL_INFO("sc.opencl", "Dynamic formula compiler: Unhandled at " << uh.mFile << ":" << uh.mLineNumber);

        // Unhandled used to go to the catch-all below, and not delete pDynamicKernel. Was that
        // intentional, should we not do it here then either?
        openclwrapper::kernelFailures++;
        return nullptr;
    }
    catch (...)
    {
        // FIXME: Do we really want to catch random exceptions here?
        SAL_WARN("sc.opencl", "Dynamic formula compiler: unexpected exception");
        openclwrapper::kernelFailures++;
        return nullptr;
    }
    return pDynamicKernel;
}

namespace {

class CLInterpreterResult
{
    DynamicKernel* mpKernel;

    SCROW mnGroupLength;

    cl_mem mpCLResBuf;
    double* mpResBuf;

public:
    CLInterpreterResult() : mpKernel(nullptr), mnGroupLength(0), mpCLResBuf(nullptr), mpResBuf(nullptr) {}
    CLInterpreterResult( DynamicKernel* pKernel, SCROW nGroupLength ) :
        mpKernel(pKernel), mnGroupLength(nGroupLength), mpCLResBuf(nullptr), mpResBuf(nullptr) {}

    bool isValid() const { return mpKernel != nullptr; }

    void fetchResultFromKernel()
    {
        if (!isValid())
            return;

        OpenCLZone zone;

        // Map results back
        mpCLResBuf = mpKernel->GetResultBuffer();

        openclwrapper::KernelEnv kEnv;
        openclwrapper::setKernelEnv(&kEnv);

        cl_int err;
        mpResBuf = static_cast<double*>(clEnqueueMapBuffer(kEnv.mpkCmdQueue,
            mpCLResBuf,
            CL_TRUE, CL_MAP_READ, 0,
            mnGroupLength * sizeof(double), 0, nullptr, nullptr,
            &err));

        if (err != CL_SUCCESS)
        {
            SAL_WARN("sc.opencl", "clEnqueueMapBuffer failed:: " << openclwrapper::errorString(err));
            mpResBuf = nullptr;
            return;
        }
        SAL_INFO("sc.opencl", "Kernel results: cl_mem: " << mpResBuf << " (" << DebugPeekDoubles(mpResBuf, mnGroupLength) << ")");
    }

    bool pushResultToDocument( ScDocument& rDoc, const ScAddress& rTopPos )
    {
        if (!mpResBuf)
            return false;

        OpenCLZone zone;

        rDoc.SetFormulaResults(rTopPos, mpResBuf, mnGroupLength);

        openclwrapper::KernelEnv kEnv;
        openclwrapper::setKernelEnv(&kEnv);

        cl_int err;
        err = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue, mpCLResBuf, mpResBuf, 0, nullptr, nullptr);

        if (err != CL_SUCCESS)
        {
            SAL_WARN("sc.opencl", "clEnqueueUnmapMemObject failed: " << openclwrapper::errorString(err));
            return false;
        }

        return true;
    }
};

class CLInterpreterContext
{
    std::shared_ptr<DynamicKernel> mpKernelStore; /// for managed kernel instance.
    DynamicKernel* mpKernel;

    SCROW mnGroupLength;

public:
    explicit CLInterpreterContext(SCROW nGroupLength, std::shared_ptr<DynamicKernel> pKernel )
        : mpKernelStore(std::move(pKernel))
        , mpKernel(mpKernelStore.get())
        , mnGroupLength(nGroupLength) {}

    ~CLInterpreterContext()
    {
        DynamicKernelArgument::ClearStringIds();
    }

    bool isValid() const
    {
        return mpKernel != nullptr;
    }

    CLInterpreterResult launchKernel()
    {
        if (!isValid())
            return CLInterpreterResult();

        try
        {
            // Run the kernel.
            mpKernel->Launch(mnGroupLength);
        }
        catch (const UnhandledToken& ut)
        {
            SAL_INFO("sc.opencl", "Dynamic formula compiler: UnhandledToken: " << ut.mMessage << " at " << ut.mFile << ":" << ut.mLineNumber);
            openclwrapper::kernelFailures++;
            return CLInterpreterResult();
        }
        catch (const OpenCLError& oce)
        {
            SAL_WARN("sc.opencl", "Dynamic formula compiler: OpenCLError from " << oce.mFunction << ": " << openclwrapper::errorString(oce.mError) << " at " << oce.mFile << ":" << oce.mLineNumber);
            openclwrapper::kernelFailures++;
            return CLInterpreterResult();
        }
        catch (const Unhandled& uh)
        {
            SAL_INFO("sc.opencl", "Dynamic formula compiler: Unhandled at " << uh.mFile << ":" << uh.mLineNumber);
            openclwrapper::kernelFailures++;
            return CLInterpreterResult();
        }
        catch (...)
        {
            SAL_WARN("sc.opencl", "Dynamic formula compiler: unexpected exception");
            openclwrapper::kernelFailures++;
            return CLInterpreterResult();
        }

        return CLInterpreterResult(mpKernel, mnGroupLength);
    }
};


CLInterpreterContext createCLInterpreterContext( const ScCalcConfig& rConfig,
    const ScFormulaCellGroupRef& xGroup, const ScTokenArray& rCode )
{
    return CLInterpreterContext(xGroup->mnLength, DynamicKernel::create(rConfig, rCode, xGroup->mnLength));
}

void genRPNTokens( ScDocument& rDoc, const ScAddress& rTopPos, ScTokenArray& rCode )
{
    ScCompiler aComp(rDoc, rTopPos, rCode, rDoc.GetGrammar());
    // Disable special ordering for jump commands for the OpenCL interpreter.
    aComp.EnableJumpCommandReorder(false);
    aComp.CompileTokenArray(); // Regenerate RPN tokens.
}

bool waitForResults()
{
    OpenCLZone zone;
    openclwrapper::KernelEnv kEnv;
    openclwrapper::setKernelEnv(&kEnv);

    cl_int err = clFinish(kEnv.mpkCmdQueue);
    if (err != CL_SUCCESS)
        SAL_WARN("sc.opencl", "clFinish failed: " << openclwrapper::errorString(err));

    return err == CL_SUCCESS;
}

}

bool FormulaGroupInterpreterOpenCL::interpret( ScDocument& rDoc,
    const ScAddress& rTopPos, ScFormulaCellGroupRef& xGroup,
    ScTokenArray& rCode )
{
    SAL_INFO("sc.opencl", "Interpret cell group " << rTopPos);
    MergeCalcConfig(rDoc);

    genRPNTokens(rDoc, rTopPos, rCode);

    if( rCode.GetCodeLen() == 0 )
        return false;

    CLInterpreterContext aCxt = createCLInterpreterContext(maCalcConfig, xGroup, rCode);
    if (!aCxt.isValid())
        return false;

    CLInterpreterResult aRes = aCxt.launchKernel();
    if (!aRes.isValid())
        return false;

    if (!waitForResults())
        return false;

    aRes.fetchResultFromKernel();

    return aRes.pushResultToDocument(rDoc, rTopPos);
}

} // namespace sc::opencl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
