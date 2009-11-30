/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ftpcfunc.hxx,v $
 * $Revision: 1.6.22.1 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#ifndef _FTP_FTPCFUNC_HXX_
#define _FTP_FTPCFUNC_HXX_

#include <rtl/ustring.hxx>


namespace ftp {

    class FTPStreamContainer
    {
    public:

        virtual size_t write(void *buffer,size_t size,size_t nmemb) = 0;
    };

}


extern "C" {

    /** callback for curl_easy_perform(),
     *  forwarding the written content to the stream.
     *  stream has to be of type 'FTPStreamContainer'.
     */


    int file_write(void *buffer,size_t size,size_t nmemb,void *stream);

}

#endif
