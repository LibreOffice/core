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

#ifndef INCLUDED_STORE_SOURCE_STORCACH_HXX
#define INCLUDED_STORE_SOURCE_STORCACH_HXX

#include "sal/types.h"
#include "rtl/ref.hxx"

#include "store/types.h"
#include "storbase.hxx"

namespace store
{

/*========================================================================
 *
 * PageCache interface.
 *
 *======================================================================*/

class PageCache : public virtual salhelper::SimpleReferenceObject
{
public:
    /** load.
     */
    storeError lookupPageAt (
        PageHolder & rxPage,
        sal_uInt32   nOffset);

    /** insert.
     */
    storeError insertPageAt (
        PageHolder const & rxPage,
        sal_uInt32         nOffset);

    /** update, or insert.
     */
    storeError updatePageAt (
        PageHolder const & rxPage,
        sal_uInt32         nOffset);

    /** remove (invalidate).
     */
    storeError removePageAt (
        sal_uInt32 nOffset);

protected:
    virtual ~PageCache() {}

private:
    /** Implementation (abstract).
     */
    virtual storeError lookupPageAt_Impl (
        PageHolder & rxPage,
        sal_uInt32   nOffset) = 0;

    virtual storeError insertPageAt_Impl (
        PageHolder const & rxPage,
        sal_uInt32         nOffset) = 0;

    virtual storeError updatePageAt_Impl (
        PageHolder const & rxPage,
        sal_uInt32         nOffset) = 0;

    virtual storeError removePageAt_Impl (
        sal_uInt32 nOffset) = 0;
};

/*========================================================================
 *
 * PageCache factory.
 *
 *======================================================================*/

storeError
PageCache_createInstance (
    rtl::Reference< store::PageCache > & rxCache,
    sal_uInt16                           nPageSize
);

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

}

#endif // INCLUDED_STORE_SOURCE_STORCACH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
