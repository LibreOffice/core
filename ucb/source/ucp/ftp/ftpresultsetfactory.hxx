#ifndef _FTP_FTPRESULTSETFACTORY_HXX_
#define _FTP_FTPRESULTSETFACTORY_HXX_

#ifndef _FTP_FTPRESULTSETBASE_HXX_
#include "ftpresultsetbase.hxx"
#endif



namespace ftp {

    class ResultSetBase;

    class ResultSetFactory
    {
    public:

        virtual ~ResultSetFactory() { };

        virtual ResultSetBase* createResultSet() = 0;
    };

}



#endif
