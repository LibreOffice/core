#include <string.h>

#include "ftpcontentidentifier.hxx"
#include "ftpinpstr.hxx"

using namespace ftp;
using namespace com::sun::star::uno;

extern "C" {

    int ftp_write(void *buffer,size_t size,size_t nmemb,void *stream)
    {
        FTPStreamContainer *_stream =
            reinterpret_cast<FTPStreamContainer*>(stream);

        if(!_stream)
            return 0;

        return _stream->write(buffer,size,nmemb);
    }


//      int write2InputStream(void *buffer,size_t size,size_t nmemb,void *stream)
//      {
//          size_t ret = size*nmemb;
//          if(!(stream && ret))   // OK, no error if nothing can be written.
//              return ret;

//          FTPBufferContainer *p =
//              reinterpret_cast<FTPBufferContainer*>(stream);
//          if(p && p->m_out)
//              p->m_out->append(buffer,size,nmemb);
//          return ret;
//      }


//      /** Callback for curl_easy_perform(),
//       *  forwarding the written content to the outputstream.
//       */

//      int write2OutputStream(void *buffer,size_t size,size_t nmemb,void *stream)
//      {
//          size_t ret = size*nmemb;

//          if(!(stream && ret))  // OK, no error if nothing can be written.
//              return ret;

//          try{
//              FTPStreamContainer *p =
//                  reinterpret_cast<FTPStreamContainer*>(stream);
//              if(p && p->m_out.is())
//                  p->m_out->writeBytes(
//                      Sequence<sal_Int8>(static_cast<sal_Int8*>(buffer),
//                                         size*nmemb)
//                  );
//              return ret;
//          } catch(const Exception&) {
//              return 0;
//          }
//      }


    int ftp_passwd(void *client,char*prompt,char*buffer,int bufferlength)
    {
//          FTPClient *p =
//              reinterpret_cast<FTPClient>(client);

//          // 'passwd' returns actually "username:password"
//          rtl::OUString passwd = p->passwd();
//          rtl::OString opasswd(passwd.getStr(),
//                               passwd.getLength(),
//                               RTL_TEXTENCODING_UTF8);
//          if(strlen(opasswd.getStr()) <= bufferlength)
//              strncpy(buffer,opasswd.getStr(),opasswd.getLength());
//          else
//      strcpy(buffer,opasswd.getStr());
        strcpy(buffer,"psswd");

        return 0;
    }


}
