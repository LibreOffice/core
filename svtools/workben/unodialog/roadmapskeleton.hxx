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
        virtual sal_Bool        onFinish();

    private:
        ::comphelper::ComponentContext  m_aContext;
    };

//........................................................................
} // namespace udlg
//........................................................................

#endif // SVTOOLS_ROADMAPSKELETONDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
