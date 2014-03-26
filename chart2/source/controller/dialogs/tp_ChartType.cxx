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

#include "tp_ChartType.hxx"
#include "Strings.hrc"
#include "ResId.hxx"
#include "ChartModelHelper.hxx"
#include "DiagramHelper.hxx"
#include "res_BarGeometry.hxx"
#include "ControllerLockGuard.hxx"
#include "macros.hxx"
#include "GL3DBarChartDialogController.hxx"

#include <svtools/controldims.hrc>

#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

namespace
{
    sal_Bool lcl_getSortByXValues( const uno::Reference< chart2::XChartDocument >& xChartDoc )
    {
        sal_Bool bRet = sal_False;
        if( xChartDoc.is() )
        {
            try
            {
                uno::Reference< beans::XPropertySet > xDiaProp( xChartDoc->getFirstDiagram(), uno::UNO_QUERY_THROW );
                xDiaProp->getPropertyValue( "SortByXValues" ) >>= bRet;
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
        return bRet;
    }
}

#define POS_3DSCHEME_SIMPLE    0
#define POS_3DSCHEME_REALISTIC 1

class Dim3DLookResourceGroup : public ChangingResource
{
public:
    Dim3DLookResourceGroup(VclBuilderContainer* pWindow);

    void showControls( bool bShow );

    void fillControls( const ChartTypeParameter& rParameter );
    void fillParameter( ChartTypeParameter& rParameter );

private:
    DECL_LINK( Dim3DLookCheckHdl, void* );
    DECL_LINK( SelectSchemeHdl, void* );

private:
    CheckBox* m_pCB_3DLook;
    ListBox*  m_pLB_Scheme;
};

Dim3DLookResourceGroup::Dim3DLookResourceGroup(VclBuilderContainer* pWindow)
    : ChangingResource()
{
    pWindow->get(m_pCB_3DLook, "3dlook");
    pWindow->get(m_pLB_Scheme, "3dscheme");

    m_pCB_3DLook->SetToggleHdl( LINK( this, Dim3DLookResourceGroup, Dim3DLookCheckHdl ) );
    m_pLB_Scheme->SetSelectHdl( LINK( this, Dim3DLookResourceGroup, SelectSchemeHdl ) );
}

void Dim3DLookResourceGroup::showControls( bool bShow )
{
    m_pCB_3DLook->Show(bShow);
    m_pLB_Scheme->Show(bShow);
}

void Dim3DLookResourceGroup::fillControls( const ChartTypeParameter& rParameter )
{
    m_pCB_3DLook->Check(rParameter.b3DLook);
    m_pLB_Scheme->Enable(rParameter.b3DLook);

    if( rParameter.eThreeDLookScheme == ThreeDLookScheme_Simple )
        m_pLB_Scheme->SelectEntryPos(POS_3DSCHEME_SIMPLE);
    else if( rParameter.eThreeDLookScheme == ThreeDLookScheme_Realistic )
        m_pLB_Scheme->SelectEntryPos(POS_3DSCHEME_REALISTIC);
    else
        m_pLB_Scheme->SetNoSelection();
}

void Dim3DLookResourceGroup::fillParameter( ChartTypeParameter& rParameter )
{
    rParameter.b3DLook = m_pCB_3DLook->IsChecked();
    sal_uInt16 nPos = m_pLB_Scheme->GetSelectEntryPos();
    if( POS_3DSCHEME_SIMPLE == nPos )
        rParameter.eThreeDLookScheme = ThreeDLookScheme_Simple;
    else if( POS_3DSCHEME_REALISTIC == nPos )
        rParameter.eThreeDLookScheme = ThreeDLookScheme_Realistic;
    else
        rParameter.eThreeDLookScheme = ThreeDLookScheme_Unknown;
}

IMPL_LINK_NOARG(Dim3DLookResourceGroup, Dim3DLookCheckHdl)
{
    if(m_pChangeListener)
        m_pChangeListener->stateChanged(this);
    return 0;
}

IMPL_LINK_NOARG(Dim3DLookResourceGroup, SelectSchemeHdl)
{
    if(m_pChangeListener)
        m_pChangeListener->stateChanged(this);
    return 0;
}

class SortByXValuesResourceGroup : public ChangingResource
{
public:
    SortByXValuesResourceGroup(VclBuilderContainer* pWindow);

    void showControls( bool bShow );

    void fillControls( const ChartTypeParameter& rParameter );
    void fillParameter( ChartTypeParameter& rParameter );

private:
    DECL_LINK( SortByXValuesCheckHdl, void* );

private:
    CheckBox* m_pCB_XValueSorting;
};

SortByXValuesResourceGroup::SortByXValuesResourceGroup(VclBuilderContainer* pWindow )
    : ChangingResource()
{
    pWindow->get(m_pCB_XValueSorting, "sort");
    m_pCB_XValueSorting->SetToggleHdl( LINK( this, SortByXValuesResourceGroup, SortByXValuesCheckHdl ) );
}

void SortByXValuesResourceGroup::showControls( bool bShow )
{
    m_pCB_XValueSorting->Show(bShow);
}

void SortByXValuesResourceGroup::fillControls( const ChartTypeParameter& rParameter )
{
    m_pCB_XValueSorting->Check( rParameter.bSortByXValues );
}

void SortByXValuesResourceGroup::fillParameter( ChartTypeParameter& rParameter )
{
    rParameter.bSortByXValues = m_pCB_XValueSorting->IsChecked();
}

IMPL_LINK_NOARG(SortByXValuesResourceGroup, SortByXValuesCheckHdl)
{
    if(m_pChangeListener)
        m_pChangeListener->stateChanged(this);
    return 0;
}

class StackingResourceGroup : public ChangingResource
{
public:
    StackingResourceGroup(VclBuilderContainer* pWindow);

    void showControls( bool bShow, bool bShowDeepStacking );

    void fillControls( const ChartTypeParameter& rParameter );
    void fillParameter( ChartTypeParameter& rParameter );

private:
    DECL_LINK( StackingChangeHdl, RadioButton* );
    DECL_LINK( StackingEnableHdl, void* );

private:
    CheckBox*    m_pCB_Stacked;
    RadioButton* m_pRB_Stack_Y;
    RadioButton* m_pRB_Stack_Y_Percent;
    RadioButton* m_pRB_Stack_Z;

    bool m_bShowDeepStacking;
};

StackingResourceGroup::StackingResourceGroup(VclBuilderContainer* pWindow)
        : ChangingResource()
        , m_bShowDeepStacking(true)
{
    pWindow->get(m_pCB_Stacked, "stack");
    pWindow->get(m_pRB_Stack_Y, "ontop");
    pWindow->get(m_pRB_Stack_Y_Percent, "percent");
    pWindow->get(m_pRB_Stack_Z, "deep");

    m_pCB_Stacked->SetToggleHdl( LINK( this, StackingResourceGroup, StackingEnableHdl ) );
    m_pRB_Stack_Y->SetToggleHdl( LINK( this, StackingResourceGroup, StackingChangeHdl ) );
    m_pRB_Stack_Y_Percent->SetToggleHdl( LINK( this, StackingResourceGroup, StackingChangeHdl ) );
    m_pRB_Stack_Z->SetToggleHdl( LINK( this, StackingResourceGroup, StackingChangeHdl ) );
}

void StackingResourceGroup::showControls( bool bShow, bool bShowDeepStacking )
{
    m_bShowDeepStacking = bShowDeepStacking;
    m_pCB_Stacked->Show(bShow);
    m_pRB_Stack_Y->Show(bShow);
    m_pRB_Stack_Y_Percent->Show(bShow);
    m_pRB_Stack_Z->Show(bShow&&bShowDeepStacking);
}

void StackingResourceGroup::fillControls( const ChartTypeParameter& rParameter )
{
    m_pCB_Stacked->Check( rParameter.eStackMode!=GlobalStackMode_NONE
        && rParameter.eStackMode!=GlobalStackMode_STACK_Z ); //todo remove this condition if z stacking radio button is really used
    switch( rParameter.eStackMode )
    {
        case GlobalStackMode_STACK_Y:
            m_pRB_Stack_Y->Check();
            break;
        case GlobalStackMode_STACK_Y_PERCENT:
            m_pRB_Stack_Y_Percent->Check();
            break;
        case GlobalStackMode_STACK_Z:
            //todo uncomment this condition if z stacking radio button is really used
            /*
            if( rParameter.b3DLook )
                m_pRB_Stack_Z->Check();
            else
            */
                m_pRB_Stack_Y->Check();
            break;
        default:
            m_pRB_Stack_Y->Check();
            break;
    }
    //dis/enabling
    m_pCB_Stacked->Enable( !rParameter.bXAxisWithValues );
    m_pRB_Stack_Y->Enable( m_pCB_Stacked->IsChecked() && !rParameter.bXAxisWithValues );
    m_pRB_Stack_Y_Percent->Enable( m_pCB_Stacked->IsChecked() && !rParameter.bXAxisWithValues );
    m_pRB_Stack_Z->Enable( m_pCB_Stacked->IsChecked() && rParameter.b3DLook );
}
void StackingResourceGroup::fillParameter( ChartTypeParameter& rParameter )
{
    if(!m_pCB_Stacked->IsChecked())
        rParameter.eStackMode = GlobalStackMode_NONE;
    else if(m_pRB_Stack_Y->IsChecked())
        rParameter.eStackMode = GlobalStackMode_STACK_Y;
    else if(m_pRB_Stack_Y_Percent->IsChecked())
        rParameter.eStackMode = GlobalStackMode_STACK_Y_PERCENT;
    else if(m_pRB_Stack_Z->IsChecked())
        rParameter.eStackMode = GlobalStackMode_STACK_Z;
}
IMPL_LINK( StackingResourceGroup, StackingChangeHdl, RadioButton*, pRadio )
{
    //for each radio click ther are coming two change events
    //first uncheck of previous button -> ignore that call
    //the second call gives the check of the new button
    if( m_pChangeListener && pRadio && pRadio->IsChecked() )
        m_pChangeListener->stateChanged(this);
    return 0;
}
IMPL_LINK_NOARG(StackingResourceGroup, StackingEnableHdl)
{
    if( m_pChangeListener )
        m_pChangeListener->stateChanged(this);
    return 0;
}
class SplinePropertiesDialog : public ModalDialog
{
public:
    SplinePropertiesDialog( Window* pParent );
    virtual ~SplinePropertiesDialog();

    void fillControls( const ChartTypeParameter& rParameter );
    void fillParameter( ChartTypeParameter& rParameter, bool bSmoothLines );

    virtual void StateChanged( StateChangedType nType );

private:
    DECL_LINK( SplineTypeListBoxHdl, void* );

private:
    ListBox* m_pLB_Spline_Type;

    NumericField* m_pMF_SplineResolution;
    FixedText*    m_pFT_SplineOrder;
    NumericField* m_pMF_SplineOrder;
};

const sal_uInt16 CUBIC_SPLINE_POS = 0;
const sal_uInt16 B_SPLINE_POS = 1;

SplinePropertiesDialog::SplinePropertiesDialog( Window* pParent )
        : ModalDialog( pParent, "SmoothLinesDialog", "modules/schart/ui/smoothlinesdlg.ui")
{
    get(m_pLB_Spline_Type, "SplineTypeComboBox");
    get(m_pMF_SplineResolution, "ResolutionSpinbutton");
    get(m_pFT_SplineOrder, "PolynomialsLabel");
    get(m_pMF_SplineOrder, "PolynomialsSpinButton");

    this->SetText( SCH_RESSTR( STR_DLG_SMOOTH_LINE_PROPERTIES ) );

    m_pLB_Spline_Type->SetSelectHdl( LINK (this, SplinePropertiesDialog, SplineTypeListBoxHdl ) );
}

SplinePropertiesDialog::~SplinePropertiesDialog()
{
}

void SplinePropertiesDialog::StateChanged( StateChangedType nType )
{
    Dialog::StateChanged( nType );
}

void SplinePropertiesDialog::fillControls( const ChartTypeParameter& rParameter )
{
    switch(rParameter.eCurveStyle)
    {
    case CurveStyle_CUBIC_SPLINES:
        m_pLB_Spline_Type->SelectEntryPos(CUBIC_SPLINE_POS);
        break;
    case CurveStyle_B_SPLINES:
        m_pLB_Spline_Type->SelectEntryPos(B_SPLINE_POS);
        break;
    default:
        m_pLB_Spline_Type->SelectEntryPos(CUBIC_SPLINE_POS);
        break;
    }
    m_pMF_SplineOrder->SetValue( rParameter.nSplineOrder );
    m_pMF_SplineResolution->SetValue( rParameter.nCurveResolution );

    //dis/enabling
    m_pFT_SplineOrder->Enable(B_SPLINE_POS == m_pLB_Spline_Type->GetSelectEntryPos());
    m_pMF_SplineOrder->Enable(B_SPLINE_POS == m_pLB_Spline_Type->GetSelectEntryPos());
}
void SplinePropertiesDialog::fillParameter( ChartTypeParameter& rParameter, bool bSmoothLines )
{
    if(!bSmoothLines)
        rParameter.eCurveStyle=CurveStyle_LINES;
    else if(CUBIC_SPLINE_POS == m_pLB_Spline_Type->GetSelectEntryPos())
        rParameter.eCurveStyle=CurveStyle_CUBIC_SPLINES;
    else if(B_SPLINE_POS == m_pLB_Spline_Type->GetSelectEntryPos())
        rParameter.eCurveStyle=CurveStyle_B_SPLINES;

    rParameter.nCurveResolution = static_cast< sal_Int32 >( m_pMF_SplineResolution->GetValue());
    rParameter.nSplineOrder = static_cast< sal_Int32 >( m_pMF_SplineOrder->GetValue());
}
IMPL_LINK_NOARG(SplinePropertiesDialog, SplineTypeListBoxHdl)
{
    m_pFT_SplineOrder->Enable(B_SPLINE_POS == m_pLB_Spline_Type->GetSelectEntryPos());
    m_pMF_SplineOrder->Enable(B_SPLINE_POS == m_pLB_Spline_Type->GetSelectEntryPos());
    return 0;
}

class SteppedPropertiesDialog : public ModalDialog
{
public:
    SteppedPropertiesDialog( Window* pParent );
    virtual ~SteppedPropertiesDialog();

    void fillControls( const ChartTypeParameter& rParameter );
    void fillParameter( ChartTypeParameter& rParameter, bool bSteppedLines );

    virtual void StateChanged( StateChangedType nType );

private:

private:
    RadioButton* m_pRB_Start;
    RadioButton* m_pRB_End;
    RadioButton* m_pRB_CenterX;
    RadioButton* m_pRB_CenterY;
};

SteppedPropertiesDialog::SteppedPropertiesDialog( Window* pParent )
    : ModalDialog( pParent, "SteppedLinesDialog", "modules/schart/ui/steppedlinesdlg.ui")
{
    get(m_pRB_Start, "step_start_rb");
    get(m_pRB_End, "step_end_rb");
    get(m_pRB_CenterX, "step_center_x_rb");
    get(m_pRB_CenterY, "step_center_y_rb");

    SetText(SCH_RESSTR(STR_DLG_STEPPED_LINE_PROPERTIES));
}

SteppedPropertiesDialog::~SteppedPropertiesDialog()
{
}

void SteppedPropertiesDialog::StateChanged( StateChangedType nType )
{
    Dialog::StateChanged( nType );
}

void SteppedPropertiesDialog::fillControls( const ChartTypeParameter& rParameter )
{
    switch(rParameter.eCurveStyle)
    {
        case CurveStyle_STEP_END:
            m_pRB_End->Check();
            break;
        case CurveStyle_STEP_CENTER_X:
            m_pRB_CenterX->Check();
            break;
        case CurveStyle_STEP_CENTER_Y:
            m_pRB_CenterY->Check();
            break;
        default: // includes CurveStyle_STEP_START
            m_pRB_Start->Check();
            break;
    }
}
void SteppedPropertiesDialog::fillParameter( ChartTypeParameter& rParameter, bool bSteppedLines )
{
    if (!bSteppedLines)
        rParameter.eCurveStyle=CurveStyle_LINES;
    else if(m_pRB_CenterY->IsChecked())
        rParameter.eCurveStyle=CurveStyle_STEP_CENTER_Y;
    else if(m_pRB_Start->IsChecked())
        rParameter.eCurveStyle=CurveStyle_STEP_START;
    else if(m_pRB_End->IsChecked())
        rParameter.eCurveStyle=CurveStyle_STEP_END;
    else if(m_pRB_CenterX->IsChecked())
        rParameter.eCurveStyle=CurveStyle_STEP_CENTER_X;
}

#define POS_LINETYPE_STRAIGHT    0
#define POS_LINETYPE_SMOOTH      1
#define POS_LINETYPE_STEPPED     2

class SplineResourceGroup : public ChangingResource
{
public:
    SplineResourceGroup(VclBuilderContainer* pWindow);

    void showControls( bool bShow );

    void fillControls( const ChartTypeParameter& rParameter );
    void fillParameter( ChartTypeParameter& rParameter );

private:
    DECL_LINK( LineTypeChangeHdl, void* );
    DECL_LINK( SplineDetailsDialogHdl, void* );
    DECL_LINK( SteppedDetailsDialogHdl, void* );
    SplinePropertiesDialog& getSplinePropertiesDialog();
    SteppedPropertiesDialog& getSteppedPropertiesDialog();

private:
    FixedText*  m_pFT_LineType;
    ListBox*    m_pLB_LineType;
    PushButton* m_pPB_DetailsDialog;
    boost::scoped_ptr< SplinePropertiesDialog > m_pSplinePropertiesDialog;
    boost::scoped_ptr< SteppedPropertiesDialog > m_pSteppedPropertiesDialog;
};

SplineResourceGroup::SplineResourceGroup(VclBuilderContainer* pWindow)
    : ChangingResource()
{
    pWindow->get(m_pFT_LineType, "linetypeft");
    pWindow->get(m_pLB_LineType, "linetype");
    pWindow->get(m_pPB_DetailsDialog, "properties");

    m_pLB_LineType->SetSelectHdl( LINK( this, SplineResourceGroup, LineTypeChangeHdl ) );
}

SplinePropertiesDialog& SplineResourceGroup::getSplinePropertiesDialog()
{
    if( !m_pSplinePropertiesDialog.get() )
        m_pSplinePropertiesDialog.reset( new SplinePropertiesDialog( m_pPB_DetailsDialog->GetParentDialog() ) );
    return *m_pSplinePropertiesDialog;
}

SteppedPropertiesDialog& SplineResourceGroup::getSteppedPropertiesDialog()
{
    if( !m_pSteppedPropertiesDialog.get() )
    {
        m_pSteppedPropertiesDialog.reset( new SteppedPropertiesDialog( m_pPB_DetailsDialog->GetParentDialog() ) );
    }
    return *m_pSteppedPropertiesDialog;
}

void SplineResourceGroup::showControls( bool bShow )
{
    m_pFT_LineType->Show(bShow);
    m_pLB_LineType->Show(bShow);
    m_pPB_DetailsDialog->Show(bShow);
}

void SplineResourceGroup::fillControls( const ChartTypeParameter& rParameter )
{
    switch (rParameter.eCurveStyle)
    {
        case CurveStyle_LINES:
            m_pLB_LineType->SelectEntryPos(POS_LINETYPE_STRAIGHT);
            m_pPB_DetailsDialog->Enable(false);
            break;
        case CurveStyle_CUBIC_SPLINES:
        case CurveStyle_B_SPLINES:
            m_pLB_LineType->SelectEntryPos(POS_LINETYPE_SMOOTH);
            m_pPB_DetailsDialog->Enable(true);
            m_pPB_DetailsDialog->SetClickHdl( LINK( this, SplineResourceGroup, SplineDetailsDialogHdl ) );
            m_pPB_DetailsDialog->SetQuickHelpText( SCH_RESSTR(STR_DLG_SMOOTH_LINE_PROPERTIES) );
            getSplinePropertiesDialog().fillControls( rParameter );
            break;
        case CurveStyle_STEP_START:
        case CurveStyle_STEP_END:
        case CurveStyle_STEP_CENTER_X:
        case CurveStyle_STEP_CENTER_Y:
            m_pLB_LineType->SelectEntryPos(POS_LINETYPE_STEPPED);
            m_pPB_DetailsDialog->Enable(true);
            m_pPB_DetailsDialog->SetClickHdl( LINK( this, SplineResourceGroup, SteppedDetailsDialogHdl ) );
            m_pPB_DetailsDialog->SetQuickHelpText( SCH_RESSTR(STR_DLG_STEPPED_LINE_PROPERTIES) );
            getSteppedPropertiesDialog().fillControls( rParameter );
            break;
        default:
            m_pLB_LineType->SetNoSelection();
            m_pPB_DetailsDialog->Enable(false);
    }
}
void SplineResourceGroup::fillParameter( ChartTypeParameter& rParameter )
{
    switch (m_pLB_LineType->GetSelectEntryPos())
    {
        case POS_LINETYPE_SMOOTH:
            getSplinePropertiesDialog().fillParameter( rParameter, true );
            break;
        case POS_LINETYPE_STEPPED:
            getSteppedPropertiesDialog().fillParameter( rParameter, true );
            break;
        default: // includes POS_LINETYPE_STRAIGHT
            rParameter.eCurveStyle = CurveStyle_LINES;
            break;
    }
}
IMPL_LINK_NOARG(SplineResourceGroup, LineTypeChangeHdl)
{
    if( m_pChangeListener )
        m_pChangeListener->stateChanged(this);
    return 0;
}
IMPL_LINK_NOARG(SplineResourceGroup, SplineDetailsDialogHdl)
{

    ChartTypeParameter aOldParameter;
    getSplinePropertiesDialog().fillParameter( aOldParameter, POS_LINETYPE_SMOOTH == m_pLB_LineType->GetSelectEntryPos() );

    sal_uInt16 iOldLineTypePos = m_pLB_LineType->GetSelectEntryPos();
    m_pLB_LineType->SelectEntryPos(POS_LINETYPE_SMOOTH);
    if( RET_OK == getSplinePropertiesDialog().Execute() )
    {
        if( m_pChangeListener )
            m_pChangeListener->stateChanged(this);
    }
    else
    {
        //restore old state:
        m_pLB_LineType->SelectEntryPos( iOldLineTypePos );
        getSplinePropertiesDialog().fillControls( aOldParameter );
    }
    return 0;
}
IMPL_LINK_NOARG(SplineResourceGroup, SteppedDetailsDialogHdl)
{

    ChartTypeParameter aOldParameter;
    getSteppedPropertiesDialog().fillParameter( aOldParameter, POS_LINETYPE_STEPPED == m_pLB_LineType->GetSelectEntryPos() );

    sal_uInt16 iOldLineTypePos = m_pLB_LineType->GetSelectEntryPos();
    m_pLB_LineType->SelectEntryPos(POS_LINETYPE_STEPPED);
    if( RET_OK == getSteppedPropertiesDialog().Execute() )
    {
        if( m_pChangeListener )
            m_pChangeListener->stateChanged(this);
    }
    else
    {
        //restore old state:
        m_pLB_LineType->SelectEntryPos( iOldLineTypePos );
        getSteppedPropertiesDialog().fillControls( aOldParameter );
    }
    return 0;
}

class GeometryResourceGroup : public ChangingResource
{
public:
    GeometryResourceGroup(VclBuilderContainer* pWindow);

    void showControls( bool bShow );

    void fillControls( const ChartTypeParameter& rParameter );
    void fillParameter( ChartTypeParameter& rParameter );

private:
    DECL_LINK( GeometryChangeHdl, void* );

private:
    BarGeometryResources m_aGeometryResources;
};

GeometryResourceGroup::GeometryResourceGroup(VclBuilderContainer* pWindow )
    : ChangingResource()
    , m_aGeometryResources( pWindow )
{
    m_aGeometryResources.SetSelectHdl( LINK( this, GeometryResourceGroup, GeometryChangeHdl ) );
}

void GeometryResourceGroup::showControls( bool bShow )
{
    m_aGeometryResources.Show(bShow);
}

void GeometryResourceGroup::fillControls( const ChartTypeParameter& rParameter )
{
    sal_uInt16 nGeometry3D = static_cast<sal_uInt16>(rParameter.nGeometry3D);
    m_aGeometryResources.SelectEntryPos(nGeometry3D);
    m_aGeometryResources.Enable(rParameter.b3DLook);
}

void GeometryResourceGroup::fillParameter( ChartTypeParameter& rParameter )
{
    rParameter.nGeometry3D = 1;
    if( m_aGeometryResources.GetSelectEntryCount() )
        rParameter.nGeometry3D = m_aGeometryResources.GetSelectEntryPos();
}

IMPL_LINK_NOARG(GeometryResourceGroup, GeometryChangeHdl)
{
    if( m_pChangeListener )
        m_pChangeListener->stateChanged(this);
    return 0;
}

ChartTypeTabPage::ChartTypeTabPage(Window* pParent
        , const uno::Reference< XChartDocument >& xChartModel
        , const uno::Reference< uno::XComponentContext >& xContext
        , bool bDoLiveUpdate, bool bHideDescription)
        : OWizardPage(pParent, "tp_ChartType",
            "modules/schart/ui/tp_ChartType.ui")
        , m_pDim3DLookResourceGroup( new Dim3DLookResourceGroup(this) )
        , m_pStackingResourceGroup( new StackingResourceGroup(this) )
        , m_pSplineResourceGroup( new SplineResourceGroup(this) )
        , m_pGeometryResourceGroup( new GeometryResourceGroup( this ) )
        , m_pSortByXValuesResourceGroup( new SortByXValuesResourceGroup( this ) )
        , m_xChartModel( xChartModel )
        , m_xCC( xContext )
        , m_aChartTypeDialogControllerList(0)
        , m_pCurrentMainType(0)
        , m_nChangingCalls(0)
        , m_bDoLiveUpdate(bDoLiveUpdate)
        , m_aTimerTriggeredControllerLock( uno::Reference< frame::XModel >( m_xChartModel, uno::UNO_QUERY ) )
{
    get(m_pFT_ChooseType, "FT_CAPTION_FOR_WIZARD");
    get(m_pMainTypeList, "charttype");
    get(m_pSubTypeList, "subtype");
    Size aSize(m_pSubTypeList->LogicToPixel(Size(150, 50), MAP_APPFONT));
    m_pSubTypeList->set_width_request(aSize.Width());
    m_pSubTypeList->set_height_request(aSize.Height());

    if( bHideDescription )
    {
        m_pFT_ChooseType->Hide();
    }
    else
    {
        m_pFT_ChooseType->SetStyle(m_pFT_ChooseType->GetStyle() | WB_NOLABEL);
    }

    this->SetText( SCH_RESSTR(STR_PAGE_CHARTTYPE) );

    m_pMainTypeList->SetStyle(m_pMainTypeList->GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER | WB_FLATVALUESET | WB_3DLOOK );
    m_pMainTypeList->SetSelectHdl( LINK( this, ChartTypeTabPage, SelectMainTypeHdl ) );
    m_pSubTypeList->SetSelectHdl( LINK( this, ChartTypeTabPage, SelectSubTypeHdl ) );

    m_pSubTypeList->SetStyle(m_pSubTypeList->GetStyle() |
        WB_ITEMBORDER | WB_DOUBLEBORDER | WB_NAMEFIELD | WB_FLATVALUESET | WB_3DLOOK );
    m_pSubTypeList->SetColCount(4);
    m_pSubTypeList->SetLineCount(1);

    bool bEnableComplexChartTypes = true;
    uno::Reference< beans::XPropertySet > xProps( m_xChartModel, uno::UNO_QUERY );
    if ( xProps.is() )
    {
        try
        {
            xProps->getPropertyValue("EnableComplexChartTypes") >>= bEnableComplexChartTypes;
        }
        catch( const uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }

    m_aChartTypeDialogControllerList.push_back(new ColumnChartDialogController() );
    m_aChartTypeDialogControllerList.push_back(new BarChartDialogController() );
    m_aChartTypeDialogControllerList.push_back(new PieChartDialogController() );
    m_aChartTypeDialogControllerList.push_back(new AreaChartDialogController() );
    m_aChartTypeDialogControllerList.push_back(new LineChartDialogController() );
    if (bEnableComplexChartTypes)
    {
        m_aChartTypeDialogControllerList.push_back(new XYChartDialogController() );
        m_aChartTypeDialogControllerList.push_back(new BubbleChartDialogController() );
    }
    m_aChartTypeDialogControllerList.push_back(new NetChartDialogController() );
    if (bEnableComplexChartTypes)
    {
        m_aChartTypeDialogControllerList.push_back(new StockChartDialogController() );
    }
    m_aChartTypeDialogControllerList.push_back(new CombiColumnLineChartDialogController() );
#if ENABLE_GL3D_BARCHART
    m_aChartTypeDialogControllerList.push_back(new GL3DBarChartDialogController());
#endif

    ::std::vector< ChartTypeDialogController* >::const_iterator       aIter = m_aChartTypeDialogControllerList.begin();
    const ::std::vector< ChartTypeDialogController* >::const_iterator aEnd  = m_aChartTypeDialogControllerList.end();
    for( ; aIter != aEnd; ++aIter )
    {
        m_pMainTypeList->InsertEntry( (*aIter)->getName(), (*aIter)->getImage() );
        (*aIter)->setChangeListener( this );
    }

    m_pDim3DLookResourceGroup->setChangeListener( this );
    m_pStackingResourceGroup->setChangeListener( this );
    m_pSplineResourceGroup->setChangeListener( this );
    m_pGeometryResourceGroup->setChangeListener( this );
    m_pSortByXValuesResourceGroup->setChangeListener( this );
}

ChartTypeTabPage::~ChartTypeTabPage()
{
    //delete all dialog controller
    ::std::vector< ChartTypeDialogController* >::const_iterator       aIter = m_aChartTypeDialogControllerList.begin();
    const ::std::vector< ChartTypeDialogController* >::const_iterator aEnd  = m_aChartTypeDialogControllerList.end();
    for( ; aIter != aEnd; ++aIter )
    {
        delete *aIter;
    }
    m_aChartTypeDialogControllerList.clear();

    //delete all resource helper
    delete m_pDim3DLookResourceGroup;
    delete m_pStackingResourceGroup;
    delete m_pSplineResourceGroup;
    delete m_pGeometryResourceGroup;
    delete m_pSortByXValuesResourceGroup;
}
ChartTypeParameter ChartTypeTabPage::getCurrentParamter() const
{
    ChartTypeParameter aParameter;
    aParameter.nSubTypeIndex = static_cast<sal_Int32>( m_pSubTypeList->GetSelectItemId() );
    m_pDim3DLookResourceGroup->fillParameter( aParameter );
    m_pStackingResourceGroup->fillParameter( aParameter );
    m_pSplineResourceGroup->fillParameter( aParameter );
    m_pGeometryResourceGroup->fillParameter( aParameter );
    m_pSortByXValuesResourceGroup->fillParameter( aParameter );
    return aParameter;
}
void ChartTypeTabPage::commitToModel( const ChartTypeParameter& rParameter )
{
    if( !m_pCurrentMainType )
        return;

    m_aTimerTriggeredControllerLock.startTimer();
    ControllerLockGuardUNO aLockedControllers( uno::Reference< frame::XModel >( m_xChartModel, uno::UNO_QUERY ) );
    m_pCurrentMainType->commitToModel( rParameter, m_xChartModel );
}
void ChartTypeTabPage::stateChanged( ChangingResource* /*pResource*/ )
{
    if(m_nChangingCalls)
        return;
    m_nChangingCalls++;

    ChartTypeParameter aParameter( this->getCurrentParamter() );
    if( m_pCurrentMainType )
    {
        m_pCurrentMainType->adjustParameterToSubType( aParameter );
        m_pCurrentMainType->adjustSubTypeAndEnableControls( aParameter );
    }
    if( m_bDoLiveUpdate )
        commitToModel( aParameter );

    //detect the new ThreeDLookScheme
    aParameter.eThreeDLookScheme = ThreeDHelper::detectScheme( ChartModelHelper::findDiagram( m_xChartModel ) );
    aParameter.bSortByXValues = lcl_getSortByXValues( m_xChartModel );
    //the controls have to be enabled/disabled accordingly
    this->fillAllControls( aParameter );

    m_nChangingCalls--;
}
ChartTypeDialogController* ChartTypeTabPage::getSelectedMainType()
{
    ChartTypeDialogController* pTypeController = 0;
    ::std::vector< ChartTypeDialogController* >::size_type nM = static_cast< ::std::vector< ChartTypeDialogController* >::size_type >(
        m_pMainTypeList->GetSelectEntryPos() );
    if( nM<m_aChartTypeDialogControllerList.size() )
        pTypeController = m_aChartTypeDialogControllerList[nM];
    return pTypeController;
}
IMPL_LINK_NOARG(ChartTypeTabPage, SelectSubTypeHdl)
{
    if( m_pCurrentMainType )
    {
        ChartTypeParameter aParameter( this->getCurrentParamter() );
        m_pCurrentMainType->adjustParameterToSubType( aParameter );
        this->fillAllControls( aParameter, false );
        if( m_bDoLiveUpdate )
            commitToModel( aParameter );
    }
    return 0;
}

IMPL_LINK_NOARG(ChartTypeTabPage, SelectMainTypeHdl)
{
    selectMainType();
    return 0;
}

void ChartTypeTabPage::selectMainType()
{
    ChartTypeParameter aParameter( this->getCurrentParamter() );

    if( m_pCurrentMainType )
    {
        m_pCurrentMainType->adjustParameterToSubType( aParameter );
        m_pCurrentMainType->hideExtraControls();
    }

    m_pCurrentMainType = this->getSelectedMainType();
    if( m_pCurrentMainType )
    {
        this->showAllControls(*m_pCurrentMainType);

        m_pCurrentMainType->adjustParameterToMainType( aParameter );
        if( m_bDoLiveUpdate )
            commitToModel( aParameter );
        //detect the new ThreeDLookScheme
        aParameter.eThreeDLookScheme = ThreeDHelper::detectScheme( ChartModelHelper::findDiagram( m_xChartModel ) );
        if(!aParameter.b3DLook && aParameter.eThreeDLookScheme!=ThreeDLookScheme_Realistic )
            aParameter.eThreeDLookScheme=ThreeDLookScheme_Realistic;

        aParameter.bSortByXValues = lcl_getSortByXValues( m_xChartModel );
        this->fillAllControls( aParameter );
        uno::Reference< beans::XPropertySet > xTemplateProps( this->getCurrentTemplate(), uno::UNO_QUERY );
        m_pCurrentMainType->fillExtraControls(aParameter,m_xChartModel,xTemplateProps);
    }
}

void ChartTypeTabPage::showAllControls( ChartTypeDialogController& rTypeController )
{
    m_pSubTypeList->Show();

    bool bShow = rTypeController.shouldShow_3DLookControl();
    m_pDim3DLookResourceGroup->showControls( bShow );
    bShow = rTypeController.shouldShow_StackingControl();
    m_pStackingResourceGroup->showControls( bShow, rTypeController.shouldShow_DeepStackingControl() );
    bShow = rTypeController.shouldShow_SplineControl();
    m_pSplineResourceGroup->showControls( bShow );
    bShow = rTypeController.shouldShow_GeometryControl();
    m_pGeometryResourceGroup->showControls( bShow );
    bShow = rTypeController.shouldShow_SortByXValuesResourceGroup();
    m_pSortByXValuesResourceGroup->showControls( bShow );
    rTypeController.showExtraControls(this);
}

void ChartTypeTabPage::fillAllControls( const ChartTypeParameter& rParameter, bool bAlsoResetSubTypeList )
{
    m_nChangingCalls++;
    if( m_pCurrentMainType && bAlsoResetSubTypeList )
    {
        m_pCurrentMainType->fillSubTypeList(*m_pSubTypeList, rParameter);
    }
    m_pSubTypeList->SelectItem( static_cast<sal_uInt16>( rParameter.nSubTypeIndex) );
    m_pDim3DLookResourceGroup->fillControls( rParameter );
    m_pStackingResourceGroup->fillControls( rParameter );
    m_pSplineResourceGroup->fillControls( rParameter );
    m_pGeometryResourceGroup->fillControls( rParameter );
    m_pSortByXValuesResourceGroup->fillControls( rParameter );
    m_nChangingCalls--;
}

void ChartTypeTabPage::initializePage()
{
    if( !m_xChartModel.is() )
        return;
    uno::Reference< lang::XMultiServiceFactory > xTemplateManager( m_xChartModel->getChartTypeManager(), uno::UNO_QUERY );
    uno::Reference< frame::XModel > xModel( m_xChartModel, uno::UNO_QUERY);
    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xModel ) );
    DiagramHelper::tTemplateWithServiceName aTemplate =
        DiagramHelper::getTemplateForDiagram( xDiagram, xTemplateManager );
    OUString aServiceName( aTemplate.second );

    bool bFound = false;

    ::std::vector< ChartTypeDialogController* >::iterator             aIter = m_aChartTypeDialogControllerList.begin();
    const ::std::vector< ChartTypeDialogController* >::const_iterator aEnd  = m_aChartTypeDialogControllerList.end();
    for( sal_uInt16 nM=0; aIter != aEnd; ++aIter, ++nM )
    {
        if( (*aIter)->isSubType(aServiceName) )
        {
            bFound = true;

            m_pMainTypeList->SelectEntryPos( nM );
            this->showAllControls( **aIter );
            uno::Reference< beans::XPropertySet > xTemplateProps( aTemplate.first, uno::UNO_QUERY );
            ChartTypeParameter aParameter = (*aIter)->getChartTypeParameterForService( aServiceName, xTemplateProps );
            m_pCurrentMainType = this->getSelectedMainType();

            //set ThreeDLookScheme
            aParameter.eThreeDLookScheme = ThreeDHelper::detectScheme( xDiagram );
            if(!aParameter.b3DLook && aParameter.eThreeDLookScheme!=ThreeDLookScheme_Realistic )
                aParameter.eThreeDLookScheme=ThreeDLookScheme_Realistic;

            aParameter.bSortByXValues = lcl_getSortByXValues( m_xChartModel );

            this->fillAllControls( aParameter );
            if( m_pCurrentMainType )
                m_pCurrentMainType->fillExtraControls(aParameter,m_xChartModel,xTemplateProps);
            break;
        }
    }

    if( !bFound )
    {
        m_pSubTypeList->Hide();
        m_pDim3DLookResourceGroup->showControls( false );
        m_pStackingResourceGroup->showControls( false, false );
        m_pSplineResourceGroup->showControls( false );
        m_pGeometryResourceGroup->showControls( false );
        m_pSortByXValuesResourceGroup->showControls( false );
    }
}

bool ChartTypeTabPage::commitPage( ::svt::WizardTypes::CommitPageReason /*eReason*/ )
{
    //commit changes to model
    if( !m_bDoLiveUpdate && m_pCurrentMainType )
    {
        ChartTypeParameter aParameter( this->getCurrentParamter() );
        m_pCurrentMainType->adjustParameterToSubType( aParameter );
        commitToModel( aParameter );
    }

    return true; // return false if this page should not be left
}

uno::Reference< XChartTypeTemplate > ChartTypeTabPage::getCurrentTemplate() const
{
    if( m_pCurrentMainType && m_xChartModel.is() )
    {
        ChartTypeParameter aParameter( this->getCurrentParamter() );
        m_pCurrentMainType->adjustParameterToSubType( aParameter );
        uno::Reference< lang::XMultiServiceFactory > xTemplateManager( m_xChartModel->getChartTypeManager(), uno::UNO_QUERY );
        return m_pCurrentMainType->getCurrentTemplate( aParameter, xTemplateManager );
    }
    return 0;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
