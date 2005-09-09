/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftpcfunc.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:31:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


    int ftp_write(void *buffer,size_t size,size_t nmemb,void *stream);

    int file_write(void *buffer,size_t size,size_t nmemb,void *stream);

}




namespace ftp {

    /** Is the first argument to 'passwd' below.
     */

    class FTPClient
    {
    public:

        virtual rtl::OUString passwd() const = 0;
    };

}


extern "C" {

    int ftp_passwd(void *client,char*prompt,char*buffer,int bufferlength);

}

#endif
