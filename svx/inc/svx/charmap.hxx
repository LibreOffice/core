/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
    sal_Bool            bDrag;

    sal_Int32       nSelectedIndex;

    FontCharMap     maFontCharMap;
    ScrollBar       aVscrollSB;
    Size            aOrigSize;
    Point           aOrigPos;

private:
    void            DrawChars_Impl( int n1, int n2);
    void            InitSettings( sal_Bool bForeground, sal_Bool bBackground);
    // abstraction layers are: Unicode<->MapIndex<->Pixel
    Point           MapIndexToPixel( int) const;
    DECL_LINK(VscrollHdl, void *);

    void            init();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
