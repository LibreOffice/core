/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_activepackages.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-01-18 14:54:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_MANAGER_DP_ACTIVEPACKAGES_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_MANAGER_DP_ACTIVEPACKAGES_HXX

#include "sal/config.h"

#include <utility>
#include <vector>

#include "dp_persmap.h"

namespace rtl { class OUString; }

namespace dp_manager {

class ActivePackages {
public:
    struct Data {
        ::rtl::OUString temporaryName;
        ::rtl::OUString fileName;
        ::rtl::OUString mediaType;
    };

    typedef ::std::vector< ::std::pair< ::rtl::OUString, Data > > Entries;

    ActivePackages();

    ActivePackages(::rtl::OUString const & url, bool readOnly);

    ~ActivePackages();

    bool has(::rtl::OUString const & id, ::rtl::OUString const & fileName)
        const;

    bool get(
        Data * data, ::rtl::OUString const & id,
        ::rtl::OUString const & fileName) const;

    Entries getEntries() const;

    void put(::rtl::OUString const & id, Data const & value);

    void erase(::rtl::OUString const & id, ::rtl::OUString const & fileName);

private:
    ActivePackages(ActivePackages &); // not defined
    void operator =(ActivePackages &); // not defined

    ::dp_misc::PersistentMap m_map;
};

}

#endif
