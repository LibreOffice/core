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
#ifndef _CHART2_TP_3D_SCENE_ILLUMIMATION_HXX
#define _CHART2_TP_3D_SCENE_ILLUMIMATION_HXX

#include "ModifyListenerCallBack.hxx"
#include "TimerTriggeredControllerLock.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>

// header for class TabPage
#include <vcl/tabpage.hxx>
// header for class FixedText
#include <vcl/fixed.hxx>
// header for class CheckBox
#include <vcl/button.hxx>
// header for class ColorLB
#include <svx/dlgctrl.hxx>
// header for class SvxLightCtl3D
#include <svx/dlgctl3d.hxx>

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

    void applyLightSourceToModel( sal_uInt32 nLightNumber );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
