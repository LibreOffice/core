/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IdleDetection.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-01-19 12:51:46 $
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

#ifndef SD_IDLE_DETECTION_HXX
#define SD_IDLE_DETECTION_HXX

#include <sal/types.h>

class Window;

namespace sd { namespace tools {

/** Detect whether the system is idle and some time consuming operation may
    be carried out.  This class ditinguishes between different states of
    idle-ness.
*/
class IdleDetection
{
public:
    /** When GetIdleState() returns this value, then the system is idle.
    */
    static const sal_Int32 IDET_IDLE = 0x0000;

    /** There are system event pending.
    */
    static const sal_Int32 IDET_SYSTEM_EVENT_PENDING = 0x0001;

    /** A full screen slide show is running and is active.  In contrast
        there may be a full screen show be running in an inactive window,
        i.e. in the background.
    */
    static const sal_Int32 IDET_FULL_SCREEN_SHOW_ACTIVE = 0x0002;

    /** A slide show is running in a window.
    */
    static const sal_Int32 IDET_WINDOW_SHOW_ACTIVE = 0x0004;

    /** A window is being painted.
    */
    static const sal_Int32 IDET_WINDOW_PAINTING = 0x0008;

    /** Determine whether the system is idle.
        @param pWindow
            When a valid Window pointer is given then it is checked
            whether the window is currently being painting.
        @return
            This method either returns IDET_IDLE or a combination of
            IdleStates values or-ed together that describe what the system
            is currently doing so that the caller can decide what to do.
    */
    static sal_Int32 GetIdleState (const ::Window* pWindow = NULL);

private:
    /** Check whether there are input events pending.
    */
    static sal_Int32 CheckInputPending (void);

    /** Check whether a slide show is running full screen or in a window.
    */
    static sal_Int32 CheckSlideShowRunning (void);

    static sal_Int32 CheckWindowPainting (const ::Window& rWindow);
};

} } // end of namespace ::sd::tools

#endif
