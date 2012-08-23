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

#include <mmoutputtypepage.hxx>
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <vcl/msgbox.hxx>
#include <dbui.hrc>
#include <swtypes.hxx>
#include <mmoutputtypepage.hrc>

SwMailMergeOutputTypePage::SwMailMergeOutputTypePage( SwMailMergeWizard* _pParent) :
    svt::OWizardPage( _pParent, SW_RES(DLG_MM_OUTPUTTYPE_PAGE)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aHeaderFI(    this, SW_RES( FI_HEADER    )),
    m_aTypeFT(      this, SW_RES( FT_TYPE    )),
    m_aLetterRB(    this, SW_RES( RB_LETTER  )),
    m_aMailRB(      this, SW_RES( RB_MAIL    )),
    m_aHintHeaderFI(this, SW_RES( FI_HINTHEADER)),
    m_aHintFI(      this, SW_RES( FI_HINT)),
    m_aNoMailHintFI(this, SW_RES( FT_NOMAILHINT)),
#ifdef MSC
#pragma warning (default : 4355)
#endif
    m_sLetterHintHeader(  SW_RES( ST_LETTERHINTHEADER)),
    m_sMailHintHeader(      SW_RES( ST_MAILHINTHEADER)),
    m_sLetterHint(          SW_RES( ST_LETTERHINT)),
    m_sMailHint(            SW_RES( ST_MAILHINT)),
    m_pWizard(_pParent)
{
    FreeResource();
    Link aLink = LINK(this, SwMailMergeOutputTypePage, TypeHdl_Impl);
    m_aLetterRB.SetClickHdl(aLink);
    m_aMailRB.SetClickHdl(aLink);

    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    m_pWizard->EnterWait();
    bool bMailAvailable = rConfigItem.IsMailAvailable();
    m_pWizard->LeaveWait();
    if(rConfigItem.IsOutputToLetter())
        m_aLetterRB.Check();
    else
        m_aMailRB.Check();
    if(!bMailAvailable)
    {
        m_aNoMailHintFI.Show();
        m_aMailRB.Enable(sal_False);
        m_aLetterRB.Check();
    }
    TypeHdl_Impl(&m_aLetterRB);

}

SwMailMergeOutputTypePage::~SwMailMergeOutputTypePage()
{
}

IMPL_LINK_NOARG(SwMailMergeOutputTypePage, TypeHdl_Impl)
{
    bool bLetter = m_aLetterRB.IsChecked();
        m_aHintHeaderFI.SetText(bLetter ? m_sLetterHintHeader : m_sMailHintHeader);
        m_aHintFI.SetText(bLetter ? m_sLetterHint : m_sMailHint);
    m_pWizard->GetConfigItem().SetOutputToLetter(bLetter);
    m_pWizard->updateRoadmapItemLabel( MM_ADDRESSBLOCKPAGE );
    m_pWizard->UpdateRoadmap();
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
