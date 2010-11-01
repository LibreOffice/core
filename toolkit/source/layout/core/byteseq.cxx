/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
