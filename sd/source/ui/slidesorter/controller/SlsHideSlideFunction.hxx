/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsHideSlideFunction.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:25:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

