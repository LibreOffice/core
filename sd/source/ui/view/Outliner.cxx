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
#include <vcl/wrkwin.hxx>
#include <svl/srchitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editstat.hxx>
#include <vcl/outdev.hxx>
#include <svx/dlgutil.hxx>
#include <svx/xtable.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/printer.hxx>
#include <svx/svxerr.hxx>
#include <svx/svdotext.hxx>
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

class SfxStyleSheetPool;

namespace sd {

class Outliner::Implementation
{
public:
    /** The original edit mode directly after switching to a different view
        mode.  Used for restoring the edit mode when leaving that view mode
        again.
    */
    EditMode meOriginalEditMode;

    Implementation (void);
    ~Implementation (void);

    /** Return the OutlinerView that was provided by the last call to
        ProvideOutlinerView() (or NULL when there was no such call.)
    */
    OutlinerView* GetOutlinerView (void);

    /** Provide in the member mpOutlineView an instance of OutlinerView that
        is either taken from the ViewShell, when it is an OutlineViewShell,
        or is created.  When an OutlinerView already exists it is initialied.
    */
    void ProvideOutlinerView (
        Outliner& rOutliner,
        const ::boost::shared_ptr<ViewShell>& rpViewShell,
        ::Window* pWindow);

    /** This method is called when the OutlinerView is no longer used.
    */
    void ReleaseOutlinerView (void);

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
      mpView(NULL),
      mpWeakViewShell(),
      mpWindow(NULL),
      mpDrawDocument(pDoc),
      mnConversionLanguage(LANGUAGE_NONE),
      mnIgnoreCurrentPageChangesLevel(0),
      mbStringFound(sal_False),
      mbMatchMayExist(false),
      mnPageCount(0),
      mnObjectCount(0),
      mbEndOfSearch(sal_False),
      mbFoundObject(sal_False),
      mbError(sal_False),
      mbDirectionIsForward(true),
      mbRestrictSearchToSelection(false),
      maMarkListCopy(),
      mbProcessCurrentViewOnly(false),
      mpObj(NULL),
      mpFirstObj(NULL),
      mpTextObj(NULL),
      mnText(0),
      mpParaObj(NULL),
      meStartViewMode(PK_STANDARD),
      meStartEditMode(EM_PAGE),
      mnStartPageIndex((sal_uInt16)-1),
      mpStartEditedObject(NULL),
      maStartSelection(),
      mpSearchItem(NULL),
      maObjectIterator(),
      maCurrentPosition(),
      maSearchStartPosition(),
      maLastValidPosition(),
      mbSelectionHasChanged(false),
      mbExpectingSelectionChangeEvent(false),
      mbWholeDocumentProcessed(false),
      mbPrepareSpellingPending(true)
{
    SetStyleSheetPool((SfxStyleSheetPool*) mpDrawDocument->GetStyleSheetPool());
    SetEditTextObjectPool( &pDoc->GetItemPool() );
    SetCalcFieldValueHdl(LINK(SD_MOD(), SdModule, CalcFieldValueHdl));
    SetForbiddenCharsTable( pDoc->GetForbiddenCharsTable() );

    sal_uLong nCntrl = GetControlWord();
    nCntrl |= EE_CNTRL_ALLOWBIGOBJS;
    nCntrl |= EE_CNTRL_URLSFXEXECUTE;
    nCntrl |= EE_CNTRL_MARKFIELDS;
    nCntrl |= EE_CNTRL_AUTOCORRECT;

    sal_Bool bOnlineSpell = false;

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
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;

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
Outliner::~Outliner (void)
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
void Outliner::PrepareSpelling (void)
{
    mbPrepareSpellingPending = false;

    ViewShellBase* pBase = PTR_CAST(ViewShellBase,SfxViewShell::Current());
    if (pBase != NULL)
        SetViewShell (pBase->GetMainViewShell());
    SetRefDevice( SD_MOD()->GetRefDevice( *mpDrawDocument->GetDocSh() ) );

    ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if (pViewShell)
    {
        mbStringFound = sal_False;

        mbWholeDocumentProcessed = false;
        // Supposed that we are not located at the very beginning/end of
        // the document then there may be a match in the document
        // prior/after the current position.
        mbMatchMayExist = sal_True;

        maObjectIterator = ::sd::outliner::Iterator();
        maSearchStartPosition = ::sd::outliner::Iterator();
        RememberStartPosition();

        mpImpl->ProvideOutlinerView(*this, pViewShell, mpWindow);

        HandleChangedSelection ();
    }
    ClearModifyFlag();
}





void Outliner::StartSpelling (void)
{
    meMode = SPELL;
    mbDirectionIsForward = true;
    mpSearchItem = NULL;
}

/** Proxy for method from base class to avoid compiler warning */
void Outliner::StartSpelling(EditView& rView, unsigned char c)
{
    SdrOutliner::StartSpelling( rView, c );
}

/** Free all resources acquired during the search/spell check.  After a
    spell check the start position is restored here.
*/
void Outliner::EndSpelling (void)
{
    // Keep old view shell alive until we release the outliner view.
    ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    ::boost::shared_ptr<ViewShell> pOldViewShell (pViewShell);

    ViewShellBase* pBase = PTR_CAST(ViewShellBase,SfxViewShell::Current());
    if (pBase != NULL)
        pViewShell = pBase->GetMainViewShell();
    else
        pViewShell.reset();
    mpWeakViewShell = pViewShell;

    // When in <member>PrepareSpelling()</member> a new outline view has
    // been created then delete it here.
    sal_Bool bViewIsDrawViewShell(pViewShell && pViewShell->ISA(DrawViewShell));
    if (bViewIsDrawViewShell)
    {
        SetStatusEventHdl(Link());
        mpView = pViewShell->GetView();
        mpView->UnmarkAllObj (mpView->GetSdrPageView());
        mpView->SdrEndTextEdit();
        // Make FuSelection the current function.
        pViewShell->GetDispatcher()->Execute(
            SID_OBJECT_SELECT,
            SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);

        // Remove and, if previously created by us, delete the outline
        // view.
        OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
        if (pOutlinerView != NULL)
        {
            RemoveView(pOutlinerView);
            mpImpl->ReleaseOutlinerView();
        }

        SetUpdateMode(sal_True);
    }

    // Before clearing the modify flag use it as a hint that
    // changes were done at SpellCheck
    if(IsModified())
    {
        if(mpView && mpView->ISA(OutlineView))
            static_cast<OutlineView*>(mpView)->PrepareClose(sal_False);
        if(mpDrawDocument && !mpDrawDocument->IsChanged())
            mpDrawDocument->SetChanged(sal_True);
    }

    // Now clear the modify flag to have a specified state of
    // Outliner
    ClearModifyFlag();

    // When spell checking then restore the start position.
    if (meMode==SPELL || meMode==TEXT_CONVERSION)
        RestoreStartPosition ();

    mpWeakViewShell.reset();
    mpView = NULL;
    mpWindow = NULL;
    mnStartPageIndex = (sal_uInt16) -1;
}




sal_Bool Outliner::SpellNextDocument (void)
{
    ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if (pViewShell->ISA(OutlineViewShell))
    {
        // When doing a spell check in the outline view then there is
        // only one document.
        mbEndOfSearch = true;
        EndOfSearch ();
    }
    else
    {
        if (mpView->ISA(OutlineView))
            ((OutlineView*)mpView)->PrepareClose(sal_False);
        mpDrawDocument->GetDocSh()->SetWaitCursor( sal_True );

        Initialize (true);

        mpWindow = pViewShell->GetActiveWindow();
        OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
        if (pOutlinerView != NULL)
            pOutlinerView->SetWindow(mpWindow);
        ProvideNextTextObject ();

        mpDrawDocument->GetDocSh()->SetWaitCursor( sal_False );
        ClearModifyFlag();
    }

    return mbEndOfSearch ? sal_False : sal_True;

}


/**
 * check next text object
 */
::svx::SpellPortions Outliner::GetNextSpellSentence (void)
{
    ::svx::SpellPortions aResult;

    DetectChange();
    // Iterate over sentences and text shapes until a sentence with a
    // spelling error has been found.  If no such sentence can be
    // found the loop is left through a break.
    // It is the responsibility of the sd outliner object to correctly
    // iterate over all text shapes, i.e. switch between views, wrap
    // arround at the end of the document, stop when all text shapes
    // have been examined exactly once.
    bool bFoundNextSentence = false;
    while ( ! bFoundNextSentence)
    {
        OutlinerView* pOutlinerView = GetView(0);
        if (pOutlinerView != NULL)
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
    sal_Bool bEndOfSearch = sal_True;

    mpDrawDocument->GetDocSh()->SetWaitCursor( sal_True );
    if (mbPrepareSpellingPending)
        PrepareSpelling();
    ViewShellBase* pBase = PTR_CAST(ViewShellBase,SfxViewShell::Current());
    // Determine whether we have to abort the search.  This is necessary
    // when the main view shell does not support searching.
    bool bAbort = false;
    if (pBase != NULL)
    {
        ::boost::shared_ptr<ViewShell> pShell (pBase->GetMainViewShell());
        SetViewShell(pShell);
        if (pShell.get() == NULL)
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

    ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if ( ! pViewShell)
    {
        OSL_ASSERT(pViewShell);
        return true;
    }

    if ( ! bAbort)
    {
        meMode = SEARCH;
        mpSearchItem = pSearchItem;

        mbFoundObject = sal_False;

        Initialize ( ! mpSearchItem->GetBackward());

        const sal_uInt16 nCommand (mpSearchItem->GetCommand());
        if (nCommand == SVX_SEARCHCMD_REPLACE_ALL)
            bEndOfSearch = SearchAndReplaceAll ();
        else
        {
            RememberStartPosition ();
            bEndOfSearch = SearchAndReplaceOnce ();
            // restore start position if nothing was found
            if(!mbStringFound)
                RestoreStartPosition ();
            mnStartPageIndex = (sal_uInt16)-1;
        }
    }
    else
        mpDrawDocument->GetDocSh()->SetWaitCursor( sal_False );

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

        ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
        if ( ! pViewShell)
        {
            OSL_ASSERT(pViewShell);
            return;
        }

        // In case we are searching in an outline view then first remove the
        // current selection and place cursor at its start or end.
        if (pViewShell->ISA(OutlineViewShell))
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
        // Requested iteration direction has changed.  Turn arround the iterator.
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




bool Outliner::SearchAndReplaceAll (void)
{
    // Save the current position to be restored after having replaced all
    // matches.
    RememberStartPosition ();

    ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if ( ! pViewShell)
    {
        OSL_ASSERT(pViewShell);
        return true;
    }

    if (pViewShell->ISA(OutlineViewShell))
    {
        // Put the cursor to the beginning/end of the outliner.
        mpImpl->GetOutlinerView()->SetSelection (GetSearchStartPosition ());

        // The outliner does all the work for us when we are in this mode.
        SearchAndReplaceOnce();
    }
    else if (pViewShell->ISA(DrawViewShell))
    {
        // Go to beginning/end of document.
        maObjectIterator = ::sd::outliner::OutlinerContainer(this).begin();
        // Switch to the current object only if it is a valid text object.
        ::sd::outliner::IteratorPosition aNewPosition (*maObjectIterator);
        if (IsValidTextObject (aNewPosition))
        {
            maCurrentPosition = aNewPosition;
            SetObject (maCurrentPosition);
        }

        // Search/replace until the end of the document is reached.
        bool bFoundMatch;
        do
        {
            bFoundMatch = ! SearchAndReplaceOnce();
        }
        while (bFoundMatch);
    }

    RestoreStartPosition ();
    mnStartPageIndex = (sal_uInt16)-1;

    return true;
}




bool Outliner::SearchAndReplaceOnce (void)
{
    DetectChange ();

    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
    DBG_ASSERT(pOutlinerView!=NULL && GetEditEngine().HasView( &pOutlinerView->GetEditView() ),
        "SearchAndReplace without valid view!" );

    if( NULL == pOutlinerView || !GetEditEngine().HasView( &pOutlinerView->GetEditView() ) )
        return true;

    ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if (pViewShell != 0)
    {
        mpView = pViewShell->GetView();
        mpWindow = pViewShell->GetActiveWindow();
        pOutlinerView->SetWindow(mpWindow);

        if (pViewShell->ISA(DrawViewShell) )
        {
            // When replacing we first check if there is a selection
            // indicating a match.  If there is then replace it.  The
            // following call to StartSearchAndReplace will then search for
            // the next match.
            if (meMode == SEARCH
                && mpSearchItem->GetCommand() == SVX_SEARCHCMD_REPLACE)
                if (pOutlinerView->GetSelection().HasRange())
                    pOutlinerView->StartSearchAndReplace(*mpSearchItem);

            // Search for the next match.
            sal_uLong nMatchCount = 0;
            if (mpSearchItem->GetCommand() != SVX_SEARCHCMD_REPLACE_ALL)
                nMatchCount = pOutlinerView->StartSearchAndReplace(*mpSearchItem);

            // Go to the next text object when there have been no matches in
            // the current object or the whole object has already been
            // processed.
            if (nMatchCount==0 || mpSearchItem->GetCommand()==SVX_SEARCHCMD_REPLACE_ALL)
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
                        mpDrawDocument->GetDocSh()->SetWaitCursor( sal_False );
                        return true;
                    }

                    if (meMode == SEARCH)
                        nMatchCount = pOutlinerView->StartSearchAndReplace(*mpSearchItem);
                }
            }
        }
        else if (pViewShell->ISA(OutlineViewShell))
        {
            mpDrawDocument->GetDocSh()->SetWaitCursor (sal_False);
            // The following loop is executed more then once only when a
            // wrap arround search is done.
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

    mpDrawDocument->GetDocSh()->SetWaitCursor( sal_False );

    return mbEndOfSearch;
}




/** Try to detect whether the document or the view (shell) has changed since
    the last time <member>StartSearchAndReplace()</member> has been called.
*/
void Outliner::DetectChange (void)
{
    ::sd::outliner::IteratorPosition aPosition (maCurrentPosition);

    ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    ::boost::shared_ptr<DrawViewShell> pDrawViewShell (
        ::boost::dynamic_pointer_cast<DrawViewShell>(pViewShell));

    // Detect whether the view has been switched from the outside.
    if (pDrawViewShell.get() != NULL
        && (aPosition.meEditMode != pDrawViewShell->GetEditMode()
            || aPosition.mePageKind != pDrawViewShell->GetPageKind()))
    {
        // Either the edit mode or the page kind has changed.
        SetStatusEventHdl(Link());

        SdrPageView* pPageView = mpView->GetSdrPageView();
        if (pPageView != NULL)
            mpView->UnmarkAllObj (pPageView);
        mpView->SdrEndTextEdit();
        SetUpdateMode(sal_False);
        OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
        if (pOutlinerView != NULL)
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




bool Outliner::DetectSelectionChange (void)
{
    bool bSelectionHasChanged = false;
    sal_uLong nMarkCount = mpView->GetMarkedObjectList().GetMarkCount();

    // If mpObj is NULL then we have not yet found our first match.
    // Detecting a change makes no sense.
    if (mpObj != NULL)
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
                if (mpView != NULL)
                {
                    SdrMark* pMark = mpView->GetMarkedObjectList().GetMark(0);
                    if (pMark != NULL)
                        bSelectionHasChanged = (mpObj != pMark->GetMarkedSdrObj ());
                }
                break;
            default:
                // We had selected exactly one object.
                bSelectionHasChanged = true;
                break;
        }

    return bSelectionHasChanged;
}




void Outliner::RememberStartPosition (void)
{
    ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if ( ! pViewShell)
    {
        OSL_ASSERT(pViewShell);
        return;
    }

    if ( mnStartPageIndex != (sal_uInt16) -1 )
        return;

    if (pViewShell->ISA(DrawViewShell))
    {
        ::boost::shared_ptr<DrawViewShell> pDrawViewShell (
            ::boost::dynamic_pointer_cast<DrawViewShell>(pViewShell));
        if (pDrawViewShell.get() != NULL)
        {
            meStartViewMode = pDrawViewShell->GetPageKind();
            meStartEditMode = pDrawViewShell->GetEditMode();
            mnStartPageIndex = pDrawViewShell->GetCurPageId() - 1;
        }

        if (mpView != NULL)
        {
            mpStartEditedObject = mpView->GetTextEditObject();
            if (mpStartEditedObject != NULL)
            {
                // Try to retrieve current caret position only when there is an
                // edited object.
                ::Outliner* pOutliner =
                    static_cast<DrawView*>(mpView)->GetTextEditOutliner();
                if (pOutliner!=NULL && pOutliner->GetViewCount()>0)
                {
                    OutlinerView* pOutlinerView = pOutliner->GetView(0);
                    maStartSelection = pOutlinerView->GetSelection();
                }
            }
        }
    }
    else if (pViewShell->ISA(OutlineViewShell))
    {
        // Remember the current cursor position.
        OutlinerView* pView = GetView(0);
        if (pView != NULL)
            pView->GetSelection();
    }
    else
    {
        mnStartPageIndex = (sal_uInt16)-1;
    }
}




void Outliner::RestoreStartPosition (void)
{
    bool bRestore = true;
    // Take a negative start page index as inidicator that restoring the
    // start position is not requested.
    if (mnStartPageIndex == (sal_uInt16)-1 )
        bRestore = false;
    // Dont't restore when the view shell is not valid.
    ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if (pViewShell == 0)
        bRestore = false;

    if (bRestore)
    {
        if (pViewShell->ISA(DrawViewShell))
        {
            ::boost::shared_ptr<DrawViewShell> pDrawViewShell (
                ::boost::dynamic_pointer_cast<DrawViewShell>(pViewShell));
            SetViewMode (meStartViewMode);
            if (pDrawViewShell.get() != NULL)
                SetPage (meStartEditMode, mnStartPageIndex);


            if (mpStartEditedObject != NULL)
            {
                // Turn on the text toolbar as it is done in FuText so that
                // undo manager setting/restoring in
                // sd::View::{Beg,End}TextEdit() works on the same view shell.
                pViewShell->GetViewShellBase().GetToolBarManager()->SetToolBarShell(
                    ToolBarManager::TBG_FUNCTION,
                    RID_DRAW_TEXT_TOOLBOX);

                mpView->SdrBeginTextEdit(mpStartEditedObject);
                ::Outliner* pOutliner =
                      static_cast<DrawView*>(mpView)->GetTextEditOutliner();
                if (pOutliner!=NULL && pOutliner->GetViewCount()>0)
                {
                    OutlinerView* pOutlinerView = pOutliner->GetView(0);
                    pOutlinerView->SetSelection(maStartSelection);
                }
            }
        }
        else if (pViewShell->ISA(OutlineViewShell))
        {
            // Set cursor to its old position.
            OutlinerView* pView = GetView(0);
            if (pView != NULL)
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
void Outliner::ProvideNextTextObject (void)
{
    mbEndOfSearch = false;
    mbFoundObject = false;

    mpView->UnmarkAllObj (mpView->GetSdrPageView());
    try
    {
        mpView->SdrEndTextEdit();
    }
    catch (const ::com::sun::star::uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    SetUpdateMode(sal_False);
    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
    if (pOutlinerView != NULL)
        pOutlinerView->SetOutputArea( Rectangle( Point(), Size(1, 1) ) );
    if (meMode == SPELL)
        SetPaperSize( Size(1, 1) );
    SetText(OUString(), GetParagraph(0));

    mpTextObj = NULL;

    // Iterate until a valid text object has been found or the search ends.
    do
    {
        mpObj = NULL;
        mpParaObj = NULL;

        if (maObjectIterator != ::sd::outliner::OutlinerContainer(this).end())
        {
            maCurrentPosition = *maObjectIterator;
            // Switch to the current object only if it is a valid text object.
            if (IsValidTextObject (maCurrentPosition))
            {
                mpObj = SetObject (maCurrentPosition);
            }
            ++maObjectIterator;

            if (mpObj != NULL)
            {
                PutTextIntoOutliner ();

                ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
                if (pViewShell != 0)
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
            mbEndOfSearch = true;
            EndOfSearch ();
        }
    }
    while ( ! (mbFoundObject || mbEndOfSearch));
}




void Outliner::EndOfSearch (void)
{
    ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if ( ! pViewShell)
    {
        OSL_ASSERT(pViewShell);
        return;
    }

    // Before we display a dialog we first jump to where the last valid text
    // object was found.  All page and view mode switching since then was
    // temporary and should not be visible to the user.
    if ( ! pViewShell->ISA(OutlineViewShell))
        SetObject (maLastValidPosition);

    if (mbRestrictSearchToSelection)
        ShowEndOfSearchDialog ();
    else
    {
        // When no match has been found so far then terminate the search.
        if ( ! mbMatchMayExist)
        {
            ShowEndOfSearchDialog ();
            mbEndOfSearch = sal_True;
        }
        // Ask the user whether to wrap arround and continue the search or
        // to terminate.
        else if (meMode==TEXT_CONVERSION || ShowWrapArroundDialog ())
        {
            mbMatchMayExist = false;
            // Everything back to beginning (or end?) of the document.
            maObjectIterator = ::sd::outliner::OutlinerContainer(this).begin();
            if (pViewShell->ISA(OutlineViewShell))
            {
                // Set cursor to first character of the document.
                OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
                if (pOutlinerView != NULL)
                    pOutlinerView->SetSelection (GetSearchStartPosition ());
            }

            mbEndOfSearch = false;
        }
        else
        {
            // No wrap arround.
            mbEndOfSearch = true;
        }
    }
}

void Outliner::ShowEndOfSearchDialog (void)
{
    OUString aString;
    if (meMode == SEARCH)
    {
        if (mbStringFound)
            aString = SD_RESSTR(STR_END_SEARCHING);
        else
            aString = SD_RESSTR(STR_STRING_NOTFOUND);
    }
    else
    {
        if (mpView->AreObjectsMarked())
            aString = SD_RESSTR(STR_END_SPELLING_OBJ);
        else
            aString = SD_RESSTR(STR_END_SPELLING);
    }

    // Show the message in an info box that is modal with respect to the
    // whole application.
    InfoBox aInfoBox (NULL, aString);
    ShowModalMessageBox (aInfoBox);

    mbWholeDocumentProcessed = true;
}




bool Outliner::ShowWrapArroundDialog (void)
{
    bool bDoWrapArround = false;

    // Determine whether to show the dialog.
    bool bShowDialog = false;
    if (mpSearchItem != NULL)
    {
        // When searching display the dialog only for single find&replace.
        const sal_uInt16 nCommand (mpSearchItem->GetCommand());
        bShowDialog = (nCommand==SVX_SEARCHCMD_REPLACE)
            || (nCommand==SVX_SEARCHCMD_FIND);
    }
    else
        // Spell checking needs the dialog, too.
        bShowDialog = (meMode == SPELL);

    if (bShowDialog)
    {
        // The question text depends on the search direction.
        sal_Bool bImpress = mpDrawDocument!=NULL
            && mpDrawDocument->GetDocumentType() == DOCUMENT_TYPE_IMPRESS;
        sal_uInt16 nStringId;
        if (mbDirectionIsForward)
            nStringId = bImpress
                ? STR_SAR_WRAP_FORWARD
                : STR_SAR_WRAP_FORWARD_DRAW;
        else
            nStringId = bImpress
                ? STR_SAR_WRAP_BACKWARD
                : STR_SAR_WRAP_BACKWARD_DRAW;

        // Pop up question box that asks the user whether to wrap arround.
        // The dialog is made modal with respect to the whole application.
        QueryBox aQuestionBox (
            NULL,
            WB_YES_NO | WB_DEF_YES,
            SD_RESSTR(nStringId));
        aQuestionBox.SetImage (QueryBox::GetStandardImage());
        sal_uInt16 nBoxResult = ShowModalMessageBox(aQuestionBox);
        bDoWrapArround = (nBoxResult == RET_YES);
    }

    return bDoWrapArround;
}




bool Outliner::IsValidTextObject (const ::sd::outliner::IteratorPosition& rPosition)
{
    SdrTextObj* pObject = dynamic_cast< SdrTextObj* >( rPosition.mxObject.get() );
    return (pObject != NULL) && pObject->HasText() && ! pObject->IsEmptyPresObj();
}




void Outliner::PutTextIntoOutliner()
{
    mpTextObj = dynamic_cast<SdrTextObj*>( mpObj );
    if ( mpTextObj && mpTextObj->HasText() && !mpTextObj->IsEmptyPresObj() )
    {
        SdrText* pText = mpTextObj->getText( mnText );
        mpParaObj = pText ? pText->GetOutlinerParaObject() : NULL;

        if (mpParaObj != NULL)
        {
            SetText(*mpParaObj);

            ClearModifyFlag();
        }
    }
    else
    {
        mpTextObj = NULL;
    }
}




void Outliner::PrepareSpellCheck (void)
{
    EESpellState eState = HasSpellErrors();
    DBG_ASSERT(eState != EE_SPELL_NOSPELLER, "No SpellChecker");

    if (eState == EE_SPELL_NOLANGUAGE)
    {
        mbError = sal_True;
        mbEndOfSearch = sal_True;
        ErrorBox aErrorBox (NULL,
            WB_OK,
            SD_RESSTR(STR_NOLANGUAGE));
        ShowModalMessageBox (aErrorBox);
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

        EnterEditMode( sal_False );
    }
}




void Outliner::PrepareSearchAndReplace (void)
{
    if (HasText( *mpSearchItem ))
    {
        mbStringFound = true;
        mbMatchMayExist = true;

        EnterEditMode ();

        mpDrawDocument->GetDocSh()->SetWaitCursor( sal_False );
        // Start seach at the right end of the current object's text
        // depending on the search direction.
        OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
        if (pOutlinerView != NULL)
            pOutlinerView->SetSelection (GetSearchStartPosition ());
    }
}




void Outliner::SetViewMode (PageKind ePageKind)
{
    ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    ::boost::shared_ptr<DrawViewShell> pDrawViewShell(
        ::boost::dynamic_pointer_cast<DrawViewShell>(pViewShell));
    if (pDrawViewShell.get()!=NULL && ePageKind != pDrawViewShell->GetPageKind())
    {
        // Restore old edit mode.
        pDrawViewShell->ChangeEditMode(mpImpl->meOriginalEditMode, sal_False);

        SetStatusEventHdl(Link());
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
        SetViewShell(::boost::shared_ptr<ViewShell>());
        framework::FrameworkHelper::Instance(rBase)->RequestView(
            sViewURL,
            framework::FrameworkHelper::msCenterPaneURL);

        // Force (well, request) a synchronous update of the configuration.
        // In a better world we would handle the asynchronous view update
        // instead.  But that would involve major restucturing of the
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
        pDrawViewShell = ::boost::dynamic_pointer_cast<DrawViewShell>(pViewShell);
        OSL_ASSERT(pDrawViewShell.get()!=NULL);
        if (pDrawViewShell.get() != NULL)
            mpImpl->meOriginalEditMode = pDrawViewShell->GetEditMode();
    }
}




void Outliner::SetPage (EditMode eEditMode, sal_uInt16 nPageIndex)
{
    if ( ! mbRestrictSearchToSelection)
    {
        ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
        ::boost::shared_ptr<DrawViewShell> pDrawViewShell(
            ::boost::dynamic_pointer_cast<DrawViewShell>(pViewShell));
        OSL_ASSERT(pDrawViewShell.get()!=NULL);
        if (pDrawViewShell.get() != NULL)
        {
            pDrawViewShell->ChangeEditMode(eEditMode, sal_False);
            pDrawViewShell->SwitchPage(nPageIndex);
        }
    }
}




void Outliner::EnterEditMode (sal_Bool bGrabFocus)
{
    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
    if (pOutlinerView != NULL)
    {
        pOutlinerView->SetOutputArea( Rectangle( Point(), Size(1, 1)));
        SetPaperSize( mpTextObj->GetLogicRect().GetSize() );
        SdrPageView* pPV = mpView->GetSdrPageView();

        // Make FuText the current function.
        SfxUInt16Item aItem (SID_TEXTEDIT, 1);
        ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
        pViewShell->GetDispatcher()->
            Execute(SID_TEXTEDIT, SFX_CALLMODE_SYNCHRON |
                SFX_CALLMODE_RECORD, &aItem, 0L);

        // To be consistent with the usual behaviour in the Office the text
        // object that is put into edit mode would have also to be selected.
        // Starting the text edit mode is not enough so we do it here by
        // hand.
        mbExpectingSelectionChangeEvent = true;
        mpView->UnmarkAllObj (pPV);
        mpView->MarkObj (mpTextObj, pPV);

        if( mpTextObj )
            mpTextObj->setActiveText( mnText );

        // Turn on the edit mode for the text object.
        mpView->SdrBeginTextEdit(mpTextObj, pPV, mpWindow, sal_True, this, pOutlinerView, sal_True, sal_True, bGrabFocus);

        SetUpdateMode(sal_True);
        mbFoundObject = sal_True;
    }
}




/**
 * SpellChecker: Error link handler
 */
IMPL_LINK_INLINE_START( Outliner, SpellError, void *, nLang )
{
    mbError = true;
    OUString aError( SvtLanguageTable::GetLanguageString( (LanguageType)(sal_uLong)nLang ) );
    ErrorHandler::HandleError(* new StringErrorInfo(
                                ERRCODE_SVX_LINGU_LANGUAGENOTEXISTS, aError) );
    return 0;
}
IMPL_LINK_INLINE_END( Outliner, SpellError, void *, nLang )




ESelection Outliner::GetSearchStartPosition (void)
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
            xub_StrLen nLastParagraphLength = GetEditEngine().GetTextLen (
                nParagraphCount-1);
            aPosition = ESelection (nParagraphCount-1, nLastParagraphLength);
        }
    }

    return aPosition;
}




bool Outliner::HasNoPreviousMatch (void)
{
    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();

    DBG_ASSERT (pOutlinerView!=NULL, "outline view in Outliner::HasNoPreviousMatch is NULL");

    // Detect whether the cursor stands at the beginning
    // resp. at the end of the text.
    return pOutlinerView->GetSelection().IsEqual(GetSearchStartPosition ()) == sal_True;
}




bool Outliner::HandleFailedSearch (void)
{
    bool bContinueSearch = false;

    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
    if (pOutlinerView != NULL && mpSearchItem != NULL)
    {
        // Detect whether there is/may be a prior match.  If there is then
        // ask the user whether to wrap arround.  Otherwise tell the user
        // that there is no match.
        if (HasNoPreviousMatch ())
        {
            // No match found in the whole presentation.  Tell the user.
            InfoBox aInfoBox (NULL, SD_RESSTR(STR_SAR_NOT_FOUND));
            ShowModalMessageBox (aInfoBox);
        }

        else
        {
            // No further matches found.  Ask the user whether to wrap
            // arround and start again.
            bContinueSearch = ShowWrapArroundDialog ();
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




void Outliner::SetViewShell (const ::boost::shared_ptr<ViewShell>& rpViewShell)
{
    ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
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
            if (pOutlinerView != NULL)
                pOutlinerView->SetWindow(mpWindow);
        }
        else
        {
            mpView = NULL;
            mpWindow = NULL;
        }
    }
}




void Outliner::HandleChangedSelection (void)
{
    maMarkListCopy.clear();
    mbRestrictSearchToSelection = (mpView->AreObjectsMarked()==sal_True);
    if (mbRestrictSearchToSelection)
    {
        // Make a copy of the current mark list.
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
        sal_uLong nCount = rMarkList.GetMarkCount();
        if (nCount > 0)
        {
            maMarkListCopy.clear();
            maMarkListCopy.reserve (nCount);
            for (sal_uLong i=0; i<nCount; i++)
                maMarkListCopy.push_back (rMarkList.GetMark(i)->GetMarkedSdrObj ());
        }
        else
            // No marked object.  Is this case possible?
            mbRestrictSearchToSelection = false;
    }
}





void Outliner::StartConversion( sal_Int16 nSourceLanguage,  sal_Int16 nTargetLanguage,
        const Font *pTargetFont, sal_Int32 nOptions, sal_Bool bIsInteractive )
{
    ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    sal_Bool bMultiDoc = pViewShell->ISA(DrawViewShell);

    meMode = TEXT_CONVERSION;
    mbDirectionIsForward = true;
    mpSearchItem = NULL;
    mnConversionLanguage = nSourceLanguage;

    BeginConversion();

    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
    if (pOutlinerView != NULL)
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
void Outliner::PrepareConversion (void)
{
    SetUpdateMode(sal_True);
    if( HasConvertibleTextPortion( mnConversionLanguage ) )
    {
        SetUpdateMode(sal_False);
        mbStringFound = sal_True;
        mbMatchMayExist = sal_True;

        EnterEditMode ();

        mpDrawDocument->GetDocSh()->SetWaitCursor( sal_False );
        // Start seach at the right end of the current object's text
        // depending on the search direction.
    }
    else
    {
        SetUpdateMode(sal_False);
    }
}




void Outliner::BeginConversion (void)
{
    SetRefDevice( SD_MOD()->GetRefDevice( *mpDrawDocument->GetDocSh() ) );

    ViewShellBase* pBase = PTR_CAST(ViewShellBase, SfxViewShell::Current());
    if (pBase != NULL)
        SetViewShell (pBase->GetMainViewShell());

    ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if (pViewShell)
    {
        mbStringFound = sal_False;

        // Supposed that we are not located at the very beginning/end of the
        // document then there may be a match in the document prior/after
        // the current position.
        mbMatchMayExist = sal_True;

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




sal_Bool Outliner::ConvertNextDocument()
{
    ::boost::shared_ptr<ViewShell> pViewShell (mpWeakViewShell.lock());
    if (pViewShell && pViewShell->ISA(OutlineViewShell) )
        return false;

    mpDrawDocument->GetDocSh()->SetWaitCursor( sal_True );

    Initialize ( true );

    OutlinerView* pOutlinerView = mpImpl->GetOutlinerView();
    if (pOutlinerView != NULL)
    {
        mpWindow = pViewShell->GetActiveWindow();
        pOutlinerView->SetWindow(mpWindow);
    }
    ProvideNextTextObject ();

    mpDrawDocument->GetDocSh()->SetWaitCursor( sal_False );
    ClearModifyFlag();

    // for text conversion we automaticly wrap around one
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
    // We assume that the parent of the given messge box is NULL, i.e. it is
    // modal with respect to the top application window. However, this
    // does not affect the search dialog.  Therefore we have to lock it here
    // while the message box is being shown.  We also have to take into
    // account that we are called during a spell check and the search dialog
    // is not available.
    ::Window* pSearchDialog = NULL;
    SfxChildWindow* pChildWindow = NULL;
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

    if (pChildWindow != NULL)
        pSearchDialog = pChildWindow->GetWindow();
    if (pSearchDialog != NULL)
        pSearchDialog->EnableInput(sal_False,sal_True);

    sal_uInt16 nResult = rMessageBox.Execute();

    // Unlock the search dialog.
    if (pSearchDialog != NULL)
        pSearchDialog->EnableInput(sal_True,sal_True);

    return nResult;
}




//===== Outliner::Implementation ==============================================

Outliner::Implementation::Implementation (void)
    : meOriginalEditMode(EM_PAGE),
      mbOwnOutlineView(false),
      mpOutlineView(NULL)
{
}




Outliner::Implementation::~Implementation (void)
{
    if (mbOwnOutlineView && mpOutlineView!=NULL)
    {
        mpOutlineView->SetWindow(NULL);
        delete mpOutlineView;
        mpOutlineView = NULL;
    }
}




OutlinerView* Outliner::Implementation::GetOutlinerView ()
{
    return mpOutlineView;
}




/** We try to create a new OutlinerView only when there is none available,
    either from an OutlinerViewShell or a previous call to
    ProvideOutlinerView().  This is necessary to support the spell checker
    which can not cope with exchanging the OutlinerView.
*/
void Outliner::Implementation::ProvideOutlinerView (
    Outliner& rOutliner,
    const ::boost::shared_ptr<ViewShell>& rpViewShell,
    ::Window* pWindow)
{
    if (rpViewShell.get() != NULL)
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
                if (mpOutlineView!=NULL && !mbOwnOutlineView)
                    mpOutlineView = NULL;
                if (mpOutlineView == NULL)
                {
                    mpOutlineView = new OutlinerView(&rOutliner, pWindow);
                    mbOwnOutlineView = true;
                    bInsert = true;
                }
                else
                    mpOutlineView->SetWindow(pWindow);
                sal_uLong nStat = mpOutlineView->GetControlWord();
                nStat &= ~EV_CNTRL_AUTOSCROLL;
                mpOutlineView->SetControlWord(nStat);
                if (bInsert)
                    rOutliner.InsertView( mpOutlineView );
                rOutliner.SetUpdateMode(sal_False);
                mpOutlineView->SetOutputArea (Rectangle (Point(), Size(1, 1)));
                rOutliner.SetPaperSize( Size(1, 1) );
                rOutliner.SetText(OUString(), rOutliner.GetParagraph(0));

                meOriginalEditMode =
                    ::boost::static_pointer_cast<DrawViewShell>(rpViewShell)->GetEditMode();
            }
            break;

            case ViewShell::ST_OUTLINE:
            {
                if (mpOutlineView!=NULL && mbOwnOutlineView)
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




void Outliner::Implementation::ReleaseOutlinerView (void)
{
    if (mbOwnOutlineView)
    {
        OutlinerView* pView = mpOutlineView;
        mpOutlineView = NULL;
        mbOwnOutlineView = false;
        if (pView != NULL)
        {
            pView->SetWindow(NULL);
            delete pView;
        }
    }
    else
    {
        mpOutlineView = NULL;
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
