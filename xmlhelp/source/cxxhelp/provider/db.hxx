/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef BERKELEYDBPROXY_DB_HXX_
#define BERKELEYDBPROXY_DB_HXX_

#ifdef SYSTEM_DB
#include <db.h>
#else
#include <berkeleydb/db.h>
#endif

#include "com/sun/star/ucb/XSimpleFileAccess.hpp"

#include <boost/unordered_map.hpp>
#include <rtl/string.hxx>

extern "C" {
  typedef void *(*db_malloc_fcn_type)(size_t);
  typedef void *(*db_realloc_fcn_type)(void *, size_t);
  typedef void (*db_free_fcn_type)(void *);
}


namespace berkeleydbproxy {

    class Dbc;
    class Dbt;

    namespace db_internal
    {
        class Noncopyable
        {
            // not implemented
            Noncopyable(const Noncopyable&);
            void operator=(const Noncopyable&);
        protected:
            Noncopyable() {}
            ~Noncopyable() {}
        };
    }

    class DbException
    {
        rtl::OString what_;
    public:
        explicit DbException(rtl::OString const & whatparam)
        : what_(whatparam)
        {}

        const char *what() const
        { return what_.getStr(); }
    };

    struct eq
    {
        bool operator()( const rtl::OString& rKey1, const rtl::OString& rKey2 ) const
            { return rKey1.compareTo( rKey2 ) == 0; }
    };

    struct ha
    {
        size_t operator()( const rtl::OString& rName ) const
            { return rName.hashCode(); }
    };


    class DBData
    {
        friend class        DBHelp;

        int                 m_nSize;
        char*               m_pBuffer;

        void copyToBuffer( const char* pSrcData, int nSize );

    public:
        DBData( void )
            : m_nSize( 0 )
            , m_pBuffer( NULL )
        {}
        ~DBData()
            { delete [] m_pBuffer; }

          int getSize() const
            { return m_nSize; }
          const char* getData() const
            { return m_pBuffer; }
    };

    typedef boost::unordered_map< rtl::OString,std::pair<int,int>,ha,eq >   StringToValPosMap;
    typedef boost::unordered_map< rtl::OString,rtl::OString,ha,eq >     StringToDataMap;

    class DBHelp
    {
        rtl::OUString       m_aFileName;
        StringToDataMap*    m_pStringToDataMap;
        StringToValPosMap*  m_pStringToValPosMap;
        com::sun::star::uno::Reference< com::sun::star::ucb::XSimpleFileAccess >
                            m_xSFA;

        com::sun::star::uno::Sequence< sal_Int8 >
                            m_aItData;
        const char*         m_pItData;
        int                 m_nItRead;
        int                 m_iItPos;

        bool implReadLenAndData( const char* pData, int& riPos, DBData& rValue );

    public:
        DBHelp( const rtl::OUString& rFileName,
            com::sun::star::uno::Reference< com::sun::star::ucb::XSimpleFileAccess > xSFA )
                : m_aFileName( rFileName )
                , m_pStringToDataMap( NULL )
                , m_pStringToValPosMap( NULL )
                , m_xSFA( xSFA )
                , m_pItData( NULL )
                , m_nItRead( -1 )
                , m_iItPos( -1 )
        {}
        ~DBHelp()
            { releaseHashMap(); }

        void createHashMap( bool bOptimizeForPerformance = false );
        void releaseHashMap( void );

        bool getValueForKey( const rtl::OString& rKey, DBData& rValue );

        bool startIteration( void );
        bool getNextKeyAndValue( DBData& rKey, DBData& rValue );
        void stopIteration( void );
    };

    class Db : db_internal::Noncopyable
    {
    private:
        DB* m_pDBP;
        DBHelp* m_pDBHelp;

    public:
        Db();
        ~Db();

        void setDBHelp( DBHelp* pDBHelp )
            { m_pDBHelp = pDBHelp; }
        DBHelp* getDBHelp( void )
            { return m_pDBHelp; }

        int close(u_int32_t flags);

        int open(DB_TXN *txnid,
                 const char *file,
                 const char *database,
                 DBTYPE type,
                 u_int32_t flags,
                 int mode);


        int get(DB_TXN* txnid, Dbt *key, Dbt *data, u_int32_t flags);

        int cursor(DB_TXN *txnid, Dbc **cursorp, u_int32_t flags);
    };

    class Dbc : db_internal::Noncopyable
    {
        friend class Db;
        friend class Dbt;

    private:
        DBC* m_pDBC;

        explicit Dbc(DBC* pDBC);
        ~Dbc();

    public:
        int close();

        int get(Dbt *key, Dbt *data, u_int32_t flags);
    };

    class Dbt: private DBT
    {
        friend class Db;
        friend class Dbc;

    public:
        Dbt(void *data_arg, u_int32_t size_arg);

        Dbt();
        //Dbt(const Dbt & other);
        //Dbt & operator=(const Dbt & other);

        ~Dbt();

          void *get_data() const;
        void set_data(void *value);

          u_int32_t get_size() const;
        void set_size(u_int32_t value);

        void set_flags(u_int32_t);
    };
}

#endif
















/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
