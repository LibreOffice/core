/*************************************************************************
 *
 *  $RCSfile: db.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-03-30 08:38:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef BERKELEYDBPROXY_DB_HXX_
#define BERKELEYDBPROXY_DB_HXX_

#include <berkeleydb/db.h>

#include <rtl/string.hxx>

extern "C" {
  typedef void *(*db_malloc_fcn_type)(size_t);
  typedef void *(*db_realloc_fcn_type)(void *, size_t);
  typedef void (*db_free_fcn_type)(void *);
};


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

    class DbException
    {
        rtl::OString what_;
    public:
        explicit DbException(rtl::OString const & what)
        : what_(what)
        {}

        const char *what() const
        { return what_.getStr(); }
    };


    class DbEnv : db_internal::Noncopyable
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
    };

    class Db : db_internal::Noncopyable
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
        Dbt(void *data, size_t size);

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
















