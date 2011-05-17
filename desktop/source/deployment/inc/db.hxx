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

#include <boost/noncopyable.hpp>

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


    class DESKTOP_DEPLOYMENTMISC_DLLPUBLIC DbEnv : boost::noncopyable
    {
        friend class Db;

    private:
        DB_ENV* m_pDBENV;

    public:
        static char *strerror(int);
    };

    class DESKTOP_DEPLOYMENTMISC_DLLPUBLIC Db : boost::noncopyable
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

    class DESKTOP_DEPLOYMENTMISC_DLLPUBLIC Dbc : boost::noncopyable
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
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
