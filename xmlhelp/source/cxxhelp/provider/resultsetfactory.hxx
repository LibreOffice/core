#ifndef _RESULTSETFACTORY_HXX
#define _RESULTSETFACTORY_HXX


#include "resultsetbase.hxx"


namespace chelp {

    class ResultSetBase;

    class ResultSetFactory
    {
    public:

        virtual ~ResultSetFactory() { };

        virtual ResultSetBase* createResultSet() = 0;
    };



}



#endif
