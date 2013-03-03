/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#define CSV_USE_CSV_ASSERTIONS
#include <cosv/csv_env.hxx>

#include <cosv/comfunc.hxx>
#include <cosv/string.hxx>
#include <cosv/streamstr.hxx>
#include <cosv/std_outp.hxx>
#include <cosv/tpl/dyn.hxx>

// NOT FULLY DECLARED SERVICES
#include <stdlib.h>


namespace csv
{

#ifdef CSV_NO_IOSTREAMS
redirect_out *  redirect_out::pStdOut_ = 0;
redirect_out *  redirect_out::pStdErr_ = 0;
#endif // defined(CSV_NO_IOSTREAMS)


void
Endl( ostream & io_rStream )
{
#ifndef CSV_NO_IOSTREAMS
//    if (NOT bUseRedirect)
        io_rStream << std::endl;
//    else
#endif
}

void
Flush( ostream & io_rStream )
{
#ifndef CSV_NO_IOSTREAMS
//    if (NOT bUseRedirect)
        io_rStream << std::flush;
//    else
#endif
}

void
PerformAssertion(const char * condition, const char * file, unsigned line)
{
    Cout() << "assertion failed: "
         << condition
         << " in file: "
         << file
         << " at line: "
         << line
         << Endl;

    exit(3);
}

}   // namespace csv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
