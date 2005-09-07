/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fwkutil.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:35:48 $
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
#if !defined INCLUDED_JVMFWK_FWKUTIL_HXX
#define INCLUDED_JVMFWK_FWKUTIL_HXX

#include "osl/mutex.hxx"
#include "rtl/bootstrap.hxx"
namespace jfw
{
osl::Mutex * getFwkMutex();

rtl::ByteSequence encodeBase16(const rtl::ByteSequence& rawData);
rtl::ByteSequence decodeBase16(const rtl::ByteSequence& data);

rtl::OUString getPlatform();
/** provides a bootstrap class which already knows the values from the
    jvmfkwrc file.
*/
const rtl::Bootstrap& getBootstrap();


rtl::OUString getDirFromFile(const rtl::OUString& usFilePath);

rtl::OUString getFileFromURL(const rtl::OUString& sFileURL);

/** Returns the file URL of the directory where the framework library
    (this library) resides.
*/
rtl::OUString getLibraryLocation();

/** Returns the file URL of the folder where the executable resides.
 */
rtl::OUString getExecutableDirectory();
/** Locates the plugin library and returns the file URL.

    First tries to locate plugin relative to baseUrl.  If that fails, tries to
    locate plugin relative to the executable.  If that fails, and plugin
    contains no slashes, tries to locate plugin in a platform-specific way
    (e.g., LD_LIBRARY_PATH).

    @param baseUrl
    The base file URL relative to which the plugin argument is interpreted.

    @param plugin
    The argument is just the name of the plugin or a relative path
    from the directory of the executable.
 */
rtl::OUString findPlugin(
    const rtl::OUString & baseUrl, const rtl::OUString & plugin);
//Todo still needed?
rtl::OUString searchFileNextToThisLib(const rtl::OUString & sFile);
class CNodeJava;


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
