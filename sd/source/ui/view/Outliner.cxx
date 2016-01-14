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

#include "Outliner.hxx"
#include <boost/property_tree/json_parser.hpp>
#include <vcl/wrkwin.hxx>
#include <vcl/settings.hxx>

#include <svl/srchitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editstat.hxx>
#include <vcl/outdev.hxx>
#include <svx/dlgutil.hxx>
#include <svx/xtable.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/printer.hxx>
#include <svx/svxerr.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdotable.hxx>
#include <editeng/unolingu.hxx>
#include <svx/svditer.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <svx/srchdlg.hxx>
#include <unotools/linguprops.hxx>
#include <unotools/lingucfg.hxx>
#include <editeng/editeng.hxx>
#include <vcl/metric.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/langtab.hxx>
#include <tools/diagnose_ex.h>

#include "strings.hrc"
#include "sdstring.hrc"
#include <editeng/outliner.hxx>
#include "sdpage.hxx"
#include "sdmod.hxx"
#include "Window.hxx"
#include "sdresid.hxx"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "FrameView.hxx"
#include "optsitem.hxx"
#include "drawview.hxx"
#include "ViewShellBase.hxx"
#include "SpellDialogChildWindow.hxx"
#include "ToolBarManager.hxx"
#include "framework/FrameworkHelper.hxx"
#include <svx/svxids.hrc>
#include <editeng/editerr.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/string.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/scopeguard.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

class SfxStyleSheetPool;

namespace sd {

SearchSelection::SearchSelection(int nPage, const OString& rRectangles)
    : m_nPage(nPage),
    m_aRectangles(rRectangles)
{
}

class Outliner::Implementation
{
public:
    /** The original edit mode directly after switching to a different view
        mode.  Used for restoring the edit mode when leaving that view mode
        again.
    */
    EditMode meOriginalEditMode;

    Implementation();
    ~Implementation();

    /** Return the OutlinerView that was provided by the last call to
        ProvideOutlinerView() (or NULL when there was no such call.)
    */
    OutlinerView* GetOutlinerView() { return mpOutlineView;}

    /** Provide in the member mpOutlineView an instance of OutlinerView that
        is either taken from the ViewShell, when it is an OutlineViewShell,
        or is created.  When an OutlinerView already exists it is initialied.
    */
    void ProvideOutlinerView (
        Outliner& rOutliner,
        const std::shared_ptr<ViewShell>& rpViewShell,
        vcl::Window* pWindow);

    /** This method is called when the OutlinerView is no longer used.
    */
    void ReleaseOutlinerView();

private:
    /** Flag that specifies whether we own the outline view pointed to by
        <member>mpOutlineView</member> and thus have to
        delete it in <member>EndSpelling()</member>.
    */
    bool mbOwnOutlineView;

    /** The outline view used for searching and spelling.  If searching or
        spell checking an outline view this data member points to that view.
        For all other views an instance is created.  The
        <member>mbOwnOutlineView</member> distinguishes between both cases.
    */
    OutlinerView* mpOutlineView;
};

Outliner::Outliner( SdDrawDocument* pDoc, sal_uInt16 nMode )
    : SdrOutliner( &pDoc->GetItemPool(), nMode ),
      mpImpl(new Implementation()),
      meMode(SEARCH),
      mpView(nullptr),
      mpWeakViewShell(),
      mpWindow(nullptr),
      mpDrawDocument(pDoc),
      mnConversionLanguage(LANGUAGE_NONE),
      mnIgnoreCurrentPageChangesLevel(0),
      mbStringFound(false),
      mbMatchMayExist(false),
      mnPageCount(0),
      mbEndOfSearch(false),
      mbFoundObject(false),
      mbError(false),
      mbDirectionIsForward(true),
      mbRestrictSearchToSelection(false),
      maMarkListCopy(),
      mpObj(nullptr),
      mpFirstObj(nullptr),
      mpTextObj(nullptr),
      mnText(0),
      mpParaObj(nullptr),
      meStartViewMode(PK_STANDARD),
      meStartEditMode(EM_PAGE),
      mnStartPageIndex((sal_uInt16)-1),
      mpStartEditedObject(nullptr),
      maStartSelection(),
      mpSearchItem(nullptr),
      maObjectIterator(),
      maCurrentPosition(),
      maSearchStartPosition(),
      maLastValidPosition(),
      mbExpectingSelectionChangeEvent(false),
      mbWholeDocumentProcessed(false),
      mbPrepareSpellingPending(true)
{
    SetStyleSheetPool(static_cast<SfxStyleSheetPool*>( mpDrawDocument->GetStyleSheetPool() ));
    SetEditTextObjectPool( &pDoc->GetItemPool() );
    SetCalcFieldValueHdl(LINK(SD_MOD(), SdModule, CalcFieldValueHdl));
    SetForbiddenCharsTable( pDoc->GetForbiddenCharsTable() );

    EEControlBits nCntrl = GetControlWord();
    nCntrl |= EEControlBits::ALLOWBIGOBJS;
    nCntrl |= EEControlBits::MARKFIELDS;
    nCntrl |= EEControlBits::AUTOCORRECT;

    bool bOnlineSpell = false;

    DrawDocShell* pDocSh = mpDrawDocument->GetDocSh();

    if (pDocSh)
    {
        bOnlineSpell = mpDrawDocument->GetOnlineSpell();
    }
    else
    {
        bOnlineSpell = false;

        try
        {
            const SvtLinguConfig    aLinguConfig;
            Any                     aAny;

            aAny = aLinguConfig.GetProperty( UPN_IS_SPELL_AUTO );
            aAny >>= bOnlineSpell;
        }
        catch( ... )
        {
            OSL_FAIL( "Ill. type in linguistic property" );
        }
    }

    if (bOnlineSpell)
        nCntrl |= EEControlBits::ONLINESPELLING;
    else
        nCntrl &= ~EEControlBits::ONLINESPELLING;

    SetControlWord(nCntrl);

    Reference< XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
    if ( xSpellChecker.is() )
        SetSpeller( xSpellChecker );

    Reference< XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
    if( xHyphenator.is() )
        SetHyphenator( xHyphenator );

    SetDefaultLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() );
}

/// Nothing spectecular in the destructor.
Outliner::~Outliner()
{
    mpImpl.reset();
}

/** Prepare find&replace or spellchecking.  This distinguishes between three
    cases:
    <ol>
    <li>The current shell is a <type>DrawViewShell</type>: Create a
    <type>OutlinerView</type> object and search all objects of (i) the
    current mark list, (ii) of the current view, or (iii) of all the view
    combinations:
    <ol>
    <li>Draw view, slide view</li>
    <li>Draw view, background view</li>
    <li>Notes view, slide view</li>
    <li>Notes view, background view</li>
    <li>Handout view, slide view</li>
    <li>Handout view, background view</li>
    </ol>

    <li>When the current shell is a <type>SdOutlineViewShell</type> then
    directly operate on it.  No switching into other views takes place.</li>
    </ol>
*/
void Outliner::PrepareSpelling()
{
    mbPrepareSpellingPending = false;

    ViewShellBase* pBase = dynamic_cast< ViewShellBase *>( SfxViewShell::Current() );
    if (pBase != nullptr)
        SetViewShell (pBase->GetMainViewShell());
    SetRefDevice( SD_MOD()->GetRefDevice( *mpDrawDocument->GetDocSh() ) );

    std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if (pViewShell)
    {
        mbStringFound = false;

        mbWholeDocumentProcessed = false;
        // Supposed that we are not located at the very beginning/end of
        // the document then there may be a match in the document
        // prior/after the current position.
        mbMatchMayExist = true;

        maObjectIterator = ::sd::outliner::Iterator();
        maSearchStartPosition = ::sd::outliner::Iterator();
        RememberStartPosition();

        mpImpl->ProvideOutlinerView(*this, pViewShell, mpWindow);

        HandleChangedSelection ();
    }
    ClearModifyFlag();
}

void Outliner::StartSpelling()
{
    meMode = SPELL;
    mbDirectionIsForward = true;
    mpSearchItem = nullptr;
}

/** Free all resources acquired during the search/spell check.  After a
    spell check the start position is restored here.
*/
void Outliner::EndSpelling()
{
    // Keep old view shell alive until we release the outliner view.
    std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    std::shared_ptr<ViewShell> pOldViewShell (pViewShell);

    ViewShellBase* pBase = dynamic_cast< ViewShellBase *>( SfxViewShell::Current() );
    if (pBase != nullptr)
        pViewShell = pBase->GetMainViewShell();
    else
        pViewShell.reset();
    mpWeakViewShell = pViewShell;

    // When in <member>PrepareSpelling()</member> a new outline view has
    // been created then delete it here.
    bool bViewIsDrawViewShell(pViewShell && nullptr != dynamic_cast< const DrawViewShell *>( pViewShell.get() ));
    if (bViewIsDrawViewShell)
    {
        SetStatusEventHdl(Link<EditStatus&,void>());
        mpView = pViewShell->GetView();
        mpView->UnmarkAllObj (mpView->GetSdrPageView());
        mpView->SdrEndTextEdit();
        // Make FuSelection the current function.
        pViewShell->GetDispatcher()->Execute(
            SID_OBJECT_SELECT,
            SfxCallMode::SYNCHRON | SfxCallMode::RECORD);

        // Remove and, if previously created by us, delete the outline
        // view.
        OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
        if (pOutlinerView != nullptr)
        {
            RemoveView(pOutlinerView);
            mpImpl->ReleaseOutlinerView();
        }

        SetUpdateMode(true);
    }

    // Before clearing the modify flag use it as a hint that
    // changes were done at SpellCheck
    if(IsModified())
    {
        if(mpView && dynamic_cast< const OutlineView *>( mpView ) !=  nullptr)
            static_cast<OutlineView*>(mpView)->PrepareClose(false);
        if(mpDrawDocument && !mpDrawDocument->IsChanged())
            mpDrawDocument->SetChanged();
    }

    // Now clear the modify flag to have a specified state of
    // Outliner
    ClearModifyFlag();

    // When spell checking then restore the start position.
    if (meMode==SPELL || meMode==TEXT_CONVERSION)
        RestoreStartPosition ();

    mpWeakViewShell.reset();
    mpView = nullptr;
    mpWindow = nullptr;
    mnStartPageIndex = (sal_uInt16) -1;
}

bool Outliner::SpellNextDocument()
{
    std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if( nullptr != dynamic_cast< const OutlineViewShell *>( pViewShell.get() ))
    {
        // When doing a spell check in the outline view then there is
        // only one document.
        mbEndOfSearch = true;
        EndOfSearch ();
    }
    else
    {
        if( dynamic_cast< const OutlineView *>( mpView ) !=  nullptr)
            static_cast<OutlineView*>(mpView)->PrepareClose(false);
        mpDrawDocument->GetDocSh()->SetWaitCursor( true );

        Initialize (true);

        mpWindow = pViewShell->GetActiveWindow();
        OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
        if (pOutlinerView != nullptr)
            pOutlinerView->SetWindow(mpWindow);
        ProvideNextTextObject ();

        mpDrawDocument->GetDocSh()->SetWaitCursor( false );
        ClearModifyFlag();
    }

    return !mbEndOfSearch;
}

/**
 * check next text object
 */
svx::SpellPortions Outliner::GetNextSpellSentence()
{
    svx::SpellPortions aResult;

    DetectChange();
    // Iterate over sentences and text shapes until a sentence with a
    // spelling error has been found.  If no such sentence can be
    // found the loop is left through a break.
    // It is the responsibility of the sd outliner object to correctly
    // iterate over all text shapes, i.e. switch between views, wrap
    // around at the end of the document, stop when all text shapes
    // have been examined exactly once.
    bool bFoundNextSentence = false;
    while ( ! bFoundNextSentence)
    {
        OutlinerView* pOutlinerView = GetView(0);
        if (pOutlinerView != nullptr)
        {
            ESelection aCurrentSelection (pOutlinerView->GetSelection());
            if ( ! mbMatchMayExist
                && maStartSelection.IsLess(aCurrentSelection))
                EndOfSearch();

            // Advance to the next sentence.
            bFoundNextSentence = SpellSentence (
                pOutlinerView->GetEditView(),
                aResult, false);
        }

        // When no sentence with spelling errors has been found in the
        // currently selected text shape or there is no selected text
        // shape then advance to the next text shape.
        if ( ! bFoundNextSentence)
            if ( ! SpellNextDocument())
                // All text objects have been processed so exit the
                // loop and return an empty portions list.
                break;
    }

    return aResult;
}

/** Go to next match.
*/
bool Outliner::StartSearchAndReplace (const SvxSearchItem* pSearchItem)
{
    bool bEndOfSearch = true;

    // clear the search toolbar entry
    SvxSearchDialogWrapper::SetSearchLabel(SL_Empty);

    mpDrawDocument->GetDocSh()->SetWaitCursor( true );
    if (mbPrepareSpellingPending)
        PrepareSpelling();
    ViewShellBase* pBase = dynamic_cast< ViewShellBase *>( SfxViewShell::Current() );
    // Determine whether we have to abort the search.  This is necessary
    // when the main view shell does not support searching.
    bool bAbort = false;
    if (pBase != nullptr)
    {
        std::shared_ptr<ViewShell> pShell (pBase->GetMainViewShell());
        SetViewShell(pShell);
        if (pShell.get() == nullptr)
            bAbort = true;
        else
            switch (pShell->GetShellType())
            {
                case ViewShell::ST_DRAW:
                case ViewShell::ST_IMPRESS:
                case ViewShell::ST_NOTES:
                case ViewShell::ST_HANDOUT:
                case ViewShell::ST_OUTLINE:
                    bAbort = false;
                    break;
                default:
                    bAbort = true;
                    break;
            }
    }

    std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if ( ! pViewShell)
    {
        OSL_ASSERT(pViewShell);
        return true;
    }

    if ( ! bAbort)
    {
        meMode = SEARCH;
        mpSearchItem = pSearchItem;

        mbFoundObject = false;

        Initialize ( ! mpSearchItem->GetBackward());

        const SvxSearchCmd nCommand (mpSearchItem->GetCommand());
        if (nCommand == SvxSearchCmd::FIND_ALL || nCommand == SvxSearchCmd::REPLACE_ALL)
            bEndOfSearch = SearchAndReplaceAll ();
        else
        {
            RememberStartPosition ();
            bEndOfSearch = SearchAndReplaceOnce ();
            // restore start position if nothing was found
            if(!mbStringFound)
            {
                RestoreStartPosition ();
                // Nothing was changed, no need to restart the spellchecker.
                if (nCommand == SvxSearchCmd::FIND)
                    bEndOfSearch = false;
            }
            mnStartPageIndex = (sal_uInt16)-1;
        }

        SfxChildWindow *pChildWin =
            SfxViewFrame::Current()->GetChildWindow(
            SvxSearchDialogWrapper::GetChildWindowId());
        if (pChildWin)
        {
            SvxSearchDialog* pSearchDlg =
                static_cast<SvxSearchDialog*>(pChildWin->GetWindow());
            pSearchDlg->SetDocWin( pViewShell->GetActiveWindow() );
            pSearchDlg->SetSrchFlag(false);
        }
    }
    else
        mpDrawDocument->GetDocSh()->SetWaitCursor( false );

    return bEndOfSearch;
}

void Outliner::Initialize (bool bDirectionIsForward)
{
    const bool bIsAtEnd (maObjectIterator == ::sd::outliner::OutlinerContainer(this).end());
    const bool bOldDirectionIsForward = mbDirectionIsForward;
    mbDirectionIsForward = bDirectionIsForward;

    if (maObjectIterator == ::sd::outliner::Iterator())
    {
        // Initialize a new search.
        maObjectIterator = ::sd::outliner::OutlinerContainer(this).current();
        maCurrentPosition = *maObjectIterator;

        std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
        if ( ! pViewShell)
        {
            OSL_ASSERT(pViewShell);
            return;
        }

        // In case we are searching in an outline view then first remove the
        // current selection and place cursor at its start or end.
        if( nullptr != dynamic_cast< const OutlineViewShell *>( pViewShell.get() ))
        {
            ESelection aSelection = mpImpl->GetOutlinerView()->GetSelection ();
            if (mbDirectionIsForward)
            {
                aSelection.nEndPara = aSelection.nStartPara;
                aSelection.nEndPos = aSelection.nStartPos;
            }
            else
            {
                aSelection.nStartPara = aSelection.nEndPara;
                aSelection.nStartPos = aSelection.nEndPos;
            }
            mpImpl->GetOutlinerView()->SetSelection (aSelection);
        }

        // When not beginning the search at the beginning of the search area
        // then there may be matches before the current position.
        mbMatchMayExist = (maObjectIterator!=::sd::outliner::OutlinerContainer(this).begin());
    }
    else if (bOldDirectionIsForward != mbDirectionIsForward)
    {
        // Requested iteration direction has changed.  Turn around the iterator.
        maObjectIterator.Reverse();
        if (bIsAtEnd)
        {
            // The iterator has pointed to end(), which after the search
            // direction is reversed, becomes begin().
            maObjectIterator = ::sd::outliner::OutlinerContainer(this).begin();
        }
        else
        {
            // The iterator has pointed to the object one ahead/before the current
            // one.  Now move it to the one before/ahead the current one.
            ++maObjectIterator;
            ++maObjectIterator;
        }

        mbMatchMayExist = true;
    }

    // Initialize the last valid position with where the search starts so
    // that it always points to a valid position.
    maLastValidPosition = *::sd::outliner::OutlinerContainer(this).current();
}

bool Outliner::SearchAndReplaceAll()
{
    DetectChange();

    bool bRet = true;
    // Save the current position to be restored after having replaced all
    // matches.
    RememberStartPosition ();

    std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if ( ! pViewShell)
    {
        OSL_ASSERT(pViewShell);
        return true;
    }

    std::vector<SearchSelection> aSelections;
    if( nullptr != dynamic_cast< const OutlineViewShell *>( pViewShell.get() ))
    {
        // Put the cursor to the beginning/end of the outliner.
        mpImpl->GetOutlinerView()->SetSelection (GetSearchStartPosition ());

        // The outliner does all the work for us when we are in this mode.
        SearchAndReplaceOnce();
    }
    else if( nullptr != dynamic_cast< const DrawViewShell *>( pViewShell.get() ))
    {
        // Disable selection change notifications during search all.
        SfxViewShell& rSfxViewShell = pViewShell->GetViewShellBase();
        if (comphelper::LibreOfficeKit::isViewCallback())
            rSfxViewShell.setTiledSearching(true);
        else
            pViewShell->GetDoc()->setTiledSearching(true);
        comphelper::ScopeGuard aGuard([pViewShell, &rSfxViewShell]()
        {
            if (comphelper::LibreOfficeKit::isViewCallback())
                rSfxViewShell.setTiledSearching(false);
            else
                pViewShell->GetDoc()->setTiledSearching(false);
        });

        // Go to beginning/end of document.
        maObjectIterator = ::sd::outliner::OutlinerContainer(this).begin();
        // Switch to the first object which contains the search string.
        ProvideNextTextObject();
        if( !mbStringFound  )
        {
            RestoreStartPosition ();
            mnStartPageIndex = (sal_uInt16)-1;
            return true;
        }

        // Search/replace until the end of the document is reached.
        bool bFoundMatch;
        do
        {
            bFoundMatch = ! SearchAndReplaceOnce(&aSelections);
            if (mpSearchItem->GetCommand() == SvxSearchCmd::FIND_ALL && comphelper::LibreOfficeKit::isActive() && bFoundMatch && aSelections.size() == 1)
            {
                // Without this, RememberStartPosition() will think it already has a remembered position.
                mnStartPageIndex = (sal_uInt16)-1;

                RememberStartPosition();

                // So when RestoreStartPosition() restores the first match, then spellchecker doesn't kill the selection.
                bRet = false;
            }
        }
        while (bFoundMatch);

        if (mpSearchItem->GetCommand() == SvxSearchCmd::FIND_ALL && comphelper::LibreOfficeKit::isActive() && !aSelections.empty())
        {
            boost::property_tree::ptree aTree;
            aTree.put("searchString", mpSearchItem->GetSearchString().toUtf8().getStr());
            aTree.put("highlightAll", true);

            boost::property_tree::ptree aChildren;
            for (const SearchSelection& rSelection : aSelections)
            {
                boost::property_tree::ptree aChild;
                aChild.put("part", OString::number(rSelection.m_nPage).getStr());
                aChild.put("rectangles", rSelection.m_aRectangles.getStr());
                aChildren.push_back(std::make_pair("", aChild));
            }
            aTree.add_child("searchResultSelection", aChildren);

            std::stringstream aStream;
            boost::property_tree::write_json(aStream, aTree);
            OString aPayload = aStream.str().c_str();
            if (comphelper::LibreOfficeKit::isViewCallback())
                rSfxViewShell.libreOfficeKitViewCallback(LOK_CALLBACK_SEARCH_RESULT_SELECTION, aPayload.getStr());
            else
                pViewShell->GetDoc()->libreOfficeKitCallback(LOK_CALLBACK_SEARCH_RESULT_SELECTION, aPayload.getStr());
        }
    }

    RestoreStartPosition ();

    if (mpSearchItem->GetCommand() == SvxSearchCmd::FIND_ALL && comphelper::LibreOfficeKit::isActive() && !bRet)
    {
        // Find-all, tiled rendering and we have at least one match.
        OString aPayload = OString::number(mnStartPageIndex);
        if (comphelper::LibreOfficeKit::isViewCallback())
        {
            SfxViewShell& rSfxViewShell = pViewShell->GetViewShellBase();
            rSfxViewShell.libreOfficeKitViewCallback(LOK_CALLBACK_SET_PART, aPayload.getStr());
        }
        else
            pViewShell->GetDoc()->libreOfficeKitCallback(LOK_CALLBACK_SET_PART, aPayload.getStr());

        // Emit a selection callback here:
        // 1) The original one is no longer valid, as we there was a SET_PART in between
        // 2) The underlying editeng will only talk about the first match till
        // it doesn't support multi-selection.
        std::vector<OString> aRectangles;
        for (const SearchSelection& rSelection : aSelections)
        {
            if (rSelection.m_nPage == mnStartPageIndex)
                aRectangles.push_back(rSelection.m_aRectangles);
        }
        OString sRectangles = comphelper::string::join("; ", aRectangles);
        if (comphelper::LibreOfficeKit::isViewCallback())
        {
            SfxViewShell& rSfxViewShell = pViewShell->GetViewShellBase();
            rSfxViewShell.libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, sRectangles.getStr());
        }
        else
            pViewShell->GetDoc()->libreOfficeKitCallback(LOK_CALLBACK_TEXT_SELECTION, sRectangles.getStr());
    }

    mnStartPageIndex = (sal_uInt16)-1;

    return bRet;
}

bool Outliner::SearchAndReplaceOnce(std::vector<SearchSelection>* pSelections)
{
    DetectChange ();

    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
    if (!pOutlinerView)
        return true; // end of search

    std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if (pViewShell != nullptr)
    {
        mpView = pViewShell->GetView();
        mpWindow = pViewShell->GetActiveWindow();
        pOutlinerView->SetWindow(mpWindow);

        if( nullptr != dynamic_cast< const DrawViewShell *>( pViewShell.get() ))
        {
            // When replacing we first check if there is a selection
            // indicating a match.  If there is then replace it.  The
            // following call to StartSearchAndReplace will then search for
            // the next match.
            if (meMode == SEARCH
                && mpSearchItem->GetCommand() == SvxSearchCmd::REPLACE)
                if (pOutlinerView->GetSelection().HasRange())
                    pOutlinerView->StartSearchAndReplace(*mpSearchItem);

            // Search for the next match.
            sal_uLong nMatchCount = 0;
            if (mpSearchItem->GetCommand() != SvxSearchCmd::REPLACE_ALL)
                nMatchCount = pOutlinerView->StartSearchAndReplace(*mpSearchItem);

            // Go to the next text object when there have been no matches in
            // the current object or the whole object has already been
            // processed.
            if (nMatchCount==0 || mpSearchItem->GetCommand()==SvxSearchCmd::REPLACE_ALL)
            {
                ProvideNextTextObject ();

                if ( ! mbEndOfSearch)
                {
                    // Remember the current position as the last one with a
                    // text object.
                    maLastValidPosition = maCurrentPosition;

                    // Now that the mbEndOfSearch flag guards this block the
                    // following assertion and return should not be
                    // necessary anymore.
                    DBG_ASSERT(GetEditEngine().HasView(&pOutlinerView->GetEditView() ),
                        "SearchAndReplace without valid view!" );
                    if ( ! GetEditEngine().HasView( &pOutlinerView->GetEditView() ) )
                    {
                        mpDrawDocument->GetDocSh()->SetWaitCursor( false );
                        return true;
                    }

                    if (meMode == SEARCH)
                        nMatchCount = pOutlinerView->StartSearchAndReplace(*mpSearchItem);
                }
            }
        }
        else if( nullptr != dynamic_cast< const OutlineViewShell *>( pViewShell.get() ))
        {
            mpDrawDocument->GetDocSh()->SetWaitCursor(false);
            // The following loop is executed more than once only when a
            // wrap around search is done.
            while (true)
            {
                int nResult = pOutlinerView->StartSearchAndReplace(*mpSearchItem);
                if (nResult == 0)
                {
                    if (HandleFailedSearch ())
                    {
                        pOutlinerView->SetSelection (GetSearchStartPosition ());
                        continue;
                    }
                }
                else
                    mbStringFound = true;
                break;
            }
        }
    }

    mpDrawDocument->GetDocSh()->SetWaitCursor( false );

    if (pViewShell && comphelper::LibreOfficeKit::isActive() && mbStringFound)
    {
        std::vector<Rectangle> aLogicRects;
        pOutlinerView->GetSelectionRectangles(aLogicRects);

        std::vector<OString> aLogicRectStrings;
        std::transform(aLogicRects.begin(), aLogicRects.end(), std::back_inserter(aLogicRectStrings), [](const Rectangle& rRectangle) { return rRectangle.toString(); });
        OString sRectangles = comphelper::string::join("; ", aLogicRectStrings);

        if (!pSelections)
        {
            // notify LibreOfficeKit about changed page
            OString aPayload = OString::number(maCurrentPosition.mnPageIndex);
            if (comphelper::LibreOfficeKit::isViewCallback())
            {
                SfxViewShell& rSfxViewShell = pViewShell->GetViewShellBase();
                rSfxViewShell.libreOfficeKitViewCallback(LOK_CALLBACK_SET_PART, aPayload.getStr());
            }
            else
                pViewShell->GetDoc()->libreOfficeKitCallback(LOK_CALLBACK_SET_PART, aPayload.getStr());

            // also about search result selections
            boost::property_tree::ptree aTree;
            aTree.put("searchString", mpSearchItem->GetSearchString().toUtf8().getStr());
            aTree.put("highlightAll", false);

            boost::property_tree::ptree aChildren;
            boost::property_tree::ptree aChild;
            aChild.put("part", OString::number(maCurrentPosition.mnPageIndex).getStr());
            aChild.put("rectangles", sRectangles.getStr());
            aChildren.push_back(std::make_pair("", aChild));
            aTree.add_child("searchResultSelection", aChildren);

            std::stringstream aStream;
            boost::property_tree::write_json(aStream, aTree);
            aPayload = aStream.str().c_str();
            if (comphelper::LibreOfficeKit::isViewCallback())
            {
                SfxViewShell& rSfxViewShell = pViewShell->GetViewShellBase();
                rSfxViewShell.libreOfficeKitViewCallback(LOK_CALLBACK_SEARCH_RESULT_SELECTION, aPayload.getStr());
            }
            else
                pViewShell->GetDoc()->libreOfficeKitCallback(LOK_CALLBACK_SEARCH_RESULT_SELECTION, aPayload.getStr());
        }
        else
        {
            pSelections->push_back(SearchSelection(maCurrentPosition.mnPageIndex, sRectangles));
        }
    }

    return mbEndOfSearch;
}

/** Try to detect whether the document or the view (shell) has changed since
    the last time <member>StartSearchAndReplace()</member> has been called.
*/
void Outliner::DetectChange()
{
    ::sd::outliner::IteratorPosition aPosition (maCurrentPosition);

    std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    std::shared_ptr<DrawViewShell> pDrawViewShell (
        std::dynamic_pointer_cast<DrawViewShell>(pViewShell));

    // Detect whether the view has been switched from the outside.
    if (pDrawViewShell.get() != nullptr
        && (aPosition.meEditMode != pDrawViewShell->GetEditMode()
            || aPosition.mePageKind != pDrawViewShell->GetPageKind()))
    {
        // Either the edit mode or the page kind has changed.
        SetStatusEventHdl(Link<EditStatus&,void>());

        SdrPageView* pPageView = mpView->GetSdrPageView();
        if (pPageView != nullptr)
            mpView->UnmarkAllObj (pPageView);
        mpView->SdrEndTextEdit();
        SetUpdateMode(false);
        OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
        if (pOutlinerView != nullptr)
            pOutlinerView->SetOutputArea( Rectangle( Point(), Size(1, 1) ) );
        if (meMode == SPELL)
            SetPaperSize( Size(1, 1) );
        SetText(OUString(), GetParagraph(0));

        RememberStartPosition ();

        mnPageCount = mpDrawDocument->GetSdPageCount(pDrawViewShell->GetPageKind());
        maObjectIterator = ::sd::outliner::OutlinerContainer(this).current();
    }

    // Detect change of the set of selected objects.  If their number has
    // changed start again with the first selected object.
    else if (DetectSelectionChange())
    {
        HandleChangedSelection ();
        maObjectIterator = ::sd::outliner::OutlinerContainer(this).current();
    }

    // Detect change of page count.  Restart search at first/last page in
    // that case.
    else if (aPosition.meEditMode == EM_PAGE
        && mpDrawDocument->GetSdPageCount(aPosition.mePageKind) != mnPageCount)
    {
        // The number of pages has changed.
        mnPageCount = mpDrawDocument->GetSdPageCount(aPosition.mePageKind);
        maObjectIterator = ::sd::outliner::OutlinerContainer(this).current();
    }
    else if (aPosition.meEditMode == EM_MASTERPAGE
        && mpDrawDocument->GetSdPageCount(aPosition.mePageKind) != mnPageCount)
    {
        // The number of master pages has changed.
        mnPageCount = mpDrawDocument->GetSdPageCount(aPosition.mePageKind);
        maObjectIterator = ::sd::outliner::OutlinerContainer(this).current();
    }
}

bool Outliner::DetectSelectionChange()
{
    bool bSelectionHasChanged = false;

    // If mpObj is NULL then we have not yet found our first match.
    // Detecting a change makes no sense.
    if (mpObj != nullptr)
    {
        const size_t nMarkCount = mpView ? mpView->GetMarkedObjectList().GetMarkCount() : 0;
        switch (nMarkCount)
        {
            case 0:
                // The selection has changed when previously there have been
                // selected objects.
                bSelectionHasChanged = mbRestrictSearchToSelection;
                break;
            case 1:
                // Check if the only selected object is not the one that we
                // had selected.
                if (mpView != nullptr)
                {
                    SdrMark* pMark = mpView->GetMarkedObjectList().GetMark(0);
                    if (pMark != nullptr)
                        bSelectionHasChanged = (mpObj != pMark->GetMarkedSdrObj ());
                }
                break;
            default:
                // We had selected exactly one object.
                bSelectionHasChanged = true;
                break;
        }
    }

    return bSelectionHasChanged;
}

void Outliner::RememberStartPosition()
{
    std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if ( ! pViewShell)
    {
        OSL_ASSERT(pViewShell);
        return;
    }

    if ( mnStartPageIndex != (sal_uInt16) -1 )
        return;

    if( nullptr != dynamic_cast< const DrawViewShell *>( pViewShell.get() ))
    {
        std::shared_ptr<DrawViewShell> pDrawViewShell (
            std::dynamic_pointer_cast<DrawViewShell>(pViewShell));
        if (pDrawViewShell.get() != nullptr)
        {
            meStartViewMode = pDrawViewShell->GetPageKind();
            meStartEditMode = pDrawViewShell->GetEditMode();
            mnStartPageIndex = pDrawViewShell->GetCurPageId() - 1;
        }

        if (mpView != nullptr)
        {
            mpStartEditedObject = mpView->GetTextEditObject();
            if (mpStartEditedObject != nullptr)
            {
                // Try to retrieve current caret position only when there is an
                // edited object.
                ::Outliner* pOutliner =
                    static_cast<DrawView*>(mpView)->GetTextEditOutliner();
                if (pOutliner!=nullptr && pOutliner->GetViewCount()>0)
                {
                    OutlinerView* pOutlinerView = pOutliner->GetView(0);
                    maStartSelection = pOutlinerView->GetSelection();
                }
            }
        }
    }
    else if( nullptr != dynamic_cast< const OutlineViewShell *>( pViewShell.get() ))
    {
        // Remember the current cursor position.
        OutlinerView* pView = GetView(0);
        if (pView != nullptr)
            pView->GetSelection();
    }
    else
    {
        mnStartPageIndex = (sal_uInt16)-1;
    }
}

void Outliner::RestoreStartPosition()
{
    bool bRestore = true;
    // Take a negative start page index as inidicator that restoring the
    // start position is not requested.
    if (mnStartPageIndex == (sal_uInt16)-1 )
        bRestore = false;
    // Dont't restore when the view shell is not valid.
    std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if (pViewShell == nullptr)
        bRestore = false;

    if (bRestore)
    {
        if( nullptr != dynamic_cast< const DrawViewShell *>( pViewShell.get() ))
        {
            std::shared_ptr<DrawViewShell> pDrawViewShell (
                std::dynamic_pointer_cast<DrawViewShell>(pViewShell));
            SetViewMode (meStartViewMode);
            if (pDrawViewShell.get() != nullptr)
            {
                SetPage (meStartEditMode, mnStartPageIndex);
                mpObj = mpStartEditedObject;
                if (mpObj)
                {
                    PutTextIntoOutliner();
                    EnterEditMode(false);
                    if (OutlinerView* pOutlinerView = mpImpl->GetOutlinerView())
                        pOutlinerView->SetSelection(maStartSelection);
                }
            }
        }
        else if( nullptr != dynamic_cast< const OutlineViewShell *>( pViewShell.get() ))
        {
            // Set cursor to its old position.
            OutlinerView* pView = GetView(0);
            if (pView != nullptr)
                pView->SetSelection (maStartSelection);
        }
    }
}

/** The main purpose of this method is to iterate over all shape objects of
    the search area (current selection, current view, or whole document)
    until a text object has been found that contains at least one match or
    until no such object can be found anymore.   These two conditions are
    expressed by setting one of the flags <member>mbFoundObject</member> or
    <member>mbEndOfSearch</member> to <TRUE/>.
*/
void Outliner::ProvideNextTextObject()
{
    mbEndOfSearch = false;
    mbFoundObject = false;

    mpView->UnmarkAllObj (mpView->GetSdrPageView());
    try
    {
        mpView->SdrEndTextEdit();
    }
    catch (const css::uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    SetUpdateMode(false);
    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
    if (pOutlinerView != nullptr)
        pOutlinerView->SetOutputArea( Rectangle( Point(), Size(1, 1) ) );
    if (meMode == SPELL)
        SetPaperSize( Size(1, 1) );
    SetText(OUString(), GetParagraph(0));

    mpTextObj = nullptr;

    // Iterate until a valid text object has been found or the search ends.
    do
    {
        mpObj = nullptr;
        mpParaObj = nullptr;

        if (maObjectIterator != ::sd::outliner::OutlinerContainer(this).end())
        {
            maCurrentPosition = *maObjectIterator;
            // Switch to the current object only if it is a valid text object.
            if (IsValidTextObject (maCurrentPosition))
            {
                // Don't set yet in case of searching: the text object may not match.
                if (meMode != SEARCH)
                    mpObj = SetObject(maCurrentPosition);
                else
                    mpObj = maCurrentPosition.mxObject.get();
            }
            ++maObjectIterator;

            if (mpObj != nullptr)
            {
                PutTextIntoOutliner ();

                std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
                if (pViewShell != nullptr)
                    switch (meMode)
                    {
                        case SEARCH:
                            PrepareSearchAndReplace ();
                            break;
                        case SPELL:
                            PrepareSpellCheck ();
                            break;
                        case TEXT_CONVERSION:
                            PrepareConversion();
                            break;
                    }
            }
        }
        else
        {
            if (meMode == SEARCH)
                // Instead of doing a full-blown SetObject(), which would do the same -- but would also possibly switch pages.
                mbStringFound = false;

            mbEndOfSearch = true;
            EndOfSearch ();
        }
    }
    while ( ! (mbFoundObject || mbEndOfSearch));
}

void Outliner::EndOfSearch()
{
    std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if ( ! pViewShell)
    {
        OSL_ASSERT(pViewShell);
        return;
    }

    // Before we display a dialog we first jump to where the last valid text
    // object was found.  All page and view mode switching since then was
    // temporary and should not be visible to the user.
    if(  nullptr == dynamic_cast< const OutlineViewShell *>( pViewShell.get() ))
        SetObject (maLastValidPosition);

    if (mbRestrictSearchToSelection)
        ShowEndOfSearchDialog ();
    else
    {
        // When no match has been found so far then terminate the search.
        if ( ! mbMatchMayExist)
        {
            ShowEndOfSearchDialog ();
            mbEndOfSearch = true;
        }
        // Ask the user whether to wrap around and continue the search or
        // to terminate.
        else if (meMode==TEXT_CONVERSION || ShowWrapArroundDialog ())
        {
            mbMatchMayExist = false;
            // Everything back to beginning (or end?) of the document.
            maObjectIterator = ::sd::outliner::OutlinerContainer(this).begin();
            if( nullptr != dynamic_cast< const OutlineViewShell *>( pViewShell.get() ))
            {
                // Set cursor to first character of the document.
                OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
                if (pOutlinerView != nullptr)
                    pOutlinerView->SetSelection (GetSearchStartPosition ());
            }

            mbEndOfSearch = false;
        }
        else
        {
            // No wrap around.
            mbEndOfSearch = true;
        }
    }
}

void Outliner::ShowEndOfSearchDialog()
{
    mbWholeDocumentProcessed = true;

    if (meMode == SEARCH)
    {
        if (!mbStringFound)
        {
            SvxSearchDialogWrapper::SetSearchLabel(SL_NotFound);
            if (comphelper::LibreOfficeKit::isViewCallback())
            {
                std::shared_ptr<ViewShell> pViewShell(mpWeakViewShell.lock());
                if (pViewShell)
                {
                    SfxViewShell& rSfxViewShell = pViewShell->GetViewShellBase();
                    rSfxViewShell.libreOfficeKitViewCallback(LOK_CALLBACK_SEARCH_NOT_FOUND, mpSearchItem->GetSearchString().toUtf8().getStr());
                }
            }
            else
                mpDrawDocument->libreOfficeKitCallback(LOK_CALLBACK_SEARCH_NOT_FOUND, mpSearchItem->GetSearchString().toUtf8().getStr());
        }

        // don't do anything else for search
        return;
    }

    OUString aString;
    if (mpView->AreObjectsMarked())
        aString = SD_RESSTR(STR_END_SPELLING_OBJ);
    else
        aString = SD_RESSTR(STR_END_SPELLING);

    // Show the message in an info box that is modal with respect to the
    // whole application.
    ScopedVclPtrInstance< MessageDialog > aInfoBox(nullptr, aString, VCL_MESSAGE_INFO);
    ShowModalMessageBox (*aInfoBox.get());
}

bool Outliner::ShowWrapArroundDialog()
{
    // Determine whether to show the dialog.
    if (mpSearchItem)
    {
        // When searching display the dialog only for single find&replace.
        const SvxSearchCmd nCommand(mpSearchItem->GetCommand());
        if (nCommand == SvxSearchCmd::REPLACE || nCommand == SvxSearchCmd::FIND)
        {
            if (mbDirectionIsForward)
                SvxSearchDialogWrapper::SetSearchLabel(SL_End);
            else
                SvxSearchDialogWrapper::SetSearchLabel(SL_Start);

            return true;
        }
        else
            return false;
    }

    // show dialog only for spelling
    if (meMode != SPELL)
        return false;

    // The question text depends on the search direction.
    bool bImpress = mpDrawDocument && mpDrawDocument->GetDocumentType() == DOCUMENT_TYPE_IMPRESS;

    sal_uInt16 nStringId;
    if (mbDirectionIsForward)
        nStringId = bImpress ? STR_SAR_WRAP_FORWARD : STR_SAR_WRAP_FORWARD_DRAW;
    else
        nStringId = bImpress ? STR_SAR_WRAP_BACKWARD : STR_SAR_WRAP_BACKWARD_DRAW;

    // Pop up question box that asks the user whether to wrap around.
    // The dialog is made modal with respect to the whole application.
    ScopedVclPtrInstance<QueryBox> aQuestionBox(nullptr, WB_YES_NO | WB_DEF_YES, SD_RESSTR(nStringId));
    aQuestionBox->SetImage(QueryBox::GetStandardImage());
    sal_uInt16 nBoxResult = ShowModalMessageBox(*aQuestionBox.get());

    return (nBoxResult == RET_YES);
}

bool Outliner::IsValidTextObject (const ::sd::outliner::IteratorPosition& rPosition)
{
    // TODO implement iteration through table cells and remove this workaround
    ::sdr::table::SdrTableObj* pTableObject = dynamic_cast< ::sdr::table::SdrTableObj* >( rPosition.mxObject.get() );
    if( pTableObject != nullptr )
        return false;

    SdrTextObj* pObject = dynamic_cast< SdrTextObj* >( rPosition.mxObject.get() );
    return (pObject != nullptr) && pObject->HasText() && ! pObject->IsEmptyPresObj();
}

void Outliner::PutTextIntoOutliner()
{
    mpTextObj = dynamic_cast<SdrTextObj*>( mpObj );
    if ( mpTextObj && mpTextObj->HasText() && !mpTextObj->IsEmptyPresObj() )
    {
        SdrText* pText = mpTextObj->getText( maCurrentPosition.mnText );
        mpParaObj = pText ? pText->GetOutlinerParaObject() : nullptr;

        if (mpParaObj != nullptr)
        {
            SetText(*mpParaObj);

            ClearModifyFlag();
        }
    }
    else
    {
        mpTextObj = nullptr;
    }
}

void Outliner::PrepareSpellCheck()
{
    EESpellState eState = HasSpellErrors();
    DBG_ASSERT(eState != EE_SPELL_NOSPELLER, "No SpellChecker");

    if (eState == EE_SPELL_NOLANGUAGE)
    {
        mbError = true;
        mbEndOfSearch = true;
        ScopedVclPtrInstance<MessageDialog> aErrorBox (
            nullptr, SD_RESSTR(STR_NOLANGUAGE));
        ShowModalMessageBox (*aErrorBox.get());
    }
    else if (eState != EE_SPELL_OK)
    {
        // When spell checking we have to test whether we have processed the
        // whole document and have reached the start page again.
        if (meMode == SPELL)
        {
            if (maSearchStartPosition == ::sd::outliner::Iterator())
                // Remember the position of the first text object so that we
                // know when we have processed the whole document.
                maSearchStartPosition = maObjectIterator;
            else if (maSearchStartPosition == maObjectIterator)
            {
                mbEndOfSearch = true;
            }
        }

        EnterEditMode( false );
    }
}

void Outliner::PrepareSearchAndReplace()
{
    if (HasText( *mpSearchItem ))
    {
        // Set the object now that we know it matches.
        mpObj = SetObject(maCurrentPosition);

        mbStringFound = true;
        mbMatchMayExist = true;

        EnterEditMode(false);

        mpDrawDocument->GetDocSh()->SetWaitCursor( false );
        // Start search at the right end of the current object's text
        // depending on the search direction.
        OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
        if (pOutlinerView != nullptr)
            pOutlinerView->SetSelection (GetSearchStartPosition ());
    }
}

void Outliner::SetViewMode (PageKind ePageKind)
{
    std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    std::shared_ptr<DrawViewShell> pDrawViewShell(
        std::dynamic_pointer_cast<DrawViewShell>(pViewShell));
    if (pDrawViewShell.get()!=nullptr && ePageKind != pDrawViewShell->GetPageKind())
    {
        // Restore old edit mode.
        pDrawViewShell->ChangeEditMode(mpImpl->meOriginalEditMode, false);

        SetStatusEventHdl(Link<EditStatus&,void>());
        OUString sViewURL;
        switch (ePageKind)
        {
            case PK_STANDARD:
            default:
                sViewURL = framework::FrameworkHelper::msImpressViewURL;
                break;
            case PK_NOTES:
                sViewURL = framework::FrameworkHelper::msNotesViewURL;
                break;
            case PK_HANDOUT:
                sViewURL = framework::FrameworkHelper::msHandoutViewURL;
                break;
        }
        // The text object iterator is destroyed when the shells are
        // switched but we need it so save it and restore it afterwards.
        ::sd::outliner::Iterator aIterator (maObjectIterator);
        bool bMatchMayExist = mbMatchMayExist;

        ViewShellBase& rBase = pViewShell->GetViewShellBase();
        SetViewShell(std::shared_ptr<ViewShell>());
        framework::FrameworkHelper::Instance(rBase)->RequestView(
            sViewURL,
            framework::FrameworkHelper::msCenterPaneURL);

        // Force (well, request) a synchronous update of the configuration.
        // In a better world we would handle the asynchronous view update
        // instead.  But that would involve major restructuring of the
        // Outliner code.
        framework::FrameworkHelper::Instance(rBase)->RequestSynchronousUpdate();
        SetViewShell(rBase.GetMainViewShell());

        // Switching to another view shell has intermediatly called
        // EndSpelling().  A PrepareSpelling() is pending, so call that now.
        PrepareSpelling();

        // Update the number of pages so that
        // <member>DetectChange()</member> has the correct value to compare
        // to.
        mnPageCount = mpDrawDocument->GetSdPageCount(ePageKind);

        maObjectIterator = aIterator;
        mbMatchMayExist = bMatchMayExist;

        // Save edit mode so that it can be restored when switching the view
        // shell again.
        pDrawViewShell = std::dynamic_pointer_cast<DrawViewShell>(pViewShell);
        OSL_ASSERT(pDrawViewShell.get()!=nullptr);
        if (pDrawViewShell.get() != nullptr)
            mpImpl->meOriginalEditMode = pDrawViewShell->GetEditMode();
    }
}

void Outliner::SetPage (EditMode eEditMode, sal_uInt16 nPageIndex)
{
    if ( ! mbRestrictSearchToSelection)
    {
        std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
        std::shared_ptr<DrawViewShell> pDrawViewShell(
            std::dynamic_pointer_cast<DrawViewShell>(pViewShell));
        OSL_ASSERT(pDrawViewShell.get()!=nullptr);
        if (pDrawViewShell.get() != nullptr)
        {
            pDrawViewShell->ChangeEditMode(eEditMode, false);
            pDrawViewShell->SwitchPage(nPageIndex);
        }
    }
}

void Outliner::EnterEditMode (bool bGrabFocus)
{
    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
    if (pOutlinerView && mpTextObj)
    {
        pOutlinerView->SetOutputArea( Rectangle( Point(), Size(1, 1)));
        SetPaperSize( mpTextObj->GetLogicRect().GetSize() );
        SdrPageView* pPV = mpView->GetSdrPageView();

        // Make FuText the current function.
        SfxUInt16Item aItem (SID_TEXTEDIT, 1);
        std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
        pViewShell->GetDispatcher()->
            Execute(SID_TEXTEDIT, SfxCallMode::SYNCHRON |
                SfxCallMode::RECORD, &aItem, 0L);

        // To be consistent with the usual behaviour in the Office the text
        // object that is put into edit mode would have also to be selected.
        // Starting the text edit mode is not enough so we do it here by
        // hand.
        mbExpectingSelectionChangeEvent = true;
        mpView->UnmarkAllObj (pPV);
        mpView->MarkObj (mpTextObj, pPV);

        mpTextObj->setActiveText( mnText );

        // Turn on the edit mode for the text object.
        mpView->SdrBeginTextEdit(mpTextObj, pPV, mpWindow, true, this, pOutlinerView, true, true, bGrabFocus);

        SetUpdateMode(true);
        mbFoundObject = true;
    }
}

ESelection Outliner::GetSearchStartPosition()
{
    ESelection aPosition;
    if (mbDirectionIsForward)
    {
        // The default constructor uses the beginning of the text as default.
        aPosition = ESelection ();
    }
    else
    {
        // Retrieve the position after the last character in the last
        // paragraph.
        sal_Int32 nParagraphCount = GetParagraphCount();
        if (nParagraphCount == 0)
            aPosition = ESelection();
        else
        {
            sal_Int32 nLastParagraphLength = GetEditEngine().GetTextLen (
                nParagraphCount-1);
            aPosition = ESelection (nParagraphCount-1, nLastParagraphLength);
        }
    }

    return aPosition;
}

bool Outliner::HasNoPreviousMatch()
{
    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();

    DBG_ASSERT (pOutlinerView!=nullptr, "outline view in Outliner::HasNoPreviousMatch is NULL");

    // Detect whether the cursor stands at the beginning
    // resp. at the end of the text.
    return pOutlinerView->GetSelection().IsEqual(GetSearchStartPosition ());
}

bool Outliner::HandleFailedSearch()
{
    bool bContinueSearch = false;

    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
    if (pOutlinerView != nullptr && mpSearchItem != nullptr)
    {
        // Detect whether there is/may be a prior match.  If there is then
        // ask the user whether to wrap around.  Otherwise tell the user
        // that there is no match.
        if (HasNoPreviousMatch ())
        {
            // No match found in the whole presentation.
            SvxSearchDialogWrapper::SetSearchLabel(SL_NotFound);
        }

        else
        {
            // No further matches found.  Ask the user whether to wrap
            // around and start again.
            bContinueSearch = ShowWrapArroundDialog();
        }
    }

    return bContinueSearch;
}

SdrObject* Outliner::SetObject (
    const ::sd::outliner::IteratorPosition& rPosition)
{
    SetViewMode (rPosition.mePageKind);
    SetPage (rPosition.meEditMode, (sal_uInt16)rPosition.mnPageIndex);
    mnText = rPosition.mnText;
    return rPosition.mxObject.get();
}

void Outliner::SetViewShell (const std::shared_ptr<ViewShell>& rpViewShell)
{
    std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if (pViewShell != rpViewShell)
    {
        // Set the new view shell.
        mpWeakViewShell = rpViewShell;
        // When the outline view is not owned by us then we have to clear
        // that pointer so that the current one for the new view shell will
        // be used (in ProvideOutlinerView).
        if (rpViewShell)
        {
            mpView = rpViewShell->GetView();

            mpWindow = rpViewShell->GetActiveWindow();

            mpImpl->ProvideOutlinerView(*this, rpViewShell, mpWindow);
            OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
            if (pOutlinerView != nullptr)
                pOutlinerView->SetWindow(mpWindow);
        }
        else
        {
            mpView = nullptr;
            mpWindow = nullptr;
        }
    }
}

void Outliner::HandleChangedSelection()
{
    maMarkListCopy.clear();
    mbRestrictSearchToSelection = mpView->AreObjectsMarked();
    if (mbRestrictSearchToSelection)
    {
        // Make a copy of the current mark list.
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
        const size_t nCount = rMarkList.GetMarkCount();
        if (nCount > 0)
        {
            maMarkListCopy.clear();
            maMarkListCopy.reserve (nCount);
            for (size_t i=0; i<nCount; ++i)
                maMarkListCopy.push_back (rMarkList.GetMark(i)->GetMarkedSdrObj ());
        }
        else
            // No marked object.  Is this case possible?
            mbRestrictSearchToSelection = false;
    }
}

void Outliner::StartConversion( sal_Int16 nSourceLanguage,  sal_Int16 nTargetLanguage,
        const vcl::Font *pTargetFont, sal_Int32 nOptions, bool bIsInteractive )
{
    std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    bool bMultiDoc = nullptr != dynamic_cast< const DrawViewShell *>( pViewShell.get() );

    meMode = TEXT_CONVERSION;
    mbDirectionIsForward = true;
    mpSearchItem = nullptr;
    mnConversionLanguage = nSourceLanguage;

    BeginConversion();

    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
    if (pOutlinerView != nullptr)
    {
        pOutlinerView->StartTextConversion(
            nSourceLanguage,
            nTargetLanguage,
            pTargetFont,
            nOptions,
            bIsInteractive,
            bMultiDoc);
    }

    EndConversion();
}

/** Prepare to do a text conversion on the current text object. This
    includes putting it into edit mode.
*/
void Outliner::PrepareConversion()
{
    SetUpdateMode(true);
    if( HasConvertibleTextPortion( mnConversionLanguage ) )
    {
        SetUpdateMode(false);
        mbStringFound = true;
        mbMatchMayExist = true;

        EnterEditMode ();

        mpDrawDocument->GetDocSh()->SetWaitCursor( false );
        // Start search at the right end of the current object's text
        // depending on the search direction.
    }
    else
    {
        SetUpdateMode(false);
    }
}

void Outliner::BeginConversion()
{
    SetRefDevice( SD_MOD()->GetRefDevice( *mpDrawDocument->GetDocSh() ) );

    ViewShellBase* pBase = dynamic_cast<ViewShellBase*>( SfxViewShell::Current() );
    if (pBase != nullptr)
        SetViewShell (pBase->GetMainViewShell());

    std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if (pViewShell)
    {
        mbStringFound = false;

        // Supposed that we are not located at the very beginning/end of the
        // document then there may be a match in the document prior/after
        // the current position.
        mbMatchMayExist = true;

        maObjectIterator = ::sd::outliner::Iterator();
        maSearchStartPosition = ::sd::outliner::Iterator();
        RememberStartPosition();

        mpImpl->ProvideOutlinerView(*this, pViewShell, mpWindow);

        HandleChangedSelection ();
    }
    ClearModifyFlag();
}

void Outliner::EndConversion()
{
    EndSpelling();
}

bool Outliner::ConvertNextDocument()
{
    std::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if (pViewShell && nullptr != dynamic_cast< const OutlineViewShell *>( pViewShell.get() ) )
        return false;

    mpDrawDocument->GetDocSh()->SetWaitCursor( true );

    Initialize ( true );

    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
    if (pOutlinerView != nullptr)
    {
        mpWindow = pViewShell->GetActiveWindow();
        pOutlinerView->SetWindow(mpWindow);
    }
    ProvideNextTextObject ();

    mpDrawDocument->GetDocSh()->SetWaitCursor( false );
    ClearModifyFlag();

    // for text conversion we automatically wrap around one
    // time and stop at the start shape
    if( mpFirstObj )
    {
        if( (mnText == 0) && (mpFirstObj == mpObj) )
            return false;
    }
    else
    {
        mpFirstObj = mpObj;
    }

    return !mbEndOfSearch;
}

sal_uInt16 Outliner::ShowModalMessageBox (Dialog& rMessageBox)
{
    // We assume that the parent of the given message box is NULL, i.e. it is
    // modal with respect to the top application window. However, this
    // does not affect the search dialog. Therefore we have to lock it here
    // while the message box is being shown. We also have to take into
    // account that we are called during a spell check and the search dialog
    // is not available.
    vcl::Window* pSearchDialog = nullptr;
    SfxChildWindow* pChildWindow = nullptr;
    switch (meMode)
    {
        case SEARCH:
            pChildWindow = SfxViewFrame::Current()->GetChildWindow(
                SvxSearchDialogWrapper::GetChildWindowId());
            break;

        case SPELL:
            pChildWindow = SfxViewFrame::Current()->GetChildWindow(
                SpellDialogChildWindow::GetChildWindowId());
            break;

        case TEXT_CONVERSION:
            // There should no messages boxes be displayed while doing the
            // hangul hanja conversion.
            break;
    }

    if (pChildWindow != nullptr)
        pSearchDialog = pChildWindow->GetWindow();
    if (pSearchDialog != nullptr)
        pSearchDialog->EnableInput(false);

    sal_uInt16 nResult = rMessageBox.Execute();

    // Unlock the search dialog.
    if (pSearchDialog != nullptr)
        pSearchDialog->EnableInput();

    return nResult;
}

//===== Outliner::Implementation ==============================================

Outliner::Implementation::Implementation()
    : meOriginalEditMode(EM_PAGE),
      mbOwnOutlineView(false),
      mpOutlineView(nullptr)
{
}

Outliner::Implementation::~Implementation()
{
    if (mbOwnOutlineView && mpOutlineView!=nullptr)
    {
        mpOutlineView->SetWindow(nullptr);
        delete mpOutlineView;
        mpOutlineView = nullptr;
    }
}

/** We try to create a new OutlinerView only when there is none available,
    either from an OutlinerViewShell or a previous call to
    ProvideOutlinerView().  This is necessary to support the spell checker
    which can not cope with exchanging the OutlinerView.
*/
void Outliner::Implementation::ProvideOutlinerView (
    Outliner& rOutliner,
    const std::shared_ptr<ViewShell>& rpViewShell,
    vcl::Window* pWindow)
{
    if (rpViewShell.get() != nullptr)
    {
        switch (rpViewShell->GetShellType())
        {
            case ViewShell::ST_DRAW:
            case ViewShell::ST_IMPRESS:
            case ViewShell::ST_NOTES:
            case ViewShell::ST_HANDOUT:
            {
                // Create a new outline view to do the search on.
                bool bInsert = false;
                if (mpOutlineView!=nullptr && !mbOwnOutlineView)
                    mpOutlineView = nullptr;
                if (mpOutlineView == nullptr)
                {
                    mpOutlineView = new OutlinerView(&rOutliner, pWindow);
                    mbOwnOutlineView = true;
                    bInsert = true;
                }
                else
                    mpOutlineView->SetWindow(pWindow);
                EVControlBits nStat = mpOutlineView->GetControlWord();
                nStat &= ~EVControlBits::AUTOSCROLL;
                mpOutlineView->SetControlWord(nStat);
                if (bInsert)
                    rOutliner.InsertView( mpOutlineView );
                rOutliner.SetUpdateMode(false);
                mpOutlineView->SetOutputArea (Rectangle (Point(), Size(1, 1)));
                rOutliner.SetPaperSize( Size(1, 1) );
                rOutliner.SetText(OUString(), rOutliner.GetParagraph(0));

                meOriginalEditMode =
                    std::static_pointer_cast<DrawViewShell>(rpViewShell)->GetEditMode();
            }
            break;

            case ViewShell::ST_OUTLINE:
            {
                if (mpOutlineView!=nullptr && mbOwnOutlineView)
                    delete mpOutlineView;
                mpOutlineView = rOutliner.GetView(0);
                mbOwnOutlineView = false;
            }
            break;

            default:
            case ViewShell::ST_NONE:
            case ViewShell::ST_PRESENTATION:
                // Ignored
                break;
        }
    }
}

void Outliner::Implementation::ReleaseOutlinerView()
{
    if (mbOwnOutlineView)
    {
        OutlinerView* pView = mpOutlineView;
        mpOutlineView = nullptr;
        mbOwnOutlineView = false;
        if (pView != nullptr)
        {
            pView->SetWindow(nullptr);
            delete pView;
        }
    }
    else
    {
        mpOutlineView = nullptr;
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
