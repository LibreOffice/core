/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_3D_SceneAppearance.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:41:53 $
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
#ifndef _CHART2_TP_3D_SCENE_APPEARANCE_HXX
#define _CHART2_TP_3D_SCENE_APPEARANCE_HXX

// header for class TabPage
#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
// header for class FixedLine
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for class MetricField
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
// header for class CheckBox
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for class ListBox
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_SHADEMODE_HPP_
#include <com/sun/star/drawing/ShadeMode.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#include "ControllerLockGuard.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

class ThreeD_SceneAppearance_TabPage : public TabPage
{
public:
    ThreeD_SceneAppearance_TabPage(
        Window* pWindow,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xChartModel,
        ControllerLockHelper & rControllerLockHelper );
    virtual ~ThreeD_SceneAppearance_TabPage();

    // has to be called in case the dialog was closed with OK
    void commitPendingChanges();

    virtual void ActivatePage();

private:
    DECL_LINK( SelectSchemeHdl, void* );
    DECL_LINK( SelectShading, void* );
    DECL_LINK( SelectRoundedEdgeOrObjectLines, CheckBox* );

    void initControlsFromModel();
    void applyShadeModeToModel();
    void applyRoundedEdgeAndObjectLinesToModel();
    void updateScheme();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        m_xChartModel;

    FixedText       m_aFT_Scheme;
    ListBox         m_aLB_Scheme;

    FixedLine       m_aFL_Seperator;

    CheckBox        m_aCB_RoundedEdge;
    CheckBox        m_aCB_Shading;
    CheckBox        m_aCB_ObjectLines;

    bool            m_bUpdateOtherControls;
    bool            m_bCommitToModel;

    ControllerLockHelper & m_rControllerLockHelper;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
