/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: db.hxx,v $
 * $Revision: 1.8 $
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


    class Db : db_internal::Noncopyable
    {
    private:
        DB* m_pDBP;

    public:
        Db();
        ~Db();

        int close(u_int32_t flags);

        int open(DB_TXN *txnid,
                 const char *file,
                 const char *database,
                 DBTYPE type,
                 u_int32_t flags,
                 int mode);


        int get(DB_TXN* txnid, Dbt *key, Dbt *data, u_int32_t flags);
        int put(DB_TXN* txnid, Dbt *key, Dbt *data, u_int32_t flags);

        int cursor(DB_TXN *txnid, Dbc **cursorp, u_int32_t flags);

        int set_alloc(
                  db_malloc_fcn_type app_malloc,
                  db_realloc_fcn_type app_realloc,
                  db_free_fcn_type app_free);
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
















