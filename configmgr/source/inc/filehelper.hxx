/*************************************************************************
 *
 *  $RCSfile: filehelper.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:28:39 $
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

#ifndef _CONFIGMGR_FILEHELPER_HXX_
#define _CONFIGMGR_FILEHELPER_HXX_

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif

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

        /// string representation of the delimiter
        const rtl::OUString& delimiterAsString();

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
        void replaceFile(const rtl::OUString& _aToURL, const rtl::OUString &_aFromURL) CFG_THROW1(io::IOException);

        /** removes a file specified by _aURL. Ignores the case of a non-existing file.
        */
        void removeFile(const rtl::OUString& _aURL) CFG_THROW1(io::IOException);

        /** creates an error msg string for a given file error return code.
        */
        rtl::OUString createOSLErrorString(osl::FileBase::RC eError);

        /** determines the modification time of a directory entry specified by a URL.
            @return the TimeValue of the last modification, if the file exists, otherwise a TimeValue(0,0).
        */
        TimeValue getModifyTime(rtl::OUString const& _aNormalizedFilename);
    }
} // namespace configmgr

#endif
