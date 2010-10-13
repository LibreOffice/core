/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/animation/animationtiming.hxx>
#include <basegfx/numeric/ftools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace animation
    {
        //////////////////////////////////////////////////////////////////////////////

        AnimationEntry::AnimationEntry()
        {
        }

        AnimationEntry::~AnimationEntry()
        {
        }

        //////////////////////////////////////////////////////////////////////////////

        AnimationEntryFixed::AnimationEntryFixed(double fDuration, double fState)
        :   mfDuration(fDuration),
            mfState(fState)
        {
        }

        AnimationEntryFixed::~AnimationEntryFixed()
        {
        }

        AnimationEntry* AnimationEntryFixed::clone() const
        {
            return new AnimationEntryFixed(mfDuration, mfState);
        }

        bool AnimationEntryFixed::operator==(const AnimationEntry& rCandidate) const
        {
            const AnimationEntryFixed* pCompare = dynamic_cast< const AnimationEntryFixed* >(&rCandidate);

            return (pCompare
                && basegfx::fTools::equal(mfDuration, pCompare->mfDuration)
                && basegfx::fTools::equal(mfState, pCompare->mfState));
        }

        double AnimationEntryFixed::getDuration() const
        {
            return mfDuration;
        }

        double AnimationEntryFixed::getStateAtTime(double /*fTime*/) const
        {
            return mfState;
        }

        double AnimationEntryFixed::getNextEventTime(double fTime) const
        {
            if(basegfx::fTools::less(fTime, mfDuration))
            {
                return mfDuration;
            }
            else
            {
                return 0.0;
            }
        }

        //////////////////////////////////////////////////////////////////////////////

        AnimationEntryLinear::AnimationEntryLinear(double fDuration, double fFrequency, double fStart, double fStop)
        :   mfDuration(fDuration),
            mfFrequency(fFrequency),
            mfStart(fStart),
            mfStop(fStop)
        {
        }

        AnimationEntryLinear::~AnimationEntryLinear()
        {
        }

        AnimationEntry* AnimationEntryLinear::clone() const
        {
            return new AnimationEntryLinear(mfDuration, mfFrequency, mfStart, mfStop);
        }

        bool AnimationEntryLinear::operator==(const AnimationEntry& rCandidate) const
        {
            const AnimationEntryLinear* pCompare = dynamic_cast< const AnimationEntryLinear* >(&rCandidate);

            return (pCompare
                && basegfx::fTools::equal(mfDuration, pCompare->mfDuration)
                && basegfx::fTools::equal(mfStart, pCompare->mfStart)
                && basegfx::fTools::equal(mfStop, pCompare->mfStop));
        }

        double AnimationEntryLinear::getDuration() const
        {
            return mfDuration;
        }

        double AnimationEntryLinear::getStateAtTime(double fTime) const
        {
            if(basegfx::fTools::more(mfDuration, 0.0))
            {
                const double fFactor(fTime / mfDuration);

                if(fFactor > 1.0)
                {
                    return mfStop;
                }
                else
                {
                    return mfStart + ((mfStop - mfStart) * fFactor);
                }
            }
            else
            {
                return mfStart;
            }
        }

        double AnimationEntryLinear::getNextEventTime(double fTime) const
        {
            if(basegfx::fTools::less(fTime, mfDuration))
            {
                // use the simple solution: just add the frequency. More correct (but also more
                // complicated) would be to calculate the slice of time we are in and when this
                // slice will end. For the animations, this makes no quality difference.
                fTime += mfFrequency;

                if(basegfx::fTools::more(fTime, mfDuration))
                {
                    fTime = mfDuration;
                }

                return fTime;
            }
            else
            {
                return 0.0;
            }
        }

        //////////////////////////////////////////////////////////////////////////////

        sal_uInt32 AnimationEntryList::impGetIndexAtTime(double fTime, double &rfAddedTime) const
        {
            sal_uInt32 nIndex(0L);

            while(nIndex < maEntries.size() && basegfx::fTools::lessOrEqual(rfAddedTime + maEntries[nIndex]->getDuration(), fTime))
            {
                rfAddedTime += maEntries[nIndex++]->getDuration();
            }

            return nIndex;
        }

        AnimationEntryList::AnimationEntryList()
        :   mfDuration(0.0)
        {
        }

        AnimationEntryList::~AnimationEntryList()
        {
            for(sal_uInt32 a(0L); a < maEntries.size(); a++)
            {
                delete maEntries[a];
            }
        }

        AnimationEntry* AnimationEntryList::clone() const
        {
            AnimationEntryList* pNew = new AnimationEntryList();

            for(sal_uInt32 a(0L); a < maEntries.size(); a++)
            {
                pNew->append(*maEntries[a]);
            }

            return pNew;
        }

        bool AnimationEntryList::operator==(const AnimationEntry& rCandidate) const
        {
            const AnimationEntryList* pCompare = dynamic_cast< const AnimationEntryList* >(&rCandidate);

            if(pCompare && mfDuration == pCompare->mfDuration)
            {
                for(sal_uInt32 a(0L); a < maEntries.size(); a++)
                {
                    if(!(*maEntries[a] == *pCompare->maEntries[a]))
                    {
                        return false;
                    }
                }

                return true;
            }

            return false;
        }

        void AnimationEntryList::append(const AnimationEntry& rCandidate)
        {
            const double fDuration(rCandidate.getDuration());

            if(!basegfx::fTools::equalZero(fDuration))
            {
                maEntries.push_back(rCandidate.clone());
                mfDuration += fDuration;
            }
        }

        double AnimationEntryList::getDuration() const
        {
            return mfDuration;
        }

        double AnimationEntryList::getStateAtTime(double fTime) const
        {
            if(!basegfx::fTools::equalZero(mfDuration))
            {
                double fAddedTime(0.0);
                const sal_uInt32 nIndex(impGetIndexAtTime(fTime, fAddedTime));

                if(nIndex < maEntries.size())
                {
                    return maEntries[nIndex]->getStateAtTime(fTime - fAddedTime);
                }
            }

            return 0.0;
        }

        double AnimationEntryList::getNextEventTime(double fTime) const
        {
            double fNewTime(0.0);

            if(!basegfx::fTools::equalZero(mfDuration))
            {
                double fAddedTime(0.0);
                const sal_uInt32 nIndex(impGetIndexAtTime(fTime, fAddedTime));

                if(nIndex < maEntries.size())
                {
                    fNewTime = maEntries[nIndex]->getNextEventTime(fTime - fAddedTime) + fAddedTime;
                }
            }

            return fNewTime;
        }

        //////////////////////////////////////////////////////////////////////////////

        AnimationEntryLoop::AnimationEntryLoop(sal_uInt32 nRepeat)
        :   AnimationEntryList(),
            mnRepeat(nRepeat)
        {
        }

        AnimationEntryLoop::~AnimationEntryLoop()
        {
        }

        AnimationEntry* AnimationEntryLoop::clone() const
        {
            AnimationEntryLoop* pNew = new AnimationEntryLoop(mnRepeat);

            for(sal_uInt32 a(0L); a < maEntries.size(); a++)
            {
                pNew->append(*maEntries[a]);
            }

            return pNew;
        }

        bool AnimationEntryLoop::operator==(const AnimationEntry& rCandidate) const
        {
            const AnimationEntryLoop* pCompare = dynamic_cast< const AnimationEntryLoop* >(&rCandidate);

            return (pCompare
                && mnRepeat == pCompare->mnRepeat
                && AnimationEntryList::operator==(rCandidate));
        }

        double AnimationEntryLoop::getDuration() const
        {
            return (mfDuration * (double)mnRepeat);
        }

        double AnimationEntryLoop::getStateAtTime(double fTime) const
        {
            if(mnRepeat && !basegfx::fTools::equalZero(mfDuration))
            {
                const sal_uInt32 nCurrentLoop((sal_uInt32)(fTime / mfDuration));

                if(nCurrentLoop > mnRepeat)
                {
                    return 1.0;
                }
                else
                {
                    const double fTimeAtLoopStart((double)nCurrentLoop * mfDuration);
                    const double fRelativeTime(fTime - fTimeAtLoopStart);
                    return AnimationEntryList::getStateAtTime(fRelativeTime);
                }
            }

            return 0.0;
        }

        double AnimationEntryLoop::getNextEventTime(double fTime) const
        {
            double fNewTime(0.0);

            if(mnRepeat && !basegfx::fTools::equalZero(mfDuration))
            {
                const sal_uInt32 nCurrentLoop((sal_uInt32)(fTime / mfDuration));

                if(nCurrentLoop <= mnRepeat)
                {
                    const double fTimeAtLoopStart((double)nCurrentLoop * mfDuration);
                    const double fRelativeTime(fTime - fTimeAtLoopStart);
                    const double fNextEventAtLoop(AnimationEntryList::getNextEventTime(fRelativeTime));

                    if(!basegfx::fTools::equalZero(fNextEventAtLoop))
                    {
                        fNewTime = fNextEventAtLoop + fTimeAtLoopStart;
                    }
                }
            }

            return fNewTime;
        }
    } // end of namespace animation
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
