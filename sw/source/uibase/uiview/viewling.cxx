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
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/weld.hxx>
#include <svtools/ehdl.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <svx/dialmgr.hxx>
#include <svx/svxerr.hxx>
#include <svx/svxdlg.hxx>
#include <osl/diagnose.h>
#include <swwait.hxx>
#include <uitool.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <swundo.hxx>
#include <hyp.hxx>
#include <olmenu.hxx>
#include <pam.hxx>
#include <edtwin.hxx>
#include <ndtxt.hxx>
#include <txtfrm.hxx>
#include <cmdid.h>
#include <strings.hrc>
#include <hhcwrp.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/ContextMenuExecuteEvent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/awt/PopupMenuDirection.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <vcl/svapp.hxx>
#include <rtl/ustring.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <svtools/langtab.hxx>

#include <editeng/editerr.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <memory>

using namespace sw::mark;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

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
            StartTextConversion( LANGUAGE_KOREAN, LANGUAGE_KOREAN, nullptr,
                    i18n::TextConversionOption::CHARACTER_BY_CHARACTER, true );
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
                                "com.sun.star.linguistic2.ChineseTranslationDialog", xContext),
                            UNO_QUERY);
                    Reference< lang::XInitialization > xInit( xDialog, UNO_QUERY );
                    if( xInit.is() )
                    {
                        Reference<awt::XWindow> xParentWindow;
                        if (weld::Window* pParentWindow = rReq.GetFrameWeld())
                            xParentWindow = pParentWindow->GetXWindow();
                        //  initialize dialog
                        uno::Sequence<uno::Any> aSeq(comphelper::InitAnyPropertySequence(
                        {
                            {"ParentWindow", uno::Any(xParentWindow)}
                        }));
                        xInit->initialize( aSeq );

                        //execute dialog
                        sal_Int16 nDialogRet = xDialog->execute();
                        if( RET_OK == nDialogRet )
                        {
                            //get some parameters from the dialog
                            bool bToSimplified = true;
                            bool bUseVariants = true;
                            bool bCommonTerms = true;
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
                            LanguageType nSourceLang = bToSimplified ? LANGUAGE_CHINESE_TRADITIONAL : LANGUAGE_CHINESE_SIMPLIFIED;
                            LanguageType nTargetLang = bToSimplified ? LANGUAGE_CHINESE_SIMPLIFIED : LANGUAGE_CHINESE_TRADITIONAL;
                            sal_Int32 nOptions       = bUseVariants ? i18n::TextConversionOption::USE_CHARACTER_VARIANTS : 0;
                            if( !bCommonTerms )
                                nOptions = nOptions | i18n::TextConversionOption::CHARACTER_BY_CHARACTER;

                            vcl::Font aTargetFont = OutputDevice::GetDefaultFont( DefaultFontType::CJK_TEXT,
                                                    nTargetLang, GetDefaultFontFlags::OnlyOne );

                            // disallow formatting, updating the view, ... while
                            // converting the document. (saves time)
                            // Also remember the current view and cursor position for later
                            m_pWrtShell->StartAction();

                            // remember cursor position data for later restoration of the cursor
                            const SwPosition *pPoint = m_pWrtShell->GetCursor()->GetPoint();
                            bool bRestoreCursor = pPoint->nNode.GetNode().IsTextNode();
                            const SwNodeIndex aPointNodeIndex( pPoint->nNode );
                            sal_Int32 nPointIndex = pPoint->nContent.GetIndex();

                            // since this conversion is not interactive the whole converted
                            // document should be undone in a single undo step.
                            m_pWrtShell->StartUndo( SwUndoId::OVERWRITE );

                            StartTextConversion( nSourceLang, nTargetLang, &aTargetFont, nOptions, false );

                            m_pWrtShell->EndUndo( SwUndoId::OVERWRITE );

                            if (bRestoreCursor)
                            {
                                SwTextNode *pTextNode = aPointNodeIndex.GetNode().GetTextNode();
                                // check for unexpected error case
                                OSL_ENSURE(pTextNode && pTextNode->GetText().getLength() >= nPointIndex,
                                    "text missing: corrupted node?" );
                                if (!pTextNode || pTextNode->GetText().getLength() < nPointIndex)
                                    nPointIndex = 0;
                                // restore cursor to its original position
                                m_pWrtShell->GetCursor()->GetPoint()->nContent.Assign( pTextNode, nPointIndex );
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
            OSL_ENSURE(false, "wrong Dispatcher");
            return;
    }
}

// start language specific text conversion

void SwView::StartTextConversion(
        LanguageType nSourceLang,
        LanguageType nTargetLang,
        const vcl::Font *pTargetFont,
        sal_Int32 nOptions,
        bool bIsInteractive )
{
    // do not do text conversion if it is active elsewhere
    if (SwEditShell::HasConvIter())
    {
        return;
    }

    SpellContext();

    const SwViewOption* pVOpt = m_pWrtShell->GetViewOptions();
    const bool bOldIdle = pVOpt->IsIdle();
    pVOpt->SetIdle( false );

    bool bOldIns = m_pWrtShell->IsInsMode();
    m_pWrtShell->SetInsMode();

    const bool bSelection = static_cast<SwCursorShell*>(m_pWrtShell.get())->HasSelection() ||
        m_pWrtShell->GetCursor() != m_pWrtShell->GetCursor()->GetNext();

    const bool  bStart = bSelection || m_pWrtShell->IsStartOfDoc();
    const bool  bOther = !bSelection && !(m_pWrtShell->GetFrameType(nullptr,true) & FrameTypeFlags::BODY);

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
    SpellContext(false);
}

// spellcheck and text conversion related stuff

void SwView::SpellStart( SvxSpellArea eWhich,
        bool bStartDone, bool bEndDone,
        SwConversionArgs *pConvArgs )
{
    Reference< XLinguProperties >  xProp = ::GetLinguPropertySet();
    bool bIsWrapReverse = !pConvArgs && xProp.is() && xProp->getIsWrapReverse();

    SwDocPositions eStart = SwDocPositions::Start;
    SwDocPositions eEnd   = SwDocPositions::End;
    SwDocPositions eCurr  = SwDocPositions::Curr;
    switch ( eWhich )
    {
        case SvxSpellArea::Body:
            if( bIsWrapReverse )
                eCurr = SwDocPositions::End;
            else
                eCurr = SwDocPositions::Start;
            break;
        case SvxSpellArea::BodyEnd:
            if( bIsWrapReverse )
            {
                if( bStartDone )
                    eStart = SwDocPositions::Curr;
                eCurr = SwDocPositions::End;
            }
            else if( bStartDone )
                eCurr = SwDocPositions::Start;
            break;
        case SvxSpellArea::BodyStart:
            if( !bIsWrapReverse )
            {
                if( bEndDone )
                    eEnd = SwDocPositions::Curr;
                eCurr = SwDocPositions::Start;
            }
            else if( bEndDone )
                eCurr = SwDocPositions::End;
            break;
        case SvxSpellArea::Other:
            if( bIsWrapReverse )
            {
                eStart = SwDocPositions::OtherStart;
                eEnd  = SwDocPositions::OtherEnd;
                eCurr = SwDocPositions::OtherEnd;
            }
            else
            {
                eStart = SwDocPositions::OtherStart;
                eEnd  = SwDocPositions::OtherEnd;
                eCurr = SwDocPositions::OtherStart;
            }
            break;
        default:
            OSL_ENSURE( false, "SpellStart with unknown Area" );
    }
    m_pWrtShell->SpellStart( eStart, eEnd, eCurr, pConvArgs );
}

// Error message while Spelling

// The passed pointer nlang is itself the value
void SwView::SpellError(LanguageType eLang)
{
    int nPend = 0;

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
    OUString aErr(SvtLanguageTable::GetLanguageString( eLang ) );

    SwEditWin &rEditWin = GetEditWin();
    int nWaitCnt = 0;
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

    if ( nPend )
    {
        while( nPend-- )
            m_pWrtShell->StartAction();
        m_pWrtShell->Combine();
    }
}

// Finish spelling and restore cursor

void SwView::SpellEnd( SwConversionArgs const *pConvArgs )
{
    m_pWrtShell->SpellEnd( pConvArgs );
    if( m_pWrtShell->IsExtMode() )
        m_pWrtShell->SetMark();
}

void SwView::HyphStart( SvxSpellArea eWhich )
{
    switch ( eWhich )
    {
        case SvxSpellArea::Body:
            m_pWrtShell->HyphStart( SwDocPositions::Start, SwDocPositions::End );
            break;
        case SvxSpellArea::BodyEnd:
            m_pWrtShell->HyphStart( SwDocPositions::Curr, SwDocPositions::End );
            break;
        case SvxSpellArea::BodyStart:
            m_pWrtShell->HyphStart( SwDocPositions::Start, SwDocPositions::Curr );
            break;
        case SvxSpellArea::Other:
            m_pWrtShell->HyphStart( SwDocPositions::OtherStart, SwDocPositions::OtherEnd );
            break;
        default:
            OSL_ENSURE( false, "HyphStart with unknown Area" );
    }
}

// Interactive separation

void SwView::HyphenateDocument()
{
    // do not hyphenate if interactive hyphenation is active elsewhere
    if (SwEditShell::HasHyphIter())
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetEditWin().GetFrameWeld(),
            VclMessageType::Warning, VclButtonsType::Ok, SwResId(STR_MULT_INTERACT_HYPH_WARN)));
        xBox->set_title(SwResId(STR_HYPH_TITLE));
        xBox->run();
        return;
    }

    SfxErrorContext aContext( ERRCTX_SVX_LINGU_HYPHENATION, OUString(), m_pEditWin->GetFrameWeld(),
         RID_SVXERRCTX, SvxResLocale() );

    Reference< XHyphenator >  xHyph( ::GetHyphenator() );
    if (!xHyph.is())
    {
        ErrorHandler::HandleError( ERRCODE_SVX_LINGU_LINGUNOTEXISTS );
        return;
    }

    if (m_pWrtShell->GetSelectionType() & (SelectionType::DrawObjectEditMode|SelectionType::DrawObject))
    {
        // Hyphenation in a Draw object
        HyphenateDrawText();
    }
    else
    {
        SwViewOption* pVOpt = const_cast<SwViewOption*>(m_pWrtShell->GetViewOptions());
        bool bOldIdle = pVOpt->IsIdle();
        pVOpt->SetIdle( false );

        Reference< XLinguProperties >  xProp( ::GetLinguPropertySet() );

        m_pWrtShell->StartUndo(SwUndoId::INSATTR);         // valid later

        bool bHyphSpecial = xProp.is() && xProp->getIsHyphSpecial();
        bool bSelection = static_cast<SwCursorShell*>(m_pWrtShell.get())->HasSelection() ||
            m_pWrtShell->GetCursor() != m_pWrtShell->GetCursor()->GetNext();
        bool bOther = m_pWrtShell->HasOtherCnt() && bHyphSpecial && !bSelection;
        bool bStart = bSelection || ( !bOther && m_pWrtShell->IsStartOfDoc() );
        bool bStop = false;
        if( !bOther && !(m_pWrtShell->GetFrameType(nullptr,true) & FrameTypeFlags::BODY) && !bSelection )
        // turned on no special area
        {
            // I want also in special areas hyphenation
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetEditWin().GetFrameWeld(),
                                                      VclMessageType::Question, VclButtonsType::YesNo,
                                                      SwResId(STR_QUERY_SPECIAL_FORCED)));
            if (xBox->run() == RET_YES)
            {
                bOther = true;
                if (xProp.is())
                {
                    xProp->setIsHyphSpecial( true );
                }
            }
            else
                bStop = true; // No hyphenation
        }

        if( !bStop )
        {
            SwHyphWrapper aWrap( this, xHyph, bStart, bOther, bSelection );
            aWrap.SpellDocument();
            m_pWrtShell->EndUndo(SwUndoId::INSATTR);
        }
        pVOpt->SetIdle( bOldIdle );
    }
}

bool SwView::IsValidSelectionForThesaurus() const
{
    // must not be a multi-selection, and if it is a selection it needs
    // to be within a single paragraph

    const bool bMultiSel = m_pWrtShell->GetCursor()->IsMultiSelection();
    const bool bSelection = static_cast<SwCursorShell*>(m_pWrtShell.get())->HasSelection();
    return !bMultiSel && (!bSelection || m_pWrtShell->IsSelOnePara() );
}

OUString SwView::GetThesaurusLookUpText( bool bSelection ) const
{
    return bSelection ? m_pWrtShell->GetSelText() : m_pWrtShell->GetCurWord();
}

void SwView::InsertThesaurusSynonym( const OUString &rSynonmText, const OUString &rLookUpText, bool bSelection )
{
    bool bOldIns = m_pWrtShell->IsInsMode();
    m_pWrtShell->SetInsMode();

    m_pWrtShell->StartAllAction();
    m_pWrtShell->StartUndo(SwUndoId::DELETE);

    if( !bSelection )
    {
        if(m_pWrtShell->IsEndWrd())
            m_pWrtShell->Left(CRSR_SKIP_CELLS, false, 1, false );

        m_pWrtShell->SelWrd();

        // make sure the selection build later from the data below does not
        // include "in word" character to the left and right in order to
        // preserve those. Therefore count those "in words" in order to modify
        // the selection accordingly.
        const sal_Unicode* pChar = rLookUpText.getStr();
        sal_Int32 nLeft = 0;
        while (*pChar++ == CH_TXTATR_INWORD)
            ++nLeft;
        pChar = rLookUpText.getLength() ? rLookUpText.getStr() + rLookUpText.getLength() - 1 : nullptr;
        sal_Int32 nRight = 0;
        while (pChar && *pChar-- == CH_TXTATR_INWORD)
            ++nRight;

        // adjust existing selection
        SwPaM *pCursor = m_pWrtShell->GetCursor();
        pCursor->GetPoint()->nContent -= nRight;
        pCursor->GetMark()->nContent += nLeft;
    }

    m_pWrtShell->Insert( rSynonmText );

    m_pWrtShell->EndUndo(SwUndoId::DELETE);
    m_pWrtShell->EndAllAction();

    m_pWrtShell->SetInsMode( bOldIns );
}

// Start thesaurus

void SwView::StartThesaurus()
{
    if (!IsValidSelectionForThesaurus())
        return;

    SfxErrorContext aContext( ERRCTX_SVX_LINGU_THESAURUS, OUString(), m_pEditWin->GetFrameWeld(),
         RID_SVXERRCTX, SvxResLocale() );

    // Determine language
    LanguageType eLang = m_pWrtShell->GetCurLang();
    if( LANGUAGE_SYSTEM == eLang )
       eLang = GetAppLanguage();

    if( eLang == LANGUAGE_DONTKNOW || eLang == LANGUAGE_NONE )
    {
        SpellError( LANGUAGE_NONE );
        return;
    }

    SwViewOption* pVOpt = const_cast<SwViewOption*>(m_pWrtShell->GetViewOptions());
    const bool bOldIdle = pVOpt->IsIdle();
    pVOpt->SetIdle( false );
    comphelper::ScopeGuard guard([&]() { pVOpt->SetIdle(bOldIdle); }); // restore when leaving scope

    // get initial LookUp text
    const bool bSelection = static_cast<SwCursorShell*>(m_pWrtShell.get())->HasSelection();
    OUString aTmp = GetThesaurusLookUpText( bSelection );

    Reference< XThesaurus >  xThes( ::GetThesaurus() );

    if ( !xThes.is() || !xThes->hasLocale( LanguageTag::convertToLocale( eLang ) ) )
        SpellError( eLang );
    else
    {
        VclPtr<AbstractThesaurusDialog> pDlg;
        // create dialog
        {   //Scope for SwWait-Object
            SwWait aWait( *GetDocShell(), true );
            // load library with dialog only on demand ...
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            pDlg.reset(pFact->CreateThesaurusDialog(GetEditWin().GetFrameWeld(), xThes, aTmp, eLang));
        }

        if (pDlg)
        {
            guard.dismiss(); // ignore, we'll call SetIdle() explicitly after the dialog ends

            pDlg->StartExecuteAsync([aTmp, bSelection, bOldIdle, pDlg, pVOpt, this](sal_Int32 nResult){
                if (nResult == RET_OK )
                    InsertThesaurusSynonym(pDlg->GetWord(), aTmp, bSelection);

                pVOpt->SetIdle(bOldIdle);
                pDlg->disposeOnce();
            });
        }
    }
}

// Offer online suggestions

namespace {

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
    DECL_STATIC_LINK( AsyncExecute, ExecuteHdl_Impl, void*, void );
};

}

IMPL_STATIC_LINK( AsyncExecute, ExecuteHdl_Impl, void*, p, void )
{
    ExecuteInfo* pExecuteInfo = static_cast<ExecuteInfo*>(p);
    SolarMutexReleaser aReleaser;
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

    delete pExecuteInfo;
}
//!! End of extra code for context menu modifying extensions

bool SwView::ExecSpellPopup(const Point& rPt)
{
    bool bRet = false;
    const SwViewOption* pVOpt = m_pWrtShell->GetViewOptions();
    if( pVOpt->IsOnlineSpell() &&
        !m_pWrtShell->IsSelection())
    {
        if (m_pWrtShell->GetSelectionType() & SelectionType::DrawObjectEditMode)
            bRet = ExecDrwTextSpellPopup(rPt);
        else if (!m_pWrtShell->IsSelFrameMode())
        {
            const bool bOldViewLock = m_pWrtShell->IsViewLocked();
            m_pWrtShell->LockView( true );
            m_pWrtShell->Push();
            SwRect aToFill;

            SwCursorShell *pCursorShell = m_pWrtShell.get();
            SwPaM *pCursor = pCursorShell->GetCursor();
            SwPosition aPoint(*pCursor->GetPoint());
            const SwTextNode *pNode = aPoint.nNode.GetNode().GetTextNode();

            // Spell-check in case the idle jobs haven't had a chance to kick in.
            // This makes it possible to suggest spelling corrections for
            // wrong words independent of the spell-checking idle job.
            if (pNode && pNode->IsWrongDirty() &&
                !pCursorShell->IsTableMode() &&
                !pCursor->HasMark() && !pCursor->IsMultiSelection())
            {
                std::pair<Point, bool> const tmp(rPt, false);
                SwContentFrame *const pContentFrame = pCursor->GetContentNode()->getLayoutFrame(
                                        pCursorShell->GetLayout(),
                                        &aPoint, &tmp);
                if (pContentFrame)
                {
                    SwRect aRepaint(static_cast<SwTextFrame*>(pContentFrame)->AutoSpell_(
                        *pCursor->GetContentNode()->GetTextNode(), 0));
                    if (aRepaint.HasArea())
                        m_pWrtShell->InvalidateWindows(aRepaint);
                }
            }

            // decide which variant of the context menu to use...
            // if neither spell checking nor grammar checking provides suggestions use the
            // default context menu.
            bool bUseGrammarContext = false;
            Reference< XSpellAlternatives >  xAlt( m_pWrtShell->GetCorrection(&rPt, aToFill) );
            ProofreadingResult aGrammarCheckRes;
            sal_Int32 nErrorInResult = -1;
            uno::Sequence< OUString > aSuggestions;
            bool bCorrectionRes = false;
            if (!xAlt.is() || !xAlt->getAlternatives().hasElements())
            {
                sal_Int32 nErrorPosInText = -1;
                bCorrectionRes = m_pWrtShell->GetGrammarCorrection( aGrammarCheckRes, nErrorPosInText, nErrorInResult, aSuggestions, &rPt, aToFill );
                OUString aMessageText;
                if (nErrorInResult >= 0)
                    aMessageText = aGrammarCheckRes.aErrors[ nErrorInResult ].aShortComment;
                // we like to use the grammar checking context menu if we either get
                // some suggestions or at least a comment about the error found...
                bUseGrammarContext = bCorrectionRes &&
                        (aSuggestions.hasElements() || !aMessageText.isEmpty());
            }

            // open respective context menu for spell check or grammar errors with correction suggestions...
            if ((!bUseGrammarContext && xAlt.is()) ||
                (bUseGrammarContext && bCorrectionRes && aGrammarCheckRes.aErrors.hasElements()))
            {
                // get paragraph text
                OUString aParaText;
                if (pNode)
                    aParaText = pNode->GetText();    // this may include hidden text but that should be Ok
                else
                {
                    OSL_FAIL("text node expected but not found" );
                }

                bRet = true;
                m_pWrtShell->SttSelect();
                std::unique_ptr<SwSpellPopup> xPopup(bUseGrammarContext ?
                    new SwSpellPopup(m_pWrtShell.get(), aGrammarCheckRes, nErrorInResult, aSuggestions, aParaText) :
                    new SwSpellPopup(m_pWrtShell.get(), xAlt, aParaText));
                ui::ContextMenuExecuteEvent aEvent;
                const Point aPixPos = GetEditWin().LogicToPixel( rPt );

                aEvent.SourceWindow = VCLUnoHelper::GetInterface( m_pEditWin );
                aEvent.ExecutePosition.X = aPixPos.X();
                aEvent.ExecutePosition.Y = aPixPos.Y();
                css::uno::Reference<css::awt::XPopupMenu> xMenu;

                OUString sMenuName = bUseGrammarContext ?
                    OUString("private:resource/GrammarContextMenu") : OUString("private:resource/SpellContextMenu");
                if (TryContextMenuInterception(xPopup->GetMenu(), sMenuName, xMenu, aEvent))
                {
                    //! happy hacking for context menu modifying extensions of this
                    //! 'custom made' menu... *sigh* (code copied from sfx2 and framework)
                    if (xMenu.is())
                    {
                        css::uno::Reference<css::awt::XWindowPeer> xParent(aEvent.SourceWindow, css::uno::UNO_QUERY);
                        const sal_uInt16 nId = xMenu->execute(xParent, css::awt::Rectangle(aPixPos.X(), aPixPos.Y(), 1, 1),
                                                              css::awt::PopupMenuDirection::EXECUTE_DOWN);
                        OUString aCommand = xMenu->getCommand(nId);
                        if (aCommand.isEmpty() )
                        {
                            if (!ExecuteMenuCommand(xMenu, *GetViewFrame(), nId))
                                xPopup->Execute(nId);
                        }
                        else
                        {
                            SfxViewFrame *pSfxViewFrame = GetViewFrame();
                            uno::Reference< frame::XFrame > xFrame;
                            if ( pSfxViewFrame )
                                xFrame = pSfxViewFrame->GetFrame().GetFrameInterface();
                            css::util::URL aURL;
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
                                    Application::PostUserEvent( LINK(nullptr, AsyncExecute , ExecuteHdl_Impl), pExecuteInfo );
                                }
                            }
                            catch (const Exception&)
                            {
                            }
                        }
                    }
                    else
                    {
                        if (comphelper::LibreOfficeKit::isActive())
                        {
                            if (SfxViewShell* pViewShell = SfxViewShell::Current())
                            {
                                boost::property_tree::ptree aMenu = SfxDispatcher::fillPopupMenu(&xPopup->GetMenu());
                                boost::property_tree::ptree aRoot;
                                aRoot.add_child("menu", aMenu);

                                std::stringstream aStream;
                                boost::property_tree::write_json(aStream, aRoot, true);
                                pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CONTEXT_MENU, aStream.str().c_str());
                            }
                        }
                        else
                        {
                            xPopup->Execute(aToFill.SVRect(), m_pEditWin);
                        }
                    }
                }
            }

            if (!comphelper::LibreOfficeKit::isActive())
                m_pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
            m_pWrtShell->LockView( bOldViewLock );
        }
    }
    return bRet;
}

/** Function: ExecSmartTagPopup

   This function shows the popup menu for smarttag
   actions.
*/
void SwView::ExecSmartTagPopup( const Point& rPt )
{
    const bool bOldViewLock = m_pWrtShell->IsViewLocked();
    m_pWrtShell->LockView( true );
    m_pWrtShell->Push();

    css::uno::Sequence< css::uno::Any > aArgs{
        css::uno::Any(comphelper::makePropertyValue( "Frame", GetDispatcher().GetFrame()->GetFrame().GetFrameInterface() )),
        css::uno::Any(comphelper::makePropertyValue( "CommandURL", OUString( ".uno:OpenSmartTagMenuOnCursor" ) ))
    };

    css::uno::Reference< css::uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    css::uno::Reference< css::frame::XPopupMenuController > xPopupController(
        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
        "com.sun.star.comp.svx.SmartTagMenuController", aArgs, xContext ), css::uno::UNO_QUERY );

    css::uno::Reference< css::awt::XPopupMenu > xPopupMenu( xContext->getServiceManager()->createInstanceWithContext(
        "com.sun.star.awt.PopupMenu", xContext ), css::uno::UNO_QUERY );

    if ( xPopupController.is() && xPopupMenu.is() )
    {
        xPopupController->setPopupMenu( xPopupMenu );

        SwRect aToFill;
        m_pWrtShell->GetSmartTagRect( rPt, aToFill );
        m_pWrtShell->SttSelect();

        if ( aToFill.HasArea() )
            xPopupMenu->execute( m_pEditWin->GetComponentInterface(),
                                 VCLUnoHelper::ConvertToAWTRect( m_pEditWin->LogicToPixel( aToFill.SVRect() ) ), css::awt::PopupMenuDirection::EXECUTE_DOWN );

        css::uno::Reference< css::lang::XComponent > xComponent( xPopupController, css::uno::UNO_QUERY );
        if ( xComponent.is() )
            xComponent->dispose();
    }

    m_pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    m_pWrtShell->LockView( bOldViewLock );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
