/*************************************************************************
 *
 *  $RCSfile: localoutputstream.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jb $ $Date: 2002-07-11 17:17:41 $
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

namespace configmgr { namespace localbe {

//==============================================================================

//------------------------------------------------------------------------------

LocalOutputStream::LocalOutputStream(const rtl::OUString& aFileUrl)
    throw (io::IOException)
: mFileUrl(aFileUrl), mTemporaryFileUrl(mFileUrl), mWriteFile(NULL) {
    // First, ensure the directory where the file is supposed to be
    // put exists.
    mTemporaryFileUrl += rtl::OUString::createFromAscii("_tmp") ;
    rtl::OUString parentDirectory = FileHelper::getParentDir(aFileUrl) ;

    if (!FileHelper::mkdirs(parentDirectory)) {
        rtl::OUStringBuffer message ;

        message.appendAscii("Impossible to create directory '") ;
        message.append(parentDirectory).appendAscii("'") ;
        throw io::IOException(message.makeStringAndClear(), NULL) ;
    }
    FileHelper::removeFile(mTemporaryFileUrl) ;
    mWriteFile = new osl::File(mTemporaryFileUrl) ;
    osl::FileBase::RC errorCode = mWriteFile->open(OpenFlag_Write |
                                                   OpenFlag_Create) ;

    if (errorCode != osl_File_E_None)
    {
        delete mWriteFile, mWriteFile = NULL;
        throw io::IOException(FileHelper::createOSLErrorString(errorCode),NULL) ;
    }
    mTemporaryFile = new OSLOutputStreamWrapper(*mWriteFile) ;
}
//------------------------------------------------------------------------------

LocalOutputStream::~LocalOutputStream(void)
{
    this->closeOutput();
    delete mWriteFile;

    //  if (mWriteFile)  FileHelper::removeFile(mTemporaryFileUrl) ;
}
//------------------------------------------------------------------------------

void LocalOutputStream::finishOutput()
{
    if (mWriteFile)
    {
        this->closeOutput();
        delete mWriteFile, mWriteFile = NULL;

        FileHelper::replaceFile(mFileUrl, mTemporaryFileUrl) ;
    }
}
//------------------------------------------------------------------------------

void SAL_CALL LocalOutputStream::closeOutput(void)
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
