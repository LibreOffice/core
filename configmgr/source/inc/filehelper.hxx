/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: filehelper.hxx,v $
 * $Revision: 1.14 $
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

#ifndef _CONFIGMGR_FILEHELPER_HXX_
#define _CONFIGMGR_FILEHELPER_HXX_

#include "utility.hxx"
#include <osl/file.hxx>
#include <com/sun/star/io/IOException.hpp>

namespace io = com::sun::star::io;

namespace configmgr
{
    //==========================================================================
    //= FileHelper
    //==========================================================================
    /** Within the FileHelper namespace there is a list of methods declared, which ease
        specific file operations.
    */
    namespace FileHelper
    {
        /// delimiter used in URLs and ConfPath
        static const ::sal_Unicode  delimiter = sal_Unicode('/');

        /// Tests if the file exists.
        bool fileExists(rtl::OUString const& _sFileURL);

        /// Tests if the directory exists.
        bool dirExists(rtl::OUString const& _sDirURL);

        /** Returns the parent part of the pathname of this File URL,
            or an empty string if the name has no parent part.
            The parent part is generally everything leading up to the last occurrence
            of the separator character.
        */
        rtl::OUString getParentDir(rtl::OUString const& _aFileURL);

        /**
          Returns the file name part of a file URL.

          @param aFileUrl   file URL
          @return   everything in the URL from the last delimiter on
          */
        rtl::OUString getFileName(const rtl::OUString& aFileUrl) ;

        /**
          Splits a file URL between its parent directory/file name
          parts.

          @param aFileUrl           file URL
          @param aParentDirectory   parent directory filled on return
          @param aFileName          file name filled on return
          */
        void splitFileUrl(const rtl::OUString& aFileUrl,
                          rtl::OUString& aParentDirectory,
                          rtl::OUString& aFileName) ;

        /** creates a directory whose pathname is specified by a FileURL.
            @return true if directory could be created or does exist, otherwise false.
        */
        osl::FileBase::RC mkdir(rtl::OUString const& _sDirURL);

        /** creates a directory whose pathname is specified by a FileURL,
            including any necessary parent directories.
            @return true if directory (or directories) could be created or do(es) exist, otherwise false.
        */
        osl::FileBase::RC mkdirs(rtl::OUString const& _aDirectory);

        /** replaces a file specified by _aToURL with a file specified by _aFromURL.
        */
        void replaceFile(const rtl::OUString& _aToURL, const rtl::OUString &_aFromURL) SAL_THROW((io::IOException));

        /** removes a file specified by _aURL. Ignores the case of a non-existing file.
        */
        bool tryToRemoveFile(const rtl::OUString& _aURL, bool tryBackupFirst);

        /** creates an error msg string for a given file error return code.
        */
        rtl::OUString createOSLErrorString(osl::FileBase::RC eError);

        /** determines the status of a directory entry specified by a URL.
            @return the Size of the file in bytes and the TimeValue of the last modification, if the file exists,
                    otherwise 0 and a TimeValue(0,0).
        */
        sal_uInt64 getModifyStatus(rtl::OUString const& _aNormalizedFilename, TimeValue & rModifyTime);
    }
} // namespace configmgr

#endif
