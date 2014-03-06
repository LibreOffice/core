/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "formulagroup.hxx"
#include "clkernelthread.hxx"
#include "grouptokenconverter.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"
#include "compiler.hxx"
#include "interpre.hxx"
#include "formula/vectortoken.hxx"
#include "scmatrix.hxx"

#include "openclwrapper.hxx"

#include "op_financial.hxx"
#include "op_database.hxx"
#include "op_math.hxx"
#include "op_logical.hxx"
#include "op_statistical.hxx"
#include "op_array.hxx"
#include "op_spreadsheet.hxx"
#include "op_addin.hxx"
/// CONFIGURATIONS
// Comment out this to turn off FMIN and FMAX intrinsics
#define USE_FMIN_FMAX 1
#define REDUCE_THRESHOLD 4  // set to 4 for correctness testing. priority 1
#define UNROLLING_FACTOR 16  // set to 4 for correctness testing (if no reduce)
#include "formulagroupcl_public.hxx"
#ifdef WIN32
#ifndef NAN
namespace {
static const unsigned long __nan[2] = {0xffffffff, 0x7fffffff};
}
#define NAN (*(const double*) __nan)
#endif
#endif

#include <list>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#define MD5_KERNEL 1
#ifdef MD5_KERNEL
#include <rtl/digest.h>
#endif

#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>

#undef NO_FALLBACK_TO_SWINTERP /* undef this for non-TDD runs */

using namespace formula;

namespace sc { namespace opencl {


/// Map the buffer used by an argument and do necessary argument setting
size_t VectorRef::Marshal(cl_kernel k, int argno, int, cl_program)
{
    FormulaToken *ref = mFormulaTree->GetFormulaToken();
    double *pHostBuffer = NULL;
    size_t szHostBuffer = 0;
    if (ref->GetType() == formula::svSingleVectorRef) {
        const formula::SingleVectorRefToken* pSVR =
            static_cast< const formula::SingleVectorRefToken* >(ref);
        pHostBuffer = const_cast<double*>(pSVR->GetArray().mpNumericArray);
        szHostBuffer = pSVR->GetArrayLength() * sizeof(double);
#if 0
        std::cerr << "Marshal a Single vector of size " << pSVR->GetArrayLength();
        std::cerr << " at argument "<< argno << "\n";
#endif
    } else if (ref->GetType() == formula::svDoubleVectorRef) {
        const formula::DoubleVectorRefToken* pDVR =
            static_cast< const formula::DoubleVectorRefToken* >(ref);
        pHostBuffer = const_cast<double*>(
                pDVR->GetArrays()[mnIndex].mpNumericArray);
        szHostBuffer = pDVR->GetArrayLength() * sizeof(double);
    } else {
        throw Unhandled();
    }
    // Obtain cl context
    KernelEnv kEnv;
    OpenclDevice::setKernelEnv(&kEnv);
    cl_int err;
    if (pHostBuffer)
    {
        mpClmem = clCreateBuffer(kEnv.mpkContext,
                (cl_mem_flags) CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,
                szHostBuffer,
                pHostBuffer, &err);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);
    }
    else
    {
        if (szHostBuffer == 0)
            szHostBuffer = sizeof(double); // a dummy small value
        // Marshal as a buffer of NANs
        mpClmem = clCreateBuffer(kEnv.mpkContext,
                (cl_mem_flags) CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR,
                szHostBuffer, NULL, &err);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);
        double *pNanBuffer = (double*)clEnqueueMapBuffer(
                kEnv.mpkCmdQueue, mpClmem, CL_TRUE, CL_MAP_WRITE, 0,
                szHostBuffer, 0, NULL, NULL, &err);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);
        for (size_t i = 0; i < szHostBuffer/sizeof(double); i++)
            pNanBuffer[i] = NAN;
        err = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue, mpClmem,
                pNanBuffer, 0, NULL, NULL);
    }

    err = clSetKernelArg(k, argno, sizeof(cl_mem), (void*)&mpClmem);
    if (CL_SUCCESS != err)
        throw OpenCLError(err, __FILE__, __LINE__);
    return 1;
}

/// Arguments that are actually compile-time constant string
/// Currently, only the hash is passed.
/// TBD(IJSUNG): pass also length and the actual string if there is a
/// hash function collision
class ConstStringArgument: public DynamicKernelArgument
{
public:
    ConstStringArgument(const std::string &s,
        FormulaTreeNodeRef ft):
            DynamicKernelArgument(s, ft) {}
    /// Generate declaration
    virtual void GenDecl(std::stringstream &ss) const
    {
        ss << "unsigned " << mSymName;
    }
    virtual void GenDeclRef(std::stringstream &ss) const
    {
        ss << GenSlidingWindowDeclRef(false);
    }
    virtual void GenSlidingWindowDecl(std::stringstream &ss) const
    {
        GenDecl(ss);
    }
    virtual std::string GenSlidingWindowDeclRef(bool=false) const
    {
        std::stringstream ss;
        if (GetFormulaToken()->GetType() != formula::svString)
            throw Unhandled();
        FormulaToken *Tok = GetFormulaToken();
        ss << Tok->GetString().getString().toAsciiUpperCase().hashCode() << "U";
        return ss.str();
    }
    virtual size_t GetWindowSize(void) const
    {
        return 1;
    }
    /// Pass the 32-bit hash of the string to the kernel
    virtual size_t Marshal(cl_kernel k, int argno, int, cl_program)
    {
        FormulaToken *ref = mFormulaTree->GetFormulaToken();
        cl_uint hashCode = 0;
        if (ref->GetType() == formula::svString)
        {
            const rtl::OUString s = ref->GetString().getString().toAsciiUpperCase();
            hashCode = s.hashCode();
        } else {
            throw Unhandled();
        }
        // marshaling
        // Obtain cl context
        KernelEnv kEnv;
        OpenclDevice::setKernelEnv(&kEnv);
        // Pass the scalar result back to the rest of the formula kernel
        cl_int err = clSetKernelArg(k, argno, sizeof(cl_uint), (void*)&hashCode);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);
        return 1;
    }
};

/// Arguments that are actually compile-time constants
class DynamicKernelConstantArgument: public DynamicKernelArgument
{
public:
    DynamicKernelConstantArgument(const std::string &s,
        FormulaTreeNodeRef ft):
            DynamicKernelArgument(s, ft) {}
    /// Generate declaration
    virtual void GenDecl(std::stringstream &ss) const
    {
        ss << "double " << mSymName;
    }
    virtual void GenDeclRef(std::stringstream &ss) const
    {
        ss << mSymName;
    }
    virtual void GenSlidingWindowDecl(std::stringstream &ss) const
    {
        GenDecl(ss);
    }
    virtual std::string GenSlidingWindowDeclRef(bool=false) const
    {
        if (GetFormulaToken()->GetType() != formula::svDouble)
            throw Unhandled();
        return mSymName;
    }
    virtual size_t GetWindowSize(void) const
    {
        return 1;
    }
    double GetDouble(void) const
    {
        FormulaToken *Tok = GetFormulaToken();
        if (Tok->GetType() != formula::svDouble)
            throw Unhandled();
        return Tok->GetDouble();
    }
    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal(cl_kernel k, int argno, int, cl_program)
    {
        double tmp = GetDouble();
        // Pass the scalar result back to the rest of the formula kernel
        cl_int err = clSetKernelArg(k, argno, sizeof(double), (void*)&tmp);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);
        return 1;
    }
    virtual cl_mem GetCLBuffer(void) const { return NULL; }
};

class DynamicKernelPiArgument: public DynamicKernelArgument
{
public:
    DynamicKernelPiArgument(const std::string &s,
        FormulaTreeNodeRef ft):
            DynamicKernelArgument(s, ft) {}
    /// Generate declaration
    virtual void GenDecl(std::stringstream &ss) const
    {
        ss << "double " << mSymName;
    }
    virtual void GenDeclRef(std::stringstream &ss) const
    {
        ss << "3.14159265358979";
    }
    virtual void GenSlidingWindowDecl(std::stringstream &ss) const
    {
        GenDecl(ss);
    }
    virtual std::string GenSlidingWindowDeclRef(bool=false) const
    {
        return mSymName;
    }
    virtual size_t GetWindowSize(void) const
    {
        return 1;
    }
    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal(cl_kernel k, int argno, int, cl_program)
    {
        double tmp = 0.0;
        // Pass the scalar result back to the rest of the formula kernel
        cl_int err = clSetKernelArg(k, argno, sizeof(double), (void*)&tmp);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);
        return 1;
    }
};

class DynamicKernelRandomArgument: public DynamicKernelArgument
{
public:
    DynamicKernelRandomArgument(const std::string &s,
        FormulaTreeNodeRef ft):
            DynamicKernelArgument(s, ft) {}
    /// Generate declaration
    virtual void GenDecl(std::stringstream &ss) const
    {
        ss << "double " << mSymName;
    }
    virtual void GenDeclRef(std::stringstream &ss) const
    {
        ss << mSymName;
    }
    virtual void GenSlidingWindowDecl(std::stringstream &ss) const
    {
        GenDecl(ss);
    }
    virtual std::string GenSlidingWindowDeclRef(bool=false) const
    {
        return mSymName + "_Random()";
    }
    void GenSlidingWindowFunction(std::stringstream &ss)
    {
        ss << "\ndouble " << mSymName;
        ss << "_Random ()\n{\n";
        ss << "    int i, gid0=get_global_id(0);;\n";
        ss << "    double tmp = 0;\n";
        ss << "    double M = 2147483647;\n";
        ss << "    double Lamda = 32719;\n";
        ss << "    double f;\n";
        ss << "    f = gid0 + 1;\n";
        ss << "    int k;\n";
        ss << "    for(i = 1;i <= 100; ++i){\n";
        ss << "        f = Lamda * f;\n";
        ss << "        k = (int)(f * pow(M,-1.0));\n";
        ss << "        f = f - M * k;\n";
        ss << "    }\n";
        ss << "    tmp = f * pow(M,-1.0);\n";
        ss << "    return tmp;\n";
        ss << "}";
    }
    virtual size_t GetWindowSize(void) const
    {
        return 1;
    }
    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal(cl_kernel k, int argno, int, cl_program)
    {
        double tmp = 0.0;
        // Pass the scalar result back to the rest of the formula kernel
        cl_int err = clSetKernelArg(k, argno, sizeof(double), (void*)&tmp);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);
        return 1;
    }
};

/// A vector of strings
class DynamicKernelStringArgument: public VectorRef
{
public:
    DynamicKernelStringArgument(const std::string &s,
        FormulaTreeNodeRef ft, int index = 0):
        VectorRef(s, ft, index) {}

    virtual void GenSlidingWindowFunction(std::stringstream &) {}
    /// Generate declaration
    virtual void GenDecl(std::stringstream &ss) const
    {
        ss << "__global unsigned int *"<<mSymName;
    }
    virtual void GenSlidingWindowDecl(std::stringstream& ss) const
    {
        DynamicKernelStringArgument::GenDecl(ss);
    }
    virtual size_t Marshal(cl_kernel, int, int, cl_program);
};

/// Marshal a string vector reference
size_t DynamicKernelStringArgument::Marshal(cl_kernel k, int argno, int, cl_program)
{
    FormulaToken *ref = mFormulaTree->GetFormulaToken();
    // Obtain cl context
    KernelEnv kEnv;
    OpenclDevice::setKernelEnv(&kEnv);
    cl_int err;
    formula::VectorRefArray vRef;
    size_t nStrings = 0;
    if (ref->GetType() == formula::svSingleVectorRef) {
        const formula::SingleVectorRefToken* pSVR =
            static_cast< const formula::SingleVectorRefToken* >(ref);
        nStrings = pSVR->GetArrayLength();
        vRef = pSVR->GetArray();
    } else if (ref->GetType() == formula::svDoubleVectorRef) {
        const formula::DoubleVectorRefToken* pDVR =
            static_cast< const formula::DoubleVectorRefToken* >(ref);
        nStrings = pDVR->GetArrayLength();
        vRef = pDVR->GetArrays()[mnIndex];
    }
    size_t szHostBuffer = nStrings * sizeof(cl_int);
    // Marshal strings. Right now we pass hashes of these string
    mpClmem = clCreateBuffer(kEnv.mpkContext,
            (cl_mem_flags) CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR,
            szHostBuffer, NULL, &err);
    if (CL_SUCCESS != err)
        throw OpenCLError(err, __FILE__, __LINE__);
    cl_uint *pHashBuffer = (cl_uint*)clEnqueueMapBuffer(
            kEnv.mpkCmdQueue, mpClmem, CL_TRUE, CL_MAP_WRITE, 0,
            szHostBuffer, 0, NULL, NULL, &err);
    if (CL_SUCCESS != err)
        throw OpenCLError(err, __FILE__, __LINE__);
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
    err = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue, mpClmem,
            pHashBuffer, 0, NULL, NULL);
    if (CL_SUCCESS != err)
        throw OpenCLError(err, __FILE__, __LINE__);

    err = clSetKernelArg(k, argno, sizeof(cl_mem), (void*)&mpClmem);
    if (CL_SUCCESS != err)
        throw OpenCLError(err, __FILE__, __LINE__);
    return 1;
}

/// A mixed string/numberic vector
class DynamicKernelMixedArgument: public VectorRef
{
public:
    DynamicKernelMixedArgument(const std::string &s,
        FormulaTreeNodeRef ft):
        VectorRef(s, ft), mStringArgument(s+"s", ft) {}
    virtual void GenSlidingWindowDecl(std::stringstream& ss) const
    {
        VectorRef::GenSlidingWindowDecl(ss);
        ss << ", ";
        mStringArgument.GenSlidingWindowDecl(ss);
    }
    virtual void GenSlidingWindowFunction(std::stringstream &) {}
    /// Generate declaration
    virtual void GenDecl(std::stringstream &ss) const
    {
        VectorRef::GenDecl(ss);
        ss << ", ";
        mStringArgument.GenDecl(ss);
    }
    virtual void GenDeclRef(std::stringstream &ss) const
    {
        VectorRef::GenDeclRef(ss);
        ss << ",";
        mStringArgument.GenDeclRef(ss);
    }
    virtual std::string GenSlidingWindowDeclRef(bool) const
    {
        std::stringstream ss;
        ss << "(!isNan(" << VectorRef::GenSlidingWindowDeclRef();
        ss << ")?" << VectorRef::GenSlidingWindowDeclRef();
        ss << ":" << mStringArgument.GenSlidingWindowDeclRef();
        ss << ")";
        return ss.str();
    }
    virtual std::string GenDoubleSlidingWindowDeclRef(bool=false) const
    {
        std::stringstream ss;
        ss << VectorRef::GenSlidingWindowDeclRef();
        return ss.str();
    }
    virtual std::string GenStringSlidingWindowDeclRef(bool=false) const
    {
        std::stringstream ss;
        ss << mStringArgument.GenSlidingWindowDeclRef();
        return ss.str();
    }
    virtual size_t Marshal(cl_kernel k, int argno, int vw, cl_program p)
    {
        int i = VectorRef::Marshal(k, argno, vw, p);
        i += mStringArgument.Marshal(k, argno+i, vw, p);
        return i;
    }
protected:
    DynamicKernelStringArgument mStringArgument;
};

/// Handling a Double Vector that is used as a sliding window input
/// to either a sliding window average or sum-of-products
/// Generate a sequential loop for reductions
class OpSum; // Forward Declaration
class OpAverage; // Forward Declaration
class OpMin; // Forward Declaration
class OpMax; // Forward Declaration
class OpCount; // Forward Declaration
template<class Base>
class DynamicKernelSlidingArgument: public Base
{
public:
    DynamicKernelSlidingArgument(const std::string &s,
        FormulaTreeNodeRef ft, boost::shared_ptr<SlidingFunctionBase> &CodeGen,
        int index=0):
        Base(s, ft, index), mpCodeGen(CodeGen), mpClmem2(NULL)
    {
        FormulaToken *t = ft->GetFormulaToken();
        if (t->GetType() != formula::svDoubleVectorRef)
            throw Unhandled();
        mpDVR = static_cast<const formula::DoubleVectorRefToken *>(t);
        bIsStartFixed = mpDVR->IsStartFixed();
        bIsEndFixed = mpDVR->IsEndFixed();
    }
    // Should only be called by SumIfs. Yikes!
    virtual bool NeedParallelReduction(void) const
    {
        assert(dynamic_cast<OpSumIfs*>(mpCodeGen.get()));
        return GetWindowSize()> 100 &&
            ( (GetStartFixed() && GetEndFixed()) ||
              (!GetStartFixed() && !GetEndFixed())  ) ;
    }
    virtual void GenSlidingWindowFunction(std::stringstream &) {}

    virtual std::string GenSlidingWindowDeclRef(bool nested=false) const
    {
        size_t nArrayLength = mpDVR->GetArrayLength();
        std::stringstream ss;
        if (!bIsStartFixed && !bIsEndFixed)
        {
            if (nested)
                ss << "((i+gid0) <" << nArrayLength <<"?";
            ss << Base::GetName() << "[i + gid0]";
            if (nested)
                ss << ":NAN)";
        }
        else
        {
            if (nested)
                ss << "(i <" << nArrayLength <<"?";
            ss << Base::GetName() << "[i]";
            if (nested)
                ss << ":NAN)";
        }
        return ss.str();
    }
    /// Controls how the elements in the DoubleVectorRef are traversed
    virtual size_t GenReductionLoopHeader(
        std::stringstream &ss, bool &needBody)
    {
        assert(mpDVR);
        size_t nCurWindowSize = mpDVR->GetRefRowSize();
        // original for loop
#ifndef UNROLLING_FACTOR
        needBody = true;
        // No need to generate a for-loop for degenerated cases
        if (nCurWindowSize == 1)
        {
            ss << "if (gid0 <" << mpDVR->GetArrayLength();
            ss << ")\n\t{\tint i = 0;\n\t\t";
            return nCurWindowSize;
        }

        ss << "for (int i = ";
        if (!bIsStartFixed && bIsEndFixed)
        {
#ifdef  ISNAN
            ss << "gid0; i < " << mpDVR->GetArrayLength();
            ss << " && i < " << nCurWindowSize  << "; i++){\n\t\t";
#else
            ss << "gid0; i < "<< nCurWindowSize << "; i++)\n\t\t";
#endif
        }
        else if (bIsStartFixed && !bIsEndFixed)
        {
#ifdef  ISNAN
            ss << "0; i < " << mpDVR->GetArrayLength();
            ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n\t\t";
#else
            ss << "0; i < gid0+"<< nCurWindowSize << "; i++)\n\t\t";
#endif
        }
        else if (!bIsStartFixed && !bIsEndFixed)
        {
#ifdef  ISNAN
            ss << "0; i + gid0 < " << mpDVR->GetArrayLength();
            ss << " &&  i < "<< nCurWindowSize << "; i++){\n\t\t";
#else
            ss << "0; i < "<< nCurWindowSize << "; i++)\n\t\t";
#endif
        }
        else
        {
            unsigned limit =
                std::min(mpDVR->GetArrayLength(), nCurWindowSize);
            ss << "0; i < "<< limit << "; i++){\n\t\t";
        }
return nCurWindowSize;
#endif

#ifdef UNROLLING_FACTOR
        {
            if (!mpDVR->IsStartFixed() && mpDVR->IsEndFixed()) {
                ss << "for (int i = ";
                ss << "gid0; i < " << mpDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; i++){\n\t\t";
                needBody = true;
                return nCurWindowSize;
            } else if (mpDVR->IsStartFixed() && !mpDVR->IsEndFixed()) {
                ss << "for (int i = ";
                ss << "0; i < " << mpDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n\t\t";
                needBody = true;
                return nCurWindowSize;
            } else if (!mpDVR->IsStartFixed() && !mpDVR->IsEndFixed()){
                ss << "tmpBottom = " << mpCodeGen->GetBottom() << ";\n\t";
                ss << "{int i;\n\t";
                std::stringstream temp1,temp2;
                int outLoopSize = UNROLLING_FACTOR;
                if ( nCurWindowSize/outLoopSize != 0){
                    ss << "for(int outLoop=0; outLoop<" << nCurWindowSize/outLoopSize<< "; outLoop++){\n\t";
                    for(int count=0; count < outLoopSize; count++){
                        ss << "i = outLoop*"<<outLoopSize<<"+"<<count<<";\n\t";
                        if(count==0){
                            temp1 << "if(i + gid0 < " <<mpDVR->GetArrayLength();
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
                for(unsigned int count=nCurWindowSize/outLoopSize*outLoopSize; count < nCurWindowSize; count++){
                    ss << "i = "<<count<<";\n\t";
                    if(count==nCurWindowSize/outLoopSize*outLoopSize){
                        temp2 << "if(i + gid0 < " << mpDVR->GetArrayLength();
                        temp2 << "){\n\t\t";
                        temp2 << "tmp = legalize(";
                        temp2 << mpCodeGen->Gen2(GenSlidingWindowDeclRef(), "tmp");
                        temp2 << ", tmp);\n\t\t\t";
                        temp2 << "}\n\t";
                    }
                    ss << temp2.str();
                }
                ss << "} // to scope the int i declaration\n";
                needBody = false;
                return nCurWindowSize;
            }
            // (mpDVR->IsStartFixed() && mpDVR->IsEndFixed())
            else {
                ss << "//else situation \n\t";
                ss << "tmpBottom = " << mpCodeGen->GetBottom() << ";\n\t";
                ss << "{int i;\n\t";
                std::stringstream temp1,temp2;
                int outLoopSize = UNROLLING_FACTOR;
                if (nCurWindowSize/outLoopSize != 0){
                    ss << "for(int outLoop=0; outLoop<" << nCurWindowSize/outLoopSize<< "; outLoop++){\n\t";
                    for(int count=0; count < outLoopSize; count++){
                        ss << "i = outLoop*"<<outLoopSize<<"+"<<count<<";\n\t";
                        if(count==0){
                            temp1 << "tmp = legalize(";
                            temp1 << mpCodeGen->Gen2(GenSlidingWindowDeclRef(), "tmp");
                            temp1 << ", tmp);\n\t\t\t";
                        }
                        ss << temp1.str();
                    }
                    ss << "}\n\t";
                }
                // The residual of mod outLoopSize
                for(unsigned int count=nCurWindowSize/outLoopSize*outLoopSize; count < nCurWindowSize; count++){
                    ss << "i = "<<count<<";\n\t";
                    if(count==nCurWindowSize/outLoopSize*outLoopSize){
                        temp2 << "tmp = legalize(";
                        temp2 << mpCodeGen->Gen2(GenSlidingWindowDeclRef(), "tmp");
                        temp2 << ", tmp);\n\t\t\t";
                    }
                    ss << temp2.str();
                }
                ss << "} // to scope the int i declaration\n";
                needBody = false;
                return nCurWindowSize;
            }
        }
#endif
}
    ~DynamicKernelSlidingArgument()
    {
        if (mpClmem2)
        {
            clReleaseMemObject(mpClmem2);
            mpClmem2 = NULL;
        }
    }

    size_t GetArrayLength(void) const {return mpDVR->GetArrayLength(); }

    size_t GetWindowSize(void) const {return mpDVR->GetRefRowSize(); }

    size_t GetStartFixed(void) const {return bIsStartFixed; }

    size_t GetEndFixed(void) const {return bIsEndFixed; }

protected:
    bool bIsStartFixed, bIsEndFixed;
    const formula::DoubleVectorRefToken *mpDVR;
    // from parent nodes
    boost::shared_ptr<SlidingFunctionBase> mpCodeGen;
    // controls whether to invoke the reduction kernel during marshaling or not
    cl_mem mpClmem2;
};

/// A mixed string/numberic vector
class DynamicKernelMixedSlidingArgument : public VectorRef
{
public:
    DynamicKernelMixedSlidingArgument(const std::string &s,
        FormulaTreeNodeRef ft, boost::shared_ptr<SlidingFunctionBase> &CodeGen,
        int index = 0):
        VectorRef(s, ft),
        mDoubleArgument(s, ft, CodeGen, index),
        mStringArgument(s+"s", ft, CodeGen, index) {}
    virtual void GenSlidingWindowDecl(std::stringstream& ss) const
    {
        mDoubleArgument.GenSlidingWindowDecl(ss);
        ss << ", ";
        mStringArgument.GenSlidingWindowDecl(ss);
    }
    virtual void GenSlidingWindowFunction(std::stringstream &) {}
    /// Generate declaration
    virtual void GenDecl(std::stringstream &ss) const
    {
        mDoubleArgument.GenDecl(ss);
        ss << ", ";
        mStringArgument.GenDecl(ss);
    }
    virtual void GenDeclRef(std::stringstream &ss) const
    {
        mDoubleArgument.GenDeclRef(ss);
        ss << ",";
        mStringArgument.GenDeclRef(ss);
    }
    virtual std::string GenSlidingWindowDeclRef(bool) const
    {
        std::stringstream ss;
        ss << "(!isNan(" << mDoubleArgument.GenSlidingWindowDeclRef();
        ss << ")?" << mDoubleArgument.GenSlidingWindowDeclRef();
        ss << ":" << mStringArgument.GenSlidingWindowDeclRef();
        ss << ")";
        return ss.str();
    }
    virtual std::string GenDoubleSlidingWindowDeclRef(bool=false) const
    {
        std::stringstream ss;
        ss << mDoubleArgument.GenSlidingWindowDeclRef();
        return ss.str();
    }
    virtual std::string GenStringSlidingWindowDeclRef(bool=false) const
    {
        std::stringstream ss;
        ss << mStringArgument.GenSlidingWindowDeclRef();
        return ss.str();
    }
    virtual size_t Marshal(cl_kernel k, int argno, int vw, cl_program p)
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
class SymbolTable {
public:
    typedef std::map<const formula::FormulaToken *,
        boost::shared_ptr<DynamicKernelArgument> > ArgumentMap;
    // This avoids instability caused by using pointer as the key type
    typedef std::list< boost::shared_ptr<DynamicKernelArgument> > ArgumentList;
    SymbolTable(void):mCurId(0) {}
    template <class T>
    const DynamicKernelArgument *DeclRefArg(FormulaTreeNodeRef, SlidingFunctionBase* pCodeGen);
    /// Used to generate sliding window helpers
    void DumpSlidingWindowFunctions(std::stringstream &ss)
    {
        for(ArgumentList::iterator it = mParams.begin(), e= mParams.end(); it!=e;
            ++it) {
            (*it)->GenSlidingWindowFunction(ss);
            ss << "\n";
        }
    }
    /// Memory mapping from host to device and pass buffers to the given kernel as
    /// arguments
    void Marshal(cl_kernel, int, cl_program);
    // number of result items.
    static int nR;
private:
    unsigned int mCurId;
    ArgumentMap mSymbols;
    ArgumentList mParams;
};
int SymbolTable::nR = 0;

void SymbolTable::Marshal(cl_kernel k, int nVectorWidth, cl_program pProgram)
{
    int i = 1; //The first argument is reserved for results
    for(ArgumentList::iterator it = mParams.begin(), e= mParams.end(); it!=e;
            ++it) {
        i+=(*it)->Marshal(k, i, nVectorWidth, pProgram);
    }
}

/// Handling a Double Vector that is used as a sliding window input
/// Performs parallel reduction based on given operator
template<class Base>
class ParallelReductionVectorRef: public Base
{
public:
    ParallelReductionVectorRef(const std::string &s,
        FormulaTreeNodeRef ft, boost::shared_ptr<SlidingFunctionBase> &CodeGen,
        int index=0):
        Base(s, ft, index), mpCodeGen(CodeGen), mpClmem2(NULL)
    {
        FormulaToken *t = ft->GetFormulaToken();
        if (t->GetType() != formula::svDoubleVectorRef)
            throw Unhandled();
        mpDVR = static_cast<const formula::DoubleVectorRefToken *>(t);
        bIsStartFixed = mpDVR->IsStartFixed();
        bIsEndFixed = mpDVR->IsEndFixed();
    }
    /// Emit the definition for the auxiliary reduction kernel
    virtual void GenSlidingWindowFunction(std::stringstream &ss) {
      if ( !dynamic_cast<OpAverage*>(mpCodeGen.get()))
      {
        std::string name = Base::GetName();
        ss << "__kernel void "<<name;
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
        ss << "    tmp = "<< mpCodeGen->GetBottom() << ";\n";
        ss << "    int loopOffset = l*512;\n";
        ss << "    if((loopOffset + lidx + offset + 256) < end) {\n";
        ss << "        tmp = legalize(" << mpCodeGen->Gen2(
                "A[loopOffset + lidx + offset]", "tmp") <<", tmp);\n";
        ss << "        tmp = legalize(" << mpCodeGen->Gen2(
                "A[loopOffset + lidx + offset + 256]", "tmp") <<", tmp);\n";
        ss << "    } else if ((loopOffset + lidx + offset) < end)\n";
        ss << "        tmp = legalize(" << mpCodeGen->Gen2(
                "A[loopOffset + lidx + offset]", "tmp") <<", tmp);\n";
        ss << "    shm_buf[lidx] = tmp;\n";
        ss << "    barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    for (int i = 128; i >0; i/=2) {\n";
        ss << "        if (lidx < i)\n";
        ss << "            shm_buf[lidx] = ";
        // Special case count
        if (dynamic_cast<OpCount*>(mpCodeGen.get()))
            ss << "shm_buf[lidx] + shm_buf[lidx + i];\n";
        else
            ss << mpCodeGen->Gen2("shm_buf[lidx]", "shm_buf[lidx + i]")<<";\n";
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
      else{
        std::string name = Base::GetName();
        /*sum reduction*/
        ss << "__kernel void "<<name<<"_sum";
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
        ss << "    tmp = "<< mpCodeGen->GetBottom() << ";\n";
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
        ss << "__kernel void "<<name<<"_count";
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
        ss << "    tmp = "<< mpCodeGen->GetBottom() << ";\n";
        ss << "    int loopOffset = l*512;\n";
        ss << "    if((loopOffset + lidx + offset + 256) < end) {\n";
        ss << "        tmp = legalize((isNan(A[loopOffset + lidx + offset])?tmp:tmp+1.0)";
        ss << ", tmp);\n";
        ss << "        tmp = legalize((isNan(A[loopOffset + lidx + offset+256])?tmp:tmp+1.0)";
        ss << ", tmp);\n";
        ss << "    } else if ((loopOffset + lidx + offset) < end)\n";
        ss << "        tmp = legalize((isNan(A[loopOffset + lidx + offset])?tmp:tmp+1.0)";
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
    virtual std::string GenSlidingWindowDeclRef(bool=false) const
    {
        std::stringstream ss;
        if (!bIsStartFixed && !bIsEndFixed)
            ss << Base::GetName() << "[i + gid0]";
        else
            ss << Base::GetName() << "[i]";
        return ss.str();
    }
    /// Controls how the elements in the DoubleVectorRef are traversed
    virtual size_t GenReductionLoopHeader(
        std::stringstream &ss, bool &needBody)
    {
        assert(mpDVR);
        size_t nCurWindowSize = mpDVR->GetRefRowSize();
        std::string temp = Base::GetName() + "[gid0]";
        ss << "tmp = ";
        // Special case count
        if ( dynamic_cast<OpAverage*>(mpCodeGen.get()))
        {
            ss << mpCodeGen->Gen2(temp, "tmp")<<";\n";
            ss <<"nCount = nCount-1;\n";
            ss <<"nCount = nCount +";/*re-assign nCount from count reduction*/
            ss << Base::GetName()<<"[gid0+"<<SymbolTable::nR<<"]"<<";\n";
        }
        else if (dynamic_cast<OpCount*>(mpCodeGen.get()))
            ss << temp << "+ tmp";
        else
            ss << mpCodeGen->Gen2(temp, "tmp");
        ss << ";\n\t";
        needBody = false;
        return nCurWindowSize;
    }

    virtual size_t Marshal(cl_kernel k, int argno, int w, cl_program mpProgram)
    {
        assert(Base::mpClmem == NULL);
        // Obtain cl context
        KernelEnv kEnv;
        OpenclDevice::setKernelEnv(&kEnv);
        cl_int err;
        size_t nInput = mpDVR->GetArrayLength();
        size_t nCurWindowSize = mpDVR->GetRefRowSize();
        // create clmem buffer
        if (mpDVR->GetArrays()[Base::mnIndex].mpNumericArray == NULL)
            throw Unhandled();
        double *pHostBuffer = const_cast<double*>(
                mpDVR->GetArrays()[Base::mnIndex].mpNumericArray);
        size_t szHostBuffer = nInput * sizeof(double);
        Base::mpClmem = clCreateBuffer(kEnv.mpkContext,
                (cl_mem_flags) CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,
                szHostBuffer,
                pHostBuffer, &err);
        mpClmem2 = clCreateBuffer(kEnv.mpkContext,
        CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR,
                sizeof(double)*w, NULL, NULL);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);
        // reproduce the reduction function name
        std::string kernelName;
        if ( !dynamic_cast<OpAverage*>(mpCodeGen.get()))
           kernelName = Base::GetName() + "_reduction";
        else
           kernelName = Base::GetName() + "_sum_reduction";
        cl_kernel redKernel = clCreateKernel(mpProgram, kernelName.c_str(), &err);
        if (err != CL_SUCCESS)
            throw OpenCLError(err, __FILE__, __LINE__);
        // set kernel arg of reduction kernel
        // TODO(Wei Wei): use unique name for kernel
        cl_mem buf = Base::GetCLBuffer();
        err = clSetKernelArg(redKernel, 0, sizeof(cl_mem),
                (void *)&buf);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);

        err = clSetKernelArg(redKernel, 1, sizeof(cl_mem), (void *)&mpClmem2);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);

        err = clSetKernelArg(redKernel, 2, sizeof(cl_int), (void*)&nInput);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);

        err = clSetKernelArg(redKernel, 3, sizeof(cl_int), (void*)&nCurWindowSize);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);

        // set work group size and execute
        size_t global_work_size[] = {256, (size_t)w };
        size_t local_work_size[] = {256, 1};
        err = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, redKernel, 2, NULL,
                global_work_size, local_work_size, 0, NULL, NULL);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);
        err = clFinish(kEnv.mpkCmdQueue);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);
        if ( dynamic_cast<OpAverage*>(mpCodeGen.get()))
        {
             /*average need more reduction kernel for count computing*/
            boost::scoped_array<double> pAllBuffer(new double[2*w]);
            double *resbuf = (double*)clEnqueueMapBuffer(kEnv.mpkCmdQueue,
                    mpClmem2,
                    CL_TRUE, CL_MAP_READ, 0,
                    sizeof(double)*w, 0, NULL, NULL,
                    &err);
            if (err != CL_SUCCESS)
                throw OpenCLError(err, __FILE__, __LINE__);

            for (int i=0 ; i < w; i++)
                pAllBuffer[i] = resbuf[i];
            err = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue, mpClmem2, resbuf, 0, NULL, NULL);
            if (err != CL_SUCCESS)
                throw OpenCLError(err, __FILE__, __LINE__);

            kernelName = Base::GetName() + "_count_reduction";
            redKernel = clCreateKernel(mpProgram, kernelName.c_str(), &err);
            if (err != CL_SUCCESS)
                throw OpenCLError(err, __FILE__, __LINE__);
            // set kernel arg of reduction kernel
            buf = Base::GetCLBuffer();
            err = clSetKernelArg(redKernel, 0, sizeof(cl_mem),
                    (void *)&buf);
            if (CL_SUCCESS != err)
                throw OpenCLError(err, __FILE__, __LINE__);

            err = clSetKernelArg(redKernel, 1, sizeof(cl_mem), (void *)&mpClmem2);
            if (CL_SUCCESS != err)
                throw OpenCLError(err, __FILE__, __LINE__);

            err = clSetKernelArg(redKernel, 2, sizeof(cl_int), (void*)&nInput);
            if (CL_SUCCESS != err)
                throw OpenCLError(err, __FILE__, __LINE__);

            err = clSetKernelArg(redKernel, 3, sizeof(cl_int), (void*)&nCurWindowSize);
            if (CL_SUCCESS != err)
                throw OpenCLError(err, __FILE__, __LINE__);

            // set work group size and execute
            size_t global_work_size1[] = {256, (size_t)w };
            size_t local_work_size1[] = {256, 1};
            err = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, redKernel, 2, NULL,
                    global_work_size1, local_work_size1, 0, NULL, NULL);
            if (CL_SUCCESS != err)
                throw OpenCLError(err, __FILE__, __LINE__);
            err = clFinish(kEnv.mpkCmdQueue);
            if (CL_SUCCESS != err)
                throw OpenCLError(err, __FILE__, __LINE__);
            resbuf = (double*)clEnqueueMapBuffer(kEnv.mpkCmdQueue,
                mpClmem2,
                CL_TRUE, CL_MAP_READ, 0,
                sizeof(double)*w, 0, NULL, NULL,
                &err);
            if (err != CL_SUCCESS)
                throw OpenCLError(err, __FILE__, __LINE__);
            for (int i=0 ; i < w; i++)
                pAllBuffer[i+w] = resbuf[i];
            err = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue, mpClmem2, resbuf, 0, NULL, NULL);
            if (mpClmem2)
            {
                clReleaseMemObject(mpClmem2);
                mpClmem2 = NULL;
            }
            mpClmem2 = clCreateBuffer(kEnv.mpkContext,
                (cl_mem_flags) CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
                w*sizeof(double)*2, pAllBuffer.get(), &err);
            if (CL_SUCCESS != err)
                throw OpenCLError(err, __FILE__, __LINE__);
        }
        // set kernel arg
        err = clSetKernelArg(k, argno, sizeof(cl_mem), (void*)&(mpClmem2));
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);
        return 1;
    }
    ~ParallelReductionVectorRef()
    {
        if (mpClmem2)
        {
            clReleaseMemObject(mpClmem2);
            mpClmem2 = NULL;
        }
    }

    size_t GetArrayLength(void) const {return mpDVR->GetArrayLength(); }

    size_t GetWindowSize(void) const {return mpDVR->GetRefRowSize(); }

    size_t GetStartFixed(void) const {return bIsStartFixed; }

    size_t GetEndFixed(void) const {return bIsEndFixed; }

protected:
    bool bIsStartFixed, bIsEndFixed;
    const formula::DoubleVectorRefToken *mpDVR;
    // from parent nodes
    boost::shared_ptr<SlidingFunctionBase> mpCodeGen;
    // controls whether to invoke the reduction kernel during marshaling or not
    cl_mem mpClmem2;
};

class Reduction: public SlidingFunctionBase
{
public:
    typedef DynamicKernelSlidingArgument<VectorRef> NumericRange;
    typedef DynamicKernelSlidingArgument<DynamicKernelStringArgument> StringRange;
    typedef ParallelReductionVectorRef<VectorRef> ParallelNumericRange;

    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
    {
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ", ";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss << "double tmp = " << GetBottom() <<";\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        if (isAverage())
            ss << "int nCount = 0;\n\t";
        ss << "double tmpBottom;\n\t";
        unsigned i = vSubArguments.size();
        size_t nItems = 0;
        while (i--)
        {
            if (NumericRange *NR =
                    dynamic_cast<NumericRange *> (vSubArguments[i].get()))
            {
                bool needBody;
                nItems += NR->GenReductionLoopHeader(ss, needBody);
                if (needBody == false) continue;
            }
            else if (ParallelNumericRange *PNR =
                    dynamic_cast<ParallelNumericRange *> (vSubArguments[i].get()))
            {
                //did not handle yet
                bool needBody;
                nItems += PNR->GenReductionLoopHeader(ss, needBody);
                if (needBody == false) continue;
            }
            else if (StringRange *SR =
                    dynamic_cast<StringRange *> (vSubArguments[i].get()))
            {
                //did not handle yet
                bool needBody;
                nItems += SR->GenReductionLoopHeader(ss, needBody);
                if (needBody == false) continue;
            }
            else
            {
                FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
                assert(pCur);
                assert(pCur->GetType() != formula::svDoubleVectorRef);

                if (pCur->GetType() == formula::svSingleVectorRef)
                {
#ifdef  ISNAN
                    const formula::SingleVectorRefToken* pSVR =
                        static_cast< const formula::SingleVectorRefToken* >(pCur);
                    ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n\t\t";
#else
                    nItems += 1;
#endif
                }
                else if (pCur->GetType() == formula::svDouble)
                {
#ifdef  ISNAN
                    ss << "{\n\t\t";
#endif
                    nItems += 1;
                }
                else
                {
                    nItems += 1;
                }
            }
#ifdef  ISNAN
            if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
            {
                ss << "tmpBottom = " << GetBottom() << ";\n\t\t";
                ss << "if (isNan(";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << "))\n\t\t\t";
                ss << "tmp = ";
                ss << Gen2("tmpBottom", "tmp") << ";\n\t\t";
                ss << "else{\n\t\t\t";
                ss << "tmp = ";
                ss << Gen2(vSubArguments[i]->GenSlidingWindowDeclRef(), "tmp");
                ss << ";\n\t\t\t";
                ss << "}\n\t";
                ss << "}\n\t";
            }
            else
            {
                ss << "tmp = ";
                ss << Gen2(vSubArguments[i]->GenSlidingWindowDeclRef(), "tmp");
                ss << ";\n\t";
            }
#else
            ss << "tmp = ";
            // Generate the operation in binary form
            ss << Gen2(vSubArguments[i]->GenSlidingWindowDeclRef(), "tmp");
            ss << ";\n\t";
#endif
        }
        ss << "return tmp";
#ifdef  ISNAN
        if (isAverage())
            ss << "/(double)nCount";
#else
        if (isAverage())
            ss << "/(double)"<<nItems;
#endif
        ss << ";\n}";
    }
    virtual bool isAverage() const { return false; }
    virtual bool takeString() const { return false; }
    virtual bool takeNumeric() const { return true; }
};

// Strictly binary operators
class Binary: public SlidingFunctionBase
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
    {
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        assert(vSubArguments.size() == 2);
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ", ";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss << "int gid0 = get_global_id(0), i = 0;\n\t";
        ss << "double tmp = ";
        ss << Gen2(vSubArguments[0]->GenSlidingWindowDeclRef(false),
                vSubArguments[1]->GenSlidingWindowDeclRef(false)) << ";\n\t";
        ss << "return tmp;\n}";
    }
    virtual bool takeString() const { return true; }
    virtual bool takeNumeric() const { return true; }
};

class SumOfProduct: public SlidingFunctionBase
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
    {
        size_t nCurWindowSize = 0;
        FormulaToken *tmpCur = NULL;
        const formula::DoubleVectorRefToken *pCurDVR = NULL;
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
            size_t nCurChildWindowSize = vSubArguments[i]->GetWindowSize();
            nCurWindowSize = (nCurWindowSize < nCurChildWindowSize)?
                nCurChildWindowSize:nCurWindowSize;
            tmpCur = vSubArguments[i]->GetFormulaToken();
            if (  ocPush==tmpCur->GetOpCode() )
            {

                pCurDVR = static_cast<
                    const formula::DoubleVectorRefToken*>(tmpCur);
                if ( !
                    ( (!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
                    || (pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed()) )
                    )
                    throw Unhandled();
            }
        }
        ss << ") {\n";
        ss << "    double tmp = 0.0;\n";
        ss << "    int gid0 = get_global_id(0);\n";
#ifndef UNROLLING_FACTOR
        ss << "    int i ;\n";
        ss << "    for (i = 0; i < "<< nCurWindowSize <<"; i++)\n";
        ss << "    {\n";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            tmpCur = vSubArguments[i]->GetFormulaToken();
            if(ocPush==tmpCur->GetOpCode())
            {
                pCurDVR= static_cast<
                    const formula::DoubleVectorRefToken *>(tmpCur);
                if(!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
                {
                   ss << "        int currentCount";
                   ss << i;
                   ss <<" =i+gid0+1;\n";
                }
                else
                {
                    ss << "        int currentCount";
                    ss << i;
                    ss << " =i+1;\n";
                }
             }
        }
        ss << "        tmp += fsum(";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << "*";
#ifdef  ISNAN
            if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
            {
                ss <<"(";
                ss <<"(currentCount";
                ss << i;
                ss<< ">";
                if(vSubArguments[i]->GetFormulaToken()->GetType() ==
                     formula::svSingleVectorRef)
                {
                    const formula::SingleVectorRefToken* pSVR =
                    static_cast< const formula::SingleVectorRefToken*>
                         (vSubArguments[i]->GetFormulaToken());
                    ss<<pSVR->GetArrayLength();
                }
                else if(vSubArguments[i]->GetFormulaToken()->GetType() ==
                          formula::svDoubleVectorRef)
                {
                    const formula::DoubleVectorRefToken* pSVR =
                    static_cast< const formula::DoubleVectorRefToken*>
                          (vSubArguments[i]->GetFormulaToken());
                    ss<<pSVR->GetArrayLength();
                }
                ss << ")||isNan("<<vSubArguments[i]
                    ->GenSlidingWindowDeclRef(true);
                ss << ")?0:";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef(true);
                ss << ")";
            }
            else
                ss << vSubArguments[i]->GenSlidingWindowDeclRef(true);
#else
            ss << vSubArguments[i]->GenSlidingWindowDeclRef(true);
#endif
        }
        ss << ", 0.0);\n\t}\n\t";
        ss << "return tmp;\n";
        ss << "}";
#endif

#ifdef UNROLLING_FACTOR
        ss << "\tint i;\n\t";
        ss << "int currentCount0, currentCount1;\n\t";
        std::stringstream temp3,temp4;
        int outLoopSize = UNROLLING_FACTOR;
        if (nCurWindowSize/outLoopSize != 0){
            ss << "for(int outLoop=0; outLoop<" <<
            nCurWindowSize/outLoopSize<< "; outLoop++){\n\t";
            for(int count=0; count < outLoopSize; count++){
                ss << "i = outLoop*"<<outLoopSize<<"+"<<count<<";\n";
                if(count==0){
                    for (unsigned i = 0; i < vSubArguments.size(); i++)
                    {
                        tmpCur = vSubArguments[i]->GetFormulaToken();
                        if(ocPush==tmpCur->GetOpCode())
                        {
                            pCurDVR= static_cast<
                                const formula::DoubleVectorRefToken *>(tmpCur);
                            if(!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
                            {
                                temp3 << "        currentCount";
                                temp3 << i;
                                temp3 <<" =i+gid0+1;\n";
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
                    for (unsigned i = 0; i < vSubArguments.size(); i++){
                        if (i)
                            temp3 << "*";
                        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode()){
                            temp3 <<"(";
                            temp3 <<"(currentCount";
                            temp3 << i;
                            temp3 << ">";
                            if(vSubArguments[i]->GetFormulaToken()->GetType() ==
                                    formula::svSingleVectorRef){
                                const formula::SingleVectorRefToken* pSVR =
                                    static_cast< const formula::SingleVectorRefToken*>
                                    (vSubArguments[i]->GetFormulaToken());
                                temp3<<pSVR->GetArrayLength();
                            }
                            else if(vSubArguments[i]->GetFormulaToken()->GetType() ==
                                    formula::svDoubleVectorRef){
                                const formula::DoubleVectorRefToken* pSVR =
                                    static_cast< const formula::DoubleVectorRefToken*>
                                    (vSubArguments[i]->GetFormulaToken());
                                temp3<<pSVR->GetArrayLength();
                            }
                            temp3 << ")||isNan("<<vSubArguments[i]
                                ->GenSlidingWindowDeclRef(true);
                            temp3 << ")?0:";
                            temp3 << vSubArguments[i]->GenSlidingWindowDeclRef(true);
                            temp3  << ")";
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
        for(unsigned int count=nCurWindowSize/outLoopSize*outLoopSize;
        count < nCurWindowSize; count++)
        {
            ss << "i =" <<count<<";\n";
            if(count==nCurWindowSize/outLoopSize*outLoopSize){
                for (unsigned i = 0; i < vSubArguments.size(); i++)
                {
                    tmpCur = vSubArguments[i]->GetFormulaToken();
                    if(ocPush==tmpCur->GetOpCode())
                    {
                        pCurDVR= static_cast<
                            const formula::DoubleVectorRefToken *>(tmpCur);
                        if(!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
                        {
                            temp4 << "        currentCount";
                            temp4 << i;
                            temp4 <<" =i+gid0+1;\n";
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
                for (unsigned i = 0; i < vSubArguments.size(); i++)
                {
                    if (i)
                        temp4 << "*";
                    if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
                    {
                        temp4 <<"(";
                        temp4 <<"(currentCount";
                        temp4 << i;
                        temp4 << ">";
                       if(vSubArguments[i]->GetFormulaToken()->GetType() ==
                                formula::svSingleVectorRef)
                        {
                            const formula::SingleVectorRefToken* pSVR =
                                static_cast< const formula::SingleVectorRefToken*>
                                (vSubArguments[i]->GetFormulaToken());
                            temp4<<pSVR->GetArrayLength();
                        }
                        else if(vSubArguments[i]->GetFormulaToken()->GetType() ==
                                formula::svDoubleVectorRef)
                        {
                            const formula::DoubleVectorRefToken* pSVR =
                                static_cast< const formula::DoubleVectorRefToken*>
                                (vSubArguments[i]->GetFormulaToken());
                            temp4<<pSVR->GetArrayLength();
                        }
                        temp4 << ")||isNan("<<vSubArguments[i]
                            ->GenSlidingWindowDeclRef(true);
                        temp4 << ")?0:";
                        temp4 << vSubArguments[i]->GenSlidingWindowDeclRef(true);
                        temp4  << ")";
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
#endif

    }
    virtual bool takeString() const { return false; }
    virtual bool takeNumeric() const { return true; }
};

/// operator traits
class OpNop: public Reduction {
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string Gen2(const std::string &lhs, const std::string &) const
    {
        return lhs;
    }
    virtual std::string BinFuncName(void) const { return "nop"; }
};

class OpCount: public Reduction {
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string Gen2(const std::string &lhs, const std::string &rhs) const
    {
        std::stringstream ss;
        ss << "(isNan(" << lhs << ")?"<<rhs<<":"<<rhs<<"+1.0)";
        return ss.str();
    }
    virtual std::string BinFuncName(void) const { return "fcount"; }
};

class OpEqual: public Binary {
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string Gen2(const std::string &lhs, const std::string &rhs) const
    {
        std::stringstream ss;
        ss << "strequal("<< lhs << "," << rhs <<")";
        return ss.str();
    }
    virtual std::string BinFuncName(void) const { return "eq"; }
};

class OpLessEqual: public Binary {
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string Gen2(const std::string &lhs, const std::string &rhs) const
    {
        std::stringstream ss;
        ss << "("<< lhs << "<=" << rhs <<")";
        return ss.str();
    }
    virtual std::string BinFuncName(void) const { return "leq"; }
};

class OpGreater: public Binary {
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string Gen2(const std::string &lhs, const std::string &rhs) const
    {
        std::stringstream ss;
        ss << "("<< lhs << ">" << rhs <<")";
        return ss.str();
    }
    virtual std::string BinFuncName(void) const { return "gt"; }
};

class OpSum: public Reduction {
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string Gen2(const std::string &lhs, const std::string &rhs) const
    {
        std::stringstream ss;
        ss << "((" << lhs <<")+("<< rhs<<"))";
        return ss.str();
    }
    virtual std::string BinFuncName(void) const { return "fsum"; }
};

class OpAverage: public Reduction {
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string Gen2(const std::string &lhs, const std::string &rhs) const
    {
        std::stringstream ss;
        ss << "fsum_count(" << lhs <<","<< rhs<<", &nCount)";
        return ss.str();
    }
    virtual std::string BinFuncName(void) const { return "fsum"; }
    virtual bool isAverage() const { return true; }
};

class OpSub: public Reduction {
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string Gen2(const std::string &lhs, const std::string &rhs) const
    {
        return lhs + "-" + rhs;
    }
    virtual std::string BinFuncName(void) const { return "fsub"; }
};

class OpMul: public Reduction {
public:
    virtual std::string GetBottom(void) { return "1"; }
    virtual std::string Gen2(const std::string &lhs, const std::string &rhs) const
    {
        return lhs + "*" + rhs;
    }
    virtual std::string BinFuncName(void) const { return "fmul"; }
};

/// Technically not a reduction, but fits the framework.
class OpDiv: public Reduction {
public:
    virtual std::string GetBottom(void) { return "1.0"; }
    virtual std::string Gen2(const std::string &lhs, const std::string &rhs) const
    {
        return "(" + lhs + "/" + rhs + ")";
    }
    virtual std::string BinFuncName(void) const { return "fdiv"; }
};

class OpMin: public Reduction {
public:
    virtual std::string GetBottom(void) { return "MAXFLOAT"; }
    virtual std::string Gen2(const std::string &lhs, const std::string &rhs) const
    {
        return "mcw_fmin("+lhs + "," + rhs +")";
    }
    virtual std::string BinFuncName(void) const { return "min"; }
};

class OpMax: public Reduction {
public:
    virtual std::string GetBottom(void) { return "-MAXFLOAT"; }
    virtual std::string Gen2(const std::string &lhs, const std::string &rhs) const
    {
        return "mcw_fmax("+lhs + "," + rhs +")";
    }
    virtual std::string BinFuncName(void) const { return "max"; }
};
class OpSumProduct: public SumOfProduct {
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string Gen2(const std::string &lhs, const std::string &rhs) const
    {
        return lhs + "*" + rhs;
    }
    virtual std::string BinFuncName(void) const { return "fsop"; }
};
namespace {
struct SumIfsArgs {
    SumIfsArgs(cl_mem x): mCLMem(x), mConst(0.0) {}
    SumIfsArgs(double x): mCLMem(NULL), mConst(x) {}
    cl_mem mCLMem;
    double mConst;
};
}
/// Helper functions that have multiple buffers
class DynamicKernelSoPArguments: public DynamicKernelArgument
{
public:
    typedef boost::shared_ptr<DynamicKernelArgument> SubArgument;
    typedef std::vector<SubArgument> SubArgumentsType;

    DynamicKernelSoPArguments(
        const std::string &s, const FormulaTreeNodeRef& ft, SlidingFunctionBase* pCodeGen);

    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal(cl_kernel k, int argno, int nVectorWidth, cl_program pProgram)
    {
        unsigned i = 0;
        for (SubArgumentsType::iterator it = mvSubArguments.begin(), e= mvSubArguments.end(); it!=e;
                ++it)
        {
            i += (*it)->Marshal(k, argno + i, nVectorWidth, pProgram);
        }
        if (OpGeoMean *OpSumCodeGen = dynamic_cast<OpGeoMean*>(mpCodeGen.get()))
        {
            // Obtain cl context
            KernelEnv kEnv;
            OpenclDevice::setKernelEnv(&kEnv);
            cl_int err;
            cl_mem pClmem2;

            if (OpSumCodeGen->NeedReductionKernel())
            {
                std::vector<cl_mem> vclmem;
                for (SubArgumentsType::iterator it = mvSubArguments.begin(),
                        e= mvSubArguments.end(); it!=e; ++it)
                {
                    if (VectorRef *VR = dynamic_cast<VectorRef *>(it->get()))
                        vclmem.push_back(VR->GetCLBuffer());
                    else
                        vclmem.push_back(NULL);
                }
                pClmem2 = clCreateBuffer(kEnv.mpkContext, CL_MEM_READ_WRITE,
                        sizeof(double)*nVectorWidth, NULL, &err);
                if (CL_SUCCESS != err)
                    throw OpenCLError(err, __FILE__, __LINE__);

                std::string kernelName = "GeoMean_reduction";
                cl_kernel redKernel = clCreateKernel(pProgram, kernelName.c_str(), &err);
                if (err != CL_SUCCESS)
                    throw OpenCLError(err, __FILE__, __LINE__);
                    // set kernel arg of reduction kernel
                for (size_t j=0; j< vclmem.size(); j++){
                    err = clSetKernelArg(redKernel, j,
                            vclmem[j]?sizeof(cl_mem):sizeof(double),
                            (void *)&vclmem[j]);
                    if (CL_SUCCESS != err)
                        throw OpenCLError(err, __FILE__, __LINE__);
                }
                err = clSetKernelArg(redKernel, vclmem.size(), sizeof(cl_mem), (void *)&pClmem2);
                if (CL_SUCCESS != err)
                    throw OpenCLError(err, __FILE__, __LINE__);

                // set work group size and execute
                size_t global_work_size[] = {256, (size_t)nVectorWidth };
                size_t local_work_size[] = {256, 1};
                err = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, redKernel, 2, NULL,
                        global_work_size, local_work_size, 0, NULL, NULL);
                if (CL_SUCCESS != err)
                    throw OpenCLError(err, __FILE__, __LINE__);
                err = clFinish(kEnv.mpkCmdQueue);
                if (CL_SUCCESS != err)
                    throw OpenCLError(err, __FILE__, __LINE__);

                 // Pass pClmem2 to the "real" kernel
                err = clSetKernelArg(k, argno, sizeof(cl_mem), (void *)&pClmem2);
                if (CL_SUCCESS != err)
                    throw OpenCLError(err, __FILE__, __LINE__);
            }
         }
        if (OpSumIfs *OpSumCodeGen = dynamic_cast<OpSumIfs*>(mpCodeGen.get()))
        {
            // Obtain cl context
            KernelEnv kEnv;
            OpenclDevice::setKernelEnv(&kEnv);
            cl_int err;
            DynamicKernelArgument *Arg = mvSubArguments[0].get();
            DynamicKernelSlidingArgument<VectorRef> *slidingArgPtr =
                static_cast< DynamicKernelSlidingArgument<VectorRef> *> (Arg);
            mpClmem2 = NULL;

            if (OpSumCodeGen->NeedReductionKernel())
            {
                size_t nInput = slidingArgPtr -> GetArrayLength();
                size_t nCurWindowSize = slidingArgPtr -> GetWindowSize();
                std::vector<SumIfsArgs> vclmem;

                for (SubArgumentsType::iterator it = mvSubArguments.begin(),
                        e= mvSubArguments.end(); it!=e; ++it)
                {
                    if (VectorRef *VR = dynamic_cast<VectorRef *>(it->get()))
                        vclmem.push_back(SumIfsArgs(VR->GetCLBuffer()));
                    else if (DynamicKernelConstantArgument *CA =
                            dynamic_cast<
                            DynamicKernelConstantArgument *>(it->get()))
                        vclmem.push_back(SumIfsArgs(CA->GetDouble()));
                    else
                        vclmem.push_back(SumIfsArgs((cl_mem)NULL));
                }
                mpClmem2 = clCreateBuffer(kEnv.mpkContext, CL_MEM_READ_WRITE,
                        sizeof(double)*nVectorWidth, NULL, &err);
                if (CL_SUCCESS != err)
                    throw OpenCLError(err, __FILE__, __LINE__);

                std::string kernelName = mvSubArguments[0]->GetName() + "_SumIfs_reduction";
                cl_kernel redKernel = clCreateKernel(pProgram, kernelName.c_str(), &err);
                if (err != CL_SUCCESS)
                    throw OpenCLError(err, __FILE__, __LINE__);

                    // set kernel arg of reduction kernel
                for (size_t j=0; j< vclmem.size(); j++){
                    err = clSetKernelArg(redKernel, j,
                            vclmem[j].mCLMem?sizeof(cl_mem):sizeof(double),
                            vclmem[j].mCLMem?(void *)&vclmem[j].mCLMem:
                            (void*)&vclmem[j].mConst);
                    if (CL_SUCCESS != err)
                        throw OpenCLError(err, __FILE__, __LINE__);
                }
                err = clSetKernelArg(redKernel, vclmem.size(), sizeof(cl_mem), (void *)&mpClmem2);
                if (CL_SUCCESS != err)
                    throw OpenCLError(err, __FILE__, __LINE__);

                err = clSetKernelArg(redKernel, vclmem.size()+1, sizeof(cl_int), (void*)&nInput);
                if (CL_SUCCESS != err)
                    throw OpenCLError(err, __FILE__, __LINE__);

                err = clSetKernelArg(redKernel, vclmem.size()+2, sizeof(cl_int), (void*)&nCurWindowSize);
                if (CL_SUCCESS != err)
                    throw OpenCLError(err, __FILE__, __LINE__);
                // set work group size and execute
                size_t global_work_size[] = {256, (size_t)nVectorWidth };
                size_t local_work_size[] = {256, 1};
                err = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, redKernel, 2, NULL,
                        global_work_size, local_work_size, 0, NULL, NULL);
                if (CL_SUCCESS != err)
                    throw OpenCLError(err, __FILE__, __LINE__);
                err = clFinish(kEnv.mpkCmdQueue);
                if (CL_SUCCESS != err)
                    throw OpenCLError(err, __FILE__, __LINE__);
                clReleaseKernel(redKernel);
                 // Pass mpClmem2 to the "real" kernel
                err = clSetKernelArg(k, argno, sizeof(cl_mem), (void *)&mpClmem2);
                if (CL_SUCCESS != err)
                    throw OpenCLError(err, __FILE__, __LINE__);
            }
        }
        return i;
    }

    virtual void GenSlidingWindowFunction(std::stringstream &ss) {
        for (unsigned i = 0; i < mvSubArguments.size(); i++)
            mvSubArguments[i]->GenSlidingWindowFunction(ss);
        mpCodeGen->GenSlidingWindowFunction(ss, mSymName, mvSubArguments);
    }
    virtual void GenDeclRef(std::stringstream &ss) const
    {
        for (unsigned i = 0; i < mvSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            mvSubArguments[i]->GenDeclRef(ss);
        }
    }
    virtual void GenDecl(std::stringstream &ss) const
    {
        for (SubArgumentsType::const_iterator it = mvSubArguments.begin(), e= mvSubArguments.end(); it!=e;
            ++it) {
            if (it != mvSubArguments.begin())
                ss << ", ";
            (*it)->GenDecl(ss);
        }
    }

    virtual size_t GetWindowSize(void) const
    {
        size_t nCurWindowSize = 0;
        for (unsigned i = 0; i < mvSubArguments.size(); i++)
        {
            size_t nCurChildWindowSize = mvSubArguments[i]->GetWindowSize();
            nCurWindowSize = (nCurWindowSize < nCurChildWindowSize) ?
                nCurChildWindowSize:nCurWindowSize;
        }
        return nCurWindowSize;
    }

    /// When declared as input to a sliding window function
    virtual void GenSlidingWindowDecl(std::stringstream &ss) const
    {
        for (SubArgumentsType::const_iterator it = mvSubArguments.begin(), e= mvSubArguments.end(); it!=e;
            ++it)
        {
            if (it != mvSubArguments.begin())
                ss << ", ";
            (*it)->GenSlidingWindowDecl(ss);
        }
    }
    /// Generate either a function call to each children
    /// or direclty inline it if we are already inside a loop
    virtual std::string GenSlidingWindowDeclRef(bool nested=false) const
    {
        std::stringstream ss;
        if (!nested)
        {
            ss << mSymName << "_" << mpCodeGen->BinFuncName() <<"(";
            for (unsigned i = 0; i < mvSubArguments.size(); i++)
            {
                if (i)
                    ss << ", ";
                if (!nested)
                    mvSubArguments[i]->GenDeclRef(ss);
                else
                    ss << mvSubArguments[i]->GenSlidingWindowDeclRef(true);
            }
            ss << ")";
        } else {
            if (mvSubArguments.size() != 2)
                throw Unhandled();
            ss << "(" << mpCodeGen->Gen2(mvSubArguments[0]->GenSlidingWindowDeclRef(true),
                         mvSubArguments[1]->GenSlidingWindowDeclRef(true)) << ")";
        }
        return ss.str();
    }
    virtual std::string DumpOpName(void) const
    {
        std::string t = "_" + mpCodeGen->BinFuncName();
        for (unsigned i = 0; i < mvSubArguments.size(); i++)
            t = t + mvSubArguments[i]->DumpOpName();
        return t;
    }
    virtual void DumpInlineFun(std::set<std::string>& decls,
                                std::set<std::string>& funs) const
    {
        mpCodeGen->BinInlineFun(decls,funs);
        for (unsigned i = 0; i < mvSubArguments.size(); i++)
            mvSubArguments[i]->DumpInlineFun(decls,funs);
    }
   ~DynamicKernelSoPArguments()
    {
        if (mpClmem2)
        {
            clReleaseMemObject(mpClmem2);
            mpClmem2 = NULL;
        }
    }
private:
    SubArgumentsType mvSubArguments;
    boost::shared_ptr<SlidingFunctionBase> mpCodeGen;
    cl_mem mpClmem2;
};

boost::shared_ptr<DynamicKernelArgument> SoPHelper(
    const std::string &ts, const FormulaTreeNodeRef& ft, SlidingFunctionBase* pCodeGen)
{
    return boost::shared_ptr<DynamicKernelArgument>(new DynamicKernelSoPArguments(ts, ft, pCodeGen));
}

template<class Base>
DynamicKernelArgument *VectorRefFactory(const std::string &s,
        const FormulaTreeNodeRef& ft,
        boost::shared_ptr<SlidingFunctionBase> &pCodeGen,
        int index)
{
    //Black lists ineligible classes here ..
    // SUMIFS does not perform parallel reduction at DoubleVectorRef level
    if (dynamic_cast<OpSumIfs*>(pCodeGen.get())) {
        if (index == 0) // the first argument of OpSumIfs cannot be strings anyway
            return new DynamicKernelSlidingArgument<VectorRef>(s, ft, pCodeGen, index);
        return new DynamicKernelSlidingArgument<Base>(s, ft, pCodeGen, index);
    }
    // AVERAGE is not supported yet
    //Average has been supported by reduction kernel
    /*else if (dynamic_cast<OpAverage*>(pCodeGen.get()))
    {
        return new DynamicKernelSlidingArgument<Base>(s, ft, pCodeGen, index);
    }*/
    // MUL is not supported yet
    else if (dynamic_cast<OpMul*>(pCodeGen.get()))
    {
        return new DynamicKernelSlidingArgument<Base>(s, ft, pCodeGen, index);
    }
    // Sub is not a reduction per se
    else if (dynamic_cast<OpSub*>(pCodeGen.get()))
    {
        return new DynamicKernelSlidingArgument<Base>(s, ft, pCodeGen, index);
    }
    // Only child class of Reduction is supported
    else if (!dynamic_cast<Reduction*>(pCodeGen.get()))
    {
        return new DynamicKernelSlidingArgument<Base>(s, ft, pCodeGen, index);
    }

    const formula::DoubleVectorRefToken* pDVR =
        static_cast< const formula::DoubleVectorRefToken* >(
                ft->GetFormulaToken());
    // Window being too small to justify a parallel reduction
    if (pDVR->GetRefRowSize() < REDUCE_THRESHOLD)
        return new DynamicKernelSlidingArgument<Base>(s, ft, pCodeGen, index);
    if ((pDVR->IsStartFixed() && pDVR->IsEndFixed()) ||
            (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()))
        return new ParallelReductionVectorRef<Base>(s, ft, pCodeGen, index);
    else // Other cases are not supported as well
        return new DynamicKernelSlidingArgument<Base>(s, ft, pCodeGen, index);
}

DynamicKernelSoPArguments::DynamicKernelSoPArguments(
    const std::string &s, const FormulaTreeNodeRef& ft, SlidingFunctionBase* pCodeGen) :
    DynamicKernelArgument(s, ft), mpCodeGen(pCodeGen), mpClmem2(NULL)
{
    size_t nChildren = ft->Children.size();

    for (unsigned i = 0; i < nChildren; i++)
    {
        FormulaToken *pChild = ft->Children[i]->GetFormulaToken();
        if (!pChild)
            throw Unhandled();
        OpCode opc = pChild->GetOpCode();
        std::stringstream tmpname;
        tmpname << s << "_" << i;
        std::string ts = tmpname.str();
        switch(opc) {
            case ocPush:
                if (pChild->GetType() == formula::svDoubleVectorRef)
                {
                    const formula::DoubleVectorRefToken* pDVR =
                        static_cast< const formula::DoubleVectorRefToken* >(pChild);
                    for (size_t j = 0; j < pDVR->GetArrays().size(); ++j)
                    {
                        if (pDVR->GetArrays()[j].mpNumericArray ||
                            (pDVR->GetArrays()[j].mpNumericArray == NULL &&
                            pDVR->GetArrays()[j].mpStringArray == NULL ))
                        {
                            if(pDVR->GetArrays()[j].mpNumericArray &&
                                pCodeGen->takeNumeric() &&
                                pDVR->GetArrays()[j].mpStringArray &&
                                pCodeGen->takeString())
                            {
                                mvSubArguments.push_back(
                                    SubArgument(
                                    new DynamicKernelMixedSlidingArgument(
                                    ts, ft->Children[i], mpCodeGen, j)));
                            }
                            else
                            {
                                mvSubArguments.push_back(
                                    SubArgument(VectorRefFactory<VectorRef>(
                                    ts, ft->Children[i], mpCodeGen, j)));
                            }
                        }
                        else
                            mvSubArguments.push_back(
                                    SubArgument(VectorRefFactory
                                        <DynamicKernelStringArgument>(
                                            ts, ft->Children[i], mpCodeGen, j)));
                    }
                } else if (pChild->GetType() == formula::svSingleVectorRef) {
                    const formula::SingleVectorRefToken* pSVR =
                        static_cast< const formula::SingleVectorRefToken* >(pChild);
                    if (pSVR->GetArray().mpNumericArray &&
                        pCodeGen->takeNumeric() &&
                        pSVR->GetArray().mpStringArray &&
                        pCodeGen->takeString())
                    {
                        mvSubArguments.push_back(
                                SubArgument(new DynamicKernelMixedArgument(
                                        ts, ft->Children[i])));
                    }
                    else if (pSVR->GetArray().mpNumericArray &&
                            pCodeGen->takeNumeric())
                    {
                        mvSubArguments.push_back(
                                SubArgument(new VectorRef(ts,
                                        ft->Children[i])));
                    }
                    else if (pSVR->GetArray().mpStringArray &&
                            pCodeGen->takeString())
                    {
                        mvSubArguments.push_back(
                                SubArgument(new DynamicKernelStringArgument(
                                        ts, ft->Children[i])));
                    }
                    else if (pSVR->GetArray().mpStringArray == NULL &&
                        pSVR->GetArray().mpNumericArray == NULL)
                    {
                        // Push as an array of NANs
                        mvSubArguments.push_back(
                                SubArgument(new VectorRef(ts,
                                        ft->Children[i])));
                    }
                    else
                        throw UnhandledToken(pChild,
                                "Got unhandled case here", __FILE__, __LINE__);
                } else if (pChild->GetType() == formula::svDouble) {
                    mvSubArguments.push_back(
                            SubArgument(new DynamicKernelConstantArgument(ts,
                                    ft->Children[i])));
                } else if (pChild->GetType() == formula::svString
                && pCodeGen->takeString()) {
                    mvSubArguments.push_back(
                            SubArgument(new ConstStringArgument(ts,
                                    ft->Children[i])));
                } else {
                    throw UnhandledToken(pChild, "unknown operand for ocPush");
                }
                break;
            case ocDiv:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpDiv));
                break;
            case ocMul:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpMul));
                break;
            case ocSub:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpSub));
                break;
            case ocAdd:
            case ocSum:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpSum));
                break;
            case ocAverage:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpAverage));
                break;
            case ocMin:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpMin));
                break;
            case ocMax:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpMax));
                break;
            case ocCount:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpCount));
                break;
            case ocSumProduct:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpSumProduct));
                break;
            case ocIRR:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpIRR));
                break;
            case ocMIRR:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpMIRR));
                break;
            case ocRMZ:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpPMT));
                break;
            case ocZins:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpIntrate));
                break;
            case ocZGZ:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpRRI));
                break;
            case ocKapz:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpPPMT));
                break;
            case ocFisher:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpFisher));
                break;
            case ocFisherInv:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpFisherInv));
                break;
            case ocGamma:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpGamma));
                break;
            case ocLIA:
                 mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpSLN));
                break;
            case ocGammaLn:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpGammaLn));
                break;
            case ocGauss:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpGauss));
                break;
            /*case ocGeoMean:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpGeoMean));
                break;*/
            case ocHarMean:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpHarMean));
                break;
            case ocLessEqual:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpLessEqual));
                break;
            case ocEqual:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpEqual));
                break;
            case ocGreater:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpGreater));
                break;
            case ocDIA:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpSYD));
                break;
            case ocCorrel:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpCorrel));
                break;
            case ocCos:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpCos));
                break;
            case ocNegBinomVert :
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpNegbinomdist));
                break;
            case ocPearson:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpPearson));
                break;
            case ocRSQ:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpRsq));
                break;
            case ocCosecant:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpCsc));
                break;
            case ocISPMT:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpISPMT));
                break;
            case ocLaufz:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpDuration));
                break;
            case ocSinHyp:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpSinh));
                break;
            case ocAbs:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpAbs));
                break;
            case ocBW:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpPV));
                break;
            case ocSin:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpSin));
                break;
            case ocTan:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpTan));
                break;
            case ocTanHyp:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpTanH));
                break;
            case ocStandard:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpStandard));
                break;
            case ocWeibull:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpWeibull));
                break;
            /*case ocMedian:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpMedian));
                break;*/
            case ocGDA:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpDDB));
                break;
            case ocZW:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpFV));
                break;
            case ocSumIfs:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpSumIfs));
                 break;
            /*case ocVBD:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpVDB));
                 break;*/
            case ocKurt:
                mvSubArguments.push_back(SoPHelper(ts,
                        ft->Children[i], new OpKurt));
                 break;
            /*case ocZZR:
                mvSubArguments.push_back(SoPHelper(ts,
                        ft->Children[i], new OpNper));
                 break;*/
            case ocNormDist:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpNormdist));
                 break;
            case ocArcCos:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpArcCos));
                 break;
            case ocSqrt:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpSqrt));
                break;
            case ocArcCosHyp:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpArcCosHyp));
                break;
            case ocNPV:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpNPV));
                break;
            case ocStdNormDist:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpNormsdist));
                break;
            case ocNormInv:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpNorminv));
                break;
            case ocSNormInv:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpNormsinv));
                break;
            case ocVariationen:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpVariationen));
                break;
            case ocVariationen2:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpVariationen2));
                break;
            case ocPhi:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpPhi));
                break;
            case ocZinsZ:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpIPMT));
                 break;
            case ocConfidence:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpConfidence));
                break;
            case ocIntercept:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpIntercept));
                break;
            case ocGDA2:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                    new OpDB));
                break;
            case ocLogInv:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpLogInv));
                break;
            case ocArcCot:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpArcCot));
                break;
            case ocCosHyp:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpCosh));
                break;
            case ocKritBinom:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpCritBinom));
                break;
            case ocArcCotHyp:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpArcCotHyp));
                break;
            case ocArcSin:
                mvSubArguments.push_back(SoPHelper(ts,
                        ft->Children[i], new OpArcSin));
                break;
            case ocArcSinHyp:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpArcSinHyp));
                break;
            case ocArcTan:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpArcTan));
                break;
            case ocArcTanHyp:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpArcTanH));
                break;
            case ocBitAnd:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpBitAnd));
                break;
            case ocForecast:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpForecast));
                break;
            case ocLogNormDist:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpLogNormDist));
                break;
            /*case ocGammaDist:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpGammaDist));
                break;*/
            case ocLn:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpLn));
                break;
            case ocRound:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpRound));
                break;
            case ocCot:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpCot));
                break;
            case ocCotHyp:
                mvSubArguments.push_back(SoPHelper(ts,
                        ft->Children[i], new OpCoth));
                break;
            case ocFDist:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpFdist));
                break;
            case ocVar:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpVar));
                break;
            /*case ocChiDist:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpChiDist));
                break;*/
            case ocPow:
            case ocPower:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpPower));
                break;
            case ocOdd:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpOdd));
                break;
            /*case ocChiSqDist:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpChiSqDist));
                break;
            case ocChiSqInv:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpChiSqInv));
                break;
            case ocGammaInv:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpGammaInv));
                break;*/
            case ocFloor:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpFloor));
                break;
            /*case ocFInv:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpFInv));
                break;*/
            case ocFTest:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpFTest));
                break;
            case ocB:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpB));
                break;
            case ocBetaDist:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpBetaDist));
                break;
             case ocCosecantHyp:
                mvSubArguments.push_back(SoPHelper(ts,
                          ft->Children[i], new OpCscH));
                break;
            case ocExp:
                mvSubArguments.push_back(SoPHelper(ts,
                           ft->Children[i], new OpExp));
                break;
            case ocLog10:
                mvSubArguments.push_back(SoPHelper(ts,
                           ft->Children[i], new OpLog10));
                break;
            case ocExpDist:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpExponDist));
                break;
            case ocAverageIfs:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpAverageIfs));
                break;
            case ocCountIfs:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpCountIfs));
                break;
            case ocKombin2:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpCombina));
                break;
            case ocEven:
                mvSubArguments.push_back(SoPHelper(ts,
                        ft->Children[i], new OpEven));
                break;
            case ocLog:
                mvSubArguments.push_back(SoPHelper(ts,
                        ft->Children[i], new OpLog));
                break;
            case ocMod:
                mvSubArguments.push_back(SoPHelper(ts,
                       ft->Children[i], new OpMod));
                break;
            case ocTrunc:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpTrunc));
                break;
            case ocSchiefe:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpSkew));
                break;
            case ocArcTan2:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpArcTan2));
                break;
            case ocBitOr:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpBitOr));
                break;
            case ocBitLshift:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpBitLshift));
                break;
            case ocBitRshift:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpBitRshift));
                break;
            case ocBitXor:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpBitXor));
                break;
            /*case ocChiInv:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpChiInv));
                break;*/
            case ocPoissonDist:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpPoisson));
                break;
            case ocSumSQ:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpSumSQ));
                break;
            case ocSkewp:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpSkewp));
                break;
            case ocBinomDist:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpBinomdist));
                break;
            case ocVarP:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpVarP));
                break;
            case ocCeil:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpCeil));
                break;
            case ocKombin:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpKombin));
                break;
            case ocDevSq:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpDevSq));
                break;
            case ocStDev:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpStDev));
                break;
            case ocSlope:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpSlope));
                break;
            case ocSTEYX:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpSTEYX));
                break;
            case ocZTest:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpZTest));
                break;
            case ocPi:
                mvSubArguments.push_back(
                SubArgument(new DynamicKernelPiArgument(ts,
                ft->Children[i])));
                break;
            case ocRandom:
                mvSubArguments.push_back(
                SubArgument(new DynamicKernelRandomArgument(ts,
                ft->Children[i])));
                break;
            case ocProduct:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpProduct));
                break;
            /*case ocHypGeomDist:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpHypGeomDist));
                break;*/
            case ocSumX2MY2:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpSumX2MY2));
                break;
            case ocSumX2DY2:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpSumX2PY2));
                 break;
            /*case ocBetaInv:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpBetainv));
                 break;*/
            case ocTTest:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpTTest));
                 break;
            case ocTDist:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpTDist));
                 break;
            /*case ocTInv:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpTInv));
                 break;*/
            case ocSumXMY2:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpSumXMY2));
                 break;
            case ocStDevP:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpStDevP));
                 break;
            case ocCovar:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpCovar));
                 break;
            case ocAnd:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpAnd));
                 break;
            case ocVLookup:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpVLookup));
                 break;
            case ocOr:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpOr));
                 break;
            case ocNot:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpNot));
                 break;
            case ocXor:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpXor));
                 break;
            case ocDBMax:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpDmax));
                 break;
            case ocDBMin:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpDmin));
                 break;
            case ocDBProduct:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpDproduct));
                 break;
            case ocDBAverage:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpDaverage));
                 break;
            case ocDBStdDev:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpDstdev));
                 break;
            case ocDBStdDevP:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpDstdevp));
                 break;
            case ocDBSum:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpDsum));
                 break;
            case ocDBVar:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpDvar));
                 break;
            case ocDBVarP:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpDvarp));
                 break;
            case ocAverageIf:
                mvSubArguments.push_back(SoPHelper(ts,
                    ft->Children[i], new OpAverageIf));
                break;
            case ocDBCount:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpDcount));
                 break;
            case ocDBCount2:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpDcount2));
                 break;
            case ocDeg:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpDeg));
                 break;
            case ocRoundUp:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpRoundUp));
                break;
            case ocRoundDown:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpRoundDown));
                break;
            case ocInt:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpInt));
                break;
            case ocRad:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpRadians));
                break;
            case ocCountIf:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpCountIf));
                 break;
            case ocIsEven:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpIsEven));
                break;
            case ocIsOdd:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpIsOdd));
                break;
            case ocFact:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpFact));
                 break;
            case ocMinA:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpMinA));
                 break;
            case ocCount2:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpCountA));
                 break;
            case ocMaxA:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpMaxA));
                 break;
            case ocAverageA:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpAverageA));
                 break;
            case ocVarA:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpVarA));
                 break;
            case ocVarPA:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpVarPA));
                 break;
            case ocStDevA:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpStDevA));
                 break;
            case ocStDevPA:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpStDevPA));
                 break;
            case ocSecant:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpSec));
                break;
            case ocSecantHyp:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpSecH));
                break;
            case ocSumIf:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpSumIf));
                 break;
            /*case ocNegSub:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpNegSub));
                 break;*/
            case ocAveDev:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i], new OpAveDev));
                 break;
            case ocExternal:
                if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getEffect"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpEffective));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getCumipmt"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpCumipmt));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getNominal"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpNominal));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getCumprinc"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpCumprinc));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getXnpv"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpXNPV));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getPricemat"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpPriceMat));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getReceived"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpReceived));
                }
                else if( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getTbilleq"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpTbilleq));
                }
                else if( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getTbillprice"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpTbillprice));
                }
                else if( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getTbillyield"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpTbillyield));
                }
                else if (!(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getFvschedule"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpFvschedule));
                }
                /*else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getYield"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpYield));
                }*/
                else if ( !(pChild->GetExternal().compareTo(OUString(
                   "com.sun.star.sheet.addin.Analysis.getYielddisc"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpYielddisc));
                }
                else    if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getYieldmat"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpYieldmat));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getAccrintm"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpAccrintm));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getCoupdaybs"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpCoupdaybs));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getDollarde"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpDollarde));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getDollarfr"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpDollarfr));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getCoupdays"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpCoupdays));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getCoupdaysnc"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpCoupdaysnc));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                   "com.sun.star.sheet.addin.Analysis.getDisc"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpDISC));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                   "com.sun.star.sheet.addin.Analysis.getIntrate"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpINTRATE));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                   "com.sun.star.sheet.addin.Analysis.getPrice"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts,
                                      ft->Children[i], new OpPrice));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getCoupnum"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpCoupnum));
                }
                /*else if ( !(pChild->GetExternal().compareTo(OUString(
                   "com.sun.star.sheet.addin.Analysis.getDuration"))))
                {
                    mvSubArguments.push_back(
                        SoPHelper(ts, ft->Children[i], new OpDuration_ADD));
                }*/
                else if ( !(pChild->GetExternal().compareTo(OUString(
                   "com.sun.star.sheet.addin.Analysis.getAmordegrc"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpAmordegrc));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                   "com.sun.star.sheet.addin.Analysis.getAmorlinc"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpAmorlinc));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getMduration"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpMDuration));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getXirr"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpXirr));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getOddlprice"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts,
                        ft->Children[i], new OpOddlprice));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getOddlyield"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpOddlyield));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getPricedisc"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts,
                        ft->Children[i], new OpPriceDisc));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getCouppcd"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpCouppcd));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getCoupncd"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpCoupncd));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getAccrint"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpAccrint));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getSqrtpi"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpSqrtPi));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getConvert"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpConvert));
                }else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getIseven"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpIsEven));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getIsodd"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpIsOdd));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getMround"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpMROUND));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getQuotient"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpQuotient));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getSeriessum"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpSeriesSum));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getBesselj"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i],
                        new OpBesselj));
                }
                else
                    throw UnhandledToken(pChild, "unhandled opcode");
                break;

            default:
                throw UnhandledToken(pChild, "unhandled opcode");
        };
    }
}

class DynamicKernel : public CompiledFormula
{
public:
    DynamicKernel(FormulaTreeNodeRef r):mpRoot(r),
        mpProgram(NULL), mpKernel(NULL), mpResClmem(NULL), mpCode(NULL) {}
    static DynamicKernel *create(ScDocument& rDoc,
                                 const ScAddress& rTopPos,
                                 ScTokenArray& rCode);
    /// OpenCL code generation
    void CodeGen() {
        // Travese the tree of expression and declare symbols used
        const DynamicKernelArgument *DK= mSyms.DeclRefArg<
            DynamicKernelSoPArguments>(mpRoot, new OpNop);

        std::stringstream decl;
        if (OpenclDevice::gpuEnv.mnKhrFp64Flag) {
            decl << "#pragma OPENCL EXTENSION cl_khr_fp64: enable\n";
        } else if (OpenclDevice::gpuEnv.mnAmdFp64Flag) {
            decl << "#pragma OPENCL EXTENSION cl_amd_fp64: enable\n";
        }
        // preambles
        decl << publicFunc;
        DK->DumpInlineFun(inlineDecl,inlineFun);
        for(std::set<std::string>::iterator set_iter=inlineDecl.begin();
                                         set_iter!=inlineDecl.end();++set_iter)
        {
            decl<<*set_iter;
        }

        for(std::set<std::string>::iterator set_iter=inlineFun.begin();
                                         set_iter!=inlineFun.end();++set_iter)
        {
            decl<<*set_iter;
        }
        mSyms.DumpSlidingWindowFunctions(decl);
        mKernelSignature = DK->DumpOpName();
        decl << "__kernel void DynamicKernel" << mKernelSignature;
        decl << "(__global double *result, ";
        DK->GenSlidingWindowDecl(decl);
        decl << ") {\n\tint gid0 = get_global_id(0);\n\tresult[gid0] = " <<
            DK->GenSlidingWindowDeclRef(false) << ";\n}\n";
        mFullProgramSrc = decl.str();
        SAL_INFO("sc.opencl.source", "Program to be compiled:\n" << mFullProgramSrc);
    }
    /// Produce kernel hash
    std::string GetMD5(void)
    {
#ifdef MD5_KERNEL
        if (mKernelHash.empty()) {
            std::stringstream md5s;
            // Compute MD5SUM of kernel body to obtain the name
            sal_uInt8 result[RTL_DIGEST_LENGTH_MD5];
            rtl_digest_MD5(
                mFullProgramSrc.c_str(),
                mFullProgramSrc.length(), result,
                RTL_DIGEST_LENGTH_MD5);
            for(int i=0; i < RTL_DIGEST_LENGTH_MD5; i++) {
                md5s << std::hex << (int)result[i];
            }
            mKernelHash = md5s.str();
        }
        return mKernelHash;
#else
        return "";
#endif
    }
    /// Create program, build, and create kerenl
    /// TODO cache results based on kernel body hash
    /// TODO: abstract OpenCL part out into OpenCL wrapper.
    void CreateKernel(void);
    /// Prepare buffers, marshal them to GPU, and launch the kernel
    /// TODO: abstract OpenCL part out into OpenCL wrapper.
    void Launch(size_t nr)
    {
        // Obtain cl context
        KernelEnv kEnv;
        OpenclDevice::setKernelEnv(&kEnv);
        cl_int err;
        // The results
        mpResClmem = clCreateBuffer(kEnv.mpkContext,
                (cl_mem_flags) CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR,
                nr*sizeof(double), NULL, &err);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);
        err = clSetKernelArg(mpKernel, 0, sizeof(cl_mem), (void*)&mpResClmem);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);
        // The rest of buffers
        mSyms.Marshal(mpKernel, nr, mpProgram);
        size_t global_work_size[] = {nr};
        err = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, mpKernel, 1, NULL,
            global_work_size, NULL, 0, NULL, NULL);
        if (CL_SUCCESS != err)
            throw OpenCLError(err, __FILE__, __LINE__);
    }
    ~DynamicKernel();
    cl_mem GetResultBuffer(void) const { return mpResClmem; }
    void SetPCode(ScTokenArray *pCode) { mpCode = pCode; }

private:
    void TraverseAST(FormulaTreeNodeRef);
    FormulaTreeNodeRef mpRoot;
    SymbolTable mSyms;
    std::string mKernelSignature, mKernelHash;
    std::string mFullProgramSrc;
    cl_program mpProgram;
    cl_kernel mpKernel;
    cl_mem mpResClmem; // Results
    std::set<std::string> inlineDecl;
    std::set<std::string> inlineFun;
    ScTokenArray *mpCode;
};

DynamicKernel::~DynamicKernel()
{
    if (mpResClmem) {
        clReleaseMemObject(mpResClmem);
    }
    if (mpKernel) {
        clReleaseKernel(mpKernel);
    }
    // mpProgram is not going to be released here -- it's cached.
    if (mpCode)
        delete mpCode;
}
/// Build code
void DynamicKernel::CreateKernel(void)
{
    cl_int err;
    std::string kname = "DynamicKernel"+mKernelSignature;
    // Compile kernel here!!!
    // Obtain cl context
    KernelEnv kEnv;
    OpenclDevice::setKernelEnv(&kEnv);
    const char *src = mFullProgramSrc.c_str();
    static std::string lastOneKernelHash = "";
    static std::string lastSecondKernelHash = "";
    static cl_program lastOneProgram = NULL;
    static cl_program lastSecondProgram = NULL;
    std::string KernelHash = mKernelSignature+GetMD5();
    if (lastOneKernelHash == KernelHash && lastOneProgram)
    {
        mpProgram = lastOneProgram;
    }
    else if(lastSecondKernelHash == KernelHash && lastSecondProgram)
    {
        mpProgram = lastSecondProgram;
    }
    else
    {   // doesn't match the last compiled formula.

        if (lastSecondProgram) {
            clReleaseProgram(lastSecondProgram);
        }
        if (OpenclDevice::buildProgramFromBinary("",
                    &OpenclDevice::gpuEnv, KernelHash.c_str(), 0)) {
            mpProgram = OpenclDevice::gpuEnv.mpArryPrograms[0];
            OpenclDevice::gpuEnv.mpArryPrograms[0] = NULL;
        } else {
            mpProgram = clCreateProgramWithSource(kEnv.mpkContext, 1,
                    &src, NULL, &err);
            if (err != CL_SUCCESS)
                throw OpenCLError(err, __FILE__, __LINE__);
            err = clBuildProgram(mpProgram, 1,
                    OpenclDevice::gpuEnv.mpArryDevsID, "", NULL, NULL);
            if (err != CL_SUCCESS)
            {
#if OSL_DEBUG_LEVEL > 0
                if (err == CL_BUILD_PROGRAM_FAILURE)
                {
                    cl_build_status stat;
                    cl_int e = clGetProgramBuildInfo(
                        mpProgram, OpenclDevice::gpuEnv.mpArryDevsID[0],
                        CL_PROGRAM_BUILD_STATUS, sizeof (cl_build_status),
                        &stat, 0);
                    SAL_WARN_IF(
                        e != CL_SUCCESS, "sc.opencl",
                        "after CL_BUILD_PROGRAM_FAILURE,"
                            " clGetProgramBuildInfo(CL_PROGRAM_BUILD_STATUS)"
                            " fails with " << e);
                    if (e == CL_SUCCESS)
                    {
                        size_t n;
                        e = clGetProgramBuildInfo(
                            mpProgram, OpenclDevice::gpuEnv.mpArryDevsID[0],
                            CL_PROGRAM_BUILD_LOG, 0, 0, &n);
                        SAL_WARN_IF(
                            e != CL_SUCCESS || n == 0, "sc.opencl",
                            "after CL_BUILD_PROGRAM_FAILURE,"
                                " clGetProgramBuildInfo(CL_PROGRAM_BUILD_LOG)"
                                " fails with " << e << ", n=" << n);
                        if (e == CL_SUCCESS && n != 0)
                        {
                            std::vector<char> log(n);
                            e = clGetProgramBuildInfo(
                                mpProgram, OpenclDevice::gpuEnv.mpArryDevsID[0],
                                CL_PROGRAM_BUILD_LOG, n, &log[0], 0);
                            SAL_WARN_IF(
                                e != CL_SUCCESS || n == 0, "sc.opencl",
                                "after CL_BUILD_PROGRAM_FAILURE,"
                                    " clGetProgramBuildInfo("
                                    "CL_PROGRAM_BUILD_LOG) fails with " << e);
                            if (e == CL_SUCCESS)
                                SAL_WARN(
                                    "sc.opencl",
                                    "CL_BUILD_PROGRAM_FAILURE, status " << stat
                                        << ", log \"" << &log[0] << "\"");
                        }
                    }
                }
#endif
                throw OpenCLError(err, __FILE__, __LINE__);
            }
            // Generate binary out of compiled kernel.
            OpenclDevice::generatBinFromKernelSource(mpProgram,
                    (mKernelSignature+GetMD5()).c_str());
        }
        lastSecondKernelHash = lastOneKernelHash;
        lastSecondProgram = lastOneProgram;
        lastOneKernelHash = KernelHash;
        lastOneProgram = mpProgram;
    }
    mpKernel = clCreateKernel(mpProgram, kname.c_str(), &err);
    if (err != CL_SUCCESS)
        throw OpenCLError(err, __FILE__, __LINE__);
}
// Symbol lookup. If there is no such symbol created, allocate one
// kernel with argument with unique name and return so.
// The template argument T must be a subclass of DynamicKernelArgument
template <typename T>
const DynamicKernelArgument *SymbolTable::DeclRefArg(
                  FormulaTreeNodeRef t, SlidingFunctionBase* pCodeGen)
{
    FormulaToken *ref = t->GetFormulaToken();
    ArgumentMap::iterator it = mSymbols.find(ref);
    if (it == mSymbols.end()) {
        // Allocate new symbols
        std::stringstream ss;
        ss << "tmp"<< mCurId++;
        boost::shared_ptr<DynamicKernelArgument> new_arg(new T(ss.str(), t, pCodeGen));
        mSymbols[ref] = new_arg;
        mParams.push_back(new_arg);
        return new_arg.get();
    } else {
        return it->second.get();
    }
}

class FormulaGroupInterpreterOpenCL : public FormulaGroupInterpreter
{
public:
    FormulaGroupInterpreterOpenCL() :
        FormulaGroupInterpreter()
    {
    }
    virtual ~FormulaGroupInterpreterOpenCL()
    {
    }

    virtual ScMatrixRef inverseMatrix( const ScMatrix& rMat ) SAL_OVERRIDE;
    virtual CompiledFormula* createCompiledFormula(ScDocument& rDoc,
                                                   const ScAddress& rTopPos,
                                                   ScFormulaCellGroupRef& xGroup,
                                                   ScTokenArray& rCode) SAL_OVERRIDE;
    virtual bool interpret( ScDocument& rDoc, const ScAddress& rTopPos,
                            ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode ) SAL_OVERRIDE;
};

ScMatrixRef FormulaGroupInterpreterOpenCL::inverseMatrix( const ScMatrix& )
{
    return NULL;
}

DynamicKernel* DynamicKernel::create(ScDocument& /* rDoc */,
                                     const ScAddress& /* rTopPos */,
                                     ScTokenArray& rCode)
{
    // Constructing "AST"
    FormulaTokenIterator aCode = rCode;
    std::list<FormulaToken *> list;
    std::map<FormulaToken *, FormulaTreeNodeRef> m_hash_map;
    FormulaToken*  pCur;
    while( (pCur = (FormulaToken*)(aCode.Next()) ) != NULL)
    {
        OpCode eOp = pCur->GetOpCode();
        if ( eOp != ocPush )
        {
            FormulaTreeNodeRef m_currNode =
                 FormulaTreeNodeRef(new FormulaTreeNode(pCur));
            sal_uInt8 m_ParamCount =  pCur->GetParamCount();
            for(int i=0; i<m_ParamCount; i++)
            {
                FormulaToken* m_TempFormula = list.back();
                list.pop_back();
                if(m_TempFormula->GetOpCode()!=ocPush)
                {
                    if(m_hash_map.find(m_TempFormula)==m_hash_map.end())
                        return NULL;
                    m_currNode->Children.push_back(m_hash_map[m_TempFormula]);
                }
                else
                {
                    FormulaTreeNodeRef m_ChildTreeNode =
                      FormulaTreeNodeRef(
                               new FormulaTreeNode(m_TempFormula));
                    m_currNode->Children.push_back(m_ChildTreeNode);
                }
            }
            std::reverse(m_currNode->Children.begin(),
                                m_currNode->Children.end());
            m_hash_map[pCur] = m_currNode;
        }
        list.push_back(pCur);
    }

    FormulaTreeNodeRef Root = FormulaTreeNodeRef(new FormulaTreeNode(NULL));
    Root->Children.push_back(m_hash_map[list.back()]);

    DynamicKernel* pDynamicKernel = new DynamicKernel(Root);

    if (!pDynamicKernel)
        return NULL;

    // OpenCL source code generation and kernel compilation
    try {
        pDynamicKernel->CodeGen();
        pDynamicKernel->CreateKernel();
    }
    catch (const UnhandledToken &ut) {
        std::cerr << "\nDynamic formual compiler: unhandled token: ";
        std::cerr << ut.mMessage << " at ";
        std::cerr << ut.mFile << ":" << ut.mLineNumber << "\n";
#ifdef NO_FALLBACK_TO_SWINTERP
        assert(false);
#else
        free(pDynamicKernel);
        return NULL;
#endif
    }
    catch (...) {
        std::cerr << "Dynamic formula compiler: unhandled compiler error\n";
        return NULL;
    }
    return pDynamicKernel;
}

CompiledFormula* FormulaGroupInterpreterOpenCL::createCompiledFormula(ScDocument& rDoc,
                                                                      const ScAddress& rTopPos,
                                                                      ScFormulaCellGroupRef& xGroup,
                                                                      ScTokenArray& rCode)
{
    ScTokenArray *pCode = new ScTokenArray();
    ScGroupTokenConverter aConverter(*pCode, rDoc, *xGroup->mpTopCell, rTopPos);
    if (!aConverter.convert(rCode) || pCode->GetLen() == 0)
    {
        delete pCode;
        return NULL;
    }
    SymbolTable::nR = xGroup->mnLength;

    DynamicKernel *result = DynamicKernel::create(rDoc, rTopPos, *pCode);
    if ( result )
        result->SetPCode(pCode);
    return result;
}

bool FormulaGroupInterpreterOpenCL::interpret( ScDocument& rDoc,
    const ScAddress& rTopPos, ScFormulaCellGroupRef& xGroup,
    ScTokenArray& rCode )
{
    DynamicKernel *pKernel;

    if (xGroup->meKernelState == sc::OpenCLKernelCompilationScheduled ||
        xGroup->meKernelState == sc::OpenCLKernelBinaryCreated)
    {
        if (xGroup->meKernelState == sc::OpenCLKernelCompilationScheduled)
        {
            ScFormulaCellGroup::sxCompilationThread->maCompilationDoneCondition.wait();
            ScFormulaCellGroup::sxCompilationThread->maCompilationDoneCondition.reset();
        }

        pKernel = static_cast<DynamicKernel*>(xGroup->mpCompiledFormula);
    }
    else
    {
        assert(xGroup->meCalcState == sc::GroupCalcRunning);
        pKernel = static_cast<DynamicKernel*>(createCompiledFormula(rDoc, rTopPos, xGroup, rCode));
    }

    if (!pKernel)
        return false;

    try {
        // Obtain cl context
        KernelEnv kEnv;
        OpenclDevice::setKernelEnv(&kEnv);
        // Run the kernel.
        pKernel->Launch(xGroup->mnLength);
        // Map results back
        cl_mem res = pKernel->GetResultBuffer();
        cl_int err;
        double *resbuf = (double*)clEnqueueMapBuffer(kEnv.mpkCmdQueue,
                res,
                CL_TRUE, CL_MAP_READ, 0,
                xGroup->mnLength*sizeof(double), 0, NULL, NULL,
                &err);
        if (err != CL_SUCCESS)
            throw OpenCLError(err, __FILE__, __LINE__);
        rDoc.SetFormulaResults(rTopPos, resbuf, xGroup->mnLength);
        err = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue, res, resbuf, 0, NULL, NULL);
        if (err != CL_SUCCESS)
            throw OpenCLError(err, __FILE__, __LINE__);
        if (xGroup->meCalcState == sc::GroupCalcRunning)
            delete pKernel;
    }
    catch (const UnhandledToken &ut) {
        std::cerr << "\nDynamic formual compiler: unhandled token: ";
        std::cerr << ut.mMessage << "\n";
#ifdef NO_FALLBACK_TO_SWINTERP
        assert(false);
        return true;
#else
        return false;
#endif
    }
    catch (const OpenCLError &oce) {
        std::cerr << "Dynamic formula compiler: OpenCL error: ";
        std::cerr << oce.mError;
        std::cerr <<" at ";
        std::cerr << oce.mFile << ":" << oce.mLineNumber << "\n";
#ifdef NO_FALLBACK_TO_SWINTERP
        assert(false);
        return true;
#else
        return false;
#endif
    }
    catch (const Unhandled &uh) {
        std::cerr << "Dynamic formula compiler: unhandled case:";
        std::cerr <<" at ";
        std::cerr << uh.mFile << ":" << uh.mLineNumber << "\n";
#ifdef NO_FALLBACK_TO_SWINTERP
        assert(false);
        return true;
#else
        return false;
#endif
    }
    catch (...) {
        std::cerr << "Dynamic formula compiler: unhandled compiler error\n";
#ifdef NO_FALLBACK_TO_SWINTERP
        assert(false);
        return true;
#else
        return false;
#endif
    }
    return true;
}

}} // namespace sc::opencl

extern "C" {

SAL_DLLPUBLIC_EXPORT sc::FormulaGroupInterpreter* SAL_CALL
                   createFormulaGroupOpenCLInterpreter()
{
    return new sc::opencl::FormulaGroupInterpreterOpenCL();
}

SAL_DLLPUBLIC_EXPORT size_t getOpenCLPlatformCount()
{
    return sc::opencl::getOpenCLPlatformCount();
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL fillOpenCLInfo(
               sc::OpenclPlatformInfo* pInfos, size_t nInfoSize)
{
    const std::vector<sc::OpenclPlatformInfo>& rPlatforms =
                 sc::opencl::fillOpenCLInfo();
    size_t n = std::min(rPlatforms.size(), nInfoSize);
    for (size_t i = 0; i < n; ++i)
        pInfos[i] = rPlatforms[i];
}

SAL_DLLPUBLIC_EXPORT bool SAL_CALL switchOpenClDevice(
                       const OUString* pDeviceId, bool bAutoSelect,
                       bool bForceEvaluation)
{
    return sc::opencl::switchOpenclDevice(pDeviceId, bAutoSelect, bForceEvaluation);
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL getOpenCLDeviceInfo(size_t* pDeviceId, size_t* pPlatformId)
{
    sc::opencl::getOpenCLDeviceInfo(*pDeviceId, *pPlatformId);
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
