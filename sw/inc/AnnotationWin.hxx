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
#include <editeng/outlobj.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <vcl/customweld.hxx>

#include "postithelper.hxx"
#include "swrect.hxx"
#include "SidebarWindowsTypes.hxx"

class EditView;
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
    class SidebarWinAccessible;
}


namespace sw::annotation {

class SAL_DLLPUBLIC_RTTI SwAnnotationWin final : public InterimItemWindow
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
        void       InitAnswer(OutlinerParaObject const & rText);

        bool IsProtected() const;

        void SetSize( const Size& rNewSize );
        void SetPosSizePixelRect( tools::Long nX,
                                  tools::Long nY,
                                  tools::Long nWidth,
                                  tools::Long nHeight,
                                  const SwRect& aAnchorRect,
                                  const tools::Long PageBorder);
        void SetPosAndSize();
        void TranslateTopPosition(const tools::Long aAmount);
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
        ::sw::sidebarwindows::AnchorOverlayObject* Anchor() { return mpAnchor.get();}
        ::sw::sidebarwindows::ShadowOverlayObject* Shadow() { return mpShadow.get();}
        ::sw::overlay::OverlayRanges* TextRange() { return mpTextRangeOverlay.get();}

        tools::Long            GetPostItTextHeight();

        void            SwitchToPostIt(sal_uInt16 aDirection);
        void            SwitchToFieldPos();

        void            ExecuteCommand(sal_uInt16 nSlot);
        void            InitControls();
        void            DoResize();
        void            ResizeIfNecessary(tools::Long aOldHeight, tools::Long aNewHeight);
        void            SetScrollbar();
        void            LockView(bool bLock);

        void            SetVirtualPosSize( const Point& aPoint, const Size& aSize);
        Point           VirtualPos()    { return mPosSize.TopLeft(); }
        Size            VirtualSize()   { return mPosSize.GetSize(); }

        void            ShowAnchorOnly(const Point &aPoint);
        void            ShowNote();
        void            HideNote();

        void            ResetAttributes();

        void            SetSidebarPosition(sw::sidebarwindows::SidebarPosition eSidebarPosition);
        void            SetReadonly(bool bSet);
        bool            IsReadOnly() const
        {
            return mbReadonly;
        }

        void         SetColor(Color aColorDark,Color aColorLight, Color aColorAnchor);
        const Color& ColorDark() { return mColorDark; }
        const Color& ColorLight() { return mColorLight; }
        void         Rescale();

        void            SetViewState(::sw::sidebarwindows::ViewState bViewState);

        bool            IsFollow() const { return mbIsFollow; }
        void            SetFollow( bool bIsFollow) { mbIsFollow = bIsFollow; };

        sal_Int32   GetMetaHeight() const;
        sal_Int32   GetMinimumSizeWithMeta() const;
        sal_Int32   GetMinimumSizeWithoutMeta() const;
        int         GetPrefScrollbarWidth() const;
        sal_Int32   GetNumFields() const;

        void    SetSpellChecking();

        void    ToggleInsMode();

        void    ActivatePostIt();
        void    DeactivatePostIt();

        void SetChangeTracking( const SwPostItHelper::SwLayoutStatus aStatus,
                                const Color& aColor);
        SwPostItHelper::SwLayoutStatus GetLayoutStatus() const { return mLayoutStatus; }
        const Color& GetChangeColor() const { return mChangeColor; }

        bool IsMouseOverSidebarWin() const { return mbMouseOver; }

        void SetLanguage(const SvxLanguageItem& rNewItem);

        void ChangeSidebarItem( SwSidebarItem const & rSidebarItem );
        virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

        void DrawForPage(OutputDevice* pDev, const Point& rPos);

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

        virtual void    LoseFocus() override;
        virtual void    GetFocus() override;

        void        SetSizePixel( const Size& rNewSize ) override;
        SfxItemSet  DefaultItem();

        DECL_DLLPRIVATE_LINK(ModifyHdl, LinkParamNone*, void);
        DECL_DLLPRIVATE_LINK(ScrollHdl, weld::ScrolledWindow&, void);
        DECL_DLLPRIVATE_LINK(DeleteHdl, void*, void);
        DECL_DLLPRIVATE_LINK(ToggleHdl, weld::Toggleable&, void);
        DECL_DLLPRIVATE_LINK(SelectHdl, const OString&, void);
        DECL_DLLPRIVATE_LINK(KeyInputHdl, const KeyEvent&, bool);
        DECL_DLLPRIVATE_LINK(MouseMoveHdl, const MouseEvent&, bool);

        sal_uInt32 CountFollowing();

        SvxLanguageItem GetLanguage() const;

        void SetMenuButtonColors();

        SwPostItMgr&    mrMgr;
        SwView&         mrView;

        ImplSVEvent*    mnDeleteEventId;

        std::unique_ptr<OutlinerView>   mpOutlinerView;
        std::unique_ptr<Outliner>       mpOutliner;

        std::unique_ptr<weld::ScrolledWindow> mxVScrollbar;
        std::unique_ptr<sw::sidebarwindows::SidebarTextControl> mxSidebarTextControl;
        std::unique_ptr<weld::CustomWeld> mxSidebarTextControlWin;
        vcl::Font maLabelFont;
        std::unique_ptr<weld::Label> mxMetadataAuthor;
        std::unique_ptr<weld::Label> mxMetadataDate;
        std::unique_ptr<weld::Label> mxMetadataResolved;
        std::unique_ptr<weld::MenuButton> mxMenuButton;

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
        tools::Long            mPageBorder;
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

        rtl::Reference<sw::sidebarwindows::SidebarWinAccessible> mxSidebarWinAccessible;
};

} // end of namespace sw::annotation

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
