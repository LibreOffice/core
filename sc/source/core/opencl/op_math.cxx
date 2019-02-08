/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_math.hxx"

#include <formula/vectortoken.hxx>
#include "opinlinefun_math.hxx"
#include <sstream>

using namespace formula;

namespace sc { namespace opencl {

void OpCos::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
        ss << "\ndouble " << sSymName;
        ss << "_"<< BinFuncName() <<"(";
        for (size_t i = 0; i < vSubArguments.size(); i++)
        {
            if (i) ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ")\n";
        ss << "{\n";
        ss << "    int gid0=get_global_id(0);\n";
        ss << "    double arg0 = 0.0f;\n";
        FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
        assert(tmpCur);
        if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
        {
            if(tmpCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken*tmpCurDVR=
                    static_cast
                    <const formula::SingleVectorRefToken *>(tmpCur);
                ss << "    arg0 = ";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef();
                ss << ";\n";
                ss << "    if(isnan(";
                ss << vSubArguments[0]->GenSlidingWindowDeclRef();
                ss << ")||(gid0>=";
                ss << tmpCurDVR->GetArrayLength();
                ss << "))\n";
                ss << "    { arg0 = 0.0f; }\n";
            }
            else if(tmpCur->GetType() == formula::svDouble)
            {
                ss << "    arg0=" << tmpCur->GetDouble() << ";\n";
            }
        }
        else
        {
            ss << "        arg0 = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
        }
        ss << "    return cos(arg0);\n";
        ss << "}";

}
void OpSec::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR= static_cast<const
          formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss <<"    double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss<<"    if(isnan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n";
    ss<<"        arg0 = 0;\n";
    ss << "    return pow(cos(arg0),-1 );\n";
    ss << "}";
}
void OpCosh::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(local_coshDecl);
    funs.insert(local_cosh);
}
void OpSecH::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR= static_cast<const
          formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss <<"    double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss<<"    if(isnan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n";
    ss<<"        arg0 = 0;\n";
    ss << "    return pow(cosh(arg0),-1 );\n";
    ss << "}";
}
void OpMROUND::GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT(2, 2);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ", ";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss<<") {\n";
    ss<<"    double tmp = 0;\n";
    ss<<"    int gid0 = get_global_id(0);\n";
    ss<<"    double arg0=0;\n";
    ss<<"    double arg1=0;\n";
    ss <<"\n    ";
    //while (i-- > 1)
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
                static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "if (gid0 < " << pSVR->GetArrayLength() << "){\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "{\n";
        }

        if(ocPush==vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            ss << "        tmp=";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "        if (isnan(tmp))\n";
            ss << "            arg"<<i<<"= 0;\n";
            ss << "        else\n";
            ss << "            arg"<<i<<"=tmp;\n";
            ss << "    }\n";
        }
        else
        {
            ss<<"    arg"<<i<<"="<<vSubArguments[i]->GenSlidingWindowDeclRef();
            ss<<";\n";
        }
    }
    ss<<"    if(arg1==0)\n";
    ss<<"        return arg1;\n";
    ss<<"    tmp=arg1 * round(arg0 * pow(arg1,-1));\n";
    ss<<"    return tmp;\n";
    ss<<"}";
}
void OpCosh::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR= static_cast<const
          formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss<< "    if(isnan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n";
    ss<<"        arg0 = 0;\n";
    ss << "    double tmp=local_cosh(arg0);\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpCot::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = 0.0f;\n";
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR=
                static_cast
                <const formula::SingleVectorRefToken *>(tmpCur);
            ss << "    arg0 = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(isnan(";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ")||(gid0>=";
            ss << tmpCurDVR->GetArrayLength();
            ss << "))\n";
            ss << "    { arg0 = 0.0f; }\n";
        }
        else if(tmpCur->GetType() == formula::svDouble)
        {
            ss << "    arg0=";
            ss << tmpCur->GetDouble() << ";\n";
        }
    }
    else
    {
        ss << "        arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    ss << "    arg0 = arg0 * M_1_PI;\n";
    ss << "    return cospi(arg0) * pow(sinpi(arg0), -1);\n";
    ss << "}";
}

void OpCoth::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(local_cothDecl);
    funs.insert(local_coth);
}

void OpCoth::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR= static_cast<const
          formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss<< "    if(isnan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n";
    ss<<"        arg0 = 0;\n";
    ss << "    double tmp=local_coth(arg0);\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpCombinA::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(bikDecl);
    funs.insert(bik);
}

void OpCombinA::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tem;\n";
    ss << "    double arg0,arg1;\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        ss << "    arg"<<i<<" = "<<vSubArguments[i]->GenSlidingWindowDeclRef();
        ss << ";\n";
        if(pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
            static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if(isnan(arg" << i <<")||(gid0 >= ";
            ss << pSVR->GetArrayLength();
            ss << "))\n";
            ss << "        arg" << i << " = 0;\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    if(isnan(arg" << i <<"))\n";
            ss << "        arg" << i << " = 0;\n";
        }
    }
    ss << "    arg0 = trunc(arg0);\n";
    ss << "    arg1 = trunc(arg1);\n";
    ss << "    if(arg0 >= arg1 && arg0 > 0 && arg1 > 0)\n";
    ss << "        tem = bik(arg0+arg1-1,arg1);\n";
    ss << "    else if(arg0 == 0 && arg1 == 0)\n";
    ss << "        tem = 0;\n";
    ss << "    else if(arg0 > 0 && arg1 == 0)\n";
    ss << "        tem = 1;\n";
    ss << "    else\n";
    ss << "        tem = -1;\n";
    ss << "    double i = tem - trunc(tem);\n";
    ss << "    if(i < 0.5)\n";
    ss << "        tem = trunc(tem);\n";
    ss << "    else\n";
    ss << "        tem = trunc(tem) + 1;\n";
    ss << "    return tem;\n";
    ss << "}";
}
void OpEven::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR= static_cast<const
          formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss<< "    if(isnan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n";
    ss<<"        arg0 = 0;\n";
    ss << "    double tmp;\n";
    ss << "    tmp = fabs(arg0 / 2);\n";
    ss << "    if ( trunc(tmp) == tmp )\n";
    ss << "        tmp = tmp * 2;\n";
    ss << "    else\n";
    ss << "        tmp = (trunc(tmp) + 1) * 2;\n";
    ss << "    if (arg0 < 0)\n";
    ss << "        tmp = tmp * -1.0;\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpMod::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    double arg1 =" << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    if(isnan(arg0)||arg0 == 0)\n";
    ss << "        return 0;\n";
    ss << "    if(isnan(arg1) || arg1 ==0)\n";
    ss << "        return NAN;\n";
    ss << "    double tem;\n";
    ss << "        if(arg0 < 0 && arg1 > 0)\n";
    ss << "            while(arg0 < 0)\n";
    ss << "                arg0 += arg1;\n";
    ss << "        else if (arg0 > 0 && arg1 < 0)\n";
    ss << "            while(arg0 > 0)\n";
    ss << "                arg0 += arg1;\n";
    ss << "        tem = fmod(arg0,arg1);\n";
    ss << "    if(arg1 < 0 && tem > 0)\n";
    ss << "        tem = -tem;\n";
    ss << "    return tem;\n";
    ss << "}";
}
void OpLog::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tem;\n";
    ss << "    double arg0,arg1;\n";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        FormulaToken *pCur = vSubArguments[i]->GetFormulaToken();
        assert(pCur);
        ss << "    arg"<<i<<" = "<<vSubArguments[i]->GenSlidingWindowDeclRef();
        ss << ";\n";
        if(pCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* pSVR =
            static_cast< const formula::SingleVectorRefToken* >(pCur);
            ss << "    if(isnan(arg" << i <<")||(gid0 >= ";
            ss << pSVR->GetArrayLength();
            ss << "))\n";
            if( i == 0)
                ss << "        arg0 = 0;\n";
            else if ( i == 1)
                ss << "        arg1 = 10;\n";
        }
        else if (pCur->GetType() == formula::svDouble)
        {
            ss << "    if(isnan(arg" << i <<"))\n";
            if( i == 0)
                ss << "        arg0 = 0;\n";
            else if ( i == 1)
                ss << "        arg1 = 10;\n";
        }
    }
    if (vSubArguments.size() < 2)
        ss << "    arg1 = 10;\n";
    ss << "    tem = log10(arg0)/log10(arg1);;\n";
    ss << "    return tem;\n";
    ss << "}";
}
void OpCsc::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR= static_cast<const
          formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n\t";
    ss <<"int gid0=get_global_id(0);\n\t";
    ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
    ss<< "if(isnan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n\t\t";
    ss<<"arg0 = 0;\n\t";
    ss << "double tmp=1/sin(arg0);\n\t";
    ss << "return tmp;\n";
    ss << "}";
}
void OpCountIfs::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
         formula::DoubleVectorRefToken *>(tmpCur);
    size_t nCurWindowSize = pCurDVR->GetArrayLength() <
    pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength():
    pCurDVR->GetRefRowSize() ;
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n    {\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss << "    int tmp =0;\n";
    ss << "    int loop;\n";
    GenTmpVariables(ss,vSubArguments);

    ss<< "    int singleIndex =gid0;\n";
    int m=0;

    std::stringstream tmpss;

    for(size_t j=0;j<vSubArguments.size();j+=2,m++)
    {
        CheckSubArgumentIsNan(tmpss,vSubArguments,j);
        CheckSubArgumentIsNan(ss,vSubArguments,j+1);
        tmpss <<"    if(isequal(";
        tmpss <<"tmp";
        tmpss <<j;
        tmpss <<" , ";
        tmpss << "tmp";
        tmpss << j+1;
        tmpss << ")){\n";
    }
    tmpss << "    tmp ++;\n";
    for(size_t j=0;j<vSubArguments.size();j+=2,m--)
    {
        for(int n = 0;n<m+1;n++)
        {
            tmpss << "    ";
        }
        tmpss<< "}\n";
    }
    UnrollDoubleVector(ss,tmpss,pCurDVR,nCurWindowSize);

    ss << "return tmp;\n";
    ss << "}";
}
void OpSumIfs::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
         formula::DoubleVectorRefToken *>(tmpCur);
    size_t nCurWindowSize = pCurDVR->GetArrayLength() <
    pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength():
    pCurDVR->GetRefRowSize() ;

    mNeedReductionKernel = vSubArguments[0]->NeedParallelReduction();
    if (mNeedReductionKernel)
    {
        // generate reduction functions

        ss << "__kernel void ";
        ss << vSubArguments[0]->GetName();
        ss << "_SumIfs_reduction(  ";
        for (size_t i = 0; i < vSubArguments.size(); i++)
        {
            if (i)
                ss << ",";
            vSubArguments[i]->GenSlidingWindowDecl(ss);
        }
        ss << ", __global double *result,int arrayLength,int windowSize";

        ss << ")\n{\n";
        ss << "    double tmp =0;\n";
        ss << "    int i ;\n";

        GenTmpVariables(ss,vSubArguments);
        ss << "    double current_result = 0.0;\n";
        ss << "    int writePos = get_group_id(1);\n";
        if (pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed())
            ss << "    int offset = 0;\n";
        else if (!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
            ss << "    int offset = get_group_id(1);\n";
        else
            throw Unhandled(__FILE__, __LINE__);
        // actually unreachable
        ss << "    int lidx = get_local_id(0);\n";
        ss << "    __local double shm_buf[256];\n";
        ss << "    barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    int loop = arrayLength/512 + 1;\n";
        ss << "    for (int l=0; l<loop; l++){\n";
        ss << "        tmp = 0.0;\n";
        ss << "        int loopOffset = l*512;\n";

        ss << "        int p1 = loopOffset + lidx + offset, p2 = p1 + 256;\n";
        ss << "        if (p2 < min(offset + windowSize, arrayLength)) {\n";
        ss << "            tmp0 = 0.0;\n";
        int mm=0;
        std::string p1 = "p1";
        std::string p2 = "p2";
        for(size_t j=1;j<vSubArguments.size();j+=2,mm++)
        {
            CheckSubArgumentIsNan2(ss,vSubArguments,j,p1);
            CheckSubArgumentIsNan2(ss,vSubArguments,j+1,p1);
            ss << "";
            ss <<"    if(isequal(";
            ss <<"tmp";
            ss <<j;
            ss <<" , ";
            ss << "tmp";
            ss << j+1;
            ss << "))";
            ss << "{\n";
        }
        CheckSubArgumentIsNan2(ss,vSubArguments,0,p1);
        ss << "    tmp += tmp0;\n";
        for(size_t j=1;j<vSubArguments.size();j+=2,mm--)
        {
            for(int n = 0;n<mm+1;n++)
            {
                ss << "    ";
            }
            ss<< "}\n\n";
        }
        mm=0;
        for(size_t j=1;j<vSubArguments.size();j+=2,mm++)
        {
            CheckSubArgumentIsNan2(ss,vSubArguments,j,p2);
            CheckSubArgumentIsNan2(ss,vSubArguments,j+1,p2);
            ss <<"    if(isequal(";
            ss <<"tmp";
            ss <<j;
            ss <<" , ";
            ss << "tmp";
            ss << j+1;
            ss << ")){\n";
        }
        CheckSubArgumentIsNan2(ss,vSubArguments,0,p2);
        ss << "    tmp += tmp0;\n";
        for(size_t j=1;j< vSubArguments.size();j+=2,mm--)
        {
            for(int n = 0;n<mm+1;n++)
            {
                ss << "    ";
            }
            ss<< "}\n";
        }
        ss << "    }\n";

        ss << "    else if (p1 < min(arrayLength, offset + windowSize)) {\n";
        mm=0;
        for(size_t j=1;j<vSubArguments.size();j+=2,mm++)
        {
            CheckSubArgumentIsNan2(ss,vSubArguments,j,p1);
            CheckSubArgumentIsNan2(ss,vSubArguments,j+1,p1);

            ss <<"    if(isequal(";
            ss <<"tmp";
            ss <<j;
            ss <<" , ";
            ss << "tmp";
            ss << j+1;
            ss << ")){\n";
        }
        CheckSubArgumentIsNan2(ss,vSubArguments,0,p1);
        ss << "    tmp += tmp0;\n";
        for(size_t j=1;j<vSubArguments.size();j+=2,mm--)
        {
            for(int n = 0;n<mm+1;n++)
            {
                ss << "    ";
            }
            ss<< "}\n\n";
        }

        ss << "    }\n";
        ss << "    shm_buf[lidx] = tmp;\n";
        ss << "    barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    for (int i = 128; i >0; i/=2) {\n";
        ss << "        if (lidx < i)\n";
        ss << "            shm_buf[lidx] += shm_buf[lidx + i];\n";
        ss << "        barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    }\n";
        ss << "    if (lidx == 0)\n";
        ss << "        current_result += shm_buf[0];\n";
        ss << "    barrier(CLK_LOCAL_MEM_FENCE);\n";
        ss << "    }\n";

        ss << "    if (lidx == 0)\n";
        ss << "        result[writePos] = current_result;\n";
        ss << "}\n";
    }// finish generate reduction code
    // generate functions as usual
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n    {\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss << "    double tmp =0;\n";
    if (!mNeedReductionKernel)
    {
        ss << "    int i ;\n";
        GenTmpVariables(ss,vSubArguments);
        ss << "    for (i = ";
        if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed()) {
            ss << "gid0; i < "<< nCurWindowSize <<"; i++)\n";
        } else if (pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed()) {
            ss << "0; i < gid0+"<< nCurWindowSize <<"; i++)\n";
        } else {
            ss << "0; i < "<< nCurWindowSize <<"; i++)\n";
        }
        ss << "    {\n";
        if(!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
        {
            ss<< "    int doubleIndex =i+gid0;\n";
        }else
        {
            ss<< "    int doubleIndex =i;\n";
        }
        ss<< "    int singleIndex =gid0;\n";
        int m=0;
        for(size_t j=1;j<vSubArguments.size();j+=2,m++)
        {
            CheckSubArgumentIsNan(ss,vSubArguments,j);
            CheckSubArgumentIsNan(ss,vSubArguments,j+1);
            ss <<"    if(isequal(";
            ss <<"tmp";
            ss <<j;
            ss <<" , ";
            ss << "tmp";
            ss << j+1;
            ss << ")){\n";
        }
        CheckSubArgumentIsNan(ss,vSubArguments,0);
        ss << "    tmp += tmp0;\n";
        for(size_t j=1;j<=vSubArguments.size();j+=2,m--)
        {
            for(int n = 0;n<m+1;n++)
            {
                ss << "    ";
            }
            ss<< "}\n";
        }
    }
    if (mNeedReductionKernel)
    {
        ss << "tmp =";
        vSubArguments[0]->GenDeclRef(ss);
        ss << "[gid0];\n";
    }
    ss << "return tmp;\n";
    ss << "}";
}
void OpCscH::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR= static_cast<const
          formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n\t";
    ss <<"int gid0=get_global_id(0);\n\t";
    ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
    ss<< "if(isnan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n\t\t";
    ss<<"arg0 = 0;\n\t";
    ss << "double tmp=1/sinh(arg0);\n\t";
    ss << "return tmp;\n";
    ss << "}";
}
void OpExp::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = 0.0f;\n";
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR=
                static_cast
                <const formula::SingleVectorRefToken *>(tmpCur);
            ss << "    arg0 = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(isnan(";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ")||(gid0>=";
            ss << tmpCurDVR->GetArrayLength();
            ss << "))\n";
            ss << "    { arg0 = 0.0f; }\n";
        }
        else if(tmpCur->GetType() == formula::svDouble)
        {
            ss << "    arg0=" << tmpCur->GetDouble() << ";\n";
        }
    }
    else
    {
        ss << "        arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    ss << "    return pow(M_E, arg0);\n";
    ss << "}";
}

void OpAverageIfs::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
         formula::DoubleVectorRefToken *>(tmpCur);
    size_t nCurWindowSize = pCurDVR->GetArrayLength() <
    pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength():
    pCurDVR->GetRefRowSize() ;
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n    {\n";
    ss <<"     int gid0=get_global_id(0);\n";
    ss << "    double tmp =0;\n";
    ss << "    int count=0;\n";
    ss << "    int loop;";
    GenTmpVariables(ss,vSubArguments);
    ss<< "    int singleIndex =gid0;\n";
    int m=0;
    std::stringstream tmpss;
    for(size_t j=1;j<vSubArguments.size();j+=2,m++)
    {
        CheckSubArgumentIsNan(tmpss,vSubArguments,j);
        CheckSubArgumentIsNan(ss,vSubArguments,j+1);
        tmpss <<"    if(isequal(";
        tmpss <<"tmp";
        tmpss <<j;
        tmpss <<" , ";
        tmpss << "tmp";
        tmpss << j+1;
        tmpss << ")){\n";
    }
    CheckSubArgumentIsNan(tmpss,vSubArguments,0);
    tmpss << "    tmp += tmp0;\n";
    tmpss << "    count++;\n";
    for(size_t j=1;j<vSubArguments.size();j+=2,m--)
     {
        for(int n = 0;n<m+1;n++)
        {
            tmpss << "    ";
        }
        tmpss<< "}\n";
     }

    UnrollDoubleVector(ss,tmpss,pCurDVR,nCurWindowSize);

    ss << "    if(count!=0)\n";
    ss << "        tmp=tmp/count;\n";
    ss << "    else\n";
    ss << "        tmp= 0 ;\n";
    ss << "return tmp;\n";
    ss << "}";
}

void OpLog10::GenSlidingWindowFunction(std::stringstream &ss,
               const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR= static_cast<const
          formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n\t";
    ss <<"int gid0=get_global_id(0);\n\t";
    ss << "double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n\t";
    ss<< "if(isnan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n\t\t";
    ss<<"arg0 = 0;\n\t";
    ss << "double tmp=log10(arg0);\n\t";
    ss << "return tmp;\n";
    ss << "}";
}

void OpSinh::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss <<") {\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss <<"    double arg0 = " <<
        vSubArguments[0]->GenSlidingWindowDeclRef();
    ss <<";\n";
    ss<< "    if(isnan(arg0))\n";
    ss<<"        arg0 = 0;\n";
    ss << "    double tmp=( exp(arg0)-exp(-arg0) )/2;\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpSin::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = 0.0f;\n";
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR=
                static_cast
                <const formula::SingleVectorRefToken *>(tmpCur);
            ss << "    arg0 = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(isnan(";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ")||(gid0>=";
            ss << tmpCurDVR->GetArrayLength();
            ss << "))\n";
            ss << "    { arg0 = 0.0f; }\n";
        }
        else if(tmpCur->GetType() == formula::svDouble)
        {
            ss << "    arg0=" << tmpCur->GetDouble() << ";\n";
        }
    }
    else
    {
        ss << "        arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    ss << "    arg0 = arg0 * M_1_PI;\n";
    ss << "    return sinpi(arg0);\n";
    ss << "}";
}

void OpAbs::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0   = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0=
        static_cast<const formula::SingleVectorRefToken *>(tmpCur0);
    ss << "    int buffer_len = ";
    ss << tmpCurDVR0->GetArrayLength();
    ss << ";\n";
    ss << "    if((gid0)>=buffer_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << "))\n";
    ss << "        tmp = " << GetBottom() << ";\n    else \n";
    ss << "        tmp = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    return fabs(tmp);\n";
    ss << "}";
}
void OpArcCos::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(atan2Decl);
    funs.insert(atan2Content);
}
void OpArcCos::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0=
        static_cast<const formula::SingleVectorRefToken *>(tmpCur0);
    ss << "    int buffer_len = "<< tmpCurDVR0->GetArrayLength()<< ";\n";
    ss << "    if((gid0)>=buffer_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef()<< "))\n";
    ss << "        tmp = " << GetBottom() << ";\n";
    ss << "    else \n    ";
    ss << "    tmp = ";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef()<< ";\n";
    ss << "    return arctan2(sqrt(1.0 - pow(tmp, 2)), tmp);\n";
    ss << "}";
}
void OpArcCosHyp::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0   = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur0);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur0->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR0=
                static_cast<const formula::SingleVectorRefToken *>(tmpCur0);
            ss << "    int buffer_len = "<<tmpCurDVR0->GetArrayLength()<<";\n";
            ss << "    if((gid0)>=buffer_len || isnan(";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
            ss << "        tmp = " << GetBottom() << ";\n";
            ss << "    else \n    ";
            ss << "    tmp = " << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
        }
        else if(tmpCur0->GetType() == formula::svDouble)
        {
            ss << "    tmp = " << tmpCur0->GetDouble() << ";\n";
        }
    }
    else
    {
        ss << "    tmp = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }

    ss << "    return  log( tmp + pow( (pown(tmp, 2) - 1.0), 0.5));\n";
    ss << "}";
}
void OpTan::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = 0.0f;\n";
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR=
                static_cast
                <const formula::SingleVectorRefToken *>(tmpCur);
            ss << "    arg0 = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(isnan(";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ")||(gid0>=";
            ss << tmpCurDVR->GetArrayLength();
            ss << "))\n";
            ss << "    { arg0 = 0.0f; }\n";
        }
        else if(tmpCur->GetType() == formula::svDouble)
        {
            ss << "    arg0=" << tmpCur->GetDouble() << ";\n";
        }
    }
    else
    {
        ss << "        arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    ss << "    arg0 = arg0 * M_1_PI;\n";
    ss << "    return sinpi(arg0) * pow(cospi(arg0), -1);\n";
    ss << "}";
}
void OpTanH::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR= static_cast<const
          formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = "<< vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    if(isnan(arg0)||(gid0>=";
    ss << tmpCurDVR->GetArrayLength();
    ss << "))\n";
    ss << "        arg0 = 0;\n";
    ss << "    double tmp=tanh(arg0);\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpPower::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg[2];\n";
    for( size_t i=0; i < vSubArguments.size(); ++i)
    {
        FormulaToken *tmpCur = vSubArguments[i]->GetFormulaToken();
        assert(tmpCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if(tmpCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* tmpCurDVR =
                    static_cast<
                    const formula::DoubleVectorRefToken *>(tmpCur);
                ss << "    int i = 0;\n";
                ss << "    arg["<<i<<"] = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ";\n";
                ss << "    if(isnan(arg["<<i;
                ss << "])||(gid0>=";
                ss << tmpCurDVR->GetArrayLength();
                ss << "))\n";
                ss << "        arg["<<i;
                ss << "] = 0;\n";
            }
            else if(tmpCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* tmpCurDVR=
                      static_cast<
                      const formula::SingleVectorRefToken *>(tmpCur);
                ss << "    arg["<<i<<"] = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ";\n";
                ss << "    if(isnan(arg["<<i;
                ss << "])||(gid0>=";
                ss << tmpCurDVR->GetArrayLength();
                ss << "))\n";
                ss << "        arg["<<i;
                ss << "] = 0;\n";
            }
            else if(tmpCur->GetType() == formula::svDouble)
            {
                ss << "        arg["<<i<<"] = ";
                ss << tmpCur->GetDouble() << ";\n";
            }
        }
        else
        {
            ss << "        arg["<<i<<"] = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
        }
    }
    ss << "    return pow(arg[0],arg[1]);\n";
    ss << "}";
}
void OpSqrt::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = 0.0f;\n";
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR=
                static_cast
                <const formula::SingleVectorRefToken *>(tmpCur);
            ss << "    arg0 = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(isnan(";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ")||(gid0>=";
            ss << tmpCurDVR->GetArrayLength();
            ss << "))\n";
            ss << "    { arg0 = 0; }\n";
        }
        else if(tmpCur->GetType() == formula::svDouble)
        {
            ss << "    arg0=";
            ss << tmpCur->GetDouble() << ";\n";
        }
        else
        {
            throw Unhandled( __FILE__, __LINE__ );
        }
    }
    else
    {
        ss << "        arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    ss << "    if( arg0 < 0 )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    return sqrt(arg0);\n";
    ss << "}";
}
void OpArcCot::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0   = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0=
        static_cast<const formula::SingleVectorRefToken *>(tmpCur0);
    ss << "    int buffer_len = " << tmpCurDVR0->GetArrayLength()<< ";\n";
    ss << "    if((gid0)>=buffer_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        tmp = " << GetBottom() << ";\n";
    ss << "    else \n    ";
    ss << "    tmp = " << vSubArguments[0]->GenSlidingWindowDeclRef() <<";\n";
    ss << "    return M_PI_2 - atan(tmp);\n";
    ss << "}";
}
void OpArcCotHyp::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = 0.0f;\n";
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR=
                static_cast
                <const formula::SingleVectorRefToken *>(tmpCur);
            ss << "    arg0 = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(isnan(";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ")||(gid0>=";
            ss << tmpCurDVR->GetArrayLength();
            ss << "))\n";
            ss << "    { arg0 = 0.0f; }\n";
        }
        else if(tmpCur->GetType() == formula::svDouble)
        {
            ss << "    arg0=";
            ss << tmpCur->GetDouble() << ";\n";
        }
    }
    else
    {
        ss << "        arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    ss << "    return 0.5 * log(1 + 2 * pown(arg0 - 1.0, -1));\n";
    ss << "}";
}
void OpArcSin::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(atan2Decl);
    funs.insert(atan2Content);
}
void OpArcSin::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0   = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0=
        static_cast<const formula::SingleVectorRefToken *>(tmpCur0);
    ss << "    int buffer_len = " << tmpCurDVR0->GetArrayLength() << ";\n";
    ss << "    if((gid0)>=buffer_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        tmp = " << GetBottom() << ";\n";
    ss << "    else \n    ";
    ss << "    tmp = " << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    ss << "    return arctan2(tmp, sqrt(1.0 - pow(tmp, 2)));\n";
    ss << "}";
}
void OpArcSinHyp::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0   = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur0);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur0->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR0=
                static_cast<const formula::SingleVectorRefToken *>(tmpCur0);
            ss << "    int buffer_len = "<<tmpCurDVR0->GetArrayLength()<<";\n";
            ss << "    if((gid0)>=buffer_len || isnan(";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
            ss << "        tmp = " << GetBottom() << ";\n";
            ss << "    else \n    ";
            ss << "    tmp = " << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
        }
        else if(tmpCur0->GetType() == formula::svDouble)
        {
            ss << "    tmp = " << tmpCur0->GetDouble() << ";\n";
        }
    }
    else
    {
        ss << "    tmp = " << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    ss << "    return  log( tmp + pow((pown(tmp, 2) + 1.0), 0.5));\n";
    ss << "}";
}
void OpArcTan2::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(atan2Decl);
    funs.insert(atan2Content);
}
void OpArcTan2::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double x_num = " << GetBottom() << ";\n";
    ss << "    double y_num = " << GetBottom() << ";\n";
    FormulaToken *iXNum = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVRX=
        static_cast<const formula::SingleVectorRefToken *>(iXNum);
    FormulaToken *iYNum = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVRY=
        static_cast<const formula::SingleVectorRefToken *>(iYNum);
    ss << "    int buffer_x_len = " << tmpCurDVRX->GetArrayLength() << ";\n";
    ss << "    int buffer_y_len = " << tmpCurDVRY->GetArrayLength() << ";\n";
    ss << "    if((gid0)>=buffer_x_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        x_num = " << GetBottom() << ";\n";
    ss << "    else \n    ";
    ss << "    x_num = "<< vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    ss << "    if((gid0)>=buffer_y_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        y_num = " << GetBottom() << ";\n";
    ss << "    else \n    ";
    ss << "    y_num = "<< vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
    ss << "    return arctan2(y_num, x_num);\n";
    ss << "}";
}
void OpArcTan::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = 0.0f;\n";
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR=
                static_cast
                <const formula::SingleVectorRefToken *>(tmpCur);
            ss << "    arg0 = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(isnan(";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ")||(gid0>=";
            ss << tmpCurDVR->GetArrayLength();
            ss << "))\n";
            ss << "    { arg0 = 0.0f; }\n";
        }
        else if(tmpCur->GetType() == formula::svDouble)
        {
            ss << "    arg0=" << tmpCur->GetDouble() << ";\n";
        }
    }
    else
    {
        ss << "        arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    ss << "    return atan(arg0);\n";
    ss << "}";
}
void OpArcTanH::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR0=
        static_cast<const formula::SingleVectorRefToken *>(tmpCur0);
    ss << "    int buffer_len = " << tmpCurDVR0->GetArrayLength() << ";\n";
    ss << "    if((gid0)>=buffer_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        tmp = " << GetBottom() << ";\n";
    ss << "    else \n    ";
    ss << "    tmp = " << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    ss << "    double a = 1.0 + tmp;\n";
    ss << "    double b = 1.0 - tmp;\n";
    ss << "    return log(pow(a/b, 0.5));\n";
    ss << "}";
}
void OpBitAnd::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double num1 = " << GetBottom() << ";\n";
    ss << "    double num2 = " << GetBottom() << ";\n";
    FormulaToken *iNum1 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVRNum1=
        static_cast<const formula::SingleVectorRefToken *>(iNum1);
    FormulaToken *iNum2 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVRNum2=
        static_cast<const formula::SingleVectorRefToken *>(iNum2);
    ss << "    int buffer_num1_len = "<<tmpCurDVRNum1->GetArrayLength()<<";\n";
    ss << "    int buffer_num2_len = "<<tmpCurDVRNum2->GetArrayLength()<<";\n";
    ss << "    if((gid0)>=buffer_num1_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        num1 = " << GetBottom() << ";\n";
    ss << "    else \n    ";
    ss << "    num1 = " << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    ss << "    if((gid0)>=buffer_num2_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        num2 = " << GetBottom() << ";\n";
    ss << "    else \n    ";
    ss << "    num2 = " << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
    ss << "    return (long)num1 & (long)num2;\n";
    ss << "}";
}
void OpLn::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    int singleIndex =  gid0;\n";

    GenTmpVariables(ss,vSubArguments);
    CheckAllSubArgumentIsNan(ss,vSubArguments);

    ss << "    double tmp=log1p(tmp0-1);\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpRound::GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 2 );
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    int singleIndex =  gid0;\n";
    GenTmpVariables(ss,vSubArguments);
    CheckAllSubArgumentIsNan(ss,vSubArguments);
    if(vSubArguments.size() ==2)
    {
        ss << "    for(int i=0;i<tmp1;i++)\n";
        ss << "        tmp0 = tmp0 * 10;\n";
        ss << "    for(int i=0;i>tmp1;i--)\n";
        ss << "        tmp0 = tmp0 / 10;\n";
    }
    ss << "    double tmp=round(tmp0);\n";
    if(vSubArguments.size() ==2)
    {
        ss << "    for(int i=0;i<tmp1;i++)\n";
        ss << "        tmp = tmp / 10;\n";
        ss << "    for(int i=0;i>tmp1;i--)\n";
        ss << "        tmp = tmp * 10;\n";
    }
    ss << "    return tmp;\n";
    ss << "}";
}
void OpRoundUp::GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    int singleIndex =  gid0;\n";
    ss << "    int intTmp;\n";
    ss << "    double doubleTmp;\n";
    ss << "    double tmp;\n";
    GenTmpVariables(ss,vSubArguments);
    CheckAllSubArgumentIsNan(ss,vSubArguments);
    ss << "    if(tmp1 >20 || tmp1 < -20)";
    ss << "    {\n";
    ss << "        tmp = NAN;\n";
    ss << "    }else\n";
    ss << "    {\n";
    ss << "        for(int i=0;i<tmp1;i++)\n";
    ss << "            tmp0 = tmp0 * 10;\n";
    ss << "        intTmp = (int)tmp0;\n";
    ss << "        doubleTmp = intTmp;\n";
    ss << "        if(isequal(doubleTmp,tmp0))\n";
    ss << "            tmp = doubleTmp;\n";
    ss << "        else\n";
    ss << "            tmp = doubleTmp + 1;\n";
    ss << "        for(int i=0;i<tmp1;i++)\n";
    ss << "            tmp = tmp / 10;\n";
    ss << "    }\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpRoundDown::GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    int singleIndex =  gid0;\n";
    ss << "    int intTmp;\n";
    ss << "    double tmp;\n";
    GenTmpVariables(ss,vSubArguments);
    CheckAllSubArgumentIsNan(ss,vSubArguments);
    ss << "    if(tmp1 >20 || tmp1 < -20)";
    ss << "    {\n";
    ss << "        tmp = NAN;\n";
    ss << "    }else\n";
    ss << "    {\n";
    ss << "        for(int i=0;i<tmp1;i++)\n";
    ss << "            tmp0 = tmp0 * 10;\n";
    ss << "        intTmp = (int)tmp0;\n";
    ss << "        tmp = intTmp;\n";
    ss << "        for(int i=0;i<tmp1;i++)\n";
    ss << "            tmp = tmp / 10;\n";
    ss << "    }\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpInt::GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    int singleIndex =  gid0;\n";
    ss << "    int intTmp;\n";
    ss << "    double tmp;\n";
    GenTmpVariables(ss,vSubArguments);
    CheckAllSubArgumentIsNan(ss,vSubArguments);
    ss << "    intTmp = (int)tmp0;\n";
    ss << "    tmp = intTmp;\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpNegSub::GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    int singleIndex =  gid0;\n";
    GenTmpVariables(ss,vSubArguments);
    CheckAllSubArgumentIsNan(ss,vSubArguments);
    ss << "    return -tmp0;\n";
    ss << "}";
}

void OpRadians::GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    int singleIndex =  gid0;\n";
    ss << "    double tmp;\n";
    GenTmpVariables(ss,vSubArguments);
    CheckAllSubArgumentIsNan(ss,vSubArguments);
    ss << "    tmp = tmp0 * 3.14159265358979 * pow(180.0,-1);\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpIsEven::GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    int singleIndex =  gid0;\n";
    ss << "    double tmp;\n";
    GenTmpVariables(ss,vSubArguments);
    CheckAllSubArgumentIsNan(ss,vSubArguments);
    ss << "    tmp = (fmod(floor(fabs(tmp0)), 2.0)<0.5);\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpIsOdd::GenSlidingWindowFunction(std::stringstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    int singleIndex =  gid0;\n";
    ss << "    double tmp;\n";
    GenTmpVariables(ss,vSubArguments);
    CheckAllSubArgumentIsNan(ss,vSubArguments);
    ss << "    tmp = !(fmod(floor(fabs(tmp0)), 2.0)<0.5);\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpOdd::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName,
    SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur);
    ss << Math_Intg_Str;
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss << "    double tmp=0;\n";
    ss << "    double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss<< "    if(isnan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n";
    ss<<"        arg0 = 0;\n";
    ss << "    if (arg0 > 0.0 ){\n";
    ss << "        tmp=Intg(arg0);\n";
    ss << "        if(tmp-trunc(tmp/2)*2 == 0)\n";
    ss << "            tmp=tmp+1;\n";
    ss << "    }else if (arg0 < 0.0 ){\n";
    ss << "        tmp=Intg(arg0);\n";
    ss << "        if(tmp-trunc(tmp/2)*2 == 0)\n";
    ss << "            tmp=tmp-1.0;\n";
    ss << "    }else if (arg0 == 0.0 )\n";
    ss << "            tmp=1.0;\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpCountIf::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < 2; i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double vara, varb;\n";
    ss << "    int varc = 0;\n";
    FormulaToken *tmpCur = vSubArguments[1]->GetFormulaToken();
    assert(tmpCur);
    if(ocPush == vSubArguments[1]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* tmpCurDVR=
                static_cast<
                const formula::SingleVectorRefToken *>(tmpCur);
            ss << "    varb = ";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(isnan(varb)||(gid0>=";
            ss << tmpCurDVR->GetArrayLength();
            ss << "))\n";
            ss << "        varb = 0;\n";
        }
        else if(tmpCur->GetType() == formula::svDouble)
        {
            ss << "    varb = ";
            ss << tmpCur->GetDouble() << ";\n";
        }
    }
    else
    {
        ss << "    varb = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    tmpCur = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        //TODO       DoubleVector
        if (tmpCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(tmpCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
            {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; ++i)\n";
                ss << "    {\n";
            }
            else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; ++i)\n";
                ss << "    {\n";
            }
            else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; ++i)\n";
                ss << "    {\n";
            }
            else
            {
                ss << "0; i < "<< nCurWindowSize << "; ++i)\n";
                ss << "    {\n";
            }
            ss << "        vara = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "        if (isnan(vara))\n";
            ss << "            continue;\n";
            ss << "        (vara == varb) && varc++;\n";
            ss << "    }\n";
        }
        else if(tmpCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* tmpCurDVR=
                static_cast<
                const formula::SingleVectorRefToken *>(tmpCur);
            ss << "    vara = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(isnan(vara)||(gid0>=";
            ss << tmpCurDVR->GetArrayLength();
            ss << "))\n";
            ss << "        return 0;\n";
            ss << "    (vara == varb) && varc++;\n";
        }
    }
    ss << "    return varc;\n";
    ss << "}";
}
void OpSumIf::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    int flag = 3 == vSubArguments.size() ? 2 : 0;
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double vara, varb, varc, sum = 0.0f;\n";
    FormulaToken *tmpCur = vSubArguments[1]->GetFormulaToken();
    assert(tmpCur);
    if(ocPush == vSubArguments[1]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* tmpCurDVR=
                static_cast<
                const formula::SingleVectorRefToken *>(tmpCur);
            ss << "    varb = ";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(isnan(varb)||(gid0>=";
            ss << tmpCurDVR->GetArrayLength();
            ss << "))\n";
            ss << "        varb = 0;\n";
        }
        else if(tmpCur->GetType() == formula::svDouble)
        {
            ss << "    varb = ";
            ss << tmpCur->GetDouble() << ";\n";
        }
    }
    else
    {
        ss << "    varb = ";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    tmpCur = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        //TODO       DoubleVector
        if (tmpCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(tmpCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
            {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; ++i)\n";
                ss << "    {\n";
            }
            else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; ++i)\n";
                ss << "    {\n";
            }
            else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; ++i)\n";
                ss << "    {\n";
            }
            else
            {
                ss << "0; i < "<< nCurWindowSize << "; ++i)\n";
                ss << "    {\n";
            }
            ss << "        vara = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "        if (isnan(vara))\n";
            ss << "            continue;\n";
            ss << "        varc = ";
            ss << vSubArguments[flag]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "        if (isnan(varc))\n";
            ss << "            varc = 0.0f;\n";
            ss << "        (vara == varb)&&(sum = sum + varc);\n";
            ss << "    }\n";
        }
        else if(tmpCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* tmpCurDVR=
                static_cast<
                const formula::SingleVectorRefToken *>(tmpCur);
            ss << "    vara = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(isnan(vara)||(gid0>=";
            ss << tmpCurDVR->GetArrayLength();
            ss << "))\n";
            ss << "        return 0;\n";
            ss << "    int i = 0;\n";
            ss << "    varc = ";
            ss << vSubArguments[flag]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(isnan(varc)||(gid0>=";
            ss << tmpCurDVR->GetArrayLength();
            ss << "))\n";
            ss << "        varc = 0.0f;\n";

            ss << "        (vara == varb)&&(sum = sum + varc);\n";

        }
    }
    ss << "    return sum;\n";
    ss << "}";
}
void OpTrunc::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg[2];\n";
    for( size_t i=0; i < vSubArguments.size(); ++i)
    {
        FormulaToken *tmpCur = vSubArguments[i]->GetFormulaToken();
        assert(tmpCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if(tmpCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* tmpCurDVR =
                    static_cast<
                    const formula::DoubleVectorRefToken *>(tmpCur);
                ss << "    int i = 0;\n";
                ss << "    arg["<<i<<"] = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ";\n";
                ss << "    if(isnan(arg["<<i;
                ss << "])||(gid0>=";
                ss << tmpCurDVR->GetArrayLength();
                ss << "))\n";
                ss << "        arg["<<i;
                ss << "] = 0;\n";
            }
            else if(tmpCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* tmpCurDVR=
                      static_cast<
                      const formula::SingleVectorRefToken *>(tmpCur);
                ss << "    arg["<<i<<"] = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ";\n";
                ss << "    if(isnan(arg["<<i;
                ss << "])||(gid0>=";
                ss << tmpCurDVR->GetArrayLength();
                ss << "))\n";
                ss << "        arg["<<i;
                ss << "] = 0;\n";
            }
            else if(tmpCur->GetType() == formula::svDouble)
            {
                ss << "        arg["<<i<<"] = ";
                ss << tmpCur->GetDouble() << ";\n";
            }
        }
        else
        {
            ss << "        arg["<<i<<"] = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
        }
    }
    ss << "    double argm = arg[0];\n";
    ss << "    int n = (int)arg[1];\n";
    ss << "    double nn = 1.0f;\n";
    ss << "    for(int i = 0; i < n; ++i)\n";
    ss << "    {\n";
    ss << "        argm = argm * 10;\n";
    ss << "        nn = nn * 10;\n";
    ss << "    }\n";
    ss << "    modf(argm, &argm);\n";
    ss << "    return argm / nn;\n";
    ss << "}";
}
void OpFloor::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName,
    SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0,arg1,arg2=0.0;\n";
    ss << "    arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    arg1 = " << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
    if ( 3 == vSubArguments.size() )
    {
        ss << "    arg2 = " << vSubArguments[2]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    ss << "    if(isnan(arg0) || isnan(arg1))\n";
    ss << "        return 0;\n";
    ss << "    if(isnan(arg2))\n";
    ss << "        arg2 = 0.0;\n";
    ss << "    if(arg0*arg1<0)\n";
    ss << "        return NAN;\n";
    ss << "    else if(arg2==0.0&&arg0<0.0)\n";
    ss << "        return (trunc(arg0/arg1)+1)*arg1;\n";
    ss << "    else\n";
    ss << "        return trunc(arg0/arg1)*arg1;\n";
    ss << "}\n";
}
void OpBitOr::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0   = get_global_id(0);\n";
    ss << "    double num1 = " << GetBottom() << ";\n";
    ss << "    double num2 = " << GetBottom() << ";\n";
    FormulaToken *iNum1 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVRNum1=
        static_cast<const formula::SingleVectorRefToken *>(iNum1);
    FormulaToken *iNum2 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVRNum2=
        static_cast<const formula::SingleVectorRefToken *>(iNum2);
    ss << "    int buffer_num1_len = "<<tmpCurDVRNum1->GetArrayLength()<<";\n";
    ss << "    int buffer_num2_len = "<<tmpCurDVRNum2->GetArrayLength()<<";\n";
    ss << "    if((gid0)>=buffer_num1_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        num1 = " << GetBottom() << ";\n";
    ss << "    else \n    ";
    ss << "    num1 = floor(" << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ");\n";
    ss << "    if((gid0)>=buffer_num2_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        num2 = " << GetBottom() << ";\n";
    ss << "    else\n    ";
    ss << "    num2 = floor(" << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ");\n";
    ss << "    return (long)num1 | (long)num2;\n";
    ss << "}";
}
void OpBitXor::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double num1 = " << GetBottom() << ";\n";
    ss << "    double num2 = " << GetBottom() << ";\n";
    FormulaToken *iNum1 = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVRNum1=
        static_cast<const formula::SingleVectorRefToken *>(iNum1);
    FormulaToken *iNum2 = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVRNum2=
        static_cast<const formula::SingleVectorRefToken *>(iNum2);
    ss << "    int buffer_num1_len = " << tmpCurDVRNum1->GetArrayLength() << ";\n";
    ss << "    int buffer_num2_len = " << tmpCurDVRNum2->GetArrayLength() << ";\n";

    ss << "    if((gid0)>=buffer_num1_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        num1 = " << GetBottom() << ";\n";
    ss << "    else\n    ";
    ss << "    num1 = floor(" << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ");\n";
    ss << "    if((gid0)>=buffer_num2_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        num2 = " << GetBottom() << ";\n";
    ss << "    else\n    ";
    ss << "    num2 = floor(" << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ");\n";
    ss << "    return (long)num1 ^ (long)num2;\n";
    ss << "}";
}
void OpBitLshift::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double num = " << GetBottom() << ";\n";
    ss << "    double shift_amount = " << GetBottom() << ";\n";
    FormulaToken *iNum = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVRNum=
        static_cast<const formula::SingleVectorRefToken*>(iNum);
    FormulaToken *iShiftAmount = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVRShiftAmount=
        static_cast<const formula::SingleVectorRefToken*>(iShiftAmount);
    ss << "    int buffer_num_len = "<< tmpCurDVRNum->GetArrayLength()<<";\n";
    ss << "    int buffer_shift_amount_len = ";
    ss << tmpCurDVRShiftAmount->GetArrayLength() << ";\n";
    ss << "    if((gid0)>=buffer_num_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        num = " << GetBottom() << ";\n";
    ss << "    else\n    ";
    ss << "    num = floor(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ");\n";
    ss << "    if((gid0)>=buffer_shift_amount_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        shift_amount = " << GetBottom() << ";\n";
    ss << "    else\n    ";
    ss << "    shift_amount = floor(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ");\n";
    ss << "    return floor(shift_amount >= 0 ? ";
    ss << "num * pow(2.0, shift_amount) : ";
    ss << "num / pow(2.0, fabs(shift_amount)));\n";
    ss << "}";
}
void OpBitRshift::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double num = " << GetBottom() << ";\n";
    ss << "    double shift_amount = " << GetBottom() << ";\n";
    FormulaToken *iNum = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVRNum=
        static_cast<const formula::SingleVectorRefToken*>(iNum);
    FormulaToken *iShiftAmount = vSubArguments[1]->GetFormulaToken();
    const formula::SingleVectorRefToken* tmpCurDVRShiftAmount=
        static_cast<const formula::SingleVectorRefToken*>(iShiftAmount);
    ss << "    int buffer_num_len = ";
    ss << tmpCurDVRNum->GetArrayLength() << ";\n";
    ss << "    int buffer_shift_amount_len = ";
    ss << tmpCurDVRShiftAmount->GetArrayLength() << ";\n";

    ss << "    if((gid0)>=buffer_num_len || isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        num = " << GetBottom() << ";\n";
    ss << "    else\n    ";
    ss << "    num = floor(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ");\n";
    ss << "    if((gid0)>=buffer_shift_amount_len || isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        shift_amount = " <<GetBottom()<< ";\n";
    ss << "    else\n    ";
    ss << "    shift_amount = floor(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ");\n";
    ss << "    return floor(";
    ss << "shift_amount >= 0 ? num / pow(2.0, shift_amount) : ";
    ss << "num * pow(2.0, fabs(shift_amount)));\n";
    ss << "}";
}
void OpSumSQ::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); ++i)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double sum = 0.0f, arg;\n";
    for(DynamicKernelArgumentRef & rArg : vSubArguments)
    {
        FormulaToken *tmpCur = rArg->GetFormulaToken();
        assert(tmpCur);
        if(ocPush == rArg->GetFormulaToken()->GetOpCode())
        {
            if (tmpCur->GetType() == formula::svDoubleVectorRef)
            {
                const formula::DoubleVectorRefToken* pDVR =
                    static_cast<const formula::DoubleVectorRefToken *>(tmpCur);
                size_t nCurWindowSize = pDVR->GetRefRowSize();
                ss << "    for (int i = ";
                if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
                {
                    ss << "gid0; i < " << pDVR->GetArrayLength();
                    ss << " && i < " << nCurWindowSize  << "; ++i)\n";
                    ss << "    {\n";
                }
                else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i < " << pDVR->GetArrayLength();
                    ss << " && i < gid0+"<< nCurWindowSize << "; ++i)\n";
                    ss << "    {\n";
                }
                else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
                {
                    ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                    ss << " &&  i < "<< nCurWindowSize << "; ++i)\n";
                    ss << "    {\n";
                }
                else
                {
                    ss << "0; i < "<< nCurWindowSize << "; ++i)\n";
                    ss << "    {\n";
                }
                ss << "        arg = ";
                ss << rArg->GenSlidingWindowDeclRef();
                ss << ";\n";
                ss << "        if (isnan(arg))\n";
                ss << "            continue;\n";
                ss << "        sum += pown(arg, 2);\n";
                ss << "    }\n";
            }
            else if(tmpCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* tmpCurDVR=
                      static_cast<
                      const formula::SingleVectorRefToken *>(tmpCur);
                ss << "    arg = ";
                ss << rArg->GenSlidingWindowDeclRef();
                ss << ";\n";
                ss << "    if(isnan(arg)||(gid0>=";
                ss << tmpCurDVR->GetArrayLength();
                ss << "))\n";
                ss << "        arg = 0.0f;\n";
                ss << "    sum += pown(arg, 2);\n";
            }
            else if(tmpCur->GetType() == formula::svDouble)
            {
                ss << "        arg = ";
                ss << tmpCur->GetDouble() << ";\n";
                ss << "        sum += pown(arg, 2);\n";
            }
        }
        else
        {
            ss << "        arg = ";
            ss << rArg->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "        sum += pown(arg, 2);\n";
        }
    }
    ss << "    return sum;\n";
    ss << "}";
}
void OpSqrtPi::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = 0.0f;\n";
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR=
                static_cast
                <const formula::SingleVectorRefToken *>(tmpCur);
            ss << "    arg0 = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(isnan(";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ")||(gid0>=";
            ss << tmpCurDVR->GetArrayLength();
            ss << "))\n";
            ss << "    { arg0 = 0.0f; }\n";
        }
        else if(tmpCur->GetType() == formula::svDouble)
        {
            ss << "    arg0=";
            ss << tmpCur->GetDouble() << ";\n";
        }
    }
    else
    {
        ss << "        arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    ss << "    return (double)sqrt(arg0 *";
    ss << " 3.1415926535897932);\n";
    ss << "}";
}
void OpCeil::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0   = get_global_id(0);\n";
    ss << "    double num = " << GetBottom() << ";\n";
    ss << "    double significance = " << GetBottom() << ";\n";
    ss << "    double bAbs = 0;\n";
    ss << "    if(isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        num = " << GetBottom() << ";\n";
    ss << "    else\n    ";
    ss << "    num = " << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    ss << "    if(isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        return 0.0;\n";
    ss << "    else\n    ";
    ss << "    significance = ";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
    if (vSubArguments.size() > 2)
    {
        FormulaToken *bAbs = vSubArguments[2]->GetFormulaToken();
        if(bAbs->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* tmpCurSVRIsAbs=
                static_cast<const formula::SingleVectorRefToken*>(bAbs);
            ss<< "    if((gid0)>=" << tmpCurSVRIsAbs->GetArrayLength() << " ||";
        }
        if(bAbs->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* tmpCurDVRIsAbs=
                static_cast<const formula::DoubleVectorRefToken*>(bAbs);
            ss<< "    if((gid0)>=" << tmpCurDVRIsAbs->GetArrayLength() << " ||";
        }
        if(bAbs->GetType() == formula::svDouble)
        {
            ss<< "    if(";
        }
        ss << "isnan(";
        ss << vSubArguments[2]->GenSlidingWindowDeclRef() << "))\n";
        ss << "        bAbs = 0;\n";
        ss << "    else\n    ";
        ss << "    bAbs = "<<vSubArguments[2]->GenSlidingWindowDeclRef()<<";\n";
    }
    ss << "    if(significance == 0.0)\n";
    ss << "        return 0.0;\n";
    ss << "    return ";
    ss << "( !(int)bAbs && num < 0.0 ? floor( num / significance ) : ";
    ss << "ceil( num / significance ) )";
    ss << "*significance;\n";
    ss << "}";
}
void OpCombin::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double num = " << GetBottom() << ";\n";
    ss << "    double num_chosen = " << GetBottom() << ";\n";
    ss << "    double result = -1.0;\n";
    FormulaToken *iNum = vSubArguments[0]->GetFormulaToken();
    FormulaToken *iNumChosen = vSubArguments[1]->GetFormulaToken();

    assert(iNum);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(iNum->GetType() == formula::svSingleVectorRef &&
            iNumChosen->GetType() == formula::svSingleVectorRef)
        {
            ss << "    if(isnan(";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
            ss << "        num = " << GetBottom() << ";\n";
            ss << "    else\n    ";
            ss << "    num = floor(";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ");\n";
            ss << "    if(isnan(";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef() << "))\n";
            ss << "        num_chosen = " << GetBottom() << ";\n";
            ss << "    else\n    ";
            ss << "    num_chosen = floor(";
            ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ");\n";
        }
        else if(iNum->GetType() == formula::svDouble &&
            iNumChosen->GetType() == formula::svDouble)
        {
            ss << "    num = floor(" << iNum->GetDouble() << ");\n";
            ss << "    num_chosen = floor("<< iNumChosen->GetDouble()<< ");\n";
        }
    }
    else
    {
        ss << "    num = floor(";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ");\n";
        ss << "    num_chosen = floor(";
        ss << vSubArguments[1]->GenSlidingWindowDeclRef() << ");\n";
    }
    ss << "    result = select(result, 0.0, (ulong)(num < num_chosen));\n";
    ss << "    result = select(result, 1.0, (ulong)(num_chosen == 0.0));\n";
    ss << "    if(result == 0 || result ==1)\n";
    ss << "        return result;\n";
    ss << "    double4 db4num;\n";
    ss << "    double4 db4num_chosen;\n";
    ss << "    double4 db4result;\n";
    ss << "    double2 db2result;\n";
    ss << "    result = 1.0;\n";
    ss << "    int loop = num_chosen/4;\n";
    ss << "    for(int i=0; i<loop; i++)\n";
    ss << "    {\n";
    ss << "        db4num = (double4){num,\n";
    ss << "            num-1.0,\n";
    ss << "            num-2.0,\n";
    ss << "            num-3.0};\n";
    ss << "        db4num_chosen = (double4){num_chosen,\n";
    ss << "            num_chosen-1.0,\n";
    ss << "            num_chosen-2.0,\n";
    ss << "            num_chosen-3.0};\n";
    ss << "        db4result = db4num * pown(db4num_chosen, -1);\n";
    ss << "        db2result = db4result.xy * db4result.zw;\n";
    ss << "        result *=  db2result.x * db2result.y;\n";
    ss << "        num = num - 4.0;\n";
    ss << "        num_chosen = num_chosen - 4.0;\n";
    ss << "    }\n";
    ss << "    while ( num_chosen > 0){\n";
    ss << "        result *= num / num_chosen;\n";
    ss << "        num = num - 1.0;\n";
    ss << "        num_chosen = num_chosen - 1.0;\n";
    ss << "    }\n";
    ss << "    return result;\n";
    ss << "}\n";
}
void OpConvert::GenSlidingWindowFunction(
    std::stringstream &ss, const std::string &sSymName,
    SubArguments &vSubArguments)
{
    // The CONVERT function converts a value from one unit of
    // measurement to another. It takes the units of measurements to
    // convert between as string arguments. This implementation
    // handles just a very small subset of such conversions.

    int arg1=vSubArguments[1]->GetFormulaToken()->GetString().
        getString().toAsciiUpperCase().hashCode();
    int arg2=vSubArguments[2]->GetFormulaToken()->GetString().
        getString().toAsciiUpperCase().hashCode();

    // Check if the from and to units are those combinations that the
    // code below supports.
    if( !((arg1==5584&&arg2==108)||
        (arg1==108&&arg2==5584)||
        (arg1==5665&&arg2==268206)||
        (arg1==268206&&arg2==5665)) )
        throw Unhandled(__FILE__, __LINE__);

    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::SingleVectorRefToken*tmpCurDVR= static_cast<const
        formula::SingleVectorRefToken *>(tmpCur);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = " << vSubArguments[0]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    double arg1 = " << vSubArguments[1]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss << "    double arg2 = " << vSubArguments[2]->GenSlidingWindowDeclRef();
    ss << ";\n";
    ss<< "    if(isnan(arg0)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n";
    ss<<"        arg0 = 0;\n";
    ss<< "    if(isnan(arg1)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n";
    ss<<"        arg1 = 0;\n";
    ss<< "    if(isnan(arg2)||(gid0>=";
    ss<<tmpCurDVR->GetArrayLength();
    ss<<"))\n";
    ss<<"        arg2 = 0;\n";
    ss<<"    if(arg1==5584U&&arg2==108U)\n";
    ss<<"        return arg0*1000.0;\n";
    ss<<"    else if(arg1==108U&&arg2==3385U)\n";
    ss<<"        return arg0/1000.0;\n";
    ss<<"    else if(arg1==5665U&&arg2==268206U)\n";
    ss<<"        return arg0*60.0;\n";
    ss<<"    else if(arg1==268206U&&arg2==5665U)\n";
    ss<<"        return arg0/60.0;\n";
    ss<<"    else\n";
    ss<<"        return -9999999999;\n";
    ss << "}\n";

}

void OpProduct::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"( ";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    int i = 0;\n";
    ss << "    double product=1.0;\n";
    ss << "    int count = 0;\n\n";
    for (DynamicKernelArgumentRef & rArg : vSubArguments)
    {
        FormulaToken *pCur = rArg->GetFormulaToken();
        assert(pCur);
        if (pCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
            static_cast<const formula::DoubleVectorRefToken *>(pCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();

            ss << "    for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
            {
                 ss << "gid0; i < " << pDVR->GetArrayLength();
                 ss << " && i < " << nCurWindowSize  << "; i++)\n";
                 ss << "    {\n";
            }
            else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                 ss << "0; i < " << pDVR->GetArrayLength();
                 ss << " && i < gid0+" << nCurWindowSize << "; i++)\n";
                 ss << "    {\n";
            }
            else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                 ss << "0; i + gid0 < " <<  pDVR->GetArrayLength();
                 ss << " &&  i < " << nCurWindowSize << "; i++)\n";
                 ss << "    {\n";
            }
            else if (pDVR->IsStartFixed() && pDVR->IsEndFixed())
            {

                 ss << "0; i < " << pDVR->GetArrayLength() << "; i++)\n";
                 ss << "    {\n";
            }
            ss << "        if(!isnan("<<rArg->GenSlidingWindowDeclRef()<<"))\n";
            ss << "        {\n";
            ss << "            product = product*";
            ss << rArg->GenSlidingWindowDeclRef()<<";\n";
            ss << "            ++count;\n";
            ss << "        }\n";
            ss << "    }\n";
        }
        else if (pCur->GetType() == formula::svSingleVectorRef)
        {
            ss << "    if(!isnan("<<rArg->GenSlidingWindowDeclRef()<<"))\n";
            ss << "    {\n";
            ss << "        product = product*";
            ss << rArg->GenSlidingWindowDeclRef()<<";\n";
            ss << "        ++count;\n";
            ss << "    }\n";
        }
        else
        {
            ss << "    if(!isnan("<<rArg->GenSlidingWindowDeclRef()<<"))\n";
            ss << "    {\n";
            ss << "        product = product*";
            ss << rArg->GenSlidingWindowDeclRef()<<";\n";
            ss << "        ++count;\n";
            ss << "    }\n";
        }
    }
    ss << "    if(count == 0)\n";
    ss << "        return 0;\n";
    ss << "    return product;\n";
    ss << "}";
}
void OpAverageIf::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{

    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp =0;\n";
    ss << "    double count=0;\n";
    ss << "    int singleIndex =gid0;\n";
    ss << "    int doubleIndex;\n";
    ss << "    int i ;\n";
    ss << "    int j ;\n";
    GenTmpVariables(ss,vSubArguments);

    unsigned paraOneIsDoubleVector = 0;
    unsigned paraOneWidth = 1;
    unsigned paraTwoWidth = 1;
    unsigned loopTimes = 0;

    if(vSubArguments[0]->GetFormulaToken()->GetType() == formula::svDoubleVectorRef)
    {
        paraOneIsDoubleVector = 1;
        FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR0= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur0);
        paraOneWidth = pCurDVR0->GetArrays().size();
        loopTimes = paraOneWidth;
        if(paraOneWidth > 1)
        {
            throw Unhandled(__FILE__, __LINE__);
        }
    }

    if(vSubArguments[paraOneWidth]->GetFormulaToken()->GetType() ==
     formula::svDoubleVectorRef)

    {
        FormulaToken *tmpCur1 = vSubArguments[1]->GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR1= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur1);
        paraTwoWidth = pCurDVR1->GetArrays().size();
        if(paraTwoWidth > 1)
        {
            throw Unhandled(__FILE__, __LINE__);
        }
        ss << "    i = ";
        if (!pCurDVR1->IsStartFixed() && pCurDVR1->IsEndFixed()) {
            ss << "gid0;\n";
        } else {
            ss << "0;\n";
        }
        if(!pCurDVR1->IsStartFixed() && !pCurDVR1->IsEndFixed())
        {
            ss << "        doubleIndex =i+gid0;\n";
        }else
        {
            ss << "        doubleIndex =i;\n";
        }
    }

    CheckSubArgumentIsNan(ss,vSubArguments,paraOneWidth);

    unsigned paraThreeIndex = paraOneWidth + paraTwoWidth;
    if(vSubArguments.size() > paraThreeIndex)
    {
        if(vSubArguments[paraThreeIndex]->GetFormulaToken()->GetType() ==
        formula::svDoubleVectorRef)
        {
            FormulaToken *tmpCur2 =
            vSubArguments[paraThreeIndex]->GetFormulaToken();
            const formula::DoubleVectorRefToken*pCurDVR2= static_cast<const
                formula::DoubleVectorRefToken *>(tmpCur2);
            unsigned paraThreeWidth = pCurDVR2->GetArrays().size();
            if(paraThreeWidth > 1)
            {
                throw Unhandled(__FILE__, __LINE__);
            }
        }
    }

    if(paraOneIsDoubleVector)
    {
        unsigned loopIndex = 0;
        FormulaToken *tmpCur0 = vSubArguments[0]->GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR0= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur0);
        size_t nCurWindowSize = pCurDVR0->GetArrayLength() <
            pCurDVR0->GetRefRowSize() ? pCurDVR0->GetArrayLength():
            pCurDVR0->GetRefRowSize() ;

        for(loopIndex =0; loopIndex < loopTimes; loopIndex++)
        {
            ss << "    for (i = ";
            if (!pCurDVR0->IsStartFixed() && pCurDVR0->IsEndFixed()) {
                ss << "gid0; i < "<< nCurWindowSize <<"; i++)\n";
            } else if (pCurDVR0->IsStartFixed() && !pCurDVR0->IsEndFixed()) {
                ss << "0; i < gid0+"<< nCurWindowSize <<"; i++)\n";
            } else {
                ss << "0; i < "<< nCurWindowSize <<"; i++)\n";
            }
            ss << "    {\n";
            if(!pCurDVR0->IsStartFixed() && !pCurDVR0->IsEndFixed())
            {
                ss << "        doubleIndex =i+gid0;\n";
            }else
            {
                ss << "        doubleIndex =i;\n";
            }

            CheckSubArgumentIsNan(ss,vSubArguments, loopIndex);

            ss << "        if ( isequal( tmp";
            ss << loopIndex<<" , tmp"<<paraOneWidth<<") ) \n";
            ss << "        {\n";
            if(vSubArguments.size() == paraThreeIndex)
                ss << "            tmp += tmp"<<loopIndex<<";\n";
            else
            {
                CheckSubArgumentIsNan(ss,vSubArguments,
                paraThreeIndex+loopIndex);
                ss << "            tmp += tmp";
                ss << paraThreeIndex+loopIndex<<";\n";
            }
            ss << "            count+=1.0;\n";
            ss << "        }\n";
            ss << "    }\n";
        }
    }
    else
    {
        CheckSubArgumentIsNan(ss,vSubArguments, 0);
        ss << "        if ( isequal( tmp0 , tmp1 ) ) \n";
        ss << "        {\n";
        if(vSubArguments.size() == 2)
            ss << "            tmp += tmp0;\n";
        else
        {
            CheckSubArgumentIsNan(ss,vSubArguments,2);
            ss << "            tmp += tmp2;\n";
        }
        ss << "            count+=1.0;\n";
        ss << "        }\n";
    }

    ss << "    if(count!=0)\n";
    ss << "        tmp=tmp/count;\n";
    ss << "    else\n";
    ss << "        tmp= 0 ;\n";
    ss << "    return tmp;\n";
    ss << "}";
}
void OpDeg::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double arg0 = 0.0f;\n";
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    assert(tmpCur);
    if(ocPush == vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        if(tmpCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken*tmpCurDVR=
                static_cast
                <const formula::SingleVectorRefToken *>(tmpCur);
            ss << "    arg0 = ";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(isnan(";
            ss << vSubArguments[0]->GenSlidingWindowDeclRef();
            ss << ")||(gid0>=";
            ss << tmpCurDVR->GetArrayLength();
            ss << "))\n";
            ss << "    { arg0 = 0.0f; }\n";
        }
        else if(tmpCur->GetType() == formula::svDouble)
        {
            ss << "    arg0=";
            ss << tmpCur->GetDouble() << ";\n";
        }
    }
    else
    {
        ss << "        arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << ";\n";
    }
    ss << "    return arg0 * pown(M_PI, -1) * 180;;\n";
    ss << "}";
}

void OpFact::GenSlidingWindowFunction(std::stringstream& ss,
    const std::string &sSymName, SubArguments& vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_" << BinFuncName() << "(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    double tmp = " << GetBottom() << ";\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double arg0 = " << GetBottom() << ";\n";
    FormulaToken* pCur = vSubArguments[0]->GetFormulaToken();
    assert(pCur);
    if (pCur->GetType() == formula::svSingleVectorRef)
    {
        const formula::SingleVectorRefToken* pSVR =
            static_cast< const formula::SingleVectorRefToken* >(pCur);
        ss << "    if (gid0 < " << pSVR->GetArrayLength() << "){\n";
    }
    else if (pCur->GetType() == formula::svDouble)
    {
        ss << "    {\n";
    }
    if(ocPush==vSubArguments[0]->GetFormulaToken()->GetOpCode())
    {
        ss << "        if (isnan(";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef();
        ss << "))\n";
        ss << "            arg0 = 0;\n";
        ss << "        else\n";
        ss << "            arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
        ss << "    arg0 = floor(arg0);\n";
        ss << "    if (arg0 < 0.0)\n";
        ss << "        return 0.0;\n";
        ss << "    else if (arg0 == 0.0)\n";
        ss << "        return 1.0;\n";
        ss << "    else if (arg0 <= 170.0)\n";
        ss << "    {\n";
        ss << "        double fTemp = arg0;\n";
        ss << "        while (fTemp > 2.0)\n";
        ss << "        {\n";
        ss << "            fTemp = fTemp - 1;\n";
        ss << "            arg0 = arg0 * fTemp;\n";
        ss << "        }\n";
        ss << "    }\n";
        ss << "    else\n";
        ss << "        return -DBL_MAX;\n";
        ss << "    }\n";
    }
    else
    {
        ss << "    arg0 = ";
        ss << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    }
    ss << "    return arg0;\n";
    ss << "}";
}
void OpQuotient::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i) ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ") {\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    double num1 = 1.0;\n";
    ss << "    double num2 = 1.0;\n";
    ss << "    if(isnan(";
    ss << vSubArguments[0]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        num1 = 1.0;\n";
    ss << "    else \n    ";
    ss << "    num1 = " << vSubArguments[0]->GenSlidingWindowDeclRef() << ";\n";
    ss << "    if(isnan(";
    ss << vSubArguments[1]->GenSlidingWindowDeclRef() << "))\n";
    ss << "        num2 = 1.0;\n";
    ss << "    else \n    ";
    ss << "    num2 = " << vSubArguments[1]->GenSlidingWindowDeclRef() << ";\n";
    ss << "    return trunc(num1/num2);\n";
    ss << "}";
}
void OpSeriesSum::GenSlidingWindowFunction(std::stringstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT(4,4);
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (size_t i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n";
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double var[3], coeff, res = 0.0f;\n";
    FormulaToken *tmpCur;
    for(int i = 0; i < 3; ++i)
    {
        tmpCur = vSubArguments[i]->GetFormulaToken();
        assert(tmpCur);
        if(ocPush == vSubArguments[i]->GetFormulaToken()->GetOpCode())
        {
            if(tmpCur->GetType() == formula::svSingleVectorRef)
            {
                const formula::SingleVectorRefToken* tmpCurDVR=
                    static_cast<
                    const formula::SingleVectorRefToken *>(tmpCur);
                ss << "    var["<<i<<"] = ";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef();
                ss << ";\n";
                ss << "    if(isnan(var["<<i<<"])||(gid0>=";
                ss << tmpCurDVR->GetArrayLength();
                ss << "))\n";
                ss << "        var["<<i<<"] = 0;\n";
            }
            else if(tmpCur->GetType() == formula::svDouble)
            {
                ss << "    var["<<i<<"] = ";
                ss << tmpCur->GetDouble() << ";\n";
            }
        }
        else
        {
            ss << "    var["<<i<<"] = ";
            ss << vSubArguments[i]->GenSlidingWindowDeclRef();
            ss << ";\n";
        }
    }
    tmpCur = vSubArguments[3]->GetFormulaToken();
    assert(tmpCur);
    if(ocPush == vSubArguments[3]->GetFormulaToken()->GetOpCode())
    {
        //TODO       DoubleVector
        if (tmpCur->GetType() == formula::svDoubleVectorRef)
        {
            const formula::DoubleVectorRefToken* pDVR =
                static_cast<const formula::DoubleVectorRefToken *>(tmpCur);
            size_t nCurWindowSize = pDVR->GetRefRowSize();
            ss << "    int j = 0;\n";
            ss << "    for (int i = ";
            if (!pDVR->IsStartFixed() && pDVR->IsEndFixed())
            {
                ss << "gid0; i < " << pDVR->GetArrayLength();
                ss << " && i < " << nCurWindowSize  << "; ++i)\n";
                ss << "    {\n";
            }
            else if (pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i < " << pDVR->GetArrayLength();
                ss << " && i < gid0+"<< nCurWindowSize << "; ++i)\n";
                ss << "    {\n";
            }
            else if (!pDVR->IsStartFixed() && !pDVR->IsEndFixed())
            {
                ss << "0; i + gid0 < " << pDVR->GetArrayLength();
                ss << " &&  i < "<< nCurWindowSize << "; ++i)\n";
                ss << "    {\n";
            }
            else
            {
                ss << "0; i < "<< nCurWindowSize << "; ++i)\n";
                ss << "    {\n";
            }
            ss << "        coeff = ";
            ss << vSubArguments[3]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "        if (isnan(coeff))\n";
            ss << "            continue;\n";
            ss << "        res = res + coeff * pow(var[0],";
            ss << " var[1] + j * var[2]);\n";
            ss << "        ++j;\n";
            ss << "    }\n";
        }
        else if(tmpCur->GetType() == formula::svSingleVectorRef)
        {
            const formula::SingleVectorRefToken* tmpCurDVR=
                static_cast<
                const formula::SingleVectorRefToken *>(tmpCur);
            ss << "    coeff = ";
            ss << vSubArguments[3]->GenSlidingWindowDeclRef();
            ss << ";\n";
            ss << "    if(isnan(coeff)||(gid0>=";
            ss << tmpCurDVR->GetArrayLength();
            ss << "))\n";
            ss << "        return 0;\n";
        }
        else
            throw Unhandled(__FILE__, __LINE__);
    }
    ss << "    return res;\n";
    ss << "}";
}
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
