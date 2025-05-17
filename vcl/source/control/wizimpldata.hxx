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

#pragma once

#include <stack>
#include <map>
#include <set>
#include <vcl/toolkit/roadmap.hxx>

constexpr OUString HID_WIZARD_NEXT = u"SVT_HID_WIZARD_NEXT"_ustr;
constexpr OUString HID_WIZARD_PREVIOUS = u"SVT_HID_WIZARD_PREVIOUS"_ustr;

struct WizPageData
{
    WizPageData*    mpNext;
    std::unique_ptr<BuilderPage> mxPage;
};

namespace vcl
{
    struct WizardMachineImplData
    {
        OUString                        sTitleBase;         // the base for the title
        std::stack<WizardTypes::WizardState> aStateHistory;      // the history of all states (used for implementing "Back")

        WizardTypes::WizardState nFirstUnknownPage;
            // the WizardDialog does not allow non-linear transitions (e.g. it's
            // not possible to add pages in a non-linear order), so we need some own maintenance data

        bool                            m_bAutoNextButtonState;

        bool                            m_bTravelingSuspended;

        WizardMachineImplData()
            :nFirstUnknownPage( 0 )
            ,m_bAutoNextButtonState( false )
            ,m_bTravelingSuspended( false )
        {
        }
    };

    using namespace RoadmapWizardTypes;
    namespace
    {
        typedef ::std::set< WizardTypes::WizardState > StateSet;

        typedef ::std::map<
                    PathId,
                    WizardPath
                > Paths;

        typedef ::std::map<
                    WizardTypes::WizardState,
                    ::std::pair<
                        OUString,
                        RoadmapPageFactory
                    >
                > StateDescriptions;
    }

    struct RoadmapWizardImpl
    {
        ScopedVclPtr<ORoadmap> pRoadmap;
        std::map<VclPtr<vcl::Window>, short> maResponses;
        Paths               aPaths;
        PathId              nActivePath;
        StateDescriptions   aStateDescriptors;
        StateSet            aDisabledStates;
        bool                bActivePathIsDefinite;

        RoadmapWizardImpl()
            :pRoadmap( nullptr )
            ,nActivePath( PathId::INVALID )
            ,bActivePathIsDefinite( false )
        {
        }

        /// returns the index of the current state in given path, or -1
        static sal_Int32 getStateIndexInPath( WizardTypes::WizardState _nState, const WizardPath& _rPath );
        /// returns the index of the current state in the path with the given id, or -1
        sal_Int32 getStateIndexInPath( WizardTypes::WizardState _nState, PathId _nPathId );
        /// returns the index of the first state in which the two given paths differ
        static sal_Int32 getFirstDifferentIndex( const WizardPath& _rLHS, const WizardPath& _rRHS );
    };
}   // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
