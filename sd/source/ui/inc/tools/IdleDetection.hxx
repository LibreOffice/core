/*************************************************************************
 *
 *  $RCSfile: IdleDetection.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2005-02-17 09:43:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SD_IDLE_DETECTION_HXX
#define SD_IDLE_DETECTION_HXX

#include <sal/types.h>

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

    /** Determine whether the system is idle.
        @return
            This method either returns IDET_IDLE or a combination of
            IdleStates values or-ed together that describe what the system
            is currently doing so that the caller can decide what to do.
    */
    static sal_Int32 GetIdleState (void);

private:
    /** Check whether there are input events pending.
    */
    static sal_Int32 CheckInputPending (void);

    /** Check whether a slide show is running full screen or in a window.
    */
    static sal_Int32 CheckSlideShowRunning (void);
};

} } // end of namespace ::sd::tools

#endif
