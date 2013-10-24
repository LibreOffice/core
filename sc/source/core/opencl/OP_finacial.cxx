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
class OpDollarde:public Normal
{
public:
        virtual std::string GetBottom(void) { return "0"; }
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
            ss << "double fInt = " << GetBottom() <<";\n\t";
            ss << "double dollar;\n\t";
            ss << "double fFrac;\n\t";
            #ifdef ISNAN
            FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
            const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
            formula::SingleVectorRefToken *>(tmpCur0);
            FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
            const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
            formula::SingleVectorRefToken *>(tmpCur1);
            ss<< "int buffer_dollar_len = ";
            ss<< tmpCurDVR0->GetArrayLength();
            ss << ";\n\t";
            ss<< "int buffer_frac_len = ";
            ss<< tmpCurDVR1->GetArrayLength();
            ss << ";\n\t";
            #endif
            #ifdef ISNAN
            ss<<"if((gid0)>=buffer_dollar_len || isNan(";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss<<"))\n\t\t";
            ss<<"dollar = 0;\n\telse \n\t\t";
            #endif
            ss<<"dollar = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss<<";\n\t";
            #ifdef ISNAN
            ss<<"if((gid0)>=buffer_frac_len || isNan(";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef();
            ss<<"))\n\t\t";
            ss<<"fFrac = 0;\n\telse \n\t\t";
            #endif
            ss<<"fFrac = ";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef();
            ss<<";\n\t";
            ss << "tmp = modf( dollar , &fInt );\n\t";
            ss << "tmp /= fFrac;\n\t";
            ss << "tmp *= pow( 10.0 , ceil( log10(fFrac ) ) );\n\t";
            ss << "tmp += fInt;\t";
            ss << "\n\treturn tmp;\n";
            ss << "}";
        }
virtual std::string BinFuncName(void) const { return "Dollarde"; }

};
class OpDollarfr:public Normal
{
public:
        virtual std::string GetBottom(void) { return "0"; }
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
            ss << "double fInt = " << GetBottom() <<";\n\t";
            ss << "double dollar;\n\t";
            ss << "double fFrac;\n\t";
            #ifdef  ISNAN
            FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
            const formula::SingleVectorRefToken*tmpCurDVR0= dynamic_cast<const
            formula::SingleVectorRefToken *>(tmpCur0);
            FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
            const formula::SingleVectorRefToken*tmpCurDVR1= dynamic_cast<const
            formula::SingleVectorRefToken *>(tmpCur1);
            ss<< "int buffer_dollar_len = ";
            ss<< tmpCurDVR0->GetArrayLength();
            ss << ";\n\t";
            ss<< "int buffer_frac_len = ";
            ss<< tmpCurDVR1->GetArrayLength();
            ss << ";\n\t";
            #endif
            #ifdef  ISNAN
            ss<<"if((gid0)>=buffer_dollar_len || isNan(";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss<<"))\n\t\t";
            ss<<"dollar = 0;\n\telse \n\t\t";
            #endif
            ss<<"dollar = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss<<";\n\t";
            #ifdef  ISNAN
            ss<<"if((gid0)>=buffer_frac_len || isNan(";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef();
            ss<<"))\n\t\t";
            ss<<"fFrac = 0;\n\telse \n\t\t";
            #endif
            ss<<"fFrac = ";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef();
            ss<<";\n\t";
            ss << "tmp = modf( dollar , &fInt );\n\t";
            ss << "tmp *= fFrac;\n\t";
            ss << "tmp *= pow( 10.0 , -ceil( log10(fFrac ) ) );\n\t";
            ss << "tmp += fInt;\t";
            ss << "\n\treturn tmp;\n";
            ss << "}";
        }
        virtual std::string BinFuncName(void) const { return "Dollarfr"; }

};

class Fvschedule: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
            const std::string sSymName, SubArguments &vSubArguments)
    {
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
        ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n\t";
        #ifdef  ISNAN
        ss << "if (isNan(arg0))\n\t\t";
        ss << "arg0 = 0;\n\t";
        #endif
        ss << "double arg1;\n\t";
        ss << "int arrayLength = " << pCurDVR->GetArrayLength() << ";\n\t";
        #ifdef  ISNAN
        ss << "for (int i = 0; i + gid0 < arrayLength &&";
        ss << " i < " << nCurWindowSize << "; i++){\n\t\t";
        #else
        ss << "for (int i = 0; i < " << nCurWindowSize << "; i++){\n\t\t";
        #endif
        ss << "arg1 = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n\t\t\t";
        #ifdef ISNAN
        ss << "if (isNan(arg1))\n\t\t\t\t";
        ss << "arg1 = 0;\n\t\t\t";
        #endif
        ss << "tmp *= arg1 + 1.0;\n\t\t";
        ss << "}\n\t";
        ss << "return (double)tmp * arg0";
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
class IRR: public Normal
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
        FormulaToken* pCur1 = vSubArguments[1]->GetFormulaToken();
        assert(pCur1);
        const formula::SingleVectorRefToken* pSVR1 =
            dynamic_cast< const formula::SingleVectorRefToken* >(pCur1);
        assert(pSVR1);
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
        ss << "int argLen1 = " << pSVR1->GetArrayLength() << ";\n\t";
        #ifdef  ISNAN
        ss << "if (gid0 >= argLen1)\n\t\t";
        ss << "fSchaetzwert = 0.1;\n\t";
        ss << "if (isNan(fSchaetzwert))\n\t\t";
        ss << "x = 0.1;\n\t";
        ss << "else\n\t\t";
        #endif
        ss << "x = fSchaetzwert;\n\t";
        ss << "unsigned short nItCount = 0;\n\t";
        ss << "while (fEps > Epsilon && nItCount < 20){\n\t\t";
        ss << "nCount = 0.0;\n\t\tfZaehler = 0.0;\n\t\tfNenner = 0.0;\n\t\t";
        ss << "double arg0;\n\t\t";
        ss << "int arrayLength = " << pCurDVR->GetArrayLength() << ";\n\t";
        #ifdef  ISNAN
        ss << "for (int i = 0; i + gid0 < arrayLength &&";
        ss << " i < " << nCurWindowSize << "; i++){\n\t\t\t";
        #else
        ss << "for (int i = 0; i < " << nCurWindowSize << "; i++){\n\t\t\t";
        #endif
        ss << "arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n\t\t\t";
        #ifdef  ISNAN
        ss << "if (isNan(arg0))\n\t\t\t\t";
        ss << "continue;\n\t\t\t";
        #endif
        ss << "fZaehler += arg0  / pow(1.0+x, nCount);\n\t\t\t";
        ss << "fNenner  += -nCount * arg0 / pow(1.0+x,nCount+1.0);\n\t\t\t";
        ss << "nCount+=1;\n";
        ss << "\n\t\t}\n\t\t";
        ss << "xNeu = x - fZaehler / fNenner;\n\t\t";
        ss << "fEps = fabs(xNeu - x);\n\t\t";
        ss << "x = xNeu;\n\t\t";
        ss << "nItCount++;\n\t}\n\t";
        ss << "if (fSchaetzwert == 0.0 && fabs(x) < Epsilon)\n\t\t";
        ss << "x = 0.0;\n\t";
        ss << "if (fEps < Epsilon)\n\t\t";
        ss << "return x;\n\t";
        ss << "else\n\t\t";
        ss << "return (double)523;\n";
        ss << "}";
    }
};
class OpIRR: public IRR{
public:
    virtual std::string GetBottom(void) { return "0"; }
    virtual std::string BinFuncName(void) const { return "IRR"; }
};

class XNPV: public Normal
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
class PriceMat: public Normal
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
class OpSYD: public Normal
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
        ss << "double cost;\n\t";
        ss << "double salvage;\n\t";
        ss << "double life;\n\t";
        ss << "double period;\n\t";
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

        FormulaToken *tmpCur3 = vSubArguments[3]->GetFormulaToken();
        const formula::SingleVectorRefToken*tmpCurDVR3= dynamic_cast<const
            formula::SingleVectorRefToken *>(tmpCur3);

        ss<< "int buffer_cost_len = ";
        ss<< tmpCurDVR0->GetArrayLength();
        ss << ";\n\t";

         ss<< "int buffer_salvage_len = ";
        ss<< tmpCurDVR1->GetArrayLength();
        ss << ";\n\t";

        ss<< "int buffer_life_len = ";
        ss<< tmpCurDVR2->GetArrayLength();
        ss << ";\n\t";
        ss<< "int buffer_period_len = ";
        ss<< tmpCurDVR3->GetArrayLength();
        ss << ";\n\t";
#endif

#ifdef ISNAN
        ss<<"if(gid0>=buffer_cost_len || isNan(";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<<"))\n\t\t";
        ss<<"cost = 0;\n\telse \n\t\t";
#endif
        ss<<"cost = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<<";\n\t";
#ifdef ISNAN
        ss<<"if(gid0>=buffer_salvage_len || isNan(";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss<<"))\n\t\t";
        ss<<"salvage = 0;\n\telse \n\t\t";
#endif
        ss<<"salvage = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss<<";\n\t";
#ifdef ISNAN
        ss<<"if(gid0>=buffer_life_len || isNan(";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss<<"))\n\t\t";
        ss<<"life = 0;\n\telse \n\t\t";
#endif
        ss<<"life = ";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss<<";\n\t";
#ifdef ISNAN
        ss<<"if(gid0>=buffer_period_len || isNan(";
        ss << vSubArguments[3]->GenSlidingWindowDeclRef();
        ss<<"))\n\t\t";
        ss<<"period = 0;\n\telse \n\t\t";
#endif
        ss<<"period = ";
        ss << vSubArguments[3]->GenSlidingWindowDeclRef();
        ss<<";\n\t";
        ss<<"result = ((cost-salvage)*(life-period+1)/((life*(life+1))/2));\n\t";
        ss<<"return result;\n\t";
        ss<<"}\n";
        }
     virtual std::string BinFuncName(void) const { return "SYD"; }
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
        FormulaToken* pCur1 = vSubArguments[1]->GetFormulaToken();
        assert(pCur1);
        const formula::SingleVectorRefToken* pSVR1 =
            dynamic_cast< const formula::SingleVectorRefToken* >(pCur1);
        assert(pSVR1);
        FormulaToken* pCur2 = vSubArguments[2]->GetFormulaToken();
        assert(pCur2);
        const formula::SingleVectorRefToken* pSVR2 =
            dynamic_cast< const formula::SingleVectorRefToken* >(pCur2);
        assert(pSVR2);

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
        ss << "double arg0, arg1, arg2;\n\t";
        ss << "arg1 = " << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss << ";\n\t";
        ss << "arg2 = " << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss << ";\n\t";
        ss << "int argLen1 = " << pSVR1->GetArrayLength() << ";\n\t";
        ss << "int argLen2 = " << pSVR2->GetArrayLength() << ";\n\t";
        #ifdef  ISNAN
        ss << "if (gid0 >= argLen1)\n\t\t";
        ss << "arg1 = 0.0;\n\t";
        ss << "if (gid0 >= argLen2)\n\t\t";
        ss << "arg2 = 0.0;\n\t";
        ss << "if (isNan(arg1))\n\t\t";
        ss << "arg1 = 0.0;\n\t";
        ss << "if (isNan(arg2))\n\t\t";
        ss << "arg2 = 0.0;\n\t";
        #endif
        ss << "double invest = arg1 + 1.0;\n\t";
        ss << "double reinvest = arg2 + 1.0;\n\t";
        ss << "double NPV_invest = 0.0;\n\t";
        ss << "double Pow_invest = 1.0;\n\t";
        ss << "double NPV_reinvest = 0.0;\n\t";
        ss << "double Pow_reinvest = 1.0;\n\t";
        ss << "int nCount = 0;\n\t";
        ss << "int arrayLength = " << pCurDVR->GetArrayLength() << ";\n\t";
        #ifdef  ISNAN
        ss << "for (int i = 0; i + gid0 < arrayLength &&";
        ss << " i < " << nCurWindowSize << "; i++){\n\t\t";
        #else
        ss << "for (int i = 0; i < " << nCurWindowSize << "; i++){\n\t\t";
        #endif
        ss << "arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n\t\t";
        #ifdef  ISNAN
        ss << "if (isNan(arg0))\n\t\t\t";
        ss << "continue;\n\t\t";
        #endif
        ss << "if (arg0 > 0.0)\n\t\t\t";
        ss << "NPV_reinvest += arg0 * Pow_reinvest;\n\t\t";
        ss << "else if (arg0 < 0.0)\n\t\t\t";
        ss << "NPV_invest += arg0 * Pow_invest;\n\t\t";
        ss << "Pow_reinvest /= reinvest;\n\t\t";
        ss << "Pow_invest /= invest;\n\t\t";
        ss << "nCount++;\n\t";
        ss << "}\n\t";
        ss << "tmp = ";
        ss << "-NPV_reinvest /NPV_invest * pow(reinvest,(double)nCount-1);\n\t";
        ss << "tmp =  pow(tmp, 1.0 / (nCount - 1)) - 1.0;\n\t";
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
class OpAccrintm: public Normal
{
 public:
    virtual std::string GetBottom(void) { return "0"; }
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
        ss << "double tmp = " << GetBottom() <<";\n\t";
        ss << "int nStartDate,nEndDate,mode;\n\t";
        ss << "double fRate,fVal;\n\t";
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

        FormulaToken *tmpCur3 = vSubArguments[3]->GetFormulaToken();
        const formula::SingleVectorRefToken*tmpCurDVR3= dynamic_cast<const
        formula::SingleVectorRefToken *>(tmpCur3);

        FormulaToken *tmpCur4 = vSubArguments[4]->GetFormulaToken();
        const formula::SingleVectorRefToken*tmpCurDVR4= dynamic_cast<const
        formula::SingleVectorRefToken *>(tmpCur4);
        ss<< "int buffer_nIssue_len = ";
        ss<< tmpCurDVR0->GetArrayLength();
        ss << ";\n\t";

        ss<< "int buffer_nSettle_len = ";
        ss<< tmpCurDVR1->GetArrayLength();
        ss << ";\n\t";

        ss<< "int buffer_fRate_len = ";
        ss<< tmpCurDVR2->GetArrayLength();
        ss << ";\n\t";

        ss<< "int buffer_fVal_len = ";
        ss<< tmpCurDVR3->GetArrayLength();
        ss << ";\n\t";

        ss<< "int buffer_nMode_len = ";
        ss<< tmpCurDVR4->GetArrayLength();
        ss << ";\n\t";
#endif
#ifdef ISNAN
         ss <<"if(gid0 >= buffer_nIssue_len || isNan(";
         ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
         ss <<"))\n\t\t";
         ss <<"nStartDate = 0;\n\telse\n\t\t";
 #endif
         ss << "nStartDate=(int)";
         ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss <<";\n\t";
#ifdef ISNAN
        ss <<"if(gid0 >= buffer_nSettle_len || isNan(";
        ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
        ss <<"))\n\t\t";
        ss <<"nEndDate = 0;\n\telse\n\t\t";
#endif
       ss << "nEndDate=(int)";
       ss << vSubArguments[1]->GenSlidingWindowDeclRef();
       ss << ";\n\t";

#ifdef ISNAN
       ss <<"if(gid0 >= buffer_fRate_len || isNan(";
       ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
       ss <<"))\n\t\t";
       ss <<"fRate = 0;\n\telse\n\t\t";
#endif
       ss << "fRate=";
       ss << vSubArguments[2]->GenSlidingWindowDeclRef();
       ss <<";\n\t";
#ifdef ISNAN
       ss <<"if(gid0 >= buffer_fVal_len || isNan(";
       ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
       ss <<"))\n\t\t";
       ss <<"fVal = 0;\n\telse\n\t\t";
#endif
        ss << "fVal=";
        ss << vSubArguments[3]->GenSlidingWindowDeclRef();
        ss << ";\n\t";
#ifdef ISNAN
        ss <<"if(gid0 >= buffer_nMode_len || isNan(";
        ss <<vSubArguments[4]->GenSlidingWindowDeclRef();
        ss <<"))\n\t\t";
        ss <<"mode = 0;\n\telse\n\t\t";
#endif
        ss << "mode = (int)";
        ss << vSubArguments[4]->GenSlidingWindowDeclRef();
        ss << ";\n\t";
        ss <<"int nDays1stYear=0;\n\t";
        ss <<"int nNullDate=GetNullDate();\n\t";
        ss <<"int nTotalDays = GetDiffDate(nNullDate,nStartDate,";
        ss <<"nEndDate, mode,&nDays1stYear);\n\t";
        ss <<"tmp = fVal*fRate*convert_double(nTotalDays)";
        ss <<"/convert_double(nDays1stYear);\n\t";
        ss << "return tmp;\n";
        ss << "}";
        }
    virtual std::string BinFuncName(void) const { return "Accrintm"; }
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

class OpSLN: public Normal{
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
        ss << "double wert;\n\t";
        ss << "double rest;\n\t";
        ss << "double dauer;\n\t";

#ifdef ISNAN
        FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
        const formula::SingleVectorRefToken*tmpCurDVR0=
            dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur0);
        FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
        const formula::SingleVectorRefToken*tmpCurDVR1=
            dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur1);
        FormulaToken *tmpCur2 = vSubArguments[2]->GetFormulaToken();
        const formula::SingleVectorRefToken*tmpCurDVR2=
            dynamic_cast<const formula::SingleVectorRefToken *>(tmpCur2);
        ss<< "int buffer_wert_len = ";
        ss<< tmpCurDVR0->GetArrayLength();
        ss << ";\n\t";
        ss<< "int buffer_rest_len = ";
        ss<< tmpCurDVR1->GetArrayLength();
        ss << ";\n\t";
        ss<< "int buffer_dauer_len = ";
        ss<< tmpCurDVR2->GetArrayLength();
        ss << ";\n\t";
#endif
#ifdef ISNAN
        ss<<"if(gid0>=buffer_wert_len || isNan(";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<<"))\n\t\t";
        ss<<"wert = 0;\n\telse \n\t\t";
#endif
        ss<<"wert = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss<<";\n\t";
#ifdef ISNAN
        ss<<"if(gid0>=buffer_rest_len || isNan(";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss<<"))\n\t\t";
        ss<<"rest = 0;\n\telse \n\t\t";
#endif
        ss<<"rest = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss<<";\n\t";
#ifdef ISNAN
        ss<<"if(gid0>=buffer_dauer_len || isNan(";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss<<"))\n\t\t";
        ss<<"dauer = 0;\n\telse \n\t\t";
#endif
        ss<<"dauer = ";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss<<";\n\t";
        ss << "tmp = (wert-rest)/dauer;\n\t";
        ss << "return tmp;\n";
        ss << "}";
    }
    virtual std::string BinFuncName(void) const { return "SLN"; }
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

class OpCoupdaybs:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0";}
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
            ss << "int nSettle,nMat,nFreq,nBase;\n\t";
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
            FormulaToken *tmpCur3 = vSubArguments[3]->GetFormulaToken();
            const formula::SingleVectorRefToken*tmpCurDVR3= dynamic_cast<const
            formula::SingleVectorRefToken *>(tmpCur3);
            ss<< "int buffer_nSettle_len = ";
            ss<< tmpCurDVR0->GetArrayLength();
            ss << ";\n\t";
            ss<< "int buffer_nMat_len = ";
            ss<< tmpCurDVR1->GetArrayLength();
            ss << ";\n\t";
            ss<< "int buffer_nFreq_len = ";
            ss<< tmpCurDVR2->GetArrayLength();
            ss << ";\n\t";
            ss<< "int buffer_nBase_len = ";
            ss<< tmpCurDVR3->GetArrayLength();
            ss << ";\n\t";
#endif
#ifdef ISNAN
            ss <<"if(gid0 >= buffer_nSettle_len || isNan(";
            ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
            ss <<"))\n\t\t";
            ss <<"nSettle = 0;\n\telse\n\t\t";
#endif
           ss << "nSettle=(int)";
           ss << vSubArguments[0]->GenSlidingWindowDeclRef();
           ss <<";\n\t";
#ifdef ISNAN
            ss <<"if(gid0 >= buffer_nMat_len || isNan(";
            ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
            ss <<"))\n\t\t";
            ss <<"nMat = 0;\n\telse\n\t\t";
#endif
            ss << "nMat=(int)";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef();
            ss << ";\n\t";
#ifdef ISNAN
            ss <<"if(gid0 >= buffer_nFreq_len || isNan(";
            ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
            ss <<"))\n\t\t";
            ss <<"nFreq = 0;\n\telse\n\t\t";
#endif
            ss << "nFreq=(int)";
            ss << vSubArguments[2]->GenSlidingWindowDeclRef();
            ss <<";\n\t";
#ifdef ISNAN
            ss <<"if(gid0 >= buffer_nBase_len || isNan(";
            ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
            ss <<"))\n\t\t";
            ss <<"nBase = 0;\n\telse\n\t\t";
#endif
            ss << "nBase=(int)";
            ss << vSubArguments[3]->GenSlidingWindowDeclRef();
            ss << ";\n\t";
            ss <<"int nNullDate=GetNullDate();\n\t";
            ss <<"tmp = lcl_Getcoupdaybs(nNullDate,";
            ss <<"nSettle, nMat,nFreq,nBase);\n\t";
            ss << "return tmp;\n";
            ss << "}";
        }
virtual std::string BinFuncName(void) const { return "Coupdaybs";}

};
class OpCoupdays:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0";}
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
            ss << "int nSettle,nMat,nFreq,nBase;\n\t";
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
            FormulaToken *tmpCur3 = vSubArguments[3]->GetFormulaToken();
            const formula::SingleVectorRefToken*tmpCurDVR3= dynamic_cast<const
            formula::SingleVectorRefToken *>(tmpCur3);
            ss<< "int buffer_nSettle_len = ";
            ss<< tmpCurDVR0->GetArrayLength();
            ss << ";\n\t";
            ss<< "int buffer_nMat_len = ";
            ss<< tmpCurDVR1->GetArrayLength();
            ss << ";\n\t";
            ss<< "int buffer_nFreq_len = ";
            ss<< tmpCurDVR2->GetArrayLength();
            ss << ";\n\t";
            ss<< "int buffer_nBase_len = ";
            ss<< tmpCurDVR3->GetArrayLength();
            ss << ";\n\t";
#endif
#ifdef ISNAN
            ss <<"if(gid0 >= buffer_nSettle_len || isNan(";
            ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
            ss <<"))\n\t\t";
            ss <<"nSettle = 0;\n\telse\n\t\t";
#endif
           ss << "nSettle=(int)";
           ss << vSubArguments[0]->GenSlidingWindowDeclRef();
           ss <<";\n\t";
#ifdef ISNAN
            ss <<"if(gid0 >= buffer_nMat_len || isNan(";
            ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
            ss <<"))\n\t\t";
            ss <<"nMat = 0;\n\telse\n\t\t";
#endif
            ss << "nMat=(int)";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef();
            ss << ";\n\t";
#ifdef ISNAN
            ss <<"if(gid0 >= buffer_nFreq_len || isNan(";
            ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
            ss <<"))\n\t\t";
            ss <<"nFreq = 0;\n\telse\n\t\t";
#endif
            ss << "nFreq=(int)";
            ss << vSubArguments[2]->GenSlidingWindowDeclRef();
            ss <<";\n\t";
#ifdef ISNAN
            ss <<"if(gid0 >= buffer_nBase_len || isNan(";
            ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
            ss <<"))\n\t\t";
            ss <<"nBase = 0;\n\telse\n\t\t";
#endif
            ss << "nBase=(int)";
            ss << vSubArguments[3]->GenSlidingWindowDeclRef();
            ss << ";\n\t";
            ss <<"int nNullDate=GetNullDate();\n\t";
            ss <<"if( nBase == 1 )\n\t\t";
            ss <<"tmp = lcl_Getcoupdays(nNullDate,";
            ss <<"nSettle, nMat,nFreq, nBase);\n\t";
            ss <<"else\n\t\t";
            ss <<"tmp = (double)GetDaysInYear(0,0,nBase)/nFreq;\n\t";
            ss << "return tmp;\n";
            ss << "}";
        }
virtual std::string BinFuncName(void) const { return "Coupdays";}

};
class OpCoupdaysnc:public Normal
{
public:
    virtual std::string GetBottom(void) { return "0";}
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
            ss << "int nSettle,nMat,nFreq,nBase;\n\t";
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
            FormulaToken *tmpCur3 = vSubArguments[3]->GetFormulaToken();
            const formula::SingleVectorRefToken*tmpCurDVR3= dynamic_cast<const
            formula::SingleVectorRefToken *>(tmpCur3);
            ss<< "int buffer_nSettle_len = ";
            ss<< tmpCurDVR0->GetArrayLength();
            ss << ";\n\t";
            ss<< "int buffer_nMat_len = ";
            ss<< tmpCurDVR1->GetArrayLength();
            ss << ";\n\t";
            ss<< "int buffer_nFreq_len = ";
            ss<< tmpCurDVR2->GetArrayLength();
            ss << ";\n\t";
            ss<< "int buffer_nBase_len = ";
            ss<< tmpCurDVR3->GetArrayLength();
            ss << ";\n\t";
#endif
#ifdef ISNAN
            ss <<"if(gid0 >= buffer_nSettle_len || isNan(";
            ss <<vSubArguments[0]->GenSlidingWindowDeclRef();
            ss <<"))\n\t\t";
            ss <<"nSettle = 0;\n\telse\n\t\t";
#endif
           ss << "nSettle=(int)";
           ss << vSubArguments[0]->GenSlidingWindowDeclRef();
           ss <<";\n\t";
#ifdef ISNAN
            ss <<"if(gid0 >= buffer_nMat_len || isNan(";
            ss <<vSubArguments[1]->GenSlidingWindowDeclRef();
            ss <<"))\n\t\t";
            ss <<"nMat = 0;\n\telse\n\t\t";
#endif
            ss << "nMat=(int)";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef();
            ss << ";\n\t";
#ifdef ISNAN
            ss <<"if(gid0 >= buffer_nFreq_len || isNan(";
            ss <<vSubArguments[2]->GenSlidingWindowDeclRef();
            ss <<"))\n\t\t";
            ss <<"nFreq = 0;\n\telse\n\t\t";
#endif
            ss << "nFreq=(int)";
            ss << vSubArguments[2]->GenSlidingWindowDeclRef();
            ss <<";\n\t";
#ifdef ISNAN
            ss <<"if(gid0 >= buffer_nBase_len || isNan(";
            ss <<vSubArguments[3]->GenSlidingWindowDeclRef();
            ss <<"))\n\t\t";
            ss <<"nBase = 0;\n\telse\n\t\t";
#endif
            ss << "nBase=(int)";
            ss << vSubArguments[3]->GenSlidingWindowDeclRef();
            ss << ";\n\t";
            ss <<"tmp = coupdaysnc(nSettle,nMat,nFreq,nBase);\n\t";
            ss << "return tmp;\n";
            ss << "}";
        }
virtual std::string BinFuncName(void) const { return "Coupdaysnc";}

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

class RATE: public Normal
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

        FormulaToken* pCur = vSubArguments[5]->GetFormulaToken();
        assert(pCur);
        const formula::SingleVectorRefToken* pSVR =
            dynamic_cast< const formula::SingleVectorRefToken* >(pCur);
        assert(pSVR);
        ss << ") {\n\t";
        ss << "double result;\n\t";
        ss << "int gid0 = get_global_id(0);\n\t";
        ss << "bool bValid = true, bFound = false;\n\t";
        ss << "double fX, fXnew, fTerm, fTermDerivation;\n\t";
        ss << "double fGeoSeries, fGeoSeriesDerivation;\n\t";
        ss << "int nIterationsMax = 150;\n\t";
        ss << "int nCount = 0;\n\t";
        ss << "double fEpsilonSmall = 1.0E-14;\n\t";
        ss << "double arg0, arg1, arg2, arg3, arg4, arg5;\n\t";
        ss << "arg0=" << vSubArguments[0]->GenSlidingWindowDeclRef()<<";\n\t";
        ss << "arg1=" << vSubArguments[1]->GenSlidingWindowDeclRef()<<";\n\t";
        ss << "arg2=" << vSubArguments[2]->GenSlidingWindowDeclRef()<<";\n\t";
        ss << "arg3=" << vSubArguments[3]->GenSlidingWindowDeclRef()<<";\n\t";
        ss << "arg4=" << vSubArguments[4]->GenSlidingWindowDeclRef()<<";\n\t";
        ss << "arg5=" << vSubArguments[5]->GenSlidingWindowDeclRef()<<";\n\t";
        ss << "int guessLen = " << pSVR->GetArrayLength() << ";\n\t";
        #ifdef  ISNAN
        ss << "if (isNan(arg0) || isNan(arg1) || isNan(arg2)){\n\t\t";
        ss << "result = 523;\n\t\t";
        ss << "return result;\n\t}\n\t";
        ss << "if (isNan(arg3))\n\t\t";
        ss << "arg3 = 0.0;\n\t";
        ss << "if (isNan(arg4))\n\t\t";
        ss << "arg4 = 0.0;\n\t";
        ss << "if (isNan(arg5))\n\t\t";
        ss << "arg5 = 0.1;\n\t";
        ss << "if (gid0 >= guessLen)\n\t\t";
        ss << "arg5 = 0.1;\n\t";
        #endif
        ss << "arg3 = arg3 - arg1 * arg4;\n\t";
        ss << "arg2 = arg2 + arg1 * arg4;\n\t";
        ss << "if (arg0 == Round(arg0)){\n\t\t";
        ss << "fX = arg5;\n\t\t";
        ss << "double fPowN, fPowNminus1;\n\t\t";
        ss << "while (!bFound && nCount < nIterationsMax)\n\t\t";
        ss << "{\n\t\t\t";
        ss << "fPowNminus1 = pow( 1.0+fX, arg0-1.0);\n\t\t\t";
        ss << "fPowN = fPowNminus1 * (1.0+fX);\n\t\t\t";
        ss << "if (approxEqual( fabs(fX), 0.0))\n\t\t\t" << "{\n\t\t\t\t";
        ss << "fGeoSeries = arg0;\n\t\t\t\t";
        ss << "fGeoSeriesDerivation = arg0 * (arg0-1.0)/2.0;\n\t\t\t";
        ss << "}\n\t\t\t" << "else\n\t\t\t{\n\t\t\t\t";
        ss << "fGeoSeries = (fPowN-1.0)/fX;\n\t\t\t\t";
        ss << "fGeoSeriesDerivation =";
        ss << " arg0 * fPowNminus1 / fX - fGeoSeries /fX;";
        ss << "\n\t\t\t" << "}\n\t\t\t";
        ss << "fTerm = arg3 + arg2 *fPowN+ arg1 * fGeoSeries;\n\t\t\t";
        ss << "fTermDerivation = arg2 * arg0 * fPowNminus1 +";
        ss << " arg1 * fGeoSeriesDerivation;\n\t\t\t";
        ss << "if (fabs(fTerm) < fEpsilonSmall)\n\t\t\t\t";
        ss << "bFound = true;\n\t\t\t";
        ss << "else\n\t\t\t{\n\t\t\t\t";
        ss << "if (approxEqual(fabs(fTermDerivation), 0.0))\n\t\t\t\t\t";
        ss << "fXnew = fX + 1.1 * SCdEpsilon;\n\t\t\t\t";
        ss << "else\n\t\t\t\t\tfXnew = fX - fTerm / fTermDerivation;\n\t\t";
        ss << "nCount++;\n\t\t\t\t";
        ss << "bFound = (fabs(fXnew - fX) < SCdEpsilon);\n\t\t\t\t";
        ss << "fX = fXnew;\n\t\t\t" << "}\n\t\t}\n\t}\n\telse\n\t{\n\t\t";
        ss << "fX = (arg5 < -1.0) ? -1.0 : arg5;\n\t\t";
        ss << "while (bValid && !bFound && nCount < nIterationsMax)\n\t\t";
        ss << "{\n\t\t\t";
        ss << "if (approxEqual(fabs(fX), 0.0)){\n\t\t\t\t";
        ss << "fGeoSeries = arg0;\n\t\t\t\t";
        ss << "fGeoSeriesDerivation = arg0 * (arg0-1.0)/2.0;\n\t\t\t";
        ss << "}else{\n\t\t\t\t";
        ss << "fGeoSeries = (pow( 1.0+fX, arg0) - 1.0) / fX;\n\t\t\t\t";
        ss << "fGeoSeriesDerivation =";
        ss << " arg0 * pow(1.0+fX,arg0-1.0) /";
        ss << " fX - fGeoSeries / fX;\n\t\t\t}\n\t\t\t";
        ss << "fTerm = arg3 + arg2 *pow(1.0+fX, arg0)";
        ss << "+ arg1 * fGeoSeries;\n\t\t\t";
        ss << "fTermDerivation =";
        ss << "arg2*arg0*pow(1.0+fX,arg0-1.0)";
        ss << "+arg1*fGeoSeriesDerivation;\n\t\t\t";
        ss << "if (fabs(fTerm) < fEpsilonSmall)\n\t\t\t\t";
        ss << "bFound = true;\n\t\t\t";
        ss << "else{\n\t\t\t\t";
        ss << "if (approxEqual(fabs(fTermDerivation), 0.0))\n\t\t\t\t\t";
        ss << "fXnew = fX + 1.1 * SCdEpsilon;\n\t\t\t\t";
        ss << "else\n\t\t\t\t\t";
        ss << "fXnew = fX - fTerm / fTermDerivation;\n\t\t\t\t";
        ss << "nCount++;\n\t\t\t\t";
        ss << "bFound = (fabs(fXnew - fX) < SCdEpsilon);\n\t\t\t\t";
        ss << "fX = fXnew;\n\t\t\t\t";
        ss << "bValid = (fX >= -1.0);\n\t\t\t";
        ss << "}\n\t\t}\n\t}\n\t";
        ss << "if (bValid && bFound)\n\t\t";
        ss << "result = fX;\n\t";
        ss << "else\n\t\t";
        ss << "result = 523;\n\t";
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
