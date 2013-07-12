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
#include <initui.hxx>               // for SpellPointer
#include <uitool.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <basesh.hxx>
#include <docsh.hxx>                // CheckSpellChanges
#include <viewopt.hxx>              // Viewoptions
#include <swundo.hxx>               // for Undo-Ids
#include <hyp.hxx>                  // hyphenation
#include <olmenu.hxx>               // PopupMenu for OnlineSpelling
#include <pam.hxx>                  // Spelling: Multiselection
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
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::smarttags;

// Lingu-Dispatcher

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
                                OUString("com.sun.star.linguistic2.ChineseTranslationDialog")
                                , xContext), UNO_QUERY);
                    Reference< lang::XInitialization > xInit( xDialog, UNO_QUERY );
                    if( xInit.is() )
                    {
                        //  initialize dialog
                        Reference< awt::XWindow > xDialogParentWindow(0);
                        Sequence<Any> aSeq(1);
                        Any* pArray = aSeq.getArray();
                        PropertyValue aParam;
                        aParam.Name = OUString("ParentWindow");
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
                                    xProp->getPropertyValue( "IsDirectionToSimplified" ) >>= bToSimplified;
                                    xProp->getPropertyValue( "IsUseCharacterVariants" ) >>= bUseVariants;
                                    xProp->getPropertyValue( "IsTranslateCommonTerms" ) >>= bCommonTerms;
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
                            m_pWrtShell->StartAction();

                            // remember cursor position data for later restoration of the cursor
                            const SwPosition *pPoint = m_pWrtShell->GetCrsr()->GetPoint();
                            sal_Bool bRestoreCursor = pPoint->nNode.GetNode().IsTxtNode();
                            const SwNodeIndex aPointNodeIndex( pPoint->nNode );
                            xub_StrLen nPointIndex = pPoint->nContent.GetIndex();

                            // since this conversion is not interactive the whole converted
                            // document should be undone in a single undo step.
                            m_pWrtShell->StartUndo( UNDO_OVERWRITE );

                            StartTextConversion( nSourceLang, nTargetLang, &aTargetFont, nOptions, sal_False );

                            m_pWrtShell->EndUndo( UNDO_OVERWRITE );

                            if (bRestoreCursor)
                            {
                                SwTxtNode *pTxtNode = aPointNodeIndex.GetNode().GetTxtNode();
                                // check for unexpected error case
                                OSL_ENSURE(pTxtNode && pTxtNode->GetTxt().getLength() >= nPointIndex,
                                    "text missing: corrupted node?" );
                                if (!pTxtNode || pTxtNode->GetTxt().getLength() < nPointIndex)
                                    nPointIndex = 0;
                                // restore cursor to its original position
                                m_pWrtShell->GetCrsr()->GetPoint()->nContent.Assign( pTxtNode, nPointIndex );
                            }

                            // enable all, restore view and cursor position
                            m_pWrtShell->EndAction();
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

// start language specific text conversion

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

    const SwViewOption* pVOpt = m_pWrtShell->GetViewOptions();
    const sal_Bool bOldIdle = pVOpt->IsIdle();
    pVOpt->SetIdle( sal_False );

    sal_Bool bOldIns = m_pWrtShell->IsInsMode();
    m_pWrtShell->SetInsMode( sal_True );

    const bool bSelection = ((SwCrsrShell*)m_pWrtShell)->HasSelection() ||
        m_pWrtShell->GetCrsr() != m_pWrtShell->GetCrsr()->GetNext();

    const bool  bStart = bSelection || m_pWrtShell->IsStartOfDoc();
    const bool  bOther = !bSelection && !(m_pWrtShell->GetFrmType(0,sal_True) & FRMTYPE_BODY);

    {
        const uno::Reference< uno::XComponentContext > xContext(
                    comphelper::getProcessComponentContext() );
        SwHHCWrapper aWrap( this, xContext, nSourceLang, nTargetLang, pTargetFont,
                            nOptions, bIsInteractive,
                            bStart, bOther, bSelection );
        aWrap.Convert();
    }

    m_pWrtShell->SetInsMode( bOldIns );
    pVOpt->SetIdle( bOldIdle );
    SpellKontext(sal_False);
}

// spellcheck and text conversion related stuff

void SwView::SpellStart( SvxSpellArea eWhich,
        bool bStartDone, bool bEndDone,
        SwConversionArgs *pConvArgs )
{
    Reference< XLinguProperties >  xProp = ::GetLinguPropertySet();
    sal_Bool bIsWrapReverse = (!pConvArgs && xProp.is()) ? xProp->getIsWrapReverse() : sal_False;

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
    m_pWrtShell->SpellStart( eStart, eEnde, eCurr, pConvArgs );
}

// Error message while Spelling

// The passed pointer nlang is itself the value
void SwView::SpellError(LanguageType eLang)
{
#if OSL_DEBUG_LEVEL > 1
    sal_Bool bFocus = GetEditWin().HasFocus();
#endif
    sal_uInt16 nPend = 0;

    if ( m_pWrtShell->ActionPend() )
    {
        m_pWrtShell->Push();
        m_pWrtShell->ClearMark();
        do
        {
            m_pWrtShell->EndAction();
            ++nPend;
        }
        while( m_pWrtShell->ActionPend() );
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
            m_pWrtShell->StartAction();
        m_pWrtShell->Combine();
    }
#if OSL_DEBUG_LEVEL > 1
    if( !bFocus )
        GetEditWin().GrabFocus();
#endif

}

// Finish spelling and restore cursor

void SwView::SpellEnd( SwConversionArgs *pConvArgs )
{
    m_pWrtShell->SpellEnd( pConvArgs );
    if( m_pWrtShell->IsExtMode() )
        m_pWrtShell->SetMark();
}

void SwView::HyphStart( SvxSpellArea eWhich )
{
    switch ( eWhich )
    {
        case SVX_SPELL_BODY:
            m_pWrtShell->HyphStart( DOCPOS_START, DOCPOS_END );
            break;
        case SVX_SPELL_BODY_END:
            m_pWrtShell->HyphStart( DOCPOS_CURR, DOCPOS_END );
            break;
        case SVX_SPELL_BODY_START:
            m_pWrtShell->HyphStart( DOCPOS_START, DOCPOS_CURR );
            break;
        case SVX_SPELL_OTHER:
            m_pWrtShell->HyphStart( DOCPOS_OTHERSTART, DOCPOS_OTHEREND );
            break;
        default:
            OSL_ENSURE( !this, "HyphStart with unknown Area" );
    }
}

// Interactive separation

void SwView::HyphenateDocument()
{
    // do not hyphenate if interactive hyphenation is active elsewhere
    if (GetWrtShell().HasHyphIter())
    {
        MessBox( 0, WB_OK, String( SW_RES( STR_HYPH_TITLE ) ),
                String( SW_RES( STR_MULT_INTERACT_HYPH_WARN ) ) ).Execute();
        return;
    }

    SfxErrorContext aContext( ERRCTX_SVX_LINGU_HYPHENATION, aEmptyStr, m_pEditWin,
         RID_SVXERRCTX, &DIALOG_MGR() );

    Reference< XHyphenator >  xHyph( ::GetHyphenator() );
    if (!xHyph.is())
    {
        ErrorHandler::HandleError( ERRCODE_SVX_LINGU_LINGUNOTEXISTS );
        return;
    }

    if (m_pWrtShell->GetSelectionType() & (nsSelectionType::SEL_DRW_TXT|nsSelectionType::SEL_DRW))
    {
        // Hyphenation in a Draw object
        HyphenateDrawText();
    }
    else
    {
        SwViewOption* pVOpt = (SwViewOption*)m_pWrtShell->GetViewOptions();
        sal_Bool bOldIdle = pVOpt->IsIdle();
        pVOpt->SetIdle( sal_False );

        Reference< XLinguProperties >  xProp( ::GetLinguPropertySet() );


        m_pWrtShell->StartUndo(UNDO_INSATTR);         // valid later

        sal_Bool bHyphSpecial = xProp.is() ? xProp->getIsHyphSpecial() : sal_False;
        sal_Bool bSelection = ((SwCrsrShell*)m_pWrtShell)->HasSelection() ||
            m_pWrtShell->GetCrsr() != m_pWrtShell->GetCrsr()->GetNext();
        sal_Bool bOther = m_pWrtShell->HasOtherCnt() && bHyphSpecial && !bSelection;
        sal_Bool bStart = bSelection || ( !bOther && m_pWrtShell->IsStartOfDoc() );
        bool bStop = false;
        if( !bOther && !(m_pWrtShell->GetFrmType(0,sal_True) & FRMTYPE_BODY) && !bSelection )
        // turned on no special area
        {
            // I want also in special areas hyphenation
            QueryBox aBox( &GetEditWin(), SW_RES( DLG_SPECIAL_FORCED ) );
            if( aBox.Execute() == RET_YES )
            {
                bOther = sal_True;
                if (xProp.is())
                {
                    xProp->setIsHyphSpecial( sal_True );
                }
            }
            else
                bStop = true; // No hyphenation
        }

        if( !bStop )
        {
            SwHyphWrapper aWrap( this, xHyph, bStart, bOther, bSelection );
            aWrap.SpellDocument();
            m_pWrtShell->EndUndo(UNDO_INSATTR);
        }
        pVOpt->SetIdle( bOldIdle );
    }
}

bool SwView::IsValidSelectionForThesaurus() const
{
    // must not be a multi-selection, and if it is a selection it needs
    // to be within a single paragraph

    const bool bMultiSel = m_pWrtShell->GetCrsr() != m_pWrtShell->GetCrsr()->GetNext();
    const sal_Bool bSelection = ((SwCrsrShell*)m_pWrtShell)->HasSelection();
    return !bMultiSel && (!bSelection || m_pWrtShell->IsSelOnePara() );
}

String SwView::GetThesaurusLookUpText( bool bSelection ) const
{
    return bSelection ? m_pWrtShell->GetSelTxt() : m_pWrtShell->GetCurWord();
}

void SwView::InsertThesaurusSynonym( const String &rSynonmText, const String &rLookUpText, bool bSelection )
{
    sal_Bool bOldIns = m_pWrtShell->IsInsMode();
    m_pWrtShell->SetInsMode( sal_True );

    m_pWrtShell->StartAllAction();
    m_pWrtShell->StartUndo(UNDO_DELETE);

    if( !bSelection )
    {
        if(m_pWrtShell->IsEndWrd())
            m_pWrtShell->Left(CRSR_SKIP_CELLS, sal_False, 1, sal_False );

        m_pWrtShell->SelWrd();

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
        SwPaM *pCrsr = m_pWrtShell->GetCrsr();
        pCrsr->GetPoint()->nContent -= nRight;
        pCrsr->GetMark()->nContent += nLeft;
    }

    m_pWrtShell->Insert( rSynonmText );

    m_pWrtShell->EndUndo(UNDO_DELETE);
    m_pWrtShell->EndAllAction();

    m_pWrtShell->SetInsMode( bOldIns );
}

// Start thesaurus

void SwView::StartThesaurus()
{
    if (!IsValidSelectionForThesaurus())
        return;

    SfxErrorContext aContext( ERRCTX_SVX_LINGU_THESAURUS, aEmptyStr, m_pEditWin,
         RID_SVXERRCTX, &DIALOG_MGR() );

    // Determine language
    LanguageType eLang = m_pWrtShell->GetCurLang();
    if( LANGUAGE_SYSTEM == eLang )
       eLang = GetAppLanguage();

    if( eLang == LANGUAGE_DONTKNOW || eLang == LANGUAGE_NONE )
    {
        SpellError( LANGUAGE_NONE );
        return;
    }

    SwViewOption* pVOpt = (SwViewOption*)m_pWrtShell->GetViewOptions();
    sal_Bool bOldIdle = pVOpt->IsIdle();
    pVOpt->SetIdle( sal_False );

    // get initial LookUp text
    const sal_Bool bSelection = ((SwCrsrShell*)m_pWrtShell)->HasSelection();
    String aTmp = GetThesaurusLookUpText( bSelection );

    Reference< XThesaurus >  xThes( ::GetThesaurus() );
    AbstractThesaurusDialog *pDlg = NULL;

    if ( !xThes.is() || !xThes->hasLocale( LanguageTag::convertToLocale( eLang ) ) )
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

// Offer online suggestions

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
    const SwViewOption* pVOpt = m_pWrtShell->GetViewOptions();
    if( pVOpt->IsOnlineSpell() &&
        !m_pWrtShell->IsSelection())
    {
        if (m_pWrtShell->GetSelectionType() & nsSelectionType::SEL_DRW_TXT)
            bRet = ExecDrwTxtSpellPopup(rPt);
        else if (!m_pWrtShell->IsSelFrmMode())
        {
            const sal_Bool bOldViewLock = m_pWrtShell->IsViewLocked();
            m_pWrtShell->LockView( sal_True );
            m_pWrtShell->Push();
            SwRect aToFill;

            // decide which variant of the context menu to use...
            // if neither spell checking nor grammar checking provides suggestions use the
            // default context menu.
            bool bUseGrammarContext = false;
            Reference< XSpellAlternatives >  xAlt( m_pWrtShell->GetCorrection(&rPt, aToFill) );
            ProofreadingResult aGrammarCheckRes;
            sal_Int32 nErrorInResult = -1;
            uno::Sequence< OUString > aSuggestions;
            bool bCorrectionRes = false;
            if (!xAlt.is() || xAlt->getAlternatives().getLength() == 0)
            {
                sal_Int32 nErrorPosInText = -1;
                bCorrectionRes = m_pWrtShell->GetGrammarCorrection( aGrammarCheckRes, nErrorPosInText, nErrorInResult, aSuggestions, &rPt, aToFill );
                OUString aMessageText;
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
                SwPosition aPoint( *m_pWrtShell->GetCrsr()->GetPoint() );
                const SwTxtNode *pNode = dynamic_cast< const SwTxtNode * >(
                                            &aPoint.nNode.GetNode() );
                if (pNode)
                    aParaText = pNode->GetTxt();    // this may include hidden text but that should be Ok
                else
                {
                    OSL_FAIL("text node expected but not found" );
                }

                bRet = sal_True;
                m_pWrtShell->SttSelect();
                std::auto_ptr< SwSpellPopup > pPopup;
                if (bUseGrammarContext)
                {
                    sal_Int32 nPos = aPoint.nContent.GetIndex();
                    (void) nPos;
                    pPopup = std::auto_ptr< SwSpellPopup >(new SwSpellPopup( m_pWrtShell, aGrammarCheckRes, nErrorInResult, aSuggestions, aParaText ));
                }
                else
                    pPopup = std::auto_ptr< SwSpellPopup >(new SwSpellPopup( m_pWrtShell, xAlt, aParaText ));
                ui::ContextMenuExecuteEvent aEvent;
                const Point aPixPos = GetEditWin().LogicToPixel( rPt );

                aEvent.SourceWindow = VCLUnoHelper::GetInterface( m_pEditWin );
                aEvent.ExecutePosition.X = aPixPos.X();
                aEvent.ExecutePosition.Y = aPixPos.Y();
                Menu* pMenu = 0;

                OUString sMenuName  = bUseGrammarContext ?
                    OUString("private:resource/GrammarContextMenu") : OUString("private:resource/SpellContextMenu");
                if(TryContextMenuInterception( *pPopup, sMenuName, pMenu, aEvent ))
                {

                    //! happy hacking for context menu modifying extensions of this
                    //! 'custom made' menu... *sigh* (code copied from sfx2 and framework)
                    if ( pMenu )
                    {
                        sal_uInt16 nId = ((PopupMenu*)pMenu)->Execute(m_pEditWin, aPixPos);
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
                                xDispatch = xDispatchProvider->queryDispatch( aURL, OUString(), 0 );


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
                        pPopup->Execute( aToFill.SVRect(), m_pEditWin );
                    }
                }
            }

            m_pWrtShell->Pop( sal_False );
            m_pWrtShell->LockView( bOldViewLock );
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
    const sal_Bool bOldViewLock = m_pWrtShell->IsViewLocked();
    m_pWrtShell->LockView( sal_True );
    m_pWrtShell->Push();


    // get word that was clicked on
    // This data structure maps a smart tag type string to the property bag
    SwRect aToFill;
    Sequence< OUString > aSmartTagTypes;
    Sequence< Reference< container::XStringKeyMap > > aStringKeyMaps;
    Reference<text::XTextRange> xRange;

    m_pWrtShell->GetSmartTagTerm( rPt, aToFill, aSmartTagTypes, aStringKeyMaps, xRange);
    if ( xRange.is() && aSmartTagTypes.getLength() )
    {
        bRet = sal_True;
        m_pWrtShell->SttSelect();
        SwSmartTagPopup aPopup( this, aSmartTagTypes, aStringKeyMaps, xRange );
        aPopup.Execute( aToFill.SVRect(), m_pEditWin );
    }

    m_pWrtShell->Pop( sal_False );
    m_pWrtShell->LockView( bOldViewLock );

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

        OUString sListKey = OUString(  ODF_FORMDROPDOWN_LISTENTRY  );
        IFieldmark::parameter_map_t::const_iterator pListEntries = pParameters->find( sListKey );
        if(pListEntries != pParameters->end())
        {
            Sequence< OUString > vListEntries;
            pListEntries->second >>= vListEntries;
            for( OUString* pCurrent = vListEntries.getArray();
                pCurrent != vListEntries.getArray() + vListEntries.getLength();
                ++pCurrent)
            {
                aListBox.InsertEntry(*pCurrent);
            }
        }

        // Select the current one
        OUString sResultKey = OUString( ODF_FORMDROPDOWN_RESULT  );
        IFieldmark::parameter_map_t::const_iterator pResult = pParameters->find( sResultKey );
        if ( pResult != pParameters->end() )
        {
            sal_Int32 nSelection = -1;
            pResult->second >>= nSelection;
            aListBox.SelectEntryPos( nSelection );
        }
    }

    Size lbSize(aListBox.GetOptimalSize());
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
            OUString sKey = OUString(  ODF_FORMDROPDOWN_RESULT  );
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
    if ( m_pFieldPopup )
    {
        delete m_pFieldPopup;
        m_pFieldPopup = NULL;
    }
    return 0;
}

void SwView::ExecFieldPopup( const Point& rPt, IFieldmark *fieldBM )
{
    const Point aPixPos = GetEditWin().LogicToPixel( rPt );

    m_pFieldPopup = new SwFieldDialog( m_pEditWin, fieldBM );
    m_pFieldPopup->SetPopupModeEndHdl( LINK( this, SwView, FieldPopupModeEndHdl ) );

    Rectangle aRect( m_pEditWin->OutputToScreenPixel( aPixPos ), Size( 0, 0 ) );
    m_pFieldPopup->StartPopupMode( aRect, FLOATWIN_POPUPMODE_DOWN|FLOATWIN_POPUPMODE_GRABFOCUS );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
