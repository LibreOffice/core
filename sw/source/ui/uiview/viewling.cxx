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

#include <hintids.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/ProofreadingResult.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <linguistic/lngprops.hxx>
#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/ehdl.hxx>
#include <svl/stritem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <svx/dlgutil.hxx>
#include <svx/dialmgr.hxx>
#include <editeng/langitem.hxx>
#include <svx/svxerr.hxx>
#include <editeng/unolingu.hxx>
#include <svx/svxdlg.hxx>
#include <editeng/SpellPortions.hxx>
#include <swmodule.hxx>
#include <swwait.hxx>
#include <initui.hxx>               // fuer SpellPointer
#include <uitool.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <basesh.hxx>
#include <docsh.hxx>                // CheckSpellChanges
#include <viewopt.hxx>              // Viewoptions
#include <swundo.hxx>               // fuer Undo-Ids
#include <hyp.hxx>                  // Trennung
#include <olmenu.hxx>               // PopupMenu fuer OnlineSpelling
#include <pam.hxx>                  // Spelling: Multiselektion
#include <edtwin.hxx>
#include <crsskip.hxx>
#include <ndtxt.hxx>
#include <vcl/lstbox.hxx>
#include <cmdid.h>
#include <globals.hrc>
#include <comcore.hrc>              // STR_MULT_INTERACT_SPELL_WARN
#include <view.hrc>
#include <hhcwrp.hxx>
#include <com/sun/star/frame/XStorable.hpp>

#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <vcl/svapp.hxx>
#include <rtl/ustring.hxx>

#include <cppuhelper/bootstrap.hxx>
#include "stmenu.hxx"              // PopupMenu for smarttags
#include <svx/dialogs.hrc>
#include <svtools/langtab.hxx>
#include <unomid.h>
#include <IMark.hxx>
#include <xmloff/odffields.hxx>

#include <memory>
#include <editeng/editerr.hxx>

using namespace sw::mark;
using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::smarttags;

/*--------------------------------------------------------------------
    Beschreibung:   Lingu-Dispatcher
 --------------------------------------------------------------------*/
void SwView::ExecLingu(SfxRequest &rReq)
{
    switch(rReq.GetSlot())
    {
        case SID_THESAURUS:
            StartThesaurus();
            rReq.Ignore();
            break;
        case SID_HANGUL_HANJA_CONVERSION:
            StartTextConversion( LANGUAGE_KOREAN, LANGUAGE_KOREAN, NULL,
                    i18n::TextConversionOption::CHARACTER_BY_CHARACTER, sal_True );
            break;
        case SID_CHINESE_CONVERSION:
        {
            //open ChineseTranslationDialog
            Reference< XComponentContext > xContext(
                ::cppu::defaultBootstrap_InitialComponentContext() ); //@todo get context from calc if that has one
            if(xContext.is())
            {
                Reference< lang::XMultiComponentFactory > xMCF( xContext->getServiceManager() );
                if(xMCF.is())
                {
                    Reference< ui::dialogs::XExecutableDialog > xDialog(
                            xMCF->createInstanceWithContext(
                                rtl::OUString("com.sun.star.linguistic2.ChineseTranslationDialog")
                                , xContext), UNO_QUERY);
                    Reference< lang::XInitialization > xInit( xDialog, UNO_QUERY );
                    if( xInit.is() )
                    {
                        //  initialize dialog
                        Reference< awt::XWindow > xDialogParentWindow(0);
                        Sequence<Any> aSeq(1);
                        Any* pArray = aSeq.getArray();
                        PropertyValue aParam;
                        aParam.Name = rtl::OUString("ParentWindow");
                        aParam.Value <<= makeAny(xDialogParentWindow);
                        pArray[0] <<= makeAny(aParam);
                        xInit->initialize( aSeq );

                        //execute dialog
                        sal_Int16 nDialogRet = xDialog->execute();
                        if( RET_OK == nDialogRet )
                        {
                            //get some parameters from the dialog
                            sal_Bool bToSimplified = sal_True;
                            sal_Bool bUseVariants = sal_True;
                            sal_Bool bCommonTerms = sal_True;
                            Reference< beans::XPropertySet >  xProp( xDialog, UNO_QUERY );
                            if( xProp.is() )
                            {
                                try
                                {
                                    xProp->getPropertyValue( C2U("IsDirectionToSimplified") ) >>= bToSimplified;
                                    xProp->getPropertyValue( C2U("IsUseCharacterVariants") ) >>= bUseVariants;
                                    xProp->getPropertyValue( C2U("IsTranslateCommonTerms") ) >>= bCommonTerms;
                                }
                                catch (const Exception&)
                                {
                                }
                            }

                            //execute translation
                            sal_Int16 nSourceLang = bToSimplified ? LANGUAGE_CHINESE_TRADITIONAL : LANGUAGE_CHINESE_SIMPLIFIED;
                            sal_Int16 nTargetLang = bToSimplified ? LANGUAGE_CHINESE_SIMPLIFIED : LANGUAGE_CHINESE_TRADITIONAL;
                            sal_Int32 nOptions    = bUseVariants ? i18n::TextConversionOption::USE_CHARACTER_VARIANTS : 0;
                            if( !bCommonTerms )
                                nOptions = nOptions | i18n::TextConversionOption::CHARACTER_BY_CHARACTER;

                            Font aTargetFont = GetEditWin().GetDefaultFont( DEFAULTFONT_CJK_TEXT,
                                                    nTargetLang, DEFAULTFONT_FLAGS_ONLYONE );

                            // disallow formatting, updating the view, ... while
                            // converting the document. (saves time)
                            // Also remember the current view and cursor position for later
                            pWrtShell->StartAction();

                            // remember cursor position data for later restoration of the cursor
                            const SwPosition *pPoint = pWrtShell->GetCrsr()->GetPoint();
                            sal_Bool bRestoreCursor = pPoint->nNode.GetNode().IsTxtNode();
                            const SwNodeIndex aPointNodeIndex( pPoint->nNode );
                            xub_StrLen nPointIndex = pPoint->nContent.GetIndex();

                            // since this conversion is not interactive the whole converted
                            // document should be undone in a single undo step.
                            pWrtShell->StartUndo( UNDO_OVERWRITE );

                            StartTextConversion( nSourceLang, nTargetLang, &aTargetFont, nOptions, sal_False );

                            pWrtShell->EndUndo( UNDO_OVERWRITE );

                            if (bRestoreCursor)
                            {
                                SwTxtNode *pTxtNode = aPointNodeIndex.GetNode().GetTxtNode();
                                // check for unexpected error case
                                OSL_ENSURE( pTxtNode && pTxtNode->GetTxt().Len() >= nPointIndex,
                                    "text missing: corrupted node?" );
                                if (!pTxtNode || pTxtNode->GetTxt().Len() < nPointIndex)
                                    nPointIndex = 0;
                                // restore cursor to its original position
                                pWrtShell->GetCrsr()->GetPoint()->nContent.Assign( pTxtNode, nPointIndex );
                            }

                            // enable all, restore view and cursor position
                            pWrtShell->EndAction();
                        }
                    }
                    Reference< lang::XComponent > xComponent( xDialog, UNO_QUERY );
                    if( xComponent.is() )
                        xComponent->dispose();
                }
            }
            break;
        }
        case FN_HYPHENATE_OPT_DLG:
            HyphenateDocument();
            break;
        default:
            OSL_ENSURE(!this, "wrong Dispatcher");
            return;
    }
}

/*--------------------------------------------------------------------
    Description: start language specific text conversion
 --------------------------------------------------------------------*/
void SwView::StartTextConversion(
        LanguageType nSourceLang,
        LanguageType nTargetLang,
        const Font *pTargetFont,
        sal_Int32 nOptions,
        sal_Bool bIsInteractive )
{
    // do not do text conversion if it is active elsewhere
    if (GetWrtShell().HasConvIter())
    {
        return;
    }

    SpellKontext(sal_True);

    const SwViewOption* pVOpt = pWrtShell->GetViewOptions();
    const sal_Bool bOldIdle = pVOpt->IsIdle();
    pVOpt->SetIdle( sal_False );

    sal_Bool bOldIns = pWrtShell->IsInsMode();
    pWrtShell->SetInsMode( sal_True );

    sal_Bool bSelection = ((SwCrsrShell*)pWrtShell)->HasSelection() ||
        pWrtShell->GetCrsr() != pWrtShell->GetCrsr()->GetNext();

    sal_Bool    bStart = bSelection || pWrtShell->IsStartOfDoc();
    sal_Bool    bOther = !bSelection && !(pWrtShell->GetFrmType(0,sal_True) & FRMTYPE_BODY);

    {
        const uno::Reference< lang::XMultiServiceFactory > xMgr(
                    comphelper::getProcessServiceFactory() );
        SwHHCWrapper aWrap( this, xMgr, nSourceLang, nTargetLang, pTargetFont,
                            nOptions, bIsInteractive,
                            bStart, bOther, bSelection );
        aWrap.Convert();
    }

    pWrtShell->SetInsMode( bOldIns );
    pVOpt->SetIdle( bOldIdle );
    SpellKontext(sal_False);
}

/*--------------------------------------------------------------------
     spellcheck and text conversion related stuff
 --------------------------------------------------------------------*/
void SwView::SpellStart( SvxSpellArea eWhich,
        sal_Bool bStartDone, sal_Bool bEndDone,
        SwConversionArgs *pConvArgs )
{
    Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );
    sal_Bool bIsWrapReverse = (!pConvArgs && xProp.is()) ?
            *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_WRAP_REVERSE) ).getValue() : sal_False;

    SwDocPositions eStart = DOCPOS_START;
    SwDocPositions eEnde  = DOCPOS_END;
    SwDocPositions eCurr  = DOCPOS_CURR;
    switch ( eWhich )
    {
        case SVX_SPELL_BODY:
            if( bIsWrapReverse )
                eCurr = DOCPOS_END;
            else
                eCurr = DOCPOS_START;
            break;
        case SVX_SPELL_BODY_END:
            if( bIsWrapReverse )
            {
                if( bStartDone )
                    eStart = DOCPOS_CURR;
                eCurr = DOCPOS_END;
            }
            else if( bStartDone )
                eCurr = DOCPOS_START;
            break;
        case SVX_SPELL_BODY_START:
            if( !bIsWrapReverse )
            {
                if( bEndDone )
                    eEnde = DOCPOS_CURR;
                eCurr = DOCPOS_START;
            }
            else if( bEndDone )
                eCurr = DOCPOS_END;
            break;
        case SVX_SPELL_OTHER:
            if( bIsWrapReverse )
            {
                eStart = DOCPOS_OTHERSTART;
                eEnde  = DOCPOS_OTHEREND;
                eCurr = DOCPOS_OTHEREND;
            }
            else
            {
                eStart = DOCPOS_OTHERSTART;
                eEnde  = DOCPOS_OTHEREND;
                eCurr = DOCPOS_OTHERSTART;
            }
            break;
        default:
            OSL_ENSURE( !this, "SpellStart with unknown Area" );
    }
    pWrtShell->SpellStart( eStart, eEnde, eCurr, pConvArgs );
}

/*--------------------------------------------------------------------
    Beschreibung: Fehlermeldung beim Spelling
 --------------------------------------------------------------------*/
// Der uebergebene Pointer nLang ist selbst der Wert
void SwView::SpellError(LanguageType eLang)
{
#if OSL_DEBUG_LEVEL > 1
    sal_Bool bFocus = GetEditWin().HasFocus();
#endif
    sal_uInt16 nPend = 0;

    if ( pWrtShell->ActionPend() )
    {
        pWrtShell->Push();
        pWrtShell->ClearMark();
        do
        {
            pWrtShell->EndAction();
            ++nPend;
        }
        while( pWrtShell->ActionPend() );
    }
    String aErr(SvtLanguageTable::GetLanguageString( eLang ) );

    SwEditWin &rEditWin = GetEditWin();
#if OSL_DEBUG_LEVEL > 1
    bFocus = rEditWin.HasFocus();
#endif
    sal_uInt16 nWaitCnt = 0;
    while( rEditWin.IsWait() )
    {
        rEditWin.LeaveWait();
        ++nWaitCnt;
    }
    if ( LANGUAGE_NONE == eLang )
        ErrorHandler::HandleError( ERRCODE_SVX_LINGU_NOLANGUAGE );
    else
        ErrorHandler::HandleError( *new StringErrorInfo( ERRCODE_SVX_LINGU_LANGUAGENOTEXISTS, aErr ) );

    while( nWaitCnt )
    {
        rEditWin.EnterWait();
        --nWaitCnt;
    }
#if OSL_DEBUG_LEVEL > 1
    bFocus = GetEditWin().HasFocus();
#endif

    if ( nPend )
    {
        while( nPend-- )
            pWrtShell->StartAction();
        pWrtShell->Combine();
    }
#if OSL_DEBUG_LEVEL > 1
    if( !bFocus )
        GetEditWin().GrabFocus();
#endif

}

/*--------------------------------------------------------------------
     Beschreibung: Spelling beenden und Cursor wiederherstellen
 --------------------------------------------------------------------*/
void SwView::SpellEnd( SwConversionArgs *pConvArgs )
{
    pWrtShell->SpellEnd( pConvArgs );
    if( pWrtShell->IsExtMode() )
        pWrtShell->SetMark();
}

void SwView::HyphStart( SvxSpellArea eWhich )
{
    switch ( eWhich )
    {
        case SVX_SPELL_BODY:
            pWrtShell->HyphStart( DOCPOS_START, DOCPOS_END );
            break;
        case SVX_SPELL_BODY_END:
            pWrtShell->HyphStart( DOCPOS_CURR, DOCPOS_END );
            break;
        case SVX_SPELL_BODY_START:
            pWrtShell->HyphStart( DOCPOS_START, DOCPOS_CURR );
            break;
        case SVX_SPELL_OTHER:
            pWrtShell->HyphStart( DOCPOS_OTHERSTART, DOCPOS_OTHEREND );
            break;
        default:
            OSL_ENSURE( !this, "HyphStart with unknown Area" );
    }
}

/*--------------------------------------------------------------------
     Beschreibung: Interaktive Trennung
 --------------------------------------------------------------------*/
void SwView::HyphenateDocument()
{
    // do not hyphenate if interactive hyphenationg is active elsewhere
    if (GetWrtShell().HasHyphIter())
    {
        MessBox( 0, WB_OK, String( SW_RES( STR_HYPH_TITLE ) ),
                String( SW_RES( STR_MULT_INTERACT_HYPH_WARN ) ) ).Execute();
        return;
    }

    SfxErrorContext aContext( ERRCTX_SVX_LINGU_HYPHENATION, aEmptyStr, pEditWin,
         RID_SVXERRCTX, &DIALOG_MGR() );

    Reference< XHyphenator >  xHyph( ::GetHyphenator() );
    if (!xHyph.is())
    {
        ErrorHandler::HandleError( ERRCODE_SVX_LINGU_LINGUNOTEXISTS );
        return;
    }

    if (pWrtShell->GetSelectionType() & (nsSelectionType::SEL_DRW_TXT|nsSelectionType::SEL_DRW))
    {
        // Silbentrennung in einem Draw-Objekt
        HyphenateDrawText();
    }
    else
    {
        SwViewOption* pVOpt = (SwViewOption*)pWrtShell->GetViewOptions();
        sal_Bool bOldIdle = pVOpt->IsIdle();
        pVOpt->SetIdle( sal_False );

        Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );


        pWrtShell->StartUndo(UNDO_INSATTR);         // spaeter gueltig

        sal_Bool bHyphSpecial = xProp.is() ?
                *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_HYPH_SPECIAL) ).getValue() : sal_False;
        sal_Bool bSelection = ((SwCrsrShell*)pWrtShell)->HasSelection() ||
            pWrtShell->GetCrsr() != pWrtShell->GetCrsr()->GetNext();
        sal_Bool bOther = pWrtShell->HasOtherCnt() && bHyphSpecial && !bSelection;
        sal_Bool bStart = bSelection || ( !bOther && pWrtShell->IsStartOfDoc() );
        bool bStop = false;
        if( !bOther && !(pWrtShell->GetFrmType(0,sal_True) & FRMTYPE_BODY) && !bSelection )
        // kein Sonderbereich eingeschaltet
        {
            // Ich will auch in Sonderbereichen trennen
            QueryBox aBox( &GetEditWin(), SW_RES( DLG_SPECIAL_FORCED ) );
            if( aBox.Execute() == RET_YES )
            {
                bOther = sal_True;
                if (xProp.is())
                {
                    sal_Bool bTrue = sal_True;
                    Any aTmp(&bTrue, ::getBooleanCppuType());
                    xProp->setPropertyValue( C2U(UPN_IS_HYPH_SPECIAL), aTmp );
                }
            }
            else
                bStop = true; // Nein Es wird nicht getrennt
        }

        if( !bStop )
        {
            SwHyphWrapper aWrap( this, xHyph, bStart, bOther, bSelection );
            aWrap.SpellDocument();
            pWrtShell->EndUndo(UNDO_INSATTR);
        }
        pVOpt->SetIdle( bOldIdle );
    }
}

bool SwView::IsValidSelectionForThesaurus() const
{
    // must not be a multi-selection, and if it is a selection it needs
    // to be within a single paragraph

    const bool bMultiSel = pWrtShell->GetCrsr() != pWrtShell->GetCrsr()->GetNext();
    const sal_Bool bSelection = ((SwCrsrShell*)pWrtShell)->HasSelection();
    return !bMultiSel && (!bSelection || pWrtShell->IsSelOnePara() );
}

String SwView::GetThesaurusLookUpText( bool bSelection ) const
{
    return bSelection ? pWrtShell->GetSelTxt() : pWrtShell->GetCurWord();
}

void SwView::InsertThesaurusSynonym( const String &rSynonmText, const String &rLookUpText, bool bSelection )
{
    sal_Bool bOldIns = pWrtShell->IsInsMode();
    pWrtShell->SetInsMode( sal_True );

    pWrtShell->StartAllAction();
    pWrtShell->StartUndo(UNDO_DELETE);

    if( !bSelection )
    {
        if(pWrtShell->IsEndWrd())
            pWrtShell->Left(CRSR_SKIP_CELLS, sal_False, 1, sal_False );

        pWrtShell->SelWrd();

        // make sure the selection build later from the data below does not
        // include "in word" character to the left and right in order to
        // preserve those. Therefore count those "in words" in order to modify
        // the selection accordingly.
        const sal_Unicode* pChar = rLookUpText.GetBuffer();
        xub_StrLen nLeft = 0;
        while (pChar && *pChar++ == CH_TXTATR_INWORD)
            ++nLeft;
        pChar = rLookUpText.Len() ? rLookUpText.GetBuffer() + rLookUpText.Len() - 1 : 0;
        xub_StrLen nRight = 0;
        while (pChar && *pChar-- == CH_TXTATR_INWORD)
            ++nRight;

        // adjust existing selection
        SwPaM *pCrsr = pWrtShell->GetCrsr();
        pCrsr->GetPoint()->nContent -= nRight;
        pCrsr->GetMark()->nContent += nLeft;
    }

    pWrtShell->Insert( rSynonmText );

    pWrtShell->EndUndo(UNDO_DELETE);
    pWrtShell->EndAllAction();

    pWrtShell->SetInsMode( bOldIns );
}

/*--------------------------------------------------------------------
    Beschreibung:   Thesaurus starten
 --------------------------------------------------------------------*/
void SwView::StartThesaurus()
{
    if (!IsValidSelectionForThesaurus())
        return;

    SfxErrorContext aContext( ERRCTX_SVX_LINGU_THESAURUS, aEmptyStr, pEditWin,
         RID_SVXERRCTX, &DIALOG_MGR() );

    // Sprache rausholen
    //
    LanguageType eLang = pWrtShell->GetCurLang();
    if( LANGUAGE_SYSTEM == eLang )
       eLang = GetAppLanguage();

    if( eLang == LANGUAGE_DONTKNOW || eLang == LANGUAGE_NONE )
    {
        SpellError( LANGUAGE_NONE );
        return;
    }

    SwViewOption* pVOpt = (SwViewOption*)pWrtShell->GetViewOptions();
    sal_Bool bOldIdle = pVOpt->IsIdle();
    pVOpt->SetIdle( sal_False );

    // get initial LookUp text
    const sal_Bool bSelection = ((SwCrsrShell*)pWrtShell)->HasSelection();
    String aTmp = GetThesaurusLookUpText( bSelection );

    Reference< XThesaurus >  xThes( ::GetThesaurus() );
    AbstractThesaurusDialog *pDlg = NULL;

    if ( !xThes.is() || !xThes->hasLocale( SvxCreateLocale( eLang ) ) )
        SpellError( eLang );
    else
    {
        // create dialog
        {   //Scope for SwWait-Object
            SwWait aWait( *GetDocShell(), sal_True );
            // load library with dialog only on demand ...
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            pDlg = pFact->CreateThesaurusDialog( &GetEditWin(), xThes, aTmp, eLang );
        }

        if ( pDlg->Execute()== RET_OK )
            InsertThesaurusSynonym( pDlg->GetWord(), aTmp, bSelection );
    }

    delete pDlg;

    pVOpt->SetIdle( bOldIdle );
}

/*--------------------------------------------------------------------
    Beschreibung:   Online-Vorschlaege anbieten
 *--------------------------------------------------------------------*/
//!! Start of extra code for context menu modifying extensions
struct ExecuteInfo
{
    uno::Reference< frame::XDispatch >  xDispatch;
    util::URL                           aTargetURL;
    uno::Sequence< PropertyValue >      aArgs;
};

class AsyncExecute
{
public:
    DECL_STATIC_LINK( AsyncExecute, ExecuteHdl_Impl, ExecuteInfo* );
};

IMPL_STATIC_LINK_NOINSTANCE( AsyncExecute, ExecuteHdl_Impl, ExecuteInfo*, pExecuteInfo )
{
    const sal_uInt32 nRef = Application::ReleaseSolarMutex();
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        pExecuteInfo->xDispatch->dispatch( pExecuteInfo->aTargetURL, pExecuteInfo->aArgs );
    }
    catch (const Exception&)
    {
    }

    Application::AcquireSolarMutex( nRef );
    delete pExecuteInfo;
    return 0;
}
//!! End of extra code for context menu modifying extensions

sal_Bool SwView::ExecSpellPopup(const Point& rPt)
{
    sal_Bool bRet = sal_False;
    const SwViewOption* pVOpt = pWrtShell->GetViewOptions();
    if( pVOpt->IsOnlineSpell() &&
        !pWrtShell->IsSelection())
    {
        if (pWrtShell->GetSelectionType() & nsSelectionType::SEL_DRW_TXT)
            bRet = ExecDrwTxtSpellPopup(rPt);
        else if (!pWrtShell->IsSelFrmMode())
        {
            const sal_Bool bOldViewLock = pWrtShell->IsViewLocked();
            pWrtShell->LockView( sal_True );
            pWrtShell->Push();
            SwRect aToFill;

            // decide which variant of the context menu to use...
            // if neither spell checking nor grammar checking provides suggestions use the
            // default context menu.
            bool bUseGrammarContext = false;
            Reference< XSpellAlternatives >  xAlt( pWrtShell->GetCorrection(&rPt, aToFill) );
            ProofreadingResult aGrammarCheckRes;
            sal_Int32 nErrorInResult = -1;
            uno::Sequence< rtl::OUString > aSuggestions;
            bool bCorrectionRes = false;
            if (!xAlt.is() || xAlt->getAlternatives().getLength() == 0)
            {
                sal_Int32 nErrorPosInText = -1;
                bCorrectionRes = pWrtShell->GetGrammarCorrection( aGrammarCheckRes, nErrorPosInText, nErrorInResult, aSuggestions, &rPt, aToFill );
                ::rtl::OUString aMessageText;
                if (nErrorInResult >= 0)
                    aMessageText = aGrammarCheckRes.aErrors[ nErrorInResult ].aShortComment;
                // we like to use the grammar checking context menu if we either get
                // some suggestions or at least a comment about the error found...
                bUseGrammarContext = bCorrectionRes &&
                        (aSuggestions.getLength() > 0 || !aMessageText.isEmpty());
            }

            // open respective context menu for spell check or grammar errors with correction suggestions...
            if ((!bUseGrammarContext && xAlt.is()) ||
                (bUseGrammarContext && bCorrectionRes && aGrammarCheckRes.aErrors.getLength() > 0))
            {
                // get paragraph text
                String aParaText;
                SwPosition aPoint( *pWrtShell->GetCrsr()->GetPoint() );
                const SwTxtNode *pNode = dynamic_cast< const SwTxtNode * >(
                                            &aPoint.nNode.GetNode() );
                if (pNode)
                    aParaText = pNode->GetTxt();    // this may include hidden text but that should be Ok
                else
                {
                    OSL_FAIL("text node expected but not found" );
                }

                bRet = sal_True;
                pWrtShell->SttSelect();
                std::auto_ptr< SwSpellPopup > pPopup;
                if (bUseGrammarContext)
                {
                    sal_Int32 nPos = aPoint.nContent.GetIndex();
                    (void) nPos;
                    pPopup = std::auto_ptr< SwSpellPopup >(new SwSpellPopup( pWrtShell, aGrammarCheckRes, nErrorInResult, aSuggestions, aParaText ));
                }
                else
                    pPopup = std::auto_ptr< SwSpellPopup >(new SwSpellPopup( pWrtShell, xAlt, aParaText ));
                ui::ContextMenuExecuteEvent aEvent;
                const Point aPixPos = GetEditWin().LogicToPixel( rPt );

                aEvent.SourceWindow = VCLUnoHelper::GetInterface( pEditWin );
                aEvent.ExecutePosition.X = aPixPos.X();
                aEvent.ExecutePosition.Y = aPixPos.Y();
                Menu* pMenu = 0;

                ::rtl::OUString sMenuName  = bUseGrammarContext ?
                    OUString("private:resource/GrammarContextMenu") : OUString("private:resource/SpellContextMenu");
                if(TryContextMenuInterception( *pPopup, sMenuName, pMenu, aEvent ))
                {

                    //! happy hacking for context menu modifying extensions of this
                    //! 'custom made' menu... *sigh* (code copied from sfx2 and framework)
                    if ( pMenu )
                    {
                        sal_uInt16 nId = ((PopupMenu*)pMenu)->Execute(pEditWin, aPixPos);
                        OUString aCommand = ((PopupMenu*)pMenu)->GetItemCommand(nId);
                        if (aCommand.isEmpty() )
                        {
                            if(!ExecuteMenuCommand( *dynamic_cast<PopupMenu*>(pMenu), *GetViewFrame(), nId ))
                                pPopup->Execute(nId);
                        }
                        else
                        {
                            SfxViewFrame *pSfxViewFrame = GetViewFrame();
                            uno::Reference< frame::XFrame > xFrame;
                            if ( pSfxViewFrame )
                                xFrame = pSfxViewFrame->GetFrame().GetFrameInterface();
                            com::sun::star::util::URL aURL;
                            uno::Reference< frame::XDispatchProvider > xDispatchProvider( xFrame, UNO_QUERY );

                            try
                            {
                                uno::Reference< frame::XDispatch > xDispatch;
                                uno::Reference< util::XURLTransformer > xURLTransformer = util::URLTransformer::create(comphelper::getProcessComponentContext());

                                aURL.Complete = aCommand;
                                xURLTransformer->parseStrict(aURL);
                                uno::Sequence< beans::PropertyValue > aArgs;
                                xDispatch = xDispatchProvider->queryDispatch( aURL, rtl::OUString(), 0 );


                                if (xDispatch.is())
                                {
                                    // Execute dispatch asynchronously
                                    ExecuteInfo* pExecuteInfo   = new ExecuteInfo;
                                    pExecuteInfo->xDispatch     = xDispatch;
                                    pExecuteInfo->aTargetURL    = aURL;
                                    pExecuteInfo->aArgs         = aArgs;
                                    Application::PostUserEvent( STATIC_LINK(0, AsyncExecute , ExecuteHdl_Impl), pExecuteInfo );
                                }
                            }
                            catch (const Exception&)
                            {
                            }
                        }
                    }
                    else
                    {
                        pPopup->Execute( aToFill.SVRect(), pEditWin );
                    }
                }
            }

            pWrtShell->Pop( sal_False );
            pWrtShell->LockView( bOldViewLock );
        }
    }
    return bRet;
}

/** Function: ExecSmartTagPopup

   This function shows the popup menu for smarttag
   actions.
*/
sal_Bool SwView::ExecSmartTagPopup( const Point& rPt )
{
    sal_Bool bRet = sal_False;
    const sal_Bool bOldViewLock = pWrtShell->IsViewLocked();
    pWrtShell->LockView( sal_True );
    pWrtShell->Push();


    // get word that was clicked on
    // This data structure maps a smart tag type string to the property bag
    SwRect aToFill;
    Sequence< rtl::OUString > aSmartTagTypes;
    Sequence< Reference< container::XStringKeyMap > > aStringKeyMaps;
    Reference<text::XTextRange> xRange;

    pWrtShell->GetSmartTagTerm( rPt, aToFill, aSmartTagTypes, aStringKeyMaps, xRange);
    if ( xRange.is() && aSmartTagTypes.getLength() )
    {
        bRet = sal_True;
        pWrtShell->SttSelect();
        SwSmartTagPopup aPopup( this, aSmartTagTypes, aStringKeyMaps, xRange );
        aPopup.Execute( aToFill.SVRect(), pEditWin );
    }

    pWrtShell->Pop( sal_False );
    pWrtShell->LockView( bOldViewLock );

    return bRet;
}

class SwFieldDialog : public FloatingWindow
{
private:
    ListBox aListBox;
    IFieldmark *pFieldmark;

    DECL_LINK( MyListBoxHandler, ListBox * );

public:
    SwFieldDialog( SwEditWin* parent, IFieldmark *fieldBM );
};

SwFieldDialog::SwFieldDialog( SwEditWin* parent, IFieldmark *fieldBM ) :
    FloatingWindow( parent, WB_BORDER | WB_SYSTEMWINDOW ),
    aListBox(this),
    pFieldmark( fieldBM )
{
    if ( fieldBM != NULL )
    {
        const IFieldmark::parameter_map_t* const pParameters = fieldBM->GetParameters();

        rtl::OUString sListKey = rtl::OUString(  ODF_FORMDROPDOWN_LISTENTRY  );
        IFieldmark::parameter_map_t::const_iterator pListEntries = pParameters->find( sListKey );
        if(pListEntries != pParameters->end())
        {
            Sequence< ::rtl::OUString > vListEntries;
            pListEntries->second >>= vListEntries;
            for( ::rtl::OUString* pCurrent = vListEntries.getArray();
                pCurrent != vListEntries.getArray() + vListEntries.getLength();
                ++pCurrent)
            {
                aListBox.InsertEntry(*pCurrent);
            }
        }

        // Select the current one
        rtl::OUString sResultKey = rtl::OUString( ODF_FORMDROPDOWN_RESULT  );
        IFieldmark::parameter_map_t::const_iterator pResult = pParameters->find( sResultKey );
        if ( pResult != pParameters->end() )
        {
            sal_Int32 nSelection = -1;
            pResult->second >>= nSelection;
            aListBox.SelectEntryPos( nSelection );
        }
    }

    Size lbSize=aListBox.GetOptimalSize(WINDOWSIZE_PREFERRED);
    lbSize.Width()+=50;
    lbSize.Height()+=20;
    aListBox.SetSizePixel(lbSize);
    aListBox.SetSelectHdl( LINK( this, SwFieldDialog, MyListBoxHandler ) );
    aListBox.Show();

    SetSizePixel( lbSize );
}

IMPL_LINK( SwFieldDialog, MyListBoxHandler, ListBox *, pBox )
{
    short res = 0;
    if ( !pBox->IsTravelSelect() )
    {
        sal_Int32 selection = pBox->GetSelectEntryPos();
        if ( selection >= 0 )
        {
            rtl::OUString sKey = rtl::OUString(  ODF_FORMDROPDOWN_RESULT  );
            (*pFieldmark->GetParameters())[ sKey ] = makeAny(selection);
            pFieldmark->Invalidate();
            SwView& rView = ( ( SwEditWin* )GetParent() )->GetView();
            rView.GetDocShell()->SetModified( sal_True );
        }

        EndPopupMode();
        res = 1;
    }
    return res;
}

IMPL_LINK_NOARG(SwView, FieldPopupModeEndHdl)
{
    if ( mpFieldPopup )
    {
        delete mpFieldPopup;
        mpFieldPopup = NULL;
    }
    return 0;
}

void SwView::ExecFieldPopup( const Point& rPt, IFieldmark *fieldBM )
{
    const Point aPixPos = GetEditWin().LogicToPixel( rPt );

    mpFieldPopup = new SwFieldDialog( pEditWin, fieldBM );
    mpFieldPopup->SetPopupModeEndHdl( LINK( this, SwView, FieldPopupModeEndHdl ) );

    Rectangle aRect( pEditWin->OutputToScreenPixel( aPixPos ), Size( 0, 0 ) );
    mpFieldPopup->StartPopupMode( aRect, FLOATWIN_POPUPMODE_DOWN|FLOATWIN_POPUPMODE_GRABFOCUS );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
