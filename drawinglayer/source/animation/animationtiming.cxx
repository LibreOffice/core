/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <memory>

#include <drawinglayer/animation/animationtiming.hxx>
#include <basegfx/numeric/ftools.hxx>

namespace drawinglayer
{
    namespace animation
    {


        AnimationEntry::AnimationEntry()
        {
        }

        AnimationEntry::~AnimationEntry()
        {
        }


        AnimationEntryFixed::AnimationEntryFixed(double fDuration, double fState)
        :   mfDuration(fDuration),
            mfState(fState)
        {
        }

        AnimationEntryFixed::~AnimationEntryFixed()
        {
        }

        std::unique_ptr<AnimationEntry> AnimationEntryFixed::clone() const
        {
            return std::make_unique<AnimationEntryFixed>(mfDuration, mfState);
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

        std::unique_ptr<AnimationEntry> AnimationEntryLinear::clone() const
        {
            return std::make_unique<AnimationEntryLinear>(mfDuration, mfFrequency, mfStart, mfStop);
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


        AnimationEntryList::Entries::size_type AnimationEntryList::impGetIndexAtTime(double fTime, double &rfAddedTime) const
        {
            Entries::size_type nIndex(0);

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
        }

        std::unique_ptr<AnimationEntry> AnimationEntryList::clone() const
        {
            std::unique_ptr<AnimationEntryList> pNew(std::make_unique<AnimationEntryList>());

            for(const auto &i : maEntries)
            {
                pNew->append(*i);
            }

            return pNew;
        }

        bool AnimationEntryList::operator==(const AnimationEntry& rCandidate) const
        {
            const AnimationEntryList* pCompare = dynamic_cast< const AnimationEntryList* >(&rCandidate);

            if(pCompare && mfDuration == pCompare->mfDuration)
            {
                for(size_t a(0); a < maEntries.size(); a++)
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
                const auto nIndex(impGetIndexAtTime(fTime, fAddedTime));

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
                const auto nIndex(impGetIndexAtTime(fTime, fAddedTime));

                if(nIndex < maEntries.size())
                {
                    fNewTime = maEntries[nIndex]->getNextEventTime(fTime - fAddedTime) + fAddedTime;
                }
            }

            return fNewTime;
        }


        AnimationEntryLoop::AnimationEntryLoop(sal_uInt32 nRepeat)
        :   AnimationEntryList(),
            mnRepeat(nRepeat)
        {
        }

        AnimationEntryLoop::~AnimationEntryLoop()
        {
        }

        std::unique_ptr<AnimationEntry> AnimationEntryLoop::clone() const
        {
            std::unique_ptr<AnimationEntryLoop> pNew(std::make_unique<AnimationEntryLoop>(mnRepeat));

            for(const auto &i : maEntries)
            {
                pNew->append(*i);
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
            return (mfDuration * static_cast<double>(mnRepeat));
        }

        double AnimationEntryLoop::getStateAtTime(double fTime) const
        {
            if(mnRepeat && !basegfx::fTools::equalZero(mfDuration))
            {
                const sal_uInt32 nCurrentLoop(static_cast<sal_uInt32>(fTime / mfDuration));

                if(nCurrentLoop > mnRepeat)
                {
                    return 1.0;
                }
                else
                {
                    const double fTimeAtLoopStart(static_cast<double>(nCurrentLoop) * mfDuration);
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
                const sal_uInt32 nCurrentLoop(static_cast<sal_uInt32>(fTime / mfDuration));

                if(nCurrentLoop <= mnRepeat)
                {
                    const double fTimeAtLoopStart(static_cast<double>(nCurrentLoop) * mfDuration);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
