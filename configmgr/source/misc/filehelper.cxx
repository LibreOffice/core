/*************************************************************************
 *
 *  $RCSfile: filehelper.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:32:12 $
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
        FileBase::RC eError = File::remove(_aURL);
        if (eError != osl_File_E_None &&
            eError != osl_File_E_NOENT)
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
        FileBase::RC eError = File::move(_aFromURL, _aToURL);
        if (eError != osl_File_E_None &&
            eError != osl_File_E_NOENT)
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
        return DirectoryItem::get(_sFileURL, aItem) == osl_File_E_None;
    }

    // -----------------------------------------------------------------------------
    bool FileHelper::dirExists(rtl::OUString const& _sDirURL)
    {
        return Directory(_sDirURL).open() == osl_File_E_None;
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
        case osl_File_E_None:
            break;

        case osl_File_E_PERM:
            aRet = ASCII("Operation not permitted");
            break;

        case osl_File_E_NOENT:
            aRet = ASCII("No such file or directory");
            break;

        case osl_File_E_SRCH:
            aRet = ASCII("unknown error: osl_File_E_SRCH");
            break;

        case osl_File_E_INTR:
            aRet = ASCII("function call was interrupted");
            break;

        case osl_File_E_IO:
            aRet = ASCII("I/O error");
            break;

        case osl_File_E_NXIO:
            aRet = ASCII("No such device or address");
            break;

        case osl_File_E_2BIG:
            aRet = ASCII("unknown error: osl_File_E_2BIG");
            break;

        case osl_File_E_NOEXEC:
            aRet = ASCII("unknown error: osl_File_E_NOEXEC");
            break;

        case osl_File_E_BADF:
            aRet = ASCII("Bad file");
            break;

        case osl_File_E_CHILD:
            aRet = ASCII("unknown error: osl_File_E_CHILD");
            break;

        case osl_File_E_AGAIN:
            aRet = ASCII("Operation would block");
            break;

        case osl_File_E_NOMEM:
            aRet = ASCII("not enough memory for allocating structures");
            break;

        case osl_File_E_ACCES:
            aRet = ASCII("Permission denied");
            break;

        case osl_File_E_FAULT:
            aRet = ASCII("Bad address");
            break;

        case osl_File_E_BUSY:
            aRet = ASCII("Text file busy");
            break;

        case osl_File_E_EXIST:
            aRet = ASCII("File exists");
            break;

        case osl_File_E_XDEV:
            aRet = ASCII("unknown error: osl_File_E_XDEV");
            break;

        case osl_File_E_NODEV:
            aRet = ASCII("No such device");
            break;

        case osl_File_E_NOTDIR:
            aRet = ASCII("Not a directory");
            break;

        case osl_File_E_ISDIR:
            aRet = ASCII("Is a director");
            break;

        case osl_File_E_INVAL:
            aRet = ASCII("the format of the parameters was not valid");
            break;

        case osl_File_E_NFILE:
            aRet = ASCII("too many open files in the system");
            break;

        case osl_File_E_MFILE:
            aRet = ASCII("too many open files used by the process");
            break;

        case osl_File_E_NOTTY:
            aRet = ASCII("unknown error: osl_File_E_NOTTY");
            break;

        case osl_File_E_FBIG:
            aRet = ASCII("File too large");
            break;

        case osl_File_E_NOSPC:
            aRet = ASCII("No space left on device");
            break;

        case osl_File_E_SPIPE:
            aRet = ASCII("unknown error: osl_File_E_SPIPE");
            break;

        case osl_File_E_ROFS:
            aRet = ASCII("Read-only file system");
            break;

        case osl_File_E_MLINK:
            aRet = ASCII("Too many links");
            break;

        case osl_File_E_PIPE:
            aRet = ASCII("unknown error: osl_File_E_PIPE");
            break;

        case osl_File_E_DOM:
            aRet = ASCII("unknown error: osl_File_E_DOM");
            break;

        case osl_File_E_RANGE:
            aRet = ASCII("unknown error: osl_File_E_RANGE");
            break;

        case osl_File_E_DEADLK:
            aRet = ASCII("unknown error: osl_File_E_DEADLK");
            break;

        case osl_File_E_NAMETOOLONG:
            aRet = ASCII("File name too long");
            break;

        case osl_File_E_NOLCK:
            aRet = ASCII("No record locks available");
            break;

        case osl_File_E_NOSYS:
            aRet = ASCII("Function not implemente");
            break;

        case osl_File_E_NOTEMPTY:
            aRet = ASCII("Directory not empt");
            break;

        case osl_File_E_LOOP:
            aRet = ASCII("Too many symbolic links encountered");
            break;

        case osl_File_E_ILSEQ:
            aRet = ASCII("unknown error: osl_File_E_ILSEQ");
            break;

        case osl_File_E_NOLINK:
            aRet = ASCII("Link has been severed");
            break;

        case osl_File_E_MULTIHOP:
            aRet = ASCII("Multihop attempted");
            break;

        case osl_File_E_USERS:
            aRet = ASCII("unknown error: osl_File_E_USERS");
            break;

        case osl_File_E_OVERFLOW:
            aRet = ASCII("Value too large for defined data type");
            break;

            /* unmapped error: always last entry in enum! */
        default: OSL_ENSURE(false, "Found unknown OSL File Error");
        case osl_File_E_invalidError:
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
