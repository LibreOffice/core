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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSSELECTIONOBSERVER_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSSELECTIONOBSERVER_HXX

#include <sal/types.h>
#include <memory>
#include <vector>

namespace sd { namespace slidesorter {
class SlideSorter;
} }

class SdDrawDocument;
class SdrPage;

namespace sd { namespace slidesorter { namespace controller {

/** Observe insertions and deletions of pages between calls to
    StartObservation() and EndObservation().  When the later is called
    the selection is set to just the newly inserted pages.
*/
class SelectionObserver
{
public:
    SelectionObserver (SlideSorter& rSlideSorter);
    virtual ~SelectionObserver();

    void NotifyPageEvent (const SdrPage* pPage);
    void StartObservation();
    void AbortObservation();
    void EndObservation();

    /** Use this little class instead of calling StartObservation and
        EndObservation directly so that EndObservation is not forgotten or
        omitted due to an exception or some break or return in the middle of
        code.
    */
    class Context
    {
    public:
        Context (SlideSorter& rSlideSorter);
        ~Context();
        void Abort();
    private:
        std::shared_ptr<SelectionObserver> mpSelectionObserver;
    };

private:
    SlideSorter& mrSlideSorter;
    bool mbIsOvservationActive;

    ::std::vector<const SdPage*> maInsertedPages;
    ::std::vector<sal_Int32> maDeletedPages;
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
