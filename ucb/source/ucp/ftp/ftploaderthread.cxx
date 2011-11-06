/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


