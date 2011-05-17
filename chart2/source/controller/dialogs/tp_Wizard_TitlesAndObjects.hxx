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

#ifndef _CHART2_TP_WIZARD_TITLESANDOBJECTS_HXX
#define _CHART2_TP_WIZARD_TITLESANDOBJECTS_HXX

#include "TimerTriggeredControllerLock.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <svtools/wizardmachine.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
//for auto_ptr
#include <memory>
#include <com/sun/star/uno/XComponentContext.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class TitleResources;
class LegendPositionResources;
class TitlesAndObjectsTabPage : public svt::OWizardPage
{
public:
    TitlesAndObjectsTabPage( svt::OWizardMachine* pParent
                , const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartDocument >& xChartModel
                , const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext >& xContext );
    virtual ~TitlesAndObjectsTabPage();

    virtual void        initializePage();
    virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason eReason );
    virtual bool        canAdvance() const;

protected:
    void commitToModel();
    DECL_LINK( ChangeHdl, void* );

protected:
    FixedText           m_aFT_TitleDescription;
    FixedLine           m_aFL_Vertical;

    ::std::auto_ptr< TitleResources >            m_apTitleResources;
    ::std::auto_ptr< LegendPositionResources >   m_apLegendPositionResources;

    FixedLine           m_aFL_Grids;
    CheckBox            m_aCB_Grid_X;
    CheckBox            m_aCB_Grid_Y;
    CheckBox            m_aCB_Grid_Z;

    ::com::sun::star::uno::Reference<
                       ::com::sun::star::chart2::XChartDocument >   m_xChartModel;
    ::com::sun::star::uno::Reference<
                       ::com::sun::star::uno::XComponentContext>    m_xCC;

    bool    m_bCommitToModel;
    TimerTriggeredControllerLock   m_aTimerTriggeredControllerLock;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
