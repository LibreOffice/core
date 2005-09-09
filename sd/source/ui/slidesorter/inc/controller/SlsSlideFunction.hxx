/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsSlideFunction.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:20:04 $
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

#ifndef SD_SLIDESORTER_SLIDE_FUNCTION_HXX
#define SD_SLIDESORTER_SLIDE_FUNCTION_HXX

#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif

class SdDrawDocument;

namespace sd { namespace slidesorter { namespace controller {

class SlideSorterController;


/** Base class for functions of the slide sorter.
*/
class SlideFunction
    : public FuPoor
{
public:
    TYPEINFO();

    SlideFunction (
        SlideSorterController& rController,
        SfxRequest& rRequest);

    virtual ~SlideFunction (void);

    virtual BOOL KeyInput (const KeyEvent& rKEvt);
    virtual BOOL MouseMove (const MouseEvent& rMEvt) { return FALSE; }
    virtual BOOL MouseButtonUp (const MouseEvent& rMEvt) { return FALSE; }
    virtual BOOL MouseButtonDown (const MouseEvent& rMEvt) { return FALSE; }

    virtual void Activate (void);
    virtual void Deactivate (void);

    /** Called from ForceScroll() before the actual scrolling.
    */
    virtual void ScrollStart (void);

    /** Called from ForceScroll() after the actual scrolling.
    */
    virtual void ScrollEnd (void);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif
