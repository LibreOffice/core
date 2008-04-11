/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: objectanimator.cxx,v $
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
#include <svx/sdr/animation/objectanimator.hxx>
#include <tools/debug.hxx>
#include <svx/sdr/animation/animationstate.hxx>

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
