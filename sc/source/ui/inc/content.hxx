/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SC_CONTENT_HXX
#define SC_CONTENT_HXX

#include <svtools/svtreebx.hxx>
#include "global.hxx"
#include "address.hxx"
#include <tools/solar.h>

class ScNavigatorDlg;
class ScNavigatorSettings;
class ScDocument;
class ScDocShell;
class ScAreaLink;

#define SC_CONTENT_ROOT         0
#define SC_CONTENT_TABLE        1
#define SC_CONTENT_RANGENAME    2
#define SC_CONTENT_DBAREA       3
#define SC_CONTENT_GRAPHIC      4
#define SC_CONTENT_OLEOBJECT    5
#define SC_CONTENT_NOTE         6
#define SC_CONTENT_AREALINK     7
#define SC_CONTENT_DRAWING      8
#define SC_CONTENT_COUNT        9

const ULONG SC_CONTENT_NOCHILD  = ~0UL;

//
//  TreeListBox fuer Inhalte
//

class ScContentTree : public SvTreeListBox
{
    ScNavigatorDlg*     pParentWindow;
    ImageList           aEntryImages;
    SvLBoxEntry*        pRootNodes[SC_CONTENT_COUNT];
    USHORT              nRootType;          // als Root eingestellt
    String              aManualDoc;         // im Navigator umgeschaltet (Title)
    BOOL                bHiddenDoc;         // verstecktes aktiv?
    String              aHiddenName;        // URL zum Laden
    String              aHiddenTitle;       // fuer Anzeige
    ScDocument*         pHiddenDocument;    // temporaer

    USHORT              pPosList[SC_CONTENT_COUNT];     // fuer die Reihenfolge

    static BOOL bIsInDrag;      // static, falls der Navigator im ExecuteDrag geloescht wird

    ScDocShell* GetManualOrCurrent();

    void    InitRoot(USHORT nType);
    void    ClearType(USHORT nType);
    void    ClearAll();
    void    InsertContent( USHORT nType, const String& rValue );
    void    GetDrawNames( USHORT nType );

    void    GetTableNames();
    void    GetAreaNames();
    void    GetDbNames();
    void    GetLinkNames();
    void    GetGraphicNames();
    void    GetOleNames();
    void    GetDrawingNames();
    void    GetNoteStrings();

    static bool IsPartOfType( USHORT nContentType, USHORT nObjIdentifier );

    BOOL    DrawNamesChanged( USHORT nType );
    BOOL    NoteStringsChanged();

    ScAddress GetNotePos( ULONG nIndex );
    const ScAreaLink* GetLink( ULONG nIndex );

    /** Returns the indexes of the specified listbox entry.
        @param rnRootIndex  Root index of specified entry is returned.
        @param rnChildIndex  Index of the entry inside its root is returned (or SC_CONTENT_NOCHILD if entry is root).
        @param pEntry  The entry to examine. */
    void    GetEntryIndexes( USHORT& rnRootIndex, ULONG& rnChildIndex, SvLBoxEntry* pEntry ) const;

    /** Returns the child index of the specified listbox entry.
        @param pEntry  The entry to examine or NULL for the selected entry.
        @return  Index of the entry inside its root or SC_CONTENT_NOCHILD if entry is root. */
    ULONG   GetChildIndex( SvLBoxEntry* pEntry ) const;

    void    DoDrag();

    ScDocument* GetSourceDocument();

    DECL_LINK( ContentDoubleClickHdl, ScContentTree* );
    DECL_STATIC_LINK( ScContentTree, ExecDragHdl, void* );

protected:
//  virtual BOOL    Drop( const DropEvent& rEvt );
//  virtual BOOL    QueryDrop( DropEvent& rEvt );

    using SvTreeListBox::ExecuteDrop;

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );
    virtual void        DragFinished( sal_Int8 nAction );

    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    RequestHelp( const HelpEvent& rHEvt );

public:
            ScContentTree( Window* pParent, const ResId& rResId );
            ~ScContentTree();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );

    void    InitWindowBits( BOOL bButtons );

    void    Refresh( USHORT nType = 0 );

    void    ToggleRoot();
    void    SetRootType( USHORT nNew );
    USHORT  GetRootType() const             { return nRootType; }

    void    ActiveDocChanged();
    void    ResetManualDoc();
    void    SetManualDoc(const String& rName);
    BOOL    LoadFile(const String& rUrl);
    void    SelectDoc(const String& rName);

    const String& GetHiddenTitle() const    { return aHiddenTitle; }

    /** Applies the navigator settings to the listbox. */
    void                        ApplySettings();
    /** Stores the current listbox state in the navigator settings. */
    void                        StoreSettings() const;

    static BOOL IsInDrag()  { return bIsInDrag; }
};




#endif // SC_NAVIPI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
