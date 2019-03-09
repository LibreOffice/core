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

#include <Window.hxx>
#include <DrawDocShell.hxx>

#include <svx/svxids.hrc>

#include <svx/ofaitem.hxx>
#include <svl/srchitem.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/langtab.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxdlg.hxx>
#include <vcl/abstdlg.hxx>
#include <svx/drawitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/eeitem.hxx>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>

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
        }
    }
    else    // Reset to default
    {
        for(sal_uInt16 n : aLangWhichId_EE)
            pObj->ClearMergedItem( n );
    }
}

static void lcl_setLanguage( const SdDrawDocument *pDoc, const OUString &rLanguage, bool bLanguageNone = false )
{
    LanguageType nLang = SvtLanguageTable::GetLanguageType( rLanguage );

    // Do it for SdDrawDocument->SetLanguage as well?

    sal_uInt16 nPageCount = pDoc->GetPageCount();   // Pick All Pages
    for( sal_uInt16 nPage = 0; nPage < nPageCount; nPage++ )
    {
        const SdrPage *pPage = pDoc->GetPage( nPage );
        const size_t nObjCount = pPage->GetObjCount();
        for( size_t nObj = 0; nObj < nObjCount; ++nObj )
        {
            SdrObject *pObj = pPage->GetObj( nObj );
            if (pObj->GetObjIdentifier() != OBJ_PAGE)
                lcl_setLanguageForObj( pObj, nLang, bLanguageNone );
        }
    }
}

/**
 * Handles SFX-Requests
 */
void DrawDocShell::Execute( SfxRequest& rReq )
{
    if(mpViewShell && SlideShow::IsRunning( mpViewShell->GetViewShellBase() ))
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

                SD_MOD()->SetSearchItem(std::unique_ptr<SvxSearchItem>(static_cast<SvxSearchItem*>(rSearchItem.Clone())));
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
            if( dynamic_cast< FuSearch* >(mxDocShellFunction.get()) )
            {
                // End Search&Replace in all docshells
                SfxObjectShell* pFirstShell = SfxObjectShell::GetFirst();
                SfxObjectShell* pShell = pFirstShell;

                while (pShell)
                {
                    if( dynamic_cast< const DrawDocShell *>( pShell ) !=  nullptr)
                    {
                        static_cast<DrawDocShell*>(pShell)->CancelSearching();
                    }

                    pShell = SfxObjectShell::GetNext(*pShell);

                    if (pShell == pFirstShell)
                    {
                        pShell = nullptr;
                    }
                }

                SetDocShellFunction(nullptr);
                Invalidate();
                rReq.Done();
            }
        }
        break;

        case FID_SEARCH_NOW:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if ( pReqArgs )
            {
                rtl::Reference< FuSearch > xFuSearch( dynamic_cast< FuSearch* >( GetDocShellFunction().get() ) );

                if( !xFuSearch.is() && mpViewShell )
                {
                    ::sd::View* pView = mpViewShell->GetView();
                    SetDocShellFunction( FuSearch::Create( mpViewShell, mpViewShell->GetActiveWindow(), pView, mpDoc, rReq ) );
                    xFuSearch.set( dynamic_cast< FuSearch* >( GetDocShellFunction().get() ) );
                }

                if( xFuSearch.is() )
                {
                    const SvxSearchItem& rSearchItem = pReqArgs->Get(SID_SEARCH_ITEM);

                    SD_MOD()->SetSearchItem(std::unique_ptr<SvxSearchItem>(static_cast<SvxSearchItem*>( rSearchItem.Clone() )));
                    xFuSearch->SearchAndReplace(&rSearchItem);
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
            rReq.SetReturnValue( OfaRefItem<XColorList>( SID_GET_COLORLIST, pList ) );
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
                // open the dialog "Tools/Options/Language Settings - Language"
                if (mpViewShell)
                {
                    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
                    ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateVclDialog( mpViewShell->GetActiveWindow(), SID_LANGUAGE_OPTIONS ));
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
                        const OUString aDocumentLangPrefix("Default_");
                        const OUString aStrNone("LANGUAGE_NONE");
                        const OUString aStrResetLangs("RESET_LANGUAGES");
                        SdDrawDocument* pDoc = mpViewShell->GetDoc();
                        sal_Int32 nPos = -1;
                        if (-1 != (nPos = aNewLangTxt.indexOf( aDocumentLangPrefix )))
                        {
                            aNewLangTxt = aNewLangTxt.replaceAt( nPos, aDocumentLangPrefix.getLength(), "" );
                        }
                        else
                        {
                            break;
                        }
                        if (aNewLangTxt == aStrNone)
                            lcl_setLanguage( pDoc, OUString(), true );
                        else if (aNewLangTxt == aStrResetLangs)
                            lcl_setLanguage( pDoc, OUString() );
                        else
                            lcl_setLanguage( pDoc, aNewLangTxt );

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

void DrawDocShell::SetDocShellFunction( const rtl::Reference<FuPoor>& xFunction )
{
    if( mxDocShellFunction.is() )
        mxDocShellFunction->Dispose();

    mxDocShellFunction = xFunction;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
