/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationstate.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:55:49 $
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
#define _SDR_ANIMATION_ANIMATIONSTATE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <vector>

#ifndef _SDR_ANIMATION_SCHEDULER_HXX
#include <svx/sdr/animation/scheduler.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace sdr
{
    namespace contact
    {
        class ViewObjectContact;
    } // end of namespace contact
    namespace animation
    {
        class AnimationInfo;
        class ObjectAnimator;
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace animation
    {
        class AnimationState : public Event
        {
        protected:
            // the animated VOC
            sdr::contact::ViewObjectContact&        mrVOContact;

            // get associated AnimationInfo
            AnimationInfo& GetAnimationInfo() const;

            // get associated ObectAnimator
            ObjectAnimator& GetObjectAnimator() const;

        public:
            // basic constructor.
            AnimationState(
                sdr::contact::ViewObjectContact& rVOContact);

            // destructor
            virtual ~AnimationState();

            // execute event, from base class Event
            virtual void Trigger(sal_uInt32 nTime);
        };

        // typedef for a list of AnimationState
        typedef ::std::vector< AnimationState* > AnimationStateVector;

    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_ANIMATION_ANIMATIONSTATE_HXX

// eof
