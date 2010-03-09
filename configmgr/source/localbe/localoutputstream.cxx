/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#include "localoutputstream.hxx"
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>

namespace configmgr { namespace localbe {

//==============================================================================

//------------------------------------------------------------------------------

LocalOutputStream::LocalOutputStream(const rtl::OUString& aFileUrl)
    throw (backend::BackendAccessException, uno::RuntimeException)
: mFileUrl(aFileUrl)
, mTemporaryFileUrl(mFileUrl)
, mWriteFile(NULL)
{
    // First, ensure the directory where the file is supposed to be
    // put exists.
    mTemporaryFileUrl += rtl::OUString::createFromAscii("_tmp") ;
    rtl::OUString parentDirectory = FileHelper::getParentDir(aFileUrl) ;

    if (osl::File::RC errorCode = FileHelper::mkdirs(parentDirectory))
    {
        rtl::OUStringBuffer message ;
        message.appendAscii("Cannot create directory \"") ;
        message.append(parentDirectory).appendAscii("\". Error is ") ;
        message.append(FileHelper::createOSLErrorString(errorCode));
        message.appendAscii(" [").append(sal_Int32(errorCode)).appendAscii("].") ;

        rtl::OUString const sIOMsg = message.makeStringAndClear();
        uno::Any ioe = uno::makeAny(io::IOException(sIOMsg,0));

        switch (errorCode)
        {
        case osl::File::E_ACCES:
        case osl::File::E_ROFS:
            message.appendAscii("Configuration LocalOutputStream - No Write Access: ");
            message.append(sIOMsg);
            throw backend::InsufficientAccessRightsException(message.makeStringAndClear(), NULL, ioe) ;

        case osl::File::E_None: OSL_ASSERT(!"can't happen");
        default:
            message.appendAscii("Configuration LocalOutputStream - IO Error: ");
            message.append(sIOMsg);
            throw backend::BackendAccessException(message.makeStringAndClear(), NULL, ioe) ;
        }
    }

    osl::File::remove(mTemporaryFileUrl) ;
    mWriteFile = new osl::File(mTemporaryFileUrl) ;

    if (osl::File::RC errorCode = mWriteFile->open(OpenFlag_Write |  OpenFlag_Create) )
    {
        delete mWriteFile, mWriteFile = NULL;

        rtl::OUStringBuffer message ;
        message.appendAscii("Cannot open file \"") ;
        message.append(mTemporaryFileUrl).appendAscii("\" for writing. ");
        message.appendAscii("Error is ").append(FileHelper::createOSLErrorString(errorCode));
        message.appendAscii(" [").append(sal_Int32(errorCode)).appendAscii("].") ;

        rtl::OUString const sIOMsg = message.makeStringAndClear();
        uno::Any ioe = uno::makeAny(io::IOException(sIOMsg,0));

        switch (errorCode)
        {
        case osl::File::E_EXIST: // take inability to remove as indicator of missing rights
        case osl::File::E_ACCES:
        case osl::File::E_ROFS:
            message.appendAscii("Configuration LocalOutputStream - No Write Access: ");
            message.append(sIOMsg);
            throw backend::InsufficientAccessRightsException(message.makeStringAndClear(), NULL, ioe) ;

        case osl::File::E_None: OSL_ASSERT(!"can't happen");
        default:
            message.appendAscii("Configuration LocalOutputStream - IO Error: ");
            message.append(sIOMsg);
            throw backend::BackendAccessException(message.makeStringAndClear(), NULL, ioe) ;
        }
    }
    mTemporaryFile = new OSLOutputStreamWrapper(*mWriteFile) ;
}
//------------------------------------------------------------------------------

LocalOutputStream::~LocalOutputStream()
{
    try
    {
        this->closeOutput();
    }
    catch (uno::Exception&)
    {
        OSL_ENSURE(false,"Exception from closing LocalOutputStream ignored.");
    }

    delete mWriteFile;
}
//------------------------------------------------------------------------------

void LocalOutputStream::finishOutput()
    throw (backend::BackendAccessException, uno::RuntimeException)
{
    if (mWriteFile)
    try
    {
        this->closeOutput();
        delete mWriteFile, mWriteFile = NULL;

        FileHelper::replaceFile(mFileUrl, mTemporaryFileUrl) ;
    }
    catch (io::IOException& ioe)
    {
        rtl::OUStringBuffer message ;
        message.appendAscii("Configuration LocalOutputStream - IO Error: ");
        message.appendAscii("Cannot finish output to \"").append(mTemporaryFileUrl) ;
        message.appendAscii("\" or copy the result to \"").append(mFileUrl).appendAscii("\". ");
        message.appendAscii("Error is \"").append(ioe.Message).appendAscii("\". ");
        throw backend::BackendAccessException(message.makeStringAndClear(), *this, uno::makeAny(ioe));
    }
}
//------------------------------------------------------------------------------

inline
uno::Reference<io::XOutputStream> LocalOutputStream::getOutputFile()
{
    if (!mTemporaryFile.is())
    {
        throw io::NotConnectedException(
            rtl::OUString::createFromAscii("LocalOutputStream: no output file."),
            *this);
    }
    return mTemporaryFile;
}
//------------------------------------------------------------------------------

void SAL_CALL LocalOutputStream::writeBytes(const uno::Sequence<sal_Int8>& aData)
    throw (io::NotConnectedException,
            io::BufferSizeExceededException,
            io::IOException, uno::RuntimeException)
{
    getOutputFile()->writeBytes(aData) ;
}
//------------------------------------------------------------------------------

void SAL_CALL LocalOutputStream::flush()
    throw (io::NotConnectedException,
            io::BufferSizeExceededException,
            io::IOException, uno::RuntimeException)
{
    getOutputFile()->flush() ;
}
//------------------------------------------------------------------------------

void SAL_CALL LocalOutputStream::closeOutput()
    throw (io::NotConnectedException, io::BufferSizeExceededException,
            io::IOException, uno::RuntimeException)
{
    if (mTemporaryFile.is())
    {
        mTemporaryFile->closeOutput() ;

        mTemporaryFile.clear();
    }
}
//------------------------------------------------------------------------------

} } // configmgr.localbe
