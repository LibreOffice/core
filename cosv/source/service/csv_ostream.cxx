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
#include <cosv/csv_ostream.hxx>

// NOT FULLY DECLARED SERVICES


#ifndef CSV_NO_IOSTREAMS
#else

namespace csv
{

ostream::~ostream() { }

ostream & ostream::operator<<(
                            const char *        i_s )   { *pData << i_s; return *this; }
ostream & ostream::operator<<(
                            char                i_c )   { *pData << i_c; return *this; }
ostream & ostream::operator<<(
                            unsigned char       i_c )   { *pData << i_c; return *this; }
ostream & ostream::operator<<(
                            signed char         i_c )   { *pData << i_c; return *this; }

ostream & ostream::operator<<(
                            short               i_n )   { *pData << i_n; return *this; }
ostream & ostream::operator<<(
                            unsigned short      i_n )   { *pData << i_n; return *this; }
ostream & ostream::operator<<(
                            int                 i_n )   { *pData << i_n; return *this; }
ostream & ostream::operator<<(
                            unsigned int        i_n )   { *pData << i_n; return *this; }
ostream & ostream::operator<<(
                            long                i_n )   { *pData << i_n; return *this; }
ostream & ostream::operator<<(
                            unsigned long       i_n )   { *pData << i_n; return *this; }

ostream & ostream::operator<<(
                            float               i_n )   { *pData << i_n; return *this; }
ostream & ostream::operator<<(
                            double              i_n )   { *pData << i_n; return *this; }

ostream &
ostream::seekp( intt                i_nOffset,
                seek_dir            i_eStart )
{
    pData->seekp(i_nOffset, csv::seek_dir(int(i_eStart)) );
    return *this;
}

ostream::ostream( uintt i_nStartSize )
    :   pData( new StreamStr(i_nStartSize) )
{
}



}   // namespace csv

#endif




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
