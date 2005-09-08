/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationstate.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:01:14 $
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
            }
            else
            {
                // #i38135# Advance 10 minutes
                nNewTime = nTime + (10L * 60000L);
                SetTime(nNewTime);
            }

            // insert event again
            GetObjectAnimator().InsertEvent(this);
        }
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
