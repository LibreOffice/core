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

// #i68101#
#include <svtools/svmedit.hxx>

/// Dialog for editing a name
class SvxNameDialog : public ModalDialog
{
private:
    FixedText       aFtDescription;
    Edit            aEdtName;
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    Link            aCheckNameHdl;
#if _SOLAR__PRIVATE
    DECL_LINK(ModifyHdl, void *);
#endif

public:
    SvxNameDialog( Window* pWindow, const String& rName, const String& rDesc );

    void    GetName( String& rName ){rName = aEdtName.GetText();}

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
            aBtnOK.Enable( rLink.Call( this ) > 0 );
    }

    void    SetEditHelpId( const rtl::OString& aHelpId) {aEdtName.SetHelpId(aHelpId);}
};

/** #i68101#
    Dialog for editing Object Title and Description
    plus uniqueness-callback-linkHandler */
class SvxObjectNameDialog : public ModalDialog
{
private:
    // name
    FixedText       aFtName;
    Edit            aEdtName;

    // separator
    FixedLine       aFlSeparator;

    // buttons
    HelpButton      aBtnHelp;
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;

    // callback link for name uniqueness
    Link            aCheckNameHdl;
#if _SOLAR__PRIVATE
    DECL_LINK(ModifyHdl, void *);
#endif

public:
    // constructor
    SvxObjectNameDialog(Window* pWindow, const String& rName);

    // data access
    void GetName(String& rName) {rName = aEdtName.GetText(); }

    // set handler
    void SetCheckNameHdl(const Link& rLink, bool bCheckImmediately = false)
    {
        aCheckNameHdl = rLink;

        if(bCheckImmediately)
        {
            aBtnOK.Enable(rLink.Call(this) > 0);
        }
    }
};

/** #i68101#
    Dialog for editing Object Title and Description */
class SvxObjectTitleDescDialog : public ModalDialog
{
private:
    // title
    FixedText       aFtTitle;
    Edit            aEdtTitle;

    // description
    FixedText       aFtDescription;
    MultiLineEdit   aEdtDescription;

    // separator
    FixedLine       aFlSeparator;

    // buttons
    HelpButton      aBtnHelp;
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;

public:
    // constructor
    SvxObjectTitleDescDialog(Window* pWindow, const String& rTitle, const String& rDesc);

    // data access
    void GetTitle(String& rTitle) {rTitle = aEdtTitle.GetText(); }
    void GetDescription(String& rDescription) {rDescription = aEdtDescription.GetText(); }
};

/// Dialog to cancel, save, or add
class SvxMessDialog : public ModalDialog
{
private:
    FixedText       aFtDescription;
    PushButton      aBtn1;
    PushButton      aBtn2;
    CancelButton    aBtnCancel;
    FixedImage      aFtImage;
    Image*          pImage;
#if _SOLAR__PRIVATE
    DECL_LINK(Button1Hdl, void *);
    DECL_LINK(Button2Hdl, void *);
#endif
public:
    SvxMessDialog( Window* pWindow, const String& rText, const String& rDesc, Image* pImg = NULL );
    ~SvxMessDialog();

    void    SetButtonText( sal_uInt16 nBtnId, const String& rNewTxt );
};



#endif // _SVX_DLG_NAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
