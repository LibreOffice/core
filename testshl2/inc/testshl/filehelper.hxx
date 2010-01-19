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

#ifndef FILEHELPER_HXX
#define FILEHELPER_HXX

#include <string>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <testshl/cmdlinebits.hxx>
#include "getopt.hxx"

namespace FileHelper
{

    std::string getTempPath();
    rtl::OUString convertPath( rtl::OUString const& _suSysPath );
    rtl::OUString convertPath( rtl::OString const& sysPth );
    CmdLineBits createFlags( GetOpt & _aOptions );

}
#endif

