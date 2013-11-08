/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_OPENCL_OPINLINFUN_MATH
#define SC_OPENCL_OPINLINFUN_MATH

std::string Math_Intg_Str=
"\ndouble Intg(double n)\n\
{\n\
    if(trunc(n)==n )\n\
        return n;\n\
    else if(n<0)\n\
        return trunc(n)-1;\n\
    else\n\
        return trunc(n)+1;\n\
}\n";

#endif //SC_OPENCL_OPINLINFUN_MATH

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
