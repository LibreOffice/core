/*************************************************************************
 *
 *  $RCSfile: conttree.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONTTREE_HXX
#define _CONTTREE_HXX
#ifndef _SWCONT_HXX
#include "swcont.hxx"
#endif

#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

class SwWrtShell;
class SwContentArr;
class SwContentType;
class SwNavigationPI;
class SwFmtFld;
class SwTxtINetFmt;
class SwNavigationConfig;
class SwDocShell;
class SvDataObject;
class Menu;
class ToolBox;

#define EDIT_MODE_EDIT          0
#define EDIT_MODE_UPD_IDX       1
#define EDIT_MODE_RMV_IDX       2
#define EDIT_UNPROTECT_TABLE    3
#define EDIT_MODE_DELETE        4
#define EDIT_MODE_RENAME        5

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class SwContentTree : public SvTreeListBox
{
    SwWrtShell*         pHiddenShell;   // gedropptes Doc
    SwWrtShell*         pActiveShell;   // die aktive oder eine konst. offene View
    SwNavigationConfig* pConfig;
    ImageList           aEntryImages;
    String              sSpace;
    AutoTimer           aUpdTimer;

    USHORT              nActiveBlock;
    USHORT              nHiddenBlock;
    USHORT              nRootType;
    USHORT              nLastSelType;
    USHORT              nDragMode; // wir fuer den StaticLink gebraucht
    BYTE                nOutlineLevel;

    BOOL                bIsActive           :1;
    BOOL                bIsConstant         :1;
    BOOL                bIsHidden           :1;
    BOOL                bDocChgdInDragging  :1;
    BOOL                bIsInternalDrag     :1;
    BOOL                bIsRoot             :1;
    BOOL                bIsIdleClear        :1;
    BOOL                bIsLastReadOnly     :1;
    BOOL                bIsOutlineMoveable  :1;
    BOOL                bViewHasChanged     :1;
    BOOL                bIsImageListInitialized : 1;

    static BOOL         bIsInDrag;

    SwContentType*      aActiveContentArr[CONTENT_TYPE_MAX];
    SwContentType*      aHiddenContentArr[CONTENT_TYPE_MAX];
    String              aContextStrings[CONTEXT_COUNT + 1];
    String              sRemoveIdx;
    String              sUpdateIdx;
    String              sUnprotTbl;
    String              sRename;
    String              sReadonlyIdx;
    String              sInvisible;

    void                FindActiveTypeAndRemoveUserData();

protected:
    virtual BOOL    Drop( const DropEvent& rEvt);
    virtual BOOL    QueryDrop( DropEvent& rEvt);
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    RequestHelp( const HelpEvent& rHEvt );
    virtual void    InitEntry(SvLBoxEntry*,const XubString&,const Image&,const Image&);

    SwNavigationPI* GetParentWindow(){return
                        (SwNavigationPI*)Window::GetParent();}

    void            StartExecuteDrag();
    BOOL            FillDragServer(USHORT& nDragMode);
    BOOL            HasContentChanged();

    virtual DragDropMode    NotifyBeginDrag( SvLBoxEntry* );

    virtual BOOL    NotifyMoving(   SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    ULONG&        rNewChildPos
                                );
    virtual BOOL    NotifyCopying(  SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    ULONG&        rNewChildPos
                                );
    virtual BOOL    NotifyQueryDrop( SvLBoxEntry* );
    virtual void    BeginDrag( const Point& );
    virtual void    EndDrag();
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );

    void            EditEntry(SvLBoxEntry* pEntry, BYTE nMode = EDIT_MODE_EDIT);
    void            GotoContent(SwContent* pCnt);
    static void     SetInDrag(BOOL bSet) {bIsInDrag = bSet;}

    USHORT          GetDragMode() const {return nDragMode;}
    DECL_STATIC_LINK(SwContentTree, ExecDragHdl, SwContentTree*);

public:
    SwContentTree(Window* pParent, const ResId& rResId);
    ~SwContentTree();

    BOOL            ToggleToRoot();
    BOOL            IsRoot() const {return bIsRoot;}
    USHORT          GetRootType() const {return nRootType;}
    void            SetRootType(USHORT nType);
    void            Display( BOOL bActiveView );
    void            Clear();
    void            SetHiddenShell(SwWrtShell* pSh);
    void            ShowHiddenShell();
    void            ShowActualView();
    void            SetActiveShell(SwWrtShell* pSh);
    void            SetConstantShell(SwWrtShell* pSh);

    SwWrtShell*     GetWrtShell()
                        {return bIsActive||bIsConstant ?
                                    pActiveShell :
                                        pHiddenShell;}

    static BOOL     IsInDrag() {return bIsInDrag;}
    BOOL            IsInternalDrag() const {return bIsInternalDrag != 0;}

    USHORT          GetActiveBlock() const {return nActiveBlock;}

    BYTE            GetOutlineLevel()const {return nOutlineLevel;}
    void            SetOutlineLevel(BYTE nSet);
    BOOL            Expand( SvLBoxEntry* pParent );
    BOOL            Collapse( SvLBoxEntry* pParent );
    void            ExecCommand(USHORT nCmd, BOOL bModifier);
    void            Show();
    void            Hide();

    BOOL            IsConstantView() {return bIsConstant;}
    BOOL            IsActiveView()   {return bIsActive;}
    BOOL            IsHiddenView()   {return bIsHidden;}

    const SwWrtShell*   GetActiveWrtShell() {return pActiveShell;}
    SwWrtShell*         GetHiddenWrtShell() {return pHiddenShell;}

    DECL_LINK( ContentDoubleClickHdl, SwContentTree * );
    DECL_LINK( PopupHdl, Menu* );
    DECL_LINK( TimerUpdate, Timer * );

    virtual long    GetTabPos( SvLBoxEntry*, SvLBoxTab* );
    virtual void    RequestingChilds( SvLBoxEntry* pParent );
    virtual void    GetFocus();
    virtual void    KeyInput(const KeyEvent& rKEvt);
    virtual BOOL    Select( SvLBoxEntry* pEntry, BOOL bSelect=TRUE );
};


//----------------------------------------------------------------------------
// TreeListBox fuer Globaldokumente
//----------------------------------------------------------------------------

class SwGlblDocContents;
class SwGlblDocContent;

class SwLBoxString : public SvLBoxString
{
public:

    SwLBoxString( SvLBoxEntry* pEntry, USHORT nFlags,
        const String& rStr ) : SvLBoxString(pEntry,nFlags,rStr)
    {
    }

    virtual void Paint( const Point& rPos, SvLBox& rDev, USHORT nFlags,
        SvLBoxEntry* pEntry);
};

class SwGlobalTree : public SvTreeListBox
{
    AutoTimer           aUpdateTimer;
    String              aContextStrings[GLOBAL_CONTEXT_COUNT];

    ImageList           aEntryImages;

    SwWrtShell*         pActiveShell;   //
    SvLBoxEntry*        pEmphasisEntry; // Drag'n Drop-Emphasis
    SvLBoxEntry*        pDDSource;      // Quelle beim DnD
    SwGlblDocContents*  pSwGlblDocContents; // Array mit sortierten Inhalten

    BOOL                bIsInternalDrag     :1;
    BOOL                bLastEntryEmphasis  :1; // Drag'n Drop
    BOOL                bIsImageListInitialized : 1;

    static const SfxObjectShell* pShowShell;

    protected:
        virtual BOOL    Drop( const DropEvent& rEvt);
        virtual BOOL    QueryDrop( DropEvent& rEvt);
        virtual void    Command( const CommandEvent& rCEvt );
        virtual void    RequestHelp( const HelpEvent& rHEvt );

        virtual long    GetTabPos( SvLBoxEntry*, SvLBoxTab* );
        virtual DragDropMode   NotifyBeginDrag( SvLBoxEntry* );
        virtual BOOL    NotifyMoving(   SvLBoxEntry*  pTarget,
                                        SvLBoxEntry*  pEntry,
                                        SvLBoxEntry*& rpNewParent,
                                        ULONG&        rNewChildPos
                                    );
        virtual BOOL    NotifyCopying(  SvLBoxEntry*  pTarget,
                                        SvLBoxEntry*  pEntry,
                                        SvLBoxEntry*& rpNewParent,
                                        ULONG&        rNewChildPos
                                    );
        virtual BOOL    NotifyQueryDrop( SvLBoxEntry* );
        virtual void    BeginDrag( const Point& );
        virtual void    EndDrag();
        virtual void    MouseButtonDown( const MouseEvent& rMEvt );
        virtual void    KeyInput(const KeyEvent& rKEvt);
        virtual void    GetFocus();
        virtual void    SelectHdl();
        virtual void    DeselectHdl();
        virtual void InitEntry(SvLBoxEntry*,const XubString&,const Image&,const Image&);

        void            Clear();

        DECL_LINK(      PopupHdl, Menu* );
        DECL_LINK(      Timeout, Timer* );
        DECL_LINK(      DoubleClickHdl, SwGlobalTree * );

        BOOL            IsInternalDrag() const {return bIsInternalDrag != 0;}
        SwNavigationPI* GetParentWindow(){return
                            (SwNavigationPI*)Window::GetParent();}

        void            OpenDoc(const SwGlblDocContent*);
        void            GotoContent(const SwGlblDocContent*);
        USHORT          GetEnableFlags() const;

    static const SfxObjectShell*    GetShowShell() {return pShowShell;}
    static void         SetShowShell(const SfxObjectShell*pSet) {pShowShell = pSet;}
    DECL_STATIC_LINK(SwGlobalTree, ShowFrameHdl, SwGlobalTree*);

    public:
        SwGlobalTree(Window* pParent, const ResId& rResId);
        ~SwGlobalTree();

    void                TbxMenuHdl(USHORT nTbxId, ToolBox* pBox);
    void                InsertRegion(const SwGlblDocContent* pCont,
                                        const String* pFileName = 0 );
    void                EditContent(const SwGlblDocContent* pCont );
    void                Show();
    void                Hide();

    void                ExecCommand(USHORT nCmd);

    void                Display(BOOL bOnlyUpdateUserData = FALSE);
    BOOL                Update(BOOL bHard = FALSE);
};



#endif


