/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: db.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-07 15:52:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef BERKELEYDBPROXY_DB_HXX_
#define BERKELEYDBPROXY_DB_HXX_

#ifdef SYSTEM_DB
#include <db.h>
#else
#include <berkeleydb/db.h>
#endif

#include <rtl/string.hxx>

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_MISC_API_HXX
#include "dp_misc_api.hxx"
#endif

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
        explicit DbEnv(u_int32_t flags);
        ~DbEnv();

        int open(const char *db_home, u_int32_t flags, int mode);
        void close(u_int32_t flags);

        int set_alloc(
                  db_malloc_fcn_type app_malloc,
                  db_realloc_fcn_type app_realloc,
                  db_free_fcn_type app_free);

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

        int set_alloc(
                  db_malloc_fcn_type app_malloc,
                  db_realloc_fcn_type app_realloc,
                  db_free_fcn_type app_free);
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

        int del(u_int32_t flags_arg);
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

        void set_flags(u_int32_t);
    };
}

#endif
















