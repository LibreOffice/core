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
#include <cosv/persist.hxx>

// NOT FULLY DECLARED SERVICES
#include <cosv/ploc.hxx>


#ifdef WNT
#include <io.h>

namespace csv
{
namespace ploc
{

bool
Persistent::Exists() const
{
    return access( StrPath(), 00) == 0;
}

} // namespace ploc
} // namespace csv


#elif defined(UNX)
#include <unistd.h>

namespace csv
{
namespace ploc
{

bool
Persistent::Exists() const
{
    return access( StrPath(), F_OK ) == 0;
}


} // namespace ploc
} // namespace csv

#else
#error  For using csv::ploc there has to be defined: WNT or UNX.
#endif

namespace csv
{
namespace ploc
{

const char *
Persistent::StrPath() const
{
    if (sPath.empty() )
    {
#ifndef CSV_NO_MUTABLE
        StreamStr & rsPath = sPath;
#else
        StreamStr & rsPath = const_cast< StreamStr& >(sPath);
#endif
        rsPath.seekp(0);
        rsPath << MyPath();
        if (MyPath().IsDirectory())
            rsPath.pop_back(1);    // Remove closing delimiter.
    }
    return sPath.c_str();
}

} // namespace ploc
} // namespace csv




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
