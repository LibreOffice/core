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
#include <openssl/md5.h>
#include <memory>
using namespace formula;

namespace sc { namespace opencl {
class FormulaTreeNode
{
public:
    FormulaToken *CurrentFormula;
    typedef std::vector<FormulaTreeNode *> Children_T;
    Children_T Children;
};
/// Holds an argument reference. In most of the cases the argument is introduced
/// by a Push operation in the given RPN.
class DynamicKernelArgument {
public:
    DynamicKernelArgument(const std::string &s, FormulaToken *ft):
        mSymName(s), mFormulaRef(ft), mpClmem(NULL) {}
    const std::string &GetNameAsString(void) const { return mSymName; }
    /// Generate declaration
    void GenDecl(std::stringstream &ss)
    {
        ss << "__global double *"<<mSymName;
    }
    /// Generate use/references to the argument
    void GenDeclRef(std::stringstream &ss) const
    {
        ss << mSymName << "[gid0]";
    }
    /// Create buffer and pass the buffer to a given kernel
    void Marshal(cl_kernel, int);
    ~DynamicKernelArgument()
    {
        //std::cerr << "~DynamicKernelArgument called. Freeing " << mSymName <<"\n";
        if (mpClmem) {
            //std::cerr << "\tFreeing cl_mem of " << mSymName <<"\n";
            cl_int ret = clReleaseMemObject(mpClmem);
            assert(ret == CL_SUCCESS);
        }
    }
private:
    const std::string mSymName;
    const FormulaToken *mFormulaRef;
    // Used by marshaling
    cl_mem mpClmem;
};

/// Map the buffer used by an argument and do necessary argument setting
void DynamicKernelArgument::Marshal(cl_kernel k, int argno)
{
    assert(mpClmem == NULL);
    assert(mFormulaRef->GetType() == formula::svSingleVectorRef);
    const formula::SingleVectorRefToken* pSVR =
        dynamic_cast< const formula::SingleVectorRefToken* >(mFormulaRef);
    double *pHostBuffer = const_cast<double*>(pSVR->GetArray().mpNumericArray);
    size_t szHostBuffer = pSVR->GetArrayLength() * sizeof(double);
    std::cout << "Marshal a Single vector of size " << pSVR->GetArrayLength();
                    std::cout << " at argument "<< argno << "\n";
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
}

/// Holds the symbol table for a given dynamic kernel
class SymbolTable {
public:
    typedef std::map<const FormulaToken *,
        std::shared_ptr<DynamicKernelArgument> > ArgumentMap;
    // This avoids instability caused by using pointer as the key type
    typedef std::list< std::shared_ptr<DynamicKernelArgument> > ArgumentList;
    SymbolTable(void):mCurId(0) {}
    const DynamicKernelArgument *DeclRefArg(FormulaToken *);
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
    void Marshal(cl_kernel);
private:
    unsigned int mCurId;
    ArgumentMap mSymbols;
    ArgumentList mParams;
};

void SymbolTable::Marshal(cl_kernel k)
{
    int i = 1; //The first argument is reserved for results
    for(ArgumentList::iterator it = mParams.begin(), e= mParams.end(); it!=e;
            ++it) {
        (*it)->Marshal(k, i++);
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
        decl << "__kernel void DynamicKernel" << GetMD5();
        decl << "(\n__global double *result";
        mSyms.DumpParamDecls(decl);
        decl << ") {\n\tint gid0 = get_global_id(0);\n\tresult[gid0] = " <<
        mKernelSrc.str() << ";\n}\n";
        mFullProgramSrc = decl.str();
        return decl.str();
    }
    /// Produce kernel hash
    std::string GetMD5(void)
    {
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
    }
    /// Create program, build, and create kerenl
    /// TBD cache results based on kernel body hash
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
        mSyms.Marshal(mpKernel);
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
const DynamicKernelArgument *SymbolTable::DeclRefArg(FormulaToken *ref)
{
    ArgumentMap::iterator it = mSymbols.find(ref);
    if (it == mSymbols.end()) {
        // Allocate new symbols
        std::cout << "DeclRefArg: Allocate a new symbol:";
        std::stringstream ss;
        ss << "tmp"<< mCurId++;
        std::shared_ptr<DynamicKernelArgument> new_arg(
            new DynamicKernelArgument(ss.str(), ref));
        mSymbols[ref] = new_arg;
        mParams.push_back(new_arg);
        std::cout << ss.str() <<"\n";
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
                    std::cout << "a Single vector of size " << pSVR->GetArrayLength();
                    std::cout << "\n";
#endif
                    mSyms.DeclRefArg(p)->GenDeclRef(mKernelSrc);
                    break;
                }
#if 0 //unexercised code
            case formula::svDoubleVectorRef:
                {
                    const formula::DoubleVectorRefToken* pDvr =
                        dynamic_cast< const formula::DoubleVectorRefToken* >( p );
                    std::cout << "a Dobule vector of size\n";
                    break;
                }
#endif
            default:
                mKernelSrc << "/* Unknown Operand */";
        };
        return;
    } else {
        // Operator
        switch (p->GetOpCode())
        {
            case ocMul:
                TraverseAST(cur->Children[0]);
                mKernelSrc << "*";
                TraverseAST(cur->Children[1]);
                return;
            case ocAdd:
                TraverseAST(cur->Children[0]);
                mKernelSrc << "+";
                TraverseAST(cur->Children[1]);
                return;
            default:
                std::cout << " /* UnknownOperator(" << p->GetOpCode() << ") */";
        }
    }
}
bool FormulaGroupInterpreterOpenCL::interpret( ScDocument& rDoc,
    const ScAddress& rTopPos, const ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode )
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
        return false;
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
