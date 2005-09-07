/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: systemshell.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:56:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
        if (osl::Module::getUrlFromAddress(reinterpret_cast<oslGenericFunction>(AddToRecentDocumentList), url))
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
               // convert from reinterpret_cast<PFUNC_ADD_TO_RECENTLY_USED_LIST>
               // not allowed in gcc 3.3 without permissive.
                PFUNC_ADD_TO_RECENTLY_USED_LIST add_to_recently_used_file_list =
                    reinterpret_cast<PFUNC_ADD_TO_RECENTLY_USED_LIST>(module.getFunctionSymbol(SYM_ADD_TO_RECENTLY_USED_FILE_LIST));

                if (add_to_recently_used_file_list)
                    add_to_recently_used_file_list(aFileUrl, aMimeType);
            }
        }
    }

} // namespace SystemShell

