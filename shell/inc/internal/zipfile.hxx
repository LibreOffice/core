/*************************************************************************
 *
 *  $RCSfile: zipfile.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-07 10:54:48 $
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

#ifndef ZIPFILE_HXX_INCLUDED
#define ZIPFILE_HXX_INCLUDED

#ifndef _WINDOWS
#define _WINDOWS
#endif

#ifndef _unz_H
#include <external/zlib/unzip.h>
#endif

#include <string>
#include <vector>
#include <memory>

/** A simple zip content provider based on the zlib
*/

class ZipFile
{
public:

    typedef std::vector<std::string>   Directory_t;
    typedef std::auto_ptr<Directory_t> DirectoryPtr_t;
    typedef std::vector<char>          ZipContentBuffer_t;

public:

    /** Checks whether a file is a zip file or not

    @precond    The given parameter must be a string with length > 0
            The file must exist
            The file must be readable for the current user

    @returns    true if the file is a zip file
            false if the file is not a zip file

    @throws ParameterException if the given file name is empty
            IOException if the specified file doesn't exist
            AccessViolationException if read access to the file is denied
    */
    static bool IsZipFile(const std::string& FileName);

    /** Returns wheter the version of the specified zip file may be uncompressed with the
          currently used zlib version or not

    @precond    The given parameter must be a string with length > 0
            The file must exist
            The file must be readable for the current user
            The file must be a valid zip file

    @returns    true if the file may be uncompressed with the currently used zlib
            false if the file may not be uncompressed with the currently used zlib

    @throws ParameterException if the given file name is empty
            IOException if the specified file doesn't exist or is no zip file
            AccessViolationException if read access to the file is denied
    */
    static bool IsValidZipFileVersionNumber(const std::string& FileName);


public:

    /** Constructs a zip file from a zip file

    @precond    The given parameter must be a string with length > 0
            The file must exist
            The file must be readable for the current user

    @throws ParameterException if the given file name is empty
            IOException if the specified file doesn't exist or is no valid zip file
            AccessViolationException if read access to the file is denied
            WrongZipVersionException if the zip file cannot be uncompressed
            with the used zlib version
    */
    ZipFile(const std::string& FileName);

    /** Destroys a zip file
    */
    ~ZipFile();

    /** Provides an interface to read the uncompressed data of a content of the zip file

    @param      ContentName
                The name of the content in the zip file

    @param      ppstm
                Pointer to pointer, will receive an interface pointer
                to IUnknown on success

    @precond    The specified content must exist in this file
                ppstm must not be NULL

    @throws     std::bad_alloc if the necessary buffer could not be
                allocated
                ZipException if an zip error occurs
                ZipContentMissException if the specified zip content
                does not exist in this zip file
    */
    void GetUncompressedContent(const std::string& ContentName, /*inout*/ ZipContentBuffer_t& ContentBuffer);

    /** Returns a list with the content names contained within this file

        @throws ZipException if an error in the zlib happens
    */
    DirectoryPtr_t GetDirectory() const;

    /** Convinience query function may even realized with
        iterating over a ZipFileDirectory returned by
        GetDirectory
    */
    bool HasContent(const std::string& ContentName) const;

private:

    /** Returns the length of the longest file name
        in the current zip file

        @throws ZipException if an zip error occurs
    */
    long GetFileLongestFileNameLength() const;

private:
    unzFile m_uzFile;
};

#endif

