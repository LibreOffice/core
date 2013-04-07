/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_VCL_IMPIMAGETREE_HXX
#define INCLUDED_VCL_IMPIMAGETREE_HXX

#include "sal/config.h"

#include <list>
#include <utility>
#include <vector>

#include <boost/unordered_map.hpp>

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
    bool checkStyle(OUString const & style);

    bool loadImage(
        OUString const & name, OUString const & style,
        BitmapEx & bitmap, bool localized = false, bool loadMissing = false );

    bool loadDefaultImage(
        OUString const & style,
        BitmapEx& bitmap);

    void shutDown();
        // a crude form of life cycle control (called from DeInitVCL; otherwise,
        // if the ImplImageTree singleton were destroyed during exit that would
        // be too late for the destructors of the bitmaps in m_iconCache)

private:
    bool doLoadImage(
        OUString const & name, OUString const & style,
        BitmapEx & bitmap, bool localized);

    typedef std::list<
        std::pair<
            OUString,
            com::sun::star::uno::Reference<
                com::sun::star::container::XNameAccess > > > Paths;

    typedef boost::unordered_map<
        OUString, bool, OUStringHash > CheckStyleCache;
    typedef boost::unordered_map<
        OUString, std::pair< bool, BitmapEx >, OUStringHash > IconCache;

    OUString m_style;
    Paths m_paths;
    CheckStyleCache m_checkStyleCache;
    IconCache m_iconCache;
    bool m_cacheIcons;

    void setStyle(OUString const & style );

    void resetPaths();

    bool checkStyleCacheLookup( OUString const & style, bool &exists );
    bool iconCacheLookup( OUString const & name, bool localized, BitmapEx & bitmap );

    bool find(std::vector< OUString > const & paths, BitmapEx & bitmap );
};

typedef salhelper::SingletonRef< ImplImageTree > ImplImageTreeSingletonRef;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
