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

#include<list>
#include <map>
#include <iostream>
#include <sstream>
#ifdef MD5_KERNEL
#include <openssl/md5.h>
#endif
#include <memory>
using namespace formula;

namespace sc { namespace opencl {
class FormulaTreeNode
{
public:
    FormulaToken *CurrentFormula;
    typedef std::vector<FormulaTreeNode *> Children_T;
    Children_T Children;
    FormulaToken *GetFormulaToken(void) const
    {
        return CurrentFormula;
    }
};
/// Holds an input (read-only) argument reference to a SingleVectorRef.
/// or a DoubleVectorRef for non-sliding-window argument of complex functions
/// like SumOfProduct
/// In most of the cases the argument is introduced
/// by a Push operation in the given RPN.
class DynamicKernelArgument {
public:
    DynamicKernelArgument(const std::string &s, FormulaTreeNode *ft):
        mSymName(s), mFormulaTree(ft), mpClmem(NULL) {}
    const std::string &GetNameAsString(void) const { return mSymName; }
    /// Generate declaration
    virtual void GenDecl(std::stringstream &ss)
    {
        ss << "__global double *"<<mSymName;
    }
    /// Generate use/references to the argument
    virtual void GenDeclRef(std::stringstream &ss) const
    {
        ss << mSymName << "[gid0]";
    }
    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal(cl_kernel, int, int);
    virtual ~DynamicKernelArgument()
    {
        //std::cerr << "~DynamicKernelArgument: " << mSymName <<"\n";
        if (mpClmem) {
            //std::cerr << "\tFreeing cl_mem of " << mSymName <<"\n";
            cl_int ret = clReleaseMemObject(mpClmem);
            assert(ret == CL_SUCCESS);
        }
    }
    virtual void GenSlidingWindowFunction(std::stringstream &) {};
    virtual void GenSlidingWindowDeclRef(std::stringstream &ss)
    {
        ss << mSymName << "[i]";
    }
    const std::string &GetSymName(void) const { return mSymName; }
    FormulaToken *GetFormulaToken(void) const
    {
        return mFormulaTree->CurrentFormula;
    }
protected:
    const std::string mSymName;
    FormulaTreeNode *mFormulaTree;
    // Used by marshaling
    cl_mem mpClmem;
};

/// Map the buffer used by an argument and do necessary argument setting
size_t DynamicKernelArgument::Marshal(cl_kernel k, int argno, int)
{
    FormulaToken *ref = mFormulaTree->CurrentFormula;
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

/// Arguments that are reduced from a another vector outside the dynamic kernel
class DynamicKernelReducedArgument: public DynamicKernelArgument
{
public:
    DynamicKernelReducedArgument(const std::string &s, FormulaTreeNode *ft):
        DynamicKernelArgument(s, ft) {}
    /// Generate declaration
    virtual void GenDecl(std::stringstream &ss)
    {
        ss << "double "<<mSymName;
    }
    /// Generate use/references to the argument
    virtual void GenDeclRef(std::stringstream &ss) const
    {
        ss << mSymName;
    }
    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal(cl_kernel, int, int);
};

size_t DynamicKernelReducedArgument::Marshal(cl_kernel k, int argno, int)
{
    FormulaToken *ref = mFormulaTree->CurrentFormula;
    // May have variable arguments..Not handling them now
    assert(mFormulaTree->Children.size() == ref->GetByte());
    // if the argument of this reduce operation contains only
    // fixed ranges, then each range is calculated by another
    // kernel and will have only one reduction per
    // range per vector
    OpCode opc = ref->GetOpCode();
    int total_nr = 0;
    double cur_top = 0.0;
    for (int j = 0; j < ref->GetByte(); j++)
    {
        FormulaToken *pChild = mFormulaTree->Children[j]->CurrentFormula;
        assert(pChild);
        const formula::DoubleVectorRefToken* pChildDVR =
            dynamic_cast<const formula::DoubleVectorRefToken *>(pChild);
        assert(pChildDVR);
        // Pass that ..
        double *pHostBuffer = const_cast<double*>(
                pChildDVR->GetArrays()[0].mpNumericArray);
        size_t nHostBuffer = pChildDVR->GetArrayLength();
        // TODO either call an OpenCL reduce kerenl or use Bolt (preferred)
        // This is a CPU quick hack just to show it works. But can be REALLY slow!!

        unsigned int i = 0;
        if (j == 0) {
            if (pHostBuffer[0] == pHostBuffer[0] || nHostBuffer == 1)
            {
                cur_top = pHostBuffer[i++];
            } else {
                i++;
                cur_top = pHostBuffer[i++];
            }
        }
        if (cur_top == cur_top)
            total_nr++;

        for (; i < nHostBuffer; i++) {
            double val = pHostBuffer[i];
            if (val != val) //skip nan
                continue;
            else
                total_nr++;
            switch(opc)
            {
                case ocMin:
                    cur_top = cur_top<val?cur_top:val;// fmin(cur_top, val);
                    break;
                case ocMax:
                    cur_top = cur_top>val?cur_top:val;//fmax(cur_top, val);
                    break;
                case ocSum:
                case ocAverage:
                    cur_top = cur_top + val;
                    break;
                case ocCount:
                    break;
                default:
                    assert(0);
            }
        }
    }
    if (opc == ocAverage)
        cur_top /= total_nr;
    else if (opc == ocCount)
        cur_top = total_nr;
    // Obtain cl context
    KernelEnv kEnv;
    OclCalc::setKernelEnv(&kEnv);
    cl_int err;
    // Pass the scalar result back to the rest of the formula kernel
    err = clSetKernelArg(k, argno, sizeof(double), (void*)&cur_top);
    assert(CL_SUCCESS == err);
    return 1;
}

/// Handling a Double Vector that is used as a sliding window input
/// to either a sliding window average or sum-of-products
class DynamicKernelSlidingArgument: public DynamicKernelArgument
{
public:
    DynamicKernelSlidingArgument(const std::string &s, FormulaTreeNode *ft):
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
    virtual void GenSlidingWindowFunction(std::stringstream &)
    {
        assert(0 && "This class should not be directly used");
    }
    /// Generate use/references to the argument,
    /// if used in a standalone way
    virtual void GenDeclRef(std::stringstream &) const
    {
        assert(0 && "This class should not be directly used");
    }
    virtual void GenSlidingWindowDeclRef(std::stringstream &ss)
    {
        if (!bIsStartFixed && !bIsEndFixed)
            ss << mSymName << "[i + gid0]";
        else
            ss << mSymName << "[i]";
    }
private:
    bool bIsStartFixed, bIsEndFixed;
};

/// Helper functions that have multiple buffers
template<bool isSumOfProd>
class DynamicKernelSoPArguments: public DynamicKernelArgument
{
public:
    typedef std::unique_ptr<DynamicKernelArgument> SubArgument;

    DynamicKernelSoPArguments(const std::string &s, FormulaTreeNode *ft):
        DynamicKernelArgument(s, ft) {
        size_t nChildren = ft->Children.size();
        for (unsigned i = 0; i < nChildren; i++)
        {
            FormulaToken *pChild = ft->Children[i]->CurrentFormula;
            assert(pChild);
            assert(pChild->GetOpCode() == ocPush);
            assert(pChild->GetType() == formula::svDoubleVectorRef);
            const formula::DoubleVectorRefToken* pChildDVR =
                dynamic_cast<const formula::DoubleVectorRefToken *>(pChild);
            assert(pChildDVR);
            std::stringstream tmpname;
            tmpname << s << "_" << i;
            std::string ts = tmpname.str();
            if (pChildDVR->IsStartFixed() && pChildDVR->IsEndFixed())
            {
                mvSubArguments.push_back(
                        SubArgument(new DynamicKernelArgument(ts, ft->Children[i])));
            } else {
                mvSubArguments.push_back(
                        SubArgument(new DynamicKernelSlidingArgument(ts, ft->Children[i])));
            }
        }
    }
    /// Create buffer and pass the buffer to a given kernel
    virtual size_t Marshal(cl_kernel k, int argno, int nVectorWidth)
    {
        unsigned i = 0;
        for(auto it = mvSubArguments.begin(), e= mvSubArguments.end(); it!=e;
                ++it) {
            (*it)->Marshal(k, argno + (i++), nVectorWidth);
        }
        return i;
    }

    //// Generate sliding window helper. An example for sum
    ///  double custom_sliding_dotproduct(int nWindow, __global *sliding) {
    ///    int nr = get_global_size(0);
    ///    double tmp = 0.0;
    ///    for (int i = 0; i < nWindow; i++) {
    ///        tmp += sliding[i+get_global_id(0)];
    ///    }
    ///    return tmp;
    ///  }
    /// FIXME: optimization
    void GenSlidingWindowFunctionForSumOfProd(std::stringstream &ss)
    {
        ss << "\ndouble " << mSymName;
        ss << "_SlidingDotProduct(int nWindow,";
        for (unsigned i = 0; i < mvSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            mvSubArguments[i]->GenDecl(ss);
        }
        ss << ") {\n\t";
        ss << "double tmp = 0.0;\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "for (int i = 0; i < nWindow; i++)\n\t\t";
        ss << "tmp += ";
        for (unsigned i = 0; i < mvSubArguments.size(); i++)
        {
            if (i)
                ss << "*";
            mvSubArguments[i]->GenSlidingWindowDeclRef(ss);
        }
        ss << ";\n\t";
        ss << "return tmp;\n";
        ss << "}";
    }

    void GenSlidingWindowFunctionForReduction(std::stringstream &ss)
    {
        ss << "\ndouble " << mSymName;
        ss << "_SlidingDotProduct(int nWindow,";
        for (unsigned i = 0; i < mvSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            mvSubArguments[i]->GenDecl(ss);
        }
        ss << ") {\n\t";
        ss << "double tmp = 0.0;\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        for (unsigned i = 0; i < mvSubArguments.size(); i++)
        {
            FormulaToken *pCur = mvSubArguments[i]->GetFormulaToken();
            assert(pCur);
            const formula::DoubleVectorRefToken* pCurDVR =
                dynamic_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pCurDVR->GetRefRowSize();
            assert (!pCurDVR->IsStartFixed() || !pCurDVR->IsEndFixed());
            if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed()) {
                ss << "for (int i = gid0; i < "<< nCurWindowSize <<"; i++)\n\t\t";
            } else if (pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed()) {
                ss << "for (int i = 0; i < gid0+"<< nCurWindowSize <<"; i++)\n\t\t";
            } else if (!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed()) {
                ss << "for (int i = 0; i < "<< nCurWindowSize <<"; i++)\n\t\t";
            } else {
                assert(0 && "Unreachable code executed");
            }
            ss << "tmp += ";
            mvSubArguments[i]->GenSlidingWindowDeclRef(ss);
            ss << ";\n\t";
        }
        ss << "return tmp;\n";
        ss << "}";
    }
    virtual void GenSlidingWindowFunction(std::stringstream &ss) {
        if (isSumOfProd)
            GenSlidingWindowFunctionForSumOfProd(ss);
        else
            GenSlidingWindowFunctionForReduction(ss);
    }

    /// Generate use/references to the argument
    virtual void GenDeclRef(std::stringstream &ss) const
    {
        OpCode opc = mFormulaTree->CurrentFormula->GetOpCode();
        FormulaToken *pChild = mFormulaTree->Children[0]->CurrentFormula;
        assert(pChild);
        const formula::DoubleVectorRefToken* pChildDVR =
            dynamic_cast<const formula::DoubleVectorRefToken *>(pChild);
        assert(pChildDVR);
        // Prepare intermediate results (on CPU for now)
        size_t nWindowSize = pChildDVR->GetRefRowSize();
        ss << mSymName << "_SlidingDotProduct("<<nWindowSize<<", ";
        for (unsigned i = 0; i < mvSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            ss << mvSubArguments[i]->GetSymName();
        }
        ss << ")";
        if (mFormulaTree->CurrentFormula->GetOpCode() == ocAverage)
            ss << "/(double)"<<nWindowSize;
    }
    virtual void GenDecl(std::stringstream &ss)
    {
        for(auto it = mvSubArguments.begin(), e= mvSubArguments.end(); it!=e;
            ++it) {
            if (it != mvSubArguments.begin())
                ss << ", ";
            (*it)->GenDecl(ss);
        }
    }
private:
    std::vector<SubArgument> mvSubArguments;
};

/// Holds the symbol table for a given dynamic kernel
class SymbolTable {
public:
    typedef std::map<const FormulaToken *,
        std::shared_ptr<DynamicKernelArgument> > ArgumentMap;
    // This avoids instability caused by using pointer as the key type
    typedef std::list< std::shared_ptr<DynamicKernelArgument> > ArgumentList;
    SymbolTable(void):mCurId(0) {}
    template <class T>
    const DynamicKernelArgument *DeclRefArg(FormulaTreeNode *);
    /// Used to generate sliding window helpers
    void DumpSlidingWindowFunctions(std::stringstream &ss)
    {
        for(ArgumentList::iterator it = mParams.begin(), e= mParams.end(); it!=e;
            ++it) {
            (*it)->GenSlidingWindowFunction(ss);
            ss << "\n";
        }
    }
    /// Used to generate declartion in the kernel declaration
    void DumpParamDecls(std::stringstream &ss)
    {
        for(ArgumentList::iterator it = mParams.begin(), e= mParams.end(); it!=e;
            ++it) {
            ss << ", ";
            (*it)->GenDecl(ss);
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
    DynamicKernel(FormulaTreeNode *r):mpRoot(r), mpProgram(NULL),
        mpKernel(NULL), mpResClmem(NULL) {}
    /// Code generation in OpenCL
    std::string CodeGen() {
        // Travese the tree of expression and declare symbols used
        TraverseAST(mpRoot);
        std::stringstream decl;
        if (OpenclDevice::gpuEnv.mnKhrFp64Flag) {
            decl << "#pragma OPENCL EXTENSION cl_khr_fp64: enable\n";
        } else if (OpenclDevice::gpuEnv.mnAmdFp64Flag) {
            decl << "#pragma OPENCL EXTENSION cl_amd_fp64: enable\n";
        }
        mSyms.DumpSlidingWindowFunctions(decl);
        decl << "__kernel void DynamicKernel" << GetMD5();
        decl << "(\n__global double *result";
        mSyms.DumpParamDecls(decl);
        decl << ") {\n\tint gid0 = get_global_id(0);\n\tresult[gid0] = " <<
        mKernelSrc.str() << ";\n}\n";
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
            MD5(reinterpret_cast<const unsigned char *>(mKernelSrc.str().c_str()),
                    mKernelSrc.str().length(), result);
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
                (cl_mem_flags) CL_MEM_WRITE_ONLY|CL_MEM_ALLOC_HOST_PTR,
                nr*sizeof(double), NULL, &err);
        assert(CL_SUCCESS == err);

        err = clSetKernelArg(mpKernel, 0, sizeof(cl_mem), (void*)&mpResClmem);
        assert(CL_SUCCESS == err);
        // The rest of buffers
        mSyms.Marshal(mpKernel, nr);
        size_t global_work_size[] = {nr};
        clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, mpKernel, 1, NULL,
            global_work_size, NULL, 0, NULL, NULL);
    }
    ~DynamicKernel();
    cl_mem GetResultBuffer(void) const { return mpResClmem; }
private:
    void TraverseAST(FormulaTreeNode *);
    FormulaTreeNode *mpRoot;
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
const DynamicKernelArgument *SymbolTable::DeclRefArg(FormulaTreeNode *t)
{
    FormulaToken *ref = t->CurrentFormula;
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
    void generateRPNCode(ScDocument& rDoc, const ScAddress& rPos, ScTokenArray& rCode);
    FormulaTreeNode *Root;
    DynamicKernel *mpKernel;
};

void FormulaGroupInterpreterOpenCL::generateRPNCode(ScDocument& rDoc, const ScAddress& rPos, ScTokenArray& rCode)
{
    // First, generate an RPN (reverse polish notation) token array.
    ScCompiler aComp(&rDoc, rPos, rCode);
    aComp.SetGrammar(rDoc.GetGrammar());
    aComp.CompileTokenArray(); // Create RPN token array.
    // Now, calling FirstRPN() and NextRPN() will return tokens from the RPN token array.
}

ScMatrixRef FormulaGroupInterpreterOpenCL::inverseMatrix( const ScMatrix& rMat )
{
    return NULL;
}

/// Code generation: one-pass traversal of AST and generate OpenCL kernel body
/// on-the-fly
void DynamicKernel::TraverseAST(FormulaTreeNode *cur)
{
    FormulaToken *p = cur->CurrentFormula;
    // Opearnd reference
    if (p->GetOpCode() == ocPush) {
        switch (p->GetType())
        {
            case formula::svDouble: // a double scalar
                mKernelSrc << p->GetDouble();
                break;
            case formula::svSingleVectorRef:
                {
#if 0
                    const formula::SingleVectorRefToken* pSVR =
                        dynamic_cast< const formula::SingleVectorRefToken* >( p );
                    std::cerr << "a Single vector of size ";
                    std::cerr << pSVR->GetArrayLength() << "\n";
#endif
                    mSyms.DeclRefArg<DynamicKernelArgument>(cur)
                        ->GenDeclRef(mKernelSrc);
                    break;
                }
            default:
#if 1
                std::cerr << "Unhandled operand type " << p->GetType() << "\n";
#endif
                mKernelSrc << "/* Unknown Operand */";
        };
        return;
    } else {
        // Operator
        switch (p->GetOpCode())
        {
            // FIXME: merge all binary operators..
            case ocMul:
            case ocDiv:
                mKernelSrc<<"(";
                TraverseAST(cur->Children[1]);
                mKernelSrc << (p->GetOpCode() == ocMul?"*":"/");
                TraverseAST(cur->Children[0]);
                mKernelSrc<<")";
                return;
            case ocAdd:
            case ocSub:
                // A-B would be converted as:
                // Children[1] = A
                // Children[0] = B
                mKernelSrc<<"(";
                TraverseAST(cur->Children[1]);
                mKernelSrc << (p->GetOpCode() == ocAdd?"+":"-");
                TraverseAST(cur->Children[0]);
                mKernelSrc<<")";
                return;
            case ocSum:
            case ocMin:
            case ocMax:
            case ocAverage:
            case ocCount:
                {

                    assert(cur->Children.size() == p->GetByte());
                    FormulaToken *pChild = cur->Children[0]->CurrentFormula;
                    assert(pChild);
                    const formula::DoubleVectorRefToken* pChildDVR =
                        dynamic_cast<const formula::DoubleVectorRefToken *>(pChild);
                    assert(pChildDVR);
                    if (pChildDVR->IsStartFixed() && pChildDVR->IsEndFixed())
                    {
                        // Reduce to a scalar variable
                        mSyms.DeclRefArg<DynamicKernelReducedArgument>(cur)
                            ->GenDeclRef(mKernelSrc);
                    } else {
                        // Reduce to an array on CPU
                        // for code like avg($A1:$A2, $B$3:$B$4) Generate code like
                        //  double custom_sliding_dotproduct(int nWindow,
                        //                                   __global *sliding) {
                        //    double tmp = 0.0;
                        //    for (int i = 0; i < nWindow; i++) {
                        //        tmp += sliding[i+get_global_id(0)];
                        //    }
                        //    return tmp;
                        //  }
                        //  __kernel DynamicKernel (..., __global double *windows,
                        //                          double fixed, ...
                        //  {
                        //      (custom_sliding_sum(N, sliding)+fixed) /
                        //            get_global_size(0)) ...

                        mSyms.DeclRefArg<DynamicKernelSoPArguments<false> >(cur)
                            ->GenDeclRef(mKernelSrc);
                    }
                }
                return;
            case ocSumProduct:
                // Reduce to an array on CPU
                // for code like avg($A1:$A2, $B$3:$B$4) Generate code like
                //  double custom_sliding_dotproduct(int nWindow,
                //                                   __global *sliding) {
                //    double tmp = 0.0;
                //    for (int i = 0; i < nWindow; i++) {
                //        tmp += sliding[i+get_global_id(0)];
                //    }
                //    return tmp;
                //  }
                //  __kernel DynamicKernel (..., __global double *windows,
                //                          double fixed, ...
                //  {
                //      (custom_sliding_sum(N, sliding)+fixed) /
                //            get_global_size(0)) ...
                mSyms.DeclRefArg<DynamicKernelSoPArguments<true> >(cur)
                    ->GenDeclRef(mKernelSrc);
                return;
            default:
                std::cerr << " /* UnknownOperator(" << p->GetOpCode() << ") */";
        }
    }
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
    std::map<FormulaToken *, FormulaTreeNode*> m_hash_map;
    FormulaToken*  pCur;
    while( (pCur = (FormulaToken*)(aCode.Next()) ) != NULL)
    {
        OpCode eOp = pCur->GetOpCode();
        if ( eOp != ocPush )
        {
            FormulaTreeNode *m_currNode = new FormulaTreeNode;
            m_currNode->CurrentFormula = pCur;
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
                    FormulaTreeNode *m_ChildTreeNode = new FormulaTreeNode;
                    m_ChildTreeNode->CurrentFormula = m_TempFormula;
                    m_currNode->Children.push_back(m_ChildTreeNode);
                }
            }
            m_hash_map[pCur] = m_currNode;
        }
        list.push_back(pCur);
    }

    Root = m_hash_map[list.back()];
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
        CL_TRUE, CL_MAP_WRITE, 0,
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

SAL_DLLPUBLIC_EXPORT sc::FormulaGroupInterpreter* SAL_CALL createFormulaGroupOpenCLInterpreter()
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

SAL_DLLPUBLIC_EXPORT void SAL_CALL fillOpenCLInfo(sc::OpenclPlatformInfo* pInfos, size_t nInfoSize)
{
    const std::vector<sc::OpenclPlatformInfo>& rPlatforms = sc::opencl::fillOpenCLInfo();
    size_t n = std::min(rPlatforms.size(), nInfoSize);
    for (size_t i = 0; i < n; ++i)
        pInfos[i] = rPlatforms[i];
}

SAL_DLLPUBLIC_EXPORT bool SAL_CALL switchOpenClDevice(const OUString* pDeviceId, bool bAutoSelect)
{
    return sc::opencl::switchOpenclDevice(pDeviceId, bAutoSelect);
}

SAL_DLLPUBLIC_EXPORT void compileOpenCLKernels(const OUString* pDeviceId)
{
    sc::opencl::compileOpenCLKernels(pDeviceId);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
