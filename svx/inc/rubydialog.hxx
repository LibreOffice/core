/*************************************************************************
 *
 *  $RCSfile: rubydialog.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2001-02-16 14:46:51 $
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


#ifndef _SVX_RUBYDLG_HXX_
#define _SVX_RUBYDLG_HXX_

#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _HEADBAR_HXX
#include <svtools/headbar.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif


class SvxRubyDialog;
class RubyPreview : public Window
{
protected:
    virtual void Paint( const Rectangle& rRect );
    SvxRubyDialog&  rParentDlg;

public:
    RubyPreview(SvxRubyDialog& rParent, const ResId& rResId);
};

class SvxRubyChildWindow : public SfxChildWindow
{
 public:

    SvxRubyChildWindow( Window*, USHORT, SfxBindings*, SfxChildWinInfo* );

    SFX_DECL_CHILDWINDOW( SvxRubyChildWindow );

};
struct SvxRubyData_Impl;
class RubyEdit  : public Edit
{
    virtual void        GetFocus();
public:
    RubyEdit(Window* pParent, const ResId& rResId) :
        Edit(pParent, rResId){}
};


class SvxRubyDialog : public SfxModelessDialog
{
    friend class RubyPreview;

    HeaderBar           aHeaderHB;
    RubyEdit                aLeft1ED;
    RubyEdit                aRight1ED;
    RubyEdit                aLeft2ED;
    RubyEdit                aRight2ED;
    RubyEdit                aLeft3ED;
    RubyEdit                aRight3ED;
    RubyEdit                aLeft4ED;
    RubyEdit                aRight4ED;

    Edit*               aEditArr[8];
    ScrollBar           aScrollSB;

    CheckBox            aAutoDetectionCB;

    FixedText           aAdjustFT;
    ListBox             aAdjustLB;

    FixedText           aCharStyleFT;
    ListBox             aCharStyleLB;
    PushButton          aStylistPB;

    FixedText           aPreviewFT;
    RubyPreview         aPreviewWin;

    OKButton            aApplyPB;
    PushButton          aClosePB;
    HelpButton          aHelpPB;

    String              sBaseText;
    String              sRubyText;

    long                nLastPos;
    long                nCurrentEdit;
    BOOL                bModified;

    SfxBindings*    pBindings;
    SvxRubyData_Impl* pImpl;

    DECL_LINK(ApplyHdl_Impl, PushButton*);
    DECL_LINK(CloseHdl_Impl, PushButton*);
    DECL_LINK(StylistHdl_Impl, PushButton*);
    DECL_LINK(DragHdl_Impl, HeaderBar*);
    DECL_LINK(AutomaticHdl_Impl, CheckBox*);
    DECL_LINK(ScrollHdl_Impl, ScrollBar*);
    DECL_LINK(AdjustHdl_Impl, ListBox*);
    DECL_LINK(CharStyleHdl_Impl, ListBox*);
    DECL_LINK(EditModifyHdl_Impl, Edit*);

    void                SetText(sal_Int32 nPos, Edit& rLeft, Edit& rRight);
    void                GetText();
    void                ClearCharStyleList();

    void                Update();
    virtual BOOL        Close();

    long                GetLastPos() const {return nLastPos;}
    void                SetLastPos(long nSet) {nLastPos = nSet;}

    BOOL                IsModified() const {return bModified;}
    void                SetModified(BOOL bSet) {bModified = bSet;}

    void EnableControls(sal_Bool bEnable)
        {
            aHeaderHB.Enable(bEnable);
            aLeft1ED.Enable(bEnable);
            aRight1ED.Enable(bEnable);
            aLeft2ED.Enable(bEnable);
            aRight2ED.Enable(bEnable);
            aLeft3ED.Enable(bEnable);
            aRight3ED.Enable(bEnable);
            aLeft4ED.Enable(bEnable);
            aRight4ED.Enable(bEnable);
            aScrollSB.Enable(bEnable);
            aAutoDetectionCB.Enable(bEnable);
            aAdjustFT.Enable(bEnable);
            aAdjustLB.Enable(bEnable);
            aCharStyleFT.Enable(bEnable);
            aCharStyleLB.Enable(bEnable);
            aStylistPB.Enable(bEnable);
            aPreviewFT.Enable(bEnable);
            aPreviewWin.Enable(bEnable);
            aApplyPB.Enable(bEnable);
        }

        void    GetCurrentText(String& rBase, String& rRuby);
public:

                        SvxRubyDialog( SfxBindings *pBindings, SfxChildWindow *pCW,
                                    Window* pParent, const ResId& rResId );
                        ~SvxRubyDialog();

    virtual void        Activate();
    virtual void        Deactivate();
};

#endif // _SVX_RUBYDLG_HXX_


