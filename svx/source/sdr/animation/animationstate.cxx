/*************************************************************************
 *
 *  $RCSfile: animationstate.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-11-24 16:42:53 $
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

#ifndef _SDR_ANIMATION_ANIMATIONSTATE_HXX
#include <svx/sdr/animation/animationstate.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SDR_ANIMATION_OBJECTANIMATOR_HXX
#include <svx/sdr/animation/objectanimator.hxx>
#endif

#ifndef _SDR_ANIMATION_ANIMATIONINFO_HXX
#include <svx/sdr/animation/animationinfo.hxx>
#endif

#ifndef _SDR_CONTACT_OBJECTCONTACT_HXX
#include <svx/sdr/contact/objectcontact.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#include <svx/sdr/contact/viewcontact.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace animation
    {
        // get associated AnimationInfo
        AnimationInfo& AnimationState::GetAnimationInfo() const
        {
            return *(mrVOContact.GetViewContact().GetAnimationInfo());
        }

        // get associated ObectAnimator
        ObjectAnimator& AnimationState::GetObjectAnimator() const
        {
            return mrVOContact.GetObjectContact().GetObjectAnimator();
        }

        AnimationState::AnimationState(
            sdr::contact::ViewObjectContact& rVOContact)
        :   Event(0L),
            mrVOContact(rVOContact)
        {
            const sal_uInt32 nStartTime(GetAnimationInfo().GetStartTime());

            if(0L != nStartTime)
            {
                SetTime(nStartTime);
            }
        }

        AnimationState::~AnimationState()
        {
            // ensure that Event member is removed from ObjectAnimator
            GetObjectAnimator().RemoveEvent(this);
        }

        // execute event, from base class Event
        void AnimationState::Trigger(sal_uInt32 nTime)
        {
            // schedule a repaint of associated object
            mrVOContact.ActionChanged();

            // need to produce event after nTime?
            sal_uInt32 nNewTime(nTime);

            if(GetAnimationInfo().DoRegisterAgain(nTime, nNewTime, *this))
            {
                SetTime(nNewTime);
                GetObjectAnimator().InsertEvent(this);
            }
        }
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
