/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#ifndef INCLUDED_SFX2_SOURCE_INC_APPLICATIONSTARTUPDIALOG_HXX
#define INCLUDED_SFX2_SOURCE_INC_APPLICATIONSTARTUPDIALOG_HXX

#include <vcl/button.hxx>
#include <vcl/fixedhyper.hxx>
#include <vcl/layout.hxx>
#include <sfx2/basedlgs.hxx>

class SfxApplicationStartupDialog : public SfxModalDialog
{
private:
    VclPtr<CheckBox> mpDontShowAgainCB;
    VclPtr<PushButton>  mpOkButton;
    VclPtr<FixedHyperlink> mpLinkButton;
    VclPtr<FixedHyperlink> mpLinkButton1;
    VclPtr<FixedHyperlink> mpLinkButton2;

    DECL_LINK_TYPED(OkButtonClick, Button*, void);
    DECL_LINK_TYPED(LinkClick, FixedHyperlink&, void);
    DECL_LINK_TYPED(LinkButton1Click, FixedHyperlink&, void);
    DECL_LINK_TYPED(LinkButton2Click, FixedHyperlink&, void);

public:
    SfxApplicationStartupDialog(Window* pParent);
    virtual ~SfxApplicationStartupDialog();
    virtual void dispose() SAL_OVERRIDE;
};

#endif // INCLUDED_SFX2_SOURCE_INC_APPLICATIONENTRYDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
