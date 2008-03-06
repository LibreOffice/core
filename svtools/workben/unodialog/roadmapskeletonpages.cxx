/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: roadmapskeletonpages.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:28:23 $
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
