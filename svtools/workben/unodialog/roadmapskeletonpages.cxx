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
