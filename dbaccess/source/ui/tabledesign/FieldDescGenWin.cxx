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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "FieldDescGenWin.hxx"
#include <tools/debug.hxx>
#include "dbaccess_helpid.hrc"
#include "TableDesignHelpBar.hxx"
#include "TableFieldControl.hxx"
#include "TableDesignView.hxx"
#include "TEditControl.hxx"
using namespace dbaui;

//==================================================================
// class OFieldDescGenWin
//==================================================================

DBG_NAME(OFieldDescGenWin)
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
    ::std::auto_ptr<Window> aTemp(m_pFieldControl);
    m_pFieldControl = NULL;
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
#if OSL_DEBUG_LEVEL > 0
OTableEditorCtrl* OFieldDescGenWin::GetEditorCtrl()
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);
    OTableDesignView* pDesignWin = static_cast<OTableDesignView*>(GetParent()->GetParent()->GetParent());
    return pDesignWin->GetEditorCtrl();
}
#endif
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
    if(m_pFieldControl)
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
sal_Bool OFieldDescGenWin::isCopyAllowed()
{
    return (m_pFieldControl && m_pFieldControl->isCutAllowed());
}
// -----------------------------------------------------------------------------
sal_Bool OFieldDescGenWin::isCutAllowed()
{
    return (m_pFieldControl && m_pFieldControl->isCutAllowed());
}
// -----------------------------------------------------------------------------
sal_Bool OFieldDescGenWin::isPasteAllowed()
{
    return (m_pFieldControl && m_pFieldControl->isPasteAllowed());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
