/*************************************************************************
 *
 *  $RCSfile: Outliner.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:51:13 $
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

#include "Outliner.hxx"

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _EEITEMID_HXX //autogen
#include <svx/eeitemid.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifdef ITEMID_SEARCH
#undef ITEMID_SEARCH
#endif
#define ITEMID_SEARCH           SID_SEARCH_ITEM
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _EDITSTAT_HXX //autogen
#include <svx/editstat.hxx>
#endif
#ifndef _SV_OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#ifndef _DLGUTIL_HXX
#include <svx/dlgutil.hxx>
#endif
#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SVXERR_HXX //autogen
#include <svx/svxerr.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _SVDITER_HXX //autogen
#include <svx/svditer.hxx>
#endif
#include <comphelper/extract.hxx>
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSPELLCHECKER1_HPP_
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include <svx/forbiddencharacterstable.hxx>
#endif
#ifndef _SVX_SRCHDLG_HXX
#include <svx/srchdlg.hxx>
#endif
#ifndef _SVTOOLS_LINGUPROPS_HXX_
#include <svtools/linguprops.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#ifndef _MyEDITENG_HXX
#include <svx/editeng.hxx>
#endif

#include "strings.hrc"
#include "sdstring.hrc"
#include "eetext.hxx"
#include "sdpage.hxx"
#include "app.hxx"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "sdresid.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "optsitem.hxx"
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "PaneManager.hxx"
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

class SfxStyleSheetPool;

namespace sd {

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

Outliner::Outliner( SdDrawDocument* pDoc, USHORT nMode )
    : SdrOutliner( &pDoc->GetItemPool(), nMode ),
      mpDrawDocument(pDoc),
      mbOwnOutlineView(false),
      mpOutlineView(NULL),
      mpViewShell(NULL),
      mpView(NULL),
      mbStringFound(FALSE),
      mbEndOfSearch(FALSE),
      mbFoundObject(FALSE),
      mbError(FALSE),
      mbDirectionIsForward(true),
      mbRestrictSearchToSelection(false),
      mpObj(NULL),
      mpFirstObj(NULL),
      mpTextObj(NULL),
      mpParaObj(NULL),
      mpSearchItem(NULL),
      mbPrepareSpellingPending(true),
      mbViewShellValid(true)
{
    SetStyleSheetPool((SfxStyleSheetPool*) mpDrawDocument->GetStyleSheetPool());
    SetEditTextObjectPool( &pDoc->GetItemPool() );
    SetCalcFieldValueHdl(LINK(SD_MOD(), SdModule, CalcFieldValueHdl));
    SetForbiddenCharsTable( pDoc->GetForbiddenCharsTable() );

    ULONG nCntrl = GetControlWord();
    nCntrl |= EE_CNTRL_ALLOWBIGOBJS;
    nCntrl |= EE_CNTRL_URLSFXEXECUTE;
    nCntrl |= EE_CNTRL_MARKFIELDS;
    nCntrl |= EE_CNTRL_AUTOCORRECT;

    BOOL bHideSpell = TRUE;
    BOOL bOnlineSpell = FALSE;

    DrawDocShell* pDocSh = mpDrawDocument->GetDocSh();

    if (pDocSh)
    {
        bHideSpell = mpDrawDocument->GetHideSpell();
        bOnlineSpell = mpDrawDocument->GetOnlineSpell();
    }
    else
    {
        bHideSpell = sal_True;
        bOnlineSpell = sal_False;

        try
        {
            const SvtLinguConfig    aLinguConfig;
            Any                     aAny;

            aAny = aLinguConfig.GetProperty(
                rtl::OUString::createFromAscii( UPN_IS_SPELL_HIDE ) );
            aAny >>= bHideSpell;

            aAny = aLinguConfig.GetProperty(
                rtl::OUString::createFromAscii( UPN_IS_SPELL_AUTO ) );
            aAny >>= bOnlineSpell;
        }
        catch( ... )
        {
            DBG_ERROR( "Ill. type in linguistic property" );
        }
    }

    if (bHideSpell)
        nCntrl |= EE_CNTRL_NOREDLINES;  else
        nCntrl &= ~EE_CNTRL_NOREDLINES;

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

    SetDefaultLanguage( Application::GetSettings().GetLanguage() );
}




/// Nothing spectecular in the destructor.
Outliner::~Outliner (void)
{}




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

    <li>For a <type>SlideViewShell</type> no action is performed.</li>
    </ol>
*/
void Outliner::PrepareSpelling (void)
{
    if (mbViewShellValid)
    {
        mbPrepareSpellingPending = false;

        ViewShellBase* pBase = PTR_CAST(ViewShellBase,SfxViewShell::Current());
        if (pBase != NULL)
            SetViewShell (pBase->GetMainViewShell());
        SetRefDevice( SD_MOD()->GetRefDevice( *mpDrawDocument->GetDocSh() ) );

        SetRefDevice( SD_MOD()->GetRefDevice( *mpDrawDocument->GetDocSh() ) );

        if (mpViewShell != NULL)
        {
            mbStringFound = FALSE;

            // Supposed that we are not located at the very beginning/end of
            // the document then there may be a match in the document
            // prior/after the current position.
            mbMatchMayExist = TRUE;

            maObjectIterator = ::sd::outliner::Iterator();
            maSearchStartPosition = ::sd::outliner::Iterator();
            RememberStartPosition();

            ProvideOutlinerView ();

            HandleChangedSelection ();
        }
        ClearModifyFlag();
    }
}




/*************************************************************************
|*
|* Spelling: Pruefung starten
|*
\************************************************************************/

void Outliner::StartSpelling (void)
{
    BOOL bMultiDoc = FALSE;
    meMode = SPELL;
    mbDirectionIsForward = true;
    mpSearchItem = NULL;

    if (mbViewShellValid)
    {
        //    InitPage (mnStartPageIndex);

        if (mpViewShell->ISA(DrawViewShell))
            bMultiDoc = TRUE;

        EESpellState eState = mpOutlineView->StartSpeller( bMultiDoc );

        if (bMultiDoc)
        {
            // At this point we have to be carfull when re-setting the
            // selection.  The outline view may (very likely) have been
            // removed from the view stack of the edit engine by previous
            // BegTextEdit() and EndTextEdit() calls.  Without it setting
            // the selection may lead to a crash.  To prevent this we push
            // the view temporarily on the stack and only then set the
            // selection.
            ESelection aSelection;
            bool bPutViewOnStack = false;
            if (GetView(0) == NULL)
            {
                bPutViewOnStack = true;
                InsertView (mpOutlineView);
            }
            mpOutlineView->SetSelection(aSelection);
            if (bPutViewOnStack)
                RemoveView (ULONG(0));

            mpView->UnmarkAllObj (mpView->GetPageViewPvNum(0));
            mpView->EndTextEdit();
        }

        // Restore the start position before displaying a dialog.
        RestoreStartPosition ();

        if (eState == EE_SPELL_NOLANGUAGE)
        {
            ErrorBox aErrorBox (NULL, WB_OK, String(SdResId(STR_NOLANGUAGE)));
            ShowModalMessageBox (aErrorBox);
        }
        else
        {
            if (mpView->AreObjectsMarked())
            {
                InfoBox aInfoBox (NULL, String(SdResId(STR_END_SPELLING_OBJ)));
                ShowModalMessageBox (aInfoBox);
            }
            else
            {
                InfoBox aInfoBox(NULL, String(SdResId(STR_END_SPELLING)));
                ShowModalMessageBox (aInfoBox);
            }
        }
    }
}




/** Free all resources acquired during the search/spell check.  After a
    spell check the start position is restored here.
*/
void Outliner::EndSpelling (void)
{
    if (mbViewShellValid)
    {
        ViewShellBase* pBase = PTR_CAST(ViewShellBase,SfxViewShell::Current());
        if (pBase != NULL)
            mpViewShell = pBase->GetMainViewShell();
        else
            mpViewShell = NULL;

        // When in <member>PrepareSpelling()</member> a new outline view has
        // been created then delete it here.
        sal_Bool bViewIsDrawViewShell(mpViewShell!=NULL
            && mpViewShell->ISA(DrawViewShell));
        if (bViewIsDrawViewShell)
        {
            SetStatusEventHdl(Link());
            mpView = mpViewShell->GetView();
            mpView->UnmarkAllObj (mpView->GetPageViewPvNum(0));
            mpView->EndTextEdit();
            // Make FuSelection the current function.
            mpViewShell->GetDispatcher()->Execute(
                SID_OBJECT_SELECT,
                SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);

            SetUpdateMode(TRUE);
        }

        // #95811# Before clearing the modify flag use it as a hint that
        // changes were done at SpellCheck
        if(IsModified())
        {
            if(mpView && mpView->ISA(OutlineView))
                static_cast<OutlineView*>(mpView)->PrepareClose(FALSE);
            if(mpDrawDocument && !mpDrawDocument->IsChanged())
                mpDrawDocument->SetChanged(TRUE);
        }

        // #95811# now clear the modify flag to have a specified state of
        // Outliner
        ClearModifyFlag();

        // When spell checking then restore the start position.
        if (meMode==SPELL || meMode==HANGUL_HANJA_CONVERSION)
            RestoreStartPosition ();
    }

    mpViewShell = NULL;
    mpView = NULL;
    mpWindow = NULL;
}




void Outliner::HandleOutsideChange (ChangeHint eHint)
{
    switch (eHint)
    {
        case CH_VIEW_SHELL_INVALID:
            EndSpelling();
            mbPrepareSpellingPending = true;
            mbViewShellValid = false;
            break;

        case CH_VIEW_SHELL_VALID:
            mbViewShellValid = true;
            break;
    }
}



/*************************************************************************
|*
|* Spelling: naechstes TextObjekt pruefen
|*
\************************************************************************/

BOOL Outliner::SpellNextDocument (void)
{
    mpDrawDocument->GetDocSh()->SetWaitCursor( TRUE );

    // Extract the search direction.
    BOOL bBackwards = FALSE;
    Reference<beans::XPropertySet>  xProperties (SvxGetLinguPropertySet());
    if (xProperties.is())
    {
        Any aValue = xProperties->getPropertyValue(OUString(
            RTL_CONSTASCII_USTRINGPARAM(UPN_IS_WRAP_REVERSE)));
        aValue >>= bBackwards;
    }

    Initialize ( ! bBackwards);

    mpWindow = mpViewShell->GetActiveWindow();
    mpOutlineView->SetWindow(mpWindow);
    ProvideNextTextObject ();

    mpDrawDocument->GetDocSh()->SetWaitCursor( FALSE );
    ClearModifyFlag();

    return !mbEndOfSearch;
}




/** Go to next match.
*/
BOOL Outliner::StartSearchAndReplace (const SvxSearchItem* pSearchItem)
{
    BOOL bEndOfSearch = TRUE;

    if (mbViewShellValid)
    {
        mpDrawDocument->GetDocSh()->SetWaitCursor( TRUE );
        if (mbPrepareSpellingPending)
            PrepareSpelling();
        ViewShellBase* pBase = PTR_CAST(ViewShellBase,SfxViewShell::Current());
        // Determine whether we have to abort the search.  This is necessary
        // when the main view shell does not support searching.
        bool bAbort = false;
        if (pBase != NULL)
        {
            ViewShell* pShell = pBase->GetMainViewShell();
            SetViewShell (pShell);
            if (pShell == NULL)
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

        if ( ! bAbort)
        {
            meMode = SEARCH;
            mpSearchItem = pSearchItem;

            mbFoundObject = FALSE;

            Initialize ( ! mpSearchItem->GetBackward());

            USHORT nCommand = mpSearchItem->GetCommand();
            if (nCommand == SVX_SEARCHCMD_REPLACE_ALL)
                bEndOfSearch = SearchAndReplaceAll ();
            else
            {
                RememberStartPosition ();
                bEndOfSearch = SearchAndReplaceOnce ();
                //#107233# restore start position if nothing was found
                if(!mbStringFound)
                    RestoreStartPosition ();
                else
                    mnStartPageIndex = -1;
            }
        }
        else
            mpDrawDocument->GetDocSh()->SetWaitCursor( FALSE );
    }

    return bEndOfSearch;
}




void Outliner::Initialize (bool bDirectionIsForward)
{
    bool bOldDirectionIsForward = mbDirectionIsForward;
    mbDirectionIsForward = bDirectionIsForward;

    if (maObjectIterator == ::sd::outliner::Iterator())
    {
        // Initialize a new search.
        maObjectIterator = ::sd::outliner::OutlinerContainer(this).current();
        maCurrentPosition = *maObjectIterator;

        // In case we are searching in an outline view then first remove the
        // current selection and place cursor at its start or end.
        if (mpViewShell->ISA(OutlineViewShell))
        {
            ESelection aSelection = mpOutlineView->GetSelection ();
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
            mpOutlineView->SetSelection (aSelection);
        }

        // When not beginning the search at the beginning of the search area
        // then there may be matches before the current position.
        mbMatchMayExist = (maObjectIterator!=::sd::outliner::OutlinerContainer(this).begin());
    }
    else if (bOldDirectionIsForward != mbDirectionIsForward)
    {
        // Requested iteration direction has changed.  Turn arround the iterator.
        maObjectIterator.Reverse();
        // The iterator has pointed to the object one ahead/before the current
        // one.  Now move it to the one before/ahead the current one.
        ++maObjectIterator;
        ++maObjectIterator;

        mbMatchMayExist = true;
    }

    // Initialize the last valid position with where the search starts so
    // that it always points to a valid position.
    maLastValidPosition = *::sd::outliner::OutlinerContainer(this).current();
}




BOOL Outliner::SearchAndReplaceAll (void)
{
    // Save the current position to be restored after having replaced all
    // matches.
    RememberStartPosition ();

    if (mpViewShell->ISA(OutlineViewShell))
    {
        // Put the cursor to the beginning/end of the outliner.
        mpOutlineView->SetSelection (GetSearchStartPosition ());

        // The outliner does all the work for us when we are in this mode.
        SearchAndReplaceOnce();
    }
    else if (mpViewShell->ISA(DrawViewShell))
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

    return TRUE;
}




BOOL Outliner::SearchAndReplaceOnce (void)
{
    DetectChange ();

    DBG_ASSERT( mpOutlineView && GetEditEngine().HasView( &mpOutlineView->GetEditView() ), "SearchAndReplace without valid view!" );

    if( NULL == mpOutlineView || !GetEditEngine().HasView( &mpOutlineView->GetEditView() ) )
        return true;

    if (mpViewShell != NULL)
    {
        mpView = mpViewShell->GetView();
        mpWindow = mpViewShell->GetActiveWindow();
        DBG_ASSERT (mpOutlineView!=NULL, "Outliner::SearchAndReplaceOnce(): mpOutlineView!=NULL");
        mpOutlineView->SetWindow(mpWindow);

        if (mpViewShell->ISA(DrawViewShell) )
        {
            // When replacing we first check if there is a selection
            // indicating a match.  If there is then replace it.  The
            // following call to StartSearchAndReplace will then search for
            // the next match.
            if (meMode == SEARCH
                && mpSearchItem->GetCommand() == SVX_SEARCHCMD_REPLACE)
                if (mpOutlineView != NULL)
                    if (mpOutlineView->GetSelection().HasRange())
                        mpOutlineView->StartSearchAndReplace(*mpSearchItem);

            // Search for the next match.
            ULONG nMatchCount = 0;
            if (mpSearchItem->GetCommand() != SVX_SEARCHCMD_REPLACE_ALL)
                nMatchCount = mpOutlineView->StartSearchAndReplace(*mpSearchItem);

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
                    DBG_ASSERT( mpOutlineView && GetEditEngine().HasView(
                        &mpOutlineView->GetEditView() ), "SearchAndReplace without valid view!" );
                    if( NULL == mpOutlineView
                        || !GetEditEngine().HasView( &mpOutlineView->GetEditView() ) )
                    {
                        mpDrawDocument->GetDocSh()->SetWaitCursor( FALSE );
                        return true;
                    }

                    if (meMode == SEARCH)
                        nMatchCount = mpOutlineView->StartSearchAndReplace(*mpSearchItem);
                }
            }
        }
        else if (mpViewShell->ISA(OutlineViewShell))
        {
            mpDrawDocument->GetDocSh()->SetWaitCursor (FALSE);
            // The following loop is executed more then once only when a
            // wrap arround search is done.
            while (true)
            {
                int nResult = mpOutlineView->StartSearchAndReplace(*mpSearchItem);
                if (nResult == 0)
                {
                    if (HandleFailedSearch ())
                        if (mpOutlineView != NULL)
                        {
                            mpOutlineView->SetSelection (GetSearchStartPosition ());
                            continue;
                        }
                }
                else
                    mbStringFound = true;
                break;
            }
        }
    }

    mpDrawDocument->GetDocSh()->SetWaitCursor( FALSE );

    return mbEndOfSearch;
}




/** Try to detect whether the document or the view (shell) has changed since
    the last time <member>StartSearchAndReplace()</member> has been called.
*/
void Outliner::DetectChange (void)
{
    ::sd::outliner::IteratorPosition aPosition (maCurrentPosition);

    DrawViewShell* pDrawViewShell = NULL;
    if (mpViewShell->ISA(DrawViewShell))
        pDrawViewShell = static_cast<DrawViewShell*>(mpViewShell);

    // Detect whether the view has been switched from the outside.
    if (pDrawViewShell != NULL
        && (aPosition.meEditMode != pDrawViewShell->GetEditMode()
            || aPosition.mePageKind != pDrawViewShell->GetPageKind()))
    {
        // Either the edit mode or the page kind has changed.
        SetStatusEventHdl(Link());

        SdrPageView* pPageView = mpView->GetPageViewPvNum(0);
        if (pPageView != NULL)
            mpView->UnmarkAllObj (pPageView);
        mpView->EndTextEdit();
        SetUpdateMode(FALSE);
        mpOutlineView->SetOutputArea( Rectangle( Point(), Size(1, 1) ) );
        if (meMode == SPELL)
            SetPaperSize( Size(1, 1) );
        SetText( String(), GetParagraph( 0 ) );

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
    ULONG nMarkCount = mpView->GetMarkedObjectList().GetMarkCount();

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
                        bSelectionHasChanged = (mpObj != pMark->GetObj ());
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
    if (mpViewShell->ISA(DrawViewShell))
    {
        DrawViewShell* pDrawViewShell =
            static_cast<DrawViewShell*>(mpViewShell);
        if (pDrawViewShell != NULL)
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
    else if (mpViewShell->ISA(OutlineViewShell))
    {
        // Remember the current cursor position.
        maStartSelection = GetView(0)->GetSelection();
    }
    else
    {
        mnStartPageIndex = -1;
    }
}




void Outliner::RestoreStartPosition (void)
{
    bool bRestore = true;
    // Take a negative start page index as inidicator that restoring the
    // start position is not requested.
    if (mnStartPageIndex < 0)
        bRestore = false;
    // Dont't resore when the view shell is not valid.
    if (mpViewShell == NULL)
        bRestore = false;
    if ( ! mbViewShellValid)
        bRestore = false;

    if (bRestore)
    {
        if (mpViewShell->ISA(DrawViewShell))
        {
            DrawViewShell* pDrawViewShell =
                static_cast<DrawViewShell*>(mpViewShell);
            SetViewMode (meStartViewMode);
            if (pDrawViewShell != NULL)
                SetPage (meStartEditMode, mnStartPageIndex);


            if (mpStartEditedObject != NULL)
            {
                SdrPageView* pPageView = mpView->GetPageViewPvNum(0);
                mpView->BegTextEdit (mpStartEditedObject);
                ::Outliner* pOutliner =
                      static_cast<DrawView*>(mpView)->GetTextEditOutliner();
                if (pOutliner!=NULL && pOutliner->GetViewCount()>0)
                {
                    OutlinerView* pOutlinerView = pOutliner->GetView(0);
                    pOutlinerView->SetSelection(maStartSelection);
                }
            }
        }
        else if (mpViewShell->ISA(OutlineViewShell))
        {
            // Set cursor to its old position.
            GetView(0)->SetSelection (maStartSelection);
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
    mbEndOfSearch = FALSE;
    mbFoundObject = FALSE;

    mpView->UnmarkAllObj (mpView->GetPageViewPvNum(0));
    try
    {
        mpView->EndTextEdit();
    }
    catch (::com::sun::star::uno::Exception e)
    {
        OSL_TRACE ("caught exception while ending text edit mode");
    }
    SetUpdateMode(FALSE);
    mpOutlineView->SetOutputArea( Rectangle( Point(), Size(1, 1) ) );
    if (meMode == SPELL)
        SetPaperSize( Size(1, 1) );
    SetText( String(), GetParagraph( 0 ) );

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

                if (mpViewShell != NULL)
                    switch (meMode)
                    {
                        case SEARCH:
                            PrepareSearchAndReplace ();
                            break;
                        case SPELL:
                            PrepareSpellCheck ();
                            break;
                        case HANGUL_HANJA_CONVERSION:
                            PrepareHangulHanjaConversion();
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
    // Before we display a dialog we first jump to where the last valid text
    // object was found.  All page and view mode switching since then was
    // temporary and should not be visible to the user.
    SetObject (maLastValidPosition);

    if (mbRestrictSearchToSelection)
        ShowEndOfSearchDialog ();
    else
    {
        // When no match has been found so far then terminate the search.
        if ( ! mbMatchMayExist)
        {
            ShowEndOfSearchDialog ();
            mbEndOfSearch = TRUE;
        }
        // Ask the user whether to wrap arround and continue the search or
        // to terminate.
        else if (meMode==HANGUL_HANJA_CONVERSION || ShowWrapArroundDialog ())
        {
            mbMatchMayExist = false;
            maObjectIterator = ::sd::outliner::OutlinerContainer(this).begin();
            mbEndOfSearch = FALSE;
        }
        else
        {
            // No wrap arround.
            mbEndOfSearch = TRUE;
        }
    }
}




void Outliner::InitPage (USHORT nPageIndex)
{
    ::sd::outliner::IteratorPosition aPosition (*maObjectIterator);
    if (aPosition.meEditMode == EM_PAGE)
        mnPageCount = mpDrawDocument->GetSdPageCount(aPosition.mePageKind);
    else
        mnPageCount = mpDrawDocument->GetMasterSdPageCount(aPosition.mePageKind);
}




void Outliner::ShowEndOfSearchDialog (void)
{
    if (meMode == SEARCH)
    {
        String aString;

        if (mbStringFound)
            aString = String( SdResId(STR_END_SEARCHING) );
        else
            aString = String( SdResId(STR_STRING_NOTFOUND) );

        // Show the message in an info box that is modal with respect to the
        // whole application.
        InfoBox aInfoBox (NULL, aString);
        ShowModalMessageBox (aInfoBox);
    }
}




bool Outliner::ShowWrapArroundDialog (void)
{
    bool bDoWrapArround = false;

    // Determine whether to show the dialog.
    bool bShowDialog = false;
    if (mpSearchItem != NULL)
    {
        // When searching display the dialog only for single find&replace.
        USHORT nCommand = mpSearchItem->GetCommand();
        bShowDialog = (nCommand==SVX_SEARCHCMD_REPLACE)
            || (nCommand==SVX_SEARCHCMD_FIND);
    }
    else
        // Spell checking needs the dialog, too.
        bShowDialog = (meMode == SPELL);

    if (bShowDialog)
    {
        // The question text depends on the search direction.
        BOOL bImpress = mpDrawDocument!=NULL
            && mpDrawDocument->GetDocumentType() == DOCUMENT_TYPE_IMPRESS;
        USHORT nStringId;
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
            String(SdResId(nStringId)));
        aQuestionBox.SetImage (QueryBox::GetStandardImage());
        USHORT nBoxResult = ShowModalMessageBox(aQuestionBox);
        bDoWrapArround = (nBoxResult == BUTTONID_YES);
    }

    return bDoWrapArround;
}




bool Outliner::IsValidTextObject (const ::sd::outliner::IteratorPosition& rPosition)
{
    SdrObject* pObject = rPosition.mpObject;
    return pObject != NULL
        && pObject->ISA(SdrTextObj)
        && static_cast<SdrTextObj*>(pObject)->HasText()
        && ! pObject->IsEmptyPresObj();
}




void Outliner::PutTextIntoOutliner (void)
{
    if ( mpObj && mpObj->ISA(SdrTextObj)
        && static_cast<SdrTextObj*>(mpObj)->HasText()
        && !mpObj->IsEmptyPresObj() )
    {
        mpTextObj = (SdrTextObj*) mpObj;
        mpParaObj = mpTextObj->GetOutlinerParaObject();

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
        mbError = TRUE;
        mbEndOfSearch = TRUE;
        ErrorBox aErrorBox (NULL,
            WB_OK,
            String(SdResId(STR_NOLANGUAGE)));
        ShowModalMessageBox (aErrorBox);
    }
    else if (eState != EE_SPELL_OK)
    {
        // When spell checking we have to test whether we have processed the
        // whole document and have reached the start page again.
        DrawViewShell* pDrawViewShell =
            static_cast<DrawViewShell*>(mpViewShell);
        if (meMode == SPELL)
            if (maSearchStartPosition == ::sd::outliner::Iterator())
                // Remember the position of the first text object so that we
                // know when we have processed the whole document.
                maSearchStartPosition = maObjectIterator;
            else if (maSearchStartPosition == maObjectIterator)
            {
                mbEndOfSearch = TRUE;
            }


        EnterEditMode ();
    }
}




void Outliner::PrepareSearchAndReplace (void)
{
    ULONG nMatchCount = 0;
    if (HasText( *mpSearchItem ))
    {
        mbStringFound = TRUE;
        mbMatchMayExist = TRUE;

        EnterEditMode ();

        mpDrawDocument->GetDocSh()->SetWaitCursor( FALSE );
        // Start seach at the right end of the current object's text
        // depending on the search direction.
        mpOutlineView->SetSelection (GetSearchStartPosition ());
    }
}




void Outliner::SetViewMode (PageKind ePageKind)
{
    if (ePageKind != static_cast<DrawViewShell*>(mpViewShell)->GetPageKind())
    {
        // Restore old edit mode.
        DrawViewShell* pDrawViewShell =
            static_cast<DrawViewShell*>(mpViewShell);
        pDrawViewShell->ChangeEditMode(meOriginalEditMode, FALSE);

        SetStatusEventHdl(Link());
        ViewShell::ShellType eType;
        switch (ePageKind)
        {
            case PK_STANDARD:
            default:
                eType = ViewShell::ST_IMPRESS;
                break;
            case PK_NOTES:
                eType = ViewShell::ST_NOTES;
                break;
            case PK_HANDOUT:
                eType = ViewShell::ST_HANDOUT;
                break;
        }
        // The text object iterator is destroyed when the shells are
        // switched but we need it so save it and restore it afterwards.
        ::sd::outliner::Iterator aIterator (maObjectIterator);
        bool bMatchMayExist = mbMatchMayExist;

        ViewShellBase& rBase = mpViewShell->GetViewShellBase();
        SetViewShell (NULL);
        rBase.GetPaneManager().RequestMainViewShellChange (
            eType,
            PaneManager::CM_SYNCHRONOUS);
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
        pDrawViewShell = static_cast<DrawViewShell*>(mpViewShell);
        meOriginalEditMode = pDrawViewShell->GetEditMode();
    }
}




void Outliner::SetPage (EditMode eEditMode, USHORT nPageIndex)
{
    if ( ! mbRestrictSearchToSelection)
    {
        static_cast<DrawViewShell*>(mpViewShell)->ChangeEditMode(
            eEditMode, FALSE);
        static_cast<DrawViewShell*>(mpViewShell)->SwitchPage(nPageIndex);
    }
}




void Outliner::EnterEditMode (void)
{
    if (mbViewShellValid && mpOutlineView != NULL)
    {
        mpOutlineView->SetOutputArea( Rectangle( Point(), Size(1, 1)));
        SetPaperSize( mpTextObj->GetLogicRect().GetSize() );
        SdrPageView* pPV = mpView->GetPageViewPvNum(0);
        FASTBOOL bIsNewObj = TRUE;

        // Make FuText the current function.
        SfxUInt16Item aItem (SID_TEXTEDIT, 1);
        mpViewShell->GetDispatcher()->
            Execute(SID_TEXTEDIT, SFX_CALLMODE_SYNCHRON |
                SFX_CALLMODE_RECORD, &aItem, 0L);

        // To be consistent with the usual behaviour in the Office the text
        // object that is put into edit mode would have also to be selected.
        // Starting the text edit mode is not enough so we do it here by
        // hand.
        mbExpectingSelectionChangeEvent = true;
        mpView->UnmarkAllObj (pPV);
        mpView->MarkObj (mpTextObj, pPV);

        // Turn on the edit mode for the text object.
        mpView->BegTextEdit(mpTextObj, pPV, mpWindow, bIsNewObj, this,
            mpOutlineView, TRUE, TRUE);


        SetUpdateMode(TRUE);
        mbFoundObject = TRUE;
    }
}




/*************************************************************************
|*
|* SpellChecker: Error-LinkHdl
|*
\************************************************************************/

IMPL_LINK_INLINE_START( Outliner, SpellError, void *, nLang )
{
    mbError = TRUE;
    String aError( ::GetLanguageString( (LanguageType)(ULONG)nLang ) );
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
        USHORT nParagraphCount = static_cast<USHORT>(GetParagraphCount());
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
    DBG_ASSERT (mpOutlineView!=NULL, "outline view in Outliner::HasNoPreviousMatch is NULL");

    // Detect whether the cursor stands at the beginning
    // resp. at the end of the text.
    return mpOutlineView->GetSelection().IsEqual (
        GetSearchStartPosition ()) == TRUE;
}




bool Outliner::HandleFailedSearch (void)
{
    bool bContinueSearch = false;

    if (mpOutlineView != NULL && mpSearchItem != NULL)
    {
        // Detect whether there is/may be a prior match.  If there is then
        // ask the user whether to wrap arround.  Otherwise tell the user
        // that there is no match.
        if (HasNoPreviousMatch ())
        {
            // No match found in the whole presentation.  Tell the user.
            InfoBox aInfoBox (NULL,
                String(SdResId(STR_SAR_NOT_FOUND)));
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




/** See task #95227# for discussion about correct parent for dialogs/info boxes.
*/
::Window* Outliner::GetParentForDialog (void)
{
    ::Window* pParent = NULL;

    if (meMode == SEARCH)
        pParent = static_cast<SvxSearchDialog*>(
            SfxViewFrame::Current()->GetChildWindow(
                SvxSearchDialogWrapper::GetChildWindowId())->GetWindow());

    if (pParent == NULL)
        pParent = mpViewShell->GetActiveWindow();

    if (pParent == NULL)
        pParent = Application::GetDefDialogParent();
    //1.30->1.31 of sdoutl.cxx        pParent = Application::GetDefModalDialogParent();

    return pParent;
}




SdrObject* Outliner::SetObject (
    const ::sd::outliner::IteratorPosition& rPosition)
{
    SetViewMode (rPosition.mePageKind);
    SetPage (rPosition.meEditMode, (USHORT)rPosition.mnPageIndex);
    return rPosition.mpObject;
}




void Outliner::SetViewShell (ViewShell* pViewShell)
{
    if (mpViewShell != pViewShell)
    {
        // Set the new view shell.
        mpViewShell = pViewShell;
        if (mpViewShell != NULL)
        {
            mpView = mpViewShell->GetView();

            mpWindow = mpViewShell->GetActiveWindow();
            if (mpOutlineView != NULL)
                mpOutlineView->SetWindow(mpWindow);
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
    mbRestrictSearchToSelection = (mpView->AreObjectsMarked()==TRUE);
    if (mbRestrictSearchToSelection)
    {
        // Make a copy of the current mark list.
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
        ULONG nCount = rMarkList.GetMarkCount();
        if (nCount > 0)
        {
            maMarkListCopy.clear();
            maMarkListCopy.reserve (nCount);
            for (ULONG i=0; i<nCount; i++)
                maMarkListCopy.push_back (rMarkList.GetMark(i)->GetObj ());
        }
        else
            // No marked object.  Is this case possible?
            mbRestrictSearchToSelection = false;
    }
}





void Outliner::StartTextConversion( INT16 nLanguage )
{
    BOOL bMultiDoc = mpViewShell->ISA(DrawViewShell);

    meMode = HANGUL_HANJA_CONVERSION;
    mbDirectionIsForward = true;
    mpSearchItem = NULL;
    mnConversionLanguage = nLanguage;

    BeginConversion();

    if (mpOutlineView != NULL)
        mpOutlineView->StartTextConversion( nLanguage, bMultiDoc );

    EndConversion();
}




/** Prepare to do a hangul hanja conversion on the current text object. This
    includes putting it into edit mode.
*/
void Outliner::PrepareHangulHanjaConversion (void)
{
    if( HasConvertibleTextPortion( mnConversionLanguage ) )
    {
        mbStringFound = TRUE;
        mbMatchMayExist = TRUE;

        EnterEditMode ();

        mpDrawDocument->GetDocSh()->SetWaitCursor( FALSE );
        // Start seach at the right end of the current object's text
        // depending on the search direction.
//      mpOutlineView->SetSelection (GetSearchStartPosition ());
    }
}




void Outliner::BeginConversion (void)
{
    SetRefDevice( SD_MOD()->GetRefDevice( *mpDrawDocument->GetDocSh() ) );

    ViewShellBase* pBase = PTR_CAST(ViewShellBase, SfxViewShell::Current());
    if (pBase != NULL)
        SetViewShell (pBase->GetMainViewShell());

    if (mpViewShell != NULL)
    {
        mbStringFound = FALSE;

        // Supposed that we are not located at the very beginning/end of the
        // document then there may be a match in the document prior/after
        // the current position.
        mbMatchMayExist = TRUE;

        maObjectIterator = ::sd::outliner::Iterator();
        maSearchStartPosition = ::sd::outliner::Iterator();
        RememberStartPosition();

        ProvideOutlinerView();

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
    if( mpViewShell && mpViewShell->ISA(OutlineViewShell) )
        return false;

    mpDrawDocument->GetDocSh()->SetWaitCursor( TRUE );

    Initialize ( true );

    mpWindow = mpViewShell->GetActiveWindow();
    mpOutlineView->SetWindow(mpWindow);
    ProvideNextTextObject ();

    mpDrawDocument->GetDocSh()->SetWaitCursor( FALSE );
    ClearModifyFlag();

    // for hangul hanja conversion we automaticly wrap around one
    // time and stop at the start shape
    if( mpFirstObj )
    {
        if( mpFirstObj == mpObj )
            return false;
    }
    else
    {
        mpFirstObj = mpObj;
    }

    return !mbEndOfSearch;
}




USHORT Outliner::ShowModalMessageBox (Dialog& rMessageBox)
{
    // We assume that the parent of the given messge box is NULL, i.e. it is
    // modal with respect to the top application window. However, this
    // does not affect the search dialog.  Therefore we have to lock it here
    // while the message box is being shown.  We also have to take into
    // account that we are called during a spell check and the search dialog
    // is not available.
    ::Window* pSearchDialog = NULL;
    SfxChildWindow* pChildWindow = SfxViewFrame::Current()->GetChildWindow(
        SvxSearchDialogWrapper::GetChildWindowId());
    if (pChildWindow != NULL)
        pSearchDialog = static_cast<SvxSearchDialog*>(
            pChildWindow->GetWindow());
    if (pSearchDialog != NULL)
        pSearchDialog->EnableInput(FALSE,TRUE);

    USHORT nResult = rMessageBox.Execute();

    // Unlock the search dialog.
    if (pSearchDialog != NULL)
        pSearchDialog->EnableInput(TRUE,TRUE);

    return nResult;
}




/** We try to create a new OutlinerView only when there is none available,
    either from an OutlinerViewShell or a previous call to
    ProvideOutlinerView().  This is necessary to support the spell checker
    which can not cope with exchanging the OutlinerView.
*/
void Outliner::ProvideOutlinerView (void)
{
    switch (mpViewShell->GetShellType())
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
                mpOutlineView = new OutlinerView( this, mpWindow );
                mbOwnOutlineView = true;
                bInsert = true;
            }
            else
                mpOutlineView->SetWindow(mpWindow);
            ULONG nStat = mpOutlineView->GetControlWord();
            nStat &= ~EV_CNTRL_AUTOSCROLL;
            mpOutlineView->SetControlWord(nStat);
            if (bInsert)
                InsertView( mpOutlineView );
            SetUpdateMode(FALSE);
            mpOutlineView->SetOutputArea (Rectangle (Point(), Size(1, 1)));
            SetPaperSize( Size(1, 1) );
            SetText( String(), GetParagraph( 0 ) );

            meOriginalEditMode =
                static_cast<DrawViewShell*>(mpViewShell)->GetEditMode();
        }
        break;

        case ViewShell::ST_OUTLINE:
        {
            if (mpOutlineView!=NULL && mbOwnOutlineView)
                delete mpOutlineView;
            mpOutlineView = GetView(0);
            mbOwnOutlineView = false;
        }
        break;

        default:
        case ViewShell::ST_NONE:
        case ViewShell::ST_SLIDE:
        case ViewShell::ST_PREVIEW:
        case ViewShell::ST_PRESENTATION:
            // Ignored
            break;
    }
}

} // end of namespace sd
