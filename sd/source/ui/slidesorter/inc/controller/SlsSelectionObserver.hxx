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
