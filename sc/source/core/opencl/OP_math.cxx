#ifndef OP_MATH
#define OP_MATH
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
#define ISNAN

namespace sc { namespace opencl {
class OpCos: public Normal
{
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
        ss << ")\n{\n\t";
        ss <<"int gid0=get_global_id(0);\n\t";
        ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n\t";
#ifdef ISNAN
        ss<< "if(isNan(arg0)||(gid0>=";
        ss<<tmpCurDVR->GetArrayLength();
        ss<<"))\n\t\t";
        ss<<"arg0 = 0;\n\t";
#endif
        ss << "double tmp=cos(arg0);\n\t";
        ss << "return tmp;\n";
        ss << "}";
    }
 virtual std::string BinFuncName(void) const { return "Cos"; }
};

class OpCsc: public Normal
{
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
        ss << ")\n{\n\t";
        ss <<"int gid0=get_global_id(0);\n\t";
        ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n\t";
#ifdef ISNAN
        ss<< "if(isNan(arg0)||(gid0>=";
        ss<<tmpCurDVR->GetArrayLength();
        ss<<"))\n\t\t";
        ss<<"arg0 = 0;\n\t";
#endif
        ss << "double tmp=1/sin(arg0);\n\t";
        ss << "return tmp;\n";
        ss << "}";
    }
 virtual std::string BinFuncName(void) const { return "Csc"; }
};
}}
#endif