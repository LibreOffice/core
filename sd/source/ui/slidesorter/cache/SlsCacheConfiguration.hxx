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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_CACHE_SLSCACHECONFIGURATION_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_CACHE_SLSCACHECONFIGURATION_HXX

#include <com/sun/star/uno/Any.hxx>
#include <vcl/timer.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <memory>

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
    static std::shared_ptr<CacheConfiguration> Instance();

    /** Look up the specified value in
        MultiPaneGUI/SlideSorter/PreviewCache.   When the specified value
        does not exist then an empty Any is returned.
    */
    css::uno::Any GetValue (const OUString& rName);

private:
    /** When a caller holds a reference after we have released ours we use
        this weak pointer to avoid creating a new instance.
    */
    static std::weak_ptr<CacheConfiguration> mpWeakInstance;
    static Timer maReleaseTimer;
    css::uno::Reference<css::container::XNameAccess> mxCacheNode;

    CacheConfiguration();

    DECL_STATIC_LINK_TYPED(CacheConfiguration, TimerCallback, Timer *, void);
};

} } } // end of namespace ::sd::slidesorter::cache

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
