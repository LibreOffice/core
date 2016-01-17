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

#include <com/sun/star/i18n/TextConversionOption.hpp>

#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <vcl/msgbox.hxx>
#include <svl/style.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/fontitem.hxx>

#include <fuhhconv.hxx>
#include "drawdoc.hxx"
#include "Outliner.hxx"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "Window.hxx"
#include "ViewShellBase.hxx"

#include "sdresid.hxx"
#include "strings.hrc"

class SfxRequest;

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

namespace sd {

class ViewShell;


FuHangulHanjaConversion::FuHangulHanjaConversion (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDocument,
    SfxRequest& rReq )
       : FuPoor(pViewSh, pWin, pView, pDocument, rReq),
    pSdOutliner(nullptr),
    bOwnOutliner(false)
{
    if ( dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr )
    {
        bOwnOutliner = true;
        pSdOutliner = new Outliner( mpDoc, OUTLINERMODE_TEXTOBJECT );
    }
    else if ( dynamic_cast< const OutlineViewShell *>( mpViewShell ) !=  nullptr )
    {
        bOwnOutliner = false;
        pSdOutliner = mpDoc->GetOutliner();
    }

    if (pSdOutliner)
       pSdOutliner->PrepareSpelling();
}

FuHangulHanjaConversion::~FuHangulHanjaConversion()
{
    if (pSdOutliner)
        pSdOutliner->EndConversion();

    if (bOwnOutliner)
        delete pSdOutliner;
}

rtl::Reference<FuPoor> FuHangulHanjaConversion::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuHangulHanjaConversion( pViewSh, pWin, pView, pDoc, rReq ) );
    return xFunc;
}

/**
 * Search and replace
 */
void FuHangulHanjaConversion::StartConversion( sal_Int16 nSourceLanguage, sal_Int16 nTargetLanguage,
        const vcl::Font *pTargetFont, sal_Int32 nOptions, bool bIsInteractive )
{

    mpView->BegUndo(SD_RESSTR(STR_UNDO_HANGULHANJACONVERSION));

    ViewShellBase* pBase = dynamic_cast<ViewShellBase*>( SfxViewShell::Current() );
    if (pBase != nullptr)
        mpViewShell = pBase->GetMainViewShell().get();

    if( mpViewShell )
    {
        if ( pSdOutliner && dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr && !bOwnOutliner )
        {
            pSdOutliner->EndConversion();

            bOwnOutliner = true;
            pSdOutliner = new Outliner( mpDoc, OUTLINERMODE_TEXTOBJECT );
            pSdOutliner->BeginConversion();
        }
        else if ( pSdOutliner && dynamic_cast< const OutlineViewShell *>( mpViewShell ) !=  nullptr && bOwnOutliner )
        {
            pSdOutliner->EndConversion();
            delete pSdOutliner;

            bOwnOutliner = false;
            pSdOutliner = mpDoc->GetOutliner();
            pSdOutliner->BeginConversion();
        }

        if (pSdOutliner)
            pSdOutliner->StartConversion(nSourceLanguage, nTargetLanguage, pTargetFont, nOptions, bIsInteractive );
    }

    // Due to changing between edit mode, notes mode, and handout mode the
    // view has most likely changed.  Get the new one.
    mpViewShell = pBase ? pBase->GetMainViewShell().get() : nullptr;
    if (mpViewShell != nullptr)
    {
        mpView = mpViewShell->GetView();
        mpWindow = mpViewShell->GetActiveWindow();
    }
    else
    {
        mpView = nullptr;
        mpWindow = nullptr;
    }

    if (mpView != nullptr)
        mpView->EndUndo();
}

void FuHangulHanjaConversion::ConvertStyles( sal_Int16 nTargetLanguage, const vcl::Font *pTargetFont )
{
    if( !mpDoc )
        return;

    SfxStyleSheetBasePool* pStyleSheetPool = mpDoc->GetStyleSheetPool();
    if( !pStyleSheetPool )
        return;

    SfxStyleSheetBase* pStyle = pStyleSheetPool->First();
    while( pStyle )
    {
        SfxItemSet& rSet = pStyle->GetItemSet();

        const bool bHasParent = !pStyle->GetParent().isEmpty();

        if( !bHasParent || rSet.GetItemState( EE_CHAR_LANGUAGE_CJK, false ) == SfxItemState::SET )
            rSet.Put( SvxLanguageItem( nTargetLanguage, EE_CHAR_LANGUAGE_CJK ) );

        if( pTargetFont &&
            ( !bHasParent || rSet.GetItemState( EE_CHAR_FONTINFO_CJK, false ) == SfxItemState::SET ) )
        {
            // set new font attribute
            SvxFontItem aFontItem( static_cast<const SvxFontItem&>( rSet.Get( EE_CHAR_FONTINFO_CJK ) ) );
            aFontItem.SetFamilyName(   pTargetFont->GetFamilyName());
            aFontItem.SetFamily(       pTargetFont->GetFamily());
            aFontItem.SetStyleName(    pTargetFont->GetStyleName());
            aFontItem.SetPitch(        pTargetFont->GetPitch());
            aFontItem.SetCharSet(      pTargetFont->GetCharSet());
            rSet.Put( aFontItem );
        }

        pStyle = pStyleSheetPool->Next();
    }

    mpDoc->SetLanguage( EE_CHAR_LANGUAGE_CJK, nTargetLanguage );
}

void FuHangulHanjaConversion::StartChineseConversion()
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
                    xMCF->createInstanceWithContext("com.sun.star.linguistic2.ChineseTranslationDialog"
                        , xContext), UNO_QUERY);
            Reference< lang::XInitialization > xInit( xDialog, UNO_QUERY );
            if( xInit.is() )
            {
                //  initialize dialog
                Reference< awt::XWindow > xDialogParentWindow(nullptr);
                Sequence<Any> aSeq(1);
                Any* pArray = aSeq.getArray();
                PropertyValue aParam;
                aParam.Name = "ParentWindow";
                aParam.Value <<= makeAny(xDialogParentWindow);
                pArray[0] <<= makeAny(aParam);
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
                        catch( Exception& )
                        {
                        }
                    }

                    //execute translation
                    sal_Int16 nSourceLang = bToSimplified ? LANGUAGE_CHINESE_TRADITIONAL : LANGUAGE_CHINESE_SIMPLIFIED;
                    sal_Int16 nTargetLang = bToSimplified ? LANGUAGE_CHINESE_SIMPLIFIED : LANGUAGE_CHINESE_TRADITIONAL;
                    sal_Int32 nOptions    = bUseVariants ? i18n::TextConversionOption::USE_CHARACTER_VARIANTS : 0;
                    if( !bCommonTerms )
                        nOptions = nOptions | i18n::TextConversionOption::CHARACTER_BY_CHARACTER;

                    vcl::Font aTargetFont = OutputDevice::GetDefaultFont(
                                        DefaultFontType::CJK_PRESENTATION,
                                        nTargetLang, GetDefaultFontFlags::OnlyOne );

                    StartConversion( nSourceLang, nTargetLang, &aTargetFont, nOptions, false );
                    ConvertStyles( nTargetLang, &aTargetFont );
                }
            }
            Reference< lang::XComponent > xComponent( xDialog, UNO_QUERY );
            if( xComponent.is() )
                xComponent->dispose();
        }
    }
}
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
