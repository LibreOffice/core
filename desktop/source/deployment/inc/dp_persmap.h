/*************************************************************************
 *
 *  $RCSfile: dp_persmap.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:06:02 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if ! defined INCLUDED_DP_PERSMAP_H
#define INCLUDED_DP_PERSMAP_H

#include "rtl/ustring.hxx"
#include "berkeleydb/db_cxx.h"
#include <hash_map>


namespace dp_misc
{

typedef ::std::hash_map<
    ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash > t_string2string_map;

//==============================================================================
class PersistentMap
{
    ::rtl::OUString m_sysPath;
    mutable Db m_db;
    void throw_rtexc( char const * msg ) const;

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
