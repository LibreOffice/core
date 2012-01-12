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
