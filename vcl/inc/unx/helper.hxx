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

#ifndef INCLUDED_VCL_INC_UNX_HELPER_HXX
#define INCLUDED_VCL_INC_UNX_HELPER_HXX

#include <vector>

#include <vcl/dllapi.h>
#include <vcl/vclevent.hxx>

#include <rtl/ustring.hxx>


// forwards
namespace osl { class File; }

namespace psp
{

void getPrinterPathList( std::vector< OUString >& rPathList, const char* pSubDir );

OUString const & getFontPath();

// normalized path (equivalent to realpath)
void normPath( OString& rPath );

// splits rOrgPath into dirname and basename
// rOrgPath will be subject to normPath
void splitPath( OString& rOrgPath, OString& rDir, OString& rBase );

enum class whichOfficePath { InstallationRootPath, UserPath, ConfigPath };

OUString getOfficePath( whichOfficePath ePath );

} // namespace


#endif // INCLUDED_VCL_INC_UNX_HELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
