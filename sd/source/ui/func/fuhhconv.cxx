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
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <svl/style.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/fontitem.hxx>

#include <fuhhconv.hxx>
#include <drawdoc.hxx>
#include <Outliner.hxx>
#include <DrawViewShell.hxx>
#include <OutlineViewShell.hxx>
#include <Window.hxx>
#include <ViewShellBase.hxx>

#include <sdresid.hxx>
#include <strings.hrc>

class SfxRequest;

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

namespace sd {


FuHangulHanjaConversion::FuHangulHanjaConversion (
    ViewShell& rViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument& rDocument,
    SfxRequest& rReq )
       : FuPoor(rViewSh, pWin, pView, rDocument, rReq),
    pSdOutliner(nullptr),
    bOwnOutliner(false)
{
    if ( dynamic_cast< const DrawViewShell *>( &mrViewShell ) !=  nullptr )
    {
        bOwnOutliner = true;
        pSdOutliner = new SdOutliner( mrDoc, OutlinerMode::TextObject );
    }
    else if ( dynamic_cast< const OutlineViewShell *>( &mrViewShell ) !=  nullptr )
    {
        bOwnOutliner = false;
        pSdOutliner = mrDoc.GetOutliner();
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

rtl::Reference<FuPoor> FuHangulHanjaConversion::Create( ViewShell& rViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument& rDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuHangulHanjaConversion( rViewSh, pWin, pView, rDoc, rReq ) );
    return xFunc;
}

/**
 * Search and replace
 */
void FuHangulHanjaConversion::StartConversion( LanguageType nSourceLanguage, LanguageType nTargetLanguage,
        const vcl::Font *pTargetFont, sal_Int32 nOptions, bool bIsInteractive )
{

    mpView->BegUndo(SdResId(STR_UNDO_HANGULHANJACONVERSION));

    ViewShellBase* pBase = dynamic_cast<ViewShellBase*>( SfxViewShell::Current() );
    ViewShell* pViewShell = pBase ? pBase->GetMainViewShell().get() : nullptr;

    if( pViewShell )
    {
        if ( pSdOutliner && dynamic_cast< const DrawViewShell *>( pViewShell ) !=  nullptr && !bOwnOutliner )
        {
            pSdOutliner->EndConversion();

            bOwnOutliner = true;
            pSdOutliner = new SdOutliner( mrDoc, OutlinerMode::TextObject );
            pSdOutliner->BeginConversion();
        }
        else if ( pSdOutliner && dynamic_cast< const OutlineViewShell *>( pViewShell ) !=  nullptr && bOwnOutliner )
        {
            pSdOutliner->EndConversion();
            delete pSdOutliner;

            bOwnOutliner = false;
            pSdOutliner = mrDoc.GetOutliner();
            pSdOutliner->BeginConversion();
        }

        if (pSdOutliner)
            pSdOutliner->StartConversion(nSourceLanguage, nTargetLanguage, pTargetFont, nOptions, bIsInteractive );
    }

    // Due to changing between edit mode, notes mode, and handout mode the
    // view has most likely changed.  Get the new one.
    pViewShell = pBase ? pBase->GetMainViewShell().get() : nullptr;
    if (pViewShell != nullptr)
    {
        mpView = pViewShell->GetView();
        mpWindow = pViewShell->GetActiveWindow();
    }
    else
    {
        mpView = nullptr;
        mpWindow = nullptr;
    }

    if (mpView != nullptr)
        mpView->EndUndo();
}

void FuHangulHanjaConversion::ConvertStyles( LanguageType nTargetLanguage, const vcl::Font *pTargetFont )
{
    SfxStyleSheetBasePool* pStyleSheetPool = mrDoc.GetStyleSheetPool();
    if( !pStyleSheetPool )
        return;

    SfxStyleSheetBase* pStyle = pStyleSheetPool->First(SfxStyleFamily::All);
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
            SvxFontItem aFontItem( rSet.Get( EE_CHAR_FONTINFO_CJK ) );
            aFontItem.SetFamilyName(   pTargetFont->GetFamilyName());
            aFontItem.SetFamily(       pTargetFont->GetFamilyType());
            aFontItem.SetStyleName(    pTargetFont->GetStyleName());
            aFontItem.SetPitch(        pTargetFont->GetPitch());
            aFontItem.SetCharSet(      pTargetFont->GetCharSet());
            rSet.Put( aFontItem );
        }

        pStyle = pStyleSheetPool->Next();
    }

    mrDoc.SetLanguage( nTargetLanguage, EE_CHAR_LANGUAGE_CJK );
}

void FuHangulHanjaConversion::StartChineseConversion()
{
    //open ChineseTranslationDialog
    uno::Reference< uno::XComponentContext > xContext(::comphelper::getProcessComponentContext());
    if(!xContext.is())
        return;

    Reference< lang::XMultiComponentFactory > xMCF( xContext->getServiceManager() );
    if(!xMCF.is())
        return;

    Reference< ui::dialogs::XExecutableDialog > xDialog(
            xMCF->createInstanceWithContext(u"com.sun.star.linguistic2.ChineseTranslationDialog"_ustr
                , xContext), UNO_QUERY);
    Reference< lang::XInitialization > xInit( xDialog, UNO_QUERY );
    if( xInit.is() )
    {
        //  initialize dialog
        Reference< awt::XWindow > xDialogParentWindow;
        Sequence<Any> aSeq(comphelper::InitAnyPropertySequence(
        {
            {"ParentWindow", uno::Any(xDialogParentWindow)}
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
                    xProp->getPropertyValue( u"IsDirectionToSimplified"_ustr ) >>= bToSimplified;
                    xProp->getPropertyValue( u"IsUseCharacterVariants"_ustr ) >>= bUseVariants;
                    xProp->getPropertyValue( u"IsTranslateCommonTerms"_ustr ) >>= bCommonTerms;
                }
                catch( Exception& )
                {
                }
            }

            //execute translation
            LanguageType nSourceLang = bToSimplified ? LANGUAGE_CHINESE_TRADITIONAL : LANGUAGE_CHINESE_SIMPLIFIED;
            LanguageType nTargetLang = bToSimplified ? LANGUAGE_CHINESE_SIMPLIFIED : LANGUAGE_CHINESE_TRADITIONAL;
            sal_Int32 nOptions       = bUseVariants ? i18n::TextConversionOption::USE_CHARACTER_VARIANTS : 0;
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
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
