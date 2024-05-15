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

#include <strings.hrc>
#include "openlocked.hxx"
#include <officecfg/Office/Common.hxx>
#include <vcl/weldutils.hxx>

using namespace vcl;

IMPL_LINK(OpenLockedQueryBox, ClickHdl, weld::Button&, rButton, void)
{
    if (&rButton == mxOpenReadOnlyBtn.get())
    {
        if (mxNotifyBtn->get_active())
            m_xDialog->response(RET_RETRY);
        else
            m_xDialog->response(RET_YES);
    }
    else if (&rButton == mxOpenCopyBtn.get())
    {
        m_xDialog->response(RET_NO);
    }
    else if (&rButton == mxOpenBtn.get())
    {
        m_xDialog->response(RET_IGNORE);
    }
    else if (&rButton == mxCancelBtn.get())
    {
        m_xDialog->response(RET_CANCEL);
    }
}

OpenLockedQueryBox::OpenLockedQueryBox(weld::Window* pParent, const OUString& rHiddenData, bool bEnableOverride)
    : GenericDialogController(pParent, u"vcl/ui/openlockedquerybox.ui"_ustr, u"OpenLockedQueryBox"_ustr)
    , mxQuestionMarkImage(m_xBuilder->weld_image(u"questionmark"_ustr))
    , mxOpenReadOnlyBtn(m_xBuilder->weld_button(u"readonly"_ustr))
    , mxOpenCopyBtn(m_xBuilder->weld_button(u"opencopy"_ustr))
    , mxOpenBtn(m_xBuilder->weld_button(u"open"_ustr))
    , mxCancelBtn(m_xBuilder->weld_button(u"cancel"_ustr))
    , mxNotifyBtn(m_xBuilder->weld_check_button(u"notify"_ustr))
    , mxHiddenText(m_xBuilder->weld_label(u"hiddentext"_ustr))
{
    //setup click hdl
    mxOpenReadOnlyBtn->connect_clicked(LINK(this, OpenLockedQueryBox, ClickHdl));
    mxOpenCopyBtn->connect_clicked(LINK(this, OpenLockedQueryBox, ClickHdl));
    mxOpenBtn->connect_clicked(LINK(this, OpenLockedQueryBox, ClickHdl));
    mxCancelBtn->connect_clicked(LINK(this, OpenLockedQueryBox, ClickHdl));

    if (!(bEnableOverride && officecfg::Office::Common::Misc::AllowOverrideLocking::get()))
    {
        //disable option to ignore the (stale?) lock file and open the document
        mxOpenBtn->set_sensitive(false);
    }

    mxHiddenText->set_label(rHiddenData);
    m_xDialog->set_centered_on_parent(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
