/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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
************************************************************************/

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include "rtl/ustring.hxx"

#include "modifications.hxx"

namespace configmgr {

namespace {

bool isPrefix(rtl::OUString const & prefix, rtl::OUString const & path) {
    return prefix.getLength() < path.getLength() && path.match(prefix) &&
        path[prefix.getLength()] == '/';
}

}

void Modifications::add(rtl::OUString const & path) {
    //TODO: performance
    for (List::iterator i(list.begin()); i != list.end();) {
        if (path == *i || isPrefix(*i, path)) {
            return;
        }
        if (isPrefix(path, *i)) {
            list.erase(i++);
        } else {
            ++i;
        }
    }
    list.push_back(path);
}

}
