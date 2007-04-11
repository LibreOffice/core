/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: charmap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:40:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_CHARMAP_HXX
#define _SVX_CHARMAP_HXX

// include ---------------------------------------------------------------

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif
#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif

#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif
#include <map>

#ifndef _SHL_HXX
#include <tools/shl.hxx> //add CHINA001
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx> //add CHINA001
#endif
#ifndef _SV_SOUND_HXX
#include <vcl/sound.hxx> //add CHINA001
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx> //add CHINA001
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx> //add CHINA001
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx> //add CHINA001
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx> //add CHINA001
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx> //add CHINA001
#endif
#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SubsetMap;
class SvxCharMapData;

// define ----------------------------------------------------------------

#ifdef MAC
#define CHARMAP_MAXLEN  28
#define COLUMN_COUNT    28
#define ROW_COUNT        8
#else
#define CHARMAP_MAXLEN  32
#define COLUMN_COUNT    16
#define ROW_COUNT        8
#endif

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
                    SvxCharMapData( class SfxModalDialog* pDialog, BOOL bOne_ );

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

