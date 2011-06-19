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

#ifndef SVTOOLS_ROADMAPSKELETONPAGES_HXX
#define SVTOOLS_ROADMAPSKELETONPAGES_HXX

#include <svtools/wizardmachine.hxx>

namespace svt
{
    class RoadmapWizard;
}

//........................................................................
namespace udlg
{
//........................................................................

    class RoadmapSkeletonDialog;

    //====================================================================
    //= RoadmapSkeletonPage
    //====================================================================
    typedef ::svt::OWizardPage  RoadmapSkeletonPage_Base;
    class RoadmapSkeletonPage : public RoadmapSkeletonPage_Base
    {
    public:
        RoadmapSkeletonPage( RoadmapSkeletonDialog& _rParentDialog, const ResId& _rRes );
        ~RoadmapSkeletonPage();
    };

    //====================================================================
    //= WelcomePage
    //====================================================================
    class WelcomePage : public RoadmapSkeletonPage
    {
    public:
        WelcomePage( RoadmapSkeletonDialog& _rParentDialog );

        static TabPage* Create( ::svt::RoadmapWizard& _rParentDialog );
    };

    //====================================================================
    //= PreparationPage
    //====================================================================
    class PreparationPage : public RoadmapSkeletonPage
    {
    public:
        PreparationPage( RoadmapSkeletonDialog& _rParentDialog );

        static TabPage* Create( ::svt::RoadmapWizard& _rParentDialog );
    };

    //====================================================================
    //= SetupPage
    //====================================================================
    class SetupPage : public RoadmapSkeletonPage
    {
    public:
        SetupPage( RoadmapSkeletonDialog& _rParentDialog );

        static TabPage* Create( ::svt::RoadmapWizard& _rParentDialog );
    };

    //====================================================================
    //= FinishPage
    //====================================================================
    class FinishPage : public RoadmapSkeletonPage
    {
    public:
        FinishPage( RoadmapSkeletonDialog& _rParentDialog );

        static TabPage* Create( ::svt::RoadmapWizard& _rParentDialog );
    };

//........................................................................
} // namespace udlg
//........................................................................

#endif // SVTOOLS_ROADMAPSKELETONPAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
