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
    TYPEINFO();

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
