/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftploaderthread.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:24:55 $
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
#include "ftploaderthread.hxx"
#endif
#include "curl.hxx"

using namespace ftp;


/********************************************************************************/
/*                                                                              */
/*                cleanup function for thread specific data                     */
/*                                                                              */
/********************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

    int memory_write_dummy(void *,size_t,size_t,void *)
    {
        return 0;
    }

    void delete_CURL(void *pData)
    {
        // Otherwise response for QUIT will be sent to already destroyed
        // MemoryContainer via non-dummy memory_write function.
        curl_easy_setopt(static_cast<CURL*>(pData),
                         CURLOPT_HEADERFUNCTION,
                         memory_write_dummy);
        curl_easy_cleanup(static_cast<CURL*>(pData));
    }

#ifdef __cplusplus
}
#endif


/********************************************************************************/
/*                                                                              */
/*                  Member part of FTPLoaderThread                              */
/*                                                                              */
/********************************************************************************/


FTPLoaderThread::FTPLoaderThread()
    : m_threadKey(osl_createThreadKey(delete_CURL)) {
}



FTPLoaderThread::~FTPLoaderThread() {
    osl_destroyThreadKey(m_threadKey);
}



CURL* FTPLoaderThread::handle() {
    CURL* ret = osl_getThreadKeyData(m_threadKey);
    if(!ret) {
        ret = curl_easy_init();
        if (ret != 0) {
            // Make sure curl is not internally using environment variables like
            // "ftp_proxy":
            if (curl_easy_setopt(ret, CURLOPT_PROXY, "") != CURLE_OK) {
                curl_easy_cleanup(ret);
                ret = 0;
            }
        }
        osl_setThreadKeyData(m_threadKey,static_cast<void*>(ret));
    }

    return ret;
}


