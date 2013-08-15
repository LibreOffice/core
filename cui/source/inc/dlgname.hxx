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
#ifndef _SVX_DLG_NAME_HXX
#define _SVX_DLG_NAME_HXX


#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/vclmedit.hxx> // #i68101#

/// Dialog for editing a name
class SvxNameDialog : public ModalDialog
{
private:
    FixedText*      pFtDescription;
    Edit*           pEdtName;
    OKButton*       pBtnOK;

    Link            aCheckNameHdl;

    DECL_LINK(ModifyHdl, void *);

public:
    SvxNameDialog( Window* pWindow, const String& rName, const String& rDesc );

    void    GetName( OUString& rName ){rName = pEdtName->GetText();}

    /** add a callback Link that is called whenever the content of the edit
        field is changed.  The Link result determines whether the OK
        Button is enabled (> 0) or disabled (== 0).

        @param rLink a Callback declared with DECL_LINK and implemented with
               IMPL_LINK, that is executed on modification.

        @param bCheckImmediately If true, the Link is called directly after
               setting it. It is recommended to set this flag to true to avoid
               an inconsistent state if the initial String (given in the CTOR)
               does not satisfy the check condition.

        @todo Remove the parameter bCheckImmediately and incorporate the 'true'
              behaviour as default.
     */
    void    SetCheckNameHdl( const Link& rLink, bool bCheckImmediately = false )
    {
        aCheckNameHdl = rLink;
        if ( bCheckImmediately )
            pBtnOK->Enable( rLink.Call( this ) > 0 );
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
    Edit*           pEdtName;

    // buttons
    OKButton*       pBtnOK;

    // callback link for name uniqueness
    Link            aCheckNameHdl;

    DECL_LINK(ModifyHdl, void *);

public:
    // constructor
    SvxObjectNameDialog(Window* pWindow, const String& rName);

    // data access
    void GetName(OUString& rName) {rName = pEdtName->GetText(); }

    // set handler
    void SetCheckNameHdl(const Link& rLink, bool bCheckImmediately = false)
    {
        aCheckNameHdl = rLink;

        if(bCheckImmediately)
        {
            pBtnOK->Enable(rLink.Call(this) > 0);
        }
    }
};

/** #i68101#
    Dialog for editing Object Title and Description */
class SvxObjectTitleDescDialog : public ModalDialog
{
private:
    // title
    Edit*           pEdtTitle;

    // description
    VclMultiLineEdit*  pEdtDescription;

public:
    // constructor
    SvxObjectTitleDescDialog(Window* pWindow, const String& rTitle, const String& rDesc);

    // data access
    void GetTitle(OUString& rTitle) {rTitle = pEdtTitle->GetText(); }
    void GetDescription(OUString& rDescription) {rDescription = pEdtDescription->GetText(); }
};

/// Dialog to cancel, save, or add
class SvxMessDialog : public ModalDialog
{
private:
    FixedText*      pFtDescription;
    PushButton*     pBtn1;
    PushButton*     pBtn2;
    FixedImage*     pFtImage;
    Image*          pImage;

    DECL_LINK(Button1Hdl, void *);
    DECL_LINK(Button2Hdl, void *);

public:
    SvxMessDialog( Window* pWindow, const String& rText, const String& rDesc, Image* pImg = NULL );
    ~SvxMessDialog();

    void    SetButtonText( sal_uInt16 nBtnId, const String& rNewTxt );
};



#endif // _SVX_DLG_NAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
