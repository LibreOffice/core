/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_CreationWizard.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:18:39 $
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

#ifndef _CHART2_CREATION_WIZARD_HXX
#define _CHART2_CREATION_WIZARD_HXX

#include "ServiceMacros.hxx"
#include "TimerTriggeredControllerLock.hxx"
#include "TabPageNotifiable.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>

#ifndef SVTOOLS_INC_ROADMAPWIZARD_HXX
#include <svtools/roadmapwizard.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

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
