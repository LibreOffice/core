/*************************************************************************
 *
 *  $RCSfile: systemshell.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 14:55:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SYSTEMSHELL_HXX_
#include "systemshell.hxx"
#endif

#include "osl/module.hxx"

const rtl::OUString SYM_ADD_TO_RECENTLY_USED_FILE_LIST = rtl::OUString::createFromAscii("add_to_recently_used_file_list");
const rtl::OUString LIB_RECENT_FILE = rtl::OUString::createFromAscii("librecentfile.so");

namespace SystemShell {

    typedef void (*PFUNC_ADD_TO_RECENTLY_USED_LIST)(const rtl::OUString&, const rtl::OUString&);

    //##############################
    rtl::OUString get_absolute_library_url(const rtl::OUString& lib_name)
    {
        rtl::OUString url;
        if (osl::Module::getUrlFromAddress(reinterpret_cast<void*>(AddToRecentDocumentList), url))
        {
            sal_Int32 index = url.lastIndexOf('/');
            url = url.copy(0, index + 1);
            url += LIB_RECENT_FILE;
        }
        return url;
    }

    //##############################
    void AddToRecentDocumentList(const rtl::OUString& aFileUrl, const rtl::OUString& aMimeType)
    {
        rtl::OUString librecentfile_url = get_absolute_library_url(LIB_RECENT_FILE);

        if (librecentfile_url.getLength())
        {
            osl::Module module(librecentfile_url);

            if (module.is())
            {
                PFUNC_ADD_TO_RECENTLY_USED_LIST add_to_recently_used_file_list =
                    reinterpret_cast<PFUNC_ADD_TO_RECENTLY_USED_LIST>(module.getSymbol(SYM_ADD_TO_RECENTLY_USED_FILE_LIST));

                if (add_to_recently_used_file_list)
                    add_to_recently_used_file_list(aFileUrl, aMimeType);
            }
        }
    }

} // namespace SystemShell

