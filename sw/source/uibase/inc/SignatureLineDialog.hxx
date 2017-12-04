/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_INC_SIGNATURELINEDIALOG_HXX
#define INCLUDED_SW_INC_SIGNATURELINEDIALOG_HXX

#include <svx/stddlg.hxx>
#include <swdllapi.h>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>

class SwView;

class SW_DLLPUBLIC SignatureLineDialog : public SvxStandardDialog
{
public:
    SignatureLineDialog(vcl::Window* pParent, SwView& rView);
    virtual ~SignatureLineDialog() override;
    virtual void dispose() override;

    virtual void Apply() override;

private:
    OUString getSignatureImage();

    SwView& mrView;

    VclPtr<Edit> m_pEditName;
    VclPtr<Edit> m_pEditTitle;
    VclPtr<Edit> m_pEditEmail;
    VclPtr<Edit> m_pEditInstructions;
    VclPtr<CheckBox> m_pCheckboxCanAddComments;
    VclPtr<CheckBox> m_pCheckboxShowSignDate;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
