/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
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

#ifndef AUTOREGISTER_HXX
#define AUTOREGISTER_HXX

#include <stdio.h>
#include <vector>
#include <ostream>
#include <iostream>

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

#include "dynamicregister.hxx"

// -----------------------------------------------------------------------------
// ---------------------------------- Version ----------------------------------
// -----------------------------------------------------------------------------
// extern "C" const struct VersionInfo *GetVersionInfo();

// this is a Hack, but at the moment, no change to handle this behaviour better.
struct VersionInfo
{
    const char* pTime;
    const char* pDate;
    const char* pUpd;
    const char* pMinor;
    const char* pBuild;
    const char* pInpath;
};
typedef const struct VersionInfo* (*FktGetVersionInfoPtr)();

// -----------------------------------------------------------------------------
class VersionHelper : public DynamicLibraryHelper
{
    const VersionInfo *m_pInfo;

public:
    VersionHelper(rtl::OUString const& _sDLLName, GetOpt & _aOptions);
//  void print(std::ostream& _aStream);
    void printall(FILE*);

    rtl::OString getTime() const;
    rtl::OString getDate() const;
    rtl::OString getUpd() const;
    rtl::OString getMinor() const;
    rtl::OString getBuild() const;
    rtl::OString getInpath() const;

    bool isOk() const;
};

/** insertion operator for easy output */
// std::ostream &operator <<( std::ostream &stream,
//                           VersionHelper &version );


#endif

