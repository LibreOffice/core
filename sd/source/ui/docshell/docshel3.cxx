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

#include <DrawDocShell.hxx>

#include <svx/svxids.hrc>

#include <svx/ofaitem.hxx>
#include <svl/stritem.hxx>
#include <svl/srchitem.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/langtab.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/abstdlg.hxx>
#include <svx/drawitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <editeng/editeng.hxx>
#include <osl/diagnose.h>

#include <sdmod.hxx>
#include <drawdoc.hxx>
#include <fusearch.hxx>
#include <ViewShell.hxx>
#include <slideshow.hxx>
#include <fuhhconv.hxx>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

namespace sd {

static void lcl_setLanguageForObj( SdrObject *pObj, LanguageType nLang, bool bLanguageNone )
{
    const sal_uInt16 aLangWhichId_EE[3] =
    {
        EE_CHAR_LANGUAGE,
        EE_CHAR_LANGUAGE_CJK,
        EE_CHAR_LANGUAGE_CTL
    };

    if( bLanguageNone )
        nLang = LANGUAGE_NONE;

    if( nLang != LANGUAGE_DONTKNOW )
    {
        if( nLang == LANGUAGE_NONE )
        {
            for(sal_uInt16 n : aLangWhichId_EE)
                pObj->SetMergedItem( SvxLanguageItem( nLang, n ) );
        }
        else
        {
            sal_uInt16 nLangWhichId = 0;
            SvtScriptType nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( nLang );
            switch (nScriptType)
            {
                case SvtScriptType::LATIN :    nLangWhichId = EE_CHAR_LANGUAGE; break;
                case SvtScriptType::ASIAN :    nLangWhichId = EE_CHAR_LANGUAGE_CJK; break;
                case SvtScriptType::COMPLEX :  nLangWhichId = EE_CHAR_LANGUAGE_CTL; break;
                default:
                    OSL_FAIL("unexpected case" );
                    return;
            }
            pObj->SetMergedItem( SvxLanguageItem( nLang, nLangWhichId ) );

            // Reset shape text language to default, so it inherits the shape language set above.
            OutlinerParaObject* pOutliner = pObj->GetOutlinerParaObject();
            if (pOutliner)
            {
                EditTextObject& rEditTextObject
                    = const_cast<EditTextObject&>(pOutliner->GetTextObject());
                for (sal_uInt16 n : aLangWhichId_EE)
                {
                    rEditTextObject.RemoveCharAttribs(n);
                }
            }
        }
    }
    else    // Reset to default
    {
        for(sal_uInt16 n : aLangWhichId_EE)
            pObj->ClearMergedItem( n );
    }
}

static void lcl_setLanguage( const SdDrawDocument *pDoc, std::u16string_view rLanguage, bool bLanguageNone = false )
{
    LanguageType nLang = SvtLanguageTable::GetLanguageType( rLanguage );

    // Do it for SdDrawDocument->SetLanguage as well?

    sal_uInt16 nPageCount = pDoc->GetPageCount();   // Pick All Pages
    for( sal_uInt16 nPage = 0; nPage < nPageCount; nPage++ )
    {
        const SdrPage *pPage = pDoc->GetPage( nPage );
        for (const rtl::Reference<SdrObject>& pObj : *pPage)
            if (pObj->GetObjIdentifier() != SdrObjKind::Page)
                lcl_setLanguageForObj( pObj.get(), nLang, bLanguageNone );
    }
}

/**
 * Handles SFX-Requests
 */
void DrawDocShell::Execute( SfxRequest& rReq )
{
    if(mpViewShell && SlideShow::IsRunning( mpViewShell->GetViewShellBase() ) && !SlideShow::IsInteractiveSlideshow() ) // IASS
    {
        // during a running presentation no slot will be executed
        return;
    }

    switch ( rReq.GetSlot() )
    {
        case SID_SEARCH_ITEM:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if (pReqArgs)
            {
                const SvxSearchItem & rSearchItem = pReqArgs->Get(SID_SEARCH_ITEM);

                SD_MOD()->SetSearchItem(std::unique_ptr<SvxSearchItem>(rSearchItem.Clone()));
            }

            rReq.Done();
        }
        break;

        case FID_SEARCH_ON:
        {
            // no action needed
            rReq.Done();
        }
        break;

        case FID_SEARCH_OFF:
        {
            if (mpViewShell)
            {
                sd::View* pView = mpViewShell->GetView();
                if (pView)
                {
                    auto& rFunctionContext = pView->getSearchContext();
                    rtl::Reference<FuSearch>& xFuSearch(rFunctionContext.getFunctionSearch());

                    if (xFuSearch.is())
                    {
                        // End Search&Replace in all docshells
                        SfxObjectShell* pFirstShell = SfxObjectShell::GetFirst();
                        SfxObjectShell* pShell = pFirstShell;

                        while (pShell)
                        {
                            auto pDrawDocShell = dynamic_cast<DrawDocShell*>(pShell);
                            if (pDrawDocShell)
                                pDrawDocShell->CancelSearching();

                            pShell = SfxObjectShell::GetNext(*pShell);

                            if (pShell == pFirstShell)
                                pShell = nullptr;
                        }

                        rFunctionContext.resetSearchFunction();
                        Invalidate();
                        rReq.Done();
                    }
                }
            }
        }
        break;

        case FID_SEARCH_NOW:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if (pReqArgs && mpViewShell)
            {
                sd::View* pView = mpViewShell->GetView();
                if (pView)
                {
                    rtl::Reference<FuSearch> & xFuSearch = pView->getSearchContext().getFunctionSearch();

                    if (!xFuSearch.is())
                    {
                        xFuSearch = rtl::Reference<FuSearch>(
                            FuSearch::createPtr(mpViewShell,
                                                mpViewShell->GetActiveWindow(),
                                                pView, mpDoc, rReq));

                        pView->getSearchContext().setSearchFunction(xFuSearch);
                    }

                    if (xFuSearch.is())
                    {
                        const SvxSearchItem& rSearchItem = pReqArgs->Get(SID_SEARCH_ITEM);

                        SD_MOD()->SetSearchItem(std::unique_ptr<SvxSearchItem>(rSearchItem.Clone()));
                        xFuSearch->SearchAndReplace(&rSearchItem);
                    }
                }
            }

            rReq.Done();
        }
        break;

        case SID_CLOSEDOC:
        {
            ExecuteSlot(rReq, SfxObjectShell::GetStaticInterface());
        }
        break;

        case SID_GET_COLORLIST:
        {
            const SvxColorListItem* pColItem = GetItem( SID_COLOR_TABLE );
            const XColorListRef& pList = pColItem->GetColorList();
            rReq.SetReturnValue( OfaXColorListItem( SID_GET_COLORLIST, pList ) );
        }
        break;

        case SID_VERSION:
        {
            ExecuteSlot( rReq, SfxObjectShell::GetStaticInterface() );
        }
        break;

        case SID_HANGUL_HANJA_CONVERSION:
        {
            if( mpViewShell )
            {
                rtl::Reference<FuPoor> aFunc( FuHangulHanjaConversion::Create( mpViewShell, mpViewShell->GetActiveWindow(), mpViewShell->GetView(), mpDoc, rReq ) );
                static_cast< FuHangulHanjaConversion* >( aFunc.get() )->StartConversion( LANGUAGE_KOREAN, LANGUAGE_KOREAN, nullptr, i18n::TextConversionOption::CHARACTER_BY_CHARACTER, true );
            }
        }
        break;

        case SID_CHINESE_CONVERSION:
        {
            if( mpViewShell )
            {
                rtl::Reference<FuPoor> aFunc( FuHangulHanjaConversion::Create( mpViewShell, mpViewShell->GetActiveWindow(), mpViewShell->GetView(), mpDoc, rReq ) );
                static_cast< FuHangulHanjaConversion* >( aFunc.get() )->StartChineseConversion();
            }
        }
        break;
        case SID_LANGUAGE_STATUS:
        {
            OUString aNewLangTxt;
            const SfxStringItem* pItem = rReq.GetArg<SfxStringItem>(SID_LANGUAGE_STATUS);
            if (pItem)
                aNewLangTxt = pItem->GetValue();

            if (aNewLangTxt == "*" )
            {
                // open the dialog "Tools/Options/Languages and Locales - General"
                if (mpViewShell)
                {
                    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
                    ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateVclDialog( mpViewShell->GetFrameWeld(), SID_LANGUAGE_OPTIONS ));
                    pDlg->Execute();
                }
            }
            else
            {
                if( mpViewShell )
                {
                    // setting the new language...
                    if (!aNewLangTxt.isEmpty())
                    {
                        static constexpr OUString aSelectionLangPrefix(u"Current_"_ustr);
                        static constexpr OUString aParagraphLangPrefix(u"Paragraph_"_ustr);
                        static constexpr OUString aDocumentLangPrefix(u"Default_"_ustr);

                        bool bSelection = false;
                        bool bParagraph = false;

                        SdDrawDocument* pDoc = mpViewShell->GetDoc();
                        sal_Int32 nPos = -1;
                        if (-1 != (nPos = aNewLangTxt.indexOf( aDocumentLangPrefix )))
                        {
                            aNewLangTxt = aNewLangTxt.replaceAt( nPos, aDocumentLangPrefix.getLength(), u"" );

                            if (aNewLangTxt == "LANGUAGE_NONE")
                                lcl_setLanguage( pDoc, u"", true );
                            else if (aNewLangTxt == "RESET_LANGUAGES")
                                lcl_setLanguage( pDoc, u"" );
                            else
                                lcl_setLanguage( pDoc, aNewLangTxt );
                        }
                        else if (-1 != (nPos = aNewLangTxt.indexOf( aSelectionLangPrefix )))
                        {
                            bSelection = true;
                            aNewLangTxt = aNewLangTxt.replaceAt( nPos, aSelectionLangPrefix.getLength(), u"" );
                        }
                        else if (-1 != (nPos = aNewLangTxt.indexOf( aParagraphLangPrefix )))
                        {
                            bParagraph = true;
                            aNewLangTxt = aNewLangTxt.replaceAt( nPos, aParagraphLangPrefix.getLength(), u"" );
                        }

                        if (bSelection || bParagraph)
                        {
                            SdrView* pSdrView = mpViewShell->GetDrawView();
                            if (!pSdrView)
                                return;

                            EditView& rEditView = pSdrView->GetTextEditOutlinerView()->GetEditView();
                            const LanguageType nLangToUse = SvtLanguageTable::GetLanguageType( aNewLangTxt );
                            SvtScriptType nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( nLangToUse );

                            SfxItemSet aAttrs = rEditView.GetEditEngine()->GetEmptyItemSet();
                            if (nScriptType == SvtScriptType::LATIN)
                                aAttrs.Put( SvxLanguageItem( nLangToUse, EE_CHAR_LANGUAGE ) );
                            if (nScriptType == SvtScriptType::COMPLEX)
                                aAttrs.Put( SvxLanguageItem( nLangToUse, EE_CHAR_LANGUAGE_CTL ) );
                            if (nScriptType == SvtScriptType::ASIAN)
                                aAttrs.Put( SvxLanguageItem( nLangToUse, EE_CHAR_LANGUAGE_CJK ) );
                            ESelection aOldSel;
                            if (bParagraph)
                            {
                                ESelection aSel = rEditView.GetSelection();
                                aOldSel = aSel;
                                aSel.nStartPos = 0;
                                aSel.nEndPos = EE_TEXTPOS_ALL;
                                rEditView.SetSelection( aSel );
                            }

                            rEditView.SetAttribs( aAttrs );
                            if (bParagraph)
                                rEditView.SetSelection( aOldSel );
                        }

                        if ( pDoc->GetOnlineSpell() )
                        {
                            pDoc->StartOnlineSpelling();
                        }
                    }
                }
            }
            Broadcast(SfxHint(SfxHintId::LanguageChanged));
        }
        break;
        case SID_SPELLCHECK_IGNORE_ALL:
        {
            if (!mpViewShell)
                return;
            SdrView* pSdrView = mpViewShell->GetDrawView();
            if (!pSdrView)
                return;

            EditView& rEditView = pSdrView->GetTextEditOutlinerView()->GetEditView();
            OUString sIgnoreText;
            const SfxStringItem* pItem2 = rReq.GetArg<SfxStringItem>(FN_PARAM_1);
            if (pItem2)
                sIgnoreText = pItem2->GetValue();

            if(sIgnoreText == "Spelling")
            {
                ESelection aOldSel = rEditView.GetSelection();
                rEditView.SpellIgnoreWord();
                rEditView.SetSelection( aOldSel );
            }
        }
        break;
        case SID_SPELLCHECK_APPLY_SUGGESTION:
        {
            if (!mpViewShell)
                return;
            SdrView* pSdrView = mpViewShell->GetDrawView();
            if (!pSdrView)
                return;

            EditView& rEditView = pSdrView->GetTextEditOutlinerView()->GetEditView();
            OUString sApplyText;
            const SfxStringItem* pItem2 = rReq.GetArg<SfxStringItem>(FN_PARAM_1);
            if (pItem2)
                sApplyText = pItem2->GetValue();

            static constexpr OUString sSpellingRule(u"Spelling_"_ustr);
            sal_Int32 nPos = 0;
            if(-1 != (nPos = sApplyText.indexOf( sSpellingRule )))
            {
                sApplyText = sApplyText.replaceAt(nPos, sSpellingRule.getLength(), u"");
                rEditView.InsertText( sApplyText );
            }
        }
        break;

        case SID_NOTEBOOKBAR:
        {
            const SfxStringItem* pFile = rReq.GetArg<SfxStringItem>( SID_NOTEBOOKBAR );

            if ( mpViewShell )
            {
                SfxBindings& rBindings( mpViewShell->GetFrame()->GetBindings() );

                if ( sfx2::SfxNotebookBar::IsActive() )
                    sfx2::SfxNotebookBar::ExecMethod( rBindings, pFile ? pFile->GetValue() : "" );
                else
                    sfx2::SfxNotebookBar::CloseMethod( rBindings );
            }
        }
        break;

        default:
        break;
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
