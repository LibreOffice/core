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
#include "tp_ChartType.hrc"
#include "Strings.hrc"
#include "ResId.hxx"
#include "ChartModelHelper.hxx"
#include "DiagramHelper.hxx"
#include "NoWarningThisInCTOR.hxx"
#include "res_BarGeometry.hxx"
#include "ControllerLockGuard.hxx"
#include "macros.hxx"

#include <svtools/controldims.hrc>

// header for define RET_OK
#include <vcl/msgbox.hxx>

//for auto_ptr
#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

namespace
{
    long lcl_getDistance()
    {
        return 6;
    }

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

//--------------------------------------------------------------------------
class AxisTypeResourceGroup : public ChangingResource
{
public:
    AxisTypeResourceGroup( Window* pWindow );
    virtual ~AxisTypeResourceGroup();

    void  showControls( bool bShow );
    Point getPosition();
    long  getHeight();
    void  setPosition( const Point& rPoint );

    void fillControls( const ChartTypeParameter& rParameter );
    void fillParameter( ChartTypeParameter& rParameter );

private:
    DECL_LINK( AxisTypeCheckHdl, void* );
private:
    CheckBox    m_aCB_XAxis_Categories;
};
AxisTypeResourceGroup::AxisTypeResourceGroup( Window* pWindow )
        : ChangingResource()
        , m_aCB_XAxis_Categories( pWindow, SchResId( CB_X_AXIS_CATEGORIES ) )
{
    m_aCB_XAxis_Categories.SetToggleHdl( LINK( this, AxisTypeResourceGroup, AxisTypeCheckHdl ) );
}
AxisTypeResourceGroup::~AxisTypeResourceGroup()
{
}
void AxisTypeResourceGroup::showControls( bool bShow )
{
    m_aCB_XAxis_Categories.Show(bShow);
}
Point AxisTypeResourceGroup::getPosition()
{
    return m_aCB_XAxis_Categories.GetPosPixel();
}
long AxisTypeResourceGroup::getHeight()
{
    return m_aCB_XAxis_Categories.GetSizePixel().Height();
}
void AxisTypeResourceGroup::setPosition( const Point& rPoint )
{
    m_aCB_XAxis_Categories.SetPosPixel(rPoint);
}
void AxisTypeResourceGroup::fillControls( const ChartTypeParameter& rParameter )
{
    m_aCB_XAxis_Categories.Check(!rParameter.bXAxisWithValues);
}
void AxisTypeResourceGroup::fillParameter( ChartTypeParameter& rParameter )
{
    rParameter.bXAxisWithValues = !m_aCB_XAxis_Categories.IsChecked();
}
IMPL_LINK_NOARG(AxisTypeResourceGroup, AxisTypeCheckHdl)
{
    if( m_pChangeListener )
        m_pChangeListener->stateChanged(this);
    return 0;
}
//--------------------------------------------------------------------------

#define POS_3DSCHEME_SIMPLE    0
#define POS_3DSCHEME_REALISTIC 1

class Dim3DLookResourceGroup : public ChangingResource
{
public:
    Dim3DLookResourceGroup( Window* pWindow );
    virtual ~Dim3DLookResourceGroup();

    void  showControls( bool bShow );
    long  getHeight();
    void  setPosition( const Point& rPoint );

    void fillControls( const ChartTypeParameter& rParameter );
    void fillParameter( ChartTypeParameter& rParameter );

private:
    DECL_LINK( Dim3DLookCheckHdl, void* );
    DECL_LINK( SelectSchemeHdl, void* );

private:
    CheckBox    m_aCB_3DLook;
    ListBox     m_aLB_Scheme;
};
Dim3DLookResourceGroup::Dim3DLookResourceGroup( Window* pWindow )
            : ChangingResource()
            , m_aCB_3DLook( pWindow, SchResId( CB_3D_LOOK ) )
            , m_aLB_Scheme( pWindow, SchResId( LB_3D_SCHEME ) )
{
    m_aCB_3DLook.SetToggleHdl( LINK( this, Dim3DLookResourceGroup, Dim3DLookCheckHdl ) );

    m_aLB_Scheme.InsertEntry(String(SchResId(STR_3DSCHEME_SIMPLE)));
    m_aLB_Scheme.InsertEntry(String(SchResId(STR_3DSCHEME_REALISTIC)));
    m_aLB_Scheme.SetDropDownLineCount(2);

    m_aLB_Scheme.SetSelectHdl( LINK( this, Dim3DLookResourceGroup, SelectSchemeHdl ) );
    m_aLB_Scheme.SetAccessibleName(m_aCB_3DLook.GetText());
    m_aLB_Scheme.SetAccessibleRelationLabeledBy(&m_aCB_3DLook);
}
Dim3DLookResourceGroup::~Dim3DLookResourceGroup()
{
}
void Dim3DLookResourceGroup::showControls( bool bShow )
{
    m_aCB_3DLook.Show(bShow);
    m_aLB_Scheme.Show(bShow);
}
long Dim3DLookResourceGroup::getHeight()
{
    return m_aCB_3DLook.GetSizePixel().Height() + m_aLB_Scheme.LogicToPixel( Size(0,2), MapMode(MAP_APPFONT) ).Height();
}
void Dim3DLookResourceGroup::setPosition( const Point& rPoint )
{
    m_aCB_3DLook.SetPosPixel(rPoint);
    Size aSize( m_aCB_3DLook.CalcMinimumSize() );
    m_aCB_3DLook.SetPosSizePixel(rPoint,aSize);
    Point aLBPos(rPoint);
    aLBPos.X() += aSize.Width()
                + m_aLB_Scheme.LogicToPixel( Size(6,0), MapMode(MAP_APPFONT) ).Width();
    aLBPos.Y() -= m_aLB_Scheme.LogicToPixel( Size(0,2), MapMode(MAP_APPFONT) ).Height();
    m_aLB_Scheme.SetPosPixel(aLBPos);
}
void Dim3DLookResourceGroup::fillControls( const ChartTypeParameter& rParameter )
{
    m_aCB_3DLook.Check(rParameter.b3DLook);
    m_aLB_Scheme.Enable(rParameter.b3DLook);

    if( rParameter.eThreeDLookScheme == ThreeDLookScheme_Simple )
        m_aLB_Scheme.SelectEntryPos(POS_3DSCHEME_SIMPLE);
    else if( rParameter.eThreeDLookScheme == ThreeDLookScheme_Realistic )
        m_aLB_Scheme.SelectEntryPos(POS_3DSCHEME_REALISTIC);
    else
        m_aLB_Scheme.SetNoSelection();
}
void Dim3DLookResourceGroup::fillParameter( ChartTypeParameter& rParameter )
{
    rParameter.b3DLook = m_aCB_3DLook.IsChecked();
    sal_uInt16 nPos = m_aLB_Scheme.GetSelectEntryPos();
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

//--------------------------------------------------------------------------

class SortByXValuesResourceGroup : public ChangingResource
{
public:
    SortByXValuesResourceGroup( Window* pWindow );
    virtual ~SortByXValuesResourceGroup();

    void  showControls( bool bShow );
    long  getHeight();
    void  setPosition( const Point& rPoint );

    void fillControls( const ChartTypeParameter& rParameter );
    void fillParameter( ChartTypeParameter& rParameter );

private:
    DECL_LINK( SortByXValuesCheckHdl, void* );

private:
    CheckBox    m_aCB_XValueSorting;
};
SortByXValuesResourceGroup::SortByXValuesResourceGroup( Window* pWindow )
            : ChangingResource()
            , m_aCB_XValueSorting( pWindow, SchResId( CB_XVALUE_SORTING ) )
{
    m_aCB_XValueSorting.SetToggleHdl( LINK( this, SortByXValuesResourceGroup, SortByXValuesCheckHdl ) );
}
SortByXValuesResourceGroup::~SortByXValuesResourceGroup()
{
}
void SortByXValuesResourceGroup::showControls( bool bShow )
{
    m_aCB_XValueSorting.Show(bShow);
}
long SortByXValuesResourceGroup::getHeight()
{
    return m_aCB_XValueSorting.GetSizePixel().Height();
}
void SortByXValuesResourceGroup::setPosition( const Point& rPoint )
{
    m_aCB_XValueSorting.SetPosPixel(rPoint);
}
void SortByXValuesResourceGroup::fillControls( const ChartTypeParameter& rParameter )
{
    m_aCB_XValueSorting.Check( rParameter.bSortByXValues );
}
void SortByXValuesResourceGroup::fillParameter( ChartTypeParameter& rParameter )
{
    rParameter.bSortByXValues = m_aCB_XValueSorting.IsChecked();
}
IMPL_LINK_NOARG(SortByXValuesResourceGroup, SortByXValuesCheckHdl)
{
    if(m_pChangeListener)
        m_pChangeListener->stateChanged(this);
    return 0;
}
//--------------------------------------------------------------------------
class StackingResourceGroup : public ChangingResource
{
public:
    StackingResourceGroup( Window* pWindow );
    virtual ~StackingResourceGroup();

    void  showControls( bool bShow, bool bShowDeepStacking );
    Point getPosition();
    long  getHeight();
    void  setPosition( const Point& rPoint );

    void fillControls( const ChartTypeParameter& rParameter );
    void fillParameter( ChartTypeParameter& rParameter );

private:
    DECL_LINK( StackingChangeHdl, RadioButton* );
    DECL_LINK( StackingEnableHdl, void* );

private:
    CheckBox    m_aCB_Stacked;
    RadioButton m_aRB_Stack_Y;
    RadioButton m_aRB_Stack_Y_Percent;
    RadioButton m_aRB_Stack_Z;

    bool m_bShowDeepStacking;
};
StackingResourceGroup::StackingResourceGroup( Window* pWindow )
        : ChangingResource()
        , m_aCB_Stacked( pWindow, SchResId( CB_STACKED ) )
        , m_aRB_Stack_Y( pWindow, SchResId( RB_STACK_Y ) )
        , m_aRB_Stack_Y_Percent( pWindow, SchResId( RB_STACK_Y_PERCENT ) )
        , m_aRB_Stack_Z( pWindow, SchResId( RB_STACK_Z ) )
        , m_bShowDeepStacking(true)
{
    m_aCB_Stacked.SetToggleHdl( LINK( this, StackingResourceGroup, StackingEnableHdl ) );
    m_aRB_Stack_Y.SetToggleHdl( LINK( this, StackingResourceGroup, StackingChangeHdl ) );
    m_aRB_Stack_Y_Percent.SetToggleHdl( LINK( this, StackingResourceGroup, StackingChangeHdl ) );
    m_aRB_Stack_Z.SetToggleHdl( LINK( this, StackingResourceGroup, StackingChangeHdl ) );
    m_aRB_Stack_Y.SetAccessibleRelationMemberOf(&m_aCB_Stacked);
    m_aRB_Stack_Y_Percent.SetAccessibleRelationMemberOf(&m_aCB_Stacked);
    m_aRB_Stack_Z.SetAccessibleRelationMemberOf(&m_aCB_Stacked);
}
StackingResourceGroup::~StackingResourceGroup()
{
}
void StackingResourceGroup::showControls( bool bShow, bool bShowDeepStacking )
{
    m_bShowDeepStacking = bShowDeepStacking;
    m_aCB_Stacked.Show(bShow);
    m_aRB_Stack_Y.Show(bShow);
    m_aRB_Stack_Y_Percent.Show(bShow);
    m_aRB_Stack_Z.Show(bShow&&bShowDeepStacking);
}
Point StackingResourceGroup::getPosition()
{
    return m_aCB_Stacked.GetPosPixel();
}
long StackingResourceGroup::getHeight()
{
    RadioButton& rLastButton = m_bShowDeepStacking ? m_aRB_Stack_Z : m_aRB_Stack_Y_Percent;

    long nHeight = rLastButton.GetPosPixel().Y()
        - m_aCB_Stacked.GetPosPixel().Y();
    nHeight += rLastButton.GetSizePixel().Height();
    return nHeight;
}
void StackingResourceGroup::setPosition( const Point& rPoint )
{
    Point aOld = this->getPosition();
    long nDiffY = rPoint.Y() - aOld.Y();
    long nDiffX = rPoint.X() - aOld.X();
    m_aCB_Stacked.SetPosPixel( Point( aOld.X()+nDiffX, aOld.Y()+nDiffY ) );

    aOld = m_aRB_Stack_Y.GetPosPixel();
    m_aRB_Stack_Y.SetPosPixel( Point( aOld.X()+nDiffX, aOld.Y()+nDiffY ) );

    aOld = m_aRB_Stack_Y_Percent.GetPosPixel();
    m_aRB_Stack_Y_Percent.SetPosPixel( Point( aOld.X()+nDiffX, aOld.Y()+nDiffY ) );

    aOld = m_aRB_Stack_Z.GetPosPixel();
    m_aRB_Stack_Z.SetPosPixel( Point( aOld.X()+nDiffX, aOld.Y()+nDiffY ) );
}
void StackingResourceGroup::fillControls( const ChartTypeParameter& rParameter )
{
    m_aCB_Stacked.Check( rParameter.eStackMode!=GlobalStackMode_NONE
        && rParameter.eStackMode!=GlobalStackMode_STACK_Z ); //todo remove this condition if z stacking radio button is really used
    switch( rParameter.eStackMode )
    {
        case GlobalStackMode_STACK_Y:
            m_aRB_Stack_Y.Check();
            break;
        case GlobalStackMode_STACK_Y_PERCENT:
            m_aRB_Stack_Y_Percent.Check();
            break;
        case GlobalStackMode_STACK_Z:
            //todo uncomment this condition if z stacking radio button is really used
            /*
            if( rParameter.b3DLook )
                m_aRB_Stack_Z.Check();
            else
            */
                m_aRB_Stack_Y.Check();
            break;
        default:
            m_aRB_Stack_Y.Check();
            break;
    }
    //dis/enabling
    m_aCB_Stacked.Enable( !rParameter.bXAxisWithValues );
    m_aRB_Stack_Y.Enable( m_aCB_Stacked.IsChecked() && !rParameter.bXAxisWithValues );
    m_aRB_Stack_Y_Percent.Enable( m_aCB_Stacked.IsChecked() && !rParameter.bXAxisWithValues );
    m_aRB_Stack_Z.Enable( m_aCB_Stacked.IsChecked() && rParameter.b3DLook );
}
void StackingResourceGroup::fillParameter( ChartTypeParameter& rParameter )
{
    if(!m_aCB_Stacked.IsChecked())
        rParameter.eStackMode = GlobalStackMode_NONE;
    else if(m_aRB_Stack_Y.IsChecked())
        rParameter.eStackMode = GlobalStackMode_STACK_Y;
    else if(m_aRB_Stack_Y_Percent.IsChecked())
        rParameter.eStackMode = GlobalStackMode_STACK_Y_PERCENT;
    else if(m_aRB_Stack_Z.IsChecked())
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
//--------------------------------------------------------------------------
class SplinePropertiesDialog : public ModalDialog
{
public:
    SplinePropertiesDialog( Window* pParent );
    virtual ~SplinePropertiesDialog();

    void fillControls( const ChartTypeParameter& rParameter );
    void fillParameter( ChartTypeParameter& rParameter, bool bSmoothLines );

    virtual void StateChanged( StateChangedType nType );

private:
    DECL_LINK( SplineModeRadioHdl, void* );

    void adjustControlPositions();
    void adjustSize();

private:
    RadioButton m_aRB_Splines_Cubic;
    RadioButton m_aRB_Splines_B;

    FixedLine   m_aFL_SplineSeparator;

    FixedText   m_aFT_SplineResolution;
    MetricField m_aMF_SplineResolution;
    FixedText   m_aFT_SplineOrder;
    MetricField m_aMF_SplineOrder;

    FixedLine       m_aFL_DialogButtons;
    HelpButton      m_aBP_Help;
    OKButton        m_aBP_OK;
    CancelButton    m_aBP_Cancel;
};

SplinePropertiesDialog::SplinePropertiesDialog( Window* pParent )
        : ModalDialog( pParent, SchResId( DLG_SPLINE_PROPERTIES ) )
        , m_aRB_Splines_Cubic( this, SchResId( RB_SPLINES_CUBIC ) )
        , m_aRB_Splines_B( this, SchResId( RB_SPLINES_B ) )
        , m_aFL_SplineSeparator( this, SchResId( FL_SPLINE_SEPARATOR ) )
        , m_aFT_SplineResolution( this, SchResId( FT_SPLINE_RESOLUTION ) )
        , m_aMF_SplineResolution( this, SchResId( MF_SPLINE_RESOLUTION ) )
        , m_aFT_SplineOrder( this, SchResId( FT_SPLINE_ORDER ) )
        , m_aMF_SplineOrder( this, SchResId( MF_SPLINE_ORDER ) )
        , m_aFL_DialogButtons( this, SchResId( FL_SPLINE_DIALOGBUTTONS ) )
    , m_aBP_Help( this, SchResId(BTN_HELP) )
        , m_aBP_OK( this, SchResId(BTN_OK) )
    , m_aBP_Cancel( this, SchResId(BTN_CANCEL) )
{
    FreeResource();

    this->SetText( String( SchResId( STR_DLG_SMOOTH_LINE_PROPERTIES ) ) );

    m_aRB_Splines_Cubic.SetToggleHdl( LINK( this, SplinePropertiesDialog, SplineModeRadioHdl ) );
    m_aRB_Splines_B.SetToggleHdl( LINK( this, SplinePropertiesDialog, SplineModeRadioHdl ) );
}

SplinePropertiesDialog::~SplinePropertiesDialog()
{
}

void SplinePropertiesDialog::StateChanged( StateChangedType nType )
{
    Dialog::StateChanged( nType );

    if( nType == STATE_CHANGE_INITSHOW )
    {
        adjustControlPositions();
        adjustSize();
    }
}

void SplinePropertiesDialog::adjustControlPositions()
{
    //position of controls:
    //----------------
    //fixed line and Fixed texts as near as possible to radio controls
    long nRBWidth = ::std::max( m_aRB_Splines_Cubic.CalcMinimumSize().Width(), m_aRB_Splines_B.CalcMinimumSize().Width());
    long nFLXPos = m_aRB_Splines_Cubic.GetPosPixel().X()+nRBWidth+m_aFT_SplineOrder.LogicToPixel( Size(RSC_SP_CTRL_GROUP_X,0), MapMode(MAP_APPFONT) ).Width();
    long nFTXPos = nFLXPos + m_aFL_SplineSeparator.GetSizePixel().Width() + m_aFT_SplineOrder.LogicToPixel( Size(RSC_SP_CTRL_GROUP_X,0), MapMode(MAP_APPFONT) ).Width();

    m_aRB_Splines_Cubic.SetSizePixel( Size( nRBWidth, m_aRB_Splines_Cubic.GetSizePixel().Height() ) );
    m_aRB_Splines_B.SetSizePixel( Size( nRBWidth, m_aRB_Splines_B.GetSizePixel().Height() ) );

    m_aFL_SplineSeparator.SetPosPixel( Point( nFLXPos, m_aFL_SplineSeparator.GetPosPixel().Y() ) );

    m_aFT_SplineOrder.SetPosPixel( Point( nFTXPos, m_aFT_SplineOrder.GetPosPixel().Y() ) );
    m_aFT_SplineResolution.SetPosPixel( Point( nFTXPos, m_aFT_SplineResolution.GetPosPixel().Y() ) );

    //----------------
    //move metric controls as near to text as possible
    long nFTWidth = ::std::max( m_aFT_SplineOrder.CalcMinimumSize().Width(), m_aFT_SplineResolution.CalcMinimumSize().Width());
    long nMFXPos = m_aFT_SplineOrder.GetPosPixel().X()+nFTWidth+m_aFT_SplineOrder.LogicToPixel( Size(RSC_SP_CTRL_DESC_X,0), MapMode(MAP_APPFONT) ).Width();

    m_aFT_SplineOrder.SetSizePixel( Size( nFTWidth, m_aFT_SplineOrder.GetSizePixel().Height() ) );
    m_aFT_SplineResolution.SetSizePixel( Size( nFTWidth, m_aFT_SplineResolution.GetSizePixel().Height() ) );

    m_aMF_SplineOrder.SetPosPixel( Point( nMFXPos, m_aMF_SplineOrder.GetPosPixel().Y() ) );
    m_aMF_SplineResolution.SetPosPixel( Point( nMFXPos, m_aMF_SplineResolution.GetPosPixel().Y() ) );
}

void SplinePropertiesDialog::adjustSize()
{
    Size aDlgSize( this->GetSizePixel() );
    long nBorder = m_aRB_Splines_Cubic.GetPosPixel().X();
    long nX = m_aMF_SplineOrder.GetPosPixel().X() + m_aMF_SplineOrder.GetSizePixel().Width();
    if(aDlgSize.Width()< (nX+nBorder) )
    {
        aDlgSize.Width() = (nX+nBorder);
        this->SetSizePixel(aDlgSize);

        Size aLineSize( m_aFL_DialogButtons.GetSizePixel() );
        aLineSize.Width() = aDlgSize.Width();
        m_aFL_DialogButtons.SetSizePixel(aLineSize);
    }
}
void SplinePropertiesDialog::fillControls( const ChartTypeParameter& rParameter )
{
    switch(rParameter.eCurveStyle)
    {
    case CurveStyle_CUBIC_SPLINES:
        m_aRB_Splines_Cubic.Check();
        break;
    case CurveStyle_B_SPLINES:
        m_aRB_Splines_B.Check();
        break;
    default:
        m_aRB_Splines_Cubic.Check();
        break;
    }
    m_aMF_SplineOrder.SetValue( rParameter.nSplineOrder );
    m_aMF_SplineResolution.SetValue( rParameter.nCurveResolution );

    //dis/enabling
    m_aFT_SplineOrder.Enable(m_aRB_Splines_B.IsChecked());
    m_aMF_SplineOrder.Enable(m_aRB_Splines_B.IsChecked());
}
void SplinePropertiesDialog::fillParameter( ChartTypeParameter& rParameter, bool bSmoothLines )
{
    if(!bSmoothLines)
        rParameter.eCurveStyle=CurveStyle_LINES;
    else if(m_aRB_Splines_Cubic.IsChecked())
        rParameter.eCurveStyle=CurveStyle_CUBIC_SPLINES;
    else if(m_aRB_Splines_B.IsChecked())
        rParameter.eCurveStyle=CurveStyle_B_SPLINES;

    rParameter.nCurveResolution = static_cast< sal_Int32 >( m_aMF_SplineResolution.GetValue());
    rParameter.nSplineOrder = static_cast< sal_Int32 >( m_aMF_SplineOrder.GetValue());
}
IMPL_LINK_NOARG(SplinePropertiesDialog, SplineModeRadioHdl)
{
    m_aFT_SplineOrder.Enable(m_aRB_Splines_B.IsChecked());
    m_aMF_SplineOrder.Enable(m_aRB_Splines_B.IsChecked());
    return 0;
}

//--------------------------------------------------------------------------

class SplineResourceGroup : public ChangingResource
{
public:
    SplineResourceGroup( Window* pWindow );
    virtual ~SplineResourceGroup();

    void  showControls( bool bShow );
    Point getPosition();
    long  getHeight();
    void  setPosition( const Point& rPoint );

    void fillControls( const ChartTypeParameter& rParameter );
    void fillParameter( ChartTypeParameter& rParameter );

private:
    DECL_LINK( SplineChangeHdl, void* );
    DECL_LINK( SplineDetailsDialogHdl, void* );
    SplinePropertiesDialog& getSplinePropertiesDialog();

private:
    CheckBox    m_aCB_Splines;
    PushButton  m_aPB_DetailsDialog;
    ::std::auto_ptr< SplinePropertiesDialog > m_pSplinePropertiesDialog;
};
SplineResourceGroup::SplineResourceGroup( Window* pWindow )
        : ChangingResource()
        , m_aCB_Splines( pWindow, SchResId( CB_SPLINES ) )
        , m_aPB_DetailsDialog( pWindow, SchResId( PB_SPLINE_DIALOG ) )
        , m_pSplinePropertiesDialog()
{
    m_aCB_Splines.SetToggleHdl( LINK( this, SplineResourceGroup, SplineChangeHdl ) );
    m_aPB_DetailsDialog.SetClickHdl( LINK( this, SplineResourceGroup, SplineDetailsDialogHdl ) );
    m_aPB_DetailsDialog.SetQuickHelpText( String( SchResId(STR_DLG_SMOOTH_LINE_PROPERTIES) ) );

    Size aButtonSize( m_aPB_DetailsDialog.GetSizePixel() );
    Size aMinSize( m_aPB_DetailsDialog.CalcMinimumSize() );
    sal_Int32 nDistance = 10;
    if( pWindow )
    {
        Size aDistanceSize( pWindow->LogicToPixel( Size(RSC_SP_CTRL_DESC_X,2), MapMode(MAP_APPFONT) ) );
        nDistance = 2*aDistanceSize.Width();
    }
    aButtonSize.Width() = aMinSize.Width() + nDistance;
    m_aPB_DetailsDialog.SetSizePixel( aButtonSize );
}
SplineResourceGroup::~SplineResourceGroup()
{
}
SplinePropertiesDialog& SplineResourceGroup::getSplinePropertiesDialog()
{
    if( !m_pSplinePropertiesDialog.get() )
        m_pSplinePropertiesDialog = ::std::auto_ptr< SplinePropertiesDialog >( new SplinePropertiesDialog( m_aPB_DetailsDialog.GetParent() ) );
    return *m_pSplinePropertiesDialog;
}
void SplineResourceGroup::showControls( bool bShow )
{
    m_aCB_Splines.Show(bShow);
    m_aPB_DetailsDialog.Show(bShow);
}
Point SplineResourceGroup::getPosition()
{
    return m_aCB_Splines.GetPosPixel();
}
long SplineResourceGroup::getHeight()
{
    return m_aCB_Splines.GetSizePixel().Height() + m_aPB_DetailsDialog.LogicToPixel( Size(0,2), MapMode(MAP_APPFONT) ).Height();
}
void SplineResourceGroup::setPosition( const Point& rPoint )
{
    Size aSize( m_aCB_Splines.CalcMinimumSize() );
    Size aDistanceSize( m_aCB_Splines.LogicToPixel( Size(RSC_SP_CTRL_GROUP_X,1), MapMode(MAP_APPFONT) ) );
    m_aCB_Splines.SetSizePixel( aSize );

    Point aOld = this->getPosition();
    long nDiffY = rPoint.Y() - aOld.Y();
    long nDiffX = rPoint.X() - aOld.X();

    Point aNew( aOld.X()+nDiffX, aOld.Y()+nDiffY );
    m_aCB_Splines.SetPosPixel( aNew );

    aNew.X() += ( aSize.Width() + aDistanceSize.Width() );
    aNew.Y() -= 3*aDistanceSize.Height();
    m_aPB_DetailsDialog.SetPosPixel( aNew );
}

void SplineResourceGroup::fillControls( const ChartTypeParameter& rParameter )
{
    m_aCB_Splines.Check( rParameter.eCurveStyle!=CurveStyle_LINES );
    getSplinePropertiesDialog().fillControls( rParameter );
}
void SplineResourceGroup::fillParameter( ChartTypeParameter& rParameter )
{
    getSplinePropertiesDialog().fillParameter( rParameter, m_aCB_Splines.IsChecked() );
}
IMPL_LINK_NOARG(SplineResourceGroup, SplineChangeHdl)
{
    if( m_pChangeListener )
        m_pChangeListener->stateChanged(this);
    return 0;
}

IMPL_LINK_NOARG(SplineResourceGroup, SplineDetailsDialogHdl)
{

    ChartTypeParameter aOldParameter;
    getSplinePropertiesDialog().fillParameter( aOldParameter, m_aCB_Splines.IsChecked() );

    sal_Bool bOldSmoothLines = m_aCB_Splines.IsChecked();
    m_aCB_Splines.Check();
    if( RET_OK == getSplinePropertiesDialog().Execute() )
    {
        if( m_pChangeListener )
            m_pChangeListener->stateChanged(this);
    }
    else
    {
        //restore old state:
        m_aCB_Splines.Check( bOldSmoothLines );
        getSplinePropertiesDialog().fillControls( aOldParameter );
    }
    return 0;
}

//--------------------------------------------------------------------------

class GeometryResourceGroup : public ChangingResource
{
public:
    GeometryResourceGroup( Window* pWindow );
    virtual ~GeometryResourceGroup();

    void  showControls( bool bShow );
    long  getHeight();
    void  setPosition( const Point& rPoint );

    void fillControls( const ChartTypeParameter& rParameter );
    void fillParameter( ChartTypeParameter& rParameter );

private:
    DECL_LINK( GeometryChangeHdl, void* );

private:
    BarGeometryResources       m_aGeometryResources;
};
GeometryResourceGroup::GeometryResourceGroup( Window* pWindow )
        : ChangingResource()
        , m_aGeometryResources( pWindow )
{
    m_aGeometryResources.SetSelectHdl( LINK( this, GeometryResourceGroup, GeometryChangeHdl ) );
}
GeometryResourceGroup::~GeometryResourceGroup()
{
}
void GeometryResourceGroup::showControls( bool bShow )
{
    m_aGeometryResources.Show(bShow);
}
long GeometryResourceGroup::getHeight()
{
    return m_aGeometryResources.GetSizePixel().Height();
}
void GeometryResourceGroup::setPosition( const Point& rPoint )
{
    m_aGeometryResources.SetPosPixel( rPoint );
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

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

ChartTypeTabPage::ChartTypeTabPage( Window* pParent
        , const uno::Reference< XChartDocument >& xChartModel
        , const uno::Reference< uno::XComponentContext >& xContext
        , bool bDoLiveUpdate, bool bHideDescription )
        : OWizardPage( pParent, SchResId(TP_CHARTTYPE) )
        , m_aFT_ChooseType( this, SchResId( FT_CHARTTYPE ) )
        , m_aMainTypeList( this, SchResId( LB_CHARTTYPE ) )
        , m_aSubTypeList( this, SchResId( CT_CHARTVARIANT ) )
        , m_pAxisTypeResourceGroup( new AxisTypeResourceGroup(this) )
        , m_pDim3DLookResourceGroup( new Dim3DLookResourceGroup(this) )
        , m_pStackingResourceGroup( new StackingResourceGroup(this) )
        , m_pSplineResourceGroup( new SplineResourceGroup(this) )
        , m_pGeometryResourceGroup( new GeometryResourceGroup( this ) )
        , m_pSortByXValuesResourceGroup( new SortByXValuesResourceGroup( this ) )
        , m_nYTopPos(0)
        , m_xChartModel( xChartModel )
        , m_xCC( xContext )
        , m_aChartTypeDialogControllerList(0)
        , m_pCurrentMainType(0)
        , m_nChangingCalls(0)
        , m_bDoLiveUpdate(bDoLiveUpdate)
        , m_aTimerTriggeredControllerLock( uno::Reference< frame::XModel >( m_xChartModel, uno::UNO_QUERY ) )
{
    FreeResource();

    if( bHideDescription )
    {
        m_aFT_ChooseType.Hide();
        long nYDiff = m_aMainTypeList.GetPosPixel().Y() - m_aFT_ChooseType.GetPosPixel().Y();

        Point aOldPos( m_aMainTypeList.GetPosPixel() );
        m_aMainTypeList.SetPosPixel( Point( aOldPos.X(), aOldPos.Y() - nYDiff ) );

        aOldPos = m_aSubTypeList.GetPosPixel();
        m_aSubTypeList.SetPosPixel( Point( aOldPos.X(), aOldPos.Y() - nYDiff ) );

        aOldPos = m_pAxisTypeResourceGroup->getPosition();
        m_pAxisTypeResourceGroup->setPosition( Point( aOldPos.X(), aOldPos.Y() - nYDiff ) );

        Size aSize( this->GetSizePixel() );
        this->SetSizePixel( Size( aSize.Width(), aSize.Height()-nYDiff+3 ) );
    }
    else
    {
        Font aFont( m_aFT_ChooseType.GetControlFont() );
        aFont.SetWeight( WEIGHT_BOLD );
        m_aFT_ChooseType.SetControlFont( aFont );

        m_aFT_ChooseType.SetStyle( m_aFT_ChooseType.GetStyle() | WB_NOLABEL );
    }

    this->SetText( String(SchResId(STR_PAGE_CHARTTYPE)) );

    m_aMainTypeList.SetStyle(m_aMainTypeList.GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER | WB_FLATVALUESET | WB_3DLOOK );
    m_aMainTypeList.SetSelectHdl( LINK( this, ChartTypeTabPage, SelectMainTypeHdl ) );
    m_aSubTypeList.SetSelectHdl( LINK( this, ChartTypeTabPage, SelectSubTypeHdl ) );

    m_aSubTypeList.SetStyle(m_aSubTypeList.GetStyle() |
        WB_ITEMBORDER | WB_DOUBLEBORDER | WB_NAMEFIELD | WB_FLATVALUESET | WB_3DLOOK );
    m_aSubTypeList.SetColCount(4);
    m_aSubTypeList.SetLineCount(1);

    bool bDisableComplexChartTypes = false;
    uno::Reference< beans::XPropertySet > xProps( m_xChartModel, uno::UNO_QUERY );
    if ( xProps.is() )
    {
        try
        {
            xProps->getPropertyValue( "DisableComplexChartTypes" ) >>= bDisableComplexChartTypes;
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
    if ( !bDisableComplexChartTypes )
    {
        m_aChartTypeDialogControllerList.push_back(new XYChartDialogController() );
        m_aChartTypeDialogControllerList.push_back(new BubbleChartDialogController() );
    }
    m_aChartTypeDialogControllerList.push_back(new NetChartDialogController() );
    if ( !bDisableComplexChartTypes )
    {
        m_aChartTypeDialogControllerList.push_back(new StockChartDialogController() );
    }
    m_aChartTypeDialogControllerList.push_back(new CombiColumnLineChartDialogController() );

    ::std::vector< ChartTypeDialogController* >::const_iterator       aIter = m_aChartTypeDialogControllerList.begin();
    const ::std::vector< ChartTypeDialogController* >::const_iterator aEnd  = m_aChartTypeDialogControllerList.end();
    for( ; aIter != aEnd; ++aIter )
    {
        m_aMainTypeList.InsertEntry( (*aIter)->getName(), (*aIter)->getImage() );
        (*aIter)->setChangeListener( this );
    }

    m_nYTopPos = m_pAxisTypeResourceGroup->getPosition().Y();

    m_pAxisTypeResourceGroup->setChangeListener( this );
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
    delete m_pAxisTypeResourceGroup;
    delete m_pDim3DLookResourceGroup;
    delete m_pStackingResourceGroup;
    delete m_pSplineResourceGroup;
    delete m_pGeometryResourceGroup;
    delete m_pSortByXValuesResourceGroup;
}
ChartTypeParameter ChartTypeTabPage::getCurrentParamter() const
{
    ChartTypeParameter aParameter;
    aParameter.nSubTypeIndex = static_cast<sal_Int32>( m_aSubTypeList.GetSelectItemId() );
    m_pAxisTypeResourceGroup->fillParameter( aParameter );
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
    ControllerLockGuard aLockedControllers( uno::Reference< frame::XModel >( m_xChartModel, uno::UNO_QUERY ) );
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
        m_aMainTypeList.GetSelectEntryPos() );
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
    return 0;
}

void ChartTypeTabPage::showAllControls( ChartTypeDialogController& rTypeController )
{
    m_aSubTypeList.Show();

    long nYPos = m_nYTopPos;

    //------
    bool bShow = rTypeController.shouldShow_XAxisTypeControl();
    long nXPos = m_pAxisTypeResourceGroup->getPosition().X();
    m_pAxisTypeResourceGroup->showControls( bShow );
    if(bShow)
    {
        m_pAxisTypeResourceGroup->setPosition( Point( nXPos, nYPos ) );
        nYPos += m_pAxisTypeResourceGroup->getHeight() + lcl_getDistance();
    }
    //------
    bShow = rTypeController.shouldShow_3DLookControl();
    m_pDim3DLookResourceGroup->showControls( bShow );
    if(bShow)
    {
        m_pDim3DLookResourceGroup->setPosition( Point( nXPos, nYPos ) );
        nYPos += m_pDim3DLookResourceGroup->getHeight() + lcl_getDistance();
    }
    //------
    bShow = rTypeController.shouldShow_StackingControl();
    m_pStackingResourceGroup->showControls( bShow, rTypeController.shouldShow_DeepStackingControl() );
    if(bShow)
    {
        long nStackingXPos = nXPos;
        if( rTypeController.shouldShow_XAxisTypeControl() )
            nStackingXPos += this->LogicToPixel( Size(RSC_SP_CHK_TEXTINDENT,0), MapMode(MAP_APPFONT) ).Width();
        m_pStackingResourceGroup->setPosition( Point( nStackingXPos, nYPos ) );
        nYPos += m_pStackingResourceGroup->getHeight() + lcl_getDistance();
    }
    //------
    bShow = rTypeController.shouldShow_SplineControl();
    m_pSplineResourceGroup->showControls( bShow );
    if(bShow)
    {
        m_pSplineResourceGroup->setPosition( Point( nXPos, nYPos ) );
        nYPos += m_pSplineResourceGroup->getHeight() + lcl_getDistance();
    }
    //------
    bShow = rTypeController.shouldShow_GeometryControl();
    m_pGeometryResourceGroup->showControls( bShow );
    if(bShow)
    {
        m_pGeometryResourceGroup->setPosition( Point( nXPos+17, nYPos ) );
        nYPos += m_pGeometryResourceGroup->getHeight() + lcl_getDistance();
    }
    //------
    bShow = rTypeController.shouldShow_SortByXValuesResourceGroup();
    m_pSortByXValuesResourceGroup->showControls( bShow );
    if(bShow)
    {
        m_pSortByXValuesResourceGroup->setPosition( Point( nXPos, nYPos ) );
        nYPos += m_pSortByXValuesResourceGroup->getHeight() + lcl_getDistance();
    }
    //------
    Size aPageSize( this->GetSizePixel() );
    Size aRemainingSize = Size( aPageSize.Width()-nXPos, aPageSize.Height()-nYPos );
    rTypeController.showExtraControls( this, Point( nXPos, nYPos ), aRemainingSize );
}

void ChartTypeTabPage::fillAllControls( const ChartTypeParameter& rParameter, bool bAlsoResetSubTypeList )
{
    m_nChangingCalls++;
    if( m_pCurrentMainType && bAlsoResetSubTypeList )
    {
        m_pCurrentMainType->fillSubTypeList( m_aSubTypeList, rParameter );
    }
    m_aSubTypeList.SelectItem( static_cast<sal_uInt16>( rParameter.nSubTypeIndex) );
    m_pAxisTypeResourceGroup->fillControls( rParameter );
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

            m_aMainTypeList.SelectEntryPos( nM );
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
        m_aSubTypeList.Hide();
        m_pAxisTypeResourceGroup->showControls( false );
        m_pDim3DLookResourceGroup->showControls( false );
        m_pStackingResourceGroup->showControls( false, false );
        m_pSplineResourceGroup->showControls( false );
        m_pGeometryResourceGroup->showControls( false );
        m_pSortByXValuesResourceGroup->showControls( false );
    }
}

sal_Bool ChartTypeTabPage::commitPage( ::svt::WizardTypes::CommitPageReason /*eReason*/ )
{
    //commit changes to model
    if( !m_bDoLiveUpdate && m_pCurrentMainType )
    {
        ChartTypeParameter aParameter( this->getCurrentParamter() );
        m_pCurrentMainType->adjustParameterToSubType( aParameter );
        commitToModel( aParameter );
    }

    return sal_True;//return false if this page should not be left
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

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
