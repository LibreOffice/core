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
class OpCorrel:public Normal{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string sSymName, SubArguments &vSubArguments)
    {
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss << "double vSum = 0.0;\n\t";
        ss << "double vXSum = 0.0;\n\t";
        ss << "double vYSum = 0.0;\n\t";
        ss << "double vXMean = 0.0;\n\t";
        ss << "double vYMean = 0.0;\n\t";

        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "double arg0 = 0.0;\n\t";
        ss << "double arg1 = 0.0;\n\t";
        ss << "int cnt = 0;\n\t";

        FormulaToken *pCurX = vSubArguments[0]->GetFormulaToken();
        FormulaToken *pCurY = vSubArguments[1]->GetFormulaToken();
        const formula::DoubleVectorRefToken* pCurDVRX =
            dynamic_cast<const formula::DoubleVectorRefToken *>(pCurX);
        const formula::DoubleVectorRefToken* pCurDVRY =
            dynamic_cast<const formula::DoubleVectorRefToken *>(pCurY);
        size_t nCurWindowSizeX = pCurDVRX->GetRefRowSize();
        size_t nCurWindowSizeY = pCurDVRY->GetRefRowSize();
        if(nCurWindowSizeX == nCurWindowSizeY)
        {
            ss << "for (int i = ";
            if (!pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
                ss << "gid0; i < " << nCurWindowSizeX << "; i++) {\n\t\t";
                ss << "arg0 = " << vSubArguments[0]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t";
                ss << "arg1 = " << vSubArguments[1]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t";
#ifdef ISNAN
                ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
                ss << pCurDVRX->GetArrayLength() << ") || (i >=";
                ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t";
                ss << "arg0 = 0.0;\n\t\t\t";
                ss << "arg1 = 0.0;\n\t\t\t";
                ss << "--cnt;\n\t\t";
                ss << "}\n\t\t";
#endif
                ss << "++cnt;\n\t\t";
                ss << "vXSum += arg0;\n\t\t";
                ss << "vYSum += arg1;\n\t";
                ss << "}\n\t";
            } else if (pCurDVRX->IsStartFixed() && !pCurDVRX->IsEndFixed()) {
                ss << "0; i < gid0 + " << nCurWindowSizeX << "; i++) {\n\t\t";
                ss << "arg0 = " << vSubArguments[0]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t";
                ss << "arg1 = " << vSubArguments[1]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t";
#ifdef ISNAN
                ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
                ss << pCurDVRX->GetArrayLength() << ") || (i >=";
                ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t";
                ss << "arg0 = 0.0;\n\t\t\t";
                ss << "arg1 = 0.0;\n\t\t\t";
                ss << "--cnt;\n\t\t";
                ss << "}\n\t\t";
#endif
                ss << "++cnt;\n\t\t";
                ss << "vXSum += arg0;\n\t\t";
                ss << "vYSum += arg1;\n\t";
                ss << "}\n\t";
            }
            else if (pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
                ss << "0; i < " << nCurWindowSizeX << "; i++) {\n\t\t";
                ss << "arg0 = " << vSubArguments[0]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t";
                ss << "arg1 = " << vSubArguments[1]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t";
#ifdef ISNAN
                ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
                ss << pCurDVRX->GetArrayLength() << ") || (i >=";
                ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t";
                ss << "arg0 = 0.0;\n\t\t\t";
                ss << "arg1 = 0.0;\n\t\t\t";
                ss << "--cnt;\n\t\t";
                ss << "}\n\t\t";
#endif
                ss << "++cnt;\n\t\t";
                ss << "vXSum += arg0;\n\t\t";
                ss << "vYSum += arg1;\n\t";
                ss << "}\n\t";
            } else {
                ss << "0; i < " << nCurWindowSizeX << "; i++) {\n\t\t";
                ss << "arg0 = " << vSubArguments[0]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t";
                ss << "arg1 = " << vSubArguments[1]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t";
#ifdef ISNAN
                ss << "if(isNan(arg0) || isNan(arg1) || (i + gid0 >= ";
                ss << pCurDVRX->GetArrayLength() << ") || (i + gid0 >=";
                ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t";
                ss << "arg0 = 0.0;\n\t\t\t";
                ss << "arg1 = 0.0;\n\t\t\t";
                ss << "--cnt;\n\t\t";
                ss << "}\n\t\t";
#endif
                ss << "++cnt;\n\t\t";
                ss << "vXSum += arg0;\n\t\t";
                ss << "vYSum += arg1;\n\t";
                ss << "}\n\t";
            }

            ss << "if(cnt < 1) {\n\t\t";
            ss << "return DBL_MIN;\n\t";
            ss << "}\n\t";
            ss << "else {\n\t\t";
            ss << "vXMean = vXSum/cnt;\n\t\t";
            ss << "vYMean = vYSum/cnt;\n\t\t";
            ss << "vXSum = 0.0;\n\t\t";
            ss << "vYSum = 0.0;\n\t\t";

            ss << "for (int i = ";
            if (!pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
                ss << "gid0; i < " << nCurWindowSizeX << "; i++) {\n\t\t\t";
                ss << "arg0 = " << vSubArguments[0]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
                ss << "arg1 = " << vSubArguments[1]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
#ifdef ISNAN
                ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
                ss << pCurDVRX->GetArrayLength() << ") || (i >=";
                ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t\t";
                ss << "arg0 = vXMean;\n\t\t\t\t";
                ss << "arg1 = vYMean;\n\t\t\t";
                ss << "}\n\t\t\t";
#endif
                ss << "vXSum += pow(arg0 - vXMean, 2);\n\t\t\t";
                ss << "vYSum += pow(arg1 - vYMean, 2);\n\t\t\t";
                ss << "vSum += (arg0 - vXMean)*(arg1 - vYMean);\n\t\t";
                ss << "}\n\t\t";
            } else if (pCurDVRX->IsStartFixed() && !pCurDVRX->IsEndFixed()) {
                ss << "0; i < gid0 + " << nCurWindowSizeX << "; i++) {\n\t\t\t";
                ss << "arg0 = " << vSubArguments[0]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
                ss << "arg1 = " << vSubArguments[1]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
#ifdef ISNAN
                ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
                ss << pCurDVRX->GetArrayLength() << ") || (i >=";
                ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t\t";
                ss << "arg0 = vXMean;\n\t\t\t\t";
                ss << "arg1 = vYMean;\n\t\t\t";
                ss << "}\n\t\t\t";
#endif
                ss << "vXSum += pow(arg0 - vXMean, 2);\n\t\t\t";
                ss << "vYSum += pow(arg1 - vYMean, 2);\n\t\t\t";
                ss << "vSum += (arg0 - vXMean)*(arg1 - vYMean);\n\t\t";
                ss << "}\n\t\t";
            } else if (pCurDVRX->IsStartFixed() && pCurDVRX->IsEndFixed()) {
                ss << "0; i < " << nCurWindowSizeX << "; i++) {\n\t\t\t";
                ss << "arg0 = " << vSubArguments[0]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
                ss << "arg1 = " << vSubArguments[1]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
#ifdef ISNAN
                ss << "if(isNan(arg0) || isNan(arg1) || (i >= ";
                ss << pCurDVRX->GetArrayLength() << ") || (i >=";
                ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t\t";
                ss << "arg0 = vXMean;\n\t\t\t\t";
                ss << "arg1 = vYMean;\n\t\t\t";
                ss << "}\n\t\t\t";
#endif
                ss << "vXSum += pow(arg0 - vXMean, 2);\n\t\t\t";
                ss << "vYSum += pow(arg1 - vYMean, 2);\n\t\t\t";
                ss << "vSum += (arg0 - vXMean)*(arg1 - vYMean);\n\t\t";
                ss << "}\n\t\t";
            } else {
                ss << "0; i < " << nCurWindowSizeX << "; i++) {\n\t\t\t";
                ss << "arg0 = " << vSubArguments[0]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
                ss << "arg1 = " << vSubArguments[1]
                    ->GenSlidingWindowDeclRef() << ";\n\t\t\t";
#ifdef ISNAN
                ss << "if(isNan(arg0) || isNan(arg1) || (i + gid0 >= ";
                ss << pCurDVRX->GetArrayLength() << ") || (i + gid0 >=";
                ss << pCurDVRY->GetArrayLength() << ")) {\n\t\t\t\t";
                ss << "arg0 = vXMean;\n\t\t\t\t";
                ss << "arg1 = vYMean;\n\t\t\t";
                ss << "}\n\t\t\t";
#endif
                ss << "vXSum += ((arg0 - vXMean)*(arg0 - vXMean));\n\t\t\t";
                ss << "vYSum += ((arg1 - vYMean)*(arg1 - vYMean));\n\t\t\t";
                ss << "vSum += (arg0 - vXMean)*(arg1 - vYMean);\n\t\t";
                ss << "}\n\t\t";
            }

            ss << "if(vXSum == 0.0 || vYSum == 0.0) {\n\t\t\t";
            ss << "return DBL_MIN;\n\t\t";
            ss << "}\n\t\t";
            ss << "else {\n\t\t\t";
            ss << "return vSum/pow(vXSum*vYSum, 0.5);\n\t\t";
            ss << "}\n\t";
            ss << "}\n";
            ss << "}";
        }
    }
    virtual std::string BinFuncName(void) const { return "Correl"; }
};
class OpNegbinomdist:public Normal{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
    {
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ")\n";
        ss << "{\n\t";
        ss << "double f,s,p;\n\t";
        ss << " int gid0=get_global_id(0);\n\t";
#ifdef ISNAN
        FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
        const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
        formula::SingleVectorRefToken *>(tmpCur0);

        FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
        const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
        formula::SingleVectorRefToken *>(tmpCur1);

        FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
        const formula::SingleVectorRefToken*tmpCurDVR2= dynamic_cast<const
        formula::SingleVectorRefToken *>(tmpCur2);
        ss<< "int buffer_f_len = ";
        ss<< tmpCurDVR0->GetArrayLength();
        ss << ";\n\t";

        ss<< "int buffer_s_len = ";
        ss<< tmpCurDVR1->GetArrayLength();
        ss << ";\n\t";

        ss<< "int buffer_p_len = ";
        ss<< tmpCurDVR2->GetArrayLength();
        ss << ";\n\t";
#endif
#ifdef ISNAN
        ss<<"if((gid0)>=buffer_p_len || isNan(";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss<<"))\n\t\t";
        ss<<"p = 0;\n\telse \n\t\t";
#endif
        ss << "  p = "<<vSubArguments[2]->GenSlidingWindowDeclRef();
        ss << " ;\n\t";
#ifdef ISNAN
        ss<<"if((gid0)>=buffer_s_len || isNan(";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss<<"))\n\t\t";
        ss<<"s = 0;\n\telse \n\t\t";
#endif
        ss << "  s = "<<vSubArguments[1]->GenSlidingWindowDeclRef();
        ss << " ;\n\t";
#ifdef ISNAN
        ss<<"if((gid0)>=buffer_f_len || isNan(";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<<"))\n\t\t";
        ss<<"f = 0;\n\telse \n\t\t";
#endif
        ss << "  f = "<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << " ;\n\t";
        ss << " double q = 1.0 - p;\n\t";
        ss << " double fFactor = pow(p,s);\n\t";
        ss << " for(int i=0; i<f; i++)\n\t";
        ss << " {\n\t";
        ss << "   fFactor *= ((double)i+s)/((double)i+1.0)*q;\n\t";
        ss << " }\n\t";
        ss << " double tmp=fFactor;\n\t";
        ss << " return tmp;\n";
        ss << "}\n";
    }
    virtual std::string BinFuncName(void) const { return "OpNegbinomdist"; }
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
class OpGeoMean:public Normal{
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
        ss << "nVal += log(arg0);\n\t";
        ss <<"}\n\t";
        ss<<"tmp = exp(nVal/length);\n\t";
        ss << "return tmp;\n";
        ss << "}";
    }
      virtual std::string BinFuncName(void) const { return "GeoMean"; }
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

