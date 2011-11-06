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

#include <rtl/string.hxx>
#include "dp_misc_api.hxx"

extern "C" {
  typedef void *(*db_malloc_fcn_type)(size_t);
  typedef void *(*db_realloc_fcn_type)(void *, size_t);
  typedef void (*db_free_fcn_type)(void *);
}


namespace berkeleydbproxy {

    class DbEnv;
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

    class DESKTOP_DEPLOYMENTMISC_DLLPUBLIC DbException
    {
        rtl::OString what_;
    public:
        explicit DbException(rtl::OString const & theWhat)
        : what_(theWhat)
        {}

        const char *what() const
        { return what_.getStr(); }
        int get_errno() const
        { return 0; }
    };


    class DESKTOP_DEPLOYMENTMISC_DLLPUBLIC DbEnv : db_internal::Noncopyable
    {
        friend class Db;

    private:
        DB_ENV* m_pDBENV;

    public:
        static char *strerror(int);
    };

    class DESKTOP_DEPLOYMENTMISC_DLLPUBLIC Db : db_internal::Noncopyable
    {
    private:
        DB* m_pDBP;

    public:
        Db(DbEnv* dbbenv,u_int32_t flags);
        ~Db();

        int close(u_int32_t flags);

        int open(DB_TXN *txnid,
                 const char *file,
                 const char *database,
                 DBTYPE type,
                 u_int32_t flags,
                 int mode);

        int sync(u_int32_t flags);
        int del(Dbt *key, u_int32_t flags);

        int get(DB_TXN* txnid, Dbt *key, Dbt *data, u_int32_t flags);
        int put(DB_TXN* txnid, Dbt *key, Dbt *data, u_int32_t flags);

        int cursor(DB_TXN *txnid, Dbc **cursorp, u_int32_t flags);
    };

    class DESKTOP_DEPLOYMENTMISC_DLLPUBLIC Dbc : db_internal::Noncopyable
    {
        friend class Db;
        friend class Dbt;

    private:
        DBC* m_pDBC;

        SAL_DLLPRIVATE explicit Dbc(DBC* pDBC);
        SAL_DLLPRIVATE ~Dbc();

    public:
        int close();

        int get(Dbt *key, Dbt *data, u_int32_t flags);
    };

    class DESKTOP_DEPLOYMENTMISC_DLLPUBLIC Dbt: private DBT
    {
        friend class Db;
        friend class Dbc;

    public:
        Dbt(void *data_arg, u_int32_t size_arg);

        Dbt();
        Dbt(const Dbt & other);
        Dbt & operator=(const Dbt & other);

        ~Dbt();

          void *get_data() const;
        void set_data(void *value);

          u_int32_t get_size() const;
        void set_size(u_int32_t value);
    };
}

#endif
















