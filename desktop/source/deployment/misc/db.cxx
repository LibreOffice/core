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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include <db.hxx>

#include <rtl/alloc.h>
#include <cstring>
#include <errno.h>

namespace berkeleydbproxy {

//----------------------------------------------------------------------------
    namespace db_internal
    {
        static void raise_error(int dberr, const char * where);

        static inline int check_error(int dberr, const char * where)
        {
            if (dberr) raise_error(dberr,where);
            return dberr;
        }
    }

//----------------------------------------------------------------------------

char *DbEnv::strerror(int error) {
    return (db_strerror(error));
}

//----------------------------------------------------------------------------

Db::Db(DbEnv* pDbenv,u_int32_t flags)
: m_pDBP(0)
{
    db_internal::check_error( db_create(&m_pDBP,pDbenv ? pDbenv->m_pDBENV:0,flags),"Db::Db" );
}


Db::~Db()
{
    if (m_pDBP)
    {
        // should not happen
        // TODO: add assert
    }

}


int Db::close(u_int32_t flags)
{
    int error = m_pDBP->close(m_pDBP,flags);
    m_pDBP = 0;
    return db_internal::check_error(error,"Db::close");
}

int Db::open(DB_TXN *txnid,
             const char *file,
             const char *database,
             DBTYPE type,
             u_int32_t flags,
             int mode)
{
    int err = m_pDBP->open(m_pDBP,txnid,file,database,type,flags,mode);
    return db_internal::check_error( err,"Db::open" );
}


int Db::get(DB_TXN *txnid, Dbt *key, Dbt *data, u_int32_t flags)
{
    int err = m_pDBP->get(m_pDBP,txnid,key,data,flags);

    // these are non-exceptional outcomes
    if (err != DB_NOTFOUND && err != DB_KEYEMPTY)
        db_internal::check_error( err,"Db::get" );

    return err;
}

int Db::put(DB_TXN* txnid, Dbt *key, Dbt *data, u_int32_t flags)
{
    int err = m_pDBP->put(m_pDBP,txnid,key,data,flags);

    if (err != DB_KEYEXIST) // this is a non-exceptional outcome
        db_internal::check_error( err,"Db::put" );
    return err;
}

int Db::cursor(DB_TXN *txnid, Dbc **cursorp, u_int32_t flags)
{
    DBC * dbc = 0;
    int error = m_pDBP->cursor(m_pDBP,txnid,&dbc,flags);

    if (!db_internal::check_error(error,"Db::cursor"))
        *cursorp = new Dbc(dbc);

    return error;
}


#define DB_INCOMPLETE           (-30999)/* Sync didn't finish. */

int Db::sync(u_int32_t flags)
{
    int err;
    DB *db = m_pDBP;

    if (!db) {
        db_internal::check_error(EINVAL,"Db::sync");
        return (EINVAL);
    }
    if ((err = db->sync(db, flags)) != 0 && err != DB_INCOMPLETE) {
        db_internal::check_error(err, "Db::sync");
        return (err);
    }
    return (err);
}

int Db::del(Dbt *key, u_int32_t flags)
{
    DB *db = m_pDBP;
    int err;

    if ((err = db->del(db, 0, key, flags)) != 0) {
        // DB_NOTFOUND is a "normal" return, so should not be
        // thrown as an error
        //
        if (err != DB_NOTFOUND) {
            db_internal::check_error(err, "Db::del");
            return (err);
        }
    }
    return (err);
}

//----------------------------------------------------------------------------

Dbc::Dbc(DBC * dbc)
: m_pDBC(dbc)
{
}

Dbc::~Dbc()
{
}

int Dbc::close()
{
    int err = m_pDBC->c_close(m_pDBC);
    delete this;
    return db_internal::check_error( err,"Dbcursor::close" );
}

int Dbc::get(Dbt *key, Dbt *data, u_int32_t flags)
{
    int err = m_pDBC->c_get(m_pDBC,key,data,flags);

    // these are non-exceptional outcomes
    if (err != DB_NOTFOUND && err != DB_KEYEMPTY)
        db_internal::check_error( err, "Dbcursor::get" );

    return err;
}

//----------------------------------------------------------------------------

Dbt::Dbt()
{
    using namespace std;
    DBT * thispod = this;
    memset(thispod, 0, sizeof *thispod);
}


Dbt::Dbt(void *data_arg, u_int32_t size_arg)
{
    using namespace std;
    DBT * thispod = this;
    memset(thispod, 0, sizeof *thispod);
    this->set_data(data_arg);
    this->set_size(size_arg);
}

Dbt::Dbt(const Dbt & other)
{
    using namespace std;
    const DBT *otherpod = &other;
    DBT *thispod = this;
    memcpy(thispod, otherpod, sizeof *thispod);
}

Dbt& Dbt::operator = (const Dbt & other)
{
    if (this != &other)
    {
        using namespace std;
        const DBT *otherpod = &other;
        DBT *thispod = this;
        memcpy(thispod, otherpod, sizeof *thispod);
    }
    return *this;
}

Dbt::~Dbt()
{
}

void * Dbt::get_data() const
{
    return this->data;
}

void Dbt::set_data(void *value)
{
    this->data = value;
}

u_int32_t Dbt::get_size() const
{
    return this->size;
}

void Dbt::set_size(u_int32_t value)
{
    this->size = value;
}

//----------------------------------------------------------------------------
void db_internal::raise_error(int dberr, const char * where)
{
    if (!where) where = "<unknown>";

    const char * dberrmsg = db_strerror(dberr);
    if (!dberrmsg || !*dberrmsg) dberrmsg = "<unknown DB error>";

    rtl::OString msg = where;
    msg += ": ";
    msg += dberrmsg;

    throw DbException(msg);
}

//----------------------------------------------------------------------------
} // namespace ecomp

