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

#ifndef INCLUDED_DRAWINGLAYER_ANIMATION_ANIMATIONTIMING_HXX
#define INCLUDED_DRAWINGLAYER_ANIMATION_ANIMATIONTIMING_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <sal/types.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace animation
    {
        //////////////////////////////////////////////////////////////////////////////

        class DRAWINGLAYER_DLLPUBLIC AnimationEntry
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

        class DRAWINGLAYER_DLLPUBLIC AnimationEntryFixed : public AnimationEntry
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

        class DRAWINGLAYER_DLLPUBLIC AnimationEntryLinear : public AnimationEntry
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

        class DRAWINGLAYER_DLLPUBLIC AnimationEntryList : public AnimationEntry
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

        class DRAWINGLAYER_DLLPUBLIC AnimationEntryLoop : public AnimationEntryList
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
