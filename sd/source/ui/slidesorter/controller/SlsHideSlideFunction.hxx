/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsHideSlideFunction.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:14:31 $
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

    HideSlideFunction (
        SlideSorterController& rController,
        SfxRequest& rRequest);
    virtual ~HideSlideFunction (void);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

