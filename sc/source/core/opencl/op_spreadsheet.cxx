/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_spreadsheet.hxx"

#include "formulagroup.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"
#include "compiler.hxx"
#include "interpre.hxx"
#include <formula/vectortoken.hxx>
#include <sstream>

using namespace formula;

namespace sc { namespace opencl {

void OpVLookup::GenSlidingWindowFunction(std::stringstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    ss << "\ndouble " << sSymName;
    ss << "_"<< BinFuncName() <<"(";
    for (unsigned i = 0; i < vSubArguments.size(); i++)
    {
        if (i)
            ss << ",";
        vSubArguments[i]->GenSlidingWindowDecl(ss);
    }
    ss << ")\n    {\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp = CreateDoubleError(NOTAVAILABLE);\n";
    ss << "    double intermediate = DBL_MAX;\n";
    ss << "    int singleIndex = gid0;\n";
    ss << "    int rowNum = -1;\n";

    GenTmpVariables(ss,vSubArguments);
    int arg=0;
    CheckSubArgumentIsNan(ss,vSubArguments,arg++);
    int secondParaWidth = 1;

    if (vSubArguments[1]->GetFormulaToken()->GetType() == formula::svDoubleVectorRef)
    {
        FormulaToken *tmpCur = vSubArguments[1]->GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR = static_cast<const formula::DoubleVectorRefToken *>(tmpCur);
        secondParaWidth = pCurDVR->GetArrays().size();
    }

    arg += secondParaWidth;
    CheckSubArgumentIsNan(ss,vSubArguments,arg++);

    if (vSubArguments.size() == (unsigned int)(3+(secondParaWidth-1)))
    {
        ss << "    double tmp";
        ss << 3+(secondParaWidth-1);
        ss << "= 1;\n";
    }
    else
    {
        CheckSubArgumentIsNan(ss,vSubArguments,arg++);
    }

    if (vSubArguments[1]->GetFormulaToken()->GetType() == formula::svDoubleVectorRef)
    {
        FormulaToken *tmpCur = vSubArguments[1]->GetFormulaToken();
        const formula::DoubleVectorRefToken*pCurDVR = static_cast<const formula::DoubleVectorRefToken *>(tmpCur);
        size_t nCurWindowSize = pCurDVR->GetArrayLength() < pCurDVR->GetRefRowSize() ? pCurDVR->GetArrayLength() : pCurDVR->GetRefRowSize() ;
        int unrollSize = 8;
        ss << "    int loop;\n";
        if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed())
        {
            ss << "    loop = ("<<nCurWindowSize<<" - gid0)/";
            ss << unrollSize<<";\n";

        }
        else if (pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
        {
            ss << "    loop = ("<<nCurWindowSize<<" + gid0)/";
            ss << unrollSize<<";\n";

        }
        else
        {
            ss << "    loop = "<<nCurWindowSize<<"/"<< unrollSize<<";\n";
        }

        for (int i = 0; i < secondParaWidth; i++)
        {
            ss << "    for ( int j = 0;j< loop; j++)\n";
            ss << "    {\n";
            ss << "        int i = ";
            if (!pCurDVR->IsStartFixed()&& pCurDVR->IsEndFixed())
            {
                ss << "gid0 + j * "<< unrollSize <<";\n";
            }
            else
            {
                ss << "j * "<< unrollSize <<";\n";
            }
            if (!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
            {
                ss << "        int doubleIndex = i+gid0;\n";
            }
            else
            {
                ss << "        int doubleIndex = i;\n";
            }
            ss << "        if(tmp";
            ss << 3+(secondParaWidth-1);
            ss << " == 1)\n";
            ss << "        {\n";

            for (int j = 0;j < unrollSize; j++)
            {
                CheckSubArgumentIsNan(ss,vSubArguments,1+i);

                ss << "            if((tmp0 - tmp";
                ss << 1+i;
                ss << ")>=0 && intermediate > ( tmp0 -tmp";
                ss << 1+i;
                ss << "))\n";
                ss << "            {\n";
                ss << "                rowNum = doubleIndex;\n";
                ss << "                intermediate = tmp0 - tmp";
                ss << 1+i;
                ss << ";\n";
                ss << "            }\n";
                ss << "            i++;\n";
                ss << "            doubleIndex++;\n";
            }

            ss << "        }else\n";
            ss << "        {\n";
            for (int j = 0; j < unrollSize; j++)
            {
                CheckSubArgumentIsNan(ss,vSubArguments,1+i);

                ss << "            if(tmp0 == tmp";
                ss << 1+i;
                ss << " && rowNum == -1)\n";
                ss << "            {\n";
                ss << "                rowNum = doubleIndex;\n";
                ss << "            }\n";
                ss << "            i++;\n";
                ss << "            doubleIndex++;\n";
            }
            ss << "        }\n\n";

            ss << "    }\n";
            ss << "    if(rowNum!=-1)\n";
            ss << "    {\n";
            for (int j = 0; j < secondParaWidth; j++)
            {
                ss << "        if(tmp";
                ss << 2+(secondParaWidth-1);
                ss << " == ";
                ss << j+1;
                ss << ")\n";
                ss << "            tmp = ";
                vSubArguments[1+j]->GenDeclRef(ss);
                ss << "[rowNum];\n";
            }
            ss << "        return tmp;\n";
            ss << "    }\n";
            ss << "    for (int i = ";
            if (!pCurDVR->IsStartFixed() && pCurDVR->IsEndFixed())
            {
                ss << "gid0 + loop *"<<unrollSize<<"; i < ";
                ss << nCurWindowSize <<"; i++)\n";
            }
            else if (pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
            {
                ss << "0 + loop *"<<unrollSize<<"; i < gid0+";
                ss << nCurWindowSize <<"; i++)\n";
            }
            else
            {
                ss << "0 + loop *"<<unrollSize<<"; i < ";
                ss << nCurWindowSize <<"; i++)\n";
            }
            ss << "    {\n";
            if (!pCurDVR->IsStartFixed() && !pCurDVR->IsEndFixed())
            {
               ss << "        int doubleIndex = i+gid0;\n";
            }
            else
            {
               ss << "        int doubleIndex = i;\n";
            }
            CheckSubArgumentIsNan(ss,vSubArguments,1+i);
            ss << "        if(tmp";
            ss << 3+(secondParaWidth-1);
            ss << " == 1)\n";
            ss << "        {\n";
            ss << "            if((tmp0 - tmp";
            ss << 1+i;
            ss << ")>=0 && intermediate > ( tmp0 -tmp";
            ss << 1+i;
            ss << "))\n";
            ss << "            {\n";
            ss << "                rowNum = doubleIndex;\n";
            ss << "                intermediate = tmp0 - tmp";
            ss << 1+i;
            ss << ";\n";
            ss << "            }\n";
            ss << "        }\n";
            ss << "        else\n";
            ss << "        {\n";
            ss << "            if(tmp0 == tmp";
            ss << 1+i;
            ss << " && rowNum == -1)\n";
            ss << "            {\n";
            ss << "                rowNum = doubleIndex;\n";
            ss << "            }\n";
            ss << "        }\n";

            ss << "    }\n\n";
            ss << "    if(rowNum!=-1)\n";
            ss << "    {\n";

            for (int j = 0; j < secondParaWidth; j++)
            {
                ss << "        if(tmp";
                ss << 2+(secondParaWidth-1);
                ss << " == ";
                ss << j+1;
                ss << ")\n";
                ss << "            tmp = ";
                vSubArguments[1+j]->GenDeclRef(ss);
                ss << "[rowNum];\n";
            }
            ss << "        return tmp;\n";
            ss << "    }\n";

        }
    }
    else
    {
        CheckSubArgumentIsNan(ss,vSubArguments,1);
        ss << "    if(tmp3 == 1)\n";
        ss << "    {\n";
        ss << "        tmp = tmp1;\n";
        ss << "    }else\n";
        ss << "    {\n";
        ss << "        if(tmp0 == tmp1)\n";
        ss << "            tmp = tmp1;\n";
        ss << "    }\n";
    }
    ss << "    return tmp;\n";
    ss << "}";
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
