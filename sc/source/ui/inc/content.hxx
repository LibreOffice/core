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

#ifndef INCLUDED_SC_SOURCE_UI_INC_CONTENT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CONTENT_HXX

#include <svtools/treelistbox.hxx>
#include "global.hxx"
#include "address.hxx"
#include <tools/solar.h>

class ScNavigatorDlg;
class ScDocument;
class ScDocShell;
class ScAreaLink;
class SdrPage;

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
    VclPtr<ScNavigatorDlg>  pParentWindow;
    ImageList           aEntryImages;
    SvTreeListEntry*        pRootNodes[SC_CONTENT_COUNT];
    sal_uInt16              nRootType;          // set as Root
    OUString            aManualDoc;         // Switched in Navigator (Title)
    bool                bHiddenDoc;         // Hidden active?
    OUString            aHiddenName;        // URL to load
    OUString            aHiddenTitle;       // for display
    ScDocument*         pHiddenDocument;    // temporary
    bool                bisInNavigatoeDlg;
    OUString            sKeyString;

    sal_uInt16              pPosList[SC_CONTENT_COUNT];     // for the sequence

    static bool bIsInDrag;      // static, if the Navigator is deleted in ExecuteDrag

    ScDocShell* GetManualOrCurrent();

    void    InitRoot(sal_uInt16 nType);
    void    ClearType(sal_uInt16 nType);
    void    ClearAll();
    void    InsertContent( sal_uInt16 nType, const OUString& rValue );
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

    bool    DrawNamesChanged( sal_uInt16 nType );
    bool    NoteStringsChanged();

    ScAddress GetNotePos( sal_uLong nIndex );
    const ScAreaLink* GetLink( sal_uLong nIndex );

    /** Returns the indexes of the specified listbox entry.
        @param rnRootIndex  Root index of specified entry is returned.
        @param rnChildIndex  Index of the entry inside its root is returned (or SC_CONTENT_NOCHILD if entry is root).
        @param pEntry  The entry to examine. */
    void    GetEntryIndexes( sal_uInt16& rnRootIndex, sal_uLong& rnChildIndex, SvTreeListEntry* pEntry ) const;

    /** Returns the child index of the specified listbox entry.
        @param pEntry  The entry to examine or NULL for the selected entry.
        @return  Index of the entry inside its root or SC_CONTENT_NOCHILD if entry is root. */
    sal_uLong   GetChildIndex( SvTreeListEntry* pEntry ) const;

    void    DoDrag();

    ScDocument* GetSourceDocument();

    DECL_LINK_TYPED( ContentDoubleClickHdl, SvTreeListBox*, bool );
    DECL_LINK_TYPED( ExecDragHdl, void*, void );
public:
    SvTreeListEntry* pTmpEntry;
    bool m_bFirstPaint;

protected:

    using SvTreeListBox::ExecuteDrop;

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) SAL_OVERRIDE;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) SAL_OVERRIDE;
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel ) SAL_OVERRIDE;
    virtual void        DragFinished( sal_Int8 nAction ) SAL_OVERRIDE;

    virtual void        Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
    virtual void        RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;
    virtual void        InitEntry(SvTreeListEntry*,const OUString&,const Image&,const Image&, SvLBoxButtonKind) SAL_OVERRIDE;

public:
            ScContentTree( vcl::Window* pParent, const ResId& rResId );
            virtual ~ScContentTree();
    virtual void dispose() SAL_OVERRIDE;

    OUString getAltLongDescText(SvTreeListEntry* pEntry, bool isAltText) const;
    OUString GetEntryAltText( SvTreeListEntry* pEntry ) const SAL_OVERRIDE;
    OUString GetEntryLongDescription( SvTreeListEntry* pEntry ) const SAL_OVERRIDE;

    void     ObjectFresh( sal_uInt16 nType, SvTreeListEntry* pEntry = NULL);
    bool     SetNavigatorDlgFlag(bool isInNavigatoeDlg){ return bisInNavigatoeDlg=isInNavigatoeDlg;};
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;

    void    InitWindowBits( bool bButtons );

    void    Refresh( sal_uInt16 nType = 0 );

    void    ToggleRoot();
    void    SetRootType( sal_uInt16 nNew );
    sal_uInt16  GetRootType() const             { return nRootType; }

    void    ActiveDocChanged();
    void    ResetManualDoc();
    void    SetManualDoc(const OUString& rName);
    bool    LoadFile(const OUString& rUrl);
    void    SelectDoc(const OUString& rName);

    const OUString& GetHiddenTitle() const    { return aHiddenTitle; }

    /** Applies the navigator settings to the listbox. */
    void ApplyNavigatorSettings();
    /** Stores the current listbox state in the navigator settings. */
    void StoreNavigatorSettings() const;
};

#endif // INCLUDED_SC_SOURCE_UI_INC_CONTENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
