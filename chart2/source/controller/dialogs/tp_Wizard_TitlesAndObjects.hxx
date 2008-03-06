/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_Wizard_TitlesAndObjects.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:18:23 $
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

#ifndef _CHART2_TP_WIZARD_TITLESANDOBJECTS_HXX
#define _CHART2_TP_WIZARD_TITLESANDOBJECTS_HXX

#include "TimerTriggeredControllerLock.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>

#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
#endif

#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
//for auto_ptr
#include <memory>

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

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
    virtual sal_Bool    commitPage( CommitPageReason eReason );
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
