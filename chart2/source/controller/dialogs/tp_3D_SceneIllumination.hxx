/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_3D_SceneIllumination.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:43:11 $
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
#ifndef _CHART2_TP_3D_SCENE_ILLUMIMATION_HXX
#define _CHART2_TP_3D_SCENE_ILLUMIMATION_HXX

#include "ModifyListenerCallBack.hxx"
#include "TimerTriggeredControllerLock.hxx"

// #ifndef _COM_SUN_STAR_CHART2_SCENEDESCRIPTOR_HPP_
// #include <com/sun/star/chart2/SceneDescriptor.hpp>
// #endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

// header for class TabPage
#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
// header for class FixedText
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for class CheckBox
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for class ColorLB
#ifndef _SVX_DLG_CTRL_HXX
#include <svx/dlgctrl.hxx>
#endif
// header for class SvxLightCtl3D
#ifndef _SVX_DLGCTL3D_HXX
#include <svx/dlgctl3d.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

class LightButton : public ImageButton
{
public:
    LightButton( Window* pParent, const ResId& rResId, sal_Int32 nLightNumber );
    virtual ~LightButton();

    void switchLightOn(bool bOn);
    bool isLightOn() const;

private:
    bool m_bLightOn;
};

class ColorButton : public ImageButton
{
public:
    ColorButton( Window* pParent, const ResId& rResId );
    virtual ~ColorButton();
};

struct LightSourceInfo;

class ThreeD_SceneIllumination_TabPage : public TabPage
{
public:
    ThreeD_SceneIllumination_TabPage(
        Window* pWindow,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & xSceneProperties,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel,
        XColorTable* pColorTable=0 );
    virtual ~ThreeD_SceneIllumination_TabPage();

    // has to be called in case the dialog was closed with OK
    void commitPendingChanges();

private:
    DECL_LINK( ClickLightSourceButtonHdl, LightButton* );
    DECL_LINK( SelectColorHdl, ColorLB* );
    DECL_LINK( ColorDialogHdl, Button* );
    DECL_LINK( PreviewChangeHdl, void* );
    DECL_LINK( PreviewSelectHdl, void* );

    void updatePreview();

private:
    DECL_LINK(fillControlsFromModel, void *);

    void applyLightSourceToModel( Base3DLightNumber nLightNumber );
    void applyLightSourcesToModel();

    FixedText   m_aFT_LightSource;

    LightButton m_aBtn_Light1;
    LightButton m_aBtn_Light2;
    LightButton m_aBtn_Light3;
    LightButton m_aBtn_Light4;
    LightButton m_aBtn_Light5;
    LightButton m_aBtn_Light6;
    LightButton m_aBtn_Light7;
    LightButton m_aBtn_Light8;

    ColorLB     m_aLB_LightSource;
    ColorButton m_aBtn_LightSource_Color;

    FixedText   m_aFT_AmbientLight;
    ColorLB     m_aLB_AmbientLight;
    ColorButton m_aBtn_AmbientLight_Color;

    SvxLightCtl3D   m_aCtl_Preview;

    LightSourceInfo* m_pLightSourceInfoList;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > m_xSceneProperties;

    TimerTriggeredControllerLock    m_aTimerTriggeredControllerLock;

    bool            m_bInCommitToModel;

    ModifyListenerCallBack m_aModelChangeListener;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel >       m_xChartModel;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
