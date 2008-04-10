/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: charmap.hxx,v $
 * $Revision: 1.5 $
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

// include ---------------------------------------------------------------

#include <vcl/ctrl.hxx>
#include <vcl/metric.hxx>
#include <vcl/scrbar.hxx>
#include <sfx2/basedlgs.hxx>
#include <map>
#include <tools/shl.hxx> //add CHINA001
#include <tools/debug.hxx> //add CHINA001
#include <vcl/sound.hxx> //add CHINA001
#include <vcl/svapp.hxx> //add CHINA001
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx> //add CHINA001
#endif
#include <vcl/fixed.hxx> //add CHINA001
#include <vcl/lstbox.hxx> //add CHINA001
#include <vcl/edit.hxx> //add CHINA001
#include <vcl/metric.hxx>
#include "svx/svxdllapi.h"

class SubsetMap;
class SvxCharMapData;

// define ----------------------------------------------------------------

#define CHARMAP_MAXLEN  32
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

    void            SelectCharacter( sal_uInt32 cNew, BOOL bFocus = FALSE );
    sal_UCS4        GetSelectCharacter() const;

    Link            GetDoubleClickHdl() const { return aDoubleClkHdl; }
    void            SetDoubleClickHdl( const Link& rLink ) { aDoubleClkHdl = rLink; }
    Link            GetSelectHdl() const { return aSelectHdl; }
    void            SetSelectHdl( const Link& rHdl ) { aSelectHdl = rHdl; }
    Link            GetHighlightHdl() const { return aHighHdl; }
    void            SetHighlightHdl( const Link& rHdl ) { aHighHdl = rHdl; }
    Link            GetPreSelectHdl() const { return aHighHdl; }
    void            SetPreSelectHdl( const Link& rHdl ) { aPreSelectHdl = rHdl; }

#ifdef _SVX_CHARMAP_CXX_
    ::svx::SvxShowCharSetItem*  ImplGetItem( int _nPos );
    int                         FirstInView( void) const;
    int                         LastInView( void) const;
    int                         PixelToMapIndex( const Point&) const;
    void                        SelectIndex( int index, BOOL bFocus = FALSE );
    void                        DeSelect();
    inline sal_Bool             IsSelected(USHORT _nPos) const { return _nPos == nSelectedIndex; }
    inline USHORT               GetSelectIndexId() const { return sal::static_int_cast<USHORT>(nSelectedIndex); }
    USHORT                      GetRowPos(USHORT _nPos) const;
    USHORT                      GetColumnPos(USHORT _nPos) const;

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
    BOOL            bDrag;

    sal_Int32       nSelectedIndex;

    FontCharMap     maFontCharMap;
    ScrollBar       aVscrollSB;
    Size            aOrigSize;
    Point           aOrigPos;

private:
    void            DrawChars_Impl( int n1, int n2);
    void            InitSettings( BOOL bForeground, BOOL bBackground);
    // abstraction layers are: Unicode<->MapIndex<->Pixel
    Point           MapIndexToPixel( int) const;
//#if 0 // _SOLAR__PRIVATE
    DECL_LINK( VscrollHdl, ScrollBar* );
//#endif
};

// class SvxShowText =====================================================

class SVX_DLLPUBLIC SvxShowText : public Control
{
public:
                    SvxShowText( Window* pParent,
                                 const ResId& rResId,
                                 BOOL bCenter = FALSE );
                    ~SvxShowText();

    void            SetFont( const Font& rFont );
    void            SetText( const String& rText );

protected:
    virtual void    Paint( const Rectangle& );

private:
    long            mnY;
    BOOL            mbCenter;

};

class SVX_DLLPUBLIC SvxCharMapData
{
public:
                    SvxCharMapData( class SfxModalDialog* pDialog, BOOL bOne_, ResMgr* pResContext );

    void            SetCharFont( const Font& rFont );

private:
friend class SvxCharacterMap;
    SfxModalDialog* mpDialog;

    SvxShowCharSet  aShowSet;
//    Edit            aShowText;
    SvxShowText     aShowText;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;
    PushButton      aDeleteBtn;
    FixedText       aFontText;
    ListBox         aFontLB;
    FixedText       aSubsetText;
    ListBox         aSubsetLB;
    FixedText       aSymbolText;
    SvxShowText     aShowChar;
    FixedText       aCharCodeText;

    Font            aFont;
    BOOL            bOne;
    const SubsetMap* pSubsetMap;

    DECL_LINK( OKHdl, OKButton* );
    DECL_LINK( FontSelectHdl, ListBox* );
    DECL_LINK( SubsetSelectHdl, ListBox* );
    DECL_LINK( CharDoubleClickHdl, Control* pControl );
    DECL_LINK( CharSelectHdl, Control* pControl );
    DECL_LINK( CharHighlightHdl, Control* pControl );
    DECL_LINK( CharPreSelectHdl, Control* pControl );
    DECL_LINK( DeleteHdl, PushButton* pBtn );
};
#endif

