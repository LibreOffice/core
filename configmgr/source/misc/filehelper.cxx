/*************************************************************************
 *
 *  $RCSfile: filehelper.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 13:24:48 $
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


#ifndef _RTL_USTRING_H_
#include <rtl/ustring.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _CONFIGMGR_FILEHELPER_HXX_
#include "filehelper.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#define ASCII(x) rtl::OUString::createFromAscii(x)

namespace configmgr
{
    using namespace ::osl;

    //==========================================================================
    //= FileHelper
    //==========================================================================

    // -----------------------------------------------------------------------------
    const rtl::OUString& FileHelper::delimiterAsString()
    {
        static const rtl::OUString aStringDelimiter( &FileHelper::delimiter,1);
        return aStringDelimiter;
    }

    // -----------------------------------------------------------------------------
    void FileHelper::removeFile(const rtl::OUString& _aURL) CFG_THROW1 (io::IOException)
    {
        File::RC eError = File::remove(_aURL);
        if (eError != File::E_None &&
            eError != File::E_NOENT)
        {
            rtl::OUStringBuffer sErrorBuf;
            sErrorBuf.appendAscii("Configmgr: removeFile failed ");
            sErrorBuf.appendAscii("for file \"").append(_aURL).appendAscii("\". ");
            sErrorBuf.appendAscii("Error = \"").append(FileHelper::createOSLErrorString(eError)).appendAscii("\" ");
            sErrorBuf.appendAscii("[").append(sal_Int32(eError)).appendAscii("] ");

            rtl::OUString const sError = sErrorBuf.makeStringAndClear();
            OSL_ENSURE(0, rtl::OUStringToOString(sError,RTL_TEXTENCODING_ASCII_US).getStr());
            throw io::IOException(sError, NULL);
        }
    }

    // -----------------------------------------------------------------------------
    void FileHelper::replaceFile(
        const rtl::OUString& _aToURL, const rtl::OUString &_aFromURL) CFG_THROW1 (io::IOException)
    {
        File::remove(_aToURL);
        File::RC eError = File::move(_aFromURL, _aToURL);
        if (eError != File::E_None &&
            eError != File::E_NOENT)
        {
            rtl::OUStringBuffer sErrorBuf;
            sErrorBuf.appendAscii("Configmgr: replaceFile failed ");
            sErrorBuf.appendAscii("for replacing file \"").append(_aFromURL).appendAscii("\". ");
            sErrorBuf.appendAscii("by file \"").append(_aToURL).appendAscii("\". ");
            sErrorBuf.appendAscii("Error = \"").append(FileHelper::createOSLErrorString(eError)).appendAscii("\" ");
            sErrorBuf.appendAscii("[").append(sal_Int32(eError)).appendAscii("] ");

            rtl::OUString const sError = sErrorBuf.makeStringAndClear();
            OSL_ENSURE(0, rtl::OUStringToOString(sError,RTL_TEXTENCODING_ASCII_US).getStr());
            throw io::IOException(sError, NULL);
        }
    }

    // -----------------------------------------------------------------------------
    bool FileHelper::fileExists(rtl::OUString const& _sFileURL)
    {
        DirectoryItem aItem;
        return DirectoryItem::get(_sFileURL, aItem) == Directory::E_None;
    }

    // -----------------------------------------------------------------------------
    bool FileHelper::dirExists(rtl::OUString const& _sDirURL)
    {
        return Directory(_sDirURL).open() == Directory::E_None;
    }

    // -----------------------------------------------------------------------------
    sal_uInt64 FileHelper::getModifyStatus(rtl::OUString const& _sURL, TimeValue & rModifyTime)
    {
        static const TimeValue k_NullTime = {0,0};
        sal_uInt64 aSize = 0;
        rModifyTime = k_NullTime;

        DirectoryItem aItem;
        if (osl::FileBase::E_None == DirectoryItem::get(_sURL, aItem))
        {
            FileStatus aStatus(osl_FileStatus_Mask_ModifyTime|osl_FileStatus_Mask_Type|osl_FileStatus_Mask_FileSize);
            if (osl::FileBase::E_None == aItem.getFileStatus(aStatus))
            {
                if (aStatus.isValid(osl_FileStatus_Mask_ModifyTime))
                    rModifyTime = aStatus.getModifyTime();

                if (aStatus.isValid(osl_FileStatus_Mask_FileSize))
                    aSize = aStatus.getFileSize();
            }
        }
        return aSize;
    }

    // -----------------------------------------------------------------------------
    TimeValue FileHelper::getModifyTime(rtl::OUString const& _sURL)
    {
        TimeValue aTime = {0,0};
        DirectoryItem aItem;
        if (osl::FileBase::E_None == DirectoryItem::get(_sURL, aItem))
        {
            FileStatus aStatus(osl_FileStatus_Mask_ModifyTime|osl_FileStatus_Mask_Type);
            if (osl::FileBase::E_None == aItem.getFileStatus(aStatus) && aStatus.isValid(osl_FileStatus_Mask_ModifyTime))
                aTime = aStatus.getModifyTime();
        }
        return aTime;
    }

    // -----------------------------------------------------------------------------
    rtl::OUString FileHelper::createOSLErrorString(FileBase::RC eError)
    {
        rtl::OUString aRet;
        switch(eError)
        {
        case FileBase::E_None:
            break;

        case FileBase::E_PERM:
            aRet = ASCII("Operation not permitted");
            break;

        case FileBase::E_NOENT:
            aRet = ASCII("No such file or directory");
            break;

        case FileBase::E_SRCH:
            aRet = ASCII("unknown error: osl_File_E_SRCH");
            break;

        case FileBase::E_INTR:
            aRet = ASCII("function call was interrupted");
            break;

        case FileBase::E_IO:
            aRet = ASCII("I/O error");
            break;

        case FileBase::E_NXIO:
            aRet = ASCII("No such device or address");
            break;

        case FileBase::E_2BIG:
            aRet = ASCII("unknown error: osl_File_E_2BIG");
            break;

        case FileBase::E_NOEXEC:
            aRet = ASCII("unknown error: osl_File_E_NOEXEC");
            break;

        case FileBase::E_BADF:
            aRet = ASCII("Bad file");
            break;

        case FileBase::E_CHILD:
            aRet = ASCII("unknown error: osl_File_E_CHILD");
            break;

        case FileBase::E_AGAIN:
            aRet = ASCII("Operation would block");
            break;

        case FileBase::E_NOMEM:
            aRet = ASCII("not enough memory for allocating structures");
            break;

        case FileBase::E_ACCES:
            aRet = ASCII("Permission denied");
            break;

        case FileBase::E_FAULT:
            aRet = ASCII("Bad address");
            break;

        case FileBase::E_BUSY:
            aRet = ASCII("Text file busy");
            break;

        case FileBase::E_EXIST:
            aRet = ASCII("File exists");
            break;

        case FileBase::E_XDEV:
            aRet = ASCII("unknown error: osl_File_E_XDEV");
            break;

        case FileBase::E_NODEV:
            aRet = ASCII("No such device");
            break;

        case FileBase::E_NOTDIR:
            aRet = ASCII("Not a directory");
            break;

        case FileBase::E_ISDIR:
            aRet = ASCII("Is a director");
            break;

        case FileBase::E_INVAL:
            aRet = ASCII("the format of the parameters was not valid");
            break;

        case FileBase::E_NFILE:
            aRet = ASCII("too many open files in the system");
            break;

        case FileBase::E_MFILE:
            aRet = ASCII("too many open files used by the process");
            break;

        case FileBase::E_NOTTY:
            aRet = ASCII("unknown error: osl_File_E_NOTTY");
            break;

        case FileBase::E_FBIG:
            aRet = ASCII("File too large");
            break;

        case FileBase::E_NOSPC:
            aRet = ASCII("No space left on device");
            break;

        case FileBase::E_SPIPE:
            aRet = ASCII("unknown error: osl_File_E_SPIPE");
            break;

        case FileBase::E_ROFS:
            aRet = ASCII("Read-only file system");
            break;

        case FileBase::E_MLINK:
            aRet = ASCII("Too many links");
            break;

        case FileBase::E_PIPE:
            aRet = ASCII("unknown error: osl_File_E_PIPE");
            break;

        case FileBase::E_DOM:
            aRet = ASCII("unknown error: osl_File_E_DOM");
            break;

        case FileBase::E_RANGE:
            aRet = ASCII("unknown error: osl_File_E_RANGE");
            break;

        case FileBase::E_DEADLK:
            aRet = ASCII("unknown error: osl_File_E_DEADLK");
            break;

        case FileBase::E_NAMETOOLONG:
            aRet = ASCII("File name too long");
            break;

        case FileBase::E_NOLCK:
            aRet = ASCII("No record locks available");
            break;

        case FileBase::E_NOSYS:
            aRet = ASCII("Function not implemente");
            break;

        case FileBase::E_NOTEMPTY:
            aRet = ASCII("Directory not empt");
            break;

        case FileBase::E_LOOP:
            aRet = ASCII("Too many symbolic links encountered");
            break;

        case FileBase::E_ILSEQ:
            aRet = ASCII("unknown error: osl_File_E_ILSEQ");
            break;

        case FileBase::E_NOLINK:
            aRet = ASCII("Link has been severed");
            break;

        case FileBase::E_MULTIHOP:
            aRet = ASCII("Multihop attempted");
            break;

        case FileBase::E_USERS:
            aRet = ASCII("unknown error: osl_File_E_USERS");
            break;

        case FileBase::E_OVERFLOW:
            aRet = ASCII("Value too large for defined data type");
            break;

            /* unmapped error: always last entry in enum! */
        default: OSL_ENSURE(false, "Found unknown OSL File Error");
        case FileBase::E_invalidError:
            aRet = ASCII("unmapped Error");
            break;
        }
        return aRet;
    }

    // -----------------------------------------------------------------------------
    rtl::OUString FileHelper::getParentDir(rtl::OUString const& _sURL)
    {
        rtl::OUString parentDirectory ;
        rtl::OUString fileName ;

        splitFileUrl(_sURL, parentDirectory, fileName) ;
        return parentDirectory ;
    }

    // -----------------------------------------------------------------------------
    void FileHelper::splitFileUrl(const rtl::OUString& aFileUrl,
                                  rtl::OUString& aParentDirectory,
                                  rtl::OUString& aFileName) {
        // goto last '/' and cut the rest.
        sal_Int32 nIdx = aFileUrl.lastIndexOf(delimiter, aFileUrl.getLength());
        if (nIdx > 0) {
            aParentDirectory = aFileUrl.copy(0, nIdx);
            aFileName = aFileUrl.copy(nIdx + 1) ;
        }
        else {
            aParentDirectory = rtl::OUString() ;
            aFileName = aFileUrl ;
        }
    }

    // -----------------------------------------------------------------------------
    rtl::OUString FileHelper::getFileName(const rtl::OUString& aFileUrl) {
        rtl::OUString parentDirectory ;
        rtl::OUString fileName ;

        splitFileUrl(aFileUrl, parentDirectory, fileName) ;
        return fileName ;
    }
    // -----------------------------------------------------------------------------
    osl::FileBase::RC FileHelper::mkdir(rtl::OUString const& _sDirURL)
    {
        // direct create a directory
        osl::FileBase::RC eError = osl::Directory::create(_sDirURL); // try to create the directory
        if (eError == osl::FileBase::E_EXIST ||
            eError == osl::FileBase::E_None ||
            FileHelper::dirExists(_sDirURL))
        {
            eError = osl::FileBase::E_None; // Exists or created
        }
        return eError;
    }

    // -----------------------------------------------------------------------------
    osl::FileBase::RC FileHelper::mkdirs(rtl::OUString const& _sDirURL)
    {
        osl::FileBase::RC eError = mkdir(_sDirURL);
        switch (eError)
        {
        case osl::FileBase::E_EXIST: OSL_ASSERT(false);
        case osl::FileBase::E_None:
            break;

        case osl::FileBase::E_NOENT:
            {
                rtl::OUString sParentDir = FileHelper::getParentDir(_sDirURL);
                if (sParentDir.getLength() == 0)
                    break;

                eError = mkdirs(sParentDir);
                if (eError != osl::FileBase::E_None)
                    break;

                eError = mkdir(_sDirURL);
            }
            break;

        default: OSL_TRACE("configmgr: Could not create directory (%d).", int(eError));
            break;
        }
        return eError;
    }

} // namespace configmgr
