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

const sal_uLong SC_CONTENT_NOCHILD  = ~0UL;

class ScContentTree : public SvTreeListBox
{
    ScNavigatorDlg*     pParentWindow;
    ImageList           aEntryImages;
    SvLBoxEntry*        pRootNodes[SC_CONTENT_COUNT];
    sal_uInt16              nRootType;          // set as Root
    String              aManualDoc;         // Switched in Navigator (Title)
    sal_Bool                bHiddenDoc;         // Hidden active?
    String              aHiddenName;        // URL to load
    String              aHiddenTitle;       // for display
    ScDocument*         pHiddenDocument;    // temporary

    sal_uInt16              pPosList[SC_CONTENT_COUNT];     // for the sequence

    static sal_Bool bIsInDrag;      // static, if the Navigator is deleted in ExecuteDrag

    ScDocShell* GetManualOrCurrent();

    void    InitRoot(sal_uInt16 nType);
    void    ClearType(sal_uInt16 nType);
    void    ClearAll();
    void    InsertContent( sal_uInt16 nType, const String& rValue );
    void    GetDrawNames( sal_uInt16 nType );

    void    GetTableNames();
    void    GetAreaNames();
    void    GetDbNames();
    void    GetLinkNames();
    void    GetGraphicNames();
    void    GetOleNames();
    void    GetDrawingNames();
    void    GetNoteStrings();

    static bool IsPartOfType( sal_uInt16 nContentType, sal_uInt16 nObjIdentifier );

    sal_Bool    DrawNamesChanged( sal_uInt16 nType );
    sal_Bool    NoteStringsChanged();

    ScAddress GetNotePos( sal_uLong nIndex );
    const ScAreaLink* GetLink( sal_uLong nIndex );

    /** Returns the indexes of the specified listbox entry.
        @param rnRootIndex  Root index of specified entry is returned.
        @param rnChildIndex  Index of the entry inside its root is returned (or SC_CONTENT_NOCHILD if entry is root).
        @param pEntry  The entry to examine. */
    void    GetEntryIndexes( sal_uInt16& rnRootIndex, sal_uLong& rnChildIndex, SvLBoxEntry* pEntry ) const;

    /** Returns the child index of the specified listbox entry.
        @param pEntry  The entry to examine or NULL for the selected entry.
        @return  Index of the entry inside its root or SC_CONTENT_NOCHILD if entry is root. */
    sal_uLong   GetChildIndex( SvLBoxEntry* pEntry ) const;

    void    DoDrag();

    ScDocument* GetSourceDocument();

    DECL_LINK( ContentDoubleClickHdl, void* );
    DECL_STATIC_LINK( ScContentTree, ExecDragHdl, void* );

protected:
//  virtual sal_Bool    Drop( const DropEvent& rEvt );
//  virtual sal_Bool    QueryDrop( DropEvent& rEvt );

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

    void    InitWindowBits( sal_Bool bButtons );

    void    Refresh( sal_uInt16 nType = 0 );

    void    ToggleRoot();
    void    SetRootType( sal_uInt16 nNew );
    sal_uInt16  GetRootType() const             { return nRootType; }

    void    ActiveDocChanged();
    void    ResetManualDoc();
    void    SetManualDoc(const String& rName);
    sal_Bool    LoadFile(const String& rUrl);
    void    SelectDoc(const String& rName);

    const String& GetHiddenTitle() const    { return aHiddenTitle; }

    /** Applies the navigator settings to the listbox. */
    void                        ApplySettings();
    /** Stores the current listbox state in the navigator settings. */
    void                        StoreSettings() const;

    static sal_Bool IsInDrag()  { return bIsInDrag; }
};




#endif // SC_NAVIPI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
