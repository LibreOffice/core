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
#include <svtools/headbar.hxx>
#include <svtools/svtabbx.hxx>
#include <vcl/lstbox.hxx>
#include "svx/svxdllapi.h"

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
    sal_uInt16              nHeaderItemId;
    sal_Bool                bResizeFlag;
    sal_Bool                bPaintFlag;
    sal_Bool                bSortDirection;
    sal_uInt16              nSortCol;
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
                            sal_uInt16 nCol=HEADERBAR_APPEND,
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
    sal_Bool            IsVisible() const;

    void            EnableTable();
    void            DisableTable();
    sal_Bool            IsEnabled() const;

    void            TableToTop();
    void            SetPosPixel( const Point& rNewPos );
    Point           GetPosPixel() const ;
    virtual void    SetPosSizePixel( const Point& rNewPos, Size& rNewSize );
    void            SetPosSize( const Point& rNewPos, const Size& rNewSize );
    void            SetSizePixel(const Size& rNewSize );
    void            SetOutputSizePixel(const Size& rNewSize );

    Size            GetSizePixel() const;
    Size            GetOutputSizePixel() const;

    sal_uInt16          GetSelectedCol();
    void            SortByCol(sal_uInt16,sal_Bool bDir=sal_True);
    sal_Bool            GetSortDirection(){ return bSortDirection;}
    sal_uInt16          GetSortedCol(){ return nSortCol;}
    SvLBoxItem*     GetEntryAtPos( SvLBoxEntry* pEntry, sal_uInt16 nPos ) const;

    CommandEvent    GetCommandEvent()const;
    inline sal_Bool IsFocusOnCellEnabled() const { return IsCellFocusEnabled(); }

    void            SetCommandHdl( const Link& rLink ) { aCommandLink = rLink; }
    const Link&     GetCommandHdl() const { return aCommandLink; }

    void            SetHeaderBarClickHdl( const Link& rLink ) { aHeaderBarClickLink = rLink; }
    const Link&     GetHeaderBarClickHdl() const { return aHeaderBarClickLink; }

    void            SetHeaderBarDblClickHdl( const Link& rLink ) { aHeaderBarDblClickLink = rLink; }
    const Link&     GetHeaderBarDblClickHdl() const { return aHeaderBarDblClickLink; }

    void            SetHeaderBarHelpId(const rtl::OString& rHelpId) {aHeaderBar.SetHelpId(rHelpId);}
};


#endif // _SVX_SIMPTABL_HXX

