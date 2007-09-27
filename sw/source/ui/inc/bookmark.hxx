/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bookmark.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:54:32 $
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
#ifndef _BOOKMARK_HXX
#define _BOOKMARK_HXX

#ifndef _SVX_STDDLG_HXX //autogen
#include <svx/stddlg.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#include "swlbox.hxx"       // SwComboBox

class SwWrtShell;
class SfxRequest;

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

class BookmarkCombo : public SwComboBox
{
    USHORT          GetFirstSelEntryPos() const;
    USHORT          GetNextSelEntryPos(USHORT nPos) const;
    USHORT          GetSelEntryPos(USHORT nPos) const;

    virtual long    PreNotify(NotifyEvent& rNEvt);
public:
    BookmarkCombo( Window* pWin, const ResId& rResId );

    USHORT          GetSelectEntryCount() const;
    USHORT          GetSelectEntryPos( USHORT nSelIndex = 0 ) const;

    static const String aForbiddenChars;
};

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

class SwInsertBookmarkDlg: public SvxStandardDialog
{
    BookmarkCombo   aBookmarkBox;
    FixedLine       aBookmarkFl;
    OKButton        aOkBtn;
    CancelButton    aCancelBtn;
    PushButton      aDeleteBtn;

    String          sRemoveWarning;
    SwWrtShell      &rSh;
    SfxRequest&     rReq;

    DECL_LINK( ModifyHdl, BookmarkCombo * );
    DECL_LINK( DeleteHdl, Button * );

    virtual void Apply();

public:
    SwInsertBookmarkDlg( Window *pParent, SwWrtShell &rSh, SfxRequest& rReq );
    ~SwInsertBookmarkDlg();
};

#endif
