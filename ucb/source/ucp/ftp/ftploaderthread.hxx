/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftploaderthread.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:25:13 $
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

#ifndef _FTP_FTPLOADERTHREAD_HXX_
#define _FTP_FTPLOADERTHREAD_HXX_

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#include "curl.hxx"

namespace ftp {

    /** A loaderthread acts as factory for CURL-handles,
     *  the key being ( implicit ) the threadid.
     *  Owner is a FTPContentProvider-instance
     */

    class FTPLoaderThread
    {
    public:

        FTPLoaderThread();
        ~FTPLoaderThread();

        CURL* handle();


    private:

        /** Don't enable assignment and copy construction.
         *  Not defined:
         */

        FTPLoaderThread(const FTPLoaderThread&);
        FTPLoaderThread& operator=(const FTPLoaderThread&);

        oslThreadKey m_threadKey;

    };  // end class FTPLoaderThread

}


#endif
