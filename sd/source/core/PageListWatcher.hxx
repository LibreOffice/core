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



#ifndef PAGE_LIST_WATCHER_HXX
#define PAGE_LIST_WATCHER_HXX

#include "pres.hxx"
#include <sal/types.h>
#include <vector>
#include <svl/lstner.hxx>

class SdPage;
class SdrModel;

/** Maintain a map of page indices to page objects for faster access that
    remains valid during deletions and insertions of pages (#109538#).
*/
class ImpPageListWatcher : public SfxListener
{
protected:
    // typedefs for a vector of SdPages
    typedef ::std::vector< SdPage* > SdPageVector;

    const SdrModel&                 mrModel;

    SdPageVector                    maPageVectorStandard;
    SdPageVector                    maPageVectorNotes;
    SdPage*                         mpHandoutPage;

    sal_Bool                        mbPageListValid;

    void ImpRecreateSortedPageListOnDemand();
    virtual sal_uInt32 ImpGetPageCount() const = 0;

    /** Return the page with the given index.
        @param nIndex
            When given an invalid index then NULL is returned.
    */
    virtual SdPage* ImpGetPage (sal_uInt32 nIndex) const = 0;

public:
    ImpPageListWatcher(const SdrModel& rModel);
    virtual ~ImpPageListWatcher();

    void Invalidate() { mbPageListValid = sal_False; }
    SdPage* GetSdPage(PageKind ePgKind, sal_uInt32 nPgNum = 0L);
    sal_uInt32 GetSdPageCount(PageKind ePgKind);
};

//////////////////////////////////////////////////////////////////////////////

class ImpDrawPageListWatcher : public ImpPageListWatcher
{
protected:
    virtual sal_uInt32 ImpGetPageCount() const;
    virtual SdPage* ImpGetPage(sal_uInt32 nIndex) const;

public:
    ImpDrawPageListWatcher(const SdrModel& rModel);
    virtual ~ImpDrawPageListWatcher();

    // derived from SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

//////////////////////////////////////////////////////////////////////////////

class ImpMasterPageListWatcher : public ImpPageListWatcher
{
protected:
    virtual sal_uInt32 ImpGetPageCount() const;
    virtual SdPage* ImpGetPage(sal_uInt32 nIndex) const;

public:
    ImpMasterPageListWatcher(const SdrModel& rModel);
    virtual ~ImpMasterPageListWatcher();

    // derived from SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

#endif
