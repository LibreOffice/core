/*************************************************************************
 *
 *  $RCSfile: FieldDescGenWin.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-22 07:54:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_TABLEFIELDDESCGENPAGE_HXX
#include "FieldDescGenWin.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef DBAUI_TABLEDESIGNHELPBAR_HXX
#include "TableDesignHelpBar.hxx"
#endif
#ifndef DBAUI_TABLEFIELDCONTROL_HXX
#include "TableFieldControl.hxx"
#endif
#ifndef DBAUI_TABLEDESIGNVIEW_HXX
#include "TableDesignView.hxx"
#endif
#ifndef DBAUI_TABLEEDITORCONTROL_HXX
#include "TEditControl.hxx"
#endif
using namespace dbaui;

//==================================================================
// class OFieldDescGenWin
//==================================================================

DBG_NAME(OFieldDescGenWin);
//==================================================================
//------------------------------------------------------------------------------
OFieldDescGenWin::OFieldDescGenWin( Window* pParent, OTableDesignHelpBar* pHelp ) :
     TabPage( pParent, WB_3DLOOK | WB_DIALOGCONTROL )
{
    DBG_CTOR(OFieldDescGenWin,NULL);
    m_pFieldControl = new OTableFieldControl(this,pHelp);
    m_pFieldControl->SetHelpId(HID_TAB_DESIGN_FIELDCONTROL);
    m_pFieldControl->Show();
}
//------------------------------------------------------------------------------
OFieldDescGenWin::~OFieldDescGenWin()
{
    DBG_DTOR(OFieldDescGenWin,NULL);
    delete m_pFieldControl;
}
//------------------------------------------------------------------------------
void OFieldDescGenWin::Init()
{
    DBG_ASSERT(GetEditorCtrl() != NULL, "OFieldDescGenWin::Init : have no editor control !");

    m_pFieldControl->Init();
}
//------------------------------------------------------------------------------
void OFieldDescGenWin::Resize()
{
    m_pFieldControl->SetPosSizePixel(Point(0,0),GetSizePixel());
    m_pFieldControl->Resize();
}
//------------------------------------------------------------------------------
void OFieldDescGenWin::SetReadOnly( sal_Bool bReadOnly )
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);

    m_pFieldControl->SetReadOnly(bReadOnly);
}
//------------------------------------------------------------------------------
String OFieldDescGenWin::GetControlText( sal_uInt16 nControlId )
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);
    return m_pFieldControl->GetControlText(nControlId);
}
//------------------------------------------------------------------------------
void OFieldDescGenWin::SetControlText( sal_uInt16 nControlId, const String& rText )
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);
    //////////////////////////////////////////////////////////////////////
    // Texte der Controls setzen
    m_pFieldControl->SetControlText(nControlId,rText);
}
//------------------------------------------------------------------------------
void OFieldDescGenWin::DisplayData( OFieldDescription* pFieldDescr )
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);

    m_pFieldControl->DisplayData(pFieldDescr);
}
//------------------------------------------------------------------------------
OTableEditorCtrl* OFieldDescGenWin::GetEditorCtrl()
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);
    OTableDesignView* pDesignWin = static_cast<OTableDesignView*>(GetParent()->GetParent());
    return pDesignWin->GetEditorCtrl();
}
//------------------------------------------------------------------------------
//short OFieldDescGenWin::GetFormatCategory(OFieldDescription* pFieldDescr)
//{
//  return m_pFieldControl->GetFormatCategory(pFieldDescr);
//}
//------------------------------------------------------------------------------
void OFieldDescGenWin::ActivatePropertyField(sal_uInt16 nVirtualField)
{
    m_pFieldControl->ActivatePropertyField(nVirtualField);
}
//------------------------------------------------------------------------------
void OFieldDescGenWin::SaveData( OFieldDescription* pFieldDescr )
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);
    m_pFieldControl->SaveData(pFieldDescr);
}
//------------------------------------------------------------------------------
void OFieldDescGenWin::GetFocus()
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);
    //////////////////////////////////////////////////////////////////////
    // Setzt den Focus auf das zuletzt aktive Control
    TabPage::GetFocus();
    m_pFieldControl->GetFocus();

}
//------------------------------------------------------------------------------
void OFieldDescGenWin::LoseFocus()
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);
    m_pFieldControl->LoseFocus();
    TabPage::LoseFocus();
}
//------------------------------------------------------------------
String OFieldDescGenWin::BoolStringPersistent(const String& rUIString) const
{
    return m_pFieldControl->BoolStringPersistent(rUIString);
}

//------------------------------------------------------------------
String OFieldDescGenWin::BoolStringUI(const String& rPersistentString) const
{
    return m_pFieldControl->BoolStringUI(rPersistentString);
}
// -----------------------------------------------------------------------------
sal_Bool OFieldDescGenWin::isCutAllowed()
{
    return (m_pFieldControl && m_pFieldControl->isCutAllowed());
}
// -----------------------------------------------------------------------------
void OFieldDescGenWin::cut()
{
    if(m_pFieldControl)
        m_pFieldControl->cut();
}
// -----------------------------------------------------------------------------
void OFieldDescGenWin::copy()
{
    if(m_pFieldControl)
        m_pFieldControl->copy();
}
// -----------------------------------------------------------------------------
void OFieldDescGenWin::paste()
{
    if(m_pFieldControl)
        m_pFieldControl->paste();
}
// -----------------------------------------------------------------------------




