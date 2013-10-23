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

#include "OP_finacial.cxx"
#include "OP_database.cxx"
#include "OP_math.cxx"
#include "OP_Statistical.cxx"
#include "formulagroupcl_public.hxx"
#include "formulagroupcl_finacial.hxx"

#include<list>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#ifdef MD5_KERNEL
#include <openssl/md5.h>
#endif
#include <memory>
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
        pHostBuffer = const_cast<double*>(pDVR->GetArrays()[0].mpNumericArray);
        szHostBuffer = pDVR->GetArrayLength() * sizeof(double);
    }
    // Obtain cl context
    KernelEnv kEnv;
    OclCalc::setKernelEnv(&kEnv);
    cl_int err;
    mpClmem = clCreateBuffer(kEnv.mpkContext,
        (cl_mem_flags) CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR,
        szHostBuffer,
        pHostBuffer, &err);
    assert(CL_SUCCESS == err);

    err = clSetKernelArg(k, argno, sizeof(cl_mem), (void*)&mpClmem);
    assert(CL_SUCCESS == err);
    return 1;
}

/// Arguments that are actually compile-time constants
class DynamicKernelConstantArgument: public DynamicKernelArgument
{
public:
    DynamicKernelConstantArgument(const std::string &s,
        std::shared_ptr<FormulaTreeNode> ft):
            DynamicKernelArgument(s, ft) {}
    /// Generate declaration
    virtual void GenDecl(std::stringstream &ss) const
    {
        ss << "double " << mSymName;
    }
    virtual void GenDeclRef(std::stringstream &ss) const
    {
        FormulaToken *Tok = GetFormulaToken();
        assert (Tok->GetType() == formula::svDouble);
        ss << Tok->GetDouble();
    }
    virtual void GenSlidingWindowDecl(std::stringstream &ss) const
    {
        GenDecl(ss);
    }
    virtual std::string GenSlidingWindowDeclRef(void) const
    {
        assert(GetFormulaToken()->GetType() == formula::svDouble);
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
        assert(CL_SUCCESS == err);
        return 1;
    }
};

/// Handling a Double Vector that is used as a sliding window input
/// to either a sliding window average or sum-of-products
class DynamicKernelSlidingArgument: public DynamicKernelArgument
{
public:
    DynamicKernelSlidingArgument(const std::string &s,
        std::shared_ptr<FormulaTreeNode> ft):
        DynamicKernelArgument(s, ft)
    {
        FormulaToken *t = ft->GetFormulaToken();
        assert(t->GetType() == formula::svDoubleVectorRef);
        const formula::DoubleVectorRefToken* pDVR =
            dynamic_cast<const formula::DoubleVectorRefToken *>(t);
        assert(pDVR);
        bIsStartFixed = pDVR->IsStartFixed();
        bIsEndFixed = pDVR->IsEndFixed();
    }
    virtual void GenSlidingWindowFunction(std::stringstream &) {}

    virtual std::string GenSlidingWindowDeclRef(void) const
    {
        std::stringstream ss;
        if (!bIsStartFixed && !bIsEndFixed)
            ss << mSymName << "[i + gid0]";
        else
            ss << mSymName << "[i]";
        return ss.str();
    }
private:
    bool bIsStartFixed, bIsEndFixed;
};

/// Abstract class for code generation

class Reduction: public SlidingFunctionBase, public OpBase
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
        unsigned i = vSubArguments.size();
        while (i--)
        {
            FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
            assert(pCur);
            if (const formula::DoubleVectorRefToken* pCurDVR =
                dynamic_cast<const formula::DoubleVectorRefToken *>(pCur))
            {
                size_t nCurWindowSize = pCurDVR->GetRefRowSize();
                ss << "for (int i = ";
                if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed()) {
                    ss << "gid0; i < "<< nCurWindowSize <<"; i++)\n\t\t";
                } else if (pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed()) {
                    ss << "0; i < gid0+"<< nCurWindowSize <<"; i++)\n\t\t";
                } else {
                    ss << "0; i < "<< nCurWindowSize <<"; i++)\n\t\t";
                }
            }
            ss << "tmp = ";
            // Generate the operation in binary form
            ss << Gen2(vSubArguments[i]->GenSlidingWindowDeclRef(), "tmp");
            ss << ";\n\t";
        }
        ss << "return tmp;\n";
        ss << "}";
    }
};

class SumOfProduct: public SlidingFunctionBase, public OpBase
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
    {
        FormulaToken *pCur = vSubArguments[0]->GetFormulaToken();
        assert(pCur);
        const formula::DoubleVectorRefToken* pCurDVR =
            dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
        size_t nCurWindowSize = pCurDVR->GetRefRowSize();
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss << "double tmp = 0.0;\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "for (int i = 0; i <" << nCurWindowSize << "; i++)\n\t\t";
        ss << "tmp += ";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << "*";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
        }
        ss << ";\n\t";
        ss << "return tmp;\n";
        ss << "}";
    }
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
    virtual std::string BinFuncName(void) const { return "fsum"; }
};
/// Helper functions that have multiple buffers
template<class Op>
class DynamicKernelSoPArguments: public DynamicKernelArgument
{
public:
    typedef std::unique_ptr<DynamicKernelArgument> SubArgument;

    DynamicKernelSoPArguments(const std::string &s,
        std::shared_ptr<FormulaTreeNode> ft);

    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal(cl_kernel k, int argno, int nVectorWidth)
    {
        unsigned i = 0;
        for(auto it = mvSubArguments.begin(), e= mvSubArguments.end(); it!=e;
                ++it) {
            i += (*it)->Marshal(k, argno + i, nVectorWidth);
        }
        return i;
    }

    virtual void GenSlidingWindowFunction(std::stringstream &ss) {
        for (unsigned i = 0; i < mvSubArguments.size(); i++)
            mvSubArguments[i]->GenSlidingWindowFunction(ss);
        CodeGen.GenSlidingWindowFunction(ss, mSymName, mvSubArguments);
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
        for(auto it = mvSubArguments.begin(), e= mvSubArguments.end(); it!=e;
            ++it) {
            if (it != mvSubArguments.begin())
                ss << ", ";
            (*it)->GenDecl(ss);
        }
    }

    virtual size_t GetWindowSize(void) const
    {
        return 1;
    }

    /// When declared as input to a sliding window function
    virtual void GenSlidingWindowDecl(std::stringstream &ss) const
    {
        for(auto it = mvSubArguments.begin(), e= mvSubArguments.end(); it!=e;
            ++it) {
            if (it != mvSubArguments.begin())
                ss << ", ";
            (*it)->GenSlidingWindowDecl(ss);
        }
    }

    virtual std::string GenSlidingWindowDeclRef(void) const
    {
        std::stringstream ss;
        FormulaToken *pChild = mFormulaTree->Children[0]->GetFormulaToken();
        assert(pChild);
        ss << mSymName << "_" << CodeGen.BinFuncName() <<"(";
        size_t nItems = 0;
        for (unsigned i = 0; i < mvSubArguments.size(); i++)
        {
            if (i)
                ss << ", ";
            mvSubArguments[i]->GenDeclRef(ss);
            nItems += mvSubArguments[i]->GetWindowSize();
        }
        ss << ")";
        if (mFormulaTree->GetFormulaToken() &&
                mFormulaTree->GetFormulaToken()->GetOpCode() == ocAverage)
        {
            ss << "/(double)"<<nItems;
        }
        return ss.str();
    }
private:
    std::vector<SubArgument> mvSubArguments;
    Op CodeGen;
};

template <class Op>
std::unique_ptr<DynamicKernelArgument> SoPHelper(const std::string &ts,
    std::shared_ptr<FormulaTreeNode> ft)
{
    return std::unique_ptr<DynamicKernelArgument>(
        new DynamicKernelSoPArguments<Op>(ts, ft));
}

template <class Op>
DynamicKernelSoPArguments<Op>::DynamicKernelSoPArguments(const std::string &s,
    std::shared_ptr<FormulaTreeNode> ft):
    DynamicKernelArgument(s, ft) {
    size_t nChildren = ft->Children.size();

    for (unsigned i = 0; i < nChildren; i++)
    {
        FormulaToken *pChild = ft->Children[i]->GetFormulaToken();
        assert(pChild);
        OpCode opc = pChild->GetOpCode();
        std::stringstream tmpname;
        tmpname << s << "_" << i;
        std::string ts = tmpname.str();
        switch(opc) {
            case ocPush:
                if (pChild->GetType() == formula::svDoubleVectorRef)
                {
                    mvSubArguments.push_back(
                            SubArgument(new DynamicKernelSlidingArgument(
                                    ts, ft->Children[i])));
                } else if (pChild->GetType() == formula::svSingleVectorRef) {
                    mvSubArguments.push_back(
                            SubArgument(new DynamicKernelArgument(ts,
                                    ft->Children[i])));
                } else if (pChild->GetType() == formula::svDouble) {
                    mvSubArguments.push_back(
                            SubArgument(new DynamicKernelConstantArgument(ts,
                                    ft->Children[i])));
                }
                break;
            case ocDiv:
                mvSubArguments.push_back(SoPHelper<OpDiv>(ts, ft->Children[i]));
                break;
            case ocMul:
                mvSubArguments.push_back(SoPHelper<OpMul>(ts, ft->Children[i]));
                break;
            case ocSub:
                mvSubArguments.push_back(SoPHelper<OpSub>(ts, ft->Children[i]));
                break;
            case ocAdd:
            case ocSum:
            case ocAverage:
                mvSubArguments.push_back(SoPHelper<OpSum>(ts, ft->Children[i]));
                break;
            case ocMin:
                mvSubArguments.push_back(SoPHelper<OpMin>(ts, ft->Children[i]));
                break;
            case ocMax:
                mvSubArguments.push_back(SoPHelper<OpMax>(ts, ft->Children[i]));
                break;
            case ocCount:
                mvSubArguments.push_back(SoPHelper<OpCount>(ts, ft->Children[i]));
                break;
            case ocSumProduct:
                mvSubArguments.push_back(SoPHelper<OpSumProduct>(ts,
                    ft->Children[i]));
                break;
            case ocIRR:
                mvSubArguments.push_back(SoPHelper<OpIRR>(ts,
                    ft->Children[i]));
                break;
            case ocMIRR:
                mvSubArguments.push_back(SoPHelper<OpMIRR>(ts,
                    ft->Children[i]));
                break;
            case ocRMZ:
                mvSubArguments.push_back(SoPHelper<OpPMT>(ts,
                    ft->Children[i]));
                break;
            case ocZins:
                mvSubArguments.push_back(SoPHelper<OpIntrate>(ts,
                    ft->Children[i]));
                break;
            case ocZGZ:
                mvSubArguments.push_back(SoPHelper<OpRRI>(ts, ft->Children[i]));
                break;
            case ocKapz:
                mvSubArguments.push_back(SoPHelper<OpPPMT>(ts, ft->Children[i]));
                break;
            case ocExternal:
                if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getEffect"))))
                {
                    mvSubArguments.push_back(SoPHelper<OpEffective>(ts,
                        ft->Children[i]));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getCumipmt"))))
                {
                    mvSubArguments.push_back(SoPHelper<OpCumipmt>(ts,
                        ft->Children[i]));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getNominal"))))
                {
                    mvSubArguments.push_back(SoPHelper<OpNominal>(ts,
                        ft->Children[i]));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getCumprinc"))))
                {
                    mvSubArguments.push_back(SoPHelper<OpCumprinc>(ts,
                        ft->Children[i]));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getXnpv"))))
                {
                    mvSubArguments.push_back(SoPHelper<OpXNPV>(ts,
                        ft->Children[i]));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getPricemat"))))
                {
                    mvSubArguments.push_back(SoPHelper<OpPriceMat>(ts,
                        ft->Children[i]));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getReceived"))))
                {
                    mvSubArguments.push_back(SoPHelper<OpReceived>(ts,
                        ft->Children[i]));
                }
                else if( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getTbilleq"))))
                {
                    mvSubArguments.push_back(SoPHelper<OpTbilleq>(ts,
                        ft->Children[i]));
                }
                else if( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getTbillprice"))))
                {
                    mvSubArguments.push_back(SoPHelper<OpTbillprice>(ts,
                            ft->Children[i]));
                }
               else if( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getTbillyield"))))
                {
                    mvSubArguments.push_back(SoPHelper<OpTbillyield>(ts,
                       ft->Children[i]));
                }
                else if (!(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getFvschedule"))))
                {
                    mvSubArguments.push_back(SoPHelper<OpFvschedule>(ts,
                        ft->Children[i]));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                    "com.sun.star.sheet.addin.Analysis.getYield"))))
                {
                    mvSubArguments.push_back(SoPHelper<OpYield>(ts,
                        ft->Children[i]));
                }
                else if ( !(pChild->GetExternal().compareTo(OUString(
                   "com.sun.star.sheet.addin.Analysis.getYielddisc"))))
                {
                    mvSubArguments.push_back(SoPHelper<OpYielddisc>(ts,
                        ft->Children[i]));
                }
                else    if ( !(pChild->GetExternal().compareTo(OUString(
                     "com.sun.star.sheet.addin.Analysis.getYieldmat"))))
                {
                    mvSubArguments.push_back(SoPHelper<OpYieldmat>(ts,
                        ft->Children[i]));
                }
                break;
            default:
                assert(0 && "Unsupported");
        };
    }
}
/// Holds the symbol table for a given dynamic kernel
class SymbolTable {
public:
    typedef std::map<const FormulaToken *,
        std::shared_ptr<DynamicKernelArgument> > ArgumentMap;
    // This avoids instability caused by using pointer as the key type
    typedef std::list< std::shared_ptr<DynamicKernelArgument> > ArgumentList;
    SymbolTable(void):mCurId(0) {}
    template <class T>
    const DynamicKernelArgument *DeclRefArg(std::shared_ptr<FormulaTreeNode>);
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
    DynamicKernel(std::shared_ptr<FormulaTreeNode> r):mpRoot(r),
        mpProgram(NULL), mpKernel(NULL), mpResClmem(NULL) {}
    /// Code generation in OpenCL
    std::string CodeGen() {
        // Travese the tree of expression and declare symbols used
        const DynamicKernelArgument *DK= mSyms.DeclRefArg<
            DynamicKernelSoPArguments<OpNop> >(mpRoot);

        std::stringstream decl;
        if (OpenclDevice::gpuEnv.mnKhrFp64Flag) {
            decl << "#pragma OPENCL EXTENSION cl_khr_fp64: enable\n";
        } else if (OpenclDevice::gpuEnv.mnAmdFp64Flag) {
            decl << "#pragma OPENCL EXTENSION cl_amd_fp64: enable\n";
        }
        // preambles
        decl << publicFunc;
        decl << finacialFunc;
        mSyms.DumpSlidingWindowFunctions(decl);
        decl << "__kernel void DynamicKernel" << GetMD5();
        decl << "(\n__global double *result, ";
        DK->GenSlidingWindowDecl(decl);
        decl << ") {\n\tint gid0 = get_global_id(0);\n\tresult[gid0] = " <<
            DK->GenSlidingWindowDeclRef() << ";\n}\n";
        mFullProgramSrc = decl.str();
#if 1
        std::cerr<< "Program to be compiled = \n" << mFullProgramSrc << "\n";
#endif
        return decl.str();
    }
    /// Produce kernel hash
    std::string GetMD5(void)
    {
#ifdef MD5_KERNEL
        if(mKernelSignature.empty()) {
            std::stringstream md5s;
            // Compute MD5SUM of kernel body to obtain the name
            unsigned char result[MD5_DIGEST_LENGTH];
             MD5(reinterpret_cast<const unsigned char *>
                (mKernelSrc.str().c_str()), mKernelSrc.str().length(), result);
            for(int i=0; i < MD5_DIGEST_LENGTH; i++) {
                md5s << std::hex << (int)result[i];
            }
            mKernelSignature = md5s.str();
        }
        return mKernelSignature;
#else
        return "";
#endif
    }
    /// Create program, build, and create kerenl
    /// TODO cache results based on kernel body hash
    /// TODO: abstract OpenCL part out into OpenCL wrapper.
    bool CreateKernel(void)
    {
        cl_int err;
        std::string kname = "DynamicKernel"+GetMD5();
        // Compile kernel here!!!
        // Obtain cl context
        KernelEnv kEnv;
        OclCalc::setKernelEnv(&kEnv);
        const char *src = mFullProgramSrc.c_str();
        mpProgram = clCreateProgramWithSource(kEnv.mpkContext, 1,
                &src, NULL, &err);
        if (err != CL_SUCCESS)
            return true;
        err = clBuildProgram(mpProgram, 1,
                OpenclDevice::gpuEnv.mpArryDevsID, "", NULL, NULL);
        if (err != CL_SUCCESS)
            return true;
        mpKernel = clCreateKernel(mpProgram, kname.c_str(), &err);
        return (err != CL_SUCCESS);
    }
    /// Prepare buffers, marshal them to GPU, and launch the kernel
    /// TODO: abstract OpenCL part out into OpenCL wrapper.
    void Launch(size_t nr)
    {
        // Obtain cl context
        KernelEnv kEnv;
        OclCalc::setKernelEnv(&kEnv);
        cl_int err;
        // The results
        mpResClmem = clCreateBuffer(kEnv.mpkContext,
                (cl_mem_flags) CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR,
                nr*sizeof(double), NULL, &err);
        assert(CL_SUCCESS == err);

        err = clSetKernelArg(mpKernel, 0, sizeof(cl_mem), (void*)&mpResClmem);
        assert(CL_SUCCESS == err);
        // The rest of buffers
        mSyms.Marshal(mpKernel, nr);
        size_t global_work_size[] = {nr};
        err = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, mpKernel, 1, NULL,
            global_work_size, NULL, 0, NULL, NULL);
        assert(CL_SUCCESS == err);
    }
    ~DynamicKernel();
    cl_mem GetResultBuffer(void) const { return mpResClmem; }
private:
    void TraverseAST(std::shared_ptr<FormulaTreeNode>);
    std::shared_ptr<FormulaTreeNode> mpRoot;
    SymbolTable mSyms;
    std::stringstream mKernelSrc;
    std::string mKernelSignature;
    std::string mFullProgramSrc;
    cl_program mpProgram;
    cl_kernel mpKernel;
    cl_mem mpResClmem; // Results
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

// Symbol lookup. If there is no such symbol created, allocate one
// kernel with argument with unique name and return so.
// The template argument T must be a subclass of DynamicKernelArgument
template <typename T>
const DynamicKernelArgument *SymbolTable::DeclRefArg(
                  std::shared_ptr<FormulaTreeNode> t)
{
    FormulaToken *ref = t->GetFormulaToken();
    ArgumentMap::iterator it = mSymbols.find(ref);
    if (it == mSymbols.end()) {
        // Allocate new symbols
        std::cerr << "DeclRefArg: Allocate a new symbol:";
        std::stringstream ss;
        ss << "tmp"<< mCurId++;
        std::shared_ptr<DynamicKernelArgument> new_arg(new T(ss.str(), t));
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
    void generateRPNCode(ScDocument& rDoc,
                const ScAddress& rPos, ScTokenArray& rCode);
    DynamicKernel *mpKernel;
};

void FormulaGroupInterpreterOpenCL::generateRPNCode(ScDocument& rDoc,
            const ScAddress& rPos, ScTokenArray& rCode)
{
    // First, generate an RPN (reverse polish notation) token array.
    ScCompiler aComp(&rDoc, rPos, rCode);
    aComp.SetGrammar(rDoc.GetGrammar());
    aComp.CompileTokenArray(); // Create RPN token array.
}

ScMatrixRef FormulaGroupInterpreterOpenCL::inverseMatrix( const ScMatrix& )
{
    return NULL;
}

bool FormulaGroupInterpreterOpenCL::interpret( ScDocument& rDoc,
    const ScAddress& rTopPos, const ScFormulaCellGroupRef& xGroup,
    ScTokenArray& rCode )
{
    generateRPNCode(rDoc, rTopPos, rCode);
    // printf("Vector width = %d\n", xGroup->mnLength);
    // Constructing "AST"
    FormulaTokenIterator aCode = rCode;
    std::list<FormulaToken *> list;
    std::map<FormulaToken *, std::shared_ptr<FormulaTreeNode>> m_hash_map;
    FormulaToken*  pCur;
    while( (pCur = (FormulaToken*)(aCode.Next()) ) != NULL)
    {
        OpCode eOp = pCur->GetOpCode();
        if ( eOp != ocPush )
        {
            std::shared_ptr<FormulaTreeNode> m_currNode =
                 std::shared_ptr<FormulaTreeNode>(new FormulaTreeNode(pCur));
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
                    std::shared_ptr<FormulaTreeNode> m_ChildTreeNode =
                      std::shared_ptr<FormulaTreeNode>(
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

    std::shared_ptr<FormulaTreeNode> Root =
            std::shared_ptr<FormulaTreeNode>(new FormulaTreeNode(NULL));
    Root->Children.push_back(m_hash_map[list.back()]);
    // Code generation
    mpKernel = new DynamicKernel(Root);

    std::string kSrc = mpKernel->CodeGen();
    // Obtain cl context
    KernelEnv kEnv;
    OclCalc::setKernelEnv(&kEnv);
    // Compile kernel here!!!
    if (mpKernel->CreateKernel()) {
        std::cerr << "Cannot create kernel\n";
#define NO_FALLBACK_TO_SWINTERP 1 /* undef this for non-TDD runs */
#ifdef NO_FALLBACK_TO_SWINTERP
        assert(false);
#else
        return false;
#endif
    }
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
    assert(err == CL_SUCCESS);
    rDoc.SetFormulaResults(rTopPos, resbuf, xGroup->mnLength);
    err = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue, res, resbuf, 0, NULL, NULL);
    assert(CL_SUCCESS == err);
    delete mpKernel;
    return true;
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

SAL_DLLPUBLIC_EXPORT void compileOpenCLKernels(const OUString* pDeviceId)
{
    sc::opencl::compileOpenCLKernels(pDeviceId);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
