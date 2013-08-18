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

#ifndef _SIDEBARWIN_HXX
#define _SIDEBARWIN_HXX

#include <postithelper.hxx>
#include <SidebarWindowsTypes.hxx>

#include <vcl/window.hxx>
#include <swrect.hxx>

#include <tools/date.hxx>

#include <vcl/lineinfo.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <editeng/editstat.hxx>

class SwPostItMgr;
class SwPostItField;
class OutlinerView;
class Outliner;
class ScrollBar;
class SwEditWin;
class SwView;
class Edit;
class MenuButton;
class SwFrm;


namespace sw { namespace sidebarwindows {

class SidebarTxtControl;
class AnchorOverlayObject;
class ShadowOverlayObject;

typedef sal_Int64 SwPostItBits;

#define PB_Preview ((SwPostItBits)0x00000001)


class SwSidebarWin : public Window
{
    public:
        SwSidebarWin( SwEditWin& rEditWin,
                      WinBits nBits,
                      SwPostItMgr& aMgr,
                      SwPostItBits aBits,
                      SwSidebarItem& rSidebarItem );
        virtual ~SwSidebarWin();

        void SetSize( const Size& rNewSize );
        void SetPosSizePixelRect( long nX,
                                  long nY,
                                  long nWidth,
                                  long nHeight,
                                  const SwRect &aRect,
                                  const long PageBorder);
        void SetPosAndSize();
        void TranslateTopPosition(const long aAmount);
        virtual void CheckMetaText();

        inline Point GetAnchorPos() { return mAnchorRect.Pos(); }
        SwEditWin* EditWin();

        inline OutlinerView* GetOutlinerView() { return mpOutlinerView;}
        bool HasScrollbar() const;
        bool IsScrollbarVisible() const;
        inline ScrollBar* Scrollbar() { return mpVScrollbar; }
        inline ::sw::sidebarwindows::AnchorOverlayObject* Anchor() { return mpAnchor;}
        inline ::sw::sidebarwindows::ShadowOverlayObject* Shadow() { return mpShadow;}

        long            GetPostItTextHeight();

        void            SwitchToPostIt(sal_uInt16 aDirection);
        virtual void    SwitchToFieldPos();

        virtual sal_uInt32 MoveCaret() = 0;

        virtual void    UpdateData() = 0;
        virtual void    SetPostItText() = 0;
        virtual void    Delete();
        virtual void    GotoPos() = 0;

        virtual OUString GetAuthor() = 0;
        virtual Date    GetDate() = 0;
        virtual Time    GetTime() = 0;

        void            ExecuteCommand(sal_uInt16 nSlot);
        void            InitControls();
        void            HidePostIt();
        void            DoResize();
        void            ResizeIfNeccessary(long aOldHeight, long aNewHeight);
        void            SetScrollbar();

        void            SetVirtualPosSize( const Point& aPoint, const Size& aSize);
        const Point     VirtualPos()    { return mPosSize.TopLeft(); }
        const Size      VirtualSize()   { return mPosSize.GetSize(); }

        void            ShowAnchorOnly(const Point &aPoint);
        void            ShowNote();
        void            HideNote();

        void            ResetAttributes();

        void            SetSidebarPosition(sw::sidebarwindows::SidebarPosition eSidebarPosition);
        void            SetReadonly(sal_Bool bSet);
        sal_Bool            IsReadOnly()        { return mbReadonly;}
        bool            IsPreview()         { return nFlags & PB_Preview;}

        void            SetColor(Color aColorDark,Color aColorLight, Color aColorAnchor);
        const Color& ColorAnchor() { return mColorAnchor; }
        const Color& ColorDark() { return mColorDark; }
        const Color& ColorLight() { return mColorLight; }
        void            Rescale();

        void            SetViewState(::sw::sidebarwindows::ViewState bViewState);

        bool            IsFollow() { return mbIsFollow; }
        void            SetFollow( bool bIsFollow) { mbIsFollow = bIsFollow; };
        virtual bool    CalcFollow() = 0;

        sal_Int32       GetMetaHeight();
        sal_Int32       GetMinimumSizeWithMeta();
        sal_Int32       GetMinimumSizeWithoutMeta();
        sal_Int32       GetMetaButtonAreaWidth();
        sal_Int32       GetScrollbarWidth();

        void            SetSpellChecking();

        void            ToggleInsMode();

        virtual void    ActivatePostIt();
        virtual void    DeactivatePostIt();

        void SetChangeTracking( const SwPostItHelper::SwLayoutStatus aStatus,
                                const Color& aColor);
        SwPostItHelper::SwLayoutStatus GetLayoutStatus() { return mLayoutStatus; }
        Color GetChangeColor() { return mChangeColor; }

        virtual bool    IsProtected() {return mbReadonly;};

        DECL_LINK( WindowEventListener, VclSimpleEvent* );
        inline bool IsMouseOverSidebarWin() const { return mbMouseOver; }

        void SetLanguage(const SvxLanguageItem aNewItem);

        void ChangeSidebarItem( SwSidebarItem& rSidebarItem );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    protected:
        virtual void    DataChanged( const DataChangedEvent& aEvent);
        virtual void    LoseFocus();
        virtual void    Paint( const Rectangle& rRect);
        virtual void    GetFocus();
        virtual MenuButton* CreateMenuButton() = 0;

        void        SetSizePixel( const Size& rNewSize );
        SfxItemSet  DefaultItem();

        DECL_LINK(ModifyHdl, void*);
        DECL_LINK(ScrollHdl, ScrollBar*);
        DECL_LINK(DeleteHdl, void*);

        inline SwView& DocView() { return mrView;}
        inline SwPostItMgr& Mgr() { return mrMgr; }
        inline Outliner* Engine() { return mpOutliner;}

    private:
        SwSidebarWin*   GetTopReplyNote();

        virtual SvxLanguageItem GetLanguage(void);

        SwPostItMgr&    mrMgr;
        SwView&         mrView;
        const SwPostItBits nFlags;

        sal_uLong           mnEventId;

        OutlinerView*   mpOutlinerView;
        Outliner*       mpOutliner;

        sw::sidebarwindows::SidebarTxtControl* mpSidebarTxtControl;
        ScrollBar*      mpVScrollbar;
        Edit*           mpMetadataAuthor;
        Edit*           mpMetadataDate;
        MenuButton*     mpMenuButton;

        sw::sidebarwindows::AnchorOverlayObject* mpAnchor;
        sw::sidebarwindows::ShadowOverlayObject* mpShadow;

        Color           mColorAnchor;
        Color           mColorDark;
        Color           mColorLight;
        Color           mChangeColor;

        sw::sidebarwindows::SidebarPosition meSidebarPosition;

        Rectangle       mPosSize;
        SwRect          mAnchorRect;
        long            mPageBorder;

        bool            mbMouseOver;
        SwPostItHelper::SwLayoutStatus mLayoutStatus;

        bool            mbReadonly;
        bool            mbIsFollow;

        SwSidebarItem& mrSidebarItem;
        const SwFrm* mpAnchorFrm;
};

} } // eof namespace sw::sidebarwindows

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
