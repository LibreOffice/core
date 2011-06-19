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

#ifndef _PSPRINT_HELPER_HXX_
#define _PSPRINT_HELPER_HXX_

#include <list>

#include "vcl/dllapi.h"

#include "rtl/ustring.hxx"


// forwards
namespace osl { class File; }

namespace psp {
typedef int fontID;

void VCL_DLLPUBLIC getPrinterPathList( std::list< rtl::OUString >& rPathList, const char* pSubDir );

// note: gcc 3.4.1 warns about visibility if we retunr a const rtl::OUString& here
// seems to be a bug in gcc, now we return an object instead of a reference
rtl::OUString VCL_DLLPUBLIC getFontPath();

bool VCL_DLLPUBLIC convertPfbToPfa( osl::File& rInFile, osl::File& rOutFile );

// normalized path (equivalent to realpath)
void VCL_DLLPUBLIC normPath( rtl::OString& rPath );

// splits rOrgPath into dirname and basename
// rOrgPath will be subject to normPath
void VCL_DLLPUBLIC splitPath( rtl::OString& rOrgPath, rtl::OString& rDir, rtl::OString& rBase );

enum whichOfficePath { NetPath, UserPath, ConfigPath };
// note: gcc 3.4.1 warns about visibility if we retunr a const rtl::OUString& here
// seems to be a bug in gcc, now we return an object instead of a reference
rtl::OUString VCL_DLLPUBLIC getOfficePath( enum whichOfficePath ePath );
} // namespace

#endif // _PSPRINT_HELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
