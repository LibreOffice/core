/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: roadmapskeletonpages.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:28:37 $
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

    protected:
        const RoadmapSkeletonDialog& getDialog() const;
              RoadmapSkeletonDialog& getDialog()      ;
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
