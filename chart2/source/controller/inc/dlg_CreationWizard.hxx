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

#ifndef _CHART2_CREATION_WIZARD_HXX
#define _CHART2_CREATION_WIZARD_HXX

#include "ServiceMacros.hxx"
#include "TimerTriggeredControllerLock.hxx"
#include "TabPageNotifiable.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <svtools/roadmapwizard.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

// for auto_ptr
#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................

class RangeChooserTabPage;
class DataSourceTabPage;
class ChartTypeTemplateProvider;
class DialogModel;

class CreationWizard : public
    svt::RoadmapWizard
    , public TabPageNotifiable
{
public:
    CreationWizard( Window* pParent,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel >& xChartModel
        , const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >& xContext
        , sal_Int32 nOnePageOnlyIndex=-1 );//if nOnePageOnlyIndex is an index of an exsisting  page starting with 0, then only this page is displayed without next/previous and roadmap
    virtual ~CreationWizard();

    bool isClosable();

    // TabPageNotifiable
    virtual void setInvalidPage( TabPage * pTabPage );
    virtual void setValidPage( TabPage * pTabPage );

protected:
    virtual sal_Bool        leaveState( WizardState _nState );
    virtual WizardState     determineNextState(WizardState nCurrentState) const;
    virtual void            enterState(WizardState nState);

    virtual String  getStateDisplayName( WizardState nState ) const;

private:
    //no default constructor
    CreationWizard();

    virtual svt::OWizardPage* createPage(WizardState nState);

    ::com::sun::star::uno::Reference<
                       ::com::sun::star::chart2::XChartDocument >   m_xChartModel;
    ::com::sun::star::uno::Reference<
                       ::com::sun::star::uno::XComponentContext>    m_xCC;
    bool m_bIsClosable;
    sal_Int32 m_nOnePageOnlyIndex;//if nOnePageOnlyIndex is an index of an exsisting page starting with 0, then only this page is displayed without next/previous and roadmap
    ChartTypeTemplateProvider*   m_pTemplateProvider;
    ::std::auto_ptr< DialogModel > m_apDialogModel;

    WizardState m_nFirstState;
    WizardState m_nLastState;

    TimerTriggeredControllerLock   m_aTimerTriggeredControllerLock;

//     RangeChooserTabPage * m_pRangeChooserTabePage;
//     DataSourceTabPage *   m_pDataSourceTabPage;
    bool                  m_bCanTravel;
};
//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
