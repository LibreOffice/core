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

#include <Outliner.hxx>
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
#include <vcl/weld.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/printer.hxx>
#include <svx/svxerr.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdotable.hxx>
#include <editeng/unolingu.hxx>
#include <svx/svditer.hxx>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <editeng/forbiddencharacterstable.hxx>
#include <svx/srchdlg.hxx>
#include <unotools/linguprops.hxx>
#include <unotools/lingucfg.hxx>
#include <editeng/editeng.hxx>
#include <vcl/metric.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/langtab.hxx>
#include <tools/diagnose_ex.h>

#include <strings.hrc>
#include <editeng/outliner.hxx>
#include <sdpage.hxx>
#include <sdmod.hxx>
#include <Window.hxx>
#include <sdresid.hxx>
#include <DrawViewShell.hxx>
#include <OutlineView.hxx>
#include <OutlineViewShell.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <FrameView.hxx>
#include <optsitem.hxx>
#include <drawview.hxx>
#include <ViewShellBase.hxx>
#include <SpellDialogChildWindow.hxx>
#include <ToolBarManager.hxx>
#include <framework/FrameworkHelper.hxx>
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

} // end of namespace sd


class SdOutliner::Implementation
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
        or is created.  When an OutlinerView already exists it is initialized.
    */
    void ProvideOutlinerView (
        Outliner& rOutliner,
        const std::shared_ptr<sd::ViewShell>& rpViewShell,
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

SdOutliner::SdOutliner( SdDrawDocument* pDoc, OutlinerMode nMode )
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
      mbDirectionIsForward(true),
      mbRestrictSearchToSelection(false),
      maMarkListCopy(),
      mpObj(nullptr),
      mpFirstObj(nullptr),
      mpSearchSpellTextObj(nullptr),
      mnText(0),
      mpParaObj(nullptr),
      meStartViewMode(PageKind::Standard),
      meStartEditMode(EditMode::Page),
      mnStartPageIndex(sal_uInt16(-1)),
      mpStartEditedObject(nullptr),
      maStartSelection(),
      mpSearchItem(nullptr),
      maObjectIterator(),
      maCurrentPosition(),
      maSearchStartPosition(),
      maLastValidPosition(),
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

    sd::DrawDocShell* pDocSh = mpDrawDocument->GetDocSh();

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

/// Nothing spectacular in the destructor.
SdOutliner::~SdOutliner()
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
void SdOutliner::PrepareSpelling()
{
    mbPrepareSpellingPending = false;

    sd::ViewShellBase* pBase = dynamic_cast< sd::ViewShellBase *>( SfxViewShell::Current() );
    if (pBase != nullptr)
        SetViewShell (pBase->GetMainViewShell());
    SetRefDevice( SD_MOD()->GetVirtualRefDevice() );

    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
    if (pViewShell)
    {
        mbStringFound = false;

        // Supposed that we are not located at the very beginning/end of
        // the document then there may be a match in the document
        // prior/after the current position.
        mbMatchMayExist = true;

        maObjectIterator = sd::outliner::Iterator();
        maSearchStartPosition = sd::outliner::Iterator();
        RememberStartPosition();

        mpImpl->ProvideOutlinerView(*this, pViewShell, mpWindow);

        HandleChangedSelection ();
    }
    ClearModifyFlag();
}

void SdOutliner::StartSpelling()
{
    meMode = SPELL;
    mbDirectionIsForward = true;
    mpSearchItem = nullptr;
}

/** Free all resources acquired during the search/spell check.  After a
    spell check the start position is restored here.
*/
void SdOutliner::EndSpelling()
{
    // Keep old view shell alive until we release the outliner view.
    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
    std::shared_ptr<sd::ViewShell> pOldViewShell (pViewShell);

    sd::ViewShellBase* pBase = dynamic_cast< sd::ViewShellBase *>( SfxViewShell::Current() );
    if (pBase != nullptr)
        pViewShell = pBase->GetMainViewShell();
    else
        pViewShell.reset();
    mpWeakViewShell = pViewShell;

    // When in <member>PrepareSpelling()</member> a new outline view has
    // been created then delete it here.
    bool bViewIsDrawViewShell(dynamic_cast< const sd::DrawViewShell *>( pViewShell.get() ));
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
        if(auto pOutlineView = dynamic_cast<sd::OutlineView *>( mpView ))
            pOutlineView->PrepareClose();
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
    mnStartPageIndex = sal_uInt16(-1);
}

bool SdOutliner::SpellNextDocument()
{
    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
    if( nullptr != dynamic_cast< const sd::OutlineViewShell *>( pViewShell.get() ))
    {
        // When doing a spell check in the outline view then there is
        // only one document.
        mbEndOfSearch = true;
        EndOfSearch ();
    }
    else
    {
        if( dynamic_cast< const sd::OutlineView *>( mpView ) !=  nullptr)
            static_cast<sd::OutlineView*>(mpView)->PrepareClose();
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
svx::SpellPortions SdOutliner::GetNextSpellSentence()
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
                && maStartSelection < aCurrentSelection)
                EndOfSearch();

            // Advance to the next sentence.
            bFoundNextSentence = SpellSentence( pOutlinerView->GetEditView(), aResult);
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
bool SdOutliner::StartSearchAndReplace (const SvxSearchItem* pSearchItem)
{
    bool bEndOfSearch = true;

    // clear the search toolbar entry
    SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::Empty);

    mpDrawDocument->GetDocSh()->SetWaitCursor( true );
    if (mbPrepareSpellingPending)
        PrepareSpelling();
    sd::ViewShellBase* pBase = dynamic_cast< sd::ViewShellBase *>( SfxViewShell::Current() );
    // Determine whether we have to abort the search.  This is necessary
    // when the main view shell does not support searching.
    bool bAbort = false;
    if (pBase != nullptr)
    {
        std::shared_ptr<sd::ViewShell> pShell (pBase->GetMainViewShell());
        SetViewShell(pShell);
        if (pShell == nullptr)
            bAbort = true;
        else
            switch (pShell->GetShellType())
            {
                case sd::ViewShell::ST_DRAW:
                case sd::ViewShell::ST_IMPRESS:
                case sd::ViewShell::ST_NOTES:
                case sd::ViewShell::ST_HANDOUT:
                case sd::ViewShell::ST_OUTLINE:
                    bAbort = false;
                    break;
                default:
                    bAbort = true;
                    break;
            }
    }

    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
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
            mnStartPageIndex = sal_uInt16(-1);
        }

        SfxChildWindow *pChildWin =
            SfxViewFrame::Current()->GetChildWindow(
            SvxSearchDialogWrapper::GetChildWindowId());
        if (pChildWin)
        {
            SvxSearchDialog* pSearchDlg =
                static_cast<SvxSearchDialog*>(pChildWin->GetController().get());
            pSearchDlg->SetDocWin( pViewShell->GetActiveWindow() );
            pSearchDlg->SetSrchFlag(false);
        }
    }
    else
        mpDrawDocument->GetDocSh()->SetWaitCursor( false );

    return bEndOfSearch;
}

void SdOutliner::Initialize (bool bDirectionIsForward)
{
    const bool bIsAtEnd (maObjectIterator == sd::outliner::OutlinerContainer(this).end());
    const bool bOldDirectionIsForward = mbDirectionIsForward;
    mbDirectionIsForward = bDirectionIsForward;

    if (maObjectIterator == sd::outliner::Iterator())
    {
        // Initialize a new search.
        maObjectIterator = sd::outliner::OutlinerContainer(this).current();
        maCurrentPosition = *maObjectIterator;

        std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
        if ( ! pViewShell)
        {
            OSL_ASSERT(pViewShell);
            return;
        }

        // In case we are searching in an outline view then first remove the
        // current selection and place cursor at its start or end.
        if( nullptr != dynamic_cast< const sd::OutlineViewShell *>( pViewShell.get() ))
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
        mbMatchMayExist = (maObjectIterator!=sd::outliner::OutlinerContainer(this).begin());
    }
    else if (bOldDirectionIsForward != mbDirectionIsForward)
    {
        // Requested iteration direction has changed.  Turn around the iterator.
        maObjectIterator.Reverse();
        if (bIsAtEnd)
        {
            // The iterator has pointed to end(), which after the search
            // direction is reversed, becomes begin().
            maObjectIterator = sd::outliner::OutlinerContainer(this).begin();
        }
        else
        {
            // The iterator has pointed to the object one ahead/before the current
            // one.  Now move it to the one before/ahead the current one.
            ++maObjectIterator;
            if (maObjectIterator != sd::outliner::OutlinerContainer(this).end())
            {
                ++maObjectIterator;
            }
        }

        mbMatchMayExist = true;
    }

    // Initialize the last valid position with where the search starts so
    // that it always points to a valid position.
    maLastValidPosition = *sd::outliner::OutlinerContainer(this).current();
}

bool SdOutliner::SearchAndReplaceAll()
{
    bool bRet = true;
    // Save the current position to be restored after having replaced all
    // matches.
    RememberStartPosition ();

    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
    if ( ! pViewShell)
    {
        OSL_ASSERT(pViewShell);
        return true;
    }

    std::vector<sd::SearchSelection> aSelections;
    if( nullptr != dynamic_cast< const sd::OutlineViewShell *>( pViewShell.get() ))
    {
        // Put the cursor to the beginning/end of the outliner.
        mpImpl->GetOutlinerView()->SetSelection (GetSearchStartPosition ());

        // The outliner does all the work for us when we are in this mode.
        SearchAndReplaceOnce();
    }
    else if( nullptr != dynamic_cast< const sd::DrawViewShell *>( pViewShell.get() ))
    {
        // Disable selection change notifications during search all.
        SfxViewShell& rSfxViewShell = pViewShell->GetViewShellBase();
        rSfxViewShell.setTiledSearching(true);
        comphelper::ScopeGuard aGuard([&rSfxViewShell]()
        {
            rSfxViewShell.setTiledSearching(false);
        });

        // Go to beginning/end of document.
        maObjectIterator = sd::outliner::OutlinerContainer(this).begin();
        // Switch to the first object which contains the search string.
        ProvideNextTextObject();
        if( !mbStringFound  )
        {
            RestoreStartPosition ();
            mnStartPageIndex = sal_uInt16(-1);
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
                mnStartPageIndex = sal_uInt16(-1);

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
            for (const sd::SearchSelection& rSelection : aSelections)
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
            rSfxViewShell.libreOfficeKitViewCallback(LOK_CALLBACK_SEARCH_RESULT_SELECTION, aPayload.getStr());
        }
    }

    RestoreStartPosition ();

    if (mpSearchItem->GetCommand() == SvxSearchCmd::FIND_ALL && comphelper::LibreOfficeKit::isActive() && !bRet)
    {
        // Find-all, tiled rendering and we have at least one match.
        OString aPayload = OString::number(mnStartPageIndex);
        SfxViewShell& rSfxViewShell = pViewShell->GetViewShellBase();
        rSfxViewShell.libreOfficeKitViewCallback(LOK_CALLBACK_SET_PART, aPayload.getStr());

        // Emit a selection callback here:
        // 1) The original one is no longer valid, as we there was a SET_PART in between
        // 2) The underlying editeng will only talk about the first match till
        // it doesn't support multi-selection.
        std::vector<OString> aRectangles;
        for (const sd::SearchSelection& rSelection : aSelections)
        {
            if (rSelection.m_nPage == mnStartPageIndex)
                aRectangles.push_back(rSelection.m_aRectangles);
        }
        OString sRectangles = comphelper::string::join("; ", aRectangles);
        rSfxViewShell.libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, sRectangles.getStr());
    }

    mnStartPageIndex = sal_uInt16(-1);

    return bRet;
}

bool SdOutliner::SearchAndReplaceOnce(std::vector<sd::SearchSelection>* pSelections)
{
    DetectChange ();

    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());

    if (!pOutlinerView || !GetEditEngine().HasView(&pOutlinerView->GetEditView()))
    {
        mpImpl->ProvideOutlinerView(*this, pViewShell, mpWindow);
        pOutlinerView = mpImpl->GetOutlinerView();
    }

    if (pViewShell != nullptr)
    {
        mpView = pViewShell->GetView();
        mpWindow = pViewShell->GetActiveWindow();
        pOutlinerView->SetWindow(mpWindow);

        if( nullptr != dynamic_cast< const sd::DrawViewShell *>( pViewShell.get() ))
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
                        pOutlinerView->StartSearchAndReplace(*mpSearchItem);
                }
            }
        }
        else if( nullptr != dynamic_cast< const sd::OutlineViewShell *>( pViewShell.get() ))
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
        std::vector<::tools::Rectangle> aLogicRects;
        pOutlinerView->GetSelectionRectangles(aLogicRects);

        // convert to twips if in 100thmm (seems as if LibreOfficeKit is based on twips?). Do this
        // here where we have the only place needing this, *not* in ImpEditView::GetSelectionRectangles
        // which makes that method unusable for others
        if (pOutlinerView->GetWindow() && MapUnit::Map100thMM == pOutlinerView->GetWindow()->GetMapMode().GetMapUnit())
        {
            for (tools::Rectangle& rRectangle : aLogicRects)
            {
                rRectangle = OutputDevice::LogicToLogic(rRectangle, MapMode(MapUnit::Map100thMM), MapMode(MapUnit::MapTwip));
            }
        }

        std::vector<OString> aLogicRectStrings;
        std::transform(aLogicRects.begin(), aLogicRects.end(), std::back_inserter(aLogicRectStrings), [](const ::tools::Rectangle& rRectangle) { return rRectangle.toString(); });
        OString sRectangles = comphelper::string::join("; ", aLogicRectStrings);

        if (!pSelections)
        {
            // notify LibreOfficeKit about changed page
            OString aPayload = OString::number(maCurrentPosition.mnPageIndex);
            SfxViewShell& rSfxViewShell = pViewShell->GetViewShellBase();
            rSfxViewShell.libreOfficeKitViewCallback(LOK_CALLBACK_SET_PART, aPayload.getStr());

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
            rSfxViewShell.libreOfficeKitViewCallback(LOK_CALLBACK_SEARCH_RESULT_SELECTION, aPayload.getStr());
        }
        else
        {
            sd::SearchSelection aSelection(maCurrentPosition.mnPageIndex, sRectangles);
            bool bDuplicate = !pSelections->empty() && pSelections->back() == aSelection;
            if (!bDuplicate)
                pSelections->push_back(aSelection);
        }
    }

    return mbEndOfSearch;
}

/** Try to detect whether the document or the view (shell) has changed since
    the last time <member>StartSearchAndReplace()</member> has been called.
*/
void SdOutliner::DetectChange()
{
    sd::outliner::IteratorPosition aPosition (maCurrentPosition);

    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
    std::shared_ptr<sd::DrawViewShell> pDrawViewShell (
        std::dynamic_pointer_cast<sd::DrawViewShell>(pViewShell));

    // Detect whether the view has been switched from the outside.
    if (pDrawViewShell != nullptr
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
            pOutlinerView->SetOutputArea( ::tools::Rectangle( Point(), Size(1, 1) ) );
        if (meMode == SPELL)
            SetPaperSize( Size(1, 1) );
        SetText(OUString(), GetParagraph(0));

        RememberStartPosition ();

        mnPageCount = mpDrawDocument->GetSdPageCount(pDrawViewShell->GetPageKind());
        maObjectIterator = sd::outliner::OutlinerContainer(this).current();
    }

    // Detect change of the set of selected objects.  If their number has
    // changed start again with the first selected object.
    else if (DetectSelectionChange())
    {
        HandleChangedSelection ();
        maObjectIterator = sd::outliner::OutlinerContainer(this).current();
    }

    // Detect change of page count.  Restart search at first/last page in
    // that case.
    else if (aPosition.meEditMode == EditMode::Page
        && mpDrawDocument->GetSdPageCount(aPosition.mePageKind) != mnPageCount)
    {
        // The number of pages has changed.
        mnPageCount = mpDrawDocument->GetSdPageCount(aPosition.mePageKind);
        maObjectIterator = sd::outliner::OutlinerContainer(this).current();
    }
    else if (aPosition.meEditMode == EditMode::MasterPage
        && mpDrawDocument->GetSdPageCount(aPosition.mePageKind) != mnPageCount)
    {
        // The number of master pages has changed.
        mnPageCount = mpDrawDocument->GetSdPageCount(aPosition.mePageKind);
        maObjectIterator = sd::outliner::OutlinerContainer(this).current();
    }
}

bool SdOutliner::DetectSelectionChange()
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

void SdOutliner::RememberStartPosition()
{
    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
    if ( ! pViewShell)
    {
        OSL_ASSERT(pViewShell);
        return;
    }

    if ( mnStartPageIndex != sal_uInt16(-1) )
        return;

    if( nullptr != dynamic_cast< const sd::DrawViewShell *>( pViewShell.get() ))
    {
        std::shared_ptr<sd::DrawViewShell> pDrawViewShell (
            std::dynamic_pointer_cast<sd::DrawViewShell>(pViewShell));
        if (pDrawViewShell != nullptr)
        {
            meStartViewMode = pDrawViewShell->GetPageKind();
            meStartEditMode = pDrawViewShell->GetEditMode();
            mnStartPageIndex = pDrawViewShell->GetCurPagePos();
        }

        if (mpView != nullptr)
        {
            mpStartEditedObject = mpView->GetTextEditObject();
            if (mpStartEditedObject != nullptr)
            {
                // Try to retrieve current caret position only when there is an
                // edited object.
                ::Outliner* pOutliner =
                    static_cast<sd::DrawView*>(mpView)->GetTextEditOutliner();
                if (pOutliner!=nullptr && pOutliner->GetViewCount()>0)
                {
                    OutlinerView* pOutlinerView = pOutliner->GetView(0);
                    maStartSelection = pOutlinerView->GetSelection();
                }
            }
        }
    }
    else if( nullptr != dynamic_cast< const sd::OutlineViewShell *>( pViewShell.get() ))
    {
        // Remember the current cursor position.
        OutlinerView* pView = GetView(0);
        if (pView != nullptr)
            pView->GetSelection();
    }
    else
    {
        mnStartPageIndex = sal_uInt16(-1);
    }
}

void SdOutliner::RestoreStartPosition()
{
    bool bRestore = true;
    // Take a negative start page index as inidicator that restoring the
    // start position is not requested.
    if (mnStartPageIndex == sal_uInt16(-1) )
        bRestore = false;
    // Don't restore when the view shell is not valid.
    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
    if (pViewShell == nullptr)
        bRestore = false;

    if (bRestore)
    {
        if( nullptr != dynamic_cast< const sd::DrawViewShell *>( pViewShell.get() ))
        {
            std::shared_ptr<sd::DrawViewShell> pDrawViewShell (
                std::dynamic_pointer_cast<sd::DrawViewShell>(pViewShell));
            SetViewMode (meStartViewMode);
            if (pDrawViewShell != nullptr)
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
        else if( nullptr != dynamic_cast< const sd::OutlineViewShell *>( pViewShell.get() ))
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
void SdOutliner::ProvideNextTextObject()
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
        DBG_UNHANDLED_EXCEPTION("sd.view");
    }
    SetUpdateMode(false);
    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
    if (pOutlinerView != nullptr)
        pOutlinerView->SetOutputArea( ::tools::Rectangle( Point(), Size(1, 1) ) );
    if (meMode == SPELL)
        SetPaperSize( Size(1, 1) );
    SetText(OUString(), GetParagraph(0));

    mpSearchSpellTextObj = nullptr;

    // Iterate until a valid text object has been found or the search ends.
    do
    {
        mpObj = nullptr;
        mpParaObj = nullptr;

        if (maObjectIterator != sd::outliner::OutlinerContainer(this).end())
        {
            maCurrentPosition = *maObjectIterator;

            // LOK: do not descent to notes or master pages when searching
            bool bForbiddenPage = comphelper::LibreOfficeKit::isActive() && (maCurrentPosition.mePageKind != PageKind::Standard || maCurrentPosition.meEditMode != EditMode::Page);

            // Switch to the current object only if it is a valid text object.
            if (!bForbiddenPage && IsValidTextObject(maCurrentPosition))
            {
                // Don't set yet in case of searching: the text object may not match.
                if (meMode != SEARCH)
                    mpObj = SetObject(maCurrentPosition);
                else
                    mpObj = maCurrentPosition.mxObject.get();
            }
            ++maObjectIterator;

            if (mpObj)
            {
                PutTextIntoOutliner();

                std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
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

void SdOutliner::EndOfSearch()
{
    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
    if ( ! pViewShell)
    {
        OSL_ASSERT(pViewShell);
        return;
    }

    // Before we display a dialog we first jump to where the last valid text
    // object was found.  All page and view mode switching since then was
    // temporary and should not be visible to the user.
    if(  nullptr == dynamic_cast< const sd::OutlineViewShell *>( pViewShell.get() ))
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
            maObjectIterator = sd::outliner::OutlinerContainer(this).begin();
            if( nullptr != dynamic_cast< const sd::OutlineViewShell *>( pViewShell.get() ))
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

void SdOutliner::ShowEndOfSearchDialog()
{
    if (meMode == SEARCH)
    {
        if (!mbStringFound)
        {
            SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::NotFound);
            std::shared_ptr<sd::ViewShell> pViewShell(mpWeakViewShell.lock());
            if (pViewShell)
            {
                SfxViewShell& rSfxViewShell = pViewShell->GetViewShellBase();
                rSfxViewShell.libreOfficeKitViewCallback(LOK_CALLBACK_SEARCH_NOT_FOUND, mpSearchItem->GetSearchString().toUtf8().getStr());
            }
        }

        // don't do anything else for search
        return;
    }

    OUString aString;
    if (mpView->AreObjectsMarked())
        aString = SdResId(STR_END_SPELLING_OBJ);
    else
        aString = SdResId(STR_END_SPELLING);

    // Show the message in an info box that is modal with respect to the whole application.
    VclPtr<vcl::Window> xParent(GetMessageBoxParent());
    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(xParent ? xParent->GetFrameWeld() : nullptr,
                                                  VclMessageType::Info, VclButtonsType::Ok, aString));
    xInfoBox->run();
}

bool SdOutliner::ShowWrapArroundDialog()
{
    // Determine whether to show the dialog.
    if (mpSearchItem)
    {
        // When searching display the dialog only for single find&replace.
        const SvxSearchCmd nCommand(mpSearchItem->GetCommand());
        if (nCommand == SvxSearchCmd::REPLACE || nCommand == SvxSearchCmd::FIND)
        {
            if (mbDirectionIsForward)
                SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::End);
            else
                SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::Start);

            return true;
        }
        else
            return false;
    }

    // show dialog only for spelling
    if (meMode != SPELL)
        return false;

    // The question text depends on the search direction.
    bool bImpress = mpDrawDocument && mpDrawDocument->GetDocumentType() == DocumentType::Impress;

    const char* pStringId;
    if (mbDirectionIsForward)
        pStringId = bImpress ? STR_SAR_WRAP_FORWARD : STR_SAR_WRAP_FORWARD_DRAW;
    else
        pStringId = bImpress ? STR_SAR_WRAP_BACKWARD : STR_SAR_WRAP_BACKWARD_DRAW;

    // Pop up question box that asks the user whether to wrap around.
    // The dialog is made modal with respect to the whole application.
    VclPtr<vcl::Window> xParent(GetMessageBoxParent());
    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(xParent ? xParent->GetFrameWeld() : nullptr,
                                                   VclMessageType::Question, VclButtonsType::YesNo, SdResId(pStringId)));
    sal_uInt16 nBoxResult = xQueryBox->run();

    return (nBoxResult == RET_YES);
}

bool SdOutliner::IsValidTextObject (const sd::outliner::IteratorPosition& rPosition)
{
    SdrTextObj* pObject = dynamic_cast< SdrTextObj* >( rPosition.mxObject.get() );
    return (pObject != nullptr) && pObject->HasText() && ! pObject->IsEmptyPresObj();
}

void SdOutliner::PutTextIntoOutliner()
{
    mpSearchSpellTextObj = dynamic_cast<SdrTextObj*>( mpObj );
    if ( mpSearchSpellTextObj && mpSearchSpellTextObj->HasText() && !mpSearchSpellTextObj->IsEmptyPresObj() )
    {
        SdrText* pText = mpSearchSpellTextObj->getText( maCurrentPosition.mnText );
        mpParaObj = pText ? pText->GetOutlinerParaObject() : nullptr;

        if (mpParaObj != nullptr)
        {
            SetText(*mpParaObj);

            ClearModifyFlag();
        }
    }
    else
    {
        mpSearchSpellTextObj = nullptr;
    }
}

void SdOutliner::PrepareSpellCheck()
{
    EESpellState eState = HasSpellErrors();
    DBG_ASSERT(eState != EESpellState::NoSpeller, "No SpellChecker");

    if (eState != EESpellState::Ok)
    {
        // When spell checking we have to test whether we have processed the
        // whole document and have reached the start page again.
        if (meMode == SPELL)
        {
            if (maSearchStartPosition == sd::outliner::Iterator())
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

void SdOutliner::PrepareSearchAndReplace()
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

void SdOutliner::SetViewMode (PageKind ePageKind)
{
    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
    std::shared_ptr<sd::DrawViewShell> pDrawViewShell(
        std::dynamic_pointer_cast<sd::DrawViewShell>(pViewShell));
    if (pDrawViewShell != nullptr && ePageKind != pDrawViewShell->GetPageKind())
    {
        // Restore old edit mode.
        pDrawViewShell->ChangeEditMode(mpImpl->meOriginalEditMode, false);

        SetStatusEventHdl(Link<EditStatus&,void>());
        OUString sViewURL;
        switch (ePageKind)
        {
            case PageKind::Standard:
            default:
                sViewURL = sd::framework::FrameworkHelper::msImpressViewURL;
                break;
            case PageKind::Notes:
                sViewURL = sd::framework::FrameworkHelper::msNotesViewURL;
                break;
            case PageKind::Handout:
                sViewURL = sd::framework::FrameworkHelper::msHandoutViewURL;
                break;
        }
        // The text object iterator is destroyed when the shells are
        // switched but we need it so save it and restore it afterwards.
        sd::outliner::Iterator aIterator (maObjectIterator);
        bool bMatchMayExist = mbMatchMayExist;

        sd::ViewShellBase& rBase = pViewShell->GetViewShellBase();
        SetViewShell(std::shared_ptr<sd::ViewShell>());
        sd::framework::FrameworkHelper::Instance(rBase)->RequestView(
            sViewURL,
            sd::framework::FrameworkHelper::msCenterPaneURL);

        // Force (well, request) a synchronous update of the configuration.
        // In a better world we would handle the asynchronous view update
        // instead.  But that would involve major restructuring of the
        // Outliner code.
        sd::framework::FrameworkHelper::Instance(rBase)->RequestSynchronousUpdate();
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
        pDrawViewShell = std::dynamic_pointer_cast<sd::DrawViewShell>(pViewShell);
        OSL_ASSERT(pDrawViewShell != nullptr);
        if (pDrawViewShell != nullptr)
            mpImpl->meOriginalEditMode = pDrawViewShell->GetEditMode();
    }
}

void SdOutliner::SetPage (EditMode eEditMode, sal_uInt16 nPageIndex)
{
    if ( ! mbRestrictSearchToSelection)
    {
        std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
        std::shared_ptr<sd::DrawViewShell> pDrawViewShell(
            std::dynamic_pointer_cast<sd::DrawViewShell>(pViewShell));
        OSL_ASSERT(pDrawViewShell != nullptr);
        if (pDrawViewShell != nullptr)
        {
            pDrawViewShell->ChangeEditMode(eEditMode, false);
            pDrawViewShell->SwitchPage(nPageIndex);
        }
    }
}

void SdOutliner::EnterEditMode (bool bGrabFocus)
{
    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
    if (pOutlinerView && mpSearchSpellTextObj)
    {
        pOutlinerView->SetOutputArea( ::tools::Rectangle( Point(), Size(1, 1)));
        SetPaperSize( mpSearchSpellTextObj->GetLogicRect().GetSize() );
        SdrPageView* pPV = mpView->GetSdrPageView();

        // Make FuText the current function.
        SfxUInt16Item aItem (SID_TEXTEDIT, 1);
        std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
        if (pViewShell && pViewShell->GetDispatcher())
        {
            pViewShell->GetDispatcher()->ExecuteList(
                SID_TEXTEDIT, SfxCallMode::SYNCHRON | SfxCallMode::RECORD, { &aItem });

            // To be consistent with the usual behaviour in the Office the text
            // object that is put into edit mode would have also to be selected.
            // Starting the text edit mode is not enough so we do it here by
            // hand.
            mpView->UnmarkAllObj (pPV);
            mpView->MarkObj (mpSearchSpellTextObj, pPV);

            mpSearchSpellTextObj->setActiveText( mnText );

            // Make sure we aren't editing already.
            mpView->SdrEndTextEdit();

            // Turn on the edit mode for the text object.
            mpView->SdrBeginTextEdit(mpSearchSpellTextObj, pPV, mpWindow, true, this, pOutlinerView, true, true, bGrabFocus);

            SetUpdateMode(true);
            mbFoundObject = true;
        }
    }
}

ESelection SdOutliner::GetSearchStartPosition()
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

bool SdOutliner::HasNoPreviousMatch()
{
    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();

    DBG_ASSERT (pOutlinerView!=nullptr, "outline view in SdOutliner::HasNoPreviousMatch is NULL");

    // Detect whether the cursor stands at the beginning
    // resp. at the end of the text.
    return pOutlinerView->GetSelection() == GetSearchStartPosition();
}

bool SdOutliner::HandleFailedSearch()
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
            SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::NotFound);
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

SdrObject* SdOutliner::SetObject (
    const sd::outliner::IteratorPosition& rPosition)
{
    SetViewMode (rPosition.mePageKind);
    SetPage (rPosition.meEditMode, static_cast<sal_uInt16>(rPosition.mnPageIndex));
    mnText = rPosition.mnText;
    return rPosition.mxObject.get();
}

void SdOutliner::SetViewShell (const std::shared_ptr<sd::ViewShell>& rpViewShell)
{
    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
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

void SdOutliner::HandleChangedSelection()
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
                maMarkListCopy.emplace_back(rMarkList.GetMark(i)->GetMarkedSdrObj ());
        }
        else
            // No marked object.  Is this case possible?
            mbRestrictSearchToSelection = false;
    }
}

void SdOutliner::StartConversion( LanguageType nSourceLanguage,  LanguageType nTargetLanguage,
        const vcl::Font *pTargetFont, sal_Int32 nOptions, bool bIsInteractive )
{
    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
    bool bMultiDoc = nullptr != dynamic_cast< const sd::DrawViewShell *>( pViewShell.get() );

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
void SdOutliner::PrepareConversion()
{
    SetUpdateMode(true);
    if( HasConvertibleTextPortion( mnConversionLanguage ) )
    {
        SetUpdateMode(false);
        mbStringFound = true;
        mbMatchMayExist = true;

        EnterEditMode(true);

        mpDrawDocument->GetDocSh()->SetWaitCursor( false );
        // Start search at the right end of the current object's text
        // depending on the search direction.
    }
    else
    {
        SetUpdateMode(false);
    }
}

void SdOutliner::BeginConversion()
{
    SetRefDevice( SD_MOD()->GetVirtualRefDevice() );

    sd::ViewShellBase* pBase = dynamic_cast<sd::ViewShellBase*>( SfxViewShell::Current() );
    if (pBase != nullptr)
        SetViewShell (pBase->GetMainViewShell());

    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
    if (pViewShell)
    {
        mbStringFound = false;

        // Supposed that we are not located at the very beginning/end of the
        // document then there may be a match in the document prior/after
        // the current position.
        mbMatchMayExist = true;

        maObjectIterator = sd::outliner::Iterator();
        maSearchStartPosition = sd::outliner::Iterator();
        RememberStartPosition();

        mpImpl->ProvideOutlinerView(*this, pViewShell, mpWindow);

        HandleChangedSelection ();
    }
    ClearModifyFlag();
}

void SdOutliner::EndConversion()
{
    EndSpelling();
}

bool SdOutliner::ConvertNextDocument()
{
    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
    if (dynamic_cast< const sd::OutlineViewShell *>( pViewShell.get() ) )
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

VclPtr<vcl::Window> SdOutliner::GetMessageBoxParent()
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
                sd::SpellDialogChildWindow::GetChildWindowId());
            break;

        case TEXT_CONVERSION:
            // There should no messages boxes be displayed while doing the
            // hangul hanja conversion.
            break;
    }

    if (pChildWindow != nullptr)
        pSearchDialog = pChildWindow->GetWindow();

    if (pSearchDialog)
        return pSearchDialog;

    std::shared_ptr<sd::ViewShell> pViewShell (mpWeakViewShell.lock());
    return pViewShell->GetActiveWindow();
}

//===== SdOutliner::Implementation ==============================================

SdOutliner::Implementation::Implementation()
    : meOriginalEditMode(EditMode::Page),
      mbOwnOutlineView(false),
      mpOutlineView(nullptr)
{
}

SdOutliner::Implementation::~Implementation()
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
void SdOutliner::Implementation::ProvideOutlinerView (
    Outliner& rOutliner,
    const std::shared_ptr<sd::ViewShell>& rpViewShell,
    vcl::Window* pWindow)
{
    if (rpViewShell != nullptr)
    {
        switch (rpViewShell->GetShellType())
        {
            case sd::ViewShell::ST_DRAW:
            case sd::ViewShell::ST_IMPRESS:
            case sd::ViewShell::ST_NOTES:
            case sd::ViewShell::ST_HANDOUT:
            {
                // Create a new outline view to do the search on.
                bool bInsert = false;
                if (mpOutlineView != nullptr && !mbOwnOutlineView)
                    mpOutlineView = nullptr;

                if (mpOutlineView == nullptr || !rOutliner.GetEditEngine().HasView(&mpOutlineView->GetEditView()))
                {
                    delete mpOutlineView;
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
                mpOutlineView->SetOutputArea (::tools::Rectangle (Point(), Size(1, 1)));
                rOutliner.SetPaperSize( Size(1, 1) );
                rOutliner.SetText(OUString(), rOutliner.GetParagraph(0));

                meOriginalEditMode =
                    std::static_pointer_cast<sd::DrawViewShell>(rpViewShell)->GetEditMode();
            }
            break;

            case sd::ViewShell::ST_OUTLINE:
            {
                if (mpOutlineView!=nullptr && mbOwnOutlineView)
                    delete mpOutlineView;
                mpOutlineView = rOutliner.GetView(0);
                mbOwnOutlineView = false;
            }
            break;

            default:
            case sd::ViewShell::ST_NONE:
            case sd::ViewShell::ST_PRESENTATION:
                // Ignored
                break;
        }
    }
}

void SdOutliner::Implementation::ReleaseOutlinerView()
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
