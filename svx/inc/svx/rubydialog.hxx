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


#ifndef _SVX_RUBYDLG_HXX_
#define _SVX_RUBYDLG_HXX_

#include <sfx2/childwin.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/lstbox.hxx>
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/edit.hxx>
#include <vcl/scrbar.hxx>
#include <com/sun/star/uno/Reference.h>
#include "svx/svxdllapi.h"

namespace com{namespace sun{namespace star{
    namespace view{
        class XSelectionChangeListener;
    }
}}}


class SvxRubyDialog;
class RubyPreview : public Window
{
protected:
    virtual void Paint( const Rectangle& rRect );
    SvxRubyDialog&  rParentDlg;

public:
    RubyPreview(SvxRubyDialog& rParent, const ResId& rResId);
};

class SVX_DLLPUBLIC SvxRubyChildWindow : public SfxChildWindow
{
 public:

    SvxRubyChildWindow( Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo* );

    SFX_DECL_CHILDWINDOW( SvxRubyChildWindow );

};
class SvxRubyData_Impl;
class RubyEdit  : public Edit
{
    Link    aScrollHdl;
    Link    aJumpHdl;
    virtual void        GetFocus();
    virtual long        PreNotify( NotifyEvent& rNEvt );
public:
    RubyEdit(Window* pParent, const ResId& rResId) :
        Edit(pParent, rResId){}
    void    SetScrollHdl(Link& rLink) {aScrollHdl = rLink;}
    void    SetJumpHdl(Link& rLink) {aJumpHdl = rLink;}
};


class SvxRubyDialog : public SfxModelessDialog
{
    using Window::SetText;
    using Window::GetText;

    friend class RubyPreview;

    FixedText               aLeftFT;
    RubyEdit                aLeft1ED;
    FixedText               aRightFT;
    RubyEdit                aRight1ED;
    RubyEdit                aLeft2ED;
    RubyEdit                aRight2ED;
    RubyEdit                aLeft3ED;
    RubyEdit                aRight3ED;
    RubyEdit                aLeft4ED;
    RubyEdit                aRight4ED;

    RubyEdit*               aEditArr[8];
    ScrollBar           aScrollSB;

    CheckBox            aAutoDetectionCB;

    FixedText           aAdjustFT;
    ListBox             aAdjustLB;

    FixedText           aPositionFT;
    ListBox             aPositionLB;

    FixedText           aCharStyleFT;
    ListBox             aCharStyleLB;
    PushButton          aStylistPB;

    FixedText           aPreviewFT;
    RubyPreview         aPreviewWin;

    OKButton            aApplyPB;
    PushButton          aClosePB;
    HelpButton          aHelpPB;

    long                nLastPos;
    long                nCurrentEdit;

    sal_Bool                bModified;

    com::sun::star::uno::Reference<com::sun::star::view::XSelectionChangeListener> xImpl;
    SfxBindings*    pBindings;
    SvxRubyData_Impl* pImpl;

    DECL_LINK(ApplyHdl_Impl, PushButton*);
    DECL_LINK(CloseHdl_Impl, PushButton*);
    DECL_LINK(StylistHdl_Impl, PushButton*);
    DECL_LINK(AutomaticHdl_Impl, CheckBox*);
    DECL_LINK(ScrollHdl_Impl, ScrollBar*);
    DECL_LINK(PositionHdl_Impl, ListBox*);
    DECL_LINK(AdjustHdl_Impl, ListBox*);
    DECL_LINK(CharStyleHdl_Impl, ListBox*);
    DECL_LINK(EditModifyHdl_Impl, Edit*);
    DECL_LINK(EditScrollHdl_Impl, sal_Int32*);
    DECL_LINK(EditJumpHdl_Impl, sal_Int32*);

    void                SetText(sal_Int32 nPos, Edit& rLeft, Edit& rRight);
    void                GetText();
    void                ClearCharStyleList();
    void                AssertOneEntry();

    void                Update();
    virtual sal_Bool        Close();

    long                GetLastPos() const {return nLastPos;}
    void                SetLastPos(long nSet) {nLastPos = nSet;}

    sal_Bool                IsModified() const {return bModified;}
    void                SetModified(sal_Bool bSet) {bModified = bSet;}

    void EnableControls(sal_Bool bEnable)
        {
            aLeftFT.Enable(bEnable);
            aRightFT.Enable(bEnable);
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

    void                GetCurrentText(String& rBase, String& rRuby);

    void                UpdateColors( void );
protected:
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
public:

                        SvxRubyDialog( SfxBindings *pBindings, SfxChildWindow *pCW,
                                    Window* pParent, const ResId& rResId );
    virtual             ~SvxRubyDialog();

    virtual void        Activate();
    virtual void        Deactivate();
};

#endif // _SVX_RUBYDLG_HXX_


