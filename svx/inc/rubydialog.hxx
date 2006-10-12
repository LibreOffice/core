/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rubydialog.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 11:48:19 $
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


#ifndef _SVX_RUBYDLG_HXX_
#define _SVX_RUBYDLG_HXX_

#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
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
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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

    SvxRubyChildWindow( Window*, USHORT, SfxBindings*, SfxChildWinInfo* );

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

    BOOL                bModified;

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
    virtual BOOL        Close();

    long                GetLastPos() const {return nLastPos;}
    void                SetLastPos(long nSet) {nLastPos = nSet;}

    BOOL                IsModified() const {return bModified;}
    void                SetModified(BOOL bSet) {bModified = bSet;}

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


