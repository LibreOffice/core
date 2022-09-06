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
#include "op_math_helpers.hxx"
#include <sstream>

using namespace formula;

namespace sc::opencl {

void OpMathOneArgument::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 1 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateCode( ss );
    ss << "}";
}

void OpMathTwoArguments::GenSlidingWindowFunction(outputstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    double tmp = 0;\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    GenerateArg( 1, vSubArguments, ss );
    GenerateCode( ss );
    ss << "}";
}

void OpCos::GenerateCode( outputstream& ss ) const
{
    ss << "    return cos(arg0);\n";
}

void OpSec::GenerateCode( outputstream& ss ) const
{
    ss << "    return 1.0 / cos(arg0);\n";
}

void OpSecH::GenerateCode( outputstream& ss ) const
{
    ss << "    return 1.0 / cosh(arg0);\n";
}

void OpCosh::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(local_coshDecl);
    funs.insert(local_cosh);
}

void OpCosh::GenerateCode( outputstream& ss ) const
{
    ss << "    return local_cosh(arg0);\n";
}

void OpCot::GenerateCode( outputstream& ss ) const
{
    ss << "    arg0 = arg0 * M_1_PI;\n";
    ss << "    return cospi(arg0) / sinpi(arg0);\n";
}

void OpCoth::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(local_cothDecl);
    funs.insert(local_coth);
}

void OpCoth::GenerateCode( outputstream& ss ) const
{
    ss << "    return local_coth(arg0);\n";
}

void OpEven::GenerateCode( outputstream& ss ) const
{
    ss << "    double tmp = fabs(arg0 / 2);\n";
    ss << "    if ( trunc(tmp) == tmp )\n";
    ss << "        tmp = tmp * 2;\n";
    ss << "    else\n";
    ss << "        tmp = (trunc(tmp) + 1) * 2;\n";
    ss << "    if (arg0 < 0)\n";
    ss << "        tmp = tmp * -1.0;\n";
    ss << "    return tmp;\n";
}

void OpCsc::GenerateCode( outputstream& ss ) const
{
    ss << "    return 1/sin(arg0);\n";
}

void OpCscH::GenerateCode( outputstream& ss ) const
{
    ss << "    return 1/sinh(arg0);\n";
}

void OpExp::GenerateCode( outputstream& ss ) const
{
    ss << "    return pow(M_E, arg0);\n";
}

void OpLog10::GenerateCode( outputstream& ss ) const
{
    ss << "    return log10(arg0);\n";
}

void OpSinh::GenerateCode( outputstream& ss ) const
{
    ss << "    return ( exp(arg0)-exp(-arg0) )/2;\n";
}

void OpSin::GenerateCode( outputstream& ss ) const
{
    ss << "    arg0 = arg0 * M_1_PI;\n";
    ss << "    return sinpi(arg0);\n";
}

void OpAbs::GenerateCode( outputstream& ss ) const
{
    ss << "    return fabs(arg0);\n";
}

void OpArcCos::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(atan2Decl);
    funs.insert(atan2Content);
}

void OpArcCos::GenerateCode( outputstream& ss ) const
{
    ss << "    return arctan2(sqrt(1.0 - pow(arg0, 2)), arg0);\n";
}

void OpArcCosHyp::GenerateCode( outputstream& ss ) const
{
    ss << "    if( arg0 < 1 )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    return  log( arg0 + pow( (pown(arg0, 2) - 1.0), 0.5));\n";
}

void OpTan::GenerateCode( outputstream& ss ) const
{
    ss << "    arg0 = arg0 * M_1_PI;\n";
    ss << "    return sinpi(arg0) / cospi(arg0);\n";
}

void OpTanH::GenerateCode( outputstream& ss ) const
{
    ss << "    return tanh(arg0);\n";
}

void OpSqrt::GenerateCode( outputstream& ss ) const
{
    ss << "    if( arg0 < 0 )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    return sqrt(arg0);\n";
}

void OpArcCot::GenerateCode( outputstream& ss ) const
{
    ss << "    return M_PI_2 - atan(arg0);\n";
}

void OpArcCotHyp::GenerateCode( outputstream& ss ) const
{
    ss << "    return 0.5 * log(1 + 2 / (arg0 - 1.0));\n";
}

void OpArcSin::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(atan2Decl);
    funs.insert(atan2Content);
}

void OpArcSin::GenerateCode( outputstream& ss ) const
{
    ss << "    return arctan2(arg0, sqrt(1.0 - pow(arg0, 2)));\n";
}

void OpArcSinHyp::GenerateCode( outputstream& ss ) const
{
    ss << "    return  log( arg0 + pow((pown(arg0, 2) + 1.0), 0.5));\n";
}

void OpArcTan::GenerateCode( outputstream& ss ) const
{
    ss << "    return atan(arg0);\n";
}

void OpArcTanH::GenerateCode( outputstream& ss ) const
{
    ss << "    double a = 1.0 + arg0;\n";
    ss << "    double b = 1.0 - arg0;\n";
    ss << "    return log(pow(a/b, 0.5));\n";
}

void OpLn::GenerateCode( outputstream& ss ) const
{
    ss << "    return log1p(arg0-1);\n";
}

void OpInt::GenerateCode( outputstream& ss ) const
{
    ss << "    int intTmp = (int)arg0;\n";
    // check whether rounding error caused the float to be just less than the int value
    ss << "    if( arg0 >=0 && approx_equal( intTmp + 1, arg0 ))\n";
    ss << "        ++intTmp;\n";
    // negative values are rounded down
    ss << "    if( arg0 < 0 && !approx_equal( intTmp, arg0 ))\n";
    ss << "        --intTmp;\n";
    ss << "    return intTmp;\n";
}

void OpNegSub::GenerateCode( outputstream& ss ) const
{
    ss << "    return -arg0;\n";
}

void OpRadians::GenerateCode( outputstream& ss ) const
{
    ss << "    return arg0 * M_PI / 180.0;\n";
}

void OpIsEven::GenerateCode( outputstream& ss ) const
{
    ss << "    return (fmod(floor(fabs(arg0)), 2.0)<0.5);\n";
}

void OpIsOdd::GenerateCode( outputstream& ss ) const
{
    ss << "    return !(fmod(floor(fabs(arg0)), 2.0)<0.5);\n";
}

void OpSqrtPi::GenerateCode( outputstream& ss ) const
{
    ss << "    return (double)sqrt(arg0 * M_PI);\n";
}

void OpDeg::GenerateCode( outputstream& ss ) const
{
    ss << "    return arg0 / M_PI * 180;;\n";
}

void OpFact::GenerateCode( outputstream& ss ) const
{
    ss << "    arg0 = floor(arg0);\n";
    ss << "    if (arg0 < 0.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
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
    ss << "        return CreateDoubleError(NoValue);\n";
    ss << "    return arg0;\n";
}

void OpOdd::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(Math_IntgDecl);
    funs.insert(Math_Intg);
}

void OpOdd::GenerateCode( outputstream& ss ) const
{
    ss << "    double tmp;\n";
    ss << "    if (arg0 > 0.0 ){\n";
    ss << "        tmp=Intg(arg0);\n";
    ss << "        if(tmp-trunc(tmp/2)*2 == 0)\n";
    ss << "            tmp=tmp+1;\n";
    ss << "    }else if (arg0 < 0.0 ){\n";
    ss << "        tmp=Intg(arg0);\n";
    ss << "        if(tmp-trunc(tmp/2)*2 == 0)\n";
    ss << "            tmp=tmp-1.0;\n";
    ss << "    }else\n";
    ss << "        tmp=1.0;\n";
    ss << "    return tmp;\n";
}

void OpMROUND::GenerateCode( outputstream& ss ) const
{
    ss<<"    if(arg1==0)\n";
    ss<<"        return arg1;\n";
    ss<<"    tmp=arg1 * round(arg0 / arg1);\n";
    ss<<"    return tmp;\n";
}

void OpCombinA::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(bikDecl);
    funs.insert(bik);
}

void OpCombinA::GenerateCode( outputstream& ss ) const
{
    ss << "    arg0 = trunc(arg0);\n";
    ss << "    arg1 = trunc(arg1);\n";
    ss << "    double tem;\n";
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
}

void OpCombin::GenerateCode( outputstream& ss ) const
{
    ss << "    double result = -1.0;\n";
    ss << "    double num = floor( arg0 );\n";
    ss << "    double num_chosen = floor( arg1 );\n";
    ss << "    if(num < 0 || num_chosen < 0 || num < num_chosen )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
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
    ss << "        db4result = db4num / db4num_chosen;\n";
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
}

void OpMod::GenerateCode( outputstream& ss ) const
{
    ss << "    if(isnan(arg0)||arg0 == 0)\n";
    ss << "        return 0;\n";
    ss << "    if(isnan(arg1) || arg1 ==0)\n";
    ss << "        return CreateDoubleError(DivisionByZero);\n";
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
}

void OpPower::GenerateCode( outputstream& ss ) const
{
    ss << "    return pow(arg0,arg1);\n";
}

void OpArcTan2::BinInlineFun(std::set<std::string>& decls,
    std::set<std::string>& funs)
{
    decls.insert(atan2Decl);
    funs.insert(atan2Content);
}

void OpArcTan2::GenerateCode( outputstream& ss ) const
{
    ss << "    return arctan2(arg1, arg0);\n";
}

void OpBitAnd::GenerateCode( outputstream& ss ) const
{
    ss << "    if( arg0 < 0 || arg1 < 0 || arg0 >= 281474976710656.0 || arg1 >= 281474976710656.0 )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    return (long)arg0 & (long)arg1;\n";
}

void OpBitOr::GenerateCode( outputstream& ss ) const
{
    ss << "    if( arg0 < 0 || arg1 < 0 || arg0 >= 281474976710656.0 || arg1 >= 281474976710656.0 )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    return (long)arg0 | (long)arg1;\n";
}

void OpBitXor::GenerateCode( outputstream& ss ) const
{
    ss << "    if( arg0 < 0 || arg1 < 0 || arg0 >= 281474976710656.0 || arg1 >= 281474976710656.0 )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    return (long)arg0 ^ (long)arg1;\n";
}

void OpBitLshift::GenerateCode( outputstream& ss ) const
{
    ss << "    double num = floor( arg0 );\n";
    ss << "    double shift_amount = floor( arg1 );\n";
    ss << "    if( num < 0 || num >= 281474976710656.0 )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    return floor(shift_amount >= 0 ? ";
    ss << "num * pow(2.0, shift_amount) : ";
    ss << "num / pow(2.0, fabs(shift_amount)));\n";
}

void OpBitRshift::GenerateCode( outputstream& ss ) const
{
    ss << "    double num = floor( arg0 );\n";
    ss << "    double shift_amount = floor( arg1 );\n";
    ss << "    if( num < 0 || num >= 281474976710656.0 )\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    return floor(";
    ss << "shift_amount >= 0 ? num / pow(2.0, shift_amount) : ";
    ss << "num * pow(2.0, fabs(shift_amount)));\n";
}

void OpQuotient::GenerateCode( outputstream& ss ) const
{
    ss << "    return trunc(arg0/arg1);\n";
}

void OpLog::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( "arg0", 0, vSubArguments, ss );
    GenerateArgWithDefault( "arg1", 1, 10, vSubArguments, ss );
    ss << "    return log10(arg0)/log10(arg1);;\n";
    ss << "}";
}

void OpCountIfs::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
         formula::DoubleVectorRefToken *>(tmpCur);
    size_t nCurWindowSize = pCurDVR->GetArrayLength() <
    pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength():
    pCurDVR->GetRefRowSize() ;
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss <<"    int gid0=get_global_id(0);\n";
    ss << "    int tmp =0;\n";
    ss << "    int loop;\n";
    GenTmpVariables(ss,vSubArguments);

    ss<< "    int singleIndex =gid0;\n";
    int m=0;

    outputstream tmpss;

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

void OpSumIfs::GenSlidingWindowFunction(outputstream &ss,
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
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
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

void OpAverageIfs::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
         formula::DoubleVectorRefToken *>(tmpCur);
    size_t nCurWindowSize = pCurDVR->GetArrayLength() <
    pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength():
    pCurDVR->GetRefRowSize() ;
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss <<"     int gid0=get_global_id(0);\n";
    ss << "    double tmp =0;\n";
    ss << "    int count=0;\n";
    ss << "    int loop;";
    GenTmpVariables(ss,vSubArguments);
    ss<< "    int singleIndex =gid0;\n";
    int m=0;
    outputstream tmpss;
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

void OpRound::GenSlidingWindowFunction(outputstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
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

void OpRoundUp::GenSlidingWindowFunction(outputstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    int singleIndex =  gid0;\n";
    ss << "    int intTmp;\n";
    ss << "    double tmp;\n";
    GenTmpVariables(ss,vSubArguments);
    CheckAllSubArgumentIsNan(ss,vSubArguments);
    if( vSubArguments.size() == 1 )
        ss << "    double tmp1 = 0;\n";
    ss << "    int shift = (int)tmp1;\n";
    ss << "    if(shift >20 || shift < -20)";
    ss << "    {\n";
    ss << "        tmp = NAN;\n";
    ss << "    }else\n";
    ss << "    {\n";
    ss << "        double multiply = 1.0;\n";
    ss << "        for(int i=0;i<shift;i++)\n";
    ss << "            multiply *= 10;\n";
    ss << "        for(int i=0;i>shift;i--)\n";
    ss << "            multiply /= 10;\n";
    ss << "        intTmp = (int)(tmp0*multiply);\n";
    ss << "        if(tmp0 >= 0)\n";
    ss << "            tmp = intTmp + 1;\n";
    ss << "        else\n";
    ss << "            tmp = intTmp - 1;\n";
    ss << "        tmp /= multiply;\n";
    ss << "    }\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpRoundDown::GenSlidingWindowFunction(outputstream &ss,
             const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    int singleIndex =  gid0;\n";
    ss << "    int intTmp;\n";
    ss << "    double tmp;\n";
    GenTmpVariables(ss,vSubArguments);
    CheckAllSubArgumentIsNan(ss,vSubArguments);
    if( vSubArguments.size() == 1 )
        ss << "    double tmp1 = 0;\n";
    ss << "    int shift = (int)tmp1;\n";
    ss << "    if(shift >20 || shift < -20)";
    ss << "    {\n";
    ss << "        tmp = NAN;\n";
    ss << "    }else\n";
    ss << "    {\n";
    ss << "        double multiply = 1.0;\n";
    ss << "        for(int i=0;i<shift;i++)\n";
    ss << "            multiply *= 10;\n";
    ss << "        for(int i=0;i>shift;i--)\n";
    ss << "            multiply /= 10;\n";
    ss << "        intTmp = (int)(tmp0*multiply);\n";
    ss << "        tmp = intTmp;\n";
    ss << "        tmp /= multiply;\n";
    ss << "    }\n";
    ss << "    return tmp;\n";
    ss << "}";
}

void OpCountIf::GenSlidingWindowFunction(outputstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
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

void OpSumIf::GenSlidingWindowFunction(outputstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double vara, varb, varc, sum = 0.0f;\n";
    int flag = 3 == vSubArguments.size() ? 2 : 0;
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

void OpTrunc::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 2 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( "arg0", 0, vSubArguments, ss );
    GenerateArgWithDefault( "arg1", 1, 0, vSubArguments, ss );
    ss << "    double argm = arg0;\n";
    ss << "    int n = (int)arg1;\n";
    ss << "    double nn = 1.0f;\n";
    ss << "    for(int i = 0; i < n; ++i)\n";
    ss << "    {\n";
    ss << "        argm = argm * 10;\n";
    ss << "        nn = nn * 10;\n";
    ss << "    }\n";
    ss << "    for(int i = 0; i > n; --i)\n";
    ss << "    {\n";
    ss << "        argm = argm / 10;\n";
    ss << "        nn = nn / 10;\n";
    ss << "    }\n";
    ss << "    modf(argm, &argm);\n";
    ss << "    return argm / nn;\n";
    ss << "}\n";
}

void OpFloor::GenSlidingWindowFunction(
    outputstream &ss, const std::string &sSymName,
    SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    GenerateArg( "arg0", 0, vSubArguments, ss );
    GenerateArg( "arg1", 1, vSubArguments, ss );
    GenerateArgWithDefault( "arg2", 2, 0, vSubArguments, ss );
    ss << "    if(isnan(arg0) || isnan(arg1))\n";
    ss << "        return 0;\n";
    ss << "    if(isnan(arg2))\n";
    ss << "        arg2 = 0.0;\n";
    ss << "    if(arg0*arg1<0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    if(arg1 == 0.0)\n";
    ss << "        return 0.0;\n";
    ss << "    else if(arg2==0.0&&arg0<0.0)\n";
    ss << "        return (trunc(arg0/arg1)+1)*arg1;\n";
    ss << "    else\n";
    ss << "        return trunc(arg0/arg1)*arg1;\n";
    ss << "}\n";
}

void OpSumSQ::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double sum = 0.0f, arg;\n";
    for(const DynamicKernelArgumentRef & rArg : vSubArguments)
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

void OpCeil::GenSlidingWindowFunction(outputstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0   = get_global_id(0);\n";
    GenerateArg( "num", 0, vSubArguments, ss );
    GenerateArg( "significance", 1, vSubArguments, ss );
    GenerateArgWithDefault( "bAbs", 2, 0, vSubArguments, ss );
    ss << "    if(num*significance < 0.0)\n";
    ss << "        return CreateDoubleError(IllegalArgument);\n";
    ss << "    if(significance == 0.0)\n";
    ss << "        return 0.0;\n";
    ss << "    return ";
    ss << "( !(int)bAbs && num < 0.0 ? floor( num / significance ) : ";
    ss << "ceil( num / significance ) )";
    ss << "*significance;\n";
    ss << "}";
}

void OpProduct::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
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

void OpAverageIf::GenSlidingWindowFunction(outputstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
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

void OpSeriesSum::GenSlidingWindowFunction(outputstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT(4,4);
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
