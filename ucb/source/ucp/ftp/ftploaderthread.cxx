#ifndef _FTP_FTPLOADERTHREAD_HXX_
#include "ftploaderthread.hxx"
#endif
#ifndef __CURL_CURL_H
#include <curl/curl.h>
#endif


using namespace ftp;



/********************************************************************************/
/*                                                                              */
/*                                  the joiner                                  */
/*                                                                              */
/********************************************************************************/


static void ftploaderthread_terminater(void *pData)
{
    FtpLoaderThread* pLoaderThread = static_cast<FtpLoaderThread*>(pData);
    oslThreadIdentifier threaId(pLoaderThread->threadId());
//      oslThread aThread = osl_getThreadFromIdentifier(threaId);
//      osl_joinWithThread(aThread);
//      FtpLoaderThread::remove(threaId);
}




/********************************************************************************/
/*                                                                              */
/*                  Member part of FtpLoaderThread                              */
/*                                                                              */
/********************************************************************************/


FtpLoaderThread::FtpLoaderThread()
    : m_nThreadId(osl_getThreadIdentifier(NULL)),
      m_pHandle(NULL)
{
}


FtpLoaderThread::FtpLoaderThread(oslThreadIdentifier nThreadId)
    : m_nThreadId(nThreadId),
      m_pHandle(NULL)
{
}


FtpLoaderThread::FtpLoaderThread(const FtpLoaderThread& r)
{
    m_nThreadId = r.m_nThreadId;
    m_pHandle = r.m_pHandle;
    r.m_pHandle = NULL;
}


FtpLoaderThread& FtpLoaderThread::operator=(const FtpLoaderThread& r)
{
    m_nThreadId = r.m_nThreadId;
    m_pHandle = r.m_pHandle;
    r.m_pHandle = NULL;
    return *this;
}


FtpLoaderThread::~FtpLoaderThread()
{
    if(m_pHandle)
        curl_easy_cleanup(m_pHandle);
}


bool FtpLoaderThread::operator==(const FtpLoaderThread& r) const
{
    return m_nThreadId == r.threadId();
}


oslThreadIdentifier FtpLoaderThread::threadId() const
{
    return m_nThreadId;
}


void FtpLoaderThread::init() const
{
    m_pHandle = curl_easy_init();
    osl_createThread(ftploaderthread_terminater,(void*)this);
}


/********************************************************************************/
/*                                                                              */
/*                          FtpLoaderThreadHashSet                              */
/*                                                                              */
/********************************************************************************/



size_t FtpLoaderThreadHash::operator()(const FtpLoaderThread& p) const
{
    return size_t(p.threadId());
}



/********************************************************************************/
/*                                                                              */
/*                  Static part of FtpLoaderThread                              */
/*                                                                              */
/********************************************************************************/

osl::Mutex FtpLoaderThread::ftploader_mutex;


FtpLoaderThread::FtpLoaderThreadSet FtpLoaderThread::ftploaderthread_set;



CURL* FtpLoaderThread::curlHandle()
{
    osl::MutexGuard aGuard(ftploader_mutex);

    if(ftploaderthread_set.empty())
        curl_global_init(CURL_GLOBAL_DEFAULT);

    FtpLoaderThread loader;
     std::pair<FtpLoaderThreadSet::iterator,bool> p(ftploaderthread_set.insert(loader));
    if(p.second)
        p.first->init();
    return p.first->m_pHandle;
}


void FtpLoaderThread::remove(oslThreadIdentifier nThreadId)
{
    osl::MutexGuard aGuard(ftploader_mutex);

    ftploaderthread_set.erase(FtpLoaderThread(nThreadId));
    if(ftploaderthread_set.empty())
        curl_global_cleanup();
}


