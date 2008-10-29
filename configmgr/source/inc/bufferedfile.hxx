/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bufferedfile.hxx,v $
 * $Revision: 1.5 $
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

#ifndef CONFIGMGR_BUFFEREDFILE_HXX
#define CONFIGMGR_BUFFEREDFILE_HXX

#include "utility.hxx"
#include <boost/utility.hpp>
#include <osl/file.hxx>
#include <com/sun/star/io/IOException.hpp>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

namespace configmgr
{

    namespace io = com::sun::star::io;

    class BufferedOutputFile: private boost::noncopyable, public osl::FileBase
    {
        osl::File * m_pFile;
        std::vector<sal_uInt8>      m_buffer;
    public:
        BufferedOutputFile( rtl::OUString const& aFileURL, sal_uInt32 nBufferSizeHint = 0 );
        ~BufferedOutputFile ();

        RC open( sal_uInt32 uFlags );
        RC close();

        //RC getPos( sal_uInt64& uPos )
        RC write(const void *pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64& rBytesWritten);

        // as opposed to osl::File, this method is not const here
        RC sync();
    };
} // namespace

#endif
