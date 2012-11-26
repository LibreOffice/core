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



#ifndef SD_SLIDESORTER_HIDE_SLIDE_FUNCTION_HXX
#define SD_SLIDESORTER_HIDE_SLIDE_FUNCTION_HXX

#include "controller/SlsSlideFunction.hxx"
#include "model/SlsPageEnumeration.hxx"


namespace sd { namespace slidesorter { namespace controller {

/** Toggle the hidden flag of the selected slides.
    When the selected pages have not all the same state they will all set to
    hidden.  When all selected pages have the same state this state is
    toggled for all of them
*/
class HideSlideFunction
    : public SlideFunction
{
public:
    virtual ~HideSlideFunction (void);

    static FunctionReference Create( SlideSorter& rSlideSorter, SfxRequest& rRequest );
    virtual void DoExecute( SfxRequest& rReq );

    // The state of a set of slides with respect to being excluded from the
    // slide show.
    enum ExclusionState {UNDEFINED, EXCLUDED, INCLUDED, MIXED};

    /** Return for the given set of slides whether they included are
        excluded from the slide show.
    */
    static ExclusionState GetExclusionState (model::PageEnumeration& rPageSet);

protected:
    HideSlideFunction (
        SlideSorter& rSlideSorter,
        SfxRequest& rRequest);

    SlideSorter& mrSlideSorter;
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

