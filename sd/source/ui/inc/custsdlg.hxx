/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: custsdlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:24:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef _SD_CUSTSDLG_HXX
#define _SD_CUSTSDLG_HXX

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif
#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

class SdDrawDocument;
class SdCustomShow;

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
    List*           pCustomShowList;
    SdCustomShow*   pCustomShow;
    BOOL            bModified;

    void            CheckState();

    DECL_LINK( ClickButtonHdl, void * );
    DECL_LINK( StartShowHdl, Button* );

public:
                SdCustomShowDlg( Window* pWindow, SdDrawDocument& rDrawDoc );
                ~SdCustomShowDlg();

    BOOL        IsModified() const { return( bModified ); }
    BOOL        IsCustomShow() const;
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
    BOOL            bModified;
    String          aOldName;

    void            CheckState();
    void            CheckCustomShow();

    DECL_LINK( ClickButtonHdl, void * );
    DECL_LINK( OKHdl, Button* );

public:

                    SdDefineCustomShowDlg( Window* pWindow,
                            SdDrawDocument& rDrawDoc, SdCustomShow*& rpCS );
                    ~SdDefineCustomShowDlg();

    BOOL            IsModified() const { return( bModified ); }
};

#endif // _SD_CUSTSDLG_HXX

