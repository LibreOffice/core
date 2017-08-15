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

#include <map>
#include <memory>

#include <sal/types.h>
#include <rtl/ref.hxx>
#include <svx/svxdllapi.h>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/event.hxx>
#include <vcl/outdev.hxx>
#include <vcl/metric.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/window.hxx>
#include <vcl/textview.hxx>

namespace com { namespace sun { namespace star {
    namespace accessibility { class XAccessible; }
} } }

namespace vcl { class Font; }

using namespace ::com::sun::star;

#define COLUMN_COUNT    16
#define ROW_COUNT        8

class CommandEvent;
class ScrollBar;

namespace svx
{
    struct SvxShowCharSetItem;
    class SvxShowCharSetVirtualAcc;
}

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxShowCharSet : public Control
{
public:
                    SvxShowCharSet( vcl::Window* pParent );
                    virtual ~SvxShowCharSet() override;
    virtual void    dispose() override;
    virtual void    ApplySettings(vcl::RenderContext& rRenderContext) override;

    virtual void            RecalculateFont(vcl::RenderContext& rRenderContext);

    void                    SelectCharacter( sal_uInt32 cNew );
    virtual sal_UCS4        GetSelectCharacter() const;
    void                    createContextMenu();

    void            SetDoubleClickHdl( const Link<SvxShowCharSet*,void>& rLink ) { aDoubleClkHdl = rLink; }
    void            SetSelectHdl( const Link<SvxShowCharSet*,void>& rHdl ) { aSelectHdl = rHdl; }
    void            SetHighlightHdl( const Link<SvxShowCharSet*,void>& rHdl ) { aHighHdl = rHdl; }
    void            SetPreSelectHdl( const Link<SvxShowCharSet*,void>& rHdl ) { aPreSelectHdl = rHdl; }
    void            SetFavClickHdl( const Link<SvxShowCharSet*,void>& rHdl ) { aFavClickHdl = rHdl; }
    static sal_uInt32& getSelectedChar();
    void            SetFont( const vcl::Font& rFont );
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
    void                        CopyToClipboard(const OUString& str);
    bool                 IsSelected(sal_uInt16 _nPos) const { return _nPos == nSelectedIndex; }
    sal_uInt16           GetSelectIndexId() const { return sal::static_int_cast<sal_uInt16>(nSelectedIndex); }
    static sal_uInt16           GetRowPos(sal_uInt16 _nPos);
    static sal_uInt16           GetColumnPos(sal_uInt16 _nPos);

    ScrollBar&                  getScrollBar() { return *aVscrollSB.get();}
    void                        ReleaseAccessible();
    virtual sal_Int32                   getMaxCharCount() const;

    virtual void    Resize() override;

    virtual FactoryFunction GetUITestFactory() const override;

protected:
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;
    virtual void    StateChanged( StateChangedType nStateChange ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;

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

    rtl::Reference<svx::SvxShowCharSetVirtualAcc> m_xAccessible;
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
    VclPtr<ScrollBar>  aVscrollSB;

    bool mbRecalculateFont  : 1;
    bool mbUpdateForeground : 1;
    bool mbUpdateBackground : 1;


protected:
    virtual void            DrawChars_Impl(vcl::RenderContext& rRenderContext, int n1, int n2);
    void            InitSettings(vcl::RenderContext& rRenderContext);
    // abstraction layers are: Unicode<->MapIndex<->Pixel
    Point           MapIndexToPixel( int) const;
    DECL_LINK(VscrollHdl, ScrollBar*, void);
    DECL_LINK(ContextMenuSelectHdl, Menu*, bool);

    void            init();
    tools::Rectangle       getGridRectangle(const Point &rPointUL, const Size &rOutputSize);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
