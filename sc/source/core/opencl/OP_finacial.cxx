#ifndef OP_FINACIAL
#define OP_FINACIAL
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
class RRI: public SlidingFunctionBase, public OpBase
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
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss << "double tmp = " << GetBottom() <<";\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "tmp = pow(";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss<<"/";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss<<",1.0/";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<<")-1;\n\t";
        ss << "return tmp;\n";
        ss << "}";
    }
};
class OpRRI:public RRI
{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "RRI"; }
};

class OpNominal: public Normal
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
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss << "double tmp = 0;\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss<<"tmp=( pow( "<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<<"+ 1.0, 1.0 / "<<vSubArguments[1]->GenSlidingWindowDeclRef();
        ss<<" ) - 1.0 ) *"<<vSubArguments[1]->GenSlidingWindowDeclRef();
        ss<<";\n\t";
        ss << "return tmp;\n";
        ss << "}";
    }
    virtual std::string BinFuncName(void) const { return "NOMINAL_ADD"; }
};
class Fvschedule: Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
    {
        std::cout << vSubArguments.size() << std::endl;
        FormulaToken* pCur = vSubArguments[1]->GetFormulaToken();
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
        ss << "double tmp = 1.0;\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "for (int i = 0; i <" << nCurWindowSize << "; i++)\n\t\t";
        ss << "tmp *= " << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss << " + 1.0;\n\t";
        ss << "return (double)tmp * " << vSubArguments[0]
            ->GenSlidingWindowDeclRef();
        ss << ";\n}";
    }
};
class Cumipmt: public Normal
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
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";

        ss << "   int gid0 = get_global_id(0);\n";
        ss << "   double fRmz;\n";
        ss << "   double fRate=";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ",fVal=";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss << ";\n";
        ss << "int nStartPer=";
        ss << vSubArguments[3]->GenSlidingWindowDeclRef();
        ss << ",nEndPer=";
        ss << vSubArguments[4]->GenSlidingWindowDeclRef();
        ss << ",nNumPeriods=";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss << ",nPayType=";
        ss << vSubArguments[5]->GenSlidingWindowDeclRef();
        ss << ";\n";
        ss << "fRmz = GetRmz( fRate, nNumPeriods, fVal, 0.0, nPayType );\n";
        ss << "double tmp = 0.0;\n";
        ss << "uint  nStart =  nStartPer ;\n";
        ss << "uint  nEnd =  nEndPer ;\n";
        ss << " if( nStart == 1 )\n";
        ss << "    {\n";
        ss << "        if( nPayType <= 0 )\n";
        ss << "            tmp = -fVal;\n";
        ss << "        nStart++;\n";
        ss << "    }\n";
        ss << " for( uint i = nStart ; i <= nEnd ; i++ )\n";
        ss << " {\n";
        ss << "     if( nPayType > 0 )\n";
        ss << "        tmp += GetZw( fRate, convert_double( i - 2 ), ";
        ss << "fRmz, fVal, 1 ) - fRmz;\n";
        ss << "     else\n";
        ss << "        tmp += GetZw( fRate, convert_double( i - 1 ), ";
        ss << "fRmz, fVal, 0 );\n";
        ss << " }\n";
        ss << "tmp *= fRate;\n";
        ss << "return tmp;\n";
        ss << "}";

    }
};
class IRR: Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
    {
        FormulaToken* pCur = vSubArguments[0]->GetFormulaToken();
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
        ss << ") {\n";
        ss << "#define  Epsilon   1.0E-7\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "double fSchaetzwert = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n\t";
        ss << "double fEps = 1.0;\n\t";
        ss << "double x = 0.0, xNeu = 0.0, fZaehler = 0.0, fNenner = 0.0;\n\t";
        ss << "double nCount = 0.0;\n\t";
        ss << "if (fSchaetzwert == -1.0)\n\t\t";
        ss << "x = 0.1;\n\t";
        ss << "else\n\t\t";
        ss << "x = fSchaetzwert;\n\t";
        ss << "unsigned short nItCount = 0;\n\t";
        ss << "while (fEps > Epsilon && nItCount < 20){\n\t\t";
        ss << "nCount = 0.0;\n\t\tfZaehler = 0.0;\n\t\tfNenner = 0.0;\n\t\t";
        ss << "for (int i = 0; i <" << nCurWindowSize << "; i++){\n\t\t\t";
        ss << "fZaehler += " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << " / pow(1.0+x, nCount);\n\t\t\t";
        ss << "fNenner  += -nCount * ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << " / pow(1.0+x,nCount+1.0);\n\t\t\t";
        ss << "nCount+=1;\n\t\t}\n\t\t";
        ss << "xNeu = x - fZaehler / fNenner;\n\t\t";
        ss << "fEps = fabs(xNeu - x);\n\t\t";
        ss << "x = xNeu;\n\t\t";
        ss << "nItCount++;\n\t}\n\t";
        ss << "if (fSchaetzwert == 0.0 && fabs(x) < Epsilon)\n\t\t";
        ss << "x = 0.0;\n\t";
        ss << "if (fEps < Epsilon)\n\t\t";
        ss << "return x;\n\t";
        ss << "else\n\t\treturn (double)523;\n}";
    }
};
class OpIRR: public IRR{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "IRR"; }
};
class XNPV:Normal
{
    public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
    {
        FormulaToken *pCur = vSubArguments[1]->GetFormulaToken();
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
        ss << "double result = 0.0;\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "int i=0;\n\t";
        ss << "double dateNull = ";
        ss<< vSubArguments[2]->GenSlidingWindowDeclRef();
        ss << ";\n\t";
        ss << "for (i = 0; i <" << nCurWindowSize << "; i++)\n\t\t";
        ss << "{\n\t";
        ss << "result += ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss << "/(pow((";
        ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss <<"+1),(";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss << "-dateNull)/365));\n\t";
        ss <<"}\n\t";

        ss << "return result;\n";
        ss << "}";
    }
};
class PriceMat:Normal
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
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "double result=0;\n\t";
        ss<< "int nNullDate = GetNullDate( );\n\t";
        ss <<"int settle = ";
        ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss <<";\n\t";

        ss <<"int mat = ";
        ss<<vSubArguments[1]->GenSlidingWindowDeclRef();
        ss <<";\n\t";

        ss <<"int issue = ";
        ss<<vSubArguments[2]->GenSlidingWindowDeclRef();
        ss <<";\n\t";

        ss <<"double rate = ";
        ss<<vSubArguments[3]->GenSlidingWindowDeclRef();
        ss <<";\n\t";

        ss <<"double yield = ";
        ss<<vSubArguments[4]->GenSlidingWindowDeclRef();
        ss <<";\n\t";

         ss <<"int  nBase = ";
        ss<<vSubArguments[5]->GenSlidingWindowDeclRef();
        ss <<";\n\t";

        ss<< "double      fIssMat = GetYearFrac( nNullDate, issue, mat, nBase );\n\t";
        ss<<"double      fIssSet = GetYearFrac( nNullDate, issue, settle, nBase );\n\t";
        ss<<"double      fSetMat = GetYearFrac( nNullDate, settle, mat, nBase );\n\t";
        ss<<"result = 1.0 + fIssMat * rate;\n\t";
        ss<<"result /= 1.0 + fSetMat * yield;\n\t";
        ss<<"result -= fIssSet * rate;\n\t";
        ss<<"result*= 100.0;\n\t";
        ss<<"return result;\n\t";
        ss<<"}\n";
        }
};
class MIRR: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
    {
        FormulaToken* pCur = vSubArguments[0]->GetFormulaToken();
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
        ss << "double tmp = " << GetBottom() <<";\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "double invest = "<<vSubArguments[1]->GenSlidingWindowDeclRef();
        ss << " + 1.0;\n\t";
        ss << "double reinvest ="<<vSubArguments[2]->GenSlidingWindowDeclRef();
        ss << " + 1.0;\n\t";
        ss << "double NPV_invest = 0.0;\n\t";
        ss << "double Pow_invest = 1.0;\n\t";
        ss << "double NPV_reinvest = 0.0;\n\t";
        ss << "double Pow_reinvest = 1.0;\n\t";
        ss << "int nCount = 0;\n\t";
        ss << "for (int i = 0; i < " << nCurWindowSize << "; i++)\n\t";
        ss << "{\n\t\t";
        ss << "if("<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << " > 0.0)\n\t\t\t";
        ss << "NPV_reinvest += "<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << " * Pow_reinvest;\n\t\t";
        ss << "else if("<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << " < 0.0)\n\t\t\t";
        ss << "NPV_invest += "<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << " * Pow_invest;\n\t\t";
        ss << "Pow_reinvest /= reinvest;\n\t\t";
        ss << "Pow_invest /= invest;\n\t\t";
        ss << "nCount++;\n\t";
        ss << "}\n\t";
        ss << "tmp =  -NPV_reinvest / NPV_invest *  pow( reinvest, (double) nCount - 1 );\n\t";
        ss << "tmp =  pow( tmp, 1.0 / (nCount - 1) ) - 1.0;\n\t";
        ss << "return (double)tmp;\n";
        ss << "}";
    }
};

class OpEffective: public Normal {
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string Gen(ArgVector& argVector)
    {
        std::string result = "pow(1.0+("+ argVector[0] + "/" +
          argVector[1] +")," +  argVector[1]+")-1.0";
        return result;
    }
    virtual std::string BinFuncName(void) const { return "Effective_Add"; }
};
class OpCumipmt: public Cumipmt {
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "Cumipmt"; }
};
class OpXNPV:public XNPV{

    public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "XNPV"; }

};
class OpTbilleq:public Normal{
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
        ss << "   int gid0 = get_global_id(0);\n";
        ss << "double tmp = 0;\n\t";
        ss << "double tmp000=";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss <<";\n";

        ss << "double tmp001=";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss <<";\n";

        ss << "double tmp002=";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss <<";\n";

        ss<<"tmp001+=1.0;\n";
        ss<<"int   nDiff =GetDiffDate360(GetNullDate(),tmp000,tmp001,true);\n";
        ss<<"tmp =( 365 * tmp002 ) / ( 360 - ( tmp002 * ( nDiff ) ) );\n";
        ss << "return tmp;\n";
        ss << "}";
    }
    virtual std::string BinFuncName(void) const { return "fTbilleq"; }
};
class OpCumprinc: public Normal
{
public:
    virtual std::string GetBottom(void) { return "0"; }
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
                   argVector.push_back(vSubArguments[i]
                    ->GenSlidingWindowDeclRef());
             }
             ss << ") {\n\t";
             ss << "double tmp = " << GetBottom() <<";\n\t";
             ss << "int gid0 = get_global_id(0);\n\t";
             ss <<"double fRmz;\n\t";
             ss <<"double fRate = "<<argVector[0]<<",fVal = "<<argVector[2]
                  <<";\n\t";
             ss <<"int nStartPer = (int)"<<argVector[3]<<", nEndPer = (int)"
                  <<argVector[4]<<";\n\t";
             ss <<"int nNumPeriods = (int)"<<argVector[1]<<", nPayType = (int)"
                  <<argVector[5]<<";\n\t";
             ss <<"fRmz = GetRmz( fRate, nNumPeriods,fVal,0.0,nPayType );\n\t";
             ss <<"uint nStart = nStartPer ;\n\t";
             ss <<"uint nEnd = nEndPer ;\n\t";
             ss <<"if(nStart == 1)\n\t";
             ss <<"{\n\t\t";
             ss <<"if( nPayType <= 0 )\n\t\t\t";
             ss <<"tmp = fRmz + fVal * fRate;\n\t\t";
             ss <<"else\n\t\t\t";
             ss <<"tmp = fRmz;\n\t\t";
             ss <<"nStart=nStart+1;\n\t";
             ss <<"}\n\t";
             ss <<"for( uint i = nStart ; i <= nEnd ; i++ )\n\t";
             ss <<"{\n\t\t";
             ss <<"if( nPayType > 0 )\n\t\t\t";
             ss <<"tmp += fRmz - ( GetZw( fRate,convert_double(i - 2),";
             ss <<"fRmz,fVal,1)- fRmz ) * fRate; ";
             ss <<"\n\t\telse\n\t\t\t";
             ss <<"tmp += fRmz - GetZw( fRate, convert_double(i - 1),";
             ss <<"fRmz,fVal,0 ) * fRate;\n";
             ss <<"}\n\t";
             ss << "return tmp;\n";
             ss << "}";
        }
        virtual std::string BinFuncName(void) const { return "cumprinc"; }

};
class OpYield: public Normal {
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
        ss << "double tmp = " <<"0"<<";\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "tmp = getYield_( GetNullDate(), ";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            ss<<vSubArguments[i]->GenSlidingWindowDeclRef();
        }
        ss << ");\n\t";
        ss << "return tmp;\n";
        ss << "}";
    }
    virtual std::string BinFuncName(void) const { return "Yield"; }
};
class OpFvschedule: public Fvschedule{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "Fvschedule"; }
};
class OpYieldmat: public Normal {
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
        ss << "double tmp = " <<"0"<<";\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "tmp = GetYieldmat( GetNullDate(), ";
        for (unsigned i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            ss<<vSubArguments[i]->GenSlidingWindowDeclRef();
        }
        ss << ");\n\t";
        ss << "return tmp;\n";
        ss << "}";
    }
    virtual std::string BinFuncName(void) const { return "Yieldmat"; }
};
class OpPMT: public Normal
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
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss << "double tmp = 0;\n\t";
        ss<<"double tFv=0,tType=0;\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
         if(vSubArguments.size()==4)
            ss<<"tFv="<<vSubArguments[3]->GenSlidingWindowDeclRef()<<";\n\t";
        else if(vSubArguments.size()==5)
        {
            ss<<"tType="<<vSubArguments[4]->GenSlidingWindowDeclRef();
            ss<<";\n\t";
            ss<<"tFv="<<vSubArguments[3]->GenSlidingWindowDeclRef();
            ss<<";\n\t";
        }
        ss<<"if("<<vSubArguments[0]->GenSlidingWindowDeclRef()<<"==0.0)\n\t";
        ss<<"\treturn ("<<vSubArguments[2]->GenSlidingWindowDeclRef();
        ss<<"+tFv)/";
        ss<<vSubArguments[1]->GenSlidingWindowDeclRef()<<";\n\t";
        ss<<"double abl = pow(1.0+"<<vSubArguments[0]
            ->GenSlidingWindowDeclRef();
        ss << ","<<vSubArguments[1]->GenSlidingWindowDeclRef();
        ss<<");\n\ttmp-=tFv";
        ss<<";\n\ttmp-="<<vSubArguments[2]->GenSlidingWindowDeclRef();
        ss<<"*abl;\n\t"<<"tmp =tmp/(1.0+"<<vSubArguments[0]
            ->GenSlidingWindowDeclRef();
        ss<<"*tType";
        ss<<") / ( (abl-1.0)/"<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ");\n\t";
        ss << "return tmp;\n";
        ss << "}";
    }
    virtual std::string BinFuncName(void) const { return "PMT"; }
};
class OpPPMT: public Normal
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
        ss << "int gid0 = get_global_id(0);\n\t";
        ss<<"double tFv=0,tType=0;\n\t";
        if(vSubArguments.size()==5)
            ss<<"tFv="<<vSubArguments[4]->GenSlidingWindowDeclRef()<<";\n\t";
        else if(vSubArguments.size()==6)
        {
            ss<<"tType="<<vSubArguments[5]->GenSlidingWindowDeclRef();
            ss<<";\n\t";
            ss<<"tFv="<<vSubArguments[4]->GenSlidingWindowDeclRef();
            ss<<";\n\t";
        }
        ss << "double tmp = 0.0;\n\t";
        ss <<"double pmt ;\n\t";
        ss<<"if("<<vSubArguments[0]->GenSlidingWindowDeclRef()<<"==0.0)\n\t";
        ss<<"\treturn ("<<vSubArguments[3]->GenSlidingWindowDeclRef();
        ss<<"+tFv)/";
        ss<<vSubArguments[2]->GenSlidingWindowDeclRef()<<";\n\t";
        ss <<"double temp1=0;\n\t";
        ss <<"double abl = pow(1.0+";
        ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<<",";
        ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
        ss<<");\n\t";
        ss <<"temp1-=tFv";
        ss<<";\n\t";
        ss<<"temp1-=";
        ss<<vSubArguments[3]->GenSlidingWindowDeclRef();
        ss<<"*abl;\n\t";
        ss <<"pmt = temp1/(1.0+";
        ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<<"*tType";
        ss<<") / ( (abl-1.0)/";
        ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<<");\n\t";
        ss <<"double temp = pow( 1+";
        ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<<" ,";
        ss<<vSubArguments[1]->GenSlidingWindowDeclRef();
        ss<<"-2);\n\t";
        ss <<"double re;\n\t";
        ss <<"if (tType";
        ss<<" == 0)\n\t";
        ss <<"{\n\t";
        ss <<"    re = -";
        ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<<" * ";
        ss<<vSubArguments[3]->GenSlidingWindowDeclRef();
        ss<<"*pow(1+";
        ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<<" ,";
        vSubArguments[1]->GenSlidingWindowDeclRef();
        ss<<"-1) - pmt * (pow(1+";
        ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<<" ,";
        ss<<vSubArguments[1]->GenSlidingWindowDeclRef();
        ss<<"-1)-1);\n\t";
        ss <<"}\n\t";
        ss <<"else\n\t";
        ss <<"{\n\t";
        ss <<"    re = -";
        ss<<vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<<" * (";
        ss<<vSubArguments[3]->GenSlidingWindowDeclRef();
        ss<<" + pmt)* temp - pmt * (temp-1);\n\t";
        ss <<"}\n\t";
        ss <<"tmp = pmt - re;\n\t";
        ss << "return tmp;\n";
        ss << "}";
    }
    virtual std::string BinFuncName(void) const { return "PPMT"; }
};


class OpReceived:public Normal
{
public:
        virtual std::string GetBottom(void) { return "0"; }
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
                argVector.push_back(vSubArguments[i]
                    ->GenSlidingWindowDeclRef());
            }
            ss << ") {\n\t";
            ss << "double tmp = " << GetBottom() <<";\n\t";
            ss << "int gid0 = get_global_id(0);\n\t";
            ss << "tmp = "<<argVector[2]<<"/(1.0-("<<argVector[3];
            ss <<" * GetYearDiff( GetNullDate(),(int)"<<argVector[0];
            ss <<",(int)"<<argVector[1]<<",(int)"<<argVector[4]<<")));";
            ss << "\n\treturn tmp;\n";
            ss << "}";
        }
virtual std::string BinFuncName(void) const { return "Received"; }

};
class OpYielddisc: public Normal {
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
        ss << "double tmp = " <<"0"<<";\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "tmp = ("<<vSubArguments[3]->GenSlidingWindowDeclRef();
        ss<<"/"<<vSubArguments[2]->GenSlidingWindowDeclRef();
        ss<<" ) - 1.0;\n\t";
        ss << "tmp /= GetYearFrac( GetNullDate(),";
        ss<<vSubArguments[0]->GenSlidingWindowDeclRef()<<",";
        ss<<vSubArguments[1]->GenSlidingWindowDeclRef()<<",";
        ss<<vSubArguments[4]->GenSlidingWindowDeclRef();
        ss << ");\n\t";
        ss << "return tmp;\n";
        ss << "}";
}
virtual std::string BinFuncName(void) const { return "Yielddisc"; }
};
class OpTbillprice:public Normal{
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
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "double tmp = 0;\n\t";
        ss << "double tmp000=";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss <<";\n";

        ss << "double tmp001=";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss <<";\n";

        ss << "double tmp002=";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss <<";\n";

        ss<<"tmp001+=1.0;\n";
        ss<<"double  fFraction =GetYearFrac(GetNullDate(),tmp000,tmp001,0);\n";
        ss<<"tmp = 100.0 * ( 1.0 - tmp002 * fFraction );\n";
        ss << "return tmp;\n";
        ss << "}\n";
    }
    virtual std::string BinFuncName(void) const { return "fTbillprice"; }
};
class OpPriceMat:public PriceMat
{
    public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "PriceMat"; }
};
class RATE: Normal
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
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ") {\n\t";
        ss << "double result;\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "bool bValid = true, bFound = false;\n\t";
        ss << "double fX, fXnew, fTerm, fTermDerivation;\n\t";
        ss << "double fGeoSeries, fGeoSeriesDerivation;\n\t";
        ss << "int nIterationsMax = 150;\n\t";
        ss << "int nCount = 0;\n\t";
        ss << "double fEpsilonSmall = 1.0E-14;\n\t";
        ss << vSubArguments[3]->GenSlidingWindowDeclRef() << " = ";
        ss << vSubArguments[3]->GenSlidingWindowDeclRef() << " - ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << " * ";
        ss << vSubArguments[4]->GenSlidingWindowDeclRef() << ";\n\t";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef() << " = ";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef() << " + ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << " * ";
        ss << vSubArguments[4]->GenSlidingWindowDeclRef()<< ";\n\t";
        ss << "if (" << vSubArguments[0]->GenSlidingWindowDeclRef() << " == ";
        ss << "Round( " << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n\t";
        ss << "{\n\t\t";
        ss << "fX = " << vSubArguments[5]->GenSlidingWindowDeclRef() << ";\n\t\t";
        ss << "double fPowN, fPowNminus1;\n\t\t";
        ss << "while (!bFound && nCount < nIterationsMax)\n\t\t" << "{\n\t\t\t";
        ss << "fPowNminus1 = pow( 1.0+fX, ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "-1.0);\n\t\t\t";
        ss << "fPowN = fPowNminus1 * (1.0+fX);\n\t\t\t";
        ss << "if (approxEqual( fabs(fX), 0.0))\n\t\t\t" << "{\n\t\t\t\t";
        ss << "fGeoSeries = " << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n\t\t\t\t";
        ss << "fGeoSeriesDerivation = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << " * (" << vSubArguments[0]->GenSlidingWindowDeclRef() << " -1.0)/2.0;\n\t\t\t";
        ss << "}\n\t\t\t" << "else\n\t\t\t{\n\t\t\t\t";
        ss << "fGeoSeries = (fPowN-1.0)/fX;\n\t\t\t\t";
        ss << "fGeoSeriesDerivation = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << " * ";
        ss << "fPowNminus1 / fX - fGeoSeries / fX;\n\t\t\t" << "}\n\t\t\t";
        ss << "fTerm = " << vSubArguments[3]->GenSlidingWindowDeclRef();
        ss << " + " << vSubArguments[2]->GenSlidingWindowDeclRef() << " * ";
        ss << "fPowN + "<<vSubArguments[1]->GenSlidingWindowDeclRef() << " * fGeoSeries;\n\t\t\t";
        ss << "fTermDerivation = " << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss << " * " << vSubArguments[0]->GenSlidingWindowDeclRef() << " * fPowNminus1 + ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << " * fGeoSeriesDerivation;\n\t\t\t";
        ss << "if (fabs(fTerm) < fEpsilonSmall)\n\t\t\t\t";
        ss << "bFound = true;\n\t\t\t";
        ss << "else\n\t\t\t{\n\t\t\t\t";
        ss << "if (approxEqual( fabs(fTermDerivation), 0.0))\n\t\t\t\t\t";
        ss << "fXnew = fX + 1.1 * SCdEpsilon;\n\t\t\t\t";
        ss << "else\n\t\t\t\t\tfXnew = fX - fTerm / fTermDerivation;\n\t\t\t\t" ;
        ss << "nCount++;\n\t\t\t\t";
        ss << "bFound = (fabs(fXnew - fX) < SCdEpsilon);\n\t\t\t\t";
        ss << "fX = fXnew;\n\t\t\t" << "}\n\t\t}\n\t}\n\telse\n\t{\n\t\t";
        ss << "fX = (" << vSubArguments[5]->GenSlidingWindowDeclRef();
        ss << " < -1.0) ? -1.0 : " << vSubArguments[5]->GenSlidingWindowDeclRef() << ";\n\t\t";
        ss << "while (bValid && !bFound && nCount < nIterationsMax)\n\t\t" << "{\n\t\t\t";
        ss << "if (approxEqual( fabs(fX), 0.0)){\n\t\t\t\t";
        ss << "fGeoSeries = " << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n\t\t\t\t";
        ss << "fGeoSeriesDerivation = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << " * (";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "-1.0)/2.0;\n\t\t\t";
        ss << "}else{\n\t\t\t\t";
        ss << "fGeoSeries = (pow( 1.0+fX, ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ") - 1.0) / fX;\n\t\t\t\t";
        ss << "fGeoSeriesDerivation = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << " * pow( 1.0+fX, ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "-1.0) / fX - fGeoSeries / fX;\n\t\t\t}\n\t\t\t";
        ss << "fTerm = " << vSubArguments[3]->GenSlidingWindowDeclRef();
        ss << " + " << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss << " * pow(1.0 + fX, " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ") + " << vSubArguments[1]->GenSlidingWindowDeclRef() << " * fGeoSeries;\n\t\t\t";
        ss << "fTermDerivation = " << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss << " * " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << " * " << "pow( 1.0+fX, ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << " -1.0) + ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << "* fGeoSeriesDerivation;\n\t\t\t";
        ss << "if (fabs(fTerm) < fEpsilonSmall)\n\t\t\t\t";
        ss << "bFound = true;\n\t\t\t";
        ss << "else{\n\t\t\t\t";
        ss << "if (approxEqual( fabs(fTermDerivation), 0.0))\n\t\t\t\t\t";
        ss << "fXnew = fX + 1.1 * SCdEpsilon;\n\t\t\t\t";
        ss << "else\n\t\t\t\t\t";
        ss << "fXnew = fX - fTerm / fTermDerivation;\n\t\t\t\t";
        ss << "nCount++;\n\t\t\t\t";
        ss << "bFound = (fabs(fXnew - fX) < SCdEpsilon);\n\t\t\t\t";
        ss << "fX = fXnew;\n\t\t\t\t";
        ss << "bValid = (fX >= -1.0);\n\t\t\t";
        ss << "}\n\t\t}\n\t}\n\t";
        ss << vSubArguments[5]->GenSlidingWindowDeclRef() << "= fX;\n\t";
        ss << "result = bValid && bFound;\n\t";
        ss << "result = fX;\n\t";
        ss << "return result;\n";
        ss << "}";
    }
};
class OpIntrate: public RATE {
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "rate"; }
};

class OpTbillyield:public Normal{
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
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "double tmp = 0;\n\t";
        ss << "double tmp000=";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss <<";\n";

        ss << "double tmp001=";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss <<";\n";
        ss << "double tmp002=";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss <<";\n";
        ss <<"int nDiff=GetDiffDate360(GetNullDate(),tmp000,tmp001,true);\n";
        ss <<"nDiff++;\n";
        ss <<"tmp=100.0;\n";
        ss <<"tmp /= tmp002;\n";
        ss <<"tmp-=1.0;\n";
        ss <<"tmp= tmp/( nDiff );\n";
        ss <<"tmp *= 360.0;\n";
        ss <<"return tmp;\n";
        ss << "}\n";
    }
    virtual std::string BinFuncName(void) const { return "fTbillyield"; }
};
class OpMIRR: public MIRR{
 public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "MIRR"; }
};
}
}
#endif