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
#ifndef INCLUDED_CUI_SOURCE_INC_DLGNAME_HXX
#define INCLUDED_CUI_SOURCE_INC_DLGNAME_HXX


#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/vclmedit.hxx>

/// Dialog for editing a name
class SvxNameDialog : public ModalDialog
{
private:
    VclPtr<FixedText>      pFtDescription;
    VclPtr<Edit>           pEdtName;
    VclPtr<OKButton>       pBtnOK;

    Link<SvxNameDialog&,bool> aCheckNameHdl;

    DECL_LINK_TYPED(ModifyHdl, Edit&, void);

public:
    SvxNameDialog( vcl::Window* pWindow, const OUString& rName, const OUString& rDesc );
    virtual ~SvxNameDialog();
    virtual void dispose() override;

    void    GetName( OUString& rName ){rName = pEdtName->GetText();}

    /** add a callback Link that is called whenever the content of the edit
        field is changed.  The Link result determines whether the OK
        Button is enabled (> 0) or disabled (== 0).

        @param rLink a Callback declared with DECL_LINK_TYPED and implemented with
               IMPL_LINK, that is executed on modification.

        @param bCheckImmediately If true, the Link is called directly after
               setting it. It is recommended to set this flag to true to avoid
               an inconsistent state if the initial String (given in the CTOR)
               does not satisfy the check condition.

        @todo Remove the parameter bCheckImmediately and incorporate the 'true'
              behaviour as default.
     */
    void    SetCheckNameHdl( const Link<SvxNameDialog&,bool>& rLink, bool bCheckImmediately = false )
    {
        aCheckNameHdl = rLink;
        if ( bCheckImmediately )
            pBtnOK->Enable( rLink.Call( *this ) );
    }

    void    SetEditHelpId( const OString& aHelpId) {pEdtName->SetHelpId(aHelpId);}
};

/** #i68101#
    Dialog for editing Object name
    plus uniqueness-callback-linkHandler */
class SvxObjectNameDialog : public ModalDialog
{
private:
    // name
    VclPtr<Edit>           pEdtName;

    // buttons
    VclPtr<OKButton>       pBtnOK;

    // callback link for name uniqueness
    Link<SvxObjectNameDialog&,bool> aCheckNameHdl;

    DECL_LINK_TYPED(ModifyHdl, Edit&, void);

public:
    // constructor
    SvxObjectNameDialog(vcl::Window* pWindow, const OUString& rName);
    virtual ~SvxObjectNameDialog();
    virtual void dispose() override;

    // data access
    void GetName(OUString& rName) {rName = pEdtName->GetText(); }

    // set handler
    void SetCheckNameHdl(const Link<SvxObjectNameDialog&,bool>& rLink)
    {
        aCheckNameHdl = rLink;
    }
};

/** #i68101#
    Dialog for editing Object Title and Description */
class SvxObjectTitleDescDialog : public ModalDialog
{
private:
    // title
    VclPtr<Edit>           pEdtTitle;

    // description
    VclPtr<VclMultiLineEdit>  pEdtDescription;

public:
    // constructor
    SvxObjectTitleDescDialog(vcl::Window* pWindow, const OUString& rTitle, const OUString& rDesc);
    virtual ~SvxObjectTitleDescDialog();
    virtual void dispose() override;
    // data access
    void GetTitle(OUString& rTitle) {rTitle = pEdtTitle->GetText(); }
    void GetDescription(OUString& rDescription) {rDescription = pEdtDescription->GetText(); }
};

enum class SvxMessDialogButton {
    N1, N2
};

/// Dialog to cancel, save, or add
class SvxMessDialog : public ModalDialog
{
private:
    VclPtr<FixedText>      pFtDescription;
    VclPtr<PushButton>     pBtn1;
    VclPtr<PushButton>     pBtn2;
    VclPtr<FixedImage>     pFtImage;
    Image*          pImage;

    DECL_LINK_TYPED(Button1Hdl, Button*, void);
    DECL_LINK_TYPED(Button2Hdl, Button*, void);

public:
    SvxMessDialog( vcl::Window* pWindow, const OUString& rText, const OUString& rDesc, Image* pImg = nullptr );
    virtual ~SvxMessDialog();
    virtual void dispose() override;
    void DisableButton( SvxMessDialogButton nBtnId);

    void    SetButtonText( SvxMessDialogButton nBtnId, const OUString& rNewTxt );
};


#endif // INCLUDED_CUI_SOURCE_INC_DLGNAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
