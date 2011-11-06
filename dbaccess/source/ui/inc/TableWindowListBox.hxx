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



