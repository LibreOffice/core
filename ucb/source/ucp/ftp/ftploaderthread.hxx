#ifndef _FTP_FTPLOADERTHREAD_HXX_
#define _FTP_FTPLOADERTHREAD_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef __CURL_TYPES_H
#include <curl/types.h>
#endif
#ifndef _STL_HASH_SET_INCLUDED_
#include <hash_set>
#define _STL_HASH_SET_INCLUDED_
#endif


namespace ftp {


    /*forward*/ class FtpLoaderThread;

    class FtpLoaderThreadHash
    {
    public:

        inline size_t operator()(const FtpLoaderThread& p) const;

    };  // end class FtpLoaderThreadHash



    /**
     *  Purpose of FtpThread
     *
     */

    class FtpLoaderThread
    {
        typedef std::hash_set<FtpLoaderThread,FtpLoaderThreadHash> FtpLoaderThreadSet;

    public:

        static CURL* curlHandle();
        static void remove(oslThreadIdentifier thread);

        FtpLoaderThread();
        FtpLoaderThread(oslThreadIdentifier thread);
        FtpLoaderThread(const FtpLoaderThread&);
        FtpLoaderThread& operator=(const FtpLoaderThread&);
        ~FtpLoaderThread();

        /*inline*/ void init() const;
        /*inline*/ bool operator==(const FtpLoaderThread& r) const;
        /*inline*/ oslThreadIdentifier threadId() const;

    private:

        /**
         *  Don't enable assignment and copyconstruction.
         */

        oslThreadIdentifier m_nThreadId;
        mutable CURL* m_pHandle;

        static osl::Mutex ftploader_mutex;
        static FtpLoaderThreadSet ftploaderthread_set;
    };

}


#endif
