#include <curl/types.h>

namespace ftp {

    class FTPHandleProvider {
    public:

        virtual CURL* handle() = 0;

    };


}
