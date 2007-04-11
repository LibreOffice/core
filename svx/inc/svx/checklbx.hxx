/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: checklbx.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:41:06 $
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
#ifndef _SVX_CHECKLBX_HXX
#define _SVX_CHECKLBX_HXX

// include ---------------------------------------------------------------


#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#ifndef _SVLBOXITM_HXX //autogen
#include <svtools/svlbitm.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// class SvxCheckListBox -------------------------------------------------

class SVX_DLLPUBLIC SvxCheckListBox : public SvTreeListBox
{
    using Window::GetText;

private:
    SvLBoxButtonData*   pCheckButton;

#ifdef _SVX_CHECKLBX_CXX
    SVX_DLLPRIVATE void             Init_Impl();
#endif

    using SvTreeListBox::InsertEntry;
    // Avoid ambiguity with new InsertEntry:
    virtual SvLBoxEntry*    InsertEntry( const XubString& rText, SvLBoxEntry* pParent,
                                         BOOL bChildsOnDemand,
                                         ULONG nPos, void* pUserData,
                                         SvLBoxButtonKind eButtonKind );

public:
    SvxCheckListBox( Window* pParent, WinBits nWinStyle = 0 );
    SvxCheckListBox( Window* pParent, const ResId& rResId );
    SvxCheckListBox( Window* pParent, const ResId& rResId,
                     const Image& rNormalStaticImage,
                     const Image& rHighContrastStaticImage );
    ~SvxCheckListBox();

    void            InsertEntry         ( const String& rStr,
                                          USHORT nPos = LISTBOX_APPEND,
                                          void* pUserData = NULL,
                                          SvLBoxButtonKind eButtonKind =
                                              SvLBoxButtonKind_enabledCheckbox );
    void            RemoveEntry         ( USHORT nPos );

    void            SelectEntryPos      ( USHORT nPos, BOOL bSelect = TRUE );
    USHORT          GetSelectEntryPos   () const;

    String          GetText             ( USHORT nPos ) const;
    USHORT          GetCheckedEntryCount() const;
    void            CheckEntryPos       ( USHORT nPos, BOOL bCheck = TRUE );
    BOOL            IsChecked           ( USHORT nPos ) const;
    void            ToggleCheckButton   ( SvLBoxEntry* pEntry );

    void*           SetEntryData        ( USHORT nPos, void* pNewData );
    void*           GetEntryData        ( USHORT nPos ) const;

    virtual void    MouseButtonDown     ( const MouseEvent& rMEvt );
    virtual void    KeyInput            ( const KeyEvent& rKEvt );
};


#endif

