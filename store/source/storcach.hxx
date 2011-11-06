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

