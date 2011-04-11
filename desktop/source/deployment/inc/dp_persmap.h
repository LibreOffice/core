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

#if ! defined INCLUDED_DP_PERSMAP_H
#define INCLUDED_DP_PERSMAP_H

#include "rtl/ustring.hxx"
#include "db.hxx"
#include <boost/unordered_map.hpp>

using namespace berkeleydbproxy;

namespace dp_misc
{

typedef ::boost::unordered_map<
    ::rtl::OString, ::rtl::OString, ::rtl::OStringHash > t_string2string_map;

//==============================================================================
class PersistentMap
{
    ::rtl::OUString m_sysPath;
    mutable Db m_db;
    void throw_rtexc( int err, char const * msg = 0 ) const;

public:
    ~PersistentMap();
    PersistentMap( ::rtl::OUString const & url, bool readOnly );
    /** in mem db */
    PersistentMap();

    bool has( ::rtl::OString const & key ) const;
    bool get( ::rtl::OString * value, ::rtl::OString const & key ) const;
    t_string2string_map getEntries() const;
    void put( ::rtl::OString const & key, ::rtl::OString const & value );
    bool erase( ::rtl::OString const & key, bool flush_immediately = true );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
