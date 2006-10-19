/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationtiming.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:30:10 $
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

#ifndef INCLUDED_DRAWINGLAYER_ANIMATION_ANIMATIONTIMING_HXX
#define INCLUDED_DRAWINGLAYER_ANIMATION_ANIMATIONTIMING_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <vector>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace animation
    {
        //////////////////////////////////////////////////////////////////////////////

        class AnimationEntry
        {
        private:
            AnimationEntry(const AnimationEntry&);
            AnimationEntry& operator=(const AnimationEntry& rCandidate);

        public:
            AnimationEntry();
            virtual ~AnimationEntry();
            virtual AnimationEntry* clone() const = 0;

            virtual bool operator==(const AnimationEntry& rCandidate) const = 0;
            virtual double getDuration() const = 0;
            virtual double getStateAtTime(double fTime) const = 0;
            virtual double getNextEventTime(double fTime) const = 0;
        };

        //////////////////////////////////////////////////////////////////////////////

        class AnimationEntryFixed : public AnimationEntry
        {
        protected:
            double                                      mfDuration;
            double                                      mfState;

        public:
            AnimationEntryFixed(double fDuration, double fState = 0.0);
            virtual ~AnimationEntryFixed();
            virtual AnimationEntry* clone() const;

            virtual bool operator==(const AnimationEntry& rCandidate) const;
            virtual double getDuration() const;
            virtual double getStateAtTime(double fTime) const;
            virtual double getNextEventTime(double fTime) const;
        };

        //////////////////////////////////////////////////////////////////////////////

        class AnimationEntryLinear : public AnimationEntry
        {
        protected:
            double                                      mfDuration;
            double                                      mfFrequency;
            double                                      mfStart;
            double                                      mfStop;

        public:
            AnimationEntryLinear(double fDuration, double fFrequency = 250.0, double fStart = 0.0, double fStop = 1.0);
            virtual ~AnimationEntryLinear();
            virtual AnimationEntry* clone() const;

            virtual bool operator==(const AnimationEntry& rCandidate) const;
            virtual double getDuration() const;
            virtual double getStateAtTime(double fTime) const;
            virtual double getNextEventTime(double fTime) const;
        };

        //////////////////////////////////////////////////////////////////////////////

        class AnimationEntryList : public AnimationEntry
        {
        protected:
            double                                      mfDuration;
            ::std::vector< AnimationEntry* >            maEntries;

            // helpers
            sal_uInt32 impGetIndexAtTime(double fTime, double &rfAddedTime) const;

        public:
            AnimationEntryList();
            virtual ~AnimationEntryList();
            virtual AnimationEntry* clone() const;

            virtual bool operator==(const AnimationEntry& rCandidate) const;
            void append(const AnimationEntry& rCandidate);
            virtual double getDuration() const;
            virtual double getStateAtTime(double fTime) const;
            virtual double getNextEventTime(double fTime) const;
        };

        //////////////////////////////////////////////////////////////////////////////

        class AnimationEntryLoop : public AnimationEntryList
        {
        protected:
            sal_uInt32                                  mnRepeat;

        public:
            AnimationEntryLoop(sal_uInt32 nRepeat = 0xffffffff);
            virtual ~AnimationEntryLoop();
            virtual AnimationEntry* clone() const;

            virtual bool operator==(const AnimationEntry& rCandidate) const;
            virtual double getDuration() const;
            virtual double getStateAtTime(double fTime) const;
            virtual double getNextEventTime(double fTime) const;
        };

        //////////////////////////////////////////////////////////////////////////////
    } // end of namespace animation
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ANIMATION_ANIMATIONTIMING_HXX

// eof
