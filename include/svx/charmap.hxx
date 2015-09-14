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

#include <vcl/ctrl.hxx>
#include <vcl/metric.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/vclptr.hxx>
#include <map>
#include <memory>
#include <svx/svxdllapi.h>

#define COLUMN_COUNT    16
#define ROW_COUNT        8

namespace svx
{
    struct SvxShowCharSetItem;
    class SvxShowCharSetVirtualAcc;
}

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxShowCharSet : public Control
{
public:
                    SvxShowCharSet( vcl::Window* pParent );
                    virtual ~SvxShowCharSet();
    virtual void    dispose() SAL_OVERRIDE;
    virtual void    ApplySettings(vcl::RenderContext& rRenderContext) SAL_OVERRIDE;

    void            RecalculateFont(vcl::RenderContext& rRenderContext);

    void            SelectCharacter( sal_uInt32 cNew, bool bFocus = false );
    sal_UCS4        GetSelectCharacter() const;

    void            SetDoubleClickHdl( const Link<SvxShowCharSet*,void>& rLink ) { aDoubleClkHdl = rLink; }
    void            SetSelectHdl( const Link<SvxShowCharSet*,void>& rHdl ) { aSelectHdl = rHdl; }
    void            SetHighlightHdl( const Link<SvxShowCharSet*,void>& rHdl ) { aHighHdl = rHdl; }
    void            SetPreSelectHdl( const Link<SvxShowCharSet*,void>& rHdl ) { aPreSelectHdl = rHdl; }
    static sal_uInt32& getSelectedChar();
    void            SetFont( const vcl::Font& rFont );

    svx::SvxShowCharSetItem*    ImplGetItem( int _nPos );
    int                         FirstInView() const;
    int                         LastInView() const;
    int                         PixelToMapIndex( const Point&) const;
    void                        SelectIndex( int index, bool bFocus = false );
    void                        OutputIndex( int index );
    void                        DeSelect();
    inline bool                 IsSelected(sal_uInt16 _nPos) const { return _nPos == nSelectedIndex; }
    inline sal_uInt16           GetSelectIndexId() const { return sal::static_int_cast<sal_uInt16>(nSelectedIndex); }
    static sal_uInt16           GetRowPos(sal_uInt16 _nPos);
    static sal_uInt16           GetColumnPos(sal_uInt16 _nPos);

    ScrollBar&                  getScrollBar() { return *aVscrollSB.get();}
    void                        ReleaseAccessible();
    sal_Int32                   getMaxCharCount() const;

    virtual void    Resize() SAL_OVERRIDE;

protected:
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& ) SAL_OVERRIDE;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
    virtual void    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void    GetFocus() SAL_OVERRIDE;
    virtual void    LoseFocus() SAL_OVERRIDE;
    virtual void    StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() SAL_OVERRIDE;

private:
    typedef std::map<sal_Int32, std::shared_ptr<svx::SvxShowCharSetItem> > ItemsMap;
    ItemsMap        m_aItems;
    Link<SvxShowCharSet*,void>     aDoubleClkHdl;
    Link<SvxShowCharSet*,void>     aSelectHdl;
    Link<SvxShowCharSet*,void>     aHighHdl;
    Link<SvxShowCharSet*,void>     aPreSelectHdl;
    svx::SvxShowCharSetVirtualAcc* m_pAccessible;
    css::uno::Reference<css::accessibility::XAccessible> m_xAccessible;
    long            nX;
    long            nY;
    long            m_nXGap;
    long            m_nYGap;
    bool            bDrag;

    sal_Int32       nSelectedIndex;

    FontCharMapPtr  mpFontCharMap;
    Size            maFontSize;
    VclPtr<ScrollBar>  aVscrollSB;

    bool mbRecalculateFont  : 1;
    bool mbUpdateForeground : 1;
    bool mbUpdateBackground : 1;


private:
    void            DrawChars_Impl(vcl::RenderContext& rRenderContext, int n1, int n2);
    void            InitSettings(vcl::RenderContext& rRenderContext);
    // abstraction layers are: Unicode<->MapIndex<->Pixel
    Point           MapIndexToPixel( int) const;
    DECL_LINK_TYPED(VscrollHdl, ScrollBar*, void);

    void            init();
    Rectangle       getGridRectangle(const Point &rPointUL, const Size &rOutputSize);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
