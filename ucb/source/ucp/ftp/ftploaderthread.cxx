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

    void delete_CURL(void *pData)
    {
        curl_easy_cleanup(static_cast<CURL*>(pData));
    }

#ifdef __cplusplus
}
#endif


/********************************************************************************/
/*                                                                              */
/*                  Member part of FtpLoaderThread                              */
/*                                                                              */
/********************************************************************************/


FtpLoaderThread::FtpLoaderThread()
    : m_threadKey(osl_createThreadKey(delete_CURL)) {
}



FtpLoaderThread::~FtpLoaderThread() {
    osl_destroyThreadKey(m_threadKey);
}



CURL* FtpLoaderThread::handle() {
    CURL* ret;
    if(!(ret = osl_getThreadKeyData(m_threadKey))) {
        ret = curl_easy_init();
        osl_setThreadKeyData(m_threadKey,static_cast<void*>(ret));
    }

    return ret;
}


