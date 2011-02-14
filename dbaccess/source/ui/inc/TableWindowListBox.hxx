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
#ifndef DBAUI_TABLEWINDOWLISTBOX_HXX
#define DBAUI_TABLEWINDOWLISTBOX_HXX

#include <svtools/svtreebx.hxx>
#include "callbacks.hxx"

struct AcceptDropEvent;
struct ExecuteDropEvent;
namespace dbaui
{
    class OTableWindowListBox;
    struct OJoinExchangeData
    {
    public:
        OTableWindowListBox*    pListBox;       // die ListBox innerhalb desselben (daraus kann man sich das TabWin und daraus den WinName besorgen)
        SvLBoxEntry*            pEntry;         // der Eintrag, der gedraggt oder auf den gedroppt wurde

        OJoinExchangeData(OTableWindowListBox* pBox);
        OJoinExchangeData() : pListBox(NULL), pEntry(NULL) { }
    };
    struct OJoinDropData
    {
        OJoinExchangeData aSource;
        OJoinExchangeData aDest;
    };

    class OTableWindow;
    class OTableWindowListBox
            :public SvTreeListBox
            ,public IDragTransferableListener
    {
        DECL_LINK( OnDoubleClick, SvTreeListBox* );
        DECL_LINK( ScrollUpHdl, SvTreeListBox* );
        DECL_LINK( ScrollDownHdl, SvTreeListBox* );
        DECL_LINK( DropHdl, void* );
        DECL_LINK( LookForUiHdl, void* );

        Timer                       m_aScrollTimer;
        Point                       m_aMousePos;

        OTableWindow*               m_pTabWin;
        sal_uLong                       m_nDropEvent;
        sal_uLong                       m_nUiEvent;
        OJoinDropData               m_aDropInfo;

        sal_Bool                        m_bReallyScrolled : 1;
        sal_Bool                        m_bDragSource : 1;

    protected:
        virtual void LoseFocus();
        virtual void GetFocus();
        virtual void NotifyScrolled();
        virtual void NotifyEndScroll();

        virtual long PreNotify(NotifyEvent& rNEvt);

        virtual void dragFinished( );



    public:
        OTableWindowListBox(OTableWindow* pParent);
        virtual ~OTableWindowListBox();

        // DnD stuff
        virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

        // window
        virtual void Command(const CommandEvent& rEvt);

        OTableWindow* GetTabWin(){ return m_pTabWin; }
        SvLBoxEntry* GetEntryFromText( const String& rEntryText );

    private:
        using SvTreeListBox::ExecuteDrop;
    };
}
#endif // DBAUI_TABLEWINDOWLISTBOX_HXX



