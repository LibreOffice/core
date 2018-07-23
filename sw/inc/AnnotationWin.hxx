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

#ifndef INCLUDED_SW_INC_ANNOTATIONWIN_HXX
#define INCLUDED_SW_INC_ANNOTATIONWIN_HXX

#include <basegfx/range/b2drange.hxx>
#include <tools/date.hxx>
#include <vcl/builder.hxx>
#include <vcl/window.hxx>

#include "postithelper.hxx"
#include "swrect.hxx"
#include "SidebarWindowsTypes.hxx"

class PopupMenu;
class OutlinerParaObject;
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
class SvxLanguageItem;

namespace sw { namespace overlay {
    class OverlayRanges;
}}

namespace sw { namespace sidebarwindows {
    class SidebarTextControl;
    class AnchorOverlayObject;
    class ShadowOverlayObject;
}}

namespace sw { namespace annotation {

class SwAnnotationWin : public vcl::Window
{
    public:
        SwAnnotationWin( SwEditWin& rEditWin,
                         WinBits nBits,
                         SwPostItMgr& aMgr,
                         SwSidebarItem& rSidebarItem,
                         SwFormatField* aField );
        virtual ~SwAnnotationWin() override;
        virtual void dispose() override;

        void    UpdateData();
        void    SetPostItText();
        void    Delete();
        void    GotoPos();
        const SwPostItField* GetPostItField() { return mpField; }
        void UpdateText(const OUString& aText);

        OUString GetAuthor();
        Date     GetDate();
        tools::Time GetTime();

        sal_uInt32 MoveCaret();

        /// Calculate parent postit id of current annotation window
        sal_uInt32 CalcParent();
        void       InitAnswer(OutlinerParaObject const * pText);

        bool IsProtected();

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

        Point const & GetAnchorPos() { return mAnchorRect.Pos(); }
        const SwRect& GetAnchorRect() { return mAnchorRect; }
        bool IsAnchorRectChanged() { return mbAnchorRectChanged; }
        void ResetAnchorRectChanged() { mbAnchorRectChanged = false; }
        const std::vector<basegfx::B2DRange>& GetAnnotationTextRanges() { return maAnnotationTextRanges; }
        SwEditWin& EditWin();
        SwSidebarItem& GetSidebarItem() { return mrSidebarItem; }

        OutlinerView* GetOutlinerView() { return mpOutlinerView.get();}
        bool HasScrollbar() const;
        bool IsScrollbarVisible() const;
        ScrollBar* Scrollbar() { return mpVScrollbar; }
        ::sw::sidebarwindows::AnchorOverlayObject* Anchor() { return mpAnchor;}
        ::sw::sidebarwindows::ShadowOverlayObject* Shadow() { return mpShadow;}
        ::sw::overlay::OverlayRanges* TextRange() { return mpTextRangeOverlay.get();}

        long            GetPostItTextHeight();

        void            SwitchToPostIt(sal_uInt16 aDirection);
        void            SwitchToFieldPos();

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

        void         SetColor(Color aColorDark,Color aColorLight, Color aColorAnchor);
        const Color& ColorAnchor() { return mColorAnchor; }
        const Color& ColorDark() { return mColorDark; }
        const Color& ColorLight() { return mColorLight; }
        void         Rescale();

        void            SetViewState(::sw::sidebarwindows::ViewState bViewState);

        bool            IsFollow() { return mbIsFollow; }
        void            SetFollow( bool bIsFollow) { mbIsFollow = bIsFollow; };

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

        DECL_LINK( WindowEventListener, VclWindowEvent&, void );
        bool IsMouseOverSidebarWin() const { return mbMouseOver; }

        void SetLanguage(const SvxLanguageItem& rNewItem);

        void ChangeSidebarItem( SwSidebarItem const & rSidebarItem );
        virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

        virtual void    Draw(OutputDevice* pDev, const Point&, const Size&, DrawFlags) override;
        virtual void KeyInput(const KeyEvent& rKeyEvt) override;
        virtual void MouseButtonDown(const MouseEvent& rMouseEvent) override;
        virtual void MouseButtonUp(const MouseEvent& rMouseEvent) override;
        virtual void MouseMove(const MouseEvent& rMouseEvent) override;
        void PaintTile(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);
        /// Is there a matching sub-widget inside this sidebar widget for rPointLogic?
        bool IsHitWindow(const Point& rPointLogic);
        /// Allows adjusting the point or mark of the selection to a document coordinate.
        void SetCursorLogicPosition(const Point& rPosition, bool bPoint, bool bClearMark);

    private:
        VclPtr<MenuButton> CreateMenuButton();
        virtual void    LoseFocus() override;
        virtual void    Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
        virtual void    GetFocus() override;

        void        SetSizePixel( const Size& rNewSize ) override;
        SfxItemSet  DefaultItem();

        DECL_LINK(ModifyHdl, LinkParamNone*, void);
        DECL_LINK(ScrollHdl, ScrollBar*, void);
        DECL_LINK(DeleteHdl, void*, void);

        sal_uInt32 CountFollowing();
        SwAnnotationWin*   GetTopReplyNote();
        SvxLanguageItem GetLanguage();

        VclBuilder      maBuilder;
        SwPostItMgr&    mrMgr;
        SwView&         mrView;

        ImplSVEvent *   mnEventId;

        std::unique_ptr<OutlinerView>   mpOutlinerView;
        std::unique_ptr<Outliner>       mpOutliner;

        VclPtr<sw::sidebarwindows::SidebarTextControl> mpSidebarTextControl;
        VclPtr<ScrollBar>      mpVScrollbar;
        VclPtr<Edit>           mpMetadataAuthor;
        VclPtr<Edit>           mpMetadataDate;
        VclPtr<MenuButton>     mpMenuButton;

        sw::sidebarwindows::AnchorOverlayObject* mpAnchor;
        sw::sidebarwindows::ShadowOverlayObject* mpShadow;
        std::unique_ptr<sw::overlay::OverlayRanges> mpTextRangeOverlay;

        Color           mColorAnchor;
        Color           mColorDark;
        Color           mColorLight;
        Color           mChangeColor;

        sw::sidebarwindows::SidebarPosition meSidebarPosition;

        tools::Rectangle       mPosSize;
        SwRect          mAnchorRect;
        long            mPageBorder;
        bool            mbAnchorRectChanged;

        std::vector<basegfx::B2DRange> maAnnotationTextRanges;

        bool            mbMouseOver;
        SwPostItHelper::SwLayoutStatus mLayoutStatus;

        bool            mbReadonly;
        bool            mbIsFollow;

        SwSidebarItem& mrSidebarItem;
        const SwFrame* mpAnchorFrame;

        SwFormatField*       mpFormatField;
        SwPostItField*       mpField;
        VclPtr<PopupMenu>    mpButtonPopup;
};

} } // end of namespace sw::annotation
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
