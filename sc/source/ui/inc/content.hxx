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

#pragma once

#include <vcl/weld.hxx>
#include <address.hxx>
#include <tools/solar.h>
#include <o3tl/enumarray.hxx>

class ScAreaLink;
class ScLinkTransferObj;
class ScDocument;
class ScDocShell;
class ScNavigatorDlg;
struct ImplSVEvent;

enum class ScContentId {
    ROOT, TABLE, RANGENAME, DBAREA,
    GRAPHIC, OLEOBJECT, NOTE, AREALINK,
    DRAWING, LAST = DRAWING
};

const sal_uLong SC_CONTENT_NOCHILD  = ~0UL;

class ScContentTree
{
    std::unique_ptr<weld::TreeView> m_xTreeView;
    std::unique_ptr<weld::TreeIter> m_xScratchIter;
    rtl::Reference<ScLinkTransferObj> m_xTransferObj;
    ScNavigatorDlg*  pParentWindow;
    o3tl::enumarray<ScContentId, std::unique_ptr<weld::TreeIter>> m_aRootNodes;
    ScContentId             nRootType;          // set as Root
    OUString                aManualDoc;         // Switched in Navigator (Title)
    bool                    bHiddenDoc;         // Hidden active?
    OUString                aHiddenName;        // URL to load
    OUString                aHiddenTitle;       // for display
    ScDocument*             pHiddenDocument;    // temporary
    bool                    bisInNavigatoeDlg;
    bool                    m_bFreeze;
    OUString                sKeyString;
    ImplSVEvent*            m_nAsyncMouseReleaseId;

    o3tl::enumarray<ScContentId, sal_uInt16> pPosList;     // for the sequence

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
    void GetEntryIndexes(ScContentId& rnRootIndex, sal_uLong& rnChildIndex, const weld::TreeIter* pEntry) const;

    /** Returns the child index of the specified listbox entry.
        @param pEntry  The entry to examine or NULL for the selected entry.
        @return  Index of the entry inside its root or SC_CONTENT_NOCHILD if entry is root. */
    sal_uLong GetChildIndex(const weld::TreeIter* pEntry) const;

    ScDocument* GetSourceDocument();

    void freeze()
    {
        m_xTreeView->freeze();
        m_bFreeze = true;
    }

    void thaw()
    {
        m_xTreeView->thaw();
        m_bFreeze = false;
    }

    void LaunchAsyncStoreNavigatorSettings();

    DECL_LINK(ContentDoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(MouseReleaseHdl, const MouseEvent&, bool);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(AsyncStoreNavigatorSettings, void*, void);
    DECL_LINK(CommandHdl, const CommandEvent&, bool);
    DECL_LINK(QueryTooltipHdl, const weld::TreeIter&, OUString);
    DECL_LINK(DragBeginHdl, bool&, bool);

public:
    ScContentTree(std::unique_ptr<weld::TreeView> xTreeView, ScNavigatorDlg* pNavigatorDlg);
    ~ScContentTree();

    void     ObjectFresh(ScContentId nType, const weld::TreeIter* pEntry = nullptr);
    void     SetNavigatorDlgFlag(bool isInNavigateDlg){ bisInNavigatoeDlg=isInNavigateDlg;};

    void    hide()
    {
        m_xTreeView->hide();
    }

    void    show()
    {
        m_xTreeView->show();
    }

    void    Refresh( ScContentId nType = ScContentId::ROOT );

    void    ToggleRoot();
    void    SetRootType( ScContentId nNew );
    ScContentId  GetRootType() const             { return nRootType; }

    void    ActiveDocChanged();
    void    ResetManualDoc();
    void    SetManualDoc(const OUString& rName);
    void    LoadFile(const OUString& rUrl);
    void    SelectDoc(const OUString& rName);
    void    SelectEntryByName(const ScContentId nRoot, std::u16string_view rName);

    const OUString& GetHiddenTitle() const    { return aHiddenTitle; }

    /** Applies the navigator settings to the listbox. */
    void ApplyNavigatorSettings(bool bRestoreScrollPos = false, int nScrollPos = 0);
    /** Stores the current listbox state in the navigator settings. */
    void StoreNavigatorSettings();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
