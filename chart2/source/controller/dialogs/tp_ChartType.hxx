/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_ChartType.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:17:12 $
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

#ifndef _CHART2_TP_CHARTTYPE_HXX
#define _CHART2_TP_CHARTTYPE_HXX

#include <vector>

#include "ChartTypeDialogController.hxx"
#include "ChartTypeTemplateProvider.hxx"
#include "TimerTriggeredControllerLock.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>

#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
#endif

#ifndef _VALUESET_HXX
#include <svtools/valueset.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for class MetricField
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
// header for class ListBox
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

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
class ChartTypeResourceGroup;
class AxisTypeResourceGroup;
class Dim3DLookResourceGroup;
class StackingResourceGroup;
class SplineResourceGroup;
class GeometryResourceGroup;
class ChartTypeParameter;
class SortByXValuesResourceGroup;

class ChartTypeTabPage : public ResourceChangeListener, public svt::OWizardPage, public ChartTypeTemplateProvider
{
public:
    ChartTypeTabPage( Window* pParent
                , const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartDocument >& xChartModel
                , const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext >& xContext
                , bool bDoLiveUpdate, bool bHideDescription = false );
    virtual ~ChartTypeTabPage();

    virtual void        initializePage();
    virtual sal_Bool    commitPage( CommitPageReason eReason );

    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartTypeTemplate > getCurrentTemplate() const;

protected:
    ChartTypeDialogController* getSelectedMainType();
    void showAllControls( ChartTypeDialogController& rTypeController );
    void fillAllControls( const ChartTypeParameter& rParameter, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xTemplateProps=::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >() );
    ChartTypeParameter getCurrentParamter() const;

    virtual void stateChanged( ChangingResource* pResource );

    void commitToModel( const ChartTypeParameter& rParameter );

    DECL_LINK( SelectMainTypeHdl, void* );
    DECL_LINK( SelectSubTypeHdl, void* );

protected:
    FixedText   m_aFT_ChooseType;
    ListBox     m_aMainTypeList;
    ValueSet    m_aSubTypeList;

    AxisTypeResourceGroup*      m_pAxisTypeResourceGroup;
    Dim3DLookResourceGroup*     m_pDim3DLookResourceGroup;
    StackingResourceGroup*      m_pStackingResourceGroup;
    SplineResourceGroup*        m_pSplineResourceGroup;
    GeometryResourceGroup*      m_pGeometryResourceGroup;
    SortByXValuesResourceGroup* m_pSortByXValuesResourceGroup;

    long    m_nYTopPos;//top y position of the different ResourceGroups (AxisTypeResourceGroup, etc)

    ::com::sun::star::uno::Reference<
                       ::com::sun::star::chart2::XChartDocument >   m_xChartModel;
    ::com::sun::star::uno::Reference<
                       ::com::sun::star::uno::XComponentContext >    m_xCC;

    ::std::vector< ChartTypeDialogController* > m_aChartTypeDialogControllerList;
    ChartTypeDialogController*                  m_pCurrentMainType;

    sal_Int32 m_nChangingCalls;
    bool      m_bDoLiveUpdate;

    TimerTriggeredControllerLock   m_aTimerTriggeredControllerLock;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
