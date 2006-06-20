/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftpstrcont.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:26:16 $
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
#ifndef _FTP_STRCONT_HXX_
#define _FTP_STRCONT_HXX_

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include "ftpcfunc.hxx"


namespace ftp {


    class FTPInputStream;


    class FTPOutputStreamContainer
        : public FTPStreamContainer
    {
    public:

        FTPOutputStreamContainer(const com::sun::star::uno::Reference<
                                 com::sun::star::io::XOutputStream>& out);
                                 virtual ~FTPOutputStreamContainer() {}

        virtual size_t write(void *buffer,size_t size,size_t nmemb);


    private:

        com::sun::star::uno::Reference<
        com::sun::star::io::XOutputStream> m_out;
    };


    class FTPInputStreamContainer
        : public FTPStreamContainer
    {
    public:

        FTPInputStreamContainer(FTPInputStream* out);

        virtual size_t write(void *buffer,size_t size,size_t nmemb);

        com::sun::star::uno::Reference<
        com::sun::star::io::XInputStream> operator()();

    private:

        FTPInputStream* m_out;
    };


}


#endif
