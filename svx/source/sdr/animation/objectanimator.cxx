/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objectanimator.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:33:24 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SDR_ANIMATION_OBJECTANIMATOR_HXX
#include <svx/sdr/animation/objectanimator.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SDR_ANIMATION_ANIMATIONSTATE_HXX
#include <svx/sdr/animation/animationstate.hxx>
#endif

// for SOLARIS compiler include of algorithm part of _STL is necesary to
// get access to basic algos like ::std::find
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace animation
    {
        ObjectAnimator::ObjectAnimator()
        {
        }

        ObjectAnimator::~ObjectAnimator()
        {
        }

        // get the list count
        sal_uInt32 ObjectAnimator::Count() const
        {
            return maAnimationStates.size();
        }

        // Remove AnimationState member
        void ObjectAnimator::RemoveAnimationState(AnimationState& rAnimationState)
        {
            const AnimationStateVector::iterator aFindResult = ::std::find(
                maAnimationStates.begin(), maAnimationStates.end(), &rAnimationState);

            if(aFindResult != maAnimationStates.end())
            {
                // #114376# remember content for next call
                AnimationState* pErasedState = *aFindResult;
                maAnimationStates.erase(aFindResult);
                RemoveEvent(pErasedState);
            }
        }

        // Add AnimationState member
        void ObjectAnimator::AddAnimationState(AnimationState& rAnimationState)
        {
            maAnimationStates.push_back(&rAnimationState);
            InsertEvent(&rAnimationState);
        }
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
