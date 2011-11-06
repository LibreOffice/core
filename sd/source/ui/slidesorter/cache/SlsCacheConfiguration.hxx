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



#ifndef SD_SLIDESORTER_CACHE_CONFIGURATION_HXX
#define SD_SLIDESORTER_CACHE_CONFIGURATION_HXX

#include <com/sun/star/uno/Any.hxx>
#include <vcl/timer.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace sd { namespace slidesorter { namespace cache {

/** A very simple and easy-to-use access to configuration entries regarding
    the slide sorter cache.
*/
class CacheConfiguration
{
public:
    /** Return an instance to this class.  The reference is released after 5
        seconds.  Subsequent calls to this function will create a new
        instance.
    */
    static ::boost::shared_ptr<CacheConfiguration> Instance (void);

    /** Look up the specified value in
        MultiPaneGUI/SlideSorter/PreviewCache.   When the specified value
        does not exist then an empty Any is returned.
    */
    ::com::sun::star::uno::Any GetValue (const ::rtl::OUString& rName);

private:
    static ::boost::shared_ptr<CacheConfiguration> mpInstance;
    /** When a caller holds a reference after we have released ours we use
        this weak pointer to avoid creating a new instance.
    */
    static ::boost::weak_ptr<CacheConfiguration> mpWeakInstance;
    static Timer maReleaseTimer;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameAccess> mxCacheNode;

    CacheConfiguration (void);

    DECL_LINK(TimerCallback, Timer*);
};

} } } // end of namespace ::sd::slidesorter::cache

#endif
