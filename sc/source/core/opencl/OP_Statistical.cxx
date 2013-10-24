#ifndef OP_STATISTICAL
#define OP_STATISTICAL
#include "formulagroup.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"
#include "compiler.hxx"
#include "interpre.hxx"
#include "OPBase.hxx"
#include<list>
#include <iostream>
#include <sstream>
#include <algorithm>


#include <memory>
using namespace formula;

namespace sc { namespace opencl {
 #define ISNAN
class OpFisher:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
    {
        FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
        const formula::SingleVectorRefToken*tmpCurDVR=
            dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur);
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss <<"int gid0=get_global_id(0);\n\t";
        ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n\t";
#ifdef ISNAN
        ss<< "if(isNan(arg0)||(gid0>=";
        ss<<tmpCurDVR->GetArrayLength();
        ss<<"))\n\t\t";
        ss<<"arg0 = 0;\n\t";
#endif
        ss << "double tmp=atanh(arg0);\n\t";
        ss << "return tmp;\n";
        ss << "}\n";
    }
    virtual std::string BinFuncName(void) const { return "Fisher"; }
};

class OpFisherInv:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
    {
        FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
        const formula::SingleVectorRefToken*tmpCurDVR= dynamic_cast<const
              formula::SingleVectorRefToken *>(tmpCur);
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss <<"int gid0=get_global_id(0);\n\t";
        ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n\t";
#ifdef ISNAN
        ss<< "if(isNan(arg0)||(gid0>=";
        ss<<tmpCurDVR->GetArrayLength();
        ss<<"))\n\t\t";
        ss<<"arg0 = 0;\n\t";
#endif
        ss << "double tmp=tanh(arg0);\n\t";
        ss << "return tmp;\n";
        ss << "}\n";
    }
    virtual std::string BinFuncName(void) const { return "FisherInv"; }
};

class OpGamma:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
    {
        FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
        const formula::SingleVectorRefToken*tmpCurDVR= dynamic_cast<const
              formula::SingleVectorRefToken *>(tmpCur);
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss <<"int gid0=get_global_id(0);\n\t";
        ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n\t";
        ss << "double tmp=tgamma(arg0);\n\t";
        ss << "return tmp;\n";
        ss << "}\n";
    }
    virtual std::string BinFuncName(void) const { return "Gamma"; }
};

class OpGammaLn:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
    {
        FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
        const formula::SingleVectorRefToken*tmpCurDVR= dynamic_cast<const
formula::SingleVectorRefToken *>(tmpCur);
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss <<"int gid0=get_global_id(0);\n\t";
        ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n\t";
#ifdef ISNAN
        ss<< "if(isNan(arg0)||(gid0>=";
        ss<<tmpCurDVR->GetArrayLength();
        ss<<"))\n\t\t";
        ss<<"arg0 = 0;\n\t";
#endif
        ss << "double tmp=lgamma(arg0);\n\t";
        ss << "return tmp;\n";
        ss << "}\n";
    }
    virtual std::string BinFuncName(void) const { return "GammaLn"; }
};

class OpGauss:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
    {
        FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
        const formula::SingleVectorRefToken*tmpCurDVR= dynamic_cast<const
                    formula::SingleVectorRefToken *>(tmpCur);
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss <<"int gid0=get_global_id(0);\n\t";
        ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n\t";
#ifdef ISNAN
        ss<< "if(isNan(arg0)||(gid0>=";
        ss<<tmpCurDVR->GetArrayLength();
        ss<<"))\n\t\t";
        ss<<"arg0 = 0;\n\t";
#endif
        ss << "double tmp=0.5 *erfc(-arg0 * 0.7071067811865475)-0.5;\n\t";
        ss << "return tmp;\n";
        ss << "}\n";
    }
    virtual std::string BinFuncName(void) const { return "Gauss"; }
};

class OpHarMean:public Normal{
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
        ss << "_"<< BinFuncName() <<"( ";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "double nVal=0.0;\n\t";
        ss << "int length="<<nCurWindowSize;
        ss << ";\n\tdouble tmp = 0;\n\t";
        ss << "for (int i = 0; i <" << nCurWindowSize << "; i++)\n\t";
        ss << "{\n\t";
        ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n\t";
#ifdef ISNAN
        ss<< "if(isNan(arg0)||((gid0+i)>=";
        ss<<pCurDVR->GetArrayLength();
        ss<<"))\n\t{";
        ss<<"length--;\n\t";
        ss<<"continue;\n\t}\n\t";
#endif
        ss << "nVal += (1.0/arg0);\n\t}\n\t";
        ss<<"tmp = length/nVal;\n\t";
        ss << "return tmp;\n";
        ss << "}";
    }
      virtual std::string BinFuncName(void) const { return "HarMean"; }
};
}}
#endif

