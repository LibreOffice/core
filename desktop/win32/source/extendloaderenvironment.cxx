/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: extendloaderenvironment.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-15 13:01:32 $
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

#include "precompiled_desktop.hxx"
#include "sal/config.h"

#include <cstddef>
#include <stdlib.h> // problems with cstdlib std::abort

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <shlwapi.h>
#include <wininet.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include "rtl/string.h"
#include "sal/delayload.hxx"

#include "extendloaderenvironment.hxx"

namespace desktop_win32 {

void extendLoaderEnvironment() {
    char programPath[MAX_PATH];
    char * programPathEnd = delayload::getModulePath(programPath, NULL);
    if (programPathEnd == NULL) {
        abort();
    }
    *programPathEnd = '\0';
    std::size_t const envSepLen = RTL_CONSTASCII_LENGTH(";");
    std::size_t const maxEnv = 32767;
    char pad[MAX_PATH - 1 + envSepLen + maxEnv];
        // hopefully std::size_t is large enough to not overflow
    char * ureBinPathEnd = delayload::getUreBinPath(
        pad, programPath, programPathEnd);
    //TODO  As long as there is an old style OOo that has no ure-link, do
    // nothing if ureBinPath cannot be determined; once there is only a new
    // style OOo that always has to have a ure-link, fail if ureBinPath cannot
    // be determined:
    if (ureBinPathEnd != NULL) {
        DWORD n = GetEnvironmentVariableA(
            "PATH", ureBinPathEnd + envSepLen, maxEnv);
        if (n >= maxEnv) {
            abort();
        }
        bool set = true;
        if (n == 0) {
            if (GetLastError() != ERROR_ENVVAR_NOT_FOUND) {
                abort();
            }
        } else {
            // Do not add ureBinPath to the PATH if it is already there; this
            // check fails if a differently spelled equivalent of ureBinPath is
            // already there, but the assumption is that if multiple application
            // loaders recursively execute this code the spelling of ureBinPath
            // will be equivalent, so PATH cannot grow unbounded:
            std::ptrdiff_t ureBinPathLen = ureBinPathEnd - pad;
            for (char const * p = ureBinPathEnd + envSepLen;;) {
                char const * q = p + 1;
                while (*q != '\0' && *q != ';') {
                    ++q;
                }
                if (q - p == ureBinPathLen &&
                    strncmp(p, pad, ureBinPathLen) == 0)
                {
                    set = false;
                    break;
                }
                p = q;
                if (*p == '\0') {
                    break;
                }
                ++p;
            }
            if (set) {
                *ureBinPathEnd = ';';
            }
        }
        if (set && !SetEnvironmentVariableA("PATH", pad)) {
            abort();
        }
        if (GetEnvironmentVariableA("URE_BOOTSTRAP", pad, 0) == 0) {
            if (GetLastError() != ERROR_ENVVAR_NOT_FOUND) {
                abort();
            } else {
                static char const suffix[] = "/fundamental.ini";
                char url[INTERNET_MAX_URL_LENGTH + RTL_CONSTASCII_LENGTH(suffix)
                         + 1];
                    // hopefully std::size_t is large enough to not overflow;
                    // unclear whether UrlCreateFromPathA counts
                    // INTERNET_MAX_URL_LENGTH including or excluding the
                    // terminating null character, so +1 for safety
                DWORD n = INTERNET_MAX_URL_LENGTH;
                if (UrlCreateFromPathA(programPath, url, &n, 0) != S_OK) {
                    abort();
                }
                char * p = url + n; // cannot overflow
                strcpy(p, suffix + (p == url || p[-1] != '/' ? 0 : 1));
                    // cannot overflow
                if (!SetEnvironmentVariableA("URE_BOOTSTRAP", url)) {
                    abort();
                }
            }
        }
    }
}

}
