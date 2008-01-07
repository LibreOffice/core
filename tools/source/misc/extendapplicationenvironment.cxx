/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: extendapplicationenvironment.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 09:48:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

#include "precompiled_tools.hxx"
#include "sal/config.h"

#include <stdlib.h>
    // not <cstdlib> as putenv is POSIX-only; setenv instead of putenv would be
    // better but is not supported by Solaris 9 and earlier

#if defined UNX
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#endif

#include "osl/process.h"
#include "osl/thread.h"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textcvt.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "tools/extendapplicationenvironment.hxx"

namespace tools {

void extendApplicationEnvironment() {
#if defined UNX
    // Try to set RLIMIT_NOFILE as large as possible (failure is harmless):
    rlimit l;
    if (getrlimit(RLIMIT_NOFILE, &l) == 0) {
        l.rlim_cur = l.rlim_max;
        setrlimit(RLIMIT_NOFILE, &l);
    }
#endif

    // Make sure URE_BOOTSTRAP environment variable is set (failure is fatal):
    if (getenv("URE_BOOTSTRAP") == NULL) {
        rtl::OUString p;
        if (osl_getExecutableFile(&p.pData) != osl_Process_E_None) {
            abort();
        }
        sal_Int32 i = p.lastIndexOf('/');
        if (i >= 0) {
            p = p.copy(0, i + 1);
        }
        rtl::OString s;
        if (!p.convertToString(
                &s, osl_getThreadTextEncoding(),
                RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR))
        {
            abort();
        }
        rtl::OString env(RTL_CONSTASCII_STRINGPARAM("URE_BOOTSTRAP="));
        env += s;
        env += SAL_CONFIGFILE("fundamental");
        rtl_string_acquire(env.pData); // argument to putenv must leak
        if (putenv(const_cast< char * >(env.getStr())) != 0) {
            abort();
        }
    }
}

}
