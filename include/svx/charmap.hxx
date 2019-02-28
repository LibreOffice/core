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
#ifndef INCLUDED_SVX_CHARMAP_HXX
#define INCLUDED_SVX_CHARMAP_HXX

#include <sal/config.h>

#include <deque>
#include <map>
#include <memory>

#include <sal/types.h>
#include <rtl/ref.hxx>
#include <svx/svxdllapi.h>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/event.hxx>
#include <vcl/fontcharmap.hxx>
#include <vcl/outdev.hxx>
#include <vcl/metric.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include <vcl/window.hxx>
#include <vcl/textview.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace com { namespace sun { namespace star {
    namespace accessibility { class XAccessible; }
} } }

namespace vcl { class Font; }

using namespace ::com::sun::star;

#define COLUMN_COUNT    16
#define ROW_COUNT        8

namespace svx
{
    struct SvxShowCharSetItem;
    class SvxShowCharSetAcc;
}

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxShowCharSet : public weld::CustomWidgetController
{
protected:
    VclPtr<VirtualDevice> mxVirDev;
    vcl::Font maFont;
    std::unique_ptr<weld::ScrolledWindow> mxScrollArea;
public:
    SvxShowCharSet(std::unique_ptr<weld::ScrolledWindow> pScrollArea, const VclPtr<VirtualDevice>& rVirDev);
    virtual ~SvxShowCharSet() override;

    virtual void            RecalculateFont(vcl::RenderContext& rRenderContext);

    void                    SelectCharacter( sal_UCS4 cNew );
    virtual sal_UCS4        GetSelectCharacter() const;
    void                    createContextMenu();

    void            SetDoubleClickHdl( const Link<SvxShowCharSet*,void>& rLink ) { aDoubleClkHdl = rLink; }
    void            SetSelectHdl( const Link<SvxShowCharSet*,void>& rHdl ) { aSelectHdl = rHdl; }
    void            SetHighlightHdl( const Link<SvxShowCharSet*,void>& rHdl ) { aHighHdl = rHdl; }
    void            SetPreSelectHdl( const Link<SvxShowCharSet*,void>& rHdl ) { aPreSelectHdl = rHdl; }
    void            SetFavClickHdl( const Link<SvxShowCharSet*,void>& rHdl ) { aFavClickHdl = rHdl; }
    static sal_uInt32& getSelectedChar();
    void            SetFont( const vcl::Font& rFont );
    vcl::Font const & GetFont() const { return maFont; }
    FontCharMapRef  GetFontCharMap();
    bool            isFavChar(const OUString& sTitle, const OUString& rFont);
    void            getFavCharacterList(); //gets both Fav char and Fav char font list
    void            updateFavCharacterList(const OUString& rChar, const OUString& rFont);

    virtual svx::SvxShowCharSetItem*    ImplGetItem( int _nPos );
    int                         FirstInView() const;
    virtual int                         LastInView() const;
    int                         PixelToMapIndex( const Point&) const;
    virtual void                        SelectIndex( int index, bool bFocus = false );
    void                        OutputIndex( int index );
    void                        DeSelect();
    static void                 CopyToClipboard(const OUString& str);
    bool                 IsSelected(sal_uInt16 _nPos) const { return _nPos == nSelectedIndex; }
    sal_uInt16           GetSelectIndexId() const { return sal::static_int_cast<sal_uInt16>(nSelectedIndex); }
    static sal_uInt16           GetRowPos(sal_uInt16 _nPos);
    static sal_uInt16           GetColumnPos(sal_uInt16 _nPos);

    virtual sal_Int32                   getMaxCharCount() const;

    void Show() { mxScrollArea->show(); }
    void Hide() { mxScrollArea->hide(); }

    uno::Reference<css::accessibility::XAccessible> getAccessibleParent() { return GetDrawingArea()->get_accessible_parent(); }

private:
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void Resize() override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual bool MouseMove(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;
    virtual bool KeyInput(const KeyEvent&) override;

    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;
    virtual FactoryFunction GetUITestFactory() const override;

protected:
    typedef std::map<sal_Int32, std::shared_ptr<svx::SvxShowCharSetItem> > ItemsMap;
    ItemsMap        m_aItems;
    Link<SvxShowCharSet*,void>     aDoubleClkHdl;
    Link<SvxShowCharSet*,void>     aSelectHdl;
    Link<SvxShowCharSet*,void>     aFavClickHdl;
    Link<SvxShowCharSet*,void>     aHighHdl;
    Link<SvxShowCharSet*,void>     aPreSelectHdl;

    std::deque<OUString>           maFavCharList;
    std::deque<OUString>           maFavCharFontList;

    rtl::Reference<svx::SvxShowCharSetAcc> m_xAccessible;
    uno::Reference< uno::XComponentContext > mxContext;
    long            nX;
    long            nY;
    long            m_nXGap;
    long            m_nYGap;
    bool            bDrag;

    sal_Int32       nSelectedIndex;

    FontCharMapRef  mxFontCharMap;
    Size            maFontSize;
    Point           maPosition;

    bool mbRecalculateFont  : 1;
    bool mbUpdateForeground : 1;
    bool mbUpdateBackground : 1;


protected:
    virtual void            DrawChars_Impl(vcl::RenderContext& rRenderContext, int n1, int n2);
    void            InitSettings(vcl::RenderContext& rRenderContext);
    // abstraction layers are: Unicode<->MapIndex<->Pixel
    Point           MapIndexToPixel( int) const;
    DECL_LINK(VscrollHdl, weld::ScrolledWindow&, void);
    void ContextMenuSelect(const OString& rIdent);

    void            init();
    tools::Rectangle       getGridRectangle(const Point &rPointUL, const Size &rOutputSize);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
