/*************************************************************************
 *
 *  $RCSfile: custsdlg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

