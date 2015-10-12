/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_CERTPATH_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_CERTPATH_HXX

#include <sfx2/basedlgs.hxx>
#include <svtools/simptabl.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include "radiobtnbox.hxx"

class CertPathDialog : public ModalDialog
{
private:
    VclPtr<SvSimpleTableContainer> m_pCertPathListContainer;
    VclPtr<svx::SvxRadioButtonListBox> m_pCertPathList;
    VclPtr<PushButton> m_pAddBtn;
    VclPtr<OKButton>   m_pOKBtn;
    OUString    m_sAddDialogText;
    OUString    m_sManual;

    DECL_LINK_TYPED(CheckHdl_Impl, SvTreeListBox*, void);
    DECL_LINK_TYPED(AddHdl_Impl, Button*, void);
    DECL_LINK_TYPED(OKHdl_Impl, Button*, void);

    void HandleCheckEntry(SvTreeListEntry* _pEntry);
    void AddCertPath(const OUString &rProfile, const OUString &rPath);
public:
    explicit CertPathDialog(vcl::Window* pParent);
    virtual ~CertPathDialog();
    virtual void dispose() override;

    OUString getDirectory() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
