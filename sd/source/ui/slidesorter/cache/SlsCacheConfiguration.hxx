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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
