/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simptabl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:16:15 $
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

#ifndef _SVX_SIMPTABL_HXX
#define _SVX_SIMPTABL_HXX

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _HEADBAR_HXX //autogen
#include <svtools/headbar.hxx>
#endif

#ifndef _SVTABBX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SvxSimpleTable;
class SvxSimpTblContainer : public Control
{
private:
    SvxSimpleTable*     m_pTable;

protected:
    virtual long        PreNotify( NotifyEvent& rNEvt );

public:
    SvxSimpTblContainer( Window* pParent, WinBits nWinStyle = 0 );
    SvxSimpTblContainer( Window* pParent, const ResId& rResId );

    inline void         SetTable( SvxSimpleTable* _pTable ) { m_pTable = _pTable; }
};

class SVX_DLLPUBLIC SvxSimpleTable : public SvHeaderTabListBox
{
    using Window::SetPosSizePixel;
private:

    Link                aHeaderBarClickLink;
    Link                aHeaderBarDblClickLink;
    Link                aCommandLink;
    CommandEvent        aCEvt;
    SvxSimpTblContainer aPrivContainer;
    HeaderBar           aHeaderBar;
    long                nOldPos;
    USHORT              nHeaderItemId;
    BOOL                bResizeFlag;
    BOOL                bPaintFlag;
    BOOL                bSortDirection;
    USHORT              nSortCol;
    Window*             pMyParentWin;

    DECL_LINK( StartDragHdl, HeaderBar* );
    DECL_LINK( DragHdl, HeaderBar* );
    DECL_LINK( EndDragHdl, HeaderBar* );
    DECL_LINK( HeaderBarClick, HeaderBar* );
    DECL_LINK( HeaderBarDblClick, HeaderBar* );
    DECL_LINK( CompareHdl, SvSortData* );

protected:

    virtual void            NotifyScrolled();

    virtual void            SetTabs();
    virtual void            Paint( const Rectangle& rRect );
    virtual void            UpdateViewSize();

    virtual void            HBarClick();
    virtual void            HBarDblClick();
    virtual void            HBarStartDrag();
    virtual void            HBarDrag();
    virtual void            HBarEndDrag();

    virtual void            Command( const CommandEvent& rCEvt );

    virtual StringCompare   ColCompare(SvLBoxEntry*,SvLBoxEntry*);
    HeaderBar*              GetTheHeaderBar(){return &aHeaderBar;}

public:

    SvxSimpleTable( Window* pParent,WinBits nBits =WB_BORDER );
    SvxSimpleTable( Window* pParent,const ResId& );
    ~SvxSimpleTable();

    void            InsertHeaderEntry(const XubString& rText,
                            USHORT nCol=HEADERBAR_APPEND,
                            HeaderBarItemBits nBits = HIB_STDSTYLE);

    void            SetTabs( long* pTabs, MapUnit = MAP_APPFONT );

    void            ClearAll();
    void            ClearHeader();

    // to be removed all calls of the related methods are redirected to *Table() methods
    using Window::Show;
    using Window::Hide;
    using Window::Enable;
    using Window::Disable;
    using Window::ToTop;

    void            Show();
    void            Hide();
    void            Enable();
    void            Disable();
    void            ToTop();

    // remove until this line

    void            ShowTable();
    void            HideTable();
    BOOL            IsVisible() const;

    void            EnableTable();
    void            DisableTable();
    BOOL            IsEnabled() const;

    void            TableToTop();
    void            SetPosPixel( const Point& rNewPos );
    Point           GetPosPixel() const ;
    virtual void    SetPosSizePixel( const Point& rNewPos, Size& rNewSize );
    void            SetPosSize( const Point& rNewPos, const Size& rNewSize );
    void            SetSizePixel(const Size& rNewSize );
    void            SetOutputSizePixel(const Size& rNewSize );

    Size            GetSizePixel() const;
    Size            GetOutputSizePixel() const;

    USHORT          GetSelectedCol();
    void            SortByCol(USHORT,BOOL bDir=TRUE);
    BOOL            GetSortDirection(){ return bSortDirection;}
    USHORT          GetSortedCol(){ return nSortCol;}
    SvLBoxItem*     GetEntryAtPos( SvLBoxEntry* pEntry, USHORT nPos ) const;

    CommandEvent    GetCommandEvent()const;
    inline sal_Bool IsFocusOnCellEnabled() const { return IsCellFocusEnabled(); }

    void            SetCommandHdl( const Link& rLink ) { aCommandLink = rLink; }
    const Link&     GetCommandHdl() const { return aCommandLink; }

    void            SetHeaderBarClickHdl( const Link& rLink ) { aHeaderBarClickLink = rLink; }
    const Link&     GetHeaderBarClickHdl() const { return aHeaderBarClickLink; }

    void            SetHeaderBarDblClickHdl( const Link& rLink ) { aHeaderBarDblClickLink = rLink; }
    const Link&     GetHeaderBarDblClickHdl() const { return aHeaderBarDblClickLink; }

    void            SetHeaderBarHelpId(ULONG nHelpId) {aHeaderBar.SetHelpId(nHelpId);}
};


#endif // _SVX_SIMPTABL_HXX

