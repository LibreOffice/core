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

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_WIZARD_TITLESANDOBJECTS_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_WIZARD_TITLESANDOBJECTS_HXX

#include "TimerTriggeredControllerLock.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <svtools/wizardmachine.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <boost/scoped_ptr.hpp>

#include "res_LegendPosition.hxx"
#include "res_Titles.hxx"

namespace chart
{

class TitlesAndObjectsTabPage : public svt::OWizardPage
{
public:
    TitlesAndObjectsTabPage( svt::OWizardMachine* pParent
                , const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartDocument >& xChartModel
                , const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext >& xContext );

    virtual void        initializePage();
    virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason eReason );
    virtual bool        canAdvance() const;

protected:
    void commitToModel();
    DECL_LINK( ChangeHdl, void* );

protected:
    boost::scoped_ptr< TitleResources >            m_xTitleResources;
    boost::scoped_ptr< LegendPositionResources >   m_xLegendPositionResources;

    CheckBox*          m_pCB_Grid_X;
    CheckBox*          m_pCB_Grid_Y;
    CheckBox*          m_pCB_Grid_Z;

    ::com::sun::star::uno::Reference<
                       ::com::sun::star::chart2::XChartDocument >   m_xChartModel;
    ::com::sun::star::uno::Reference<
                       ::com::sun::star::uno::XComponentContext>    m_xCC;

    bool    m_bCommitToModel;
    TimerTriggeredControllerLock   m_aTimerTriggeredControllerLock;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
