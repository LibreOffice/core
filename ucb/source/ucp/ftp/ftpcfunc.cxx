/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftpcfunc.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:23:00 $
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
#include <string.h>

#include "ftpcontentidentifier.hxx"
#include "ftpinpstr.hxx"
#include <stdio.h>

using namespace ftp;
using namespace com::sun::star::uno;

extern "C" {

    int ftp_write(void *buffer,size_t size,size_t nmemb,void *stream)
    {
        FTPStreamContainer *_stream =
            reinterpret_cast<FTPStreamContainer*>(stream);

        if(!_stream)
            return 0;

        return _stream->write(buffer,size,nmemb);
    }


    int file_write(void *buffer,size_t size,size_t nmemb,void *stream)
    {
        FILE* file =
            reinterpret_cast<FILE*>(stream);
        if(!file)
            return 0;
        return fwrite(buffer,size,nmemb,file);
    }


    int ftp_passwd(void *, char*, char*, int)
    {
        return 0;
    }


}
