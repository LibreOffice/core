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
#include <tools/time.hxx>
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
class FixedText;
class MenuButton;
class SwFrame;
class SvxLanguageItem;
namespace sw::overlay { class OverlayRanges; }
namespace sw::sidebarwindows {
    class SidebarTextControl;
    class AnchorOverlayObject;
    class ShadowOverlayObject;
}


namespace sw::annotation {

class SAL_DLLPUBLIC_RTTI SwAnnotationWin : public vcl::Window
{
    public:
        SwAnnotationWin( SwEditWin& rEditWin,
                         SwPostItMgr& aMgr,
                         SwSidebarItem& rSidebarItem,
                         SwFormatField* aField );
        virtual ~SwAnnotationWin() override;
        virtual void dispose() override;

        void    UpdateData();
        void    SetPostItText();
        void    Delete();
        void    GotoPos();
        const SwPostItField* GetPostItField() const { return mpField; }
        void UpdateText(const OUString& aText);

        OUString GetAuthor() const;
        Date     GetDate() const;
        tools::Time GetTime() const;

        sal_uInt32 MoveCaret();

        /// Calculate parent postit id of current annotation window
        sal_uInt32 CalcParent();
        void       InitAnswer(OutlinerParaObject const * pText);

        bool IsProtected() const;

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
        const SwRect& GetAnchorRect() const { return mAnchorRect; }
        bool IsAnchorRectChanged() const { return mbAnchorRectChanged; }
        void ResetAnchorRectChanged() { mbAnchorRectChanged = false; }
        const std::vector<basegfx::B2DRange>& GetAnnotationTextRanges() const { return maAnnotationTextRanges; }
        SwEditWin& EditWin();
        SwSidebarItem& GetSidebarItem() { return mrSidebarItem; }

        OutlinerView* GetOutlinerView() { return mpOutlinerView.get();}
        Outliner* GetOutliner() { return mpOutliner.get();}
        bool HasScrollbar() const;
        bool IsScrollbarVisible() const;
        ScrollBar* Scrollbar() { return mpVScrollbar; }
        ::sw::sidebarwindows::AnchorOverlayObject* Anchor() { return mpAnchor.get();}
        ::sw::sidebarwindows::ShadowOverlayObject* Shadow() { return mpShadow.get();}
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
        Point           VirtualPos()    { return mPosSize.TopLeft(); }
        Size            VirtualSize()   { return mPosSize.GetSize(); }

        void            ShowAnchorOnly(const Point &aPoint);
        void            ShowNote();
        void            HideNote();
        void            InvalidateControl();

        void            ResetAttributes();

        void            SetSidebarPosition(sw::sidebarwindows::SidebarPosition eSidebarPosition);
        void            SetReadonly(bool bSet);
        bool            IsReadOnly() const
        {
            return mbReadonly;
        }

        void         SetColor(Color aColorDark,Color aColorLight, Color aColorAnchor);
        const Color& ColorAnchor() { return mColorAnchor; }
        const Color& ColorDark() { return mColorDark; }
        const Color& ColorLight() { return mColorLight; }
        void         Rescale();

        void            SetViewState(::sw::sidebarwindows::ViewState bViewState);

        bool            IsFollow() const { return mbIsFollow; }
        void            SetFollow( bool bIsFollow) { mbIsFollow = bIsFollow; };

        sal_Int32   GetMetaHeight();
        sal_Int32   GetMinimumSizeWithMeta() const;
        sal_Int32   GetMinimumSizeWithoutMeta() const;
        sal_Int32   GetMetaButtonAreaWidth() const;
        sal_Int32   GetScrollbarWidth() const;
        sal_Int32   GetNumFields();

        void    SetSpellChecking();

        void    ToggleInsMode();

        void    ActivatePostIt();
        void    DeactivatePostIt();

        void SetChangeTracking( const SwPostItHelper::SwLayoutStatus aStatus,
                                const Color& aColor);
        SwPostItHelper::SwLayoutStatus GetLayoutStatus() const { return mLayoutStatus; }
        const Color& GetChangeColor() const { return mChangeColor; }

        DECL_LINK( WindowEventListener, VclWindowEvent&, void );
        bool IsMouseOverSidebarWin() const { return mbMouseOver; }

        void SetLanguage(const SvxLanguageItem& rNewItem);

        void ChangeSidebarItem( SwSidebarItem const & rSidebarItem );
        virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

        virtual void    Draw(OutputDevice* pDev, const Point&, DrawFlags) override;
        virtual void KeyInput(const KeyEvent& rKeyEvt) override;
        virtual void MouseButtonDown(const MouseEvent& rMouseEvent) override;
        virtual void MouseButtonUp(const MouseEvent& rMouseEvent) override;
        virtual void MouseMove(const MouseEvent& rMouseEvent) override;
        void PaintTile(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);
        /// Is there a matching sub-widget inside this sidebar widget for rPointLogic?
        bool IsHitWindow(const Point& rPointLogic);
        /// Allows adjusting the point or mark of the selection to a document coordinate.
        void SetCursorLogicPosition(const Point& rPosition, bool bPoint, bool bClearMark);

        // Various access functions for 'resolved' status
        void SetResolved(bool resolved);
        void ToggleResolved();
        void ToggleResolvedForThread();
        void DeleteThread();
        bool IsResolved() const;
        bool IsThreadResolved();

        // Set this SwAnnotationWin as the currently active one
        // return false if it was already active
        bool SetActiveSidebarWin();
        // Unset this SwAnnotationWin as the currently active one
        void UnsetActiveSidebarWin();

        /// Find the first annotation for the thread which this annotation is in.
        /// This may be the same annotation as this one.
        SwAnnotationWin*   GetTopReplyNote();

        virtual FactoryFunction GetUITestFactory() const override;

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

        SvxLanguageItem GetLanguage() const;

        VclBuilder      maBuilder;
        SwPostItMgr&    mrMgr;
        SwView&         mrView;

        ImplSVEvent *   mnEventId;

        std::unique_ptr<OutlinerView>   mpOutlinerView;
        std::unique_ptr<Outliner>       mpOutliner;

        VclPtr<sw::sidebarwindows::SidebarTextControl> mpSidebarTextControl;
        VclPtr<ScrollBar>      mpVScrollbar;
        VclPtr<FixedText>      mpMetadataAuthor;
        VclPtr<FixedText>      mpMetadataDate;
        VclPtr<FixedText>      mpMetadataResolved;
        VclPtr<MenuButton>     mpMenuButton;

        std::unique_ptr<sw::sidebarwindows::AnchorOverlayObject> mpAnchor;
        std::unique_ptr<sw::sidebarwindows::ShadowOverlayObject> mpShadow;
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

        bool            mbResolvedStateUpdated;

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

} // end of namespace sw::annotation

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
