/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "formulagroup.hxx"
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
#include "op_statistical.hxx"
#include "formulagroupcl_public.hxx"

#include <list>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#define MD5_KERNEL 1
#ifdef MD5_KERNEL
#include <rtl/digest.h>
#endif
#include <memory>

#include <boost/scoped_ptr.hpp>

using namespace formula;

namespace sc { namespace opencl {


/// Map the buffer used by an argument and do necessary argument setting
size_t DynamicKernelArgument::Marshal(cl_kernel k, int argno, int)
{
    FormulaToken *ref = mFormulaTree->GetFormulaToken();
    assert(mpClmem == NULL);
    double *pHostBuffer = NULL;
    size_t szHostBuffer = 0;
    if (ref->GetType() == formula::svSingleVectorRef) {
        const formula::SingleVectorRefToken* pSVR =
            dynamic_cast< const formula::SingleVectorRefToken* >(ref);
        assert(pSVR);
        pHostBuffer = const_cast<double*>(pSVR->GetArray().mpNumericArray);
        szHostBuffer = pSVR->GetArrayLength() * sizeof(double);
#if 0
        std::cerr << "Marshal a Single vector of size " << pSVR->GetArrayLength();
        std::cerr << " at argument "<< argno << "\n";
#endif
    } else if (ref->GetType() == formula::svDoubleVectorRef) {
        const formula::DoubleVectorRefToken* pDVR =
            dynamic_cast< const formula::DoubleVectorRefToken* >(ref);
        assert(pDVR);
        if (pDVR->GetArrays()[0].mpNumericArray == NULL)
            throw Unhandled();
        pHostBuffer = const_cast<double*>(pDVR->GetArrays()[0].mpNumericArray);
        szHostBuffer = pDVR->GetArrayLength() * sizeof(double);
    } else {
        throw Unhandled();
    }
    // Obtain cl context
    KernelEnv kEnv;
    OpenclDevice::setKernelEnv(&kEnv);
    cl_int err;
    mpClmem = clCreateBuffer(kEnv.mpkContext,
        (cl_mem_flags) CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,
        szHostBuffer,
        pHostBuffer, &err);
    if (CL_SUCCESS != err)
        throw OpenCLError(err);

    err = clSetKernelArg(k, argno, sizeof(cl_mem), (void*)&mpClmem);
    if (CL_SUCCESS != err)
        throw OpenCLError(err);
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
    virtual size_t Marshal(cl_kernel k, int argno, int)
    {
        FormulaToken *ref = mFormulaTree->GetFormulaToken();
        assert(mpClmem == NULL);
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
            throw OpenCLError(err);
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
        FormulaToken *Tok = GetFormulaToken();
        if (Tok->GetType() != formula::svDouble)
            throw Unhandled();
        ss << Tok->GetDouble();
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
    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal(cl_kernel k, int argno, int)
    {
        double tmp = 0.0;
        // Pass the scalar result back to the rest of the formula kernel
        cl_int err = clSetKernelArg(k, argno, sizeof(double), (void*)&tmp);
        if (CL_SUCCESS != err)
            throw OpenCLError(err);
        return 1;
    }
};

class DynamicKernelStringArgument: public DynamicKernelArgument
{
public:
    DynamicKernelStringArgument(const std::string &s,
        FormulaTreeNodeRef ft):
        DynamicKernelArgument(s, ft) {}

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
    virtual size_t Marshal(cl_kernel, int, int);
};

/// Marshal a string vector reference
size_t DynamicKernelStringArgument::Marshal(cl_kernel k, int argno, int)
{
    FormulaToken *ref = mFormulaTree->GetFormulaToken();
    assert(mpClmem == NULL);
    // Obtain cl context
    KernelEnv kEnv;
    OpenclDevice::setKernelEnv(&kEnv);
    cl_int err;
    formula::VectorRefArray vRef;
    size_t nStrings = 0;
    if (ref->GetType() == formula::svSingleVectorRef) {
        const formula::SingleVectorRefToken* pSVR =
            dynamic_cast< const formula::SingleVectorRefToken* >(ref);
        assert(pSVR);
        nStrings = pSVR->GetArrayLength();
        vRef = pSVR->GetArray();
    } else if (ref->GetType() == formula::svDoubleVectorRef) {
        const formula::DoubleVectorRefToken* pDVR =
            dynamic_cast< const formula::DoubleVectorRefToken* >(ref);
        assert(pDVR);
        nStrings = pDVR->GetArrayLength();
        vRef = pDVR->GetArrays()[0];
    }
    size_t szHostBuffer = nStrings * sizeof(cl_int);
    // Marshal strings. Right now we pass hashes of these string
    mpClmem = clCreateBuffer(kEnv.mpkContext,
            (cl_mem_flags) CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR,
            szHostBuffer, NULL, &err);
    if (CL_SUCCESS != err)
        throw OpenCLError(err);
    cl_uint *pHashBuffer = (cl_uint*)clEnqueueMapBuffer(
            kEnv.mpkCmdQueue, mpClmem, CL_TRUE, CL_MAP_WRITE, 0,
            szHostBuffer, 0, NULL, NULL, &err);
    if (CL_SUCCESS != err)
        throw OpenCLError(err);
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
        throw OpenCLError(err);

    err = clSetKernelArg(k, argno, sizeof(cl_mem), (void*)&mpClmem);
    if (CL_SUCCESS != err)
        throw OpenCLError(err);
    return 1;
}

/// A mixed string/numberic vector
class DynamicKernelMixedArgument: public DynamicKernelArgument
{
public:
    DynamicKernelMixedArgument(const std::string &s,
        FormulaTreeNodeRef ft):
        DynamicKernelArgument(s, ft), mStringArgument(s+"s", ft) {}
    virtual void GenSlidingWindowDecl(std::stringstream& ss) const
    {
        DynamicKernelArgument::GenSlidingWindowDecl(ss);
        ss << ", ";
        mStringArgument.GenSlidingWindowDecl(ss);
    }
    virtual void GenSlidingWindowFunction(std::stringstream &) {}
    /// Generate declaration
    virtual void GenDecl(std::stringstream &ss) const
    {
        DynamicKernelArgument::GenDecl(ss);
        ss << ", ";
        mStringArgument.GenDecl(ss);
    }
    virtual void GenDeclRef(std::stringstream &ss) const
    {
        DynamicKernelArgument::GenDeclRef(ss);
        ss << ",";
        mStringArgument.GenDeclRef(ss);
    }
    virtual std::string GenSlidingWindowDeclRef(bool) const
    {
        std::stringstream ss;
        ss << "(!isNan(" << DynamicKernelArgument::GenSlidingWindowDeclRef();
        ss << ")?" << DynamicKernelArgument::GenSlidingWindowDeclRef();
        ss << ":" << mStringArgument.GenSlidingWindowDeclRef();
        ss << ")";
        return ss.str();
    }
    virtual size_t Marshal(cl_kernel k, int argno, int vw)
    {
        int i = DynamicKernelArgument::Marshal(k, argno, vw);
        i += mStringArgument.Marshal(k, argno+i, vw);
        return i;
    }
protected:
    DynamicKernelStringArgument mStringArgument;
};

/// Handling a Double Vector that is used as a sliding window input
/// to either a sliding window average or sum-of-products
template<class Base>
class DynamicKernelSlidingArgument: public Base
{
public:
    DynamicKernelSlidingArgument(const std::string &s,
        FormulaTreeNodeRef ft):
        Base(s, ft)
    {
        FormulaToken *t = ft->GetFormulaToken();
        if (t->GetType() != formula::svDoubleVectorRef)
            throw Unhandled();
        const formula::DoubleVectorRefToken* pDVR =
            dynamic_cast<const formula::DoubleVectorRefToken *>(t);
        assert(pDVR);
        bIsStartFixed = pDVR->IsStartFixed();
        bIsEndFixed = pDVR->IsEndFixed();
    }
    virtual void GenSlidingWindowFunction(std::stringstream &) {}

    virtual std::string GenSlidingWindowDeclRef(bool=false) const
    {
        std::stringstream ss;
        if (!bIsStartFixed && !bIsEndFixed)
            ss << Base::GetName() << "[i + gid0]";
        else
            ss << Base::GetName() << "[i]";
        return ss.str();
    }
protected:
    bool bIsStartFixed, bIsEndFixed;
};

/// Abstract class for code generation

class Reduction: public SlidingFunctionBase
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
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
        ss << "int nCount = 0;\n\t";
        ss << "double tmpBottom;\n\t";
        unsigned i = vSubArguments.size();
        size_t nItems = 0;
        while (i--)
        {
            FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
            assert(pCur);
            if (pCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed()) {
#ifdef  ISNAN
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; i++){\n\t\t";
#else
                    ss << "gid0; i < "<< nCurWindowSize << "; i++)\n\t\t";
#endif
                } else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed()) {
#ifdef  ISNAN
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+"<< nCurWindowSize << "; i++){\n\t\t";
#else
                    ss << "0; i < gid0+"<< nCurWindowSize << "; i++)\n\t\t";
#endif
                } else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed()){
#ifdef  ISNAN
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < "<< nCurWindowSize << "; i++){\n\t\t";
#else
                    ss << "0; i < "<< nCurWindowSize << "; i++)\n\t\t";
#endif
                }
                else {
#ifdef  ISNAN
                    ss << "0; i < "<< nCurWindowSize << "; i++){\n\t\t";
#else
                    ss << "0; i < "<< nCurWindowSize << "; i++)\n\t\t";
#endif
                }
                nItems += nCurWindowSize;
            }
            else if (pCur->GetType() == formula::svSingleVectorRef)
            {
#ifdef  ISNAN
                    const formula::SingleVectorRefToken* pSVR =
                    dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
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
#ifdef  ISNAN
                ss << "nCount += 1;\n\t";
#endif
                nItems += 1;
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
                ss << "nCount += 1;\n\t\t";
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
            const std::string sSymName, SubArguments &vSubArguments)
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
            const std::string sSymName, SubArguments &vSubArguments)
    {
        size_t nCurWindowSize = 0;
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
            size_t nCurChildWindowSize = vSubArguments[i]->GetWindowSize();
            nCurWindowSize = (nCurWindowSize < nCurChildWindowSize) ?
                nCurChildWindowSize:nCurWindowSize;
        }
        ss << ") {\n\t";
        ss << "double tmp = 0.0;\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "for (int i = 0; i <" << nCurWindowSize << "; i++){\n\t\t";
        ss << "int currentCount = i+gid0+1;\n";
        ss << "tmp += ";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << "*";
#ifdef  ISNAN
            if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
            {
                ss <<"(";
                ss <<"(currentCount>";
                if(vSubArguments[i]->GetFormulaToken()->GetType() ==
                     formula::svSingleVectorRef)
                {
                    const formula::SingleVectorRefToken* pSVR =
                    dynamic_cast< const formula::SingleVectorRefToken*>
                         (vSubArguments[i]->GetFormulaToken());
                    ss<<pSVR->GetArrayLength();
                }
                else if(vSubArguments[i]->GetFormulaToken()->GetType() ==
                          formula::svDoubleVectorRef)
                {
                    const formula::DoubleVectorRefToken* pSVR =
                    dynamic_cast< const formula::DoubleVectorRefToken*>
                          (vSubArguments[i]->GetFormulaToken());
                    ss<<pSVR->GetArrayLength();
                }
                ss << ")&&isNan("<<vSubArguments[i]
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
        ss << ";\n\t}\n\t";
        ss << "return tmp;\n";
        ss << "}";
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
#ifdef  ISNAN
        ss << "(0 =="<< lhs << ")? tmp : (" << rhs<<"+1.0)";
#else
        ss << "(isNan(" << lhs << ")?"<<rhs<<":"<<rhs<<"+1.0)";
#endif
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
        ss << "fsum(" << lhs <<","<< rhs<<")";
        return ss.str();
    }
    virtual std::string BinFuncName(void) const { return "fsum"; }
};

class OpAverage: public OpSum {
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
        return "fmin("+lhs + "," + rhs +")";
    }
    virtual std::string BinFuncName(void) const { return "fmin"; }
};

class OpMax: public Reduction {
public:
    virtual std::string GetBottom(void) { return "-MAXFLOAT"; }
    virtual std::string Gen2(const std::string &lhs, const std::string &rhs) const
    {
        return "fmax("+lhs + "," + rhs +")";
    }
    virtual std::string BinFuncName(void) const { return "fmax"; }
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

/// Helper functions that have multiple buffers
class DynamicKernelSoPArguments: public DynamicKernelArgument
{
public:
    typedef boost::shared_ptr<DynamicKernelArgument> SubArgument;
    typedef std::vector<SubArgument> SubArgumentsType;

    DynamicKernelSoPArguments(
        const std::string &s, const FormulaTreeNodeRef& ft, SlidingFunctionBase* pCodeGen);

    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal(cl_kernel k, int argno, int nVectorWidth)
    {
        unsigned i = 0;
        for (SubArgumentsType::iterator it = mvSubArguments.begin(), e= mvSubArguments.end(); it!=e;
                ++it)
        {
            i += (*it)->Marshal(k, argno + i, nVectorWidth);
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
private:
    SubArgumentsType mvSubArguments;
    boost::scoped_ptr<SlidingFunctionBase> mpCodeGen;
};

boost::shared_ptr<DynamicKernelArgument> SoPHelper(
    const std::string &ts, const FormulaTreeNodeRef& ft, SlidingFunctionBase* pCodeGen)
{
    return boost::shared_ptr<DynamicKernelArgument>(new DynamicKernelSoPArguments(ts, ft, pCodeGen));
}

DynamicKernelSoPArguments::DynamicKernelSoPArguments(
    const std::string &s, const FormulaTreeNodeRef& ft, SlidingFunctionBase* pCodeGen) :
    DynamicKernelArgument(s, ft), mpCodeGen(pCodeGen)
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
                        dynamic_cast< const formula::DoubleVectorRefToken* >(pChild);
                    assert(pDVR);
                    if (pDVR->GetArrays()[0].mpNumericArray)
                        mvSubArguments.push_back(
                                SubArgument(new DynamicKernelSlidingArgument
                                    <DynamicKernelArgument>(ts, ft->Children[i])));
                    else
                        mvSubArguments.push_back(
                                SubArgument(new DynamicKernelSlidingArgument
                                    <DynamicKernelStringArgument>(
                                        ts, ft->Children[i])));
                } else if (pChild->GetType() == formula::svSingleVectorRef) {
                    const formula::SingleVectorRefToken* pSVR =
                        dynamic_cast< const formula::SingleVectorRefToken* >(pChild);
                    assert(pSVR);
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
                                SubArgument(new DynamicKernelArgument(ts,
                                        ft->Children[i])));
                    }
                    else if (pSVR->GetArray().mpStringArray &&
                            pCodeGen->takeString())
                    {
                        mvSubArguments.push_back(
                                SubArgument(new DynamicKernelStringArgument(
                                        ts, ft->Children[i])));
                    }
                    else
                        throw UnhandledToken(pChild,
                                "Got unhandled case here");
                } else if (pChild->GetType() == formula::svDouble) {
                    mvSubArguments.push_back(
                            SubArgument(new DynamicKernelConstantArgument(ts,
                                    ft->Children[i])));
                } else if (pChild->GetType() == formula::svString) {
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
            case ocGeoMean:
                mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpGeoMean));
                break;
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
            case ocMedian:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpMedian));
                break;
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
            case ocVBD:
                mvSubArguments.push_back(SoPHelper(ts,
                         ft->Children[i],new OpVDB));
                 break;
            case ocKurt:
                mvSubArguments.push_back(SoPHelper(ts,
                        ft->Children[i], new OpKurt));
                 break;
            case ocZZR:
                mvSubArguments.push_back(SoPHelper(ts,
                        ft->Children[i], new OpNPER));
                 break;
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
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getYield"))))
                {
                    mvSubArguments.push_back(SoPHelper(ts, ft->Children[i], new OpYield));
                }
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
                else if ( !(pChild->GetExternal().compareTo(OUString(
                   "com.sun.star.sheet.addin.Analysis.getDuration"))))
                {
                    mvSubArguments.push_back(
                        SoPHelper(ts, ft->Children[i], new OpDuration_ADD));
                }
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
                break;
            default:
                throw UnhandledToken(pChild, "unhandled opcode");
        };
    }
}
/// Holds the symbol table for a given dynamic kernel
class SymbolTable {
public:
    typedef std::map<const FormulaToken *,
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
    void Marshal(cl_kernel, int);
private:
    unsigned int mCurId;
    ArgumentMap mSymbols;
    ArgumentList mParams;
};

void SymbolTable::Marshal(cl_kernel k, int nVectorWidth)
{
    int i = 1; //The first argument is reserved for results
    for(ArgumentList::iterator it = mParams.begin(), e= mParams.end(); it!=e;
            ++it) {
        i+=(*it)->Marshal(k, i, nVectorWidth);
    }
}

/// Code generation
class DynamicKernel {
public:
    DynamicKernel(FormulaTreeNodeRef r):mpRoot(r),
        mpProgram(NULL), mpKernel(NULL), mpResClmem(NULL) {}
    /// Code generation in OpenCL
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
                                         set_iter!=inlineDecl.end();set_iter++)
        {
            decl<<*set_iter;
        }

        for(std::set<std::string>::iterator set_iter=inlineFun.begin();
                                         set_iter!=inlineFun.end();set_iter++)
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
#if 1
        std::cerr<< "Program to be compiled = \n" << mFullProgramSrc << "\n";
#endif
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
            throw OpenCLError(err);
        err = clSetKernelArg(mpKernel, 0, sizeof(cl_mem), (void*)&mpResClmem);
        if (CL_SUCCESS != err)
            throw OpenCLError(err);
        // The rest of buffers
        mSyms.Marshal(mpKernel, nr);
        size_t global_work_size[] = {nr};
        err = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, mpKernel, 1, NULL,
            global_work_size, NULL, 0, NULL, NULL);
        if (CL_SUCCESS != err)
            throw OpenCLError(err);
    }
    ~DynamicKernel();
    cl_mem GetResultBuffer(void) const { return mpResClmem; }
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
};

DynamicKernel::~DynamicKernel()
{
    if (mpResClmem) {
        std::cerr<<"Freeing kernel "<< GetMD5() << " result buffer\n";
        clReleaseMemObject(mpResClmem);
    }
    if (mpKernel) {
        std::cerr<<"Freeing kernel "<< GetMD5() << " kernel\n";
        clReleaseKernel(mpKernel);
    }
    if (mpProgram) {
        std::cerr<<"Freeing kernel "<< GetMD5() << " program\n";
        clReleaseProgram(mpProgram);
    }
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
    if (OpenclDevice::buildProgramFromBinary("",
        &OpenclDevice::gpuEnv,
        (mKernelSignature+GetMD5()).c_str(), 0)) {
        mpProgram = OpenclDevice::gpuEnv.mpArryPrograms[0];
        OpenclDevice::gpuEnv.mpArryPrograms[0] = NULL;
    } else {
        mpProgram = clCreateProgramWithSource(kEnv.mpkContext, 1,
                &src, NULL, &err);
        if (err != CL_SUCCESS)
            throw OpenCLError(err);
        err = clBuildProgram(mpProgram, 1,
                OpenclDevice::gpuEnv.mpArryDevsID, "", NULL, NULL);
        if (err != CL_SUCCESS)
            throw OpenCLError(err);
        // Generate binary out of compiled kernel.
        OpenclDevice::generatBinFromKernelSource(mpProgram,
                (mKernelSignature+GetMD5()).c_str());
    }
    mpKernel = clCreateKernel(mpProgram, kname.c_str(), &err);
    if (err != CL_SUCCESS)
        throw OpenCLError(err);
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
        std::cerr << "DeclRefArg: Allocate a new symbol:";
        std::stringstream ss;
        ss << "tmp"<< mCurId++;
        boost::shared_ptr<DynamicKernelArgument> new_arg(new T(ss.str(), t, pCodeGen));
        mSymbols[ref] = new_arg;
        mParams.push_back(new_arg);
        std::cerr << ss.str() <<"\n";
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

    virtual ScMatrixRef inverseMatrix( const ScMatrix& rMat );
    virtual bool interpret( ScDocument& rDoc, const ScAddress& rTopPos,
                const ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode );
    DynamicKernel *mpKernel;
};

ScMatrixRef FormulaGroupInterpreterOpenCL::inverseMatrix( const ScMatrix& )
{
    return NULL;
}

bool FormulaGroupInterpreterOpenCL::interpret( ScDocument& rDoc,
    const ScAddress& rTopPos, const ScFormulaCellGroupRef& xGroup,
    ScTokenArray& rCode )
{
    // printf("Vector width = %d\n", xGroup->mnLength);
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
                        return false;
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
    // Code generation
    mpKernel = new DynamicKernel(Root);

    try {
        mpKernel->CodeGen();
        // Obtain cl context
        KernelEnv kEnv;
        OpenclDevice::setKernelEnv(&kEnv);
        // Compile kernel here!!!
        mpKernel->CreateKernel();
        // Run the kernel.
        mpKernel->Launch(xGroup->mnLength);
        // Map results back
        cl_mem res = mpKernel->GetResultBuffer();
        cl_int err;
        double *resbuf = (double*)clEnqueueMapBuffer(kEnv.mpkCmdQueue,
                res,
                CL_TRUE, CL_MAP_READ, 0,
                xGroup->mnLength*sizeof(double), 0, NULL, NULL,
                &err);
        if (err != CL_SUCCESS)
            throw OpenCLError(err);
        rDoc.SetFormulaResults(rTopPos, resbuf, xGroup->mnLength);
        err = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue, res, resbuf, 0, NULL, NULL);
        if (err != CL_SUCCESS)
            throw OpenCLError(err);
        delete mpKernel;
        return true;
    }
#undef NO_FALLBACK_TO_SWINTERP /* undef this for non-TDD runs */
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
        std::cerr << oce.mError << "\n";
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
} // namespace opencl

}} // namespace sc

extern "C" {

SAL_DLLPUBLIC_EXPORT sc::FormulaGroupInterpreter* SAL_CALL
                   createFormulaGroupOpenCLInterpreter()
{
#if 0// USE_GROUNDWATER_INTERPRETER
    if (getenv("SC_GROUNDWATER"))
        return new sc::opencl::FormulaGroupInterpreterGroundwater();
#endif

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
                       const OUString* pDeviceId, bool bAutoSelect)
{
    return sc::opencl::switchOpenclDevice(pDeviceId, bAutoSelect);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
