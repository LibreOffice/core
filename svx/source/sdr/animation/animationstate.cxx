/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: animationstate.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/animation/animationstate.hxx>
#include <tools/debug.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/animation/objectanimator.hxx>
#include <svx/sdr/animation/animationinfo.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>

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
