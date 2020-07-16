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

#ifndef INCLUDED_DRAWINGLAYER_ANIMATION_ANIMATIONTIMING_HXX
#define INCLUDED_DRAWINGLAYER_ANIMATION_ANIMATIONTIMING_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <vector>
#include <memory>


namespace drawinglayer::animation
    {


        class DRAWINGLAYER_DLLPUBLIC AnimationEntry
        {
        private:
            AnimationEntry(const AnimationEntry&) = delete;
            AnimationEntry& operator=(const AnimationEntry& rCandidate) = delete;

        public:
            AnimationEntry();
            virtual ~AnimationEntry();
            virtual std::unique_ptr<AnimationEntry> clone() const = 0;

            virtual bool operator==(const AnimationEntry& rCandidate) const = 0;
            virtual double getDuration() const = 0;
            virtual double getStateAtTime(double fTime) const = 0;
            virtual double getNextEventTime(double fTime) const = 0;
        };


        class DRAWINGLAYER_DLLPUBLIC AnimationEntryFixed final : public AnimationEntry
        {
            double                                      mfDuration;
            double                                      mfState;

        public:
            AnimationEntryFixed(double fDuration, double fState);
            virtual ~AnimationEntryFixed() override;
            virtual std::unique_ptr<AnimationEntry> clone() const override;

            virtual bool operator==(const AnimationEntry& rCandidate) const override;
            virtual double getDuration() const override;
            virtual double getStateAtTime(double fTime) const override;
            virtual double getNextEventTime(double fTime) const override;
        };


        class DRAWINGLAYER_DLLPUBLIC AnimationEntryLinear final : public AnimationEntry
        {
            double                                      mfDuration;
            double                                      mfFrequency;
            double                                      mfStart;
            double                                      mfStop;

        public:
            AnimationEntryLinear(double fDuration, double fFrequency, double fStart, double fStop);
            virtual ~AnimationEntryLinear() override;
            virtual std::unique_ptr<AnimationEntry> clone() const override;

            virtual bool operator==(const AnimationEntry& rCandidate) const override;
            virtual double getDuration() const override;
            virtual double getStateAtTime(double fTime) const override;
            virtual double getNextEventTime(double fTime) const override;
        };


        class DRAWINGLAYER_DLLPUBLIC AnimationEntryList : public AnimationEntry
        {
        protected:
            using Entries = std::vector<std::unique_ptr<AnimationEntry>>;

            double                                      mfDuration;
            Entries maEntries;

            // helpers
            Entries::size_type impGetIndexAtTime(double fTime, double &rfAddedTime) const;

        public:
            AnimationEntryList();
            virtual ~AnimationEntryList() override;
            virtual std::unique_ptr<AnimationEntry> clone() const override;

            virtual bool operator==(const AnimationEntry& rCandidate) const override;
            void append(const AnimationEntry& rCandidate);
            virtual double getDuration() const override;
            virtual double getStateAtTime(double fTime) const override;
            virtual double getNextEventTime(double fTime) const override;
        };


        class DRAWINGLAYER_DLLPUBLIC AnimationEntryLoop final : public AnimationEntryList
        {
            sal_uInt32                                  mnRepeat;

        public:
            AnimationEntryLoop(sal_uInt32 nRepeat);
            virtual ~AnimationEntryLoop() override;
            virtual std::unique_ptr<AnimationEntry> clone() const override;

            virtual bool operator==(const AnimationEntry& rCandidate) const override;
            virtual double getDuration() const override;
            virtual double getStateAtTime(double fTime) const override;
            virtual double getNextEventTime(double fTime) const override;
        };


} // end of namespace drawinglayer::animation


#endif //INCLUDED_DRAWINGLAYER_ANIMATION_ANIMATIONTIMING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
