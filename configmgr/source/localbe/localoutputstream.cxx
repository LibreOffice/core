/*************************************************************************
 *
 *  $RCSfile: localoutputstream.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:29:55 $
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

#ifndef CONFIGMGR_LOCALBE_LOCALOUTPUTSTREAM_HXX_
#include "localoutputstream.hxx"
#endif // CONFIGMGR_LOCALBE_LOCALOUTPUTSTREAM_HXX_

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_INSUFFICIENTACCESSRIGHTSEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>
#endif

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
    //  if (mWriteFile)  FileHelper::removeFile(mTemporaryFileUrl) ;
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
