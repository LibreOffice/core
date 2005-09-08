/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_persmap.h,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:18:56 $
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

#if ! defined INCLUDED_DP_PERSMAP_H
#define INCLUDED_DP_PERSMAP_H

#include "rtl/ustring.hxx"
#include "db.hxx"
#include <hash_map>

using namespace berkeleydbproxy;

namespace dp_misc
{

typedef ::std::hash_map<
    ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash > t_string2string_map;

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

    void flush() const;

    bool has( ::rtl::OUString const & key ) const;
    bool get( ::rtl::OUString * value, ::rtl::OUString const & key ) const;
    t_string2string_map getEntries() const;
    bool put( ::rtl::OUString const & key, ::rtl::OUString & value,
              bool overwrite = true, bool flush_immediately = true );
    bool erase( ::rtl::OUString const & key, bool flush_immediately = true );
};

}

#endif
