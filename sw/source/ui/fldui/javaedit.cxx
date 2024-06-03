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

#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <svl/urihelper.hxx>
#include <view.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/filedlghelper.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <fldbas.hxx>
#include <fldmgr.hxx>
#include <docufld.hxx>
#include <javaedit.hxx>

#include <strings.hrc>

using namespace ::com::sun::star;

SwJavaEditDialog::SwJavaEditDialog(weld::Window* pParent, SwWrtShell* pWrtSh)
    : GenericDialogController(pParent, u"modules/swriter/ui/insertscript.ui"_ustr, u"InsertScriptDialog"_ustr)
    , m_bNew(true)
    , m_bIsUrl(false)
    , m_pSh(pWrtSh)
    , m_xTypeED(m_xBuilder->weld_entry(u"scripttype"_ustr))
    , m_xUrlRB(m_xBuilder->weld_radio_button(u"url"_ustr))
    , m_xEditRB(m_xBuilder->weld_radio_button(u"text"_ustr))
    , m_xUrlPB(m_xBuilder->weld_button(u"browse"_ustr))
    , m_xUrlED(m_xBuilder->weld_entry(u"urlentry"_ustr))
    , m_xEditED(m_xBuilder->weld_text_view(u"textentry"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xPrevBtn(m_xBuilder->weld_button(u"previous"_ustr))
    , m_xNextBtn(m_xBuilder->weld_button(u"next"_ustr))
{
    // install handler
    m_xPrevBtn->connect_clicked( LINK( this, SwJavaEditDialog, PrevHdl ) );
    m_xNextBtn->connect_clicked( LINK( this, SwJavaEditDialog, NextHdl ) );
    m_xOKBtn->connect_clicked( LINK( this, SwJavaEditDialog, OKHdl ) );

    Link<weld::Toggleable&,void> aLk = LINK(this, SwJavaEditDialog, RadioButtonHdl);
    m_xUrlRB->connect_toggled(aLk);
    m_xEditRB->connect_toggled(aLk);
    m_xUrlPB->connect_clicked(LINK(this, SwJavaEditDialog, InsertFileHdl));

    m_pMgr.reset(new SwFieldMgr(m_pSh));
    m_pField = static_cast<SwScriptField*>(m_pMgr->GetCurField());

    m_bNew = !(m_pField && m_pField->GetTyp()->Which() == SwFieldIds::Script);

    CheckTravel();

    if (!m_bNew)
        m_xDialog->set_title(SwResId(STR_JAVA_EDIT));

    UpdateFromRadioButtons();
}

SwJavaEditDialog::~SwJavaEditDialog()
{
    m_pSh->EnterStdMode();
    m_pMgr.reset();
    m_pFileDlg.reset();
}

IMPL_LINK_NOARG(SwJavaEditDialog, PrevHdl, weld::Button&, void)
{
    m_pSh->EnterStdMode();

    SetField();
    m_pMgr->GoPrev();
    m_pField = static_cast<SwScriptField*>(m_pMgr->GetCurField());
    CheckTravel();
    UpdateFromRadioButtons();
}

IMPL_LINK_NOARG(SwJavaEditDialog, NextHdl, weld::Button&, void)
{
    m_pSh->EnterStdMode();

    SetField();
    m_pMgr->GoNext();
    m_pField = static_cast<SwScriptField*>(m_pMgr->GetCurField());
    CheckTravel();
    UpdateFromRadioButtons();
}

IMPL_LINK_NOARG(SwJavaEditDialog, OKHdl, weld::Button&, void)
{
    SetField();
    m_xDialog->response(RET_OK);
}

void SwJavaEditDialog::CheckTravel()
{
    bool bTravel = false;
    bool bNext(false), bPrev(false);

    if (!m_bNew)
    {
        // Traveling only when more than one field
        m_pSh->StartAction();
        m_pSh->CreateCursor();

        bNext = m_pMgr->GoNext();
        if( bNext )
            m_pMgr->GoPrev();

        bPrev = m_pMgr->GoPrev();
        if( bPrev )
            m_pMgr->GoNext();
        bTravel |= bNext || bPrev;

        m_pSh->DestroyCursor();
        m_pSh->EndAction();

        if (m_pField->IsCodeURL())
        {
            OUString sURL(m_pField->GetPar2());
            if(!sURL.isEmpty())
            {
                INetURLObject aINetURL(sURL);
                if(INetProtocol::File == aINetURL.GetProtocol())
                    sURL = aINetURL.PathToFileName();
            }
            m_xUrlED->set_text(sURL);
            m_xEditED->set_text(OUString());
            m_xUrlRB->set_active(true);
        }
        else
        {
            m_xEditED->set_text(m_pField->GetPar2());
            m_xUrlED->set_text(OUString());
            m_xEditRB->set_active(true);
        }
        m_xTypeED->set_text(m_pField->GetPar1());
    }

    if ( !bTravel )
    {
        m_xPrevBtn->hide();
        m_xNextBtn->hide();
    }
    else
    {
        m_xPrevBtn->set_sensitive(bPrev);
        m_xNextBtn->set_sensitive(bNext);
    }
}

void SwJavaEditDialog::SetField()
{
    if( !m_xOKBtn->get_sensitive() )
        return ;

    m_aType = m_xTypeED->get_text();
    m_bIsUrl = m_xUrlRB->get_active();

    if (m_bIsUrl)
    {
        m_aText = m_xUrlED->get_text();
        if (!m_aText.isEmpty())
        {
            SfxMedium* pMedium = m_pSh->GetView().GetDocShell()->GetMedium();
            INetURLObject aAbs;
            if( pMedium )
                aAbs = pMedium->GetURLObject();

            m_aText = URIHelper::SmartRel2Abs(
                aAbs, m_aText, URIHelper::GetMaybeFileHdl());
        }
    }
    else
        m_aText = m_xEditED->get_text();

    if (m_aType.isEmpty())
        m_aType = "JavaScript";
}

bool SwJavaEditDialog::IsUpdate() const
{
    return m_pField && ( sal_uInt32(m_bIsUrl ? 1 : 0) != m_pField->GetFormat() || m_pField->GetPar2() != m_aType || m_pField->GetPar1() != m_aText );
}

IMPL_LINK(SwJavaEditDialog, RadioButtonHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;
    UpdateFromRadioButtons();
}

void SwJavaEditDialog::UpdateFromRadioButtons()
{
    bool bEnable = m_xUrlRB->get_active();
    m_xUrlPB->set_sensitive(bEnable);
    m_xUrlED->set_sensitive(bEnable);
    m_xEditED->set_sensitive(!bEnable);

    if (!m_bNew)
    {
        bEnable = !m_pSh->IsReadOnlyAvailable() || !m_pSh->HasReadonlySel();
        m_xOKBtn->set_sensitive(bEnable);
        m_xUrlED->set_editable(bEnable);
        m_xEditED->set_editable(bEnable);
        m_xTypeED->set_editable(bEnable);
        if( m_xUrlPB->get_sensitive() && !bEnable )
            m_xUrlPB->set_sensitive( false );
    }
}

IMPL_LINK_NOARG( SwJavaEditDialog, InsertFileHdl, weld::Button&, void )
{
    if (!m_pFileDlg)
    {
        m_pFileDlg.reset(new ::sfx2::FileDialogHelper(
            ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            FileDialogFlags::Insert, u"swriter"_ustr, SfxFilterFlags::NONE, SfxFilterFlags::NONE, m_xDialog.get()));
    }
    m_pFileDlg->SetContext(sfx2::FileDialogHelper::WriterInsertScript);
    m_pFileDlg->StartExecuteModal( LINK( this, SwJavaEditDialog, DlgClosedHdl ) );
}

IMPL_LINK_NOARG(SwJavaEditDialog, DlgClosedHdl, sfx2::FileDialogHelper *, void)
{
    if (m_pFileDlg->GetError() == ERRCODE_NONE)
    {
        OUString sFileName = m_pFileDlg->GetPath();
        if ( !sFileName.isEmpty() )
        {
            INetURLObject aINetURL( sFileName );
            if ( INetProtocol::File == aINetURL.GetProtocol() )
                sFileName = aINetURL.PathToFileName();
        }
        m_xUrlED->set_text(sFileName);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
