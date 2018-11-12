/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_database.hxx"

#include <formula/vectortoken.hxx>
#include <sstream>

namespace sc { namespace opencl {

void OpDmax::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double max = -1000000000000;\n";
    ss << "    double value=0.0;\n";
    GenTmpVariables(ss,vSubArguments);
    int dataCol = 0;
    int dataRow = 0;
    if(vSubArguments[0]->GetFormulaToken()->GetType() !=
        formula::svDoubleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    formula::FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    dataCol = pCurDVR->GetArrays().size();
    dataRow = pCurDVR->GetArrayLength();

    if(vSubArguments[dataCol]->GetFormulaToken()->GetType() !=
       formula::svSingleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    const formula::SingleVectorRefToken*pTmpDVR1= static_cast<const
    formula::SingleVectorRefToken *>(vSubArguments[dataCol]->
    GetFormulaToken());
    ss << "    tmp"<<dataCol<<"=";
    ss << vSubArguments[dataCol]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    if(gid0>="<<pTmpDVR1->GetArrayLength()<<" ||isnan(";
    ss << "tmp"<<dataCol<<"))\n";
    ss << "        tmp"<<dataCol<<"=0;\n";

    int conditionCol = 0;
    int conditionRow = 0;
    if(vSubArguments[dataCol + 1]->GetFormulaToken()->GetType() !=
        formula::svDoubleVectorRef)
    {
        throw Unhandled(__FILE__, __LINE__);
    }
    tmpCur = vSubArguments[dataCol + 1]->
        GetFormulaToken();
    pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    conditionCol = pCurDVR->GetArrays().size();
    conditionRow = pCurDVR->GetArrayLength();

    if(dataCol!=conditionCol)
            throw Unhandled(__FILE__, __LINE__);
    if(dataCol > 0 && dataRow > 0)
    {
        formula::FormulaToken *tmpCur1 = vSubArguments[0]->GetFormulaToken();
        formula::FormulaToken *tmpCur2 = vSubArguments[dataCol + 1]->
            GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR1= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur1);
        const formula::DoubleVectorRefToken*pCurDVR2= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur2);

        if(pCurDVR1->IsStartFixed() && pCurDVR1->IsEndFixed() &&
            pCurDVR2->IsStartFixed() && pCurDVR2->IsEndFixed())
        {
            ss << "    int i,j,p;\n";
            ss << "    bool flag;\n";
            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isnan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isnan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            if(max<value)\n";
            ss << "                max=value;";
            ss << "        }\n";
            ss << "    }\n";
        }
        else
            ss << "max = -1;\n";
    }
    else
        ss << "max = -1;\n";
    ss << "    return max;\n";
    ss << "}";
}

void OpDmin::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double min = 1000000000000;\n";
    ss << "    double value=0.0;\n";
    GenTmpVariables(ss,vSubArguments);
    int dataCol = 0;
    int dataRow = 0;
    if(vSubArguments[0]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    formula::FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    dataCol = pCurDVR->GetArrays().size();
    dataRow = pCurDVR->GetArrayLength();

    if(vSubArguments[dataCol]->GetFormulaToken()->GetType() !=
       formula::svSingleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    const formula::SingleVectorRefToken*pTmpDVR1= static_cast<const
    formula::SingleVectorRefToken *>(vSubArguments[dataCol]->
    GetFormulaToken());
    ss << "    tmp"<<dataCol<<"=";
    ss << vSubArguments[dataCol]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    if(gid0>="<<pTmpDVR1->GetArrayLength()<<" ||isnan(";
    ss << "tmp"<<dataCol<<"))\n";
    ss << "        tmp"<<dataCol<<"=0;\n";

    int conditionCol = 0;
    int conditionRow = 0;
    if(vSubArguments[dataCol + 1]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
    {
        throw Unhandled(__FILE__, __LINE__);
    }
    tmpCur = vSubArguments[dataCol + 1]->
        GetFormulaToken();
    pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    conditionCol = pCurDVR->GetArrays().size();
    conditionRow = pCurDVR->GetArrayLength();

    if(dataCol!=conditionCol)
            throw Unhandled(__FILE__, __LINE__);
    if(dataCol > 0 && dataRow > 0)
    {
        formula::FormulaToken *tmpCur1 = vSubArguments[0]->GetFormulaToken();
        formula::FormulaToken *tmpCur2 = vSubArguments[dataCol + 1]->
            GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR1= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur1);
        const formula::DoubleVectorRefToken*pCurDVR2= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur2);

        if(pCurDVR1->IsStartFixed() && pCurDVR1->IsEndFixed() &&
            pCurDVR2->IsStartFixed() && pCurDVR2->IsEndFixed())
        {
            ss << "    int i,j,p;\n";
            ss << "    bool flag;\n";

            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isnan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isnan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            if(min>value)\n";
            ss << "                min=value;";
            ss << "        }\n";
            ss << "    }\n";
        }
        else
            ss << "min = -1;\n";
    }
    else
        ss << "min = -1;\n";
    ss << "    return min;\n";
    ss << "}";
}

void OpDproduct::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double product = 1;\n";
    ss << "    double value =0;\n";
    GenTmpVariables(ss,vSubArguments);
    int dataCol = 0;
    int dataRow = 0;
    if(vSubArguments[0]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    formula::FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    dataCol = pCurDVR->GetArrays().size();
    dataRow = pCurDVR->GetArrayLength();

    if(vSubArguments[dataCol]->GetFormulaToken()->GetType() !=
       formula::svSingleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    const formula::SingleVectorRefToken*pTmpDVR1= static_cast<const
    formula::SingleVectorRefToken *>(vSubArguments[dataCol]->
    GetFormulaToken());
    ss << "    tmp"<<dataCol<<"=";
    ss << vSubArguments[dataCol]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    if(gid0>="<<pTmpDVR1->GetArrayLength()<<" ||isnan(";
    ss << "tmp"<<dataCol<<"))\n";
    ss << "        tmp"<<dataCol<<"=0;\n";

    int conditionCol = 0;
    int conditionRow = 0;
    if(vSubArguments[dataCol + 1]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
    {
        throw Unhandled(__FILE__, __LINE__);
    }
    tmpCur = vSubArguments[dataCol + 1]->
        GetFormulaToken();
    pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    conditionCol = pCurDVR->GetArrays().size();
    conditionRow = pCurDVR->GetArrayLength();

    if(dataCol!=conditionCol)
            throw Unhandled(__FILE__, __LINE__);
    if(dataCol > 0 && dataRow > 0)
    {
        formula::FormulaToken *tmpCur1 = vSubArguments[0]->GetFormulaToken();
        formula::FormulaToken *tmpCur2 = vSubArguments[dataCol + 1]->
            GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR1= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur1);
        const formula::DoubleVectorRefToken*pCurDVR2= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur2);

        if(pCurDVR1->IsStartFixed() && pCurDVR1->IsEndFixed() &&
            pCurDVR2->IsStartFixed() && pCurDVR2->IsEndFixed())
        {
            ss << "    int i,j,p;\n";
            ss << "    bool flag;\n";
            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isnan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isnan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            product*=value;\n";
            ss << "        }\n";
            ss << "    }\n";
        }
        else
            ss << "product = -1;\n";
    }
    else
        ss << "product = -1;\n";
    ss << "    return product;\n";
    ss << "}";
}

void OpDaverage::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double sum = 0;\n";
    ss << "    int count = 0;\n";
    ss << "    double value =0;\n";
    GenTmpVariables(ss,vSubArguments);
    int dataCol = 0;
    int dataRow = 0;
    if(vSubArguments[0]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    formula::FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    dataCol = pCurDVR->GetArrays().size();
    dataRow = pCurDVR->GetArrayLength();

    if(vSubArguments[dataCol]->GetFormulaToken()->GetType() !=
       formula::svSingleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    const formula::SingleVectorRefToken*pTmpDVR1= static_cast<const
    formula::SingleVectorRefToken *>(vSubArguments[dataCol]->
    GetFormulaToken());
    ss << "    tmp"<<dataCol<<"=";
    ss << vSubArguments[dataCol]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    if(gid0>="<<pTmpDVR1->GetArrayLength()<<" ||isnan(";
    ss << "tmp"<<dataCol<<"))\n";
    ss << "        tmp"<<dataCol<<"=0;\n";

    int conditionCol = 0;
    int conditionRow = 0;
    if(vSubArguments[dataCol + 1]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
    {
        throw Unhandled(__FILE__, __LINE__);
    }
    tmpCur = vSubArguments[dataCol + 1]->
        GetFormulaToken();
    pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    conditionCol = pCurDVR->GetArrays().size();
    conditionRow = pCurDVR->GetArrayLength();

    if(dataCol!=conditionCol)
            throw Unhandled(__FILE__, __LINE__);
    if(dataCol > 0 && dataRow > 0)
    {
        formula::FormulaToken *tmpCur1 = vSubArguments[0]->GetFormulaToken();
        formula::FormulaToken *tmpCur2 = vSubArguments[dataCol + 1]->
            GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR1= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur1);
        const formula::DoubleVectorRefToken*pCurDVR2= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur2);

        if(pCurDVR1->IsStartFixed() && pCurDVR1->IsEndFixed() &&
            pCurDVR2->IsStartFixed() && pCurDVR2->IsEndFixed())
        {
            ss << "    int i,j,p;\n";
            ss << "    bool flag;\n";

            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isnan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isnan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            ss << "            count++;\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            sum+=value;\n";
            ss << "        }\n";
            ss << "    }\n";
        }
        else
            ss << "sum = -1;\n";
    }
    else
        ss << "sum = -1;\n";
    ss << "    if(count==0)\n";
    ss << "        return 0;\n";
    ss << "    return sum/count;\n";
    ss << "}";
}

void OpDstdev::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double var = 0;\n";
    ss << "    double mean = 0;\n";
    ss << "    double value =0;\n";
    ss << "    int count = 0;\n";
    GenTmpVariables(ss,vSubArguments);
    int dataCol = 0;
    int dataRow = 0;
    if(vSubArguments[0]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    formula::FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    dataCol = pCurDVR->GetArrays().size();
    dataRow = pCurDVR->GetArrayLength();

    if(vSubArguments[dataCol]->GetFormulaToken()->GetType() !=
       formula::svSingleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    const formula::SingleVectorRefToken*pTmpDVR1= static_cast<const
    formula::SingleVectorRefToken *>(vSubArguments[dataCol]->
    GetFormulaToken());
    ss << "    tmp"<<dataCol<<"=";
    ss << vSubArguments[dataCol]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    if(gid0>="<<pTmpDVR1->GetArrayLength()<<" ||isnan(";
    ss << "tmp"<<dataCol<<"))\n";
    ss << "        tmp"<<dataCol<<"=0;\n";

    int conditionCol = 0;
    int conditionRow = 0;
    if(vSubArguments[dataCol + 1]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
    {
        throw Unhandled(__FILE__, __LINE__);
    }
    tmpCur = vSubArguments[dataCol + 1]->
        GetFormulaToken();
    pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    conditionCol = pCurDVR->GetArrays().size();
    conditionRow = pCurDVR->GetArrayLength();

    if(dataCol!=conditionCol)
            throw Unhandled(__FILE__, __LINE__);
    if(dataCol > 0 && dataRow > 0)
    {
        formula::FormulaToken *tmpCur1 = vSubArguments[0]->GetFormulaToken();
        formula::FormulaToken *tmpCur2 = vSubArguments[dataCol + 1]->
            GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR1= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur1);
        const formula::DoubleVectorRefToken*pCurDVR2= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur2);

        if(pCurDVR1->IsStartFixed() && pCurDVR1->IsEndFixed() &&
            pCurDVR2->IsStartFixed() && pCurDVR2->IsEndFixed())
        {
            ss << "    int i,j,p;\n";
            ss << "    bool flag;\n";

            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isnan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isnan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            ss << "            count++;\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            mean+=value;\n";
            ss << "        }\n";
            ss << "    }\n";

            ss << "    if(count<=1)\n";
            ss << "        return 0;\n";

            ss << "    mean/=count;\n";

            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isnan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isnan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            var+=pow(mean-value,2);\n";
            ss << "        }\n";
            ss << "    }\n";

            ss << "    var = sqrt( var/(count-1) );\n";
        }
        else
            ss << "var = -1;\n";
    }
    else
        ss << "var = -1;\n";
    ss << "    return var;\n";
    ss << "}";
}

void OpDstdevp::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double var = 0;\n";
    ss << "    double mean = 0;\n";
    ss << "    double value =0;\n";
    ss << "    int count = 0;\n";
    GenTmpVariables(ss,vSubArguments);
    int dataCol = 0;
    int dataRow = 0;
    if(vSubArguments[0]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    formula::FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    dataCol = pCurDVR->GetArrays().size();
    dataRow = pCurDVR->GetArrayLength();

    if(vSubArguments[dataCol]->GetFormulaToken()->GetType() !=
       formula::svSingleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    const formula::SingleVectorRefToken*pTmpDVR1= static_cast<const
    formula::SingleVectorRefToken *>(vSubArguments[dataCol]->
    GetFormulaToken());
    ss << "    tmp"<<dataCol<<"=";
    ss << vSubArguments[dataCol]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    if(gid0>="<<pTmpDVR1->GetArrayLength()<<" ||isnan(";
    ss << "tmp"<<dataCol<<"))\n";
    ss << "        tmp"<<dataCol<<"=0;\n";

    int conditionCol = 0;
    int conditionRow = 0;
    if(vSubArguments[dataCol + 1]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
    {
        throw Unhandled(__FILE__, __LINE__);
    }
    tmpCur = vSubArguments[dataCol + 1]->
        GetFormulaToken();
    pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    conditionCol = pCurDVR->GetArrays().size();
    conditionRow = pCurDVR->GetArrayLength();

    if(dataCol!=conditionCol)
            throw Unhandled(__FILE__, __LINE__);
    if(dataCol > 0 && dataRow > 0)
    {
        formula::FormulaToken *tmpCur1 = vSubArguments[0]->GetFormulaToken();
        formula::FormulaToken *tmpCur2 = vSubArguments[dataCol + 1]->
            GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR1= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur1);
        const formula::DoubleVectorRefToken*pCurDVR2= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur2);

        if(pCurDVR1->IsStartFixed() && pCurDVR1->IsEndFixed() &&
            pCurDVR2->IsStartFixed() && pCurDVR2->IsEndFixed())
        {
            ss << "    int i,j,p;\n";
            ss << "    bool flag;\n";

            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isnan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isnan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            ss << "            count++;\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            mean+=value;\n";
            ss << "        }\n";
            ss << "    }\n";

            ss << "    if(count<=1)\n";
            ss << "        return 0;\n";

            ss << "    mean/=count;\n";

            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isnan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isnan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            var+=pow(mean-value,2);\n";
            ss << "        }\n";
            ss << "    }\n";

            ss << "    var = sqrt( var/count );\n";
        }
        else
            ss << "var = -1;\n";
    }
    else
        ss << "var = -1;\n";
    ss << "    return var;\n";
    ss << "}";
}

void OpDsum::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double sum = 0;\n";
    ss << "    double value =0;\n";
    GenTmpVariables(ss,vSubArguments);
    int dataCol = 0;
    int dataRow = 0;
    if(vSubArguments[0]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    formula::FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    dataCol = pCurDVR->GetArrays().size();
    dataRow = pCurDVR->GetArrayLength();

    if(vSubArguments[dataCol]->GetFormulaToken()->GetType() !=
       formula::svSingleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    const formula::SingleVectorRefToken*pTmpDVR1= static_cast<const
    formula::SingleVectorRefToken *>(vSubArguments[dataCol]->
    GetFormulaToken());
    ss << "    tmp"<<dataCol<<"=";
    ss << vSubArguments[dataCol]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    if(gid0>="<<pTmpDVR1->GetArrayLength()<<" ||isnan(";
    ss << "tmp"<<dataCol<<"))\n";
    ss << "        tmp"<<dataCol<<"=0;\n";

    int conditionCol = 0;
    int conditionRow = 0;
    if(vSubArguments[dataCol + 1]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
    {
        throw Unhandled(__FILE__, __LINE__);
    }
    tmpCur = vSubArguments[dataCol + 1]->
        GetFormulaToken();
    pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    conditionCol = pCurDVR->GetArrays().size();
    conditionRow = pCurDVR->GetArrayLength();

    if(dataCol!=conditionCol)
            throw Unhandled(__FILE__, __LINE__);
    if(dataCol > 0 && dataRow > 0)
    {
        formula::FormulaToken *tmpCur1 = vSubArguments[0]->GetFormulaToken();
        formula::FormulaToken *tmpCur2 = vSubArguments[dataCol + 1]->
            GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR1= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur1);
        const formula::DoubleVectorRefToken*pCurDVR2= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur2);

        if(pCurDVR1->IsStartFixed() && pCurDVR1->IsEndFixed() &&
            pCurDVR2->IsStartFixed() && pCurDVR2->IsEndFixed())
        {
            ss << "    int i,j,p;\n";
            ss << "    bool flag;\n";

            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isnan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isnan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            sum+=value;\n";
            ss << "        }\n";
            ss << "    }\n";
        }
        else
            ss << "sum = -1;\n";
    }
    else
        ss << "sum = -1;\n";
    ss << "    return sum;\n";
    ss << "}";
}

void OpDvar::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double var = 0;\n";
    ss << "    double mean = 0;\n";
    ss << "    double value =0;\n";
    ss << "    int count = 0;\n";
    GenTmpVariables(ss,vSubArguments);
    int dataCol = 0;
    int dataRow = 0;
    if(vSubArguments[0]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    formula::FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    dataCol = pCurDVR->GetArrays().size();
    dataRow = pCurDVR->GetArrayLength();

    if(vSubArguments[dataCol]->GetFormulaToken()->GetType() !=
       formula::svSingleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    const formula::SingleVectorRefToken*pTmpDVR1= static_cast<const
    formula::SingleVectorRefToken *>(vSubArguments[dataCol]->
    GetFormulaToken());
    ss << "    tmp"<<dataCol<<"=";
    ss << vSubArguments[dataCol]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    if(gid0>="<<pTmpDVR1->GetArrayLength()<<" ||isnan(";
    ss << "tmp"<<dataCol<<"))\n";
    ss << "        tmp"<<dataCol<<"=0;\n";

    int conditionCol = 0;
    int conditionRow = 0;
    if(vSubArguments[dataCol + 1]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
    {
        throw Unhandled(__FILE__, __LINE__);
    }
    tmpCur = vSubArguments[dataCol + 1]->
        GetFormulaToken();
    pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    conditionCol = pCurDVR->GetArrays().size();
    conditionRow = pCurDVR->GetArrayLength();

    if(dataCol!=conditionCol)
            throw Unhandled(__FILE__, __LINE__);
    if(dataCol > 0 && dataRow > 0)
    {
        formula::FormulaToken *tmpCur1 = vSubArguments[0]->GetFormulaToken();
        formula::FormulaToken *tmpCur2 = vSubArguments[dataCol + 1]->
            GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR1= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur1);
        const formula::DoubleVectorRefToken*pCurDVR2= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur2);

        if(pCurDVR1->IsStartFixed() && pCurDVR1->IsEndFixed() &&
            pCurDVR2->IsStartFixed() && pCurDVR2->IsEndFixed())
        {
            ss << "    int i,j,p;\n";
            ss << "    bool flag;\n";

            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isnan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isnan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            ss << "            count++;\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            mean+=value;\n";
            ss << "        }\n";
            ss << "    }\n";

            ss << "    if(count<=1)\n";
            ss << "        return 0;\n";

            ss << "    mean/=count;\n";

            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isnan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isnan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            var+=pow(mean-value,2);\n";
            ss << "        }\n";
            ss << "    }\n";

            ss << "    var = var/(count-1);\n";
        }
        else
            ss << "var = -1;\n";
    }
    else
        ss << "var = -1;\n";
    ss << "    return var;\n";
    ss << "}";
}

void OpDvarp::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double var = 0;\n";
    ss << "    double mean = 0;\n";
    ss << "    double value =0;\n";
    ss << "    int count = 0;\n";
    GenTmpVariables(ss,vSubArguments);
    int dataCol = 0;
    int dataRow = 0;
    if(vSubArguments[0]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    formula::FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    dataCol = pCurDVR->GetArrays().size();
    dataRow = pCurDVR->GetArrayLength();

    if(vSubArguments[dataCol]->GetFormulaToken()->GetType() !=
       formula::svSingleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    const formula::SingleVectorRefToken*pTmpDVR1= static_cast<const
    formula::SingleVectorRefToken *>(vSubArguments[dataCol]->
    GetFormulaToken());
    ss << "    tmp"<<dataCol<<"=";
    ss << vSubArguments[dataCol]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    if(gid0>="<<pTmpDVR1->GetArrayLength()<<" ||isnan(";
    ss << "tmp"<<dataCol<<"))\n";
    ss << "        tmp"<<dataCol<<"=0;\n";

    int conditionCol = 0;
    int conditionRow = 0;
    if(vSubArguments[dataCol + 1]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
    {
        throw Unhandled(__FILE__, __LINE__);
    }
    tmpCur = vSubArguments[dataCol + 1]->
        GetFormulaToken();
    pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    conditionCol = pCurDVR->GetArrays().size();
    conditionRow = pCurDVR->GetArrayLength();

    if(dataCol!=conditionCol)
            throw Unhandled(__FILE__, __LINE__);
    if(dataCol > 0 && dataRow > 0)
    {
        formula::FormulaToken *tmpCur1 = vSubArguments[0]->GetFormulaToken();
        formula::FormulaToken *tmpCur2 = vSubArguments[dataCol + 1]->
            GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR1= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur1);
        const formula::DoubleVectorRefToken*pCurDVR2= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur2);

        if(pCurDVR1->IsStartFixed() && pCurDVR1->IsEndFixed() &&
            pCurDVR2->IsStartFixed() && pCurDVR2->IsEndFixed())
        {
            ss << "    int i,j,p;\n";
            ss << "    bool flag;\n";

            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isnan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isnan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            ss << "            count++;\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            mean+=value;\n";
            ss << "        }\n";
            ss << "    }\n";

            ss << "    if(count<=0)\n";
            ss << "        return 0;\n";

            ss << "    mean/=count;\n";

            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isnan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isnan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            var+=pow(mean-value,2);\n";
            ss << "        }\n";
            ss << "    }\n";

            ss << "    var = var/count;\n";
        }
        else
            ss << "var = -1;\n";
    }
    else
        ss << "var = -1;\n";
    ss << "    return var;\n";
    ss << "}";
}

void OpDcount::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double value=0;\n";
    ss << "    int count = 0;\n";
    GenTmpVariables(ss,vSubArguments);
    int dataCol = 0;
    int dataRow = 0;
    if(vSubArguments[0]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    formula::FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    dataCol = pCurDVR->GetArrays().size();
    dataRow = pCurDVR->GetArrayLength();

    if(vSubArguments[dataCol]->GetFormulaToken()->GetType() !=
       formula::svSingleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    const formula::SingleVectorRefToken*pTmpDVR1= static_cast<const
        formula::SingleVectorRefToken *>(vSubArguments[dataCol]
        ->GetFormulaToken());
    ss << "    tmp"<<dataCol<<"=";
    ss << vSubArguments[dataCol]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    if(gid0>="<<pTmpDVR1->GetArrayLength()<<" ||isnan(";
    ss << "tmp"<<dataCol<<"))\n";
    ss << "        tmp"<<dataCol<<"=DBL_MIN;\n";

    int conditionCol = 0;
    int conditionRow = 0;
    if(vSubArguments[dataCol + 1]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
    {
        throw Unhandled(__FILE__, __LINE__);
    }
    tmpCur = vSubArguments[dataCol + 1]
        ->GetFormulaToken();
    pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    conditionCol = pCurDVR->GetArrays().size();
    conditionRow = pCurDVR->GetArrayLength();

    if(dataCol!=conditionCol)
            throw Unhandled(__FILE__, __LINE__);
    if(dataCol > 0 && dataRow > 0)
    {
        formula::FormulaToken *tmpCur1 = vSubArguments[0]->GetFormulaToken();
        formula::FormulaToken *tmpCur2 = vSubArguments[dataCol + 1]
            ->GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR1= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur1);
        const formula::DoubleVectorRefToken*pCurDVR2= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur2);

        if(pCurDVR1->IsStartFixed() && pCurDVR1->IsEndFixed() &&
            pCurDVR2->IsStartFixed() && pCurDVR2->IsEndFixed())
        {
            ss << "    int i,j,p;\n";
            ss << "    bool flag;\n";

            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isnan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isnan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            if(value > DBL_MIN)\n";
            ss << "                count++;\n";
            ss << "        }\n";
            ss << "     }\n";
        }
        else
            ss << "count = -1;\n";
    }
    else
        ss << "count = -1;\n";
    ss << "    return count;\n";
    ss << "}";
}

void OpDcount2::GenSlidingWindowFunction(std::stringstream &ss,
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
    ss << ")\n    {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double value=0;\n";
    ss << "    int count = 0;\n";
    GenTmpVariables(ss,vSubArguments);
    int dataCol = 0;
    int dataRow = 0;
    if(vSubArguments[0]->GetFormulaToken()->GetType() !=
        formula::svDoubleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    formula::FormulaToken *tmpCur = vSubArguments[0]->GetFormulaToken();
    const formula::DoubleVectorRefToken*pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    dataCol = pCurDVR->GetArrays().size();
    dataRow = pCurDVR->GetArrayLength();

    if(vSubArguments[dataCol]->GetFormulaToken()->GetType() !=
       formula::svSingleVectorRef)
        throw Unhandled(__FILE__, __LINE__);

    const formula::SingleVectorRefToken*pTmpDVR1= static_cast<const
        formula::SingleVectorRefToken *>(vSubArguments[dataCol]
        ->GetFormulaToken());
    ss << "    tmp"<<dataCol<<"=";
    ss << vSubArguments[dataCol]->GenSlidingWindowDeclRef()<<";\n";
    ss << "    if(gid0>="<<pTmpDVR1->GetArrayLength()<<" ||isnan(";
    ss << "tmp"<<dataCol<<"))\n";
    ss << "        tmp"<<dataCol<<"=DBL_MIN;\n";

    int conditionCol = 0;
    int conditionRow = 0;
    if(vSubArguments[dataCol + 1]->GetFormulaToken()->GetType() !=
       formula::svDoubleVectorRef)
    {
        throw Unhandled(__FILE__, __LINE__);
    }
    tmpCur = vSubArguments[dataCol + 1]->
        GetFormulaToken();
    pCurDVR= static_cast<const
        formula::DoubleVectorRefToken *>(tmpCur);
    conditionCol = pCurDVR->GetArrays().size();
    conditionRow = pCurDVR->GetArrayLength();

    if(dataCol!=conditionCol)
            throw Unhandled(__FILE__, __LINE__);
    if(dataCol > 0 && dataRow > 0)
    {
        formula::FormulaToken *tmpCur1 = vSubArguments[0]->GetFormulaToken();
        formula::FormulaToken *tmpCur2 = vSubArguments[dataCol + 1]->
            GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR1= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur1);
        const formula::DoubleVectorRefToken*pCurDVR2= static_cast<const
            formula::DoubleVectorRefToken *>(tmpCur2);

        if(pCurDVR1->IsStartFixed() && pCurDVR1->IsEndFixed() &&
            pCurDVR2->IsStartFixed() && pCurDVR2->IsEndFixed())
        {
            ss << "    int i,j,p;\n";
            ss << "    bool flag;\n";

            ss << "    for(p = 1;p < " << dataRow << ";++p)\n";
            ss << "    {\n";
            ss << "        i = p;\n";
            for(int i = 0; i < dataCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "        tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "        if(isnan(tmp"<<i<<"))\n";
                ss <<"            tmp"<<i<<" = 0;\n";
            }
            ss << "        flag = false;\n";
            ss << "        for(j = 1; j < " << conditionRow << ";++j)\n";
            ss << "        {\n";
            ss << "            i = j;\n";
            ss << "            if (flag)\n";
            ss << "                break;\n";
            ss << "            else{\n";
            for(int i = dataCol + 1; i < dataCol + 1 + conditionCol; ++i){
                if(vSubArguments[i]->GetFormulaToken()->GetType() !=
                        formula::svDoubleVectorRef)
                    throw Unhandled(__FILE__, __LINE__);
                ss << "                tmp"<<i<<"=";
                ss << vSubArguments[i]->GenSlidingWindowDeclRef()<<";\n";
                ss << "                if(!isnan(tmp"<<i<<")){\n";
                ss << "                    if(tmp"<<(i-dataCol-1)<<"!=tmp";
                ss << i<<"){\n";
                ss << "                        continue;\n";
                ss << "                    }\n";
                ss << "                }\n";
            }
            ss << "                flag=true;\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        if (flag){\n";
            for(int i = 0; i < dataCol; ++i){
                ss << "            if(tmp"<<dataCol<<"=="<<(i+1)<<"){\n";
                ss << "                value=tmp"<<i<<";\n";
                ss << "            }\n";
            }
            ss << "            if(value > DBL_MIN)\n";
            ss << "                count++;\n";
            ss << "        }\n";
            ss << "     }\n";
        }
        else
            ss << "count = -1;\n";
    }
    else
        ss << "count = -1;\n";
    ss << "    return count;\n";
    ss << "}";
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
