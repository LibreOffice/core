#ifndef _FTP_FTPLOADERTHREAD_HXX_
#define _FTP_FTPLOADERTHREAD_HXX_

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef __CURL_TYPES_H
#include <curl/types.h>
#endif


namespace ftp {

    /** A loaderthread acts as factory for CURL-handles.
     *  Owner is a FtpContentProvider-instance
     */

    class FtpLoaderThread
    {
    public:

        FtpLoaderThread();
        ~FtpLoaderThread();

        CURL* handle();


    private:

        /** Don't enable assignment and copyconstruction.
         *  Not defined:
         */

        FtpLoaderThread(const FtpLoaderThread&);
        FtpLoaderThread& operator=(const FtpLoaderThread&);

        oslThreadKey m_threadKey;

    };  // end class FtpLoaderThread

}


#endif
