/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationtiming.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:39:25 $
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
#define _DRAWINGLAYER_ANIMATION_ANIMATIONTIMING_HXX

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

        class animationEntry
        {
        private:
            animationEntry(const animationEntry&);
            animationEntry& operator=(const animationEntry& rCandidate);

        public:
            animationEntry();
            virtual ~animationEntry();
            virtual animationEntry* clone() const = 0;

            virtual bool operator==(const animationEntry& rCandidate) const = 0;
            virtual double getDuration() const = 0;
            virtual double getStateAtTime(double fTime) const = 0;
            virtual double getNextEventTime(double fTime) const = 0;
        };

        //////////////////////////////////////////////////////////////////////////////

        class animationEntryFixed : public animationEntry
        {
        protected:
            double                                      mfDuration;
            double                                      mfState;

        public:
            animationEntryFixed(double fDuration, double fState = 0.0);
            virtual ~animationEntryFixed();
            virtual animationEntry* clone() const;

            virtual bool operator==(const animationEntry& rCandidate) const;
            virtual double getDuration() const;
            virtual double getStateAtTime(double fTime) const;
            virtual double getNextEventTime(double fTime) const;
        };

        //////////////////////////////////////////////////////////////////////////////

        class animationEntryLinear : public animationEntry
        {
        protected:
            double                                      mfDuration;
            double                                      mfFrequency;
            double                                      mfStart;
            double                                      mfStop;

        public:
            animationEntryLinear(double fDuration, double fFrequency = 250.0, double fStart = 0.0, double fStop = 1.0);
            virtual ~animationEntryLinear();
            virtual animationEntry* clone() const;

            virtual bool operator==(const animationEntry& rCandidate) const;
            virtual double getDuration() const;
            virtual double getStateAtTime(double fTime) const;
            virtual double getNextEventTime(double fTime) const;
        };

        //////////////////////////////////////////////////////////////////////////////

        class animationEntryList : public animationEntry
        {
        protected:
            double                                      mfDuration;
            ::std::vector< animationEntry* >            maEntries;

            // helpers
            sal_uInt32 impGetIndexAtTime(double fTime, double &rfAddedTime) const;

        public:
            animationEntryList();
            virtual ~animationEntryList();
            virtual animationEntry* clone() const;

            virtual bool operator==(const animationEntry& rCandidate) const;
            void append(const animationEntry& rCandidate);
            virtual double getDuration() const;
            virtual double getStateAtTime(double fTime) const;
            virtual double getNextEventTime(double fTime) const;
        };

        //////////////////////////////////////////////////////////////////////////////

        class animationEntryLoop : public animationEntryList
        {
        protected:
            sal_uInt32                                  mnRepeat;

        public:
            animationEntryLoop(sal_uInt32 nRepeat = 0xffffffff);
            virtual ~animationEntryLoop();
            virtual animationEntry* clone() const;

            virtual bool operator==(const animationEntry& rCandidate) const;
            virtual double getDuration() const;
            virtual double getStateAtTime(double fTime) const;
            virtual double getNextEventTime(double fTime) const;
        };

        //////////////////////////////////////////////////////////////////////////////
    } // end of namespace animation
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_ANIMATION_ANIMATIONTIMING_HXX

// eof
