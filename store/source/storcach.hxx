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

#ifndef _STORE_STORCACH_HXX
#define _STORE_STORCACH_HXX "$Revision: 1.6.8.2 $"

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

class PageCache : public rtl::IReference
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

} // namespace store

#endif /* !_STORE_STORCACH_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
