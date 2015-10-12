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

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_CREATIONWIZARD_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_CREATIONWIZARD_HXX

#include "TimerTriggeredControllerLock.hxx"
#include "TabPageNotifiable.hxx"
#include "../dialogs/DialogModel.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <svtools/roadmapwizard.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <boost/noncopyable.hpp>
#include <memory>

namespace chart
{

class RangeChooserTabPage;
class DataSourceTabPage;
class ChartTypeTemplateProvider;

class CreationWizard : public svt::RoadmapWizard, public TabPageNotifiable
{
public:
    CreationWizard( vcl::Window* pParent,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel >& xChartModel
        , const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >& xContext
        , sal_Int32 nOnePageOnlyIndex=-1 );//if nOnePageOnlyIndex is an index of an exsisting  page starting with 0, then only this page is displayed without next/previous and roadmap

    bool isClosable() { /*@todo*/ return m_bIsClosable;}

    // TabPageNotifiable
    virtual void setInvalidPage( TabPage * pTabPage ) override;
    virtual void setValidPage( TabPage * pTabPage ) override;

protected:
    virtual bool            leaveState( WizardState _nState ) override;
    virtual WizardState     determineNextState(WizardState nCurrentState) const override;
    virtual void            enterState(WizardState nState) override;

    virtual OUString        getStateDisplayName( WizardState nState ) const override;

private:
    //no default constructor
    CreationWizard();

    virtual VclPtr<TabPage> createPage(WizardState nState) override;

    ::com::sun::star::uno::Reference<
                       ::com::sun::star::chart2::XChartDocument >   m_xChartModel;
    ::com::sun::star::uno::Reference<
                       ::com::sun::star::uno::XComponentContext>    m_xCC;
    bool m_bIsClosable;
    sal_Int32 m_nOnePageOnlyIndex;//if nOnePageOnlyIndex is an index of an exsisting page starting with 0, then only this page is displayed without next/previous and roadmap
    ChartTypeTemplateProvider*   m_pTemplateProvider;
    std::unique_ptr<DialogModel> m_pDialogModel;

    WizardState m_nFirstState;
    WizardState m_nLastState;

    TimerTriggeredControllerLock   m_aTimerTriggeredControllerLock;

//     RangeChooserTabPage * m_pRangeChooserTabPage;
//     DataSourceTabPage *   m_pDataSourceTabPage;
    bool                  m_bCanTravel;
};
} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
