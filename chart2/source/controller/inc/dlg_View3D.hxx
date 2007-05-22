/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_View3D.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:59:53 $
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
#ifndef _CHART2_DLG_VIEW3D_DIAGRAM_HXX
#define _CHART2_DLG_VIEW3D_DIAGRAM_HXX

#ifndef _COM_SUN_STAR_DRAWING_SHADEMODE_HPP_
#include <com/sun/star/drawing/ShadeMode.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

// header for class TabDialog
#ifndef _SV_TABDLG_HXX
#include <vcl/tabdlg.hxx>
#endif
// header for class TabControl
#ifndef _SV_TABCTRL_HXX
#include <vcl/tabctrl.hxx>
#endif
// header for class OKButton
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for class XColorTable
#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif

#include "ControllerLockGuard.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
class ThreeD_SceneGeometry_TabPage;
class ThreeD_SceneAppearance_TabPage;
class ThreeD_SceneIllumination_TabPage;

class View3DDialog : public TabDialog
{
public:
    View3DDialog( Window* pWindow,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xChartModel,
                  XColorTable* pColorTable=0 );
    ~View3DDialog();

    // from Dialog (base of TabDialog)
    virtual short Execute();

private:
    TabControl      m_aTabControl;
    OKButton        m_aBtnOK;
    CancelButton    m_aBtnCancel;
    HelpButton      m_aBtnHelp;

    ThreeD_SceneGeometry_TabPage*       m_pGeometry;
    ThreeD_SceneAppearance_TabPage*     m_pAppearance;
    ThreeD_SceneIllumination_TabPage*   m_pIllumination;

    ControllerLockHelper                m_aControllerLocker;

    static USHORT m_nLastPageId;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
