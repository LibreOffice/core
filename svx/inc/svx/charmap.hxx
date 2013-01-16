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
#ifndef _SVX_CHARMAP_HXX
#define _SVX_CHARMAP_HXX

#include <vcl/ctrl.hxx>
#include <vcl/metric.hxx>
#include <vcl/scrbar.hxx>
#include <boost/shared_ptr.hpp>
#include <map>
#include <tools/shl.hxx>
#include "svx/svxdllapi.h"

// define ----------------------------------------------------------------

#define COLUMN_COUNT    16
#define ROW_COUNT        8

namespace svx
{
    struct SvxShowCharSetItem;
    class SvxShowCharSetVirtualAcc;
}

// class SvxShowCharSet --------------------------------------------------

class SVX_DLLPUBLIC SvxShowCharSet : public Control
{
public:
                    SvxShowCharSet( Window* pParent, const ResId& rResId );
                    SvxShowCharSet( Window* pParent );
                    ~SvxShowCharSet();

    void            SetFont( const Font& rFont );

    void            SelectCharacter( sal_uInt32 cNew, sal_Bool bFocus = sal_False );
    sal_UCS4        GetSelectCharacter() const;

    Link            GetDoubleClickHdl() const { return aDoubleClkHdl; }
    void            SetDoubleClickHdl( const Link& rLink ) { aDoubleClkHdl = rLink; }
    Link            GetSelectHdl() const { return aSelectHdl; }
    void            SetSelectHdl( const Link& rHdl ) { aSelectHdl = rHdl; }
    Link            GetHighlightHdl() const { return aHighHdl; }
    void            SetHighlightHdl( const Link& rHdl ) { aHighHdl = rHdl; }
    Link            GetPreSelectHdl() const { return aHighHdl; }
    void            SetPreSelectHdl( const Link& rHdl ) { aPreSelectHdl = rHdl; }
    static sal_uInt32& getSelectedChar();

#ifdef _SVX_CHARMAP_CXX_
    ::svx::SvxShowCharSetItem*  ImplGetItem( int _nPos );
    int                         FirstInView( void) const;
    int                         LastInView( void) const;
    int                         PixelToMapIndex( const Point&) const;
    void                        SelectIndex( int index, sal_Bool bFocus = sal_False );
    void                        DeSelect();
    inline sal_Bool             IsSelected(sal_uInt16 _nPos) const { return _nPos == nSelectedIndex; }
    inline sal_uInt16               GetSelectIndexId() const { return sal::static_int_cast<sal_uInt16>(nSelectedIndex); }
    sal_uInt16                      GetRowPos(sal_uInt16 _nPos) const;
    sal_uInt16                      GetColumnPos(sal_uInt16 _nPos) const;

    ScrollBar*                  getScrollBar();
    void                        ReleaseAccessible();
    sal_Int32                   getMaxCharCount() const;
#endif // _SVX_CHARMAP_CXX_

    virtual void    Resize();

protected:
    virtual void    Paint( const Rectangle& );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    GetFocus();
    virtual void    LoseFocus();
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );


    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();



private:
    typedef ::std::map<sal_Int32, boost::shared_ptr<svx::SvxShowCharSetItem> > ItemsMap;
    ItemsMap        m_aItems;
    Link            aDoubleClkHdl;
    Link            aSelectHdl;
    Link            aHighHdl;
    Link            aPreSelectHdl;
    ::svx::SvxShowCharSetVirtualAcc*    m_pAccessible;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > m_xAccessible;
    long            nX;
    long            nY;
    long            m_nXGap;
    long            m_nYGap;
    sal_Bool            bDrag;

    sal_Int32       nSelectedIndex;

    FontCharMap     maFontCharMap;
    ScrollBar       aVscrollSB;

private:
    void            DrawChars_Impl( int n1, int n2);
    void            InitSettings( sal_Bool bForeground, sal_Bool bBackground);
    // abstraction layers are: Unicode<->MapIndex<->Pixel
    Point           MapIndexToPixel( int) const;
    DECL_LINK(VscrollHdl, void *);

    void            init();
    Rectangle       getGridRectangle(const Point &rPointUL, const Size &rOutputSize);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
