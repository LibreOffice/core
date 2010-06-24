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
 ***********************************************************************/

#ifndef INCLUDED_VCL_IMPIMAGETREE_HXX
#define INCLUDED_VCL_IMPIMAGETREE_HXX

#include "sal/config.h"

#include <list>
#include <utility>
#include <vector>

#include <hash_map>

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "rtl/ustring.hxx"
#include "salhelper/singletonref.hxx"

namespace com { namespace sun { namespace star { namespace container {
    class XNameAccess;
} } } }
class BitmapEx;

class ImplImageTree: private boost::noncopyable {
public:
    ImplImageTree();

    ~ImplImageTree();

    // check whether the icon style is installed
    bool checkStyle(rtl::OUString const & style);

    bool loadImage(
        rtl::OUString const & name, rtl::OUString const & style,
        BitmapEx & bitmap, bool localized = false );

    void shutDown();
        // a crude form of life cycle control (called from DeInitVCL; otherwise,
        // if the ImplImageTree singleton were destroyed during exit that would
        // be too late for the destructors of the bitmaps in m_iconCache)

private:
    typedef std::list<
        std::pair<
            rtl::OUString,
            com::sun::star::uno::Reference<
                com::sun::star::container::XNameAccess > > > Zips;

    typedef std::hash_map<
        rtl::OUString, bool, rtl::OUStringHash > CheckStyleCache;
    typedef std::hash_map<
        rtl::OUString, std::pair< bool, BitmapEx >, rtl::OUStringHash > IconCache;

    rtl::OUString m_style;
    Zips m_zips;
    CheckStyleCache m_checkStyleCache;
    IconCache m_iconCache;

    void setStyle(rtl::OUString const & style );

    void resetZips();

    bool checkStyleCacheLookup( rtl::OUString const & style, bool &exists );
    bool iconCacheLookup( rtl::OUString const & name, bool localized, BitmapEx & bitmap );

    bool find(std::vector< rtl::OUString > const & paths, BitmapEx & bitmap );
};

typedef salhelper::SingletonRef< ImplImageTree > ImplImageTreeSingletonRef;

#endif
