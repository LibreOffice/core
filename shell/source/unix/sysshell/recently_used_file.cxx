/*************************************************************************
 *
 *  $RCSfile: recently_used_file.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 14:54:45 $
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

#ifndef INCLUDED_RECENTLY_USED_FILE
#include "recently_used_file.hxx"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

#ifndef _OSL_SECURITY_HXX_
#include <osl/security.hxx>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#include <sys/file.h>

#include <unistd.h>

const rtl::OUString RECENTLY_USED_FILE_NAME = rtl::OUString::createFromAscii(".recently-used");
const rtl::OUString SLASH = rtl::OUString::createFromAscii("/");

namespace /* private */ {

inline void ensure_final_slash(/*inout*/ rtl::OUString& path)
{
    if ((path.getLength() > 0) &&
        (SLASH.pData->buffer[0] != path.pData->buffer[path.getLength() - 1]))
        path += SLASH;
}

} // namespace private

//------------------------------------------------
recently_used_file::recently_used_file() :
    file_(NULL)
{
    osl::Security sec;
    rtl::OUString homedir_url;

    if (sec.getHomeDir(homedir_url))
    {
        rtl::OUString homedir;
        osl::FileBase::getSystemPathFromFileURL(homedir_url, homedir);

        rtl::OUString rufn = homedir;
        ensure_final_slash(rufn);
        rufn += RECENTLY_USED_FILE_NAME;

        rtl::OString tmp =
            rtl::OUStringToOString(rufn, osl_getThreadTextEncoding());

        file_ = fopen(tmp.getStr(), "r+");

        /* create if not exist */
        if (NULL == file_)
            file_ = fopen(tmp.getStr(), "w+");

        if (NULL == file_)
            throw "I/O error opening ~/.recently-used";

        if (lockf(fileno(file_), F_LOCK, 0) != 0)
        {
            fclose(file_);
            throw "Cannot lock ~/.recently-used";
        }
    }
    else
        throw "Cannot determine user home directory";
}

//------------------------------------------------
recently_used_file::~recently_used_file()
{
    lockf(fileno(file_), F_ULOCK, 0);
    fclose(file_);
}

//------------------------------------------------
void recently_used_file::reset() const
{
    rewind(file_);
}

//------------------------------------------------
void recently_used_file::flush()
{
    fflush(file_);
}

//------------------------------------------------
void recently_used_file::truncate(off_t length)
{
    ftruncate(fileno(file_), length);
}

//------------------------------------------------
size_t recently_used_file::read(char* buffer, size_t size) const
{
    size_t  r = fread(reinterpret_cast<void*>(buffer), sizeof(char), size, file_);

    if ((r < size) && ferror(file_))
        throw "I/O error: read failed";

    return r;
}

//----------------------------
void recently_used_file::write(const char* buffer, size_t size) const
{
    if (size != fwrite(reinterpret_cast<const void*>(buffer), sizeof(char), size, file_))
        throw "I/O error: write failed";
}

//----------------------------
bool recently_used_file::eof() const
{
    return feof(file_);
}




