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

#include "udlg_module.hxx"
#include "udlg_global.hrc"
#include "roadmapskeletonpages.hxx"
#include "roadmapskeleton.hxx"

//........................................................................
namespace udlg
{
//........................................................................

    //====================================================================
    //= RoadmapSkeletonPage
    //====================================================================
    //--------------------------------------------------------------------
    RoadmapSkeletonPage::RoadmapSkeletonPage( RoadmapSkeletonDialog& _rParentDialog, const ResId& _rRes )
        :RoadmapSkeletonPage_Base( &_rParentDialog, _rRes )
    {
    }

    //--------------------------------------------------------------------
    RoadmapSkeletonPage::~RoadmapSkeletonPage()
    {
    }

    //====================================================================
    //= WelcomePage
    //====================================================================
    //--------------------------------------------------------------------
    WelcomePage::WelcomePage( RoadmapSkeletonDialog& _rParentDialog )
        :RoadmapSkeletonPage( _rParentDialog, UdlgResId( TP_WELCOME ) )
    {
    }

    //--------------------------------------------------------------------
    TabPage* WelcomePage::Create( ::svt::RoadmapWizard& _rParentDialog )
    {
        return new WelcomePage( dynamic_cast< RoadmapSkeletonDialog& >( _rParentDialog ) );
    }

    //====================================================================
    //= PreparationPage
    //====================================================================
    //--------------------------------------------------------------------
    PreparationPage::PreparationPage( RoadmapSkeletonDialog& _rParentDialog )
        :RoadmapSkeletonPage( _rParentDialog, UdlgResId( TP_PREPARE ) )
    {
    }

    //--------------------------------------------------------------------
    TabPage* PreparationPage::Create( ::svt::RoadmapWizard& _rParentDialog )
    {
        return new PreparationPage( dynamic_cast< RoadmapSkeletonDialog& >( _rParentDialog ) );
    }

    //====================================================================
    //= SetupPage
    //====================================================================
    //--------------------------------------------------------------------
    SetupPage::SetupPage( RoadmapSkeletonDialog& _rParentDialog )
        :RoadmapSkeletonPage( _rParentDialog, UdlgResId( TP_SETUP ) )
    {
    }

    //--------------------------------------------------------------------
    TabPage* SetupPage::Create( ::svt::RoadmapWizard& _rParentDialog )
    {
        return new SetupPage( dynamic_cast< RoadmapSkeletonDialog& >( _rParentDialog ) );
    }

    //====================================================================
    //= FinishPage
    //====================================================================
    //--------------------------------------------------------------------
    FinishPage::FinishPage( RoadmapSkeletonDialog& _rParentDialog )
        :RoadmapSkeletonPage( _rParentDialog, UdlgResId( TP_FINISH ) )
    {
    }

    //--------------------------------------------------------------------
    TabPage* FinishPage::Create( ::svt::RoadmapWizard& _rParentDialog )
    {
        return new FinishPage( dynamic_cast< RoadmapSkeletonDialog& >( _rParentDialog ) );
    }

//........................................................................
} // namespace udlg
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
