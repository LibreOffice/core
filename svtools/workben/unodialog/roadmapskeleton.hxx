/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: roadmapskeleton.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:27:54 $
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

#ifndef SVTOOLS_ROADMAPSKELETONDIALOG_HXX
#define SVTOOLS_ROADMAPSKELETONDIALOG_HXX

#include <comphelper/componentcontext.hxx>
#include <svtools/roadmapwizard.hxx>

//........................................................................
namespace udlg
{
//........................................................................

    //====================================================================
    //= RoadmapSkeletonDialog
    //====================================================================
    typedef ::svt::RoadmapWizard    RoadmapSkeletonDialog_Base;
    class RoadmapSkeletonDialog : public RoadmapSkeletonDialog_Base
    {
    public:
        RoadmapSkeletonDialog( const ::comphelper::ComponentContext& _rContext, Window* _pParent );
        virtual ~RoadmapSkeletonDialog();

    protected:
        // OWizardMachine overridables
        virtual void            enterState( WizardState _nState );
        virtual sal_Bool        prepareLeaveCurrentState( CommitPageReason _eReason );
        virtual sal_Bool        leaveState( WizardState _nState );
        virtual WizardState     determineNextState( WizardState _nCurrentState ) const;
        virtual sal_Bool        onFinish( sal_Int32 _nResult );

    private:
        ::comphelper::ComponentContext  m_aContext;
    };

//........................................................................
} // namespace udlg
//........................................................................

#endif // SVTOOLS_ROADMAPSKELETONDIALOG_HXX
