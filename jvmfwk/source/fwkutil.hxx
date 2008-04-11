/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fwkutil.hxx,v $
 * $Revision: 1.15 $
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
#if !defined INCLUDED_JVMFWK_FWKUTIL_HXX
#define INCLUDED_JVMFWK_FWKUTIL_HXX

#include "sal/config.h"
#include "osl/mutex.hxx"
#include "rtl/bootstrap.hxx"
#include "rtl/instance.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/byteseq.hxx"
#include "osl/thread.hxx"



namespace jfw
{

/** Returns the file URL of the directory where the framework library
    (this library) resides.
*/
rtl::OUString getLibraryLocation();

/** provides a bootstrap class which already knows the values from the
    jvmfkwrc file.
*/
struct Bootstrap :
    public ::rtl::StaticWithInit< const rtl::Bootstrap *, Bootstrap > {
        const rtl::Bootstrap * operator () () {
            ::rtl::OUStringBuffer buf(256);
            buf.append(getLibraryLocation());
            buf.appendAscii(SAL_CONFIGFILE("/jvmfwk3"));
            ::rtl::OUString sIni = buf.makeStringAndClear();
            ::rtl::Bootstrap *  bootstrap = new ::rtl::Bootstrap(sIni);
#if OSL_DEBUG_LEVEL >=2
        rtl::OString o = rtl::OUStringToOString( sIni , osl_getThreadTextEncoding() );
        fprintf(stderr, "[Java framework] Using configuration file %s\n" , o.getStr() );
#endif
        return bootstrap;
    }
};

struct FwkMutex: public ::rtl::Static<osl::Mutex, FwkMutex> {};

//osl::Mutex * getFwkMutex();

rtl::ByteSequence encodeBase16(const rtl::ByteSequence& rawData);
rtl::ByteSequence decodeBase16(const rtl::ByteSequence& data);

rtl::OUString getPlatform();

//const rtl::Bootstrap& getBootstrap();


rtl::OUString getDirFromFile(const rtl::OUString& usFilePath);

rtl::OUString getFileFromURL(const rtl::OUString& sFileURL);


/** Returns the file URL of the folder where the executable resides.
 */
rtl::OUString getExecutableDirectory();
/** Locates the plugin library and returns the file URL.

    First tries to locate plugin relative to baseUrl (if relative);
    vnd.sun.star.expand URLs are supported.  If that fails, tries to
    locate plugin relative to the executable.  If that fails, and plugin
    contains no slashes, tries to locate plugin in a platform-specific way
    (e.g., LD_LIBRARY_PATH).

    @param baseUrl
    The base file URL relative to which the plugin argument is interpreted.

    @param plugin
    The argument is an absolute or relative URL or just the name of the plugin.
 */
rtl::OUString findPlugin(
    const rtl::OUString & baseUrl, const rtl::OUString & plugin);


enum FileStatus
{
    FILE_OK,
    FILE_DOES_NOT_EXIST,
    FILE_INVALID
};

/** checks if the URL is a file.

    If it is a link to a file than
    it is resolved. Assuming that the argument
    represents a relative URL then FILE_INVALID
    is returned.


    @return
    one of the values of FileStatus.

    @exception
    Errors occured during determining if the file exists
 */
FileStatus checkFileURL(const rtl::OUString & path);


struct PluginLibrary;
class VersionInfo;
class CJavaInfo;

bool isAccessibilitySupportDesired();

rtl::OUString buildClassPathFromDirectory(const rtl::OUString & relPath);

rtl::OUString retrieveClassPath( ::rtl::OUString const & macro );
}
#endif
