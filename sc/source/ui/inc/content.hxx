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
#include <o3tl/enumarray.hxx>

class ScNavigatorDlg;
class ScDocument;
class ScDocShell;
class ScAreaLink;
class SdrPage;

enum class ScContentId {
    ROOT, TABLE, RANGENAME, DBAREA,
    GRAPHIC, OLEOBJECT, NOTE, AREALINK,
    DRAWING, LAST = DRAWING
};

const sal_uLong SC_CONTENT_NOCHILD  = ~0UL;

class ScContentTree : public SvTreeListBox
{
    VclPtr<ScNavigatorDlg>  pParentWindow;
    ImageList               aEntryImages;
    o3tl::enumarray<ScContentId, SvTreeListEntry*> pRootNodes;
    ScContentId             nRootType;          // set as Root
    OUString                aManualDoc;         // Switched in Navigator (Title)
    bool                    bHiddenDoc;         // Hidden active?
    OUString                aHiddenName;        // URL to load
    OUString                aHiddenTitle;       // for display
    ScDocument*             pHiddenDocument;    // temporary
    bool                    bisInNavigatoeDlg;
    OUString                sKeyString;

    o3tl::enumarray<ScContentId, sal_uInt16> pPosList;     // for the sequence

    static bool bIsInDrag;      // static, if the Navigator is deleted in ExecuteDrag

    ScDocShell* GetManualOrCurrent();

    void    InitRoot(ScContentId nType);
    void    ClearType(ScContentId nType);
    void    ClearAll();
    void    InsertContent( ScContentId nType, const OUString& rValue );
    void    GetDrawNames( ScContentId nType );

    void    GetTableNames();
    void    GetAreaNames();
    void    GetDbNames();
    void    GetLinkNames();
    void    GetGraphicNames();
    void    GetOleNames();
    void    GetDrawingNames();
    void    GetNoteStrings();

    static bool IsPartOfType( ScContentId nContentType, sal_uInt16 nObjIdentifier );

    bool    DrawNamesChanged( ScContentId nType );
    bool    NoteStringsChanged();

    ScAddress GetNotePos( sal_uLong nIndex );
    const ScAreaLink* GetLink( sal_uLong nIndex );

    /** Returns the indexes of the specified listbox entry.
        @param rnRootIndex  Root index of specified entry is returned.
        @param rnChildIndex  Index of the entry inside its root is returned (or SC_CONTENT_NOCHILD if entry is root).
        @param pEntry  The entry to examine. */
    void    GetEntryIndexes( ScContentId& rnRootIndex, sal_uLong& rnChildIndex, SvTreeListEntry* pEntry ) const;

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

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) override;
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;
    virtual void        DragFinished( sal_Int8 nAction ) override;

    virtual void        Command( const CommandEvent& rCEvt ) override;
    virtual void        RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void        InitEntry(SvTreeListEntry*,const OUString&,const Image&,const Image&, SvLBoxButtonKind) override;

public:
            ScContentTree( vcl::Window* pParent, const ResId& rResId );
            virtual ~ScContentTree();
    virtual void dispose() override;

    OUString getAltLongDescText(SvTreeListEntry* pEntry, bool isAltText) const;
    OUString GetEntryAltText( SvTreeListEntry* pEntry ) const override;
    OUString GetEntryLongDescription( SvTreeListEntry* pEntry ) const override;

    void     ObjectFresh( ScContentId nType, SvTreeListEntry* pEntry = nullptr);
    bool     SetNavigatorDlgFlag(bool isInNavigatoeDlg){ return bisInNavigatoeDlg=isInNavigatoeDlg;};
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;

    void    InitWindowBits( bool bButtons );

    void    Refresh( ScContentId nType = ScContentId::ROOT );

    void    ToggleRoot();
    void    SetRootType( ScContentId nNew );
    ScContentId  GetRootType() const             { return nRootType; }

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
