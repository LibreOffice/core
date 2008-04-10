/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: roadmapskeletonpages.cxx,v $
 * $Revision: 1.3 $
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

    //--------------------------------------------------------------------
    const RoadmapSkeletonDialog& RoadmapSkeletonPage::getDialog() const
    {
        return *dynamic_cast< const RoadmapSkeletonDialog* >( GetParent() );
    }

    //--------------------------------------------------------------------
    RoadmapSkeletonDialog& RoadmapSkeletonPage::getDialog()
    {
        return *dynamic_cast< RoadmapSkeletonDialog* >( GetParent() );
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
