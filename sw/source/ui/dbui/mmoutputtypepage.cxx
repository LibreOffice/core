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

#include <mmoutputtypepage.hxx>
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <vcl/msgbox.hxx>
#include <dbui.hrc>
#include <swtypes.hxx>
#include <mmoutputtypepage.hrc>

SwMailMergeOutputTypePage::SwMailMergeOutputTypePage( SwMailMergeWizard* _pParent) :
    svt::OWizardPage( _pParent, SW_RES(DLG_MM_OUTPUTTYPE_PAGE)),
#ifdef _MSC_VER
#pragma warning (disable : 4355)
#endif
    m_aHeaderFI(    this, SW_RES( FI_HEADER    )),
    m_aTypeFT(      this, SW_RES( FT_TYPE    )),
    m_aLetterRB(    this, SW_RES( RB_LETTER  )),
    m_aMailRB(      this, SW_RES( RB_MAIL    )),
    m_aHintHeaderFI(this, SW_RES( FI_HINTHEADER)),
    m_aHintFI(      this, SW_RES( FI_HINT)),
#ifdef _MSC_VER
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
    if(rConfigItem.IsOutputToLetter())
        m_aLetterRB.Check();
    else
        m_aMailRB.Check();
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
