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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include "ftploaderthread.hxx"
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
