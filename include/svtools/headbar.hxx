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

#ifndef INCLUDED_SVTOOLS_HEADBAR_HXX
#define INCLUDED_SVTOOLS_HEADBAR_HXX

#include <svtools/svtdllapi.h>
#include <tools/link.hxx>
#include <vcl/window.hxx>

/*************************************************************************

Description
============

class HeaderBar

This class serves for displaying a header bar. A header bar can display
texts, images or both of them. The items can be changed in size, dragged or
clicked at. In many cases, it makes, for example, sense to use this control
in combination with a SvTabListBox.

--------------------------------------------------------------------------

WinBits

WB_BORDER           a border is drawn in the top and in the bottom
WB_BOTTOMBORDER     a border is drawn in the bottom
WB_BUTTONSTYLE      The items look like buttons, otherwise they are flat.
WB_3DLOOK           3D look
WB_DRAG             items can be dragged
WB_STDHEADERBAR     WB_BUTTONSTYLE | WB_BOTTOMBORDER

--------------------------------------------------------------------------

ItemBits

HIB_LEFT            content is displayed in the item left-justified
HIB_CENTER          content is displayed in the item centred
HIB_RIGHT           content is displayed in the item right-justified
HIB_TOP             content is displayed in the item at the upper border
HIB_VCENTER         content is displayed in the item vertically centred
HIB_BOTTOM          content is displayed in the item at the bottom border
HIB_LEFTIMAGE       in case of text and image, the image is displayed left of the text
HIB_RIGHTIMAGE      in case of text and image, the image is displayed right of the text
HIB_FIXED           item cannot be changed in size
HIB_FIXEDPOS        item cannot be moved
HIB_CLICKABLE       item is clickable
                    (select handler is only called on MouseButtonUp)
HIB_FLAT            item is displayed in a flat way, even if WB_BUTTONSTYLE is set
HIB_DOWNARROW       An arrow pointing downwards is displayed behind the text,
                    which should, for example, be shown, when after this item,
                    a corresponding list is sorted in descending order.
                    The status of the arrow can be set/reset with SetItemBits().
HIB_UPARROW         An arrow pointing upwards is displayed behind the text,
                    which should, for example, be shown, when after this item,
                    a corresponding list is sorted in ascending order.
                    The status of the arrow can be set/reset with SetItemBits().
HIB_USERDRAW        For this item, the UserDraw handler is called as well.
HIB_STDSTYLE        (HIB_LEFT | HIB_LEFTIMAGE | HIB_VCENTER | HIB_CLICKABLE)

--------------------------------------------------------------------------

Handler

Select()            Is called, when the item is clicked. If HIB_CLICKABLE
                    is set in the item and not HIB_FLAT, the handler is only
                    called in the MouseButtonUp handler, when the mouse has been
                    released over the item. In this case, the Select handler
                    behaves like it does with a ToolBox button.
DoubleClick()       This handler is called, when an item is double-clicked.
                    Whether the item or the separator has been clicked, can
                    be determined by IsItemMode(). Normally, when a separator
                    is double-clicked, the optimal column width should be
                    calculated and should be set.
StartDrag()         This handler is called, when dragging is started resp.
                    an item has been clicked. At the latest in this handler,
                    the size of the size-line should be set with
                    SetDragSize(), if IsItemMode() returns false.
Drag()              This handler is callled, when dragging is taking place.
                    If no size is set with SetDragSize(), this handler can
                    be used to draw the line in the neighbouring window by
                    oneself. The current dragging position can be requested
                    with GetDragPos(). In every case, IsItemMode()
                    should be checked to find out whether a separator is
                    dragged as well.
EndDrag()           This handler is called, when a dragging process has been
                    stopped. If GetCurItemId() returns 0 in the EndDrag handler,
                    the drag process was aborted. If this is not the case and
                    IsItemMode() returns false, the new size of the dragged
                    item should be requested using GetItemSize() and it
                    should be taken over in the corresponding control.
                    If IsItemMode() returns true, GetCurItemId()
                    returns an Id and IsItemDrag() returns true, this
                    item has been dragged. In this case, the new position
                    should be requested using  GetItemPos() and the data
                    in the corresponding control should be adapted.
                    Otherwise, the position to which the item has been dragged
                    could also be requested with GetItemDragPos().

Further methods that are important for the handler.

GetCurItemId()      Returns the id of the item, for which the handler has
                    currently been called. Only returns a valid id in the
                    handlers Select(), DoubleClick(), StartDrag(),
                    Drag() and EndDrag(). In the EndDrag handler,
                    this method returns the id of the dragged item or 0,
                    if the drag process has been aborted.
GetItemDragPos()    Returns the position, at which an item has been moved.
                    HEADERBAR_ITEM_NOTFOUND is returned, if the process
                    has been aborted or no ItemDrag is active.
IsItemMode()        This method can be used to determine whether the
                    handler has been called for an item or a separator.
                    true    - handler was called for the item
                    false   - handler was called for the separator
IsItemDrag()        This method can be used to determine whether an item
                    has been dragged or selected.
                    true    - item is dragged
                    false   - item is selected
SetDragSize()       This method is used to set the size of the separating
                    line that is drawn by the control. It should be
                    equivalent to the height of the neighbouring window.
                    The height of the HeaderBar is added automatically.

--------------------------------------------------------------------------

Further methods

SetOffset()             This method sets the offset, from which on the
                        items are shown. This is needed when the
                        corresponding window is scrolled.
CalcWindowSizePixel()   This method can be used to calculate the height
                        of the window, so that the content of the item
                        can be displayed.

--------------------------------------------------------------------------

Tips and tricks:

1) ContextMenu
If a context sensitive PopupMenu should be shown, the command
handler must be overlaid. Using GetItemId() and when passing the
mous position, it can be determined whether the mouse click has been
carried out over an item resp. over which item the mouse click has been
carried out.

2) last item
If ButtonStyle has been set, it looks better, if an empty item is
set at the end which takes up the remaining space.
In order to do that, you can insert an item with an empty string and
pass HEADERBAR_FULLSIZE as size. For such an item, you should not set
HIB_CLICKABLE, but HIB_FIXEDPOS.

*************************************************************************/

class Accelerator;
class ImplHeadItem;

typedef ::std::vector< ImplHeadItem* > ImplHeadItemList;


// - WinBits -


#define WB_BOTTOMBORDER         ((WinBits)0x0400)
#define WB_BUTTONSTYLE          ((WinBits)0x0800)
#define WB_STDHEADERBAR         (WB_BUTTONSTYLE | WB_BOTTOMBORDER)


// - HeaderBarItemBits -


typedef sal_uInt16 HeaderBarItemBits;


// - Bits for HeaderBarItems -


#define HIB_LEFT                ((HeaderBarItemBits)0x0001)
#define HIB_CENTER              ((HeaderBarItemBits)0x0002)
#define HIB_RIGHT               ((HeaderBarItemBits)0x0004)
#define HIB_TOP                 ((HeaderBarItemBits)0x0008)
#define HIB_VCENTER             ((HeaderBarItemBits)0x0010)
#define HIB_BOTTOM              ((HeaderBarItemBits)0x0020)
#define HIB_LEFTIMAGE           ((HeaderBarItemBits)0x0040)
#define HIB_RIGHTIMAGE          ((HeaderBarItemBits)0x0080)
#define HIB_FIXED               ((HeaderBarItemBits)0x0100)
#define HIB_FIXEDPOS            ((HeaderBarItemBits)0x0200)
#define HIB_CLICKABLE           ((HeaderBarItemBits)0x0400)
#define HIB_FLAT                ((HeaderBarItemBits)0x0800)
#define HIB_DOWNARROW           ((HeaderBarItemBits)0x1000)
#define HIB_UPARROW             ((HeaderBarItemBits)0x2000)
#define HIB_USERDRAW            ((HeaderBarItemBits)0x4000)
#define HIB_STDSTYLE            (HIB_LEFT | HIB_LEFTIMAGE | HIB_VCENTER | HIB_CLICKABLE)


// - HeaderBar-Types -


#define HEADERBAR_APPEND            ((sal_uInt16)0xFFFF)
#define HEADERBAR_ITEM_NOTFOUND     ((sal_uInt16)0xFFFF)
#define HEADERBAR_FULLSIZE          ((long)1000000000)

#define HEADERBAR_TEXTOFF           2


// - HeaderBar -


class VCLXHeaderBar;

class SVT_DLLPUBLIC HeaderBar : public vcl::Window
{
private:
    ImplHeadItemList*   mpItemList;
    long                mnBorderOff1;
    long                mnBorderOff2;
    long                mnOffset;
    long                mnDX;
    long                mnDY;
    long                mnDragSize;
    long                mnStartPos;
    long                mnDragPos;
    long                mnMouseOff;
    sal_uInt16          mnCurItemId;
    sal_uInt16          mnItemDragPos;
    bool                mbDragable;
    bool                mbDrag;
    bool                mbItemDrag;
    bool                mbOutDrag;
    bool                mbButtonStyle;
    bool                mbItemMode;
    Link                maStartDragHdl;
    Link                maDragHdl;
    Link                maEndDragHdl;
    Link                maSelectHdl;
    Link                maDoubleClickHdl;
    Link                maCreateAccessibleHdl;

    VCLXHeaderBar*      m_pVCLXHeaderBar;


    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
                        mxAccessible;

    using Window::ImplInit;
    SVT_DLLPRIVATE void             ImplInit( WinBits nWinStyle );
    SVT_DLLPRIVATE void             ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    SVT_DLLPRIVATE long             ImplGetItemPos( sal_uInt16 nPos ) const;
    SVT_DLLPRIVATE Rectangle            ImplGetItemRect( sal_uInt16 nPos ) const;
    using Window::ImplHitTest;
    SVT_DLLPRIVATE sal_uInt16               ImplHitTest( const Point& rPos, long& nMouseOff, sal_uInt16& nPos ) const;
    SVT_DLLPRIVATE void             ImplInvertDrag( sal_uInt16 nStartPos, sal_uInt16 nEndPos );
    SVT_DLLPRIVATE void             ImplDrawItem( OutputDevice* pDev,
                                      sal_uInt16 nPos, bool bHigh, bool bDrag,
                                      const Rectangle& rItemRect,
                                      const Rectangle* pRect,
                                      sal_uLong nFlags );
    SVT_DLLPRIVATE void             ImplDrawItem( sal_uInt16 nPos, bool bHigh = false,
                                      bool bDrag = false,
                                      const Rectangle* pRect = NULL );
    SVT_DLLPRIVATE void             ImplUpdate( sal_uInt16 nPos,
                                       bool bEnd = false, bool bDirect = false );
    SVT_DLLPRIVATE void             ImplStartDrag( const Point& rPos, bool bCommand );
    SVT_DLLPRIVATE void             ImplDrag( const Point& rPos );
    SVT_DLLPRIVATE void             ImplEndDrag( bool bCancel );

public:
    HeaderBar( vcl::Window* pParent, WinBits nWinBits = WB_STDHEADERBAR );
    virtual ~HeaderBar();

    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void        MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void        Tracking( const TrackingEvent& rTEvt ) SAL_OVERRIDE;
    virtual void        Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void        Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags ) SAL_OVERRIDE;
    virtual void        Resize() SAL_OVERRIDE;
    virtual void        Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
    virtual void        RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;
    virtual void        StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    void        UserDraw( const UserDrawEvent& rUDEvt );
    void        StartDrag();
    void        Drag();
    virtual void        EndDrag();
    virtual void        Select();
    virtual void        DoubleClick();

    void                InsertItem( sal_uInt16 nItemId, const OUString& rText,
                                    long nSize, HeaderBarItemBits nBits = HIB_STDSTYLE,
                                    sal_uInt16 nPos = HEADERBAR_APPEND );
    void                RemoveItem( sal_uInt16 nItemId );
    void                MoveItem( sal_uInt16 nItemId, sal_uInt16 nNewPos );
    void                Clear();

    void                SetOffset( long nNewOffset = 0 );
    long                GetOffset() const { return mnOffset; }
    inline void         SetDragSize( long nNewSize = 0 ) { mnDragSize = nNewSize; }
    long                GetDragSize() const { return mnDragSize; }

    sal_uInt16          GetItemCount() const;
    sal_uInt16          GetItemPos( sal_uInt16 nItemId ) const;
    sal_uInt16          GetItemId( sal_uInt16 nPos ) const;
    sal_uInt16          GetItemId( const Point& rPos ) const;
    Rectangle           GetItemRect( sal_uInt16 nItemId ) const;
    sal_uInt16          GetCurItemId() const { return mnCurItemId; }
    long                GetDragPos() const { return mnDragPos; }
    sal_uInt16          GetItemDragPos() const { return mnItemDragPos; }
    bool                IsItemMode() const { return mbItemMode; }
    bool                IsItemDrag() const { return mbItemDrag; }

    void                SetItemSize( sal_uInt16 nItemId, long nNewSize );
    long                GetItemSize( sal_uInt16 nItemId ) const;
    void                SetItemBits( sal_uInt16 nItemId, HeaderBarItemBits nNewBits );
    HeaderBarItemBits   GetItemBits( sal_uInt16 nItemId ) const;

    void                SetItemText( sal_uInt16 nItemId, const OUString& rText );
    OUString            GetItemText( sal_uInt16 nItemId ) const;

    OUString            GetHelpText( sal_uInt16 nItemId ) const;
    OString             GetHelpId( sal_uInt16 nItemId ) const;

    Size                CalcWindowSizePixel() const;

    inline void             SetHelpText( const OUString& rText )    { Window::SetHelpText( rText ); }
    inline const OUString&  GetHelpText() const                     { return Window::GetHelpText(); }
    inline void             SetHelpId( const OString& rId )    { Window::SetHelpId( rId ); }
    inline const OString& GetHelpId() const                    { return Window::GetHelpId(); }

    inline void         SetStartDragHdl( const Link& rLink )        { maStartDragHdl = rLink; }
    inline const Link&  GetStartDragHdl() const                     { return maStartDragHdl; }
    inline void         SetDragHdl( const Link& rLink )             { maDragHdl = rLink; }
    inline const Link&  GetDragHdl() const                          { return maDragHdl; }
    inline void         SetEndDragHdl( const Link& rLink )          { maEndDragHdl = rLink; }
    inline const Link&  GetEndDragHdl() const                       { return maEndDragHdl; }
    inline void         SetSelectHdl( const Link& rLink )           { maSelectHdl = rLink; }
    inline const Link&  GetSelectHdl() const                        { return maSelectHdl; }
    inline void         SetDoubleClickHdl( const Link& rLink )      { maDoubleClickHdl = rLink; }
    inline const Link&  GetDoubleClickHdl() const                   { return maDoubleClickHdl; }
    inline void         SetCreateAccessibleHdl( const Link& rLink ) { maCreateAccessibleHdl = rLink; }
    inline const Link&  GetCreateAccessibleHdl() const              { return maCreateAccessibleHdl; }

    inline bool         IsDragable() const                          { return mbDragable; }

    /** Creates and returns the accessible object of the header bar. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >  CreateAccessible() SAL_OVERRIDE;
    void SetAccessible( ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > GetComponentInterface( bool bCreate ) SAL_OVERRIDE;

};

#endif // INCLUDED_SVTOOLS_HEADBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
