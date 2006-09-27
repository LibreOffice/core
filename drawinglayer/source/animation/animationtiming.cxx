/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationtiming.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2006-09-27 16:32:56 $
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

#ifndef _DRAWINGLAYER_ANIMATION_ANIMATIONTIMING_HXX
#include <drawinglayer/animation/animationtiming.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace animation
    {
        //////////////////////////////////////////////////////////////////////////////

        animationEntry::animationEntry()
        {
        }

        animationEntry::~animationEntry()
        {
        }

        //////////////////////////////////////////////////////////////////////////////

        animationEntryFixed::animationEntryFixed(double fDuration, double fState)
        :   mfDuration(fDuration),
            mfState(fState)
        {
        }

        animationEntryFixed::~animationEntryFixed()
        {
        }

        animationEntry* animationEntryFixed::clone() const
        {
            return new animationEntryFixed(mfDuration, mfState);
        }

        bool animationEntryFixed::operator==(const animationEntry& rCandidate) const
        {
            const animationEntryFixed* pCompare = dynamic_cast< const animationEntryFixed* >(&rCandidate);

            return (pCompare
                && basegfx::fTools::equal(mfDuration, pCompare->mfDuration)
                && basegfx::fTools::equal(mfState, pCompare->mfState));
        }

        double animationEntryFixed::getDuration() const
        {
            return mfDuration;
        }

        double animationEntryFixed::getStateAtTime(double /*fTime*/) const
        {
            return mfState;
        }

        double animationEntryFixed::getNextEventTime(double fTime) const
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

        animationEntryLinear::animationEntryLinear(double fDuration, double fFrequency, double fStart, double fStop)
        :   mfDuration(fDuration),
            mfFrequency(fFrequency),
            mfStart(fStart),
            mfStop(fStop)
        {
        }

        animationEntryLinear::~animationEntryLinear()
        {
        }

        animationEntry* animationEntryLinear::clone() const
        {
            return new animationEntryLinear(mfDuration, mfFrequency, mfStart, mfStop);
        }

        bool animationEntryLinear::operator==(const animationEntry& rCandidate) const
        {
            const animationEntryLinear* pCompare = dynamic_cast< const animationEntryLinear* >(&rCandidate);

            return (pCompare
                && basegfx::fTools::equal(mfDuration, pCompare->mfDuration)
                && basegfx::fTools::equal(mfStart, pCompare->mfStart)
                && basegfx::fTools::equal(mfStop, pCompare->mfStop));
        }

        double animationEntryLinear::getDuration() const
        {
            return mfDuration;
        }

        double animationEntryLinear::getStateAtTime(double fTime) const
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

        double animationEntryLinear::getNextEventTime(double fTime) const
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

        sal_uInt32 animationEntryList::impGetIndexAtTime(double fTime, double &rfAddedTime) const
        {
            sal_uInt32 nIndex(0L);

            while(nIndex < maEntries.size() && basegfx::fTools::lessOrEqual(rfAddedTime + maEntries[nIndex]->getDuration(), fTime))
            {
                rfAddedTime += maEntries[nIndex++]->getDuration();
            }

            return nIndex;
        }

        animationEntryList::animationEntryList()
        :   mfDuration(0.0)
        {
        }

        animationEntryList::~animationEntryList()
        {
            for(sal_uInt32 a(0L); a < maEntries.size(); a++)
            {
                delete maEntries[a];
            }
        }

        animationEntry* animationEntryList::clone() const
        {
            animationEntryList* pNew = new animationEntryList();

            for(sal_uInt32 a(0L); a < maEntries.size(); a++)
            {
                pNew->append(*maEntries[a]);
            }

            return pNew;
        }

        bool animationEntryList::operator==(const animationEntry& rCandidate) const
        {
            const animationEntryList* pCompare = dynamic_cast< const animationEntryList* >(&rCandidate);

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

        void animationEntryList::append(const animationEntry& rCandidate)
        {
            const double fDuration(rCandidate.getDuration());

            if(!basegfx::fTools::equalZero(fDuration))
            {
                maEntries.push_back(rCandidate.clone());
                mfDuration += fDuration;
            }
        }

        double animationEntryList::getDuration() const
        {
            return mfDuration;
        }

        double animationEntryList::getStateAtTime(double fTime) const
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

        double animationEntryList::getNextEventTime(double fTime) const
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

        animationEntryLoop::animationEntryLoop(sal_uInt32 nRepeat)
        :   animationEntryList(),
            mnRepeat(nRepeat)
        {
        }

        animationEntryLoop::~animationEntryLoop()
        {
        }

        animationEntry* animationEntryLoop::clone() const
        {
            animationEntryLoop* pNew = new animationEntryLoop(mnRepeat);

            for(sal_uInt32 a(0L); a < maEntries.size(); a++)
            {
                pNew->append(*maEntries[a]);
            }

            return pNew;
        }

        bool animationEntryLoop::operator==(const animationEntry& rCandidate) const
        {
            const animationEntryLoop* pCompare = dynamic_cast< const animationEntryLoop* >(&rCandidate);

            return (pCompare
                && mnRepeat == pCompare->mnRepeat
                && animationEntryList::operator==(rCandidate));
        }

        double animationEntryLoop::getDuration() const
        {
            return (mfDuration * (double)mnRepeat);
        }

        double animationEntryLoop::getStateAtTime(double fTime) const
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
                    return animationEntryList::getStateAtTime(fRelativeTime);
                }
            }

            return 0.0;
        }

        double animationEntryLoop::getNextEventTime(double fTime) const
        {
            double fNewTime(0.0);

            if(mnRepeat && !basegfx::fTools::equalZero(mfDuration))
            {
                const sal_uInt32 nCurrentLoop((sal_uInt32)(fTime / mfDuration));

                if(nCurrentLoop <= mnRepeat)
                {
                    const double fTimeAtLoopStart((double)nCurrentLoop * mfDuration);
                    const double fRelativeTime(fTime - fTimeAtLoopStart);
                    const double fNextEventAtLoop(animationEntryList::getNextEventTime(fRelativeTime));

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
