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
#ifndef SW_SIDEBAR_PAGE_PROPERTY_PANEL_HXX
#define SW_SIDEBAR_PAGE_PROPERTY_PANEL_HXX

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/document/XUndoManager.hpp>

#include <svx/sidebar/Popup.hxx>
#include <svx/sidebar/PanelLayout.hxx>

#include <sfx2/sidebar/ControllerItem.hxx>

namespace svx { namespace sidebar {
    class PopupControl;
} }

#include <i18nutil/paper.hxx>

#include <svx/pageitem.hxx>
#include <svx/rulritem.hxx>
#include <editeng/sizeitem.hxx>

#include <vcl/ctrl.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <svl/intitem.hxx>

#include <boost/scoped_ptr.hpp>


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

namespace sw { namespace sidebar {

    class PagePropertyPanel
        : public PanelLayout,
          public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
    {
    public:
        static PagePropertyPanel* Create(
            Window* pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame>& rxFrame,
            SfxBindings* pBindings );

        // interface of ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
        virtual void NotifyItemUpdate(
            const sal_uInt16 nSId,
            const SfxItemState eState,
            const SfxPoolItem* pState,
            const bool bIsEnabled);

        SfxBindings* GetBindings() const
        {
            return mpBindings;
        }


        ::svx::sidebar::PopupControl* CreatePageOrientationControl( ::svx::sidebar::PopupContainer* pParent );
        void ExecuteOrientationChange( const sal_Bool bLandscape );
        void ClosePageOrientationPopup();

        ::svx::sidebar::PopupControl* CreatePageMarginControl( ::svx::sidebar::PopupContainer* pParent );
        void ExecuteMarginLRChange(
            const long nPageLeftMargin,
            const long nPageRightMargin );
        void ExecuteMarginULChange(
            const long nPageTopMargin,
            const long nPageBottomMargin );
        void ExecutePageLayoutChange( const bool bMirrored );
        void ClosePageMarginPopup();

        ::svx::sidebar::PopupControl* CreatePageSizeControl( ::svx::sidebar::PopupContainer* pParent );
        void ExecuteSizeChange( const Paper ePaper );
        void ClosePageSizePopup();

        ::svx::sidebar::PopupControl* CreatePageColumnControl( ::svx::sidebar::PopupContainer* pParent );
        void ExecuteColumnChange( const sal_uInt16 nColumnType );
        void ClosePageColumnPopup();

        void StartUndo();
        void EndUndo();

    private:
        PagePropertyPanel(
            Window* pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame>& rxFrame,
            SfxBindings* pBindings );
        virtual ~PagePropertyPanel(void);

        SfxBindings* mpBindings;

        // toolboxes - on click open corresponding popup
        ToolBox*                mpToolBoxOrientation;
        ToolBox*                mpToolBoxMargin;
        ToolBox*                mpToolBoxSize;
        ToolBox*                mpToolBoxColumn;

        Image*                  maImgSize;
        Image*                  maImgSize_L;
        Image                   mImgPortrait;
        Image                   mImgLandscape;
        Image                   mImgNarrow;
        Image                   mImgNormal;
        Image                   mImgWide;
        Image                   mImgMirrored;
        Image                   mImgMarginCustom;
        Image                   mImgNarrow_L;
        Image                   mImgNormal_L;
        Image                   mImgWide_L;
        Image                   mImgMirrored_L;
        Image                   mImgMarginCustom_L;
        Image                   mImgA3;
        Image                   mImgA4;
        Image                   mImgA5;
        Image                   mImgB4;
        Image                   mImgB5;
        Image                   mImgC5;
        Image                   mImgLetter;
        Image                   mImgLegal;
        Image                   mImgSizeNone;
        Image                   mImgA3_L;
        Image                   mImgA4_L;
        Image                   mImgA5_L;
        Image                   mImgB4_L;
        Image                   mImgB5_L;
        Image                   mImgC5_L;
        Image                   mImgLetter_L;
        Image                   mImgLegal_L;
        Image                   mImgSizeNone_L;
        Image                   mImgColumn1;
        Image                   mImgColumn2;
        Image                   mImgColumn3;
        Image                   mImgLeft;
        Image                   mImgRight;
        Image                   mImgColumnNone;
        Image                   mImgColumn1_L;
        Image                   mImgColumn2_L;
        Image                   mImgColumn3_L;
        Image                   mImgLeft_L;
        Image                   mImgRight_L;
        Image                   mImgColumnNone_L;

        // item keeping the following page style attributes:
        // - page orientation
        // - page usage - only left, only right, both, mirrored
        // item also hold the numbering type for the page style which should
        // be kept stable.
        ::boost::scoped_ptr<SvxPageItem> mpPageItem;

        // item keeping the page style's left and right margins
        ::boost::scoped_ptr<SvxLongLRSpaceItem> mpPageLRMarginItem;
        // item keeping the page style's top and bottom margins
        ::boost::scoped_ptr<SvxLongULSpaceItem> mpPageULMarginItem;

        // item keeping the page style's page size
        ::boost::scoped_ptr<SvxSizeItem> mpPageSizeItem;
        // Paper corresponding to the page style's page size
        Paper mePaper;

        // item keeping the page column type
        ::boost::scoped_ptr<SfxInt16Item> mpPageColumnTypeItem;

        FieldUnit           meFUnit;
        SfxMapUnit          meUnit;

        // controller items
        ::sfx2::sidebar::ControllerItem m_aSwPagePgULControl;
        ::sfx2::sidebar::ControllerItem m_aSwPagePgLRControl;
        ::sfx2::sidebar::ControllerItem m_aSwPagePgSizeControl;
        ::sfx2::sidebar::ControllerItem m_aSwPagePgControl;
        ::sfx2::sidebar::ControllerItem m_aSwPageColControl;
        ::sfx2::sidebar::ControllerItem m_aSwPagePgMetricControl;

        // popups
        ::svx::sidebar::Popup maOrientationPopup;
        ::svx::sidebar::Popup maMarginPopup;
        ::svx::sidebar::Popup maSizePopup;
        ::svx::sidebar::Popup maColumnPopup;

        const cssu::Reference< css::document::XUndoManager > mxUndoManager;

        bool mbInvalidateSIDAttrPageOnSIDAttrPageSizeNotify;

        // handler for popup toolboxes to show the popups
        DECL_LINK(ClickOrientationHdl, ToolBox* );
        DECL_LINK(ClickMarginHdl, ToolBox* );
        DECL_LINK(ClickSizeHdl, ToolBox* );
        DECL_LINK(ClickColumnHdl, ToolBox* );

        void Initialize();

        void MetricState( SfxItemState eState, const SfxPoolItem* pState );

        // helper to adjust popup toolbox' images
        void ChangeMarginImage();
        void ChangeSizeImage();
        void ChangeColumnImage( const sal_uInt16 nColumnType );

    };

} } // end of namespace ::sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
