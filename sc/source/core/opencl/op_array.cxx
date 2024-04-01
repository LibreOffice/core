/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "op_array.hxx"

#include <sstream>

namespace sc::opencl {

void OpSumX2MY2::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 0 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 1 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp =0;\n";
    GenerateRangeArgPair( 0, 1, vSubArguments, ss, EmptyIsZero,
        "        tmp +=pow(arg1,2) - pow(arg2,2);\n"
        );
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpSumX2PY2::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 0 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 1 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp =0;\n";
    GenerateRangeArgPair( 0, 1, vSubArguments, ss, EmptyIsZero,
        "        tmp +=pow(arg1,2) + pow(arg2,2);\n"
        );
    ss << "    return tmp;\n";
    ss << "}\n";
}

void OpSumXMY2::GenSlidingWindowFunction(outputstream &ss,
            const std::string &sSymName, SubArguments &vSubArguments)
{
    CHECK_PARAMETER_COUNT( 2, 2 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 0 );
    CHECK_PARAMETER_DOUBLEVECTORREF( 1 );
    GenerateFunctionDeclaration( sSymName, vSubArguments, ss );
    ss << "{\n";
    ss << "    int gid0=get_global_id(0);\n";
    ss << "    double tmp =0;\n";
    GenerateRangeArgPair( 0, 1, vSubArguments, ss, EmptyIsZero,
        "        tmp +=pow((arg1-arg2),2);\n"
        );
    ss << "    return tmp;\n";
    ss << "}\n";
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
