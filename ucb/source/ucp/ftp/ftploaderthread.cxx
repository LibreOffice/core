/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftploaderthread.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:36:52 $
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
#ifndef __CURL_CURL_H
#include <curl/curl.h>
#endif


using namespace ftp;


/********************************************************************************/
/*                                                                              */
/*                cleanup function for thread specific data                     */
/*                                                                              */
/********************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

    int memory_write_dummy(void *buffer,size_t size,size_t nmemb,void *stream)
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
    CURL* ret;
    if(!(ret = osl_getThreadKeyData(m_threadKey))) {
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


