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

#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <sal/log.hxx>
#include <unotools/charclass.hxx>

#include <content.hxx>
#include <navipi.hxx>
#include <global.hxx>
#include <docsh.hxx>
#include <scmod.hxx>
#include <rangenam.hxx>
#include <dbdata.hxx>
#include <tablink.hxx>
#include <drwlayer.hxx>
#include <transobj.hxx>
#include <drwtrans.hxx>
#include <lnktrans.hxx>
#include <strings.hrc>
#include <scresid.hxx>
#include <bitmaps.hlst>
#include <arealink.hxx>
#include <navicfg.hxx>
#include <navsett.hxx>
#include <postit.hxx>
#include <tabvwsh.hxx>
#include <drawview.hxx>
#include <clipparam.hxx>
#include <markdata.hxx>

using namespace com::sun::star;

//  order of the categories in navigator -------------------------------------

static const ScContentId pTypeList[int(ScContentId::LAST) + 1] =
{
    ScContentId::ROOT,            // ROOT (0) has to be at the front
    ScContentId::TABLE,
    ScContentId::RANGENAME,
    ScContentId::DBAREA,
    ScContentId::AREALINK,
    ScContentId::GRAPHIC,
    ScContentId::OLEOBJECT,
    ScContentId::NOTE,
    ScContentId::DRAWING
};

static const OUStringLiteral aContentBmps[]=
{
    RID_BMP_CONTENT_TABLE,
    RID_BMP_CONTENT_RANGENAME,
    RID_BMP_CONTENT_DBAREA,
    RID_BMP_CONTENT_GRAPHIC,
    RID_BMP_CONTENT_OLEOBJECT,
    RID_BMP_CONTENT_NOTE,
    RID_BMP_CONTENT_AREALINK,
    RID_BMP_CONTENT_DRAWING
};

bool ScContentTree::bIsInDrag = false;

ScDocShell* ScContentTree::GetManualOrCurrent()
{
    ScDocShell* pSh = nullptr;
    if ( !aManualDoc.isEmpty() )
    {
        SfxObjectShell* pObjSh = SfxObjectShell::GetFirst( checkSfxObjectShell<ScDocShell> );
        while ( pObjSh && !pSh )
        {
            if ( pObjSh->GetTitle() == aManualDoc )
                pSh = dynamic_cast<ScDocShell*>( pObjSh  );
            pObjSh = SfxObjectShell::GetNext( *pObjSh, checkSfxObjectShell<ScDocShell> );
        }
    }
    else
    {
        //  only current when manual isn't set
        //  (so it's detected when the documents don't exists any longer)

        SfxViewShell* pViewSh = SfxViewShell::Current();
        if ( pViewSh )
        {
            SfxObjectShell* pObjSh = pViewSh->GetViewFrame()->GetObjectShell();
            pSh = dynamic_cast<ScDocShell*>( pObjSh  );
        }
    }

    return pSh;
}

//          ScContentTree

ScContentTree::ScContentTree(std::unique_ptr<weld::TreeView> xTreeView, ScNavigatorDlg* pNavigatorDlg)
    : m_xTreeView(std::move(xTreeView))
    , m_xScratchIter(m_xTreeView->make_iterator())
    , m_xTransferObj(new ScLinkTransferObj)
    , pParentWindow(pNavigatorDlg)
    , nRootType(ScContentId::ROOT)
    , bHiddenDoc(false)
    , pHiddenDocument(nullptr)
    , bisInNavigatoeDlg(false)
    , m_bFreeze(false)
    , m_nAsyncMouseReleaseId(nullptr)
{
    for (sal_uInt16 i = 0; i <= int(ScContentId::LAST); ++i)
        pPosList[pTypeList[i]] = i;         // inverse for searching

    m_aRootNodes[ScContentId::ROOT] = nullptr;
    for (sal_uInt16 i = 1; i < int(ScContentId::LAST); ++i)
        InitRoot(static_cast<ScContentId>(i));

    m_xTreeView->connect_row_activated(LINK(this, ScContentTree, ContentDoubleClickHdl));
    m_xTreeView->connect_mouse_release(LINK(this, ScContentTree, MouseReleaseHdl));
    m_xTreeView->connect_key_press(LINK(this, ScContentTree, KeyInputHdl));
    m_xTreeView->connect_popup_menu(LINK(this, ScContentTree, CommandHdl));
    m_xTreeView->connect_query_tooltip(LINK(this, ScContentTree, QueryTooltipHdl));

    rtl::Reference<TransferDataContainer> xHelper(m_xTransferObj.get());
    m_xTreeView->enable_drag_source(xHelper, DND_ACTION_COPY | DND_ACTION_LINK);

    m_xTreeView->connect_drag_begin(LINK(this, ScContentTree, DragBeginHdl));
}

ScContentTree::~ScContentTree()
{
    if (m_nAsyncMouseReleaseId)
    {
        Application::RemoveUserEvent(m_nAsyncMouseReleaseId);
        m_nAsyncMouseReleaseId = nullptr;
    }
    pParentWindow.clear();
}

static const char* SCSTR_CONTENT_ARY[] =
{
    SCSTR_CONTENT_ROOT,
    SCSTR_CONTENT_TABLE,
    SCSTR_CONTENT_RANGENAME,
    SCSTR_CONTENT_DBAREA,
    SCSTR_CONTENT_GRAPHIC,
    SCSTR_CONTENT_OLEOBJECT,
    SCSTR_CONTENT_NOTE,
    SCSTR_CONTENT_AREALINK,
    SCSTR_CONTENT_DRAWING
};

void ScContentTree::InitRoot( ScContentId nType )
{
    if ( nType == ScContentId::ROOT )
        return;

    if ( nRootType != ScContentId::ROOT && nRootType != nType )              // hidden ?
    {
        m_aRootNodes[nType] = nullptr;
        return;
    }

    OUString aImage(aContentBmps[static_cast<int>(nType) - 1]);
    OUString aName(ScResId(SCSTR_CONTENT_ARY[static_cast<int>(nType)]));
    // back to the correct position:
    sal_uInt16 nPos = nRootType != ScContentId::ROOT ? 0 : pPosList[nType]-1;
    m_aRootNodes[nType] = m_xTreeView->make_iterator();
    m_xTreeView->insert(nullptr, nPos, &aName, nullptr, nullptr, nullptr, &aImage, false, m_aRootNodes[nType].get());
}

void ScContentTree::ClearAll()
{
    //There are one method in Control::SetUpdateMode(), and one override method SvTreeListBox::SetUpdateMode(). Here although
    //SvTreeListBox::SetUpdateMode() is called in refresh method, it only call SvTreeListBox::SetUpdateMode(), not Control::SetUpdateMode().
    //In m_xTreeView->clear(), Broadcast( LISTACTION_CLEARED ) will be called and finally, it will be trapped into the event yield() loop. And
    //the InitRoot() method won't be called. Then if a user click or press key to update the navigator tree, crash happens.
    //So the solution is to disable the UpdateMode of Control, then call Clear(), then recover the update mode
    bool bWasFrozen = m_bFreeze;
    if (!bWasFrozen)
        freeze();
    m_xTreeView->clear();
    if (!bWasFrozen)
        thaw();
    for (sal_uInt16 i=1; i<=int(ScContentId::LAST); i++)
        InitRoot(static_cast<ScContentId>(i));
}

void ScContentTree::ClearType(ScContentId nType)
{
    if (nType == ScContentId::ROOT)
        ClearAll();
    else
    {
        weld::TreeIter* pParent = m_aRootNodes[nType].get();
        if (!pParent || m_xTreeView->iter_has_child(*pParent)) // not if no children existing
        {
            if (pParent)
                m_xTreeView->remove(*pParent);          // with all children
            InitRoot( nType );                          // if needed insert anew
        }
    }
}

void ScContentTree::InsertContent( ScContentId nType, const OUString& rValue )
{
    weld::TreeIter* pParent = m_aRootNodes[nType].get();
    if (pParent)
    {
        m_xTreeView->insert(pParent, -1, &rValue, nullptr, nullptr, nullptr, nullptr, false, m_xScratchIter.get());
        m_xTreeView->set_sensitive(*m_xScratchIter, true);
    }
    else
    {
        OSL_FAIL("InsertContent without parent");
    }
}

void ScContentTree::GetEntryIndexes(ScContentId& rnRootIndex, sal_uLong& rnChildIndex, const weld::TreeIter* pEntry) const
{
    rnRootIndex = ScContentId::ROOT;
    rnChildIndex = SC_CONTENT_NOCHILD;

    if( !pEntry )
        return;

    std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator(pEntry));
    if (!m_xTreeView->iter_parent(*xParent))
        xParent.reset();
    bool bFound = false;
    for( int i = 1; !bFound && (i <= int(ScContentId::LAST)); ++i )
    {
        ScContentId nRoot = static_cast<ScContentId>(i);
        if (!m_aRootNodes[nRoot])
            continue;
        if (m_xTreeView->iter_compare(*pEntry, *m_aRootNodes[nRoot]) == 0)
        {
            rnRootIndex = nRoot;
            rnChildIndex = ~0UL;
            bFound = true;
        }
        else if (xParent && m_xTreeView->iter_compare(*xParent, *m_aRootNodes[nRoot]) == 0)
        {
            rnRootIndex = nRoot;

            // search the entry in all child entries of the parent
            sal_uLong nEntry = 0;
            std::unique_ptr<weld::TreeIter> xIterEntry(m_xTreeView->make_iterator(xParent.get()));
            bool bIterEntry = m_xTreeView->iter_children(*xIterEntry);
            while (!bFound && bIterEntry)
            {
                if (m_xTreeView->iter_compare(*pEntry, *xIterEntry) == 0)
                {
                    rnChildIndex = nEntry;
                    bFound = true;  // exit the while loop
                }
                bIterEntry = m_xTreeView->iter_next_sibling(*xIterEntry);
                ++nEntry;
            }

            bFound = true;  // exit the for loop
        }
    }
}

sal_uLong ScContentTree::GetChildIndex(const weld::TreeIter* pEntry) const
{
    ScContentId nRoot;
    sal_uLong nChild;
    GetEntryIndexes(nRoot, nChild, pEntry);
    return nChild;
}

static OUString lcl_GetDBAreaRange( const ScDocument* pDoc, const OUString& rDBName )
{
    OUString aRet;
    if (pDoc)
    {
        ScDBCollection* pDbNames = pDoc->GetDBCollection();
        const ScDBData* pData = pDbNames->getNamedDBs().findByUpperName(ScGlobal::getCharClassPtr()->uppercase(rDBName));
        if (pData)
        {
            ScRange aRange;
            pData->GetArea(aRange);
            aRet = aRange.Format(*pDoc, ScRefFlags::RANGE_ABS_3D);
        }
    }
    return aRet;
}

IMPL_LINK_NOARG(ScContentTree, ContentDoubleClickHdl, weld::TreeView&, bool)
{
    ScContentId nType;
    sal_uLong nChild;
    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_cursor(xEntry.get()))
        xEntry.reset();
    GetEntryIndexes(nType, nChild, xEntry.get());

    if (xEntry && (nType != ScContentId::ROOT) && (nChild != SC_CONTENT_NOCHILD))
    {
        if ( bHiddenDoc )
            return false;               //! later...

        OUString aText(m_xTreeView->get_text(*xEntry));

        if ( !aManualDoc.isEmpty() )
            pParentWindow->SetCurrentDoc( aManualDoc );

        switch( nType )
        {
            case ScContentId::TABLE:
                pParentWindow->SetCurrentTableStr( aText );
            break;

            case ScContentId::RANGENAME:
                pParentWindow->SetCurrentCellStr( aText );
            break;

            case ScContentId::DBAREA:
            {
                //  If the same names of area and DB exists, then
                //  SID_CURRENTCELL takes the area name.
                //  Therefore for DB areas access them directly via address.

                OUString aRangeStr = lcl_GetDBAreaRange( GetSourceDocument(), aText );
                if (!aRangeStr.isEmpty())
                    pParentWindow->SetCurrentCellStr( aRangeStr );
            }
            break;

            case ScContentId::OLEOBJECT:
            case ScContentId::GRAPHIC:
            case ScContentId::DRAWING:
                pParentWindow->SetCurrentObject( aText );
            break;

            case ScContentId::NOTE:
            {
                ScAddress aPos = GetNotePos( nChild );
                pParentWindow->SetCurrentTable( aPos.Tab() );
                pParentWindow->SetCurrentCell( aPos.Col(), aPos.Row() );
            }
            break;

            case ScContentId::AREALINK:
            {
                const ScAreaLink* pLink = GetLink(nChild);
                ScDocument* pSrcDoc = GetSourceDocument();
                if (pLink && pSrcDoc)
                {
                    const ScRange& aRange = pLink->GetDestArea();
                    OUString aRangeStr(aRange.Format(*pSrcDoc, ScRefFlags::RANGE_ABS_3D, pSrcDoc->GetAddressConvention()));
                    pParentWindow->SetCurrentCellStr( aRangeStr );
                }
            }
            break;
            default: break;
        }

        ScNavigatorDlg::ReleaseFocus();     // set focus into document
    }

    return false;
}

void ScContentTree::LaunchAsyncStoreNavigatorSettings()
{
    if (!m_nAsyncMouseReleaseId)
        m_nAsyncMouseReleaseId = Application::PostUserEvent(LINK(this, ScContentTree, AsyncStoreNavigatorSettings));
}

IMPL_LINK_NOARG(ScContentTree, MouseReleaseHdl, const MouseEvent&, bool)
{
    LaunchAsyncStoreNavigatorSettings();
    return false;
}

IMPL_LINK_NOARG(ScContentTree, AsyncStoreNavigatorSettings, void*, void)
{
    m_nAsyncMouseReleaseId = nullptr;
    StoreNavigatorSettings();
}

IMPL_LINK(ScContentTree, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bUsed = false;

    const vcl::KeyCode aCode = rKEvt.GetKeyCode();
    if (aCode.GetCode() == KEY_RETURN)
    {
        switch (aCode.GetModifier())
        {
            case KEY_MOD1:
                ToggleRoot();       // toggle root mode (as in Writer)
                bUsed = true;
                break;
            case 0:
            {
                std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
                if (!m_xTreeView->get_cursor(xEntry.get()))
                    xEntry.reset();
                if (xEntry)
                {
                    ScContentId nType;
                    sal_uLong nChild;
                    GetEntryIndexes(nType, nChild, xEntry.get());

                    if (nType != ScContentId::ROOT && nChild == SC_CONTENT_NOCHILD)
                    {
                        if (m_xTreeView->get_row_expanded(*xEntry))
                            m_xTreeView->collapse_row(*xEntry);
                        else
                            m_xTreeView->expand_row(*xEntry);
                    }
                    else
                        ContentDoubleClickHdl(*m_xTreeView);      // select content as if double clicked
                }

                bUsed = true;
            }
            break;
        }
    }
    //Make KEY_SPACE has same function as DoubleClick
    if ( bisInNavigatoeDlg )
    {
        if(aCode.GetCode() == KEY_SPACE )
        {
            bUsed = true;
            ScContentId nType;
            sal_uLong nChild;
            std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
            if (!m_xTreeView->get_cursor(xEntry.get()))
                xEntry.reset();
            GetEntryIndexes(nType, nChild, xEntry.get());

            if (xEntry && (nType != ScContentId::ROOT) && (nChild != SC_CONTENT_NOCHILD))
            {
                if ( bHiddenDoc )
                    return true;                //! later...
                OUString aText(m_xTreeView->get_text(*xEntry));
                sKeyString = aText;
                if (!aManualDoc.isEmpty())
                    pParentWindow->SetCurrentDoc( aManualDoc );
                switch (nType)
                {
                    case ScContentId::OLEOBJECT:
                    case ScContentId::GRAPHIC:
                    case ScContentId::DRAWING:
                    {
                        ScDrawView* pScDrawView = nullptr;
                        ScTabViewShell* pScTabViewShell = ScNavigatorDlg::GetTabViewShell();
                        if (pScTabViewShell)
                            pScDrawView = pScTabViewShell->GetViewData().GetScDrawView();
                        if (pScDrawView)
                        {
                            pScDrawView->SelectCurrentViewObject(aText);
                            bool bHasMakredObject = false;
                            weld::TreeIter* pParent = m_aRootNodes[nType].get();
                            std::unique_ptr<weld::TreeIter> xBeginEntry(m_xTreeView->make_iterator(pParent));
                            bool bBeginEntry = false;
                            if (pParent)
                                bBeginEntry = m_xTreeView->iter_children(*xBeginEntry);
                            while (bBeginEntry)
                            {
                                OUString aTempText(m_xTreeView->get_text(*xBeginEntry));
                                if( pScDrawView->GetObjectIsMarked( pScDrawView->GetObjectByName( aTempText ) ) )
                                {
                                    bHasMakredObject = true;
                                    break;
                                }
                                bBeginEntry = m_xTreeView->iter_next(*xBeginEntry);
                            }
                            if (!bHasMakredObject && pScTabViewShell)
                                pScTabViewShell->SetDrawShell(false);
                            ObjectFresh(nType, xEntry.get());
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }

    if (!bUsed)
    {
        if (aCode.GetCode() == KEY_F5)
            StoreNavigatorSettings();
        else
            LaunchAsyncStoreNavigatorSettings();
    }

    return bUsed;
}

IMPL_LINK(ScContentTree, CommandHdl, const CommandEvent&, rCEvt, bool)
{
    bool bDone = false;

    switch ( rCEvt.GetCommand() )
    {
        case CommandEventId::ContextMenu:
            {
                //  drag-and-drop mode
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xTreeView.get(), "modules/scalc/ui/dropmenu.ui"));
                std::unique_ptr<weld::Menu> xPop(xBuilder->weld_menu("contextmenu"));
                std::unique_ptr<weld::Menu> xDropMenu(xBuilder->weld_menu("dragmodesubmenu"));

                switch (pParentWindow->GetDropMode())
                {
                    case 0:
                        xDropMenu->set_active("hyperlink", true);
                        break;
                    case 1:
                        xDropMenu->set_active("link", true);
                        break;
                    case 2:
                        xDropMenu->set_active("copy", true);
                        break;
                }

                //  displayed document
                std::unique_ptr<weld::Menu> xDocMenu(xBuilder->weld_menu("displaymenu"));
//TODO                aDocMenu->SetMenuFlags( aDocMenu->GetMenuFlags() | MenuFlags::NoAutoMnemonics );
                sal_uInt16 i=0;
                OUString sActive;
                OUString sId;
                //  loaded documents
                ScDocShell* pCurrentSh = dynamic_cast<ScDocShell*>( SfxObjectShell::Current()  );
                SfxObjectShell* pSh = SfxObjectShell::GetFirst();
                while ( pSh )
                {
                    if ( dynamic_cast<const ScDocShell*>( pSh) !=  nullptr )
                    {
                        OUString aName = pSh->GetTitle();
                        OUString aEntry = aName;
                        if ( pSh == pCurrentSh )
                            aEntry += pParentWindow->aStrActive;
                        else
                            aEntry += pParentWindow->aStrNotActive;
                        ++i;
                        sId = "document" + OUString::number(i);
                        xDocMenu->append_radio(sId, aEntry);
                        if ( !bHiddenDoc && aName == aManualDoc )
                            sActive = sId;
                    }
                    pSh = SfxObjectShell::GetNext( *pSh );
                }
                //  "active window"
                ++i;
                sId = "document" + OUString::number(i);
                xDocMenu->append_radio(sId, pParentWindow->aStrActiveWin);
                if (!bHiddenDoc && aManualDoc.isEmpty())
                    sActive = sId;
                //  hidden document
                if ( !aHiddenTitle.isEmpty() )
                {
                    OUString aEntry = aHiddenTitle + pParentWindow->aStrHidden;
                    ++i;
                    sId = "document" + OUString::number(i);
                    xDocMenu->append_radio(sId, aEntry);
                    if (bHiddenDoc)
                        sActive = sId;
                }
                xDocMenu->set_active(sActive.toUtf8(), true);

                OString sIdent = xPop->popup_at_rect(m_xTreeView.get(), tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1, 1)));
                if (sIdent == "hyperlink")
                    pParentWindow->SetDropMode(0);
                else if (sIdent == "link")
                    pParentWindow->SetDropMode(1);
                else if (sIdent == "copy")
                    pParentWindow->SetDropMode(2);
                else if (sIdent.startsWith("document"))
                {
                    OUString aName = xDocMenu->get_label(sIdent);
                    SelectDoc(aName);
                }
            }
            break;
            default: break;
    }

    return bDone;
}

IMPL_LINK(ScContentTree, QueryTooltipHdl, const weld::TreeIter&, rEntry, OUString)
{
    OUString aHelpText;

    std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator(&rEntry));
    if (!m_xTreeView->iter_parent(*xParent))
        xParent.reset();

    if (!xParent)                                 // Top-Level ?
    {
        aHelpText = OUString::number(m_xTreeView->iter_n_children(rEntry)) +
                    " " + m_xTreeView->get_text(rEntry);
    }
    else if (m_aRootNodes[ScContentId::NOTE] && m_xTreeView->iter_compare(*xParent, *m_aRootNodes[ScContentId::NOTE]) == 0)
    {
        aHelpText = m_xTreeView->get_text(rEntry);     // notes as help text
    }
    else if (m_aRootNodes[ScContentId::AREALINK] && m_xTreeView->iter_compare(*xParent, *m_aRootNodes[ScContentId::AREALINK]) == 0)
    {
        auto nIndex = GetChildIndex(&rEntry);
        if (nIndex != SC_CONTENT_NOCHILD)
        {
            const ScAreaLink* pLink = GetLink(nIndex);
            if (pLink)
            {
                aHelpText = pLink->GetFile();           // source file as help text
            }
        }
    }

    return aHelpText;
}

ScDocument* ScContentTree::GetSourceDocument()
{
    if (bHiddenDoc)
        return pHiddenDocument;
    else
    {
        ScDocShell* pSh = GetManualOrCurrent();
        if (pSh)
            return &pSh->GetDocument();

    }
    return nullptr;
}

//Move along and draw "*" sign .
void ScContentTree::ObjectFresh(ScContentId nType, const weld::TreeIter* pEntry)
{
    if (bHiddenDoc && !pHiddenDocument)
        return;     // other document displayed

    if (nType == ScContentId::GRAPHIC || nType == ScContentId::OLEOBJECT || nType == ScContentId::DRAWING)
    {
        auto nOldChildren = m_aRootNodes[nType] ? m_xTreeView->iter_n_children(*m_aRootNodes[nType]) : 0;
        auto nOldPos = m_xTreeView->vadjustment_get_value();

        freeze();
        ClearType( nType );
        GetDrawNames( nType/*, nId*/ );
        thaw();

        auto nNewChildren = m_aRootNodes[nType] ? m_xTreeView->iter_n_children(*m_aRootNodes[nType]) : 0;
        bool bRestorePos = nOldChildren == nNewChildren;

        if (!pEntry)
            ApplyNavigatorSettings(bRestorePos, nOldPos);
        if (pEntry)
        {
            weld::TreeIter* pParent = m_aRootNodes[nType].get();
            std::unique_ptr<weld::TreeIter> xOldEntry;
            std::unique_ptr<weld::TreeIter> xBeginEntry(m_xTreeView->make_iterator(pParent));
            bool bBeginEntry = false;
            if( pParent )
                bBeginEntry = m_xTreeView->iter_children(*xBeginEntry);
            while (bBeginEntry)
            {
                OUString aTempText(m_xTreeView->get_text(*xBeginEntry));
                if (aTempText == sKeyString)
                {
                    xOldEntry = m_xTreeView->make_iterator(xBeginEntry.get());
                    break;
                }
                bBeginEntry = m_xTreeView->iter_next(*xBeginEntry);
            }
            if (xOldEntry)
            {
                m_xTreeView->expand_row(*pParent);
                m_xTreeView->select(*xOldEntry);
                m_xTreeView->set_cursor(*xOldEntry);
                StoreNavigatorSettings();
            }
        }
    }
}

void ScContentTree::Refresh( ScContentId nType )
{
    if ( bHiddenDoc && !pHiddenDocument )
        return;                                 // other document displayed

    //  if nothing has changed the cancel right away (against flicker)

    if ( nType == ScContentId::NOTE )
        if (!NoteStringsChanged())
            return;
    if ( nType == ScContentId::GRAPHIC )
        if (!DrawNamesChanged(ScContentId::GRAPHIC))
            return;
    if ( nType == ScContentId::OLEOBJECT )
        if (!DrawNamesChanged(ScContentId::OLEOBJECT))
            return;
    if ( nType == ScContentId::DRAWING )
        if (!DrawNamesChanged(ScContentId::DRAWING))
            return;

    freeze();

    ClearType( nType );

    if ( nType == ScContentId::ROOT || nType == ScContentId::TABLE )
        GetTableNames();
    if ( nType == ScContentId::ROOT || nType == ScContentId::RANGENAME )
        GetAreaNames();
    if ( nType == ScContentId::ROOT || nType == ScContentId::DBAREA )
        GetDbNames();
    if ( nType == ScContentId::ROOT || nType == ScContentId::GRAPHIC )
        GetGraphicNames();
    if ( nType == ScContentId::ROOT || nType == ScContentId::OLEOBJECT )
        GetOleNames();
    if ( nType == ScContentId::ROOT || nType == ScContentId::DRAWING )
        GetDrawingNames();
    if ( nType == ScContentId::ROOT || nType == ScContentId::NOTE )
        GetNoteStrings();
    if ( nType == ScContentId::ROOT || nType == ScContentId::AREALINK )
        GetLinkNames();

    thaw();

    ApplyNavigatorSettings();
}

void ScContentTree::GetTableNames()
{
    if ( nRootType != ScContentId::ROOT && nRootType != ScContentId::TABLE )       // hidden ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    OUString aName;
    SCTAB nCount = pDoc->GetTableCount();
    for ( SCTAB i=0; i<nCount; i++ )
    {
        pDoc->GetName( i, aName );
        InsertContent( ScContentId::TABLE, aName );
    }
}

namespace {

OUString createLocalRangeName(const OUString& rName, const OUString& rTableName)
{
    return rName + " (" + rTableName + ")";
}
}

void ScContentTree::GetAreaNames()
{
    if ( nRootType != ScContentId::ROOT && nRootType != ScContentId::RANGENAME )       // hidden ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    ScRange aDummy;
    std::set<OUString> aSet;
    ScRangeName* pRangeNames = pDoc->GetRangeName();
    for (const auto& rEntry : *pRangeNames)
    {
        if (rEntry.second->IsValidReference(aDummy))
            aSet.insert(rEntry.second->GetName());
    }
    for (SCTAB i = 0; i < pDoc->GetTableCount(); ++i)
    {
        ScRangeName* pLocalRangeName = pDoc->GetRangeName(i);
        if (pLocalRangeName && !pLocalRangeName->empty())
        {
            OUString aTableName;
            pDoc->GetName(i, aTableName);
            for (const auto& rEntry : *pLocalRangeName)
            {
                if (rEntry.second->IsValidReference(aDummy))
                    aSet.insert(createLocalRangeName(rEntry.second->GetName(), aTableName));
            }
        }
    }

    for (const auto& rItem : aSet)
    {
        InsertContent(ScContentId::RANGENAME, rItem);
    }
}

void ScContentTree::GetDbNames()
{
    if ( nRootType != ScContentId::ROOT && nRootType != ScContentId::DBAREA )      // hidden ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    ScDBCollection* pDbNames = pDoc->GetDBCollection();
    const ScDBCollection::NamedDBs& rDBs = pDbNames->getNamedDBs();
    for (const auto& rxDB : rDBs)
    {
        const OUString& aStrName = rxDB->GetName();
        InsertContent(ScContentId::DBAREA, aStrName);
    }
}

bool ScContentTree::IsPartOfType( ScContentId nContentType, sal_uInt16 nObjIdentifier )
{
    bool bRet = false;
    switch ( nContentType )
    {
        case ScContentId::GRAPHIC:
            bRet = ( nObjIdentifier == OBJ_GRAF );
            break;
        case ScContentId::OLEOBJECT:
            bRet = ( nObjIdentifier == OBJ_OLE2 );
            break;
        case ScContentId::DRAWING:
            bRet = ( nObjIdentifier != OBJ_GRAF && nObjIdentifier != OBJ_OLE2 );    // everything else
            break;
        default:
            OSL_FAIL("unknown content type");
    }
    return bRet;
}

void ScContentTree::GetDrawNames( ScContentId nType )
{
    if ( nRootType != ScContentId::ROOT && nRootType != nType )              // hidden ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    // iterate in flat mode for groups
    SdrIterMode eIter = ( nType == ScContentId::DRAWING ) ? SdrIterMode::Flat : SdrIterMode::DeepNoGroups;

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SfxObjectShell* pShell = pDoc->GetDocumentShell();
    if (pDrawLayer && pShell)
    {
        SCTAB nTabCount = pDoc->GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            OSL_ENSURE(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( pPage, eIter );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( IsPartOfType( nType, pObject->GetObjIdentifier() ) )
                    {
                        OUString aName = ScDrawLayer::GetVisibleName( pObject );
                        if (!aName.isEmpty())
                        {
                            if( bisInNavigatoeDlg )
                            {
                                weld::TreeIter* pParent = m_aRootNodes[nType].get();
                                if (pParent)
                                {
                                    m_xTreeView->insert(pParent, -1, &aName, nullptr, nullptr, nullptr, nullptr, false, m_xScratchIter.get());
                                    m_xTreeView->set_sensitive(*m_xScratchIter, true);
                                }//end if parent
                                else
                                    SAL_WARN("sc", "InsertContent without parent");
                            }
                        }

                    }

                    pObject = aIter.Next();
                }
            }
        }
    }
}

void ScContentTree::GetGraphicNames()
{
    GetDrawNames( ScContentId::GRAPHIC );
}

void ScContentTree::GetOleNames()
{
    GetDrawNames( ScContentId::OLEOBJECT );
}

void ScContentTree::GetDrawingNames()
{
    GetDrawNames( ScContentId::DRAWING );
}

void ScContentTree::GetLinkNames()
{
    if ( nRootType != ScContentId::ROOT && nRootType != ScContentId::AREALINK )                // hidden ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();
    OSL_ENSURE(pLinkManager, "no LinkManager on document?");
    const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
    sal_uInt16 nCount = rLinks.size();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = rLinks[i].get();
        if (dynamic_cast<const ScAreaLink*>( pBase) !=  nullptr)
            InsertContent( ScContentId::AREALINK, static_cast<ScAreaLink*>(pBase)->GetSource() );

            //  insert in list the names of source areas
    }
}

const ScAreaLink* ScContentTree::GetLink( sal_uLong nIndex )
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return nullptr;

    sal_uLong nFound = 0;
    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();
    OSL_ENSURE(pLinkManager, "no LinkManager on document?");
    const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
    sal_uInt16 nCount = rLinks.size();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = rLinks[i].get();
        if (auto pAreaLink = dynamic_cast<const ScAreaLink*>( pBase))
        {
            if (nFound == nIndex)
                return pAreaLink;
            ++nFound;
        }
    }

    OSL_FAIL("link not found");
    return nullptr;
}

static OUString lcl_NoteString( const ScPostIt& rNote )
{
    OUString aText = rNote.GetText();
    sal_Int32 nAt;
    while ( (nAt = aText.indexOf( '\n' )) != -1 )
        aText = aText.replaceAt( nAt, 1, " " );
    return aText;
}

void ScContentTree::GetNoteStrings()
{
    if ( nRootType != ScContentId::ROOT && nRootType != ScContentId::NOTE )        // hidden ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    // loop over cell notes
    std::vector<sc::NoteEntry> aEntries;
    pDoc->GetAllNoteEntries(aEntries);
    for (const auto& rEntry : aEntries)
        InsertContent(ScContentId::NOTE, lcl_NoteString(*rEntry.mpNote));
}

ScAddress ScContentTree::GetNotePos( sal_uLong nIndex )
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return ScAddress();

    return pDoc->GetNotePosition(nIndex);
}

bool ScContentTree::NoteStringsChanged()
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return false;

    weld::TreeIter* pParent = m_aRootNodes[ScContentId::NOTE].get();
    if (!pParent)
        return false;

    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator(pParent));
    bool bEntry = m_xTreeView->iter_children(*xEntry);

    std::vector<sc::NoteEntry> aEntries;
    pDoc->GetAllNoteEntries(aEntries);
    for (const auto& rEntry : aEntries)
    {
        const ScPostIt* pNote = rEntry.mpNote;
        if (!bEntry)
            return true;

        if (lcl_NoteString(*pNote) != m_xTreeView->get_text(*xEntry))
            return true;

        bEntry = m_xTreeView->iter_next_sibling(*xEntry);
    }

    return bEntry;
}

bool ScContentTree::DrawNamesChanged( ScContentId nType )
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return false;

    weld::TreeIter* pParent = m_aRootNodes[nType].get();
    if (!pParent)
        return false;

    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator(pParent));
    bool bEntry = m_xTreeView->iter_children(*xEntry);

    // iterate in flat mode for groups
    SdrIterMode eIter = ( nType == ScContentId::DRAWING ) ? SdrIterMode::Flat : SdrIterMode::DeepNoGroups;

    bool bEqual = true;
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SfxObjectShell* pShell = pDoc->GetDocumentShell();
    if (pDrawLayer && pShell)
    {
        SCTAB nTabCount = pDoc->GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount && bEqual; nTab++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            OSL_ENSURE(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( pPage, eIter );
                SdrObject* pObject = aIter.Next();
                while (pObject && bEqual)
                {
                    if ( IsPartOfType( nType, pObject->GetObjIdentifier() ) )
                    {
                        if ( !bEntry )
                            bEqual = false;
                        else
                        {
                            if (ScDrawLayer::GetVisibleName(pObject) != m_xTreeView->get_text(*xEntry))
                                bEqual = false;

                            bEntry = m_xTreeView->iter_next_sibling(*xEntry);
                        }
                    }
                    pObject = aIter.Next();
                }
            }
        }
    }

    if ( bEntry )
        bEqual = false;             // anything else

    return !bEqual;
}

static bool lcl_GetRange( const ScDocument* pDoc, ScContentId nType, const OUString& rName, ScRange& rRange )
{
    bool bFound = false;

    if ( nType == ScContentId::RANGENAME )
    {
        ScRangeName* pList = pDoc->GetRangeName();
        if (pList)
        {
            const ScRangeData* p = pList->findByUpperName(ScGlobal::getCharClassPtr()->uppercase(rName));
            if (p && p->IsValidReference(rRange))
                bFound = true;
        }
    }
    else if ( nType == ScContentId::DBAREA )
    {
        ScDBCollection* pList = pDoc->GetDBCollection();
        if (pList)
        {
            const ScDBData* p = pList->getNamedDBs().findByUpperName(ScGlobal::getCharClassPtr()->uppercase(rName));
            if (p)
            {
                SCTAB nTab;
                SCCOL nCol1, nCol2;
                SCROW nRow1, nRow2;
                p->GetArea(nTab, nCol1, nRow1, nCol2, nRow2);
                rRange = ScRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab);
                bFound = true;
            }
        }
    }

    return bFound;
}

static bool lcl_DoDragObject( ScDocShell* pSrcShell, const OUString& rName, ScContentId nType, weld::TreeView& rTreeView )
{
    bool bDisallow = true;

    ScDocument& rSrcDoc = pSrcShell->GetDocument();
    ScDrawLayer* pModel = rSrcDoc.GetDrawLayer();
    if (pModel)
    {
        bool bOle = ( nType == ScContentId::OLEOBJECT );
        bool bGraf = ( nType == ScContentId::GRAPHIC );
        sal_uInt16 nDrawId = sal::static_int_cast<sal_uInt16>( bOle ? OBJ_OLE2 : ( bGraf ? OBJ_GRAF : OBJ_GRUP ) );
        SCTAB nTab = 0;
        SdrObject* pObject = pModel->GetNamedObject( rName, nDrawId, nTab );
        if (pObject)
        {
            SdrView aEditView(*pModel);
            aEditView.ShowSdrPage(aEditView.GetModel()->GetPage(nTab));
            SdrPageView* pPV = aEditView.GetSdrPageView();
            aEditView.MarkObj(pObject, pPV);

            // tdf125520 this is a D&D-start potentially with an OLE object. If
            // so, we need to do similar as e.g. in ScDrawView::BeginDrag so that
            // the temporary SdrModel for transfer does have a GetPersist() so
            // that the EmbeddedObjectContainer gets copied. We need no CheckOle
            // here, test is simpler.
            ScDocShellRef aDragShellRef;
            if(OBJ_OLE2 == pObject->GetObjIdentifier())
            {
                aDragShellRef = new ScDocShell;     // DocShell needs a Ref immediately
                aDragShellRef->DoInitNew();
            }

            ScDrawLayer::SetGlobalDrawPersist(aDragShellRef.get());
            std::unique_ptr<SdrModel> pDragModel(aEditView.CreateMarkedObjModel());
            ScDrawLayer::SetGlobalDrawPersist(nullptr);

            TransferableObjectDescriptor aObjDesc;
            pSrcShell->FillTransferableObjectDescriptor( aObjDesc );
            aObjDesc.maDisplayName = pSrcShell->GetMedium()->GetURLObject().GetURLNoPass();
            // maSize is set in ScDrawTransferObj ctor

            rtl::Reference<ScDrawTransferObj> pTransferObj = new ScDrawTransferObj( std::move(pDragModel), pSrcShell, aObjDesc );

            pTransferObj->SetDragSourceObj( *pObject, nTab );
            pTransferObj->SetDragSourceFlags(ScDragSrc::Navigator);

            SC_MOD()->SetDragObject( nullptr, pTransferObj.get() );

            rtl::Reference<TransferDataContainer> xHelper(pTransferObj.get());
            rTreeView.enable_drag_source(xHelper, DND_ACTION_COPY | DND_ACTION_LINK);

            bDisallow = false;
        }
    }

    return bDisallow;
}

static bool lcl_DoDragCells( ScDocShell* pSrcShell, const ScRange& rRange, ScDragSrc nFlags, weld::TreeView& rTreeView )
{
    bool bDisallow = true;

    ScDocument& rSrcDoc = pSrcShell->GetDocument();
    ScMarkData aMark(rSrcDoc.MaxRow(), rSrcDoc.MaxCol());
    aMark.SelectTable( rRange.aStart.Tab(), true );
    aMark.SetMarkArea( rRange );

    if ( !rSrcDoc.HasSelectedBlockMatrixFragment( rRange.aStart.Col(), rRange.aStart.Row(),
                                                   rRange.aEnd.Col(),   rRange.aEnd.Row(),
                                                   aMark ) )
    {
        ScDocumentUniquePtr pClipDoc(new ScDocument( SCDOCMODE_CLIP ));
        ScClipParam aClipParam(rRange, false);
        rSrcDoc.CopyToClip(aClipParam, pClipDoc.get(), &aMark, false, false);
        // pClipDoc->ExtendMerge( rRange, sal_True );

        TransferableObjectDescriptor aObjDesc;
        pSrcShell->FillTransferableObjectDescriptor( aObjDesc );
        aObjDesc.maDisplayName = pSrcShell->GetMedium()->GetURLObject().GetURLNoPass();
        // maSize is set in ScTransferObj ctor

        rtl::Reference<ScTransferObj> pTransferObj = new ScTransferObj( std::move(pClipDoc), aObjDesc );

        pTransferObj->SetDragSource( pSrcShell, aMark );
        pTransferObj->SetDragSourceFlags( nFlags );

        SC_MOD()->SetDragObject( pTransferObj.get(), nullptr );      // for internal D&D

        rtl::Reference<TransferDataContainer> xHelper(pTransferObj.get());
        rTreeView.enable_drag_source(xHelper, DND_ACTION_COPY | DND_ACTION_LINK);

        bDisallow = false;
    }

    return bDisallow;
}

IMPL_LINK(ScContentTree, DragBeginHdl, bool&, rUnsetDragIcon, bool)
{
    rUnsetDragIcon = true;

    StoreNavigatorSettings();

    bool bDisallow = true;

    std::unique_ptr<ScDocumentLoader> pDocLoader;
    bIsInDrag = true;

    ScModule* pScMod = SC_MOD();

    ScContentId nType;
    sal_uLong nChild;

    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_cursor(xEntry.get()))
        xEntry.reset();

    GetEntryIndexes(nType, nChild, xEntry.get());

    if( xEntry &&
        (nChild != SC_CONTENT_NOCHILD) &&
        (nType != ScContentId::ROOT) &&
        (nType != ScContentId::NOTE) &&
        (nType != ScContentId::AREALINK) )
    {
        OUString aText(m_xTreeView->get_text(*xEntry));

        ScDocument* pLocalDoc = nullptr;                   // for URL drop
        OUString aDocName;
        if (bHiddenDoc)
            aDocName = aHiddenName;
        else
        {
            ScDocShell* pDocSh = GetManualOrCurrent();
            if (pDocSh)
            {
                if (pDocSh->HasName())
                    aDocName = pDocSh->GetMedium()->GetName();
                else
                    pLocalDoc = &pDocSh->GetDocument();      // drop only in this document
            }
        }

        bool bDoLinkTrans = false;      // use ScLinkTransferObj
        OUString aLinkURL;                // for ScLinkTransferObj
        OUString aLinkText;

        sal_uInt16 nDropMode = pParentWindow->GetDropMode();
        switch ( nDropMode )
        {
            case SC_DROPMODE_URL:
                {
                    OUString aUrl = aDocName + "#" + aText;

                    pScMod->SetDragJump( pLocalDoc, aUrl, aText );

                    if (!aDocName.isEmpty())
                    {
                        //  provide URL to outside only if the document has a name
                        //  (without name, only internal D&D via SetDragJump)

                        aLinkURL = aUrl;
                        aLinkText = aText;
                    }
                    bDoLinkTrans = true;
                }
                break;
            case SC_DROPMODE_LINK:
                {
                    if ( !aDocName.isEmpty() )           // link only to named documents
                    {
                        // for internal D&D, set flag to insert a link

                        switch ( nType )
                        {
                            case ScContentId::TABLE:
                                pScMod->SetDragLink( aDocName, aText, EMPTY_OUSTRING );
                                bDoLinkTrans = true;
                                break;
                            case ScContentId::RANGENAME:
                            case ScContentId::DBAREA:
                                pScMod->SetDragLink( aDocName, EMPTY_OUSTRING, aText );
                                bDoLinkTrans = true;
                                break;

                            // other types cannot be linked
                            default: break;
                        }
                    }
                }
                break;
            case SC_DROPMODE_COPY:
                {
                    ScDocShell* pSrcShell = nullptr;
                    if ( bHiddenDoc )
                    {
                        OUString aFilter, aOptions;
                        OUString aURL = aHiddenName;
                        pDocLoader.reset(new ScDocumentLoader( aURL, aFilter, aOptions ));
                        if (!pDocLoader->IsError())
                            pSrcShell = pDocLoader->GetDocShell();
                    }
                    else
                        pSrcShell = GetManualOrCurrent();

                    if ( pSrcShell )
                    {
                        ScDocument& rSrcDoc = pSrcShell->GetDocument();
                        if ( nType == ScContentId::RANGENAME || nType == ScContentId::DBAREA )
                        {
                            ScRange aRange;
                            if ( lcl_GetRange( &rSrcDoc, nType, aText, aRange ) )
                            {
                                bDisallow = lcl_DoDragCells( pSrcShell, aRange, ScDragSrc::Navigator, *m_xTreeView );
                            }
                        }
                        else if ( nType == ScContentId::TABLE )
                        {
                            SCTAB nTab;
                            if ( rSrcDoc.GetTable( aText, nTab ) )
                            {
                                ScRange aRange(0, 0, nTab, rSrcDoc.MaxCol(), rSrcDoc.MaxRow(), nTab);
                                bDisallow = lcl_DoDragCells( pSrcShell, aRange, (ScDragSrc::Navigator | ScDragSrc::Table), *m_xTreeView );
                            }
                        }
                        else if ( nType == ScContentId::GRAPHIC || nType == ScContentId::OLEOBJECT ||
                                    nType == ScContentId::DRAWING )
                        {
                            bDisallow = lcl_DoDragObject( pSrcShell, aText, nType, *m_xTreeView );

                            //  during ExecuteDrag the navigator can be deleted
                            //  -> don't access member anymore !!!
                        }
                    }
                }
                break;
        }

        if (bDoLinkTrans)
        {
            if (!aLinkURL.isEmpty())
                m_xTransferObj->SetLinkURL(aLinkURL, aLinkText);

            rtl::Reference<TransferDataContainer> xHelper(m_xTransferObj.get());
            m_xTreeView->enable_drag_source(xHelper, DND_ACTION_COPY | DND_ACTION_LINK);

            bDisallow = false;
        }
    }

    bIsInDrag = false;              // static member

    return bDisallow;
}

void ScContentTree::LoadFile( const OUString& rUrl )
{
    OUString aDocName = rUrl;
    sal_Int32 nPos = aDocName.indexOf('#');
    if ( nPos != -1 )
        aDocName = aDocName.copy(0, nPos);           // only the name without #...

    OUString aURL = aDocName;
    OUString aFilter, aOptions;
    ScDocumentLoader aLoader( aURL, aFilter, aOptions );
    if ( !aLoader.IsError() )
    {
        bHiddenDoc = true;
        aHiddenName = aDocName;
        aHiddenTitle = aLoader.GetTitle();
        pHiddenDocument = aLoader.GetDocument();

        Refresh();                      // get content from loaded document

        pHiddenDocument = nullptr;

        pParentWindow->GetDocNames( &aHiddenTitle );            // fill list
    }

    //  document is closed again by ScDocumentLoader in dtor
}

void ScContentTree::SetRootType( ScContentId nNew )
{
    if ( nNew != nRootType )
    {
        nRootType = nNew;
        Refresh();

        ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
        rCfg.SetRootType( nRootType );
    }
}

void ScContentTree::ToggleRoot()        // after selection
{
    ScContentId nNew = ScContentId::ROOT;
    if ( nRootType == ScContentId::ROOT )
    {
        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
        if (m_xTreeView->get_cursor(xEntry.get()))
        {
            std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator(xEntry.get()));
            if (!m_xTreeView->iter_parent(*xParent))
                xParent.reset();

            for (sal_uInt16 i=1; i<=int(ScContentId::LAST); i++)
            {
                if (!m_aRootNodes[static_cast<ScContentId>(i)])
                    continue;
                if ((m_xTreeView->iter_compare(*xEntry, *m_aRootNodes[static_cast<ScContentId>(i)]) == 0) ||
                    (xParent && m_xTreeView->iter_compare(*xParent, *m_aRootNodes[static_cast<ScContentId>(i)]) == 0))
                {
                    nNew = static_cast<ScContentId>(i);
                }
            }
        }
    }

    SetRootType( nNew );
}

void ScContentTree::ResetManualDoc()
{
    aManualDoc.clear();
    bHiddenDoc = false;

    ActiveDocChanged();
}

void ScContentTree::ActiveDocChanged()
{
    if ( !bHiddenDoc && aManualDoc.isEmpty() )
        Refresh();                                  // content only if automatic

        //  if flag active Listbox must be updated

    OUString aCurrent;
    if ( bHiddenDoc )
        aCurrent = aHiddenTitle;
    else
    {
        ScDocShell* pSh = GetManualOrCurrent();
        if (pSh)
            aCurrent = pSh->GetTitle();
        else
        {
            //  document is no longer available

            aManualDoc.clear();             // again automatically
            Refresh();
            pSh = GetManualOrCurrent();     // should be active now
            if (pSh)
                aCurrent = pSh->GetTitle();
        }
    }
    pParentWindow->GetDocNames( &aCurrent );        // select
}

void ScContentTree::SetManualDoc(const OUString& rName)
{
    aManualDoc = rName;
    if (!bHiddenDoc)
    {
        Refresh();
        pParentWindow->GetDocNames( &aManualDoc );      // select
    }
}

void ScContentTree::SelectDoc(const OUString& rName)      // rName like shown in Menu/Listbox
{
    if ( rName == pParentWindow->aStrActiveWin )
    {
        ResetManualDoc();
        return;
    }

    //  omit "active" or "inactive"

    OUString aRealName = rName;
    sal_Int32 nLen = rName.getLength();
    sal_Int32 nActiveStart = nLen - pParentWindow->aStrActive.getLength();
    if ( rName.copy( nActiveStart ) == pParentWindow->aStrActive )
        aRealName = rName.copy( 0, nActiveStart );
    sal_Int32 nNotActiveStart = nLen - pParentWindow->aStrNotActive.getLength();
    if ( rName.copy( nNotActiveStart ) == pParentWindow->aStrNotActive )
        aRealName = rName.copy( 0, nNotActiveStart );

    bool bLoaded = false;

    // Is it a normally loaded document?

    SfxObjectShell* pSh = SfxObjectShell::GetFirst();
    while ( pSh && !bLoaded )
    {
        if ( dynamic_cast<const ScDocShell*>( pSh) !=  nullptr )
            if ( pSh->GetTitle() == aRealName )
                bLoaded = true;
        pSh = SfxObjectShell::GetNext( *pSh );
    }

    if (bLoaded)
    {
        bHiddenDoc = false;
        SetManualDoc(aRealName);
    }
    else if (!aHiddenTitle.isEmpty())                // hidden selected
    {
        if (!bHiddenDoc)
            LoadFile(aHiddenName);
    }
    else
    {
        OSL_FAIL("SelectDoc: not found");
    }
}

void ScContentTree::SelectEntryByName(const ScContentId nRoot, const OUString& rName)
{
    weld::TreeIter* pParent = m_aRootNodes[nRoot].get();

    if (pParent || !m_xTreeView->iter_has_child(*pParent))
        return;

    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator(pParent));
    bool bEntry = m_xTreeView->iter_children(*xEntry);

    while (bEntry)
    {
        if (m_xTreeView->get_text(*xEntry) == rName)
        {
            m_xTreeView->select(*xEntry);
            m_xTreeView->set_cursor(*xEntry);

            // Scroll to the selected item
            m_xTreeView->scroll_to_row(*xEntry);

            StoreNavigatorSettings();

            return;
        }
        bEntry = m_xTreeView->iter_next(*xEntry);
    }
}

void ScContentTree::ApplyNavigatorSettings(bool bRestorePos, int nScrollPos)
{
    const ScNavigatorSettings* pSettings = ScNavigatorDlg::GetNavigatorSettings();
    if( pSettings )
    {
        ScContentId nRootSel = pSettings->GetRootSelected();
        auto nChildSel = pSettings->GetChildSelected();

        // tdf#133079 ensure Sheet root is selected if nothing
        // else would be
        if (nRootSel == ScContentId::ROOT)
        {
            nRootSel = ScContentId::TABLE;
            nChildSel = SC_CONTENT_NOCHILD;
        }

        for( int i = 1; i <= int(ScContentId::LAST); ++i )
        {
            ScContentId nEntry = static_cast<ScContentId>(i);
            if( m_aRootNodes[ nEntry ] )
            {
                // gray or ungray
                if (!m_xTreeView->iter_has_child(*m_aRootNodes[nEntry]))
                    m_xTreeView->set_sensitive(*m_aRootNodes[nEntry], false);
                else
                    m_xTreeView->set_sensitive(*m_aRootNodes[nEntry], true);

                // expand
                bool bExp = pSettings->IsExpanded( nEntry );
                if (bExp != m_xTreeView->get_row_expanded(*m_aRootNodes[nEntry]))
                {
                    if( bExp )
                        m_xTreeView->expand_row(*m_aRootNodes[nEntry]);
                    else
                        m_xTreeView->collapse_row(*m_aRootNodes[nEntry]);
                }

                // select
                if( nRootSel == nEntry )
                {
                    if (bRestorePos)
                        m_xTreeView->vadjustment_set_value(nScrollPos);

                    std::unique_ptr<weld::TreeIter> xEntry;
                    if (bExp && (nChildSel != SC_CONTENT_NOCHILD))
                    {
                        xEntry = m_xTreeView->make_iterator(m_aRootNodes[nEntry].get());
                        if (!m_xTreeView->iter_children(*xEntry) || !m_xTreeView->iter_nth_sibling(*xEntry, nChildSel))
                            xEntry.reset();
                    }
                    m_xTreeView->select(xEntry ? *xEntry : *m_aRootNodes[nEntry]);
                    m_xTreeView->set_cursor(xEntry ? *xEntry : *m_aRootNodes[nEntry]);
                }
            }
        }
    }
}

void ScContentTree::StoreNavigatorSettings() const
{
    ScNavigatorSettings* pSettings = ScNavigatorDlg::GetNavigatorSettings();
    if( pSettings )
    {
        for( int i = 1; i <= int(ScContentId::LAST); ++i )
        {
            ScContentId nEntry = static_cast<ScContentId>(i);
            bool bExp = m_aRootNodes[nEntry] && m_xTreeView->get_row_expanded(*m_aRootNodes[nEntry]);
            pSettings->SetExpanded( nEntry, bExp );
        }

        std::unique_ptr<weld::TreeIter> xCurEntry(m_xTreeView->make_iterator());
        if (!m_xTreeView->get_cursor(xCurEntry.get()))
            xCurEntry.reset();

        ScContentId nRoot;
        sal_uLong nChild;
        GetEntryIndexes(nRoot, nChild, xCurEntry.get());

        pSettings->SetRootSelected( nRoot );
        pSettings->SetChildSelected( nChild );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
