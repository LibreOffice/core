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

#include "FieldDescGenWin.hxx"
#include <osl/diagnose.h>
#include "dbaccess_helpid.hrc"
#include "TableDesignHelpBar.hxx"
#include "TableFieldControl.hxx"
#include "TableDesignView.hxx"
#include "TEditControl.hxx"
using namespace dbaui;

// class OFieldDescGenWin

DBG_NAME(OFieldDescGenWin)
OFieldDescGenWin::OFieldDescGenWin( Window* pParent, OTableDesignHelpBar* pHelp ) :
     TabPage( pParent, WB_3DLOOK | WB_DIALOGCONTROL )
{
    DBG_CTOR(OFieldDescGenWin,NULL);
    m_pFieldControl = new OTableFieldControl(this,pHelp);
    m_pFieldControl->SetHelpId(HID_TAB_DESIGN_FIELDCONTROL);
    m_pFieldControl->Show();
}

OFieldDescGenWin::~OFieldDescGenWin()
{
    DBG_DTOR(OFieldDescGenWin,NULL);
    ::std::auto_ptr<Window> aTemp(m_pFieldControl);
    m_pFieldControl = NULL;
}

void OFieldDescGenWin::Init()
{
    OSL_ENSURE(GetEditorCtrl() != NULL, "OFieldDescGenWin::Init : have no editor control !");

    m_pFieldControl->Init();
}

void OFieldDescGenWin::Resize()
{
    m_pFieldControl->SetPosSizePixel(Point(0,0),GetSizePixel());
    m_pFieldControl->Resize();
}

void OFieldDescGenWin::SetReadOnly( sal_Bool bReadOnly )
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);

    m_pFieldControl->SetReadOnly(bReadOnly);
}

String OFieldDescGenWin::GetControlText( sal_uInt16 nControlId )
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);
    return m_pFieldControl->GetControlText(nControlId);
}

void OFieldDescGenWin::SetControlText( sal_uInt16 nControlId, const String& rText )
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);
    // Texte der Controls setzen
    m_pFieldControl->SetControlText(nControlId,rText);
}

void OFieldDescGenWin::DisplayData( OFieldDescription* pFieldDescr )
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);

    m_pFieldControl->DisplayData(pFieldDescr);
}

#if OSL_DEBUG_LEVEL > 0
OTableEditorCtrl* OFieldDescGenWin::GetEditorCtrl()
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);
    OTableDesignView* pDesignWin = static_cast<OTableDesignView*>(GetParent()->GetParent()->GetParent());
    return pDesignWin->GetEditorCtrl();
}

#endif
void OFieldDescGenWin::SaveData( OFieldDescription* pFieldDescr )
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);
    m_pFieldControl->SaveData(pFieldDescr);
}

void OFieldDescGenWin::GetFocus()
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);
    // Setzt den Focus auf das zuletzt aktive Control
    TabPage::GetFocus();
    if(m_pFieldControl)
        m_pFieldControl->GetFocus();

}

void OFieldDescGenWin::LoseFocus()
{
    DBG_CHKTHIS(OFieldDescGenWin,NULL);
    m_pFieldControl->LoseFocus();
    TabPage::LoseFocus();
}

String OFieldDescGenWin::BoolStringPersistent(const String& rUIString) const
{
    return m_pFieldControl->BoolStringPersistent(rUIString);
}

String OFieldDescGenWin::BoolStringUI(const String& rPersistentString) const
{
    return m_pFieldControl->BoolStringUI(rPersistentString);
}

sal_Bool OFieldDescGenWin::isCopyAllowed()
{
    return (m_pFieldControl && m_pFieldControl->isCutAllowed());
}

sal_Bool OFieldDescGenWin::isCutAllowed()
{
    return (m_pFieldControl && m_pFieldControl->isCutAllowed());
}

sal_Bool OFieldDescGenWin::isPasteAllowed()
{
    return (m_pFieldControl && m_pFieldControl->isPasteAllowed());
}

void OFieldDescGenWin::cut()
{
    if(m_pFieldControl)
        m_pFieldControl->cut();
}

void OFieldDescGenWin::copy()
{
    if(m_pFieldControl)
        m_pFieldControl->copy();
}

void OFieldDescGenWin::paste()
{
    if(m_pFieldControl)
        m_pFieldControl->paste();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
