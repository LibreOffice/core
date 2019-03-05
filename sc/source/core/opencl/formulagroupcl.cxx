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
#include <formula/vectortoken.hxx>
#include <scmatrix.hxx>
#include <sal/log.hxx>
#include <rtl/math.hxx>

#include <opencl/openclwrapper.hxx>

#include "op_financial.hxx"
#include "op_database.hxx"
#include "op_math.hxx"
#include "op_logical.hxx"
#include "op_statistical.hxx"
#include "op_array.hxx"
#include "op_spreadsheet.hxx"
#include "op_addin.hxx"

#include <com/sun/star/sheet/FormulaLanguage.hpp>

// FIXME: The idea that somebody would bother to (now and then? once a year? once a month?) manually
// edit a source file and change the value of some #defined constant and run some ill-defined
// "correctness test" is of course ludicrous. Either things are checked in normal unit tests, in
// every 'make check', or not at all. The below comments are ridiculous.

#define REDUCE_THRESHOLD 201  // set to 4 for correctness testing. priority 1
#define UNROLLING_FACTOR 16  // set to 4 for correctness testing (if no reduce)

static const char* const publicFunc =
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
 // nVidia OpenCL, at least on Linux, seems to ignore the argument to nan(),
 // so using that would not propagate the type of error, work that around
 // by directly constructing the proper IEEE double NaN value
 // TODO: maybe use a better way to detect such systems?
 "#ifdef cl_nv_pragma_unroll\n"
 "    return as_double(0x7FF8000000000000+nErr);\n"
 "#endif\n"
 "    return nan(nErr);\n"
 "}\n"
 "\n"
 "uint GetDoubleErrorValue(double fVal)\n"
 "{\n"
 "    if (isfinite(fVal))\n"
 "        return 0;\n"
 "    if (isinf(fVal))\n"
 "        return IllegalFPOperation; // normal INF\n"
 "    if (as_ulong(fVal) & 0XFFFF0000u)\n"
 "        return NoValue;            // just a normal NAN\n"
 "    return (as_ulong(fVal) & 0XFFFF); // any other error\n"
 "}\n"
 "\n"
 "double fsum_count(double a, double b, __private int *p) {\n"
 "    bool t = isnan(a);\n"
 "    (*p) += t?0:1;\n"
 "    return t?b:a+b;\n"
 "}\n"
 "double fmin_count(double a, double b, __private int *p) {\n"
 "    double result = fmin(a, b);\n"
 "    bool t = isnan(result);\n"
 "    (*p) += t?0:1;\n"
 "    return result;\n"
 "}\n"
 "double fmax_count(double a, double b, __private int *p) {\n"
 "    double result = fmax(a, b);\n"
 "    bool t = isnan(result);\n"
 "    (*p) += t?0:1;\n"
 "    return result;\n"
 "}\n"
 "double fsum(double a, double b) { return isnan(a)?b:a+b; }\n"
 "double legalize(double a, double b) { return isnan(a)?b:a;}\n"
 "double fsub(double a, double b) { return a-b; }\n"
 "double fdiv(double a, double b) { return a/b; }\n"
 "double strequal(unsigned a, unsigned b) { return (a==b)?1.0:0; }\n"
 "int is_representable_integer(double a) {\n"
 "    long kMaxInt = (1L << 53) - 1;\n"
 "    if (a <= as_double(kMaxInt))\n"
 "    {\n"
 "        long nInt = as_long(a);\n"
 "        double fInt;\n"
 "        return (nInt <= kMaxInt &&\n"
 "                (!((fInt = as_double(nInt)) < a) && !(fInt > a)));\n"
 "    }\n"
 "    return 0;\n"
 "}\n"
 "int approx_equal(double a, double b) {\n"
 "    double e48 = 1.0 / (16777216.0 * 16777216.0);\n"
 "    double e44 = e48 * 16.0;\n"
 "    if (a == b)\n"
 "        return 1;\n"
 "    if (a == 0.0 || b == 0.0)\n"
 "        return 0;\n"
 "    double d = fabs(a - b);\n"
 "    if (!isfinite(d))\n"
 "        return 0;   // Nan or Inf involved\n"
 "    if (d > ((a = fabs(a)) * e44) || d > ((b = fabs(b)) * e44))\n"
 "        return 0;\n"
 "    if (is_representable_integer(d) && is_representable_integer(a) && is_representable_integer(b))\n"
 "        return 0;   // special case for representable integers.\n"
 "    return (d < a * e48 && d < b * e48);\n"
 "}\n"
 "double fsum_approx(double a, double b) {\n"
 "    if ( ((a < 0.0 && b > 0.0) || (b < 0.0 && a > 0.0))\n"
 "         && approx_equal( a, -b ) )\n"
 "        return 0.0;\n"
 "    return a + b;\n"
 "}\n"
 "double fsub_approx(double a, double b) {\n"
 "    if ( ((a < 0.0 && b < 0.0) || (a > 0.0 && b > 0.0)) && approx_equal( a, b ) )\n"
 "        return 0.0;\n"
 "    return a - b;\n"
 "}\n"
 ;

#include <vector>
#include <map>
#include <iostream>
#include <algorithm>

#include <rtl/digest.h>

#include <memory>

using namespace formula;

namespace sc { namespace opencl {

namespace {

std::string linenumberify(const std::string& s)
{
    std::stringstream ss;
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

OUString LimitedString( const OUString& str )
{
    if( str.getLength() < 20 )
        return "\"" + str + "\"";
    else
        return "\"" + str.copy( 0, 20 ) + "\"...";
}

// Returns formatted contents of the data (possibly shortened), to be used in debug output.
OUString DebugPeekData(const FormulaToken* ref, int doubleRefIndex = 0)
{
    if (ref->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken* pSVR =
            static_cast<const formula::SingleVectorRefToken*>(ref);
        OUStringBuffer buf = "SingleRef {";
        for( size_t i = 0; i < std::min< size_t >( 4, pSVR->GetArrayLength()); ++i )
        {
            if( i != 0 )
                buf.append( "," );
            if( pSVR->GetArray().mpNumericArray != nullptr )
                buf.append( pSVR->GetArray().mpNumericArray[ i ] );
            else if( pSVR->GetArray().mpStringArray != nullptr )
                buf.append( LimitedString( OUString( pSVR->GetArray().mpStringArray[ i ] )));
        }
        if( pSVR->GetArrayLength() > 4 )
            buf.append( ",..." );
        buf.append( "}" );
        return buf.makeStringAndClear();
    }
    else if (ref->GetType() == formula::svDoubleVectorRef)
    {
        const formula::DoubleVectorRefToken* pDVR =
            static_cast<const formula::DoubleVectorRefToken*>(ref);
        OUStringBuffer buf = "DoubleRef {";
        for( size_t i = 0; i < std::min< size_t >( 4, pDVR->GetArrayLength()); ++i )
        {
            if( i != 0 )
                buf.append( "," );
            if( pDVR->GetArrays()[doubleRefIndex].mpNumericArray != nullptr )
                buf.append( pDVR->GetArrays()[doubleRefIndex].mpNumericArray[ i ] );
            else if( pDVR->GetArrays()[doubleRefIndex].mpStringArray != nullptr )
                buf.append( LimitedString( OUString( pDVR->GetArrays()[doubleRefIndex].mpStringArray[ i ] )));
        }
        if( pDVR->GetArrayLength() > 4 )
            buf.append( ",..." );
        buf.append( "}" );
        return buf.makeStringAndClear();
    }
    else if (ref->GetType() == formula::svString)
    {
        return "String " + LimitedString( ref->GetString().getString());
    }
    else if (ref->GetType() == formula::svDouble)
    {
        return OUString::number(ref->GetDouble());
    }
    else
    {
        return "?";
    }
}

// Returns formatted contents of a doubles buffer, to be used in debug output.
OUString DebugPeekDoubles(const double* data, int size)
{
    OUStringBuffer buf = "{";
    for( int i = 0; i < std::min( 4, size ); ++i )
    {
        if( i != 0 )
            buf.append( "," );
        buf.append( data[ i ] );
    }
    if( size > 4 )
        buf.append( ",..." );
    buf.append( "}" );
    return buf.makeStringAndClear();
}

} // anonymous namespace

/// Map the buffer used by an argument and do necessary argument setting
size_t VectorRef::Marshal( cl_kernel k, int argno, int, cl_program )
{
    FormulaToken* ref = mFormulaTree->GetFormulaToken();
    double* pHostBuffer = nullptr;
    size_t szHostBuffer = 0;
    if (ref->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken* pSVR =
            static_cast<const formula::SingleVectorRefToken*>(ref);

        SAL_INFO("sc.opencl", "SingleVectorRef len=" << pSVR->GetArrayLength() << " mpNumericArray=" << pSVR->GetArray().mpNumericArray << " (mpStringArray=" << pSVR->GetArray().mpStringArray << ")");

        pHostBuffer = const_cast<double*>(pSVR->GetArray().mpNumericArray);
        szHostBuffer = pSVR->GetArrayLength() * sizeof(double);
    }
    else if (ref->GetType() == formula::svDoubleVectorRef)
    {
        const formula::DoubleVectorRefToken* pDVR =
            static_cast<const formula::DoubleVectorRefToken*>(ref);

        SAL_INFO("sc.opencl", "DoubleVectorRef index=" << mnIndex << " len=" << pDVR->GetArrayLength() << " mpNumericArray=" << pDVR->GetArrays()[mnIndex].mpNumericArray << " (mpStringArray=" << pDVR->GetArrays()[mnIndex].mpStringArray << ")");

        pHostBuffer = const_cast<double*>(
            pDVR->GetArrays()[mnIndex].mpNumericArray);
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
            rtl::math::setNan(&pNanBuffer[i]);
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

/// Arguments that are actually compile-time constant string
/// Currently, only the hash is passed.
/// TBD(IJSUNG): pass also length and the actual string if there is a
/// hash function collision

/// FIXME: This idea of passing of hashes of uppercased strings into OpenCL code is fairly potent
/// crack. It is hopefully not used at all any more, but noticing that there are string arguments
/// automatically disables use of OpenCL for a formula group. If at some point there are resources
/// to drain the OpenCL swamp, this should go away.

class ConstStringArgument : public DynamicKernelArgument
{
public:
    ConstStringArgument( const ScCalcConfig& config, const std::string& s,
        const FormulaTreeNodeRef& ft ) :
        DynamicKernelArgument(config, s, ft) { }
    /// Generate declaration
    virtual void GenDecl( std::stringstream& ss ) const override
    {
        ss << "unsigned " << mSymName;
    }
    virtual void GenDeclRef( std::stringstream& ss ) const override
    {
        ss << GenSlidingWindowDeclRef();
    }
    virtual void GenSlidingWindowDecl( std::stringstream& ss ) const override
    {
        GenDecl(ss);
    }
    virtual std::string GenSlidingWindowDeclRef( bool = false ) const override
    {
        std::stringstream ss;
        if (GetFormulaToken()->GetType() != formula::svString)
            throw Unhandled(__FILE__, __LINE__);
        FormulaToken* Tok = GetFormulaToken();
        ss << Tok->GetString().getString().toAsciiUpperCase().hashCode() << "U";
        return ss.str();
    }
    virtual size_t GetWindowSize() const override
    {
        return 1;
    }
    /// Pass the 32-bit hash of the string to the kernel
    virtual size_t Marshal( cl_kernel k, int argno, int, cl_program ) override
    {
        FormulaToken* ref = mFormulaTree->GetFormulaToken();
        cl_uint hashCode = 0;
        if (ref->GetType() != formula::svString)
        {
            throw Unhandled(__FILE__, __LINE__);
        }

        const OUString s = ref->GetString().getString().toAsciiUpperCase();
        hashCode = s.hashCode();

        // Pass the scalar result back to the rest of the formula kernel
        SAL_INFO("sc.opencl", "Kernel " << k << " arg " << argno << ": cl_uint: " << hashCode << "(" << DebugPeekData(ref) << ")" );
        cl_int err = clSetKernelArg(k, argno, sizeof(cl_uint), static_cast<void*>(&hashCode));
        if (CL_SUCCESS != err)
            throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
        return 1;
    }
};

/// Arguments that are actually compile-time constants
class DynamicKernelConstantArgument : public DynamicKernelArgument
{
public:
    DynamicKernelConstantArgument( const ScCalcConfig& config, const std::string& s,
        const FormulaTreeNodeRef& ft ) :
        DynamicKernelArgument(config, s, ft) { }
    /// Generate declaration
    virtual void GenDecl( std::stringstream& ss ) const override
    {
        ss << "double " << mSymName;
    }
    virtual void GenDeclRef( std::stringstream& ss ) const override
    {
        ss << mSymName;
    }
    virtual void GenSlidingWindowDecl( std::stringstream& ss ) const override
    {
        GenDecl(ss);
    }
    virtual std::string GenSlidingWindowDeclRef( bool = false ) const override
    {
        if (GetFormulaToken()->GetType() != formula::svDouble)
            throw Unhandled(__FILE__, __LINE__);
        return mSymName;
    }
    virtual size_t GetWindowSize() const override
    {
        return 1;
    }
    double GetDouble() const
    {
        FormulaToken* Tok = GetFormulaToken();
        if (Tok->GetType() != formula::svDouble)
            throw Unhandled(__FILE__, __LINE__);
        return Tok->GetDouble();
    }
    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal( cl_kernel k, int argno, int, cl_program ) override
    {
        double tmp = GetDouble();
        // Pass the scalar result back to the rest of the formula kernel
        SAL_INFO("sc.opencl", "Kernel " << k << " arg " << argno << ": double: " << tmp);
        cl_int err = clSetKernelArg(k, argno, sizeof(double), static_cast<void*>(&tmp));
        if (CL_SUCCESS != err)
            throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
        return 1;
    }
};

class DynamicKernelPiArgument : public DynamicKernelArgument
{
public:
    DynamicKernelPiArgument( const ScCalcConfig& config, const std::string& s,
        const FormulaTreeNodeRef& ft ) :
        DynamicKernelArgument(config, s, ft) { }
    /// Generate declaration
    virtual void GenDecl( std::stringstream& ss ) const override
    {
        ss << "double " << mSymName;
    }
    virtual void GenDeclRef( std::stringstream& ss ) const override
    {
        ss << "3.14159265358979";
    }
    virtual void GenSlidingWindowDecl( std::stringstream& ss ) const override
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
        double tmp = 0.0;
        // Pass the scalar result back to the rest of the formula kernel
        SAL_INFO("sc.opencl", "Kernel " << k << " arg " << argno << ": double: " << tmp << " (PI)");
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
    virtual void GenDecl( std::stringstream& ss ) const override
    {
        ss << "double " << mSymName;
    }
    virtual void GenDeclRef( std::stringstream& ss ) const override
    {
        ss << mSymName;
    }
    virtual void GenSlidingWindowDecl( std::stringstream& ss ) const override
    {
        ss << "int " << mSymName;
    }
    virtual std::string GenSlidingWindowDeclRef( bool = false ) const override
    {
        return mSymName + "_Random(" + mSymName + ")";
    }
    virtual void GenSlidingWindowFunction( std::stringstream& ss ) override
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
        cl_int seed = comphelper::rng::uniform_int_distribution(0, SAL_MAX_INT32);
        // Pass the scalar result back to the rest of the formula kernel
        SAL_INFO("sc.opencl", "Kernel " << k << " arg " << argno << ": cl_int: " << seed << "(RANDOM)");
        cl_int err = clSetKernelArg(k, argno, sizeof(cl_int), static_cast<void*>(&seed));
        if (CL_SUCCESS != err)
            throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
        return 1;
    }
};

/// A vector of strings
class DynamicKernelStringArgument : public VectorRef
{
public:
    DynamicKernelStringArgument( const ScCalcConfig& config, const std::string& s,
        const FormulaTreeNodeRef& ft, int index = 0 ) :
        VectorRef(config, s, ft, index) { }

    virtual void GenSlidingWindowFunction( std::stringstream& ) override { }
    /// Generate declaration
    virtual void GenDecl( std::stringstream& ss ) const override
    {
        ss << "__global unsigned int *" << mSymName;
    }
    virtual void GenSlidingWindowDecl( std::stringstream& ss ) const override
    {
        DynamicKernelStringArgument::GenDecl(ss);
    }
    virtual size_t Marshal( cl_kernel, int, int, cl_program ) override;
};

/// Marshal a string vector reference
size_t DynamicKernelStringArgument::Marshal( cl_kernel k, int argno, int, cl_program )
{
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
    size_t szHostBuffer = nStrings * sizeof(cl_int);
    cl_uint* pHashBuffer = nullptr;

    if (vRef.mpStringArray != nullptr)
    {
        // Marshal strings. Right now we pass hashes of these string
        mpClmem = clCreateBuffer(kEnv.mpkContext,
            cl_mem_flags(CL_MEM_READ_ONLY) | CL_MEM_ALLOC_HOST_PTR,
            szHostBuffer, nullptr, &err);
        if (CL_SUCCESS != err)
            throw OpenCLError("clCreateBuffer", err, __FILE__, __LINE__);
        SAL_INFO("sc.opencl", "Created buffer " << mpClmem << " size " << szHostBuffer);

        pHashBuffer = static_cast<cl_uint*>(clEnqueueMapBuffer(
            kEnv.mpkCmdQueue, mpClmem, CL_TRUE, CL_MAP_WRITE, 0,
            szHostBuffer, 0, nullptr, nullptr, &err));
        if (CL_SUCCESS != err)
            throw OpenCLError("clEnqueueMapBuffer", err, __FILE__, __LINE__);

        for (size_t i = 0; i < nStrings; i++)
        {
            if (vRef.mpStringArray[i])
            {
                const OUString tmp = OUString(vRef.mpStringArray[i]);
                pHashBuffer[i] = tmp.hashCode();
            }
            else
            {
                pHashBuffer[i] = 0;
            }
        }
    }
    else
    {
        if (nStrings == 0)
            szHostBuffer = sizeof(cl_int); // a dummy small value
                                           // Marshal as a buffer of NANs
        mpClmem = clCreateBuffer(kEnv.mpkContext,
            cl_mem_flags(CL_MEM_READ_ONLY) | CL_MEM_ALLOC_HOST_PTR,
            szHostBuffer, nullptr, &err);
        if (CL_SUCCESS != err)
            throw OpenCLError("clCreateBuffer", err, __FILE__, __LINE__);
        SAL_INFO("sc.opencl", "Created buffer " << mpClmem << " size " << szHostBuffer);

        pHashBuffer = static_cast<cl_uint*>(clEnqueueMapBuffer(
            kEnv.mpkCmdQueue, mpClmem, CL_TRUE, CL_MAP_WRITE, 0,
            szHostBuffer, 0, nullptr, nullptr, &err));
        if (CL_SUCCESS != err)
            throw OpenCLError("clEnqueueMapBuffer", err, __FILE__, __LINE__);

        for (size_t i = 0; i < szHostBuffer / sizeof(cl_int); i++)
            pHashBuffer[i] = 0;
    }
    err = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue, mpClmem,
        pHashBuffer, 0, nullptr, nullptr);
    if (CL_SUCCESS != err)
        throw OpenCLError("clEnqueueUnmapMemObject", err, __FILE__, __LINE__);

    SAL_INFO("sc.opencl", "Kernel " << k << " arg " << argno << ": cl_mem: " << mpClmem << " (" << DebugPeekData(ref,mnIndex) << ")");
    err = clSetKernelArg(k, argno, sizeof(cl_mem), static_cast<void*>(&mpClmem));
    if (CL_SUCCESS != err)
        throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
    return 1;
}

/// A mixed string/numberic vector
class DynamicKernelMixedArgument : public VectorRef
{
public:
    DynamicKernelMixedArgument( const ScCalcConfig& config, const std::string& s,
        const FormulaTreeNodeRef& ft ) :
        VectorRef(config, s, ft), mStringArgument(config, s + "s", ft) { }
    virtual void GenSlidingWindowDecl( std::stringstream& ss ) const override
    {
        VectorRef::GenSlidingWindowDecl(ss);
        ss << ", ";
        mStringArgument.GenSlidingWindowDecl(ss);
    }
    virtual void GenSlidingWindowFunction( std::stringstream& ) override { }
    /// Generate declaration
    virtual void GenDecl( std::stringstream& ss ) const override
    {
        VectorRef::GenDecl(ss);
        ss << ", ";
        mStringArgument.GenDecl(ss);
    }
    virtual void GenDeclRef( std::stringstream& ss ) const override
    {
        VectorRef::GenDeclRef(ss);
        ss << ",";
        mStringArgument.GenDeclRef(ss);
    }
    virtual std::string GenSlidingWindowDeclRef( bool nested ) const override
    {
        std::stringstream ss;
        ss << "(!isnan(" << VectorRef::GenSlidingWindowDeclRef();
        ss << ")?" << VectorRef::GenSlidingWindowDeclRef();
        ss << ":" << mStringArgument.GenSlidingWindowDeclRef(nested);
        ss << ")";
        return ss.str();
    }
    virtual std::string GenDoubleSlidingWindowDeclRef( bool = false ) const override
    {
        std::stringstream ss;
        ss << VectorRef::GenSlidingWindowDeclRef();
        return ss.str();
    }
    virtual std::string GenStringSlidingWindowDeclRef( bool = false ) const override
    {
        std::stringstream ss;
        ss << mStringArgument.GenSlidingWindowDeclRef();
        return ss.str();
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

/// Handling a Double Vector that is used as a sliding window input
/// to either a sliding window average or sum-of-products
/// Generate a sequential loop for reductions
class OpAverage;
class OpCount;

template<class Base>
class DynamicKernelSlidingArgument : public Base
{
public:
    DynamicKernelSlidingArgument(const ScCalcConfig& config, const std::string& s,
                                 const FormulaTreeNodeRef& ft,
                                 const std::shared_ptr<SlidingFunctionBase>& CodeGen, int index)
        : Base(config, s, ft, index)
        , mpCodeGen(CodeGen)
    {
        FormulaToken* t = ft->GetFormulaToken();
        if (t->GetType() != formula::svDoubleVectorRef)
            throw Unhandled(__FILE__, __LINE__);
        mpDVR = static_cast<const formula::DoubleVectorRefToken*>(t);
        bIsStartFixed = mpDVR->IsStartFixed();
        bIsEndFixed = mpDVR->IsEndFixed();
    }

    // Should only be called by SumIfs. Yikes!
    virtual bool NeedParallelReduction() const
    {
        assert(dynamic_cast<OpSumIfs*>(mpCodeGen.get()));
        return GetWindowSize() > 100 &&
               ((GetStartFixed() && GetEndFixed()) ||
            (!GetStartFixed() && !GetEndFixed()));
    }

    virtual void GenSlidingWindowFunction( std::stringstream& ) { }

    std::string GenSlidingWindowDeclRef( bool nested = false ) const
    {
        size_t nArrayLength = mpDVR->GetArrayLength();
        std::stringstream ss;
        if (!bIsStartFixed && !bIsEndFixed)
        {
            if (nested)
                ss << "((i+gid0) <" << nArrayLength << "?";
            ss << Base::GetName() << "[i + gid0]";
            if (nested)
                ss << ":NAN)";
        }
        else
        {
            if (nested)
                ss << "(i <" << nArrayLength << "?";
            ss << Base::GetName() << "[i]";
            if (nested)
                ss << ":NAN)";
        }
        return ss.str();
    }
    /// Controls how the elements in the DoubleVectorRef are traversed
    size_t GenReductionLoopHeader(
        std::stringstream& ss, bool& needBody )
    {
        assert(mpDVR);
        size_t nCurWindowSize = mpDVR->GetRefRowSize();

        {
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
                std::stringstream temp1, temp2;
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
                std::stringstream temp1, temp2;
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
    }

    size_t GetArrayLength() const { return mpDVR->GetArrayLength(); }

    size_t GetWindowSize() const { return mpDVR->GetRefRowSize(); }

    size_t GetStartFixed() const { return bIsStartFixed; }

    size_t GetEndFixed() const { return bIsEndFixed; }

protected:
    bool bIsStartFixed, bIsEndFixed;
    const formula::DoubleVectorRefToken* mpDVR;
    // from parent nodes
    std::shared_ptr<SlidingFunctionBase> mpCodeGen;
};

/// A mixed string/numberic vector
class DynamicKernelMixedSlidingArgument : public VectorRef
{
public:
    DynamicKernelMixedSlidingArgument( const ScCalcConfig& config, const std::string& s,
        const FormulaTreeNodeRef& ft, const std::shared_ptr<SlidingFunctionBase>& CodeGen,
        int index ) :
        VectorRef(config, s, ft),
        mDoubleArgument(mCalcConfig, s, ft, CodeGen, index),
        mStringArgument(mCalcConfig, s + "s", ft, CodeGen, index) { }
    virtual void GenSlidingWindowDecl( std::stringstream& ss ) const override
    {
        mDoubleArgument.GenSlidingWindowDecl(ss);
        ss << ", ";
        mStringArgument.GenSlidingWindowDecl(ss);
    }
    virtual void GenSlidingWindowFunction( std::stringstream& ) override { }
    /// Generate declaration
    virtual void GenDecl( std::stringstream& ss ) const override
    {
        mDoubleArgument.GenDecl(ss);
        ss << ", ";
        mStringArgument.GenDecl(ss);
    }
    virtual void GenDeclRef( std::stringstream& ss ) const override
    {
        mDoubleArgument.GenDeclRef(ss);
        ss << ",";
        mStringArgument.GenDeclRef(ss);
    }
    virtual std::string GenSlidingWindowDeclRef( bool nested ) const override
    {
        std::stringstream ss;
        ss << "(!isnan(" << mDoubleArgument.GenSlidingWindowDeclRef();
        ss << ")?" << mDoubleArgument.GenSlidingWindowDeclRef();
        ss << ":" << mStringArgument.GenSlidingWindowDeclRef(nested);
        ss << ")";
        return ss.str();
    }
    virtual std::string GenDoubleSlidingWindowDeclRef( bool = false ) const override
    {
        std::stringstream ss;
        ss << mDoubleArgument.GenSlidingWindowDeclRef();
        return ss.str();
    }
    virtual std::string GenStringSlidingWindowDeclRef( bool = false ) const override
    {
        std::stringstream ss;
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
                                            SlidingFunctionBase* pCodeGen, int nResultSize);
    /// Used to generate sliding window helpers
    void DumpSlidingWindowFunctions( std::stringstream& ss )
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

/// Handling a Double Vector that is used as a sliding window input
/// Performs parallel reduction based on given operator
template<class Base>
class ParallelReductionVectorRef : public Base
{
public:
    ParallelReductionVectorRef(const ScCalcConfig& config, const std::string& s,
                               const FormulaTreeNodeRef& ft,
                               const std::shared_ptr<SlidingFunctionBase>& CodeGen, int index)
        : Base(config, s, ft, index)
        , mpCodeGen(CodeGen)
        , mpClmem2(nullptr)
    {
        FormulaToken* t = ft->GetFormulaToken();
        if (t->GetType() != formula::svDoubleVectorRef)
            throw Unhandled(__FILE__, __LINE__);
        mpDVR = static_cast<const formula::DoubleVectorRefToken*>(t);
        bIsStartFixed = mpDVR->IsStartFixed();
        bIsEndFixed = mpDVR->IsEndFixed();
    }

    /// Emit the definition for the auxiliary reduction kernel
    virtual void GenSlidingWindowFunction( std::stringstream& ss )
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

    virtual std::string GenSlidingWindowDeclRef( bool ) const
    {
        std::stringstream ss;
        if (!bIsStartFixed && !bIsEndFixed)
            ss << Base::GetName() << "[i + gid0]";
        else
            ss << Base::GetName() << "[i]";
        return ss.str();
    }

    /// Controls how the elements in the DoubleVectorRef are traversed
    size_t GenReductionLoopHeader(
        std::stringstream& ss, int nResultSize, bool& needBody )
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

    virtual size_t Marshal( cl_kernel k, int argno, int w, cl_program mpProgram )
    {
        assert(Base::mpClmem == nullptr);

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
        SAL_INFO("sc.opencl", "Enqueing kernel " << redKernel);
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
            SAL_INFO("sc.opencl", "Enqueing kernel " << redKernel);
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

    ~ParallelReductionVectorRef()
    {
        if (mpClmem2)
        {
            cl_int err;
            err = clReleaseMemObject(mpClmem2);
            SAL_WARN_IF(err != CL_SUCCESS, "sc.opencl", "clReleaseMemObject failed: " << openclwrapper::errorString(err));
            mpClmem2 = nullptr;
        }
    }

    size_t GetArrayLength() const { return mpDVR->GetArrayLength(); }

    size_t GetWindowSize() const { return mpDVR->GetRefRowSize(); }

    bool GetStartFixed() const { return bIsStartFixed; }

    bool GetEndFixed() const { return bIsEndFixed; }

protected:
    bool bIsStartFixed, bIsEndFixed;
    const formula::DoubleVectorRefToken* mpDVR;
    // from parent nodes
    std::shared_ptr<SlidingFunctionBase> mpCodeGen;
    // controls whether to invoke the reduction kernel during marshaling or not
    cl_mem mpClmem2;
};

class Reduction : public SlidingFunctionBase
{
    int const mnResultSize;
public:
    explicit Reduction(int nResultSize) : mnResultSize(nResultSize) {}

    typedef DynamicKernelSlidingArgument<VectorRef> NumericRange;
    typedef DynamicKernelSlidingArgument<DynamicKernelStringArgument> StringRange;
    typedef ParallelReductionVectorRef<VectorRef> ParallelNumericRange;

    virtual bool HandleNaNArgument( std::stringstream&, unsigned, SubArguments& ) const
    {
        return false;
    }

    virtual void GenSlidingWindowFunction( std::stringstream& ss,
        const std::string& sSymName, SubArguments& vSubArguments ) override
    {
        ss << "\ndouble " << sSymName;
        ss << "_" << BinFuncName() << "(";
        for (size_t i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ", ";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n";
        ss << "double tmp = " << GetBottom() << ";\n";
        ss << "int gid0 = get_global_id(0);\n";
        if (isAverage() || isMinOrMax())
            ss << "int nCount = 0;\n";
        ss << "double tmpBottom;\n";
        unsigned i = vSubArguments.size();
        while (i--)
        {
            if (NumericRange* NR =
                dynamic_cast<NumericRange*>(vSubArguments[i].get()))
            {
                bool needBody; NR->GenReductionLoopHeader(ss, needBody); if (!needBody)
                    continue;
            }
            else if (ParallelNumericRange* PNR =
                dynamic_cast<ParallelNumericRange*>(vSubArguments[i].get()))
            {
                //did not handle yet
                bool bNeedBody = false;
                PNR->GenReductionLoopHeader(ss, mnResultSize, bNeedBody);
                if (!bNeedBody)
                    continue;
            }
            else if (StringRange* SR =
                dynamic_cast<StringRange*>(vSubArguments[i].get()))
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
                assert(pCur);
                assert(pCur->GetType() != formula::svDoubleVectorRef);

                if (pCur->GetType() == formula::svSingleVectorRef ||
                    pCur->GetType() == formula::svDouble)
                {
                    ss << "{\n";
                }
            }
            if (ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
            {
                bool bNanHandled = HandleNaNArgument(ss, i, vSubArguments);

                ss << "tmpBottom = " << GetBottom() << ";\n";

                if (!bNanHandled)
                {
                    ss << "if (isnan(";
                    ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                    ss << "))\n";
                    if (ZeroReturnZero())
                        ss << "    return 0;\n";
                    else
                    {
                        ss << "    tmp = ";
                        ss << Gen2("tmpBottom", "tmp") << ";\n";
                    }
                    ss << "else\n";
                }
                ss << "{";
                ss << "        tmp = ";
                ss << Gen2(vSubArguments[i]->GenSlidingWindowDeclRef(), "tmp");
                ss << ";\n";
                ss << "    }\n";
                ss << "}\n";
            }
            else
            {
                ss << "tmp = ";
                ss << Gen2(vSubArguments[i]->GenSlidingWindowDeclRef(), "tmp");
                ss << ";\n";
            }
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
            ss << "*pow((double)nCount,-1.0)";
        ss << ";\n}";
    }
    virtual bool isAverage() const { return false; }
    virtual bool isMinOrMax() const { return false; }
    virtual bool takeString() const override { return false; }
    virtual bool takeNumeric() const override { return true; }
};

// Strictly binary operators
class Binary : public SlidingFunctionBase
{
public:
    virtual void GenSlidingWindowFunction( std::stringstream& ss,
        const std::string& sSymName, SubArguments& vSubArguments ) override
    {
        ss << "\ndouble " << sSymName;
        ss << "_" << BinFuncName() << "(";
        assert(vSubArguments.size() == 2);
        for (size_t i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ", ";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss << "int gid0 = get_global_id(0), i = 0;\n\t";
        ss << "double tmp = ";
        ss << Gen2(vSubArguments[0]->GenSlidingWindowDeclRef(),
            vSubArguments[1]->GenSlidingWindowDeclRef()) << ";\n\t";
        ss << "return tmp;\n}";
    }
    virtual bool takeString() const override { return true; }
    virtual bool takeNumeric() const override { return true; }
};

class SumOfProduct : public SlidingFunctionBase
{
public:
    virtual void GenSlidingWindowFunction( std::stringstream& ss,
        const std::string& sSymName, SubArguments& vSubArguments ) override
    {
        size_t nCurWindowSize = 0;
        FormulaToken* tmpCur = nullptr;
        const formula::DoubleVectorRefToken* pCurDVR = nullptr;
        ss << "\ndouble " << sSymName;
        ss << "_" << BinFuncName() << "(";
        for (size_t i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
            size_t nCurChildWindowSize = vSubArguments[i]->GetWindowSize();
            nCurWindowSize = (nCurWindowSize < nCurChildWindowSize) ?
                nCurChildWindowSize : nCurWindowSize;
            tmpCur = vSubArguments[i]->GetFormulaToken();
            if (ocPush == tmpCur->GetOpCode())
            {

                pCurDVR = static_cast<const formula::DoubleVectorRefToken*>(tmpCur);
                if (pCurDVR->IsStartFixed() != pCurDVR->IsEndFixed())
                    throw Unhandled(__FILE__, __LINE__);
            }
        }
        ss << ") {\n";
        ss << "    double tmp = 0.0;\n";
        ss << "    int gid0 = get_global_id(0);\n";

        ss << "\tint i;\n\t";
        ss << "int currentCount0;\n";
        for (size_t i = 0; i < vSubArguments.size() - 1; i++)
            ss << "int currentCount" << i + 1 << ";\n";
        std::stringstream temp3, temp4;
        int outLoopSize = UNROLLING_FACTOR;
        if (nCurWindowSize / outLoopSize != 0)
        {
            ss << "for(int outLoop=0; outLoop<" <<
                nCurWindowSize / outLoopSize << "; outLoop++){\n\t";
            for (int count = 0; count < outLoopSize; count++)
            {
                ss << "i = outLoop*" << outLoopSize << "+" << count << ";\n";
                if (count == 0)
                {
                    for (size_t i = 0; i < vSubArguments.size(); i++)
                    {
                        tmpCur = vSubArguments[i]->GetFormulaToken();
                        if (ocPush == tmpCur->GetOpCode())
                        {
                            pCurDVR = static_cast<const formula::DoubleVectorRefToken*>(tmpCur);
                            if (!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
                            {
                                temp3 << "        currentCount";
                                temp3 << i;
                                temp3 << " =i+gid0+1;\n";
                            }
                            else
                            {
                                temp3 << "        currentCount";
                                temp3 << i;
                                temp3 << " =i+1;\n";
                            }
                        }
                    }

                    temp3 << "tmp = fsum(";
                    for (size_t i = 0; i < vSubArguments.size(); i++)
                    {
                        if (i)
                            temp3 << "*";
                        if (ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
                        {
                            temp3 << "(";
                            temp3 << "(currentCount";
                            temp3 << i;
                            temp3 << ">";
                            if (vSubArguments[i]->GetFormulaToken()->GetType() ==
                                    formula::svSingleVectorRef)
                            {
                                const formula::SingleVectorRefToken* pSVR =
                                    static_cast<const formula::SingleVectorRefToken*>
                                    (vSubArguments[i]->GetFormulaToken());
                                temp3 << pSVR->GetArrayLength();
                                temp3 << ")||isnan(" << vSubArguments[i]
                                    ->GenSlidingWindowDeclRef();
                                temp3 << ")?0:";
                                temp3 << vSubArguments[i]->GenSlidingWindowDeclRef();
                                temp3  << ")";
                            }
                            else if (vSubArguments[i]->GetFormulaToken()->GetType() ==
                                    formula::svDoubleVectorRef)
                            {
                                const formula::DoubleVectorRefToken* pSVR =
                                    static_cast<const formula::DoubleVectorRefToken*>
                                    (vSubArguments[i]->GetFormulaToken());
                                temp3 << pSVR->GetArrayLength();
                                temp3 << ")||isnan(" << vSubArguments[i]
                                    ->GenSlidingWindowDeclRef(true);
                                temp3 << ")?0:";
                                temp3 << vSubArguments[i]->GenSlidingWindowDeclRef(true);
                                temp3  << ")";
                            }

                        }
                        else
                            temp3 << vSubArguments[i]->GenSlidingWindowDeclRef(true);
                    }
                    temp3 << ", tmp);\n\t";
                }
                ss << temp3.str();
            }
            ss << "}\n\t";
        }
        //The residual of mod outLoopSize
        for (size_t count = nCurWindowSize / outLoopSize * outLoopSize;
            count < nCurWindowSize; count++)
        {
            ss << "i =" << count << ";\n";
            if (count == nCurWindowSize / outLoopSize * outLoopSize)
            {
                for (size_t i = 0; i < vSubArguments.size(); i++)
                {
                    tmpCur = vSubArguments[i]->GetFormulaToken();
                    if (ocPush == tmpCur->GetOpCode())
                    {
                        pCurDVR = static_cast<const formula::DoubleVectorRefToken*>(tmpCur);
                        if (!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
                        {
                            temp4 << "        currentCount";
                            temp4 << i;
                            temp4 << " =i+gid0+1;\n";
                        }
                        else
                        {
                            temp4 << "        currentCount";
                            temp4 << i;
                            temp4 << " =i+1;\n";
                        }
                    }
                }

                temp4 << "tmp = fsum(";
                for (size_t i = 0; i < vSubArguments.size(); i++)
                {
                    if (i)
                        temp4 << "*";
                    if (ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
                    {
                        temp4 << "(";
                        temp4 << "(currentCount";
                        temp4 << i;
                        temp4 << ">";
                        if (vSubArguments[i]->GetFormulaToken()->GetType() ==
                                formula::svSingleVectorRef)
                        {
                            const formula::SingleVectorRefToken* pSVR =
                                static_cast<const formula::SingleVectorRefToken*>
                                (vSubArguments[i]->GetFormulaToken());
                            temp4 << pSVR->GetArrayLength();
                            temp4 << ")||isnan(" << vSubArguments[i]
                                ->GenSlidingWindowDeclRef();
                            temp4 << ")?0:";
                            temp4 << vSubArguments[i]->GenSlidingWindowDeclRef();
                            temp4  << ")";
                        }
                        else if (vSubArguments[i]->GetFormulaToken()->GetType() ==
                                formula::svDoubleVectorRef)
                        {
                            const formula::DoubleVectorRefToken* pSVR =
                                static_cast<const formula::DoubleVectorRefToken*>
                                (vSubArguments[i]->GetFormulaToken());
                            temp4 << pSVR->GetArrayLength();
                            temp4 << ")||isnan(" << vSubArguments[i]
                                ->GenSlidingWindowDeclRef(true);
                            temp4 << ")?0:";
                            temp4 << vSubArguments[i]->GenSlidingWindowDeclRef(true);
                            temp4  << ")";
                        }

                    }
                    else
                    {
                        temp4 << vSubArguments[i]
                            ->GenSlidingWindowDeclRef(true);
                    }
                }
                temp4 << ", tmp);\n\t";
            }
            ss << temp4.str();
        }
        ss << "return tmp;\n";
        ss << "}";
    }
    virtual bool takeString() const override { return false; }
    virtual bool takeNumeric() const override { return true; }
};

/// operator traits
class OpNop : public Reduction
{
public:
    explicit OpNop(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& ) const override
    {
        return lhs;
    }
    virtual std::string BinFuncName() const override { return "nop"; }
};

class OpCount : public Reduction
{
public:
    explicit OpCount(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        std::stringstream ss;
        ss << "(isnan(" << lhs << ")?" << rhs << ":" << rhs << "+1.0)";
        return ss.str();
    }
    virtual std::string BinFuncName() const override { return "fcount"; }
    virtual bool canHandleMultiVector() const override { return true; }
};

class OpEqual : public Binary
{
public:
    virtual std::string GetBottom() override { return "0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        std::stringstream ss;
        ss << "strequal(" << lhs << "," << rhs << ")";
        return ss.str();
    }
    virtual std::string BinFuncName() const override { return "eq"; }
};

class OpLessEqual : public Binary
{
public:
    virtual std::string GetBottom() override { return "0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        std::stringstream ss;
        ss << "(" << lhs << "<=" << rhs << ")";
        return ss.str();
    }
    virtual std::string BinFuncName() const override { return "leq"; }
};

class OpLess : public Binary
{
public:
    virtual std::string GetBottom() override { return "0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        std::stringstream ss;
        ss << "(" << lhs << "<" << rhs << ")";
        return ss.str();
    }
    virtual std::string BinFuncName() const override { return "less"; }
};

class OpGreater : public Binary
{
public:
    virtual std::string GetBottom() override { return "0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        std::stringstream ss;
        ss << "(" << lhs << ">" << rhs << ")";
        return ss.str();
    }
    virtual std::string BinFuncName() const override { return "gt"; }
};

class OpSum : public Reduction
{
public:
    explicit OpSum(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        std::stringstream ss;
        ss << "fsum_approx((" << lhs << "),(" << rhs << "))";
        return ss.str();
    }
    virtual std::string BinFuncName() const override { return "fsum"; }
    // All arguments are simply summed, so it doesn't matter if SvDoubleVector is split.
    virtual bool canHandleMultiVector() const override { return true; }
};

class OpAverage : public Reduction
{
public:
    explicit OpAverage(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        std::stringstream ss;
        ss << "fsum_count(" << lhs << "," << rhs << ", &nCount)";
        return ss.str();
    }
    virtual std::string BinFuncName() const override { return "average"; }
    virtual bool isAverage() const override { return true; }
    virtual bool canHandleMultiVector() const override { return true; }
};

class OpSub : public Reduction
{
public:
    explicit OpSub(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        return "fsub_approx(" + lhs + "," + rhs + ")";
    }
    virtual std::string BinFuncName() const override { return "fsub"; }
};

class OpMul : public Reduction
{
public:
    explicit OpMul(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "1"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        return lhs + "*" + rhs;
    }
    virtual std::string BinFuncName() const override { return "fmul"; }
    virtual bool ZeroReturnZero() override { return true; }
};

/// Technically not a reduction, but fits the framework.
class OpDiv : public Reduction
{
public:
    explicit OpDiv(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "1.0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        return "(" + rhs + "==0 ? CreateDoubleError(DivisionByZero) : (" + lhs + "/" + rhs + ") )";
    }
    virtual std::string BinFuncName() const override { return "fdiv"; }

    virtual bool HandleNaNArgument( std::stringstream& ss, unsigned argno, SubArguments& vSubArguments ) const override
    {
        if (argno == 1)
        {
            ss <<
                "if (isnan(" << vSubArguments[argno]->GenSlidingWindowDeclRef() << ")) {\n"
                "    return CreateDoubleError(DivisionByZero);\n"
                "}\n";
            return true;
        }
        else if (argno == 0)
        {
            ss <<
                "if (isnan(" << vSubArguments[argno]->GenSlidingWindowDeclRef() << ") &&\n"
                "    !(isnan(" << vSubArguments[1]->GenSlidingWindowDeclRef() << ") || " << vSubArguments[1]->GenSlidingWindowDeclRef() << " == 0)) {\n"
                "    return 0;\n"
                "}\n";
        }
        return false;
    }

};

class OpMin : public Reduction
{
public:
    explicit OpMin(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "NAN"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        return "fmin_count(" + lhs + "," + rhs + ", &nCount)";
    }
    virtual std::string BinFuncName() const override { return "min"; }
    virtual bool isMinOrMax() const override { return true; }
    virtual bool canHandleMultiVector() const override { return true; }
};

class OpMax : public Reduction
{
public:
    explicit OpMax(int nResultSize) : Reduction(nResultSize) {}

    virtual std::string GetBottom() override { return "NAN"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        return "fmax_count(" + lhs + "," + rhs + ", &nCount)";
    }
    virtual std::string BinFuncName() const override { return "max"; }
    virtual bool isMinOrMax() const override { return true; }
    virtual bool canHandleMultiVector() const override { return true; }
};

class OpSumProduct : public SumOfProduct
{
public:
    virtual std::string GetBottom() override { return "0"; }
    virtual std::string Gen2( const std::string& lhs, const std::string& rhs ) const override
    {
        return lhs + "*" + rhs;
    }
    virtual std::string BinFuncName() const override { return "fsop"; }
};
namespace {
struct SumIfsArgs
{
    explicit SumIfsArgs(cl_mem x) : mCLMem(x), mConst(0.0) { }
    explicit SumIfsArgs(double x) : mCLMem(nullptr), mConst(x) { }
    cl_mem mCLMem;
    double mConst;
};
}

/// Helper functions that have multiple buffers
class DynamicKernelSoPArguments : public DynamicKernelArgument
{
public:
    typedef std::vector<DynamicKernelArgumentRef> SubArgumentsType;

    DynamicKernelSoPArguments( const ScCalcConfig& config,
        const std::string& s, const FormulaTreeNodeRef& ft,
        SlidingFunctionBase* pCodeGen, int nResultSize );

    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal( cl_kernel k, int argno, int nVectorWidth, cl_program pProgram ) override
    {
        unsigned i = 0;
        for (const auto& rxSubArgument : mvSubArguments)
        {
            i += rxSubArgument->Marshal(k, argno + i, nVectorWidth, pProgram);
        }
        if (dynamic_cast<OpGeoMean*>(mpCodeGen.get()))
        {
            openclwrapper::KernelEnv kEnv;
            openclwrapper::setKernelEnv(&kEnv);
            cl_int err;
            cl_mem pClmem2;

            std::vector<cl_mem> vclmem;
            for (const auto& rxSubArgument : mvSubArguments)
            {
                if (VectorRef* VR = dynamic_cast<VectorRef*>(rxSubArgument.get()))
                    vclmem.push_back(VR->GetCLBuffer());
                else
                    vclmem.push_back(nullptr);
            }
            pClmem2 = clCreateBuffer(kEnv.mpkContext, CL_MEM_READ_WRITE,
                sizeof(double) * nVectorWidth, nullptr, &err);
            if (CL_SUCCESS != err)
                throw OpenCLError("clCreateBuffer", err, __FILE__, __LINE__);
            SAL_INFO("sc.opencl", "Created buffer " << pClmem2 << " size " << sizeof(double) << "*" << nVectorWidth << "=" << (sizeof(double)*nVectorWidth));

            std::string kernelName = "GeoMean_reduction";
            cl_kernel redKernel = clCreateKernel(pProgram, kernelName.c_str(), &err);
            if (err != CL_SUCCESS)
                throw OpenCLError("clCreateKernel", err, __FILE__, __LINE__);
            SAL_INFO("sc.opencl", "Created kernel " << redKernel << " with name " << kernelName << " in program " << pProgram);

            // set kernel arg of reduction kernel
            for (size_t j = 0; j < vclmem.size(); j++)
            {
                SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << j << ": " << (vclmem[j] ? "cl_mem" : "double") << ": " << vclmem[j]);
                err = clSetKernelArg(redKernel, j,
                    vclmem[j] ? sizeof(cl_mem) : sizeof(double),
                    static_cast<void*>(&vclmem[j]));
                if (CL_SUCCESS != err)
                    throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
            }
            SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << vclmem.size() << ": cl_mem: " << pClmem2);
            err = clSetKernelArg(redKernel, vclmem.size(), sizeof(cl_mem), static_cast<void*>(&pClmem2));
            if (CL_SUCCESS != err)
                throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);

            // set work group size and execute
            size_t global_work_size[] = { 256, static_cast<size_t>(nVectorWidth) };
            size_t const local_work_size[] = { 256, 1 };
            SAL_INFO("sc.opencl", "Enqueing kernel " << redKernel);
            err = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, redKernel, 2, nullptr,
                global_work_size, local_work_size, 0, nullptr, nullptr);
            if (CL_SUCCESS != err)
                throw OpenCLError("clEnqueueNDRangeKernel", err, __FILE__, __LINE__);
            err = clFinish(kEnv.mpkCmdQueue);
            if (CL_SUCCESS != err)
                throw OpenCLError("clFinish", err, __FILE__, __LINE__);

            // Pass pClmem2 to the "real" kernel
            SAL_INFO("sc.opencl", "Kernel " << k << " arg " << argno << ": cl_mem: " << pClmem2);
            err = clSetKernelArg(k, argno, sizeof(cl_mem), static_cast<void*>(&pClmem2));
            if (CL_SUCCESS != err)
                throw OpenCLError("clSetKernelArg", err, __FILE__, __LINE__);
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
                        SAL_INFO("sc.opencl", "Kernel " << redKernel << " arg " << j << ": double: " << vclmem[j].mConst);
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
                SAL_INFO("sc.opencl", "Enqueing kernel " << redKernel);
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

    virtual void GenSlidingWindowFunction( std::stringstream& ss ) override
    {
        for (DynamicKernelArgumentRef & rArg : mvSubArguments)
            rArg->GenSlidingWindowFunction(ss);
        mpCodeGen->GenSlidingWindowFunction(ss, mSymName, mvSubArguments);
    }
    virtual void GenDeclRef( std::stringstream& ss ) const override
    {
        for (size_t i = 0; i < mvSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            mvSubArguments[i]->GenDeclRef(ss);
        }
    }
    virtual void GenDecl( std::stringstream& ss ) const override
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
    virtual void GenSlidingWindowDecl( std::stringstream& ss ) const override
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
        std::stringstream ss;
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
            t = t + rSubArgument->DumpOpName();
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

static DynamicKernelArgumentRef SoPHelper( const ScCalcConfig& config,
    const std::string& ts, const FormulaTreeNodeRef& ft, SlidingFunctionBase* pCodeGen,
    int nResultSize )
{
    return DynamicKernelArgumentRef(new DynamicKernelSoPArguments(config, ts, ft, pCodeGen, nResultSize));
}

template<class Base>
static DynamicKernelArgument* VectorRefFactory( const ScCalcConfig& config, const std::string& s,
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
            return new DynamicKernelSlidingArgument<VectorRef>(config, s, ft, pCodeGen, index);
        return new DynamicKernelSlidingArgument<Base>(config, s, ft, pCodeGen, index);
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
        return new DynamicKernelSlidingArgument<Base>(config, s, ft, pCodeGen, index);
    }
    // Sub is not a reduction per se
    else if (dynamic_cast<OpSub*>(pCodeGen.get()))
    {
        return new DynamicKernelSlidingArgument<Base>(config, s, ft, pCodeGen, index);
    }
    // Only child class of Reduction is supported
    else if (!dynamic_cast<Reduction*>(pCodeGen.get()))
    {
        return new DynamicKernelSlidingArgument<Base>(config, s, ft, pCodeGen, index);
    }

    const formula::DoubleVectorRefToken* pDVR =
        static_cast<const formula::DoubleVectorRefToken*>(
        ft->GetFormulaToken());
    // Window being too small to justify a parallel reduction
    if (pDVR->GetRefRowSize() < REDUCE_THRESHOLD)
        return new DynamicKernelSlidingArgument<Base>(config, s, ft, pCodeGen, index);
    if (pDVR->IsStartFixed() == pDVR->IsEndFixed())
        return new ParallelReductionVectorRef<Base>(config, s, ft, pCodeGen, index);
    else // Other cases are not supported as well
        return new DynamicKernelSlidingArgument<Base>(config, s, ft, pCodeGen, index);
}

DynamicKernelSoPArguments::DynamicKernelSoPArguments(const ScCalcConfig& config,
    const std::string& s, const FormulaTreeNodeRef& ft, SlidingFunctionBase* pCodeGen, int nResultSize ) :
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
        std::stringstream tmpname;
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
                        if( pCodeGen->canHandleMultiVector())
                            SAL_INFO("sc.opencl", "multi-column DoubleRef");
                        else
                            throw UnhandledToken(("Function '" + pCodeGen->BinFuncName()
                                + "' cannot handle multi-column DoubleRef").c_str(), __FILE__, __LINE__);
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
                                DynamicKernelArgumentRef(
                                    new DynamicKernelMixedSlidingArgument(mCalcConfig,
                                        ts, ft->Children[i], mpCodeGen, j)));
                        }
                        else if (pDVR->GetArrays()[j].mpNumericArray &&
                            pCodeGen->takeNumeric() &&
                                 (AllStringsAreNull(pDVR->GetArrays()[j].mpStringArray, pDVR->GetArrayLength()) || mCalcConfig.meStringConversion == ScCalcConfig::StringConversion::ZERO))
                        {
                            // Function takes numbers, and either there
                            // are no strings, or there are strings but
                            // they are to be treated as zero
                            SAL_INFO("sc.opencl", "Numbers (no strings or strings treated as zero)");
                            mvSubArguments.push_back(
                                DynamicKernelArgumentRef(VectorRefFactory<VectorRef>(mCalcConfig,
                                        ts, ft->Children[i], mpCodeGen, j)));
                        }
                        else if (pDVR->GetArrays()[j].mpNumericArray == nullptr &&
                            pCodeGen->takeNumeric() &&
                            pDVR->GetArrays()[j].mpStringArray &&
                            mCalcConfig.meStringConversion == ScCalcConfig::StringConversion::ZERO)
                        {
                            // Function takes numbers, and there are only
                            // strings, but they are to be treated as zero
                            SAL_INFO("sc.opencl", "Only strings even if want numbers but should be treated as zero");
                            mvSubArguments.push_back(
                                DynamicKernelArgumentRef(VectorRefFactory<VectorRef>(mCalcConfig,
                                        ts, ft->Children[i], mpCodeGen, j)));
                        }
                        else if (pDVR->GetArrays()[j].mpStringArray &&
                            pCodeGen->takeString())
                        {
                            // There are strings, and the function takes strings.
                            SAL_INFO("sc.opencl", "Strings only");
                            mvSubArguments.push_back(
                                DynamicKernelArgumentRef(VectorRefFactory
                                    <DynamicKernelStringArgument>(mCalcConfig,
                                        ts, ft->Children[i], mpCodeGen, j)));
                        }
                        else if (AllStringsAreNull(pDVR->GetArrays()[j].mpStringArray, pDVR->GetArrayLength()) &&
                            pDVR->GetArrays()[j].mpNumericArray == nullptr)
                        {
                            // There are only empty cells. Push as an
                            // array of NANs
                            SAL_INFO("sc.opencl", "Only empty cells");
                            mvSubArguments.push_back(
                                DynamicKernelArgumentRef(VectorRefFactory<VectorRef>(mCalcConfig,
                                        ts, ft->Children[i], mpCodeGen, j)));
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
                            DynamicKernelArgumentRef(new DynamicKernelMixedArgument(mCalcConfig,
                                    ts, ft->Children[i])));
                    }
                    else if (pSVR->GetArray().mpNumericArray &&
                        pCodeGen->takeNumeric() &&
                             (AllStringsAreNull(pSVR->GetArray().mpStringArray, pSVR->GetArrayLength()) || mCalcConfig.meStringConversion == ScCalcConfig::StringConversion::ZERO))
                    {
                        // Function takes numbers, and either there
                        // are no strings, or there are strings but
                        // they are to be treated as zero
                        SAL_INFO("sc.opencl", "Numbers (no strings or strings treated as zero)");
                        mvSubArguments.push_back(
                            DynamicKernelArgumentRef(new VectorRef(mCalcConfig, ts,
                                    ft->Children[i])));
                    }
                    else if (pSVR->GetArray().mpNumericArray == nullptr &&
                        pCodeGen->takeNumeric() &&
                        pSVR->GetArray().mpStringArray &&
                        mCalcConfig.meStringConversion == ScCalcConfig::StringConversion::ZERO)
                    {
                        // Function takes numbers, and there are only
                        // strings, but they are to be treated as zero
                        SAL_INFO("sc.opencl", "Only strings even if want numbers but should be treated as zero");
                        mvSubArguments.push_back(
                            DynamicKernelArgumentRef(new VectorRef(mCalcConfig, ts,
                                    ft->Children[i])));
                    }
                    else if (pSVR->GetArray().mpStringArray &&
                        pCodeGen->takeString())
                    {
                        // There are strings, and the function takes strings.
                        SAL_INFO("sc.opencl", "Strings only");
                        mvSubArguments.push_back(
                            DynamicKernelArgumentRef(new DynamicKernelStringArgument(mCalcConfig,
                                    ts, ft->Children[i])));
                    }
                    else if (AllStringsAreNull(pSVR->GetArray().mpStringArray, pSVR->GetArrayLength()) &&
                        pSVR->GetArray().mpNumericArray == nullptr)
                    {
                        // There are only empty cells. Push as an
                        // array of NANs
                        SAL_INFO("sc.opencl", "Only empty cells");
                        mvSubArguments.push_back(
                            DynamicKernelArgumentRef(new VectorRef(mCalcConfig, ts,
                                    ft->Children[i])));
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
                        DynamicKernelArgumentRef(new DynamicKernelConstantArgument(mCalcConfig, ts,
                                ft->Children[i])));
                }
                else if (pChild->GetType() == formula::svString
                    && pCodeGen->takeString())
                {
                    SAL_INFO("sc.opencl", "Constant string case");
                    mvSubArguments.push_back(
                        DynamicKernelArgumentRef(new ConstStringArgument(mCalcConfig, ts,
                                ft->Children[i])));
                }
                else
                {
                    SAL_INFO("sc.opencl", "Unhandled operand, rejecting for OpenCL");
                    throw UnhandledToken(("unhandled operand " + StackVarEnumToString(pChild->GetType()) + " for ocPush").c_str(), __FILE__, __LINE__);
                }
                break;
            case ocDiv:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpDiv(nResultSize), nResultSize));
                break;
            case ocMul:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpMul(nResultSize), nResultSize));
                break;
            case ocSub:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpSub(nResultSize), nResultSize));
                break;
            case ocAdd:
            case ocSum:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpSum(nResultSize), nResultSize));
                break;
            case ocAverage:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpAverage(nResultSize), nResultSize));
                break;
            case ocMin:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpMin(nResultSize), nResultSize));
                break;
            case ocMax:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpMax(nResultSize), nResultSize));
                break;
            case ocCount:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpCount(nResultSize), nResultSize));
                break;
            case ocSumProduct:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpSumProduct, nResultSize));
                break;
            case ocIRR:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpIRR, nResultSize));
                break;
            case ocMIRR:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpMIRR, nResultSize));
                break;
            case ocPMT:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpPMT, nResultSize));
                break;
            case ocRate:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpIntrate, nResultSize));
                break;
            case ocRRI:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpRRI, nResultSize));
                break;
            case ocPpmt:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpPPMT, nResultSize));
                break;
            case ocFisher:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpFisher, nResultSize));
                break;
            case ocFisherInv:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpFisherInv, nResultSize));
                break;
            case ocGamma:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpGamma, nResultSize));
                break;
            case ocSLN:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpSLN, nResultSize));
                break;
            case ocGammaLn:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpGammaLn, nResultSize));
                break;
            case ocGauss:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpGauss, nResultSize));
                break;
            /*case ocGeoMean:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpGeoMean));
                break;*/
            case ocHarMean:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpHarMean, nResultSize));
                break;
            case ocLessEqual:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpLessEqual, nResultSize));
                break;
            case ocLess:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpLess, nResultSize));
                break;
            case ocEqual:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpEqual, nResultSize));
                break;
            case ocGreater:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpGreater, nResultSize));
                break;
            case ocSYD:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpSYD, nResultSize));
                break;
            case ocCorrel:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpCorrel, nResultSize));
                break;
            case ocCos:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpCos, nResultSize));
                break;
            case ocNegBinomVert :
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpNegbinomdist, nResultSize));
                break;
            case ocPearson:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpPearson, nResultSize));
                break;
            case ocRSQ:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpRsq, nResultSize));
                break;
            case ocCosecant:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpCsc, nResultSize));
                break;
            case ocISPMT:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpISPMT, nResultSize));
                break;
            case ocPDuration:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpPDuration, nResultSize));
                break;
            case ocSinHyp:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpSinh, nResultSize));
                break;
            case ocAbs:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpAbs, nResultSize));
                break;
            case ocPV:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpPV, nResultSize));
                break;
            case ocSin:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpSin, nResultSize));
                break;
            case ocTan:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpTan, nResultSize));
                break;
            case ocTanHyp:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpTanH, nResultSize));
                break;
            case ocStandard:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpStandard, nResultSize));
                break;
            case ocWeibull:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpWeibull, nResultSize));
                break;
            /*case ocMedian:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                         ft->Children[i],new OpMedian));
                break;*/
            case ocDDB:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpDDB, nResultSize));
                break;
            case ocFV:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpFV, nResultSize));
                break;
            case ocSumIfs:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpSumIfs, nResultSize));
                break;
                /*case ocVBD:
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                             ft->Children[i],new OpVDB));
                     break;*/
            case ocKurt:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpKurt, nResultSize));
                break;
                /*case ocNper:
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                            ft->Children[i], new OpNper));
                     break;*/
            case ocNormDist:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpNormdist, nResultSize));
                break;
            case ocArcCos:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpArcCos, nResultSize));
                break;
            case ocSqrt:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpSqrt, nResultSize));
                break;
            case ocArcCosHyp:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpArcCosHyp, nResultSize));
                break;
            case ocNPV:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpNPV, nResultSize));
                break;
            case ocStdNormDist:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpNormsdist, nResultSize));
                break;
            case ocNormInv:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpNorminv, nResultSize));
                break;
            case ocSNormInv:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpNormsinv, nResultSize));
                break;
            case ocPermut:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpPermut, nResultSize));
                break;
            case ocPermutationA:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpPermutationA, nResultSize));
                break;
            case ocPhi:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpPhi, nResultSize));
                break;
            case ocIpmt:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpIPMT, nResultSize));
                break;
            case ocConfidence:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpConfidence, nResultSize));
                break;
            case ocIntercept:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpIntercept, nResultSize));
                break;
            case ocDB:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                        new OpDB, nResultSize));
                break;
            case ocLogInv:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpLogInv, nResultSize));
                break;
            case ocArcCot:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpArcCot, nResultSize));
                break;
            case ocCosHyp:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpCosh, nResultSize));
                break;
            case ocCritBinom:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpCritBinom, nResultSize));
                break;
            case ocArcCotHyp:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpArcCotHyp, nResultSize));
                break;
            case ocArcSin:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpArcSin, nResultSize));
                break;
            case ocArcSinHyp:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpArcSinHyp, nResultSize));
                break;
            case ocArcTan:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpArcTan, nResultSize));
                break;
            case ocArcTanHyp:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpArcTanH, nResultSize));
                break;
            case ocBitAnd:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpBitAnd, nResultSize));
                break;
            case ocForecast:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpForecast, nResultSize));
                break;
            case ocLogNormDist:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpLogNormDist, nResultSize));
                break;
            /*case ocGammaDist:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                         ft->Children[i], new OpGammaDist));
                break;*/
            case ocLn:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpLn, nResultSize));
                break;
            case ocRound:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpRound, nResultSize));
                break;
            case ocCot:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpCot, nResultSize));
                break;
            case ocCotHyp:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpCoth, nResultSize));
                break;
            case ocFDist:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpFdist, nResultSize));
                break;
            case ocVar:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpVar, nResultSize));
                break;
            /*case ocChiDist:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                         ft->Children[i],new OpChiDist));
                break;*/
            case ocPow:
            case ocPower:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpPower, nResultSize));
                break;
            case ocOdd:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpOdd, nResultSize));
                break;
            /*case ocChiSqDist:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                         ft->Children[i],new OpChiSqDist));
                break;
            case ocChiSqInv:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                         ft->Children[i],new OpChiSqInv));
                break;
            case ocGammaInv:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                         ft->Children[i], new OpGammaInv));
                break;*/
            case ocFloor:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpFloor, nResultSize));
                break;
            /*case ocFInv:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                         ft->Children[i], new OpFInv));
                break;*/
            case ocFTest:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpFTest, nResultSize));
                break;
            case ocB:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpB, nResultSize));
                break;
            case ocBetaDist:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpBetaDist, nResultSize));
                break;
            case ocCosecantHyp:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpCscH, nResultSize));
                break;
            case ocExp:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpExp, nResultSize));
                break;
            case ocLog10:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpLog10, nResultSize));
                break;
            case ocExpDist:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpExponDist, nResultSize));
                break;
            case ocAverageIfs:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpAverageIfs, nResultSize));
                break;
            case ocCountIfs:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpCountIfs, nResultSize));
                break;
            case ocCombinA:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpCombinA, nResultSize));
                break;
            case ocEven:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpEven, nResultSize));
                break;
            case ocLog:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpLog, nResultSize));
                break;
            case ocMod:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpMod, nResultSize));
                break;
            case ocTrunc:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpTrunc, nResultSize));
                break;
            case ocSkew:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpSkew, nResultSize));
                break;
            case ocArcTan2:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpArcTan2, nResultSize));
                break;
            case ocBitOr:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpBitOr, nResultSize));
                break;
            case ocBitLshift:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpBitLshift, nResultSize));
                break;
            case ocBitRshift:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpBitRshift, nResultSize));
                break;
            case ocBitXor:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpBitXor, nResultSize));
                break;
            /*case ocChiInv:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                         ft->Children[i],new OpChiInv));
                break;*/
            case ocPoissonDist:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpPoisson, nResultSize));
                break;
            case ocSumSQ:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpSumSQ, nResultSize));
                break;
            case ocSkewp:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpSkewp, nResultSize));
                break;
            case ocBinomDist:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpBinomdist, nResultSize));
                break;
            case ocVarP:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpVarP, nResultSize));
                break;
            case ocCeil:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpCeil, nResultSize));
                break;
            case ocCombin:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpCombin, nResultSize));
                break;
            case ocDevSq:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpDevSq, nResultSize));
                break;
            case ocStDev:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpStDev, nResultSize));
                break;
            case ocSlope:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpSlope, nResultSize));
                break;
            case ocSTEYX:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpSTEYX, nResultSize));
                break;
            case ocZTest:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpZTest, nResultSize));
                break;
            case ocPi:
                mvSubArguments.push_back(
                    DynamicKernelArgumentRef(new DynamicKernelPiArgument(mCalcConfig, ts,
                            ft->Children[i])));
                break;
            case ocRandom:
                mvSubArguments.push_back(
                    DynamicKernelArgumentRef(new DynamicKernelRandomArgument(mCalcConfig, ts,
                            ft->Children[i])));
                break;
            case ocProduct:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpProduct, nResultSize));
                break;
            /*case ocHypGeomDist:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                         ft->Children[i],new OpHypGeomDist));
                break;*/
            case ocSumX2MY2:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpSumX2MY2, nResultSize));
                break;
            case ocSumX2DY2:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpSumX2PY2, nResultSize));
                break;
            /*case ocBetaInv:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                         ft->Children[i],new OpBetainv));
                 break;*/
            case ocTTest:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpTTest, nResultSize));
                break;
            case ocTDist:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpTDist, nResultSize));
                break;
            /*case ocTInv:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                         ft->Children[i], new OpTInv));
                 break;*/
            case ocSumXMY2:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpSumXMY2, nResultSize));
                break;
            case ocStDevP:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpStDevP, nResultSize));
                break;
            case ocCovar:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpCovar, nResultSize));
                break;
            case ocAnd:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpAnd, nResultSize));
                break;
            case ocVLookup:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpVLookup, nResultSize));
                break;
            case ocOr:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpOr, nResultSize));
                break;
            case ocNot:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpNot, nResultSize));
                break;
            case ocXor:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpXor, nResultSize));
                break;
            case ocDBMax:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpDmax, nResultSize));
                break;
            case ocDBMin:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpDmin, nResultSize));
                break;
            case ocDBProduct:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpDproduct, nResultSize));
                break;
            case ocDBAverage:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpDaverage, nResultSize));
                break;
            case ocDBStdDev:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpDstdev, nResultSize));
                break;
            case ocDBStdDevP:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpDstdevp, nResultSize));
                break;
            case ocDBSum:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpDsum, nResultSize));
                break;
            case ocDBVar:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpDvar, nResultSize));
                break;
            case ocDBVarP:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpDvarp, nResultSize));
                break;
            case ocAverageIf:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpAverageIf, nResultSize));
                break;
            case ocDBCount:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpDcount, nResultSize));
                break;
            case ocDBCount2:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpDcount2, nResultSize));
                break;
            case ocDeg:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpDeg, nResultSize));
                break;
            case ocRoundUp:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpRoundUp, nResultSize));
                break;
            case ocRoundDown:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpRoundDown, nResultSize));
                break;
            case ocInt:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpInt, nResultSize));
                break;
            case ocRad:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpRadians, nResultSize));
                break;
            case ocCountIf:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpCountIf, nResultSize));
                break;
            case ocIsEven:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpIsEven, nResultSize));
                break;
            case ocIsOdd:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpIsOdd, nResultSize));
                break;
            case ocFact:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpFact, nResultSize));
                break;
            case ocMinA:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpMinA, nResultSize));
                break;
            case ocCount2:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpCountA, nResultSize));
                break;
            case ocMaxA:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpMaxA, nResultSize));
                break;
            case ocAverageA:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpAverageA, nResultSize));
                break;
            case ocVarA:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpVarA, nResultSize));
                break;
            case ocVarPA:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpVarPA, nResultSize));
                break;
            case ocStDevA:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpStDevA, nResultSize));
                break;
            case ocStDevPA:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpStDevPA, nResultSize));
                break;
            case ocSecant:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpSec, nResultSize));
                break;
            case ocSecantHyp:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpSecH, nResultSize));
                break;
            case ocSumIf:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpSumIf, nResultSize));
                break;
            case ocNegSub:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpNegSub, nResultSize));
                break;
            case ocAveDev:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpAveDev, nResultSize));
                break;
            case ocIf:
                mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                        ft->Children[i], new OpIf, nResultSize));
                break;
            case ocExternal:
                if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getEffect")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpEffective, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getCumipmt")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpCumipmt, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getNominal")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpNominal, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getCumprinc")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpCumprinc, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getXnpv")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpXNPV, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getPricemat")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpPriceMat, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getReceived")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpReceived, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getTbilleq")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpTbilleq, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getTbillprice")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpTbillprice, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getTbillyield")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpTbillyield, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getFvschedule")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpFvschedule, nResultSize));
                }
                /*else if ( pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getYield")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpYield));
                }*/
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getYielddisc")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpYielddisc, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getYieldmat")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpYieldmat, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getAccrintm")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpAccrintm, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getCoupdaybs")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpCoupdaybs, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getDollarde")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpDollarde, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getDollarfr")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpDollarfr, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getCoupdays")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpCoupdays, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getCoupdaysnc")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpCoupdaysnc, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getDisc")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpDISC, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getIntrate")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i], new OpINTRATE, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getPrice")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                            ft->Children[i], new OpPrice, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getCoupnum")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpCoupnum, nResultSize));
                }
                /*else if pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getDuration"))
                {
                    mvSubArguments.push_back(
                        SoPHelper(mCalcConfig, ts, ft->Children[i], new OpDuration_ADD));
                }*/
                /*else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getAmordegrc")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpAmordegrc, nResultSize));
                }*/
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getAmorlinc")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpAmorlinc, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getMduration")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpMDuration, nResultSize));
                }
                /*else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getXirr")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpXirr, nResultSize));
                }*/
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getOddlprice")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                            ft->Children[i], new OpOddlprice, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getOddlyield")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpOddlyield, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getPricedisc")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts,
                            ft->Children[i], new OpPriceDisc, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getCouppcd")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpCouppcd, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getCoupncd")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpCoupncd, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getAccrint")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpAccrint, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getSqrtpi")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpSqrtPi, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getConvert")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpConvert, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getIseven")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpIsEven, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getIsodd")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpIsOdd, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getMround")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpMROUND, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getQuotient")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpQuotient, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getSeriessum")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpSeriesSum, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getBesselj")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpBesselj, nResultSize));
                }
                else if (pChild->GetExternal() == "com.sun.star.sheet.addin.Analysis.getGestep")
                {
                    mvSubArguments.push_back(SoPHelper(mCalcConfig, ts, ft->Children[i],
                            new OpGestep, nResultSize));
                }
                else
                    throw UnhandledToken(OUString("unhandled external " + pChild->GetExternal()).toUtf8().getStr(), __FILE__, __LINE__);
                break;

            default:
                throw UnhandledToken(OUString("unhandled opcode "
                    + formula::FormulaCompiler().GetOpCodeMap(com::sun::star::sheet::FormulaLanguage::ENGLISH)->getSymbol(opc)
                    + "(" + OUString::number(opc) + ")").toUtf8().getStr(), __FILE__, __LINE__);
        }
    }
}

class DynamicKernel : public CompiledFormula
{
public:
    DynamicKernel( const ScCalcConfig& config, const FormulaTreeNodeRef& r, int nResultSize );
    virtual ~DynamicKernel() override;

    static std::unique_ptr<DynamicKernel> create( const ScCalcConfig& config, const ScTokenArray& rCode, int nResultSize );

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
    ScCalcConfig const mCalcConfig;
    FormulaTreeNodeRef const mpRoot;
    SymbolTable mSyms;
    std::string mKernelSignature, mKernelHash;
    std::string mFullProgramSrc;
    cl_program mpProgram;
    cl_kernel mpKernel;
    cl_mem mpResClmem; // Results
    std::set<std::string> inlineDecl;
    std::set<std::string> inlineFun;

    int const mnResultSize;
};

DynamicKernel::DynamicKernel( const ScCalcConfig& config, const FormulaTreeNodeRef& r, int nResultSize ) :
    mCalcConfig(config),
    mpRoot(r),
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
    const DynamicKernelArgument* DK = mSyms.DeclRefArg<DynamicKernelSoPArguments>(mCalcConfig, mpRoot, new OpNop(mnResultSize), mnResultSize);

    std::stringstream decl;
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
        std::stringstream md5s;
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
                                CL_PROGRAM_BUILD_LOG, n, &log[0], nullptr);
                            SAL_WARN_IF(
                                e != CL_SUCCESS || n == 0, "sc.opencl",
                                "after CL_BUILD_PROGRAM_FAILURE,"
                                " clGetProgramBuildInfo("
                                "CL_PROGRAM_BUILD_LOG) fails with " << openclwrapper::errorString(e));
                            if (e == CL_SUCCESS)
                                SAL_WARN(
                                    "sc.opencl",
                                    "CL_BUILD_PROGRAM_FAILURE, status " << stat
                                    << ", log \"" << &log[0] << "\"");
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
        lastOneKernelHash = KernelHash;
        lastOneProgram = mpProgram;
    }
    mpKernel = clCreateKernel(mpProgram, kname.c_str(), &err);
    if (err != CL_SUCCESS)
        throw OpenCLError("clCreateKernel", err, __FILE__, __LINE__);
    SAL_INFO("sc.opencl", "Created kernel " << mpKernel << " with name " << kname << " in program " << mpProgram);
}

void DynamicKernel::Launch( size_t nr )
{
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
    SAL_INFO("sc.opencl", "Enqueing kernel " << mpKernel);
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
                                                     SlidingFunctionBase* pCodeGen, int nResultSize)
{
    FormulaToken* ref = t->GetFormulaToken();
    ArgumentMap::iterator it = mSymbols.find(ref);
    if (it == mSymbols.end())
    {
        // Allocate new symbols
        std::stringstream ss;
        ss << "tmp" << mCurId++;
        DynamicKernelArgumentRef new_arg(new T(config, ss.str(), t, pCodeGen, nResultSize));
        mSymbols[ref] = new_arg;
        mParams.push_back(new_arg);
        return new_arg.get();
    }
    else
    {
        return it->second.get();
    }
}

FormulaGroupInterpreterOpenCL::FormulaGroupInterpreterOpenCL() :
    FormulaGroupInterpreter() {}

FormulaGroupInterpreterOpenCL::~FormulaGroupInterpreterOpenCL() {}

ScMatrixRef FormulaGroupInterpreterOpenCL::inverseMatrix( const ScMatrix& )
{
    return nullptr;
}

std::unique_ptr<DynamicKernel> DynamicKernel::create( const ScCalcConfig& rConfig, const ScTokenArray& rCode, int nResultSize )
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
            FormulaTreeNodeRef pCurNode(new FormulaTreeNode(pCur));
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
                    pCurNode->Children.push_back(pChildTreeNode);
                }
            }
            std::reverse(pCurNode->Children.begin(), pCurNode->Children.end());
            aHashMap[pCur] = pCurNode;
        }
        aTokenVector.push_back(pCur);
    }

    FormulaTreeNodeRef Root = std::make_shared<FormulaTreeNode>(nullptr);
    Root->Children.push_back(aHashMap[aTokenVector.back()]);

    std::unique_ptr<DynamicKernel> pDynamicKernel(new DynamicKernel(rConfig, Root, nResultSize));

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
    DynamicKernel* const mpKernel;

    SCROW const mnGroupLength;

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

    SCROW const mnGroupLength;

public:
    explicit CLInterpreterContext(SCROW nGroupLength)
        : mpKernel(nullptr)
        , mnGroupLength(nGroupLength) {}

    bool isValid() const
    {
        return mpKernel != nullptr;
    }

    void setManagedKernel( std::unique_ptr<DynamicKernel> pKernel )
    {
        mpKernelStore = std::move(pKernel);
        mpKernel = mpKernelStore.get();
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
    CLInterpreterContext aCxt(xGroup->mnLength);

    aCxt.setManagedKernel(DynamicKernel::create(rConfig, rCode, xGroup->mnLength));

    return aCxt;
}

void genRPNTokens( ScDocument& rDoc, const ScAddress& rTopPos, ScTokenArray& rCode )
{
    ScCompiler aComp(&rDoc, rTopPos, rCode, rDoc.GetGrammar());
    // Disable special ordering for jump commands for the OpenCL interpreter.
    aComp.EnableJumpCommandReorder(false);
    aComp.CompileTokenArray(); // Regenerate RPN tokens.
}

bool waitForResults()
{
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

}} // namespace sc::opencl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
