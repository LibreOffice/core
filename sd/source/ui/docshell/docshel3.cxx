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

#include "Window.hxx"
#include "DrawDocShell.hxx"

#include "app.hrc"

#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>

#include <svx/ofaitem.hxx>
#include <svx/svxerr.hxx>
#include <svx/dialmgr.hxx>
#include <svl/srchitem.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/langtab.hxx>
#include <svx/srchdlg.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxdlg.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/window.hxx>
#include <svl/style.hxx>
#include <svx/drawitem.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/langitem.hxx>
#include <editeng/eeitem.hxx>
#include <com/sun/star/i18n/TextConversionOption.hpp>

#include "strings.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"

#include "sdmod.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdattr.hxx"
#include "fusearch.hxx"
#include "ViewShell.hxx"
#include "View.hxx"
#include "slideshow.hxx"
#include "fuhhconv.hxx"
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

namespace sd {

static void lcl_setLanguageForObj( SdrObject *pObj, LanguageType nLang, bool bLanguageNone = false )
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
            for(sal_Int32 n = 0; n < 3; n++ )
                pObj->SetMergedItem( SvxLanguageItem( nLang, aLangWhichId_EE[n] ) );
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
        for( sal_Int32 n = 0; n < 3; n++ )
            pObj->ClearMergedItem( aLangWhichId_EE[n] );
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
                const SvxSearchItem* pSearchItem = static_cast<const SvxSearchItem*>( &pReqArgs->Get(SID_SEARCH_ITEM) );

                // would be nice to have an assign operation at SearchItem
                SvxSearchItem* pAppSearchItem = SD_MOD()->GetSearchItem();
                delete pAppSearchItem;
                pAppSearchItem = static_cast<SvxSearchItem*>( pSearchItem->Clone() );
                SD_MOD()->SetSearchItem(pAppSearchItem);
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
                        pShell = NULL;
                    }
                }

                SetDocShellFunction(0);
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
                    const SvxSearchItem* pSearchItem =
                        static_cast<const SvxSearchItem*>( &pReqArgs->Get(SID_SEARCH_ITEM) );

                    // would be nice to have an assign operation at SearchItem
                    SvxSearchItem* pAppSearchItem = SD_MOD()->GetSearchItem();
                    delete pAppSearchItem;
                    pAppSearchItem = static_cast<SvxSearchItem*>( pSearchItem->Clone() );
                    SD_MOD()->SetSearchItem(pAppSearchItem);
                    xFuSearch->SearchAndReplace(pSearchItem);
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
            const SvxColorListItem* pColItem = static_cast<const SvxColorListItem*>( GetItem( SID_COLOR_TABLE ) );
            XColorListRef pList = pColItem->GetColorList();
            rReq.SetReturnValue( OfaRefItem<XColorList>( SID_GET_COLORLIST, pList ) );
        }
        break;

        case SID_VERSION:
        {
            const SdrSwapGraphicsMode nOldSwapMode = mpDoc->GetSwapGraphicsMode();

            mpDoc->SetSwapGraphicsMode( SdrSwapGraphicsMode::TEMP );
            ExecuteSlot( rReq, SfxObjectShell::GetStaticInterface() );
            mpDoc->SetSwapGraphicsMode( nOldSwapMode );
        }
        break;

        case SID_HANGUL_HANJA_CONVERSION:
        {
            if( mpViewShell )
            {
                rtl::Reference<FuPoor> aFunc( FuHangulHanjaConversion::Create( mpViewShell, mpViewShell->GetActiveWindow(), mpViewShell->GetView(), mpDoc, rReq ) );
                static_cast< FuHangulHanjaConversion* >( aFunc.get() )->StartConversion( LANGUAGE_KOREAN, LANGUAGE_KOREAN, NULL, i18n::TextConversionOption::CHARACTER_BY_CHARACTER, true );
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
            SFX_REQUEST_ARG( rReq, pItem, SfxStringItem, SID_LANGUAGE_STATUS , false );
            if (pItem)
                aNewLangTxt = pItem->GetValue();
            if (aNewLangTxt == "*" )
            {
                // open the dialog "Tools/Options/Language Settings - Language"
                SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
                if (pFact && mpViewShell)
                {
                    std::unique_ptr<VclAbstractDialog> pDlg(pFact->CreateVclDialog( mpViewShell->GetActiveWindow(), SID_LANGUAGE_OPTIONS ));
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
                            lcl_setLanguage( mpViewShell->GetDoc(), OUString() );
                        else if (aNewLangTxt == aStrResetLangs)
                            lcl_setLanguage( mpViewShell->GetDoc(), OUString(), true );
                        else
                            lcl_setLanguage( mpViewShell->GetDoc(), aNewLangTxt );
                    }
                }
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
