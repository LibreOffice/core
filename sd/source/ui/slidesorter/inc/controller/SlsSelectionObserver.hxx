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

#ifndef SD_SLIDESORTER_CONTROLLER_SELECTION_OBSERVER_HXX
#define SD_SLIDESORTER_CONTROLLER_SELECTION_OBSERVER_HXX

#include <tools/gen.hxx>
#include <vector>
#include <boost/shared_ptr.hpp>

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
    virtual ~SelectionObserver (void);

    void NotifyPageEvent (const SdrPage* pPage);
    void StartObservation (void);
    void AbortObservation (void);
    void EndObservation (void);

    /** Use this little class instead of calling StartObservation and
        EndObservation directly so that EndObservation is not forgotten or
        omitted due to an exception or some break or return in the middle of
        code.
    */
    class Context
    {
    public:
        Context (SlideSorter& rSlideSorter);
        ~Context(void);
        void Abort (void);
    private:
        ::boost::shared_ptr<SelectionObserver> mpSelectionObserver;
    };

private:
    SlideSorter& mrSlideSorter;
    SdDrawDocument* mpDocument;
    bool mbIsOvservationActive;

    ::std::vector<const SdPage*> maInsertedPages;
    ::std::vector<sal_Int32> maDeletedPages;
};

} } } // end of namespace ::sd::slidesorter::controller

#endif
