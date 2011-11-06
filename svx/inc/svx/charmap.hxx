/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_CHARMAP_HXX
#define _SVX_CHARMAP_HXX

// include ---------------------------------------------------------------

#include <vcl/ctrl.hxx>
#include <vcl/metric.hxx>
#include <vcl/scrbar.hxx>
#include <map>
#include <tools/shl.hxx> //add CHINA001
#include <tools/debug.hxx> //add CHINA001
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

    void                        ImplFireAccessibleEvent( short nEventId,
                                                         const ::com::sun::star::uno::Any& rOldValue,
                                                         const ::com::sun::star::uno::Any& rNewValue );
    ScrollBar*                  getScrollBar();
    void                        ReleaseAccessible();
    sal_Int32                   getMaxCharCount() const;
#endif // _SVX_CHARMAP_CXX_

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
    typedef ::std::map<sal_Int32, ::svx::SvxShowCharSetItem*> ItemsMap;
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
    DECL_LINK( VscrollHdl, ScrollBar* );
};

#endif

