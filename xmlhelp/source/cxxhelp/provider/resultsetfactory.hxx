/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
