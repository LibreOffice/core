/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <com/sun/star/io/XInputStream.hpp>
#include <osl/file.hxx>
#include <comphelper/oslfile2streamwrap.hxx>

using osl::File;
using osl::FileBase;
using namespace ::com::sun::star;

namespace layoutimpl
{

uno::Reference< io::XInputStream > getFileAsStream( const rtl::OUString &rName )
{
    rtl::OUString sFileURL;
    if( FileBase::E_None != FileBase::getFileURLFromSystemPath( rName, sFileURL ) )
        sFileURL = rName; // maybe it already was a file url

    File * blobFile = new File(sFileURL);
    File::RC errorCode = blobFile->open(OpenFlag_Read);

    uno::Reference<io::XInputStream> xResult;
    switch (errorCode)
    {
    case osl::File::E_None: // got it
        xResult.set( new comphelper::OSLInputStreamWrapper(blobFile,true) );
        break;

    case osl::File::E_NOENT: // no file => no stream
        delete blobFile;
        break;

    default:
        delete blobFile;
/*            {
            rtl::OUStringBuffer sMsg;
            sMsg.appendAscii("Cannot open output file \"");
            sMsg.append(aURL);
            sMsg.appendAscii("\" : ");
            sMsg.append(configmgr::FileHelper::createOSLErrorString(errorCode));

            throw io::IOException(sMsg.makeStringAndClear(),NULL);
        }
*/
    }

    return xResult;
}

} // namespace layoutimpl
