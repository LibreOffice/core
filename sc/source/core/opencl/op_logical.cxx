/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_logical.hxx"

#include <sstream>

namespace sc::opencl {

void OpLogicalBinaryOperator::GenSlidingWindowFunction(outputstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 30 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    bool t = false;\n";
    for(size_t j = 0; j< vSubArguments.size(); j++)
    {
        GenerateArg( j, vSubArguments, ss );
        ss << "    t = t " << openclOperator() << " (arg" << j << " != 0);\n";
    }
    ss << "    return t;\n";
    ss << "}\n";
}

void OpAnd::GenSlidingWindowFunction(outputstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 30 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    ss << "    bool t = true;\n";
    for(size_t j = 0; j< vSubArguments.size(); j++)
    {
        GenerateArg( j, vSubArguments, ss, EmptyIsNan );
        // AND() with a svSingleVectorRef pointing to an empty cell skips that cell.
        // See ScInterpreter::ScAnd().
        ss << "    if( !isnan( arg" << j << " ))\n";
        ss << "        t = t " << openclOperator() << " (arg" << j << " != 0);\n";
    }
    ss << "    return t;\n";
    ss << "}\n";
}

void OpNot::GenSlidingWindowFunction(outputstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 1 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    ss << "    return arg0 == 0;\n";
    ss << "}\n";
}

void OpIf::GenSlidingWindowFunction(outputstream &ss,
    const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 1, 3 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0 = get_global_id(0);\n";
    GenerateArg( 0, vSubArguments, ss );
    if(vSubArguments.size()>1)
        GenerateArg( 1, vSubArguments, ss );
    else
        ss << "    double arg1 = 1;\n";
    if(vSubArguments.size()>2)
        GenerateArg( 2, vSubArguments, ss );
    else
        ss << "    double arg2 = 0;\n";

    ss << "    if(arg0 != 0)\n";
    ss << "        return arg1;\n";
    ss << "    else\n";
    ss << "        return arg2;\n";
    ss << "}\n";
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
