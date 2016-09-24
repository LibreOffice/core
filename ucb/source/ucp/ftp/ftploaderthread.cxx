/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

extern "C" {

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

}

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
    CURL* ret = static_cast<CURL*>(osl_getThreadKeyData(m_threadKey));
    if(!ret) {
        ret = curl_easy_init();
        if (ret != nullptr) {
            // Make sure curl is not internally using environment variables like
            // "ftp_proxy":
            if (curl_easy_setopt(ret, CURLOPT_PROXY, "") != CURLE_OK) {
                curl_easy_cleanup(ret);
                ret = nullptr;
            }
        }
        osl_setThreadKeyData(m_threadKey, ret);
    }

    return ret;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
