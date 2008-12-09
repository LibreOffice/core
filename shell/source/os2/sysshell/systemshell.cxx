/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: systemshell.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"

#ifndef _SYSTEMSHELL_HXX_
#include "../../../inc/systemshell.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

namespace SystemShell
{

    /** Add a file to the system shells recent document list if there is any.
          This function may have no effect under Unix because there is no
          standard API among the different desktop managers.

          @param aFileUrl
                    The file url of the document.
    */
    void AddToRecentDocumentList(const rtl::OUString& aFileUrl, const rtl::OUString& aMimeType)
    {
        rtl::OUString system_path;
        osl::FileBase::RC rc = osl::FileBase::getSystemPathFromFileURL(aFileUrl, system_path);

        OSL_ENSURE(osl::FileBase::E_None == rc, "Invalid file url");

        //if (osl::FileBase::E_None == rc)
        //    SHAddToRecentDocs(SHARD_PATHW, system_path.getStr());
    }

}; // end namespace SystemShell

