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

#ifndef INCLUDED_SD_SOURCE_CORE_PAGELISTWATCHER_HXX
#define INCLUDED_SD_SOURCE_CORE_PAGELISTWATCHER_HXX

#include "pres.hxx"
#include <sal/types.h>
#include <vector>

class SdPage;
class SdrModel;

/** Maintain a map of page indices to page objects for faster access that
    remains valid during deletions and insertions of pages (#109538#).
*/
class ImpPageListWatcher
{
protected:
    // typedefs for a vector of SdPages
    typedef ::std::vector< SdPage* > SdPageVector;

    const SdrModel&                 mrModel;

    SdPageVector                    maPageVectorStandard;
    SdPageVector                    maPageVectorNotes;
    SdPage*                         mpHandoutPage;

    bool                        mbPageListValid;

    void ImpRecreateSortedPageListOnDemand();
    virtual sal_uInt32 ImpGetPageCount() const = 0;

    /** Return the page with the given index.
        @param nIndex
            When given an invalid index then NULL is returned.
    */
    virtual SdPage* ImpGetPage (sal_uInt32 nIndex) const = 0;

public:
    explicit ImpPageListWatcher(const SdrModel& rModel);
    virtual ~ImpPageListWatcher();

    void Invalidate() { mbPageListValid = false; }
    SdPage* GetSdPage(PageKind ePgKind, sal_uInt32 nPgNum = 0L);
    sal_uInt32 GetSdPageCount(PageKind ePgKind);
    sal_uInt32 GetVisibleSdPageCount();
};

class ImpDrawPageListWatcher : public ImpPageListWatcher
{
protected:
    virtual sal_uInt32 ImpGetPageCount() const override;
    virtual SdPage* ImpGetPage(sal_uInt32 nIndex) const override;

public:
    explicit ImpDrawPageListWatcher(const SdrModel& rModel);
    virtual ~ImpDrawPageListWatcher();
};

class ImpMasterPageListWatcher : public ImpPageListWatcher
{
protected:
    virtual sal_uInt32 ImpGetPageCount() const override;
    virtual SdPage* ImpGetPage(sal_uInt32 nIndex) const override;

public:
    explicit ImpMasterPageListWatcher(const SdrModel& rModel);
    virtual ~ImpMasterPageListWatcher();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
