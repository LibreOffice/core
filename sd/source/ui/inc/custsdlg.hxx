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


#ifndef _SD_CUSTSDLG_HXX
#define _SD_CUSTSDLG_HXX

#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <svtools/treelistbox.hxx>
#include <vcl/dialog.hxx>

class SdDrawDocument;
class SdCustomShow;
class SdCustomShowList;

//------------------------------------------------------------------------

class SdCustomShowDlg : public ModalDialog
{
private:
    ListBox         aLbCustomShows;
    CheckBox        aCbxUseCustomShow;
    PushButton      aBtnNew;
    PushButton      aBtnEdit;
    PushButton      aBtnRemove;
    PushButton      aBtnCopy;
    HelpButton      aBtnHelp;
    PushButton      aBtnStartShow;
    OKButton        aBtnOK;

    SdDrawDocument& rDoc;
    SdCustomShowList* pCustomShowList;
    SdCustomShow*   pCustomShow;
    sal_Bool            bModified;

    void            CheckState();

    DECL_LINK( ClickButtonHdl, void * );
    DECL_LINK( StartShowHdl, void* );

public:
                SdCustomShowDlg( Window* pWindow, SdDrawDocument& rDrawDoc );
                ~SdCustomShowDlg();

    sal_Bool        IsModified() const { return( bModified ); }
    sal_Bool        IsCustomShow() const;
};


//------------------------------------------------------------------------

class SdDefineCustomShowDlg : public ModalDialog
{
private:
    FixedText       aFtName;
    Edit            aEdtName;
    FixedText       aFtPages;
    MultiListBox    aLbPages;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    FixedText       aFtCustomPages;
    SvTreeListBox   aLbCustomPages;
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    SdDrawDocument& rDoc;
    SdCustomShow*&  rpCustomShow;
    sal_Bool            bModified;
    String          aOldName;

    void            CheckState();
    void            CheckCustomShow();

    DECL_LINK( ClickButtonHdl, void * );
    DECL_LINK( OKHdl, void * );

public:

                    SdDefineCustomShowDlg( Window* pWindow,
                            SdDrawDocument& rDrawDoc, SdCustomShow*& rpCS );
                    ~SdDefineCustomShowDlg();

    sal_Bool            IsModified() const { return( bModified ); }
};

#endif // _SD_CUSTSDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
