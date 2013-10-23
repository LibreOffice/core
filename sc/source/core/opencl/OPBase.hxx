#ifndef OPBASE
#define OPBASE
#include "formulagroup.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"
#include "compiler.hxx"
#include "interpre.hxx"

#include<list>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <memory>
#define ISNAN
using namespace formula;

namespace sc { namespace opencl {

class FormulaTreeNode
{
public:
    FormulaTreeNode(FormulaToken *ft): mpCurrentFormula(ft)
    {
        Children.reserve(8);
    }
    std::vector<std::shared_ptr<FormulaTreeNode>> Children;
    FormulaToken *GetFormulaToken(void) const
    {
        return mpCurrentFormula;
    }
private:
    FormulaToken *const mpCurrentFormula;
};

/// Holds an input (read-only) argument reference to a SingleVectorRef.
/// or a DoubleVectorRef for non-sliding-window argument of complex functions
/// like SumOfProduct
/// In most of the cases the argument is introduced
/// by a Push operation in the given RPN.
class DynamicKernelArgument {
public:
    DynamicKernelArgument(const std::string &s,
       std::shared_ptr<FormulaTreeNode> ft):
        mSymName(s), mFormulaTree(ft), mpClmem(NULL) {}
    const std::string &GetNameAsString(void) const { return mSymName; }
    /// Generate declaration
    virtual void GenDecl(std::stringstream &ss) const
    {
        ss << "__global double *"<<mSymName;
    }
    /// When declared as input to a sliding window function
    virtual void GenSlidingWindowDecl(std::stringstream &ss) const
    {
        GenDecl(ss);
    }
    /// When referenced in a sliding window function
    virtual std::string GenSlidingWindowDeclRef(void) const
    {
        std::stringstream ss;
        ss << mSymName << "[gid0]";
        return ss.str();
    }
    /// Generate use/references to the argument
    virtual void GenDeclRef(std::stringstream &ss) const
    {
        ss << mSymName;
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
    const std::string &GetSymName(void) const { return mSymName; }
    FormulaToken *GetFormulaToken(void) const
    {
        return mFormulaTree->GetFormulaToken();
    }
    virtual size_t GetWindowSize(void) const
    {
        FormulaToken *pCur = mFormulaTree->GetFormulaToken();
        assert(pCur);
        if (auto *pCurDVR =
                dynamic_cast<const formula::DoubleVectorRefToken *>(pCur))
        {
            return pCurDVR->GetRefRowSize();
        } else if (dynamic_cast<const formula::SingleVectorRefToken *>(pCur))
        {
            // Prepare intermediate results (on CPU for now)
            return 1;
        } else {
            assert(0 && "Unreachable");
        }
        return 0;
    }
protected:
    const std::string mSymName;
    std::shared_ptr<FormulaTreeNode> mFormulaTree;
    // Used by marshaling
    cl_mem mpClmem;
};

/// Abstract class for code generation

class SlidingFunctionBase {
public:
    typedef std::unique_ptr<DynamicKernelArgument> SubArgument;
    typedef std::vector<SubArgument> SubArguments;
    virtual void GenSlidingWindowFunction(std::stringstream &,
        const std::string, SubArguments &) = 0;
    virtual ~SlidingFunctionBase() {};
};

class OpBase {
public:
    typedef std::vector<std::string> ArgVector;
    typedef std::vector<std::string>::iterator ArgVectorIter;
    virtual std::string GetBottom(void) {return "";};
    virtual std::string Gen2(const std::string &/*lhs*/,
        const std::string &/*rhs*/) const {return "";}
    virtual std::string Gen(ArgVector& /*argVector*/){return "";};
    virtual std::string BinFuncName(void)const {return "";};
    virtual ~OpBase() {}
};

class Normal: public SlidingFunctionBase, public OpBase
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
    {
        ArgVector argVector;
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
            argVector.push_back(vSubArguments[i]->GenSlidingWindowDeclRef());
        }
        ss << ") {\n\t";
        ss << "double tmp = " << GetBottom() <<";\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "tmp = ";
        ss << Gen(argVector);
        ss << ";\n\t";
        ss << "return tmp;\n";
        ss << "}";
    }
};
}}

#endif