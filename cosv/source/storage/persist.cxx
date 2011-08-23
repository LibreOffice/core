/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <precomp.h>
#include <cosv/persist.hxx>

// NOT FULLY DECLARED SERVICES
#include <cosv/streamstr.hxx>
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

/*
#ifndef __SUNPRO_CC
#include <unistd.h>
#else
#define	F_OK	0	// Test for existence of File
extern int access(const char *, int);
#endif
*/

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
