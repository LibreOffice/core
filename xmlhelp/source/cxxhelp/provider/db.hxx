/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef BERKELEYDBPROXY_DB_HXX_
#define BERKELEYDBPROXY_DB_HXX_

#ifdef SYSTEM_DB
#include <db.h>
#else
#include <berkeleydb/db.h>
#endif

#include "com/sun/star/ucb/XSimpleFileAccess.hpp"

#include <hash_map>
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


//#define TEST_DBHELP

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

    typedef std::hash_map< rtl::OString,std::pair<int,int>,ha,eq >  StringToValPosMap;
    typedef std::hash_map< rtl::OString,rtl::OString,ha,eq >        StringToDataMap;

    class DBHelp
    {
        rtl::OUString       m_aFileURL;
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
        //DBHelp must get a fileURL which can then directly be used by simple file access.
        //SimpleFileAccess requires file URLs as arguments. Passing file path may work but fails
        //for example when using long file paths on Windows, which start with "\\?\"
        DBHelp( const rtl::OUString& rFileURL,
            com::sun::star::uno::Reference< com::sun::star::ucb::XSimpleFileAccess > xSFA )
                : m_aFileURL( rFileURL )
                , m_pStringToDataMap( NULL )
                , m_pStringToValPosMap( NULL )
                , m_xSFA( xSFA )
                , m_pItData( NULL )
                , m_nItRead( -1 )
                , m_iItPos( -1 )
        {
            OSL_ASSERT(!rFileURL.compareTo(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:")), 5));
        }
        ~DBHelp()
            { releaseHashMap(); }

        void createHashMap( bool bOptimizeForPerformance = false );
        void releaseHashMap( void );

#ifdef TEST_DBHELP
        bool testAgainstDb( const rtl::OUString& fileURL, bool bOldDbAccess );
#endif

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

        int open(DB_TXN *txnid,
                 ::rtl::OUString const & fileURL,
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
















