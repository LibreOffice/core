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

#ifndef INCLUDED_SW_INC_SIDEBARWIN_HXX
#define INCLUDED_SW_INC_SIDEBARWIN_HXX

#include <postithelper.hxx>
#include <SidebarWindowsTypes.hxx>

#include <vcl/window.hxx>
#include <swrect.hxx>

#include <tools/date.hxx>

#include <vcl/lineinfo.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/range/b2drange.hxx>
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
class SwFrame;

namespace sw { namespace overlay {
    class OverlayRanges;
}}

namespace sw { namespace sidebarwindows {

class SidebarTextControl;
class AnchorOverlayObject;
class ShadowOverlayObject;

typedef sal_Int64 SwPostItBits;

#define PB_Preview ((SwPostItBits)0x00000001)

class SwSidebarWin : public vcl::Window
{
    public:
        SwSidebarWin( SwEditWin& rEditWin,
                      WinBits nBits,
                      SwPostItMgr& aMgr,
                      SwPostItBits aBits,
                      SwSidebarItem& rSidebarItem );
        virtual ~SwSidebarWin() override;
        virtual void dispose() override;

        void SetSize( const Size& rNewSize );
        void SetPosSizePixelRect( long nX,
                                  long nY,
                                  long nWidth,
                                  long nHeight,
                                  const SwRect& aAnchorRect,
                                  const long PageBorder);
        void SetPosAndSize();
        void TranslateTopPosition(const long aAmount);
        void CheckMetaText();

        inline Point GetAnchorPos() { return mAnchorRect.Pos(); }
        inline const SwRect& GetAnchorRect() { return mAnchorRect; }
        inline const std::vector<basegfx::B2DRange>& GetAnnotationTextRanges() { return maAnnotationTextRanges; }
        SwEditWin& EditWin();
        inline SwSidebarItem& GetSidebarItem() { return mrSidebarItem; }

        inline OutlinerView* GetOutlinerView() { return mpOutlinerView;}
        bool HasScrollbar() const;
        bool IsScrollbarVisible() const;
        inline ScrollBar* Scrollbar() { return mpVScrollbar; }
        inline ::sw::sidebarwindows::AnchorOverlayObject* Anchor() { return mpAnchor;}
        inline ::sw::sidebarwindows::ShadowOverlayObject* Shadow() { return mpShadow;}
        inline ::sw::overlay::OverlayRanges* TextRange() { return mpTextRangeOverlay;}

        long            GetPostItTextHeight();

        void            SwitchToPostIt(sal_uInt16 aDirection);
        void    SwitchToFieldPos();

        virtual sal_uInt32 MoveCaret() = 0;

        virtual void    UpdateData() = 0;
        virtual void    SetPostItText() = 0;
        virtual void    Delete();
        virtual void    GotoPos() = 0;

        virtual OUString GetAuthor() = 0;
        virtual Date    GetDate() = 0;
        virtual tools::Time GetTime() = 0;

        void            ExecuteCommand(sal_uInt16 nSlot);
        void            InitControls();
        void            DoResize();
        void            ResizeIfNecessary(long aOldHeight, long aNewHeight);
        void            SetScrollbar();

        void            SetVirtualPosSize( const Point& aPoint, const Size& aSize);
        const Point     VirtualPos()    { return mPosSize.TopLeft(); }
        const Size      VirtualSize()   { return mPosSize.GetSize(); }

        void            ShowAnchorOnly(const Point &aPoint);
        void            ShowNote();
        void            HideNote();
        void            InvalidateControl();

        void            ResetAttributes();

        void            SetSidebarPosition(sw::sidebarwindows::SidebarPosition eSidebarPosition);
        void            SetReadonly(bool bSet);
        bool            IsReadOnly()
        {
            return mbReadonly;
        }
        bool            IsPreview()
        {
            return nFlags & PB_Preview;
        }

        void         SetColor(Color aColorDark,Color aColorLight, Color aColorAnchor);
        const Color& ColorAnchor() { return mColorAnchor; }
        const Color& ColorDark() { return mColorDark; }
        const Color& ColorLight() { return mColorLight; }
        void         Rescale();

        void            SetViewState(::sw::sidebarwindows::ViewState bViewState);

        bool            IsFollow() { return mbIsFollow; }
        void            SetFollow( bool bIsFollow) { mbIsFollow = bIsFollow; };
        virtual sal_uInt32    CalcParent() = 0;

        sal_Int32   GetMetaHeight();
        sal_Int32   GetMinimumSizeWithMeta();
        sal_Int32   GetMinimumSizeWithoutMeta();
        sal_Int32   GetMetaButtonAreaWidth();
        sal_Int32   GetScrollbarWidth();

        void    SetSpellChecking();

        void    ToggleInsMode();

        void    ActivatePostIt();
        void    DeactivatePostIt();

        void SetChangeTracking( const SwPostItHelper::SwLayoutStatus aStatus,
                                const Color& aColor);
        SwPostItHelper::SwLayoutStatus GetLayoutStatus() { return mLayoutStatus; }
        const Color& GetChangeColor() { return mChangeColor; }

        virtual bool IsProtected()
        {
            return mbReadonly;
        }

        DECL_LINK( WindowEventListener, VclWindowEvent&, void );
        inline bool IsMouseOverSidebarWin() const { return mbMouseOver; }

        void SetLanguage(const SvxLanguageItem& rNewItem);

        void ChangeSidebarItem( SwSidebarItem& rSidebarItem );
        virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

        virtual void    Draw(OutputDevice* pDev, const Point&, const Size&, DrawFlags) override;
        virtual void KeyInput(const KeyEvent& rKeyEvt) override;
        virtual void MouseButtonDown(const MouseEvent& rMouseEvent) override;
        virtual void MouseButtonUp(const MouseEvent& rMouseEvent) override;
        virtual void MouseMove(const MouseEvent& rMouseEvent) override;
        void PaintTile(vcl::RenderContext& rRenderContext, const Rectangle& rRect);
        /// Is there a matching sub-widget inside this sidebar widget for rPointLogic?
        bool IsHitWindow(const Point& rPointLogic);
        /// Allows adjusting the point or mark of the selection to a document coordinate.
        void SetCursorLogicPosition(const Point& rPosition, bool bPoint, bool bClearMark);

    protected:
        virtual void    LoseFocus() override;
        virtual void    Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;
        virtual void    GetFocus() override;
        virtual VclPtr<MenuButton> CreateMenuButton() = 0;

        void        SetSizePixel( const Size& rNewSize ) override;
        SfxItemSet  DefaultItem();

        DECL_LINK(ModifyHdl, LinkParamNone*, void);
        DECL_LINK(ScrollHdl, ScrollBar*, void);
        DECL_LINK(DeleteHdl, void*, void);

        inline SwView& DocView() { return mrView;}
        inline SwPostItMgr& Mgr() { return mrMgr; }
        inline Outliner* Engine() { return mpOutliner;}

    private:
        SwSidebarWin*   GetTopReplyNote();

        virtual SvxLanguageItem GetLanguage();

        SwPostItMgr&    mrMgr;
        SwView&         mrView;
        const SwPostItBits nFlags;

        ImplSVEvent *   mnEventId;

        OutlinerView*   mpOutlinerView;
        Outliner*       mpOutliner;

        VclPtr<sw::sidebarwindows::SidebarTextControl> mpSidebarTextControl;
        VclPtr<ScrollBar>      mpVScrollbar;
        VclPtr<Edit>           mpMetadataAuthor;
        VclPtr<Edit>           mpMetadataDate;
        VclPtr<MenuButton>     mpMenuButton;

        sw::sidebarwindows::AnchorOverlayObject* mpAnchor;
        sw::sidebarwindows::ShadowOverlayObject* mpShadow;
        sw::overlay::OverlayRanges* mpTextRangeOverlay;

        Color           mColorAnchor;
        Color           mColorDark;
        Color           mColorLight;
        Color           mChangeColor;

        sw::sidebarwindows::SidebarPosition meSidebarPosition;

        Rectangle       mPosSize;
        SwRect          mAnchorRect;
        long            mPageBorder;

        std::vector<basegfx::B2DRange> maAnnotationTextRanges;

        bool            mbMouseOver;
        SwPostItHelper::SwLayoutStatus mLayoutStatus;

        bool            mbReadonly;
        bool            mbIsFollow;

        SwSidebarItem& mrSidebarItem;
        const SwFrame* mpAnchorFrame;
};

} } // eof namespace sw::sidebarwindows

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
