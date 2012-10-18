/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef _CUI_CERTPATH_HXX
#define _CUI_CERTPATH_HXX

#include <sfx2/basedlgs.hxx>
#include <svx/simptabl.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include "radiobtnbox.hxx"

class CertPathDialog : public ModalDialog
{
private:
    FixedLine       m_aCertPathFL;
    FixedText       m_aCertPathFT;
    SvxSimpleTableContainer m_aCertPathListContainer;
    svx::SvxRadioButtonListBox m_aCertPathList;
    PushButton      m_aAddBtn;
    FixedLine       m_aButtonsFL;
    OKButton        m_aOKBtn;
    CancelButton    m_aCancelBtn;
    HelpButton      m_aHelpBtn;
    rtl::OUString   m_sAddDialogText;
    rtl::OUString    m_sManual;

    DECL_LINK(CheckHdl_Impl, SvxSimpleTable *);
    DECL_LINK(AddHdl_Impl, void *);
    DECL_LINK(OKHdl_Impl, void *);

    void HandleCheckEntry(SvTreeListEntry* _pEntry);
    void AddCertPath(const rtl::OUString &rProfile, const rtl::OUString &rPath);
public:
    CertPathDialog(Window* pParent);
    ~CertPathDialog();

    rtl::OUString getDirectory() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
