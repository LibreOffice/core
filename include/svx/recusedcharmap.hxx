/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#ifndef INCLUDED_SVX_RECUSEDCHARMAP_HXX
#define INCLUDED_SVX_RECUSEDCHARMAPFAV_HXX

#include <svx/svxdllapi.h>
#include <svx/charmap.hxx>
#include <svx/recusedlist.hxx>

#include <map>
#include <memory>


#define ROW_COUNT 2
//TODO sistemare codice e nomi

class SVX_DLLPUBLIC SvxShowRecCharSet : public SvxShowCharSet
{
  public:
    SvxShowRecCharSet( vcl::Window* pParent );

    int                         PixelToMapIndex( const Point&) const;

    void                        SelectIndex(int index);
    void                        pushRecentlyChar(sal_UCS4 cChar,vcl::Font aFont);
    sal_UCS4                    GetSelectCharacter();
    vcl::Font                   GetSelectCharFont();

    void                        SetSelectHdl( const Link<SvxShowRecCharSet*,void>& rHdl ) { aSelectHdl = rHdl; }
    void                        SetHighlightHdl( const Link<SvxShowRecCharSet*,void>& rHdl ) { aHighHdl = rHdl; }
    void                        SetPreSelectHdl( const Link<SvxShowRecCharSet*,void>& rHdl ) { aPreSelectHdl = rHdl; }
    void                        SetDoubleClickHdl( const Link<SvxShowRecCharSet*,void>& rLink ) { aDoubleClkHdl = rLink; }
    svx::SvxShowCharSetItem*    ImplGetItem( int _nPos );
protected:
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;
    virtual void    StateChanged( StateChangedType nStateChange ) override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;

    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;

private:
        Link<SvxShowRecCharSet*,void>     aDoubleClkHdl;
        Link<SvxShowRecCharSet*,void>     aSelectHdl;
        Link<SvxShowRecCharSet*,void>     aHighHdl;
        Link<SvxShowRecCharSet*,void>     aPreSelectHdl;

    typedef std::map<sal_Int32, std::shared_ptr<svx::SvxShowCharSetItem> > ItemsMap;
    ItemsMap        m_aItems;
    svx::SvxShowCharSetVirtualAcc* m_pAccessible;
    css::uno::Reference<css::accessibility::XAccessible> m_xAccessible;

    long            nX;
    long            nY;
    long            m_nXGap;
    long            m_nYGap;
    bool            bDrag;
    sal_Int32       nSelectedIndex;
    Size            maFontSize;

    bool mbUpdateForeground : 1;
    bool mbUpdateBackground : 1;

    RecentlyUsedCharMap mRecentlyUsedCharMap;

private:
   void            InitSettings(vcl::RenderContext& rRenderContext);
   void            DrawChars_Impl(vcl::RenderContext& rRenderContext);
   void            init();


   Rectangle       getGridRectangle(const Point &rPointUL, const Size &rOutputSize);
   Point           MapIndexToPixel( int) const;
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */