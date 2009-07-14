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

#ifndef INCLUDED_CONFIGMGR_COMPONENTS_HXX
#define INCLUDED_CONFIGMGR_COMPONENTS_HXX

#include "sal/config.h"

#include <list>

#include "boost/noncopyable.hpp"
#include "rtl/ref.hxx"
#include "stl/hash_map"

#include "nodemap.hxx"

namespace rtl {
    class OUString;
    struct OUStringHash;
}

namespace configmgr {

class Node;

class Components: private boost::noncopyable {
public:
    static Components & singleton();

    static bool allLocales(rtl::OUString const & locale);

    static rtl::OUString createSegment(
        rtl::OUString const & templateName, rtl::OUString const & name);

    static sal_Int32 parseSegment(
        rtl::OUString const & path, sal_Int32 index, rtl::OUString * name,
        bool * setElement, rtl::OUString * templateName);

    static NodeMap::iterator resolveNode(
        rtl::OUString const & name, NodeMap * map);

    rtl::Reference< Node > resolvePath(
        rtl::OUString const & path, rtl::OUString * firstSegment,
        rtl::OUString * lastSegment, rtl::OUString * canonicalPath,
        rtl::Reference< Node > * parent);

    rtl::Reference< Node > getTemplate(rtl::OUString const & fullName) const;

    void addModification(rtl::OUString const & path);

    void writeModifications();

    typedef
        std::hash_map< rtl::OUString, rtl::Reference< Node>, rtl::OUStringHash >
        TemplateMap;

private:
    Components();

    ~Components();

    rtl::OUString getModificationFileUrl() const;

    void parseModificationLayer();

    TemplateMap templates_;

    NodeMap components_;

    typedef std::list< rtl::OUString > Modifications;

    Modifications modifications_;
};

}

#endif
