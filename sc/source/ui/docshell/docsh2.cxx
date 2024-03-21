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

#include <rtl/bootstrap.hxx>
#include <osl/file.hxx>
#include <svx/drawitem.hxx>
#include <svl/asiancfg.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <orcusfilters.hxx>
#include <config_folders.h>
#include <comphelper/configuration.hxx>
#include <comphelper/processfactory.hxx>
#include <o3tl/unit_conversion.hxx>

#include <drwlayer.hxx>
#include <stlpool.hxx>
#include <docsh.hxx>
#include <docfunc.hxx>
#include <svx/svxids.hrc>
#include <filter.hxx>
#include <functional>

using namespace com::sun::star;

bool ScDocShell::InitNew( const uno::Reference < embed::XStorage >& xStor )
{
    bool bRet = SfxObjectShell::InitNew( xStor );

    m_pDocument->MakeTable(0);

    //  Additional tables are created by the first View, if bIsEmpty is still sal_True
    if( bRet )
    {
        Size aSize(
            o3tl::convert(STD_COL_WIDTH * OLE_STD_CELLS_X, o3tl::Length::twip, o3tl::Length::mm100),
            o3tl::convert(ScGlobal::nStdRowHeight * OLE_STD_CELLS_Y, o3tl::Length::twip,
                          o3tl::Length::mm100));
        // Also adjust start here
        SetVisAreaOrSize( tools::Rectangle( Point(), aSize ) );
    }

    // InitOptions sets the document languages, must be called before CreateStandardStyles
    InitOptions(false);

    if (ScStyleSheetPool* pStyleSheetPool = m_pDocument->GetStyleSheetPool())
    {
        pStyleSheetPool->CreateStandardStyles();
        m_pDocument->getCellAttributeHelper().UpdateAllStyleSheets(*m_pDocument);

        /* Create styles that are imported through Orcus */

        OUString aURL("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/calc/styles.xml");
        rtl::Bootstrap::expandMacros(aURL);

        OUString aPath;
        osl::FileBase::getSystemPathFromFileURL(aURL, aPath);

        ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
        if (pOrcus)
        {
            pOrcus->importODS_Styles(*m_pDocument, aPath);
            pStyleSheetPool->setAllParaStandard();
        }
    }

    //  SetDocumentModified is not allowed anymore in Load/InitNew!
    InitItems();
    CalcOutputFactor();

    return bRet;
}

void ScDocShell::SetEmpty(bool bSet)
{
    m_bIsEmpty = bSet;
}

void ScDocShell::InitItems()
{
    // Fill AllItemSet for Controller with needed Items:
    // Printer Options are set in GetPrinter when printing
    UpdateFontList();

    ScDrawLayer* pDrawLayer = m_pDocument->GetDrawLayer();
    if (pDrawLayer)
    {
        PutItem( SvxColorListItem  ( pDrawLayer->GetColorList(), SID_COLOR_TABLE ) );
        PutItem( SvxGradientListItem( pDrawLayer->GetGradientList(), SID_GRADIENT_LIST ) );
        PutItem( SvxHatchListItem   ( pDrawLayer->GetHatchList(), SID_HATCH_LIST ) );
        PutItem( SvxBitmapListItem  ( pDrawLayer->GetBitmapList(), SID_BITMAP_LIST ) );
        PutItem( SvxPatternListItem ( pDrawLayer->GetPatternList(), SID_PATTERN_LIST ) );
        PutItem( SvxDashListItem    ( pDrawLayer->GetDashList(), SID_DASH_LIST ) );
        PutItem( SvxLineEndListItem ( pDrawLayer->GetLineEndList(), SID_LINEEND_LIST ) );

        // Other modifications after creation of the DrawLayer
        pDrawLayer->SetNotifyUndoActionHdl( std::bind( &ScDocFunc::NotifyDrawUndo, m_pDocFunc.get(), std::placeholders::_1 ) );
    }
    else if (!comphelper::IsFuzzing())
    {
        //  always use global color table instead of local copy
        PutItem( SvxColorListItem( XColorList::GetStdColorList(), SID_COLOR_TABLE ) );
    }

    if (comphelper::IsFuzzing() ||
        (m_pDocument->GetForbiddenCharacters() && m_pDocument->IsValidAsianCompression() && m_pDocument->IsValidAsianKerning()))
        return;

    //  get settings from SvxAsianConfig
    SvxAsianConfig aAsian;

    if (!m_pDocument->GetForbiddenCharacters())
    {
        // set forbidden characters if necessary
        const uno::Sequence<lang::Locale> aLocales = SvxAsianConfig::GetStartEndCharLocales();
        if (aLocales.hasElements())
        {
            std::shared_ptr<SvxForbiddenCharactersTable> xForbiddenTable(
                SvxForbiddenCharactersTable::makeForbiddenCharactersTable(comphelper::getProcessComponentContext()));

            for (const lang::Locale& rLocale : aLocales)
            {
                i18n::ForbiddenCharacters aForbidden;
                SvxAsianConfig::GetStartEndChars( rLocale, aForbidden.beginLine, aForbidden.endLine );
                LanguageType eLang = LanguageTag::convertToLanguageType(rLocale);

                xForbiddenTable->SetForbiddenCharacters( eLang, aForbidden );
            }

            m_pDocument->SetForbiddenCharacters( xForbiddenTable );
        }
    }

    if ( !m_pDocument->IsValidAsianCompression() )
    {
        // set compression mode from configuration if not already set (e.g. XML import)
        m_pDocument->SetAsianCompression( SvxAsianConfig::GetCharDistanceCompression() );
    }

    if ( !m_pDocument->IsValidAsianKerning() )
    {
        // set asian punctuation kerning from configuration if not already set (e.g. XML import)
        m_pDocument->SetAsianKerning( !SvxAsianConfig::IsKerningWesternTextOnly() );    // reversed
    }
}

void ScDocShell::ResetDrawObjectShell()
{
    ScDrawLayer* pDrawLayer = m_pDocument->GetDrawLayer();
    if (pDrawLayer)
        pDrawLayer->SetObjectShell( nullptr );
}

ScDrawLayer* ScDocShell::MakeDrawLayer()
{
    ScDrawLayer* pDrawLayer = m_pDocument->GetDrawLayer();
    if (!pDrawLayer)
    {
        m_pDocument->InitDrawLayer(this);
        pDrawLayer = m_pDocument->GetDrawLayer();
        InitItems(); // including Undo and Basic
        Broadcast( SfxHint( SfxHintId::ScDrawLayerNew ) );
        if (m_nDocumentLock)
            pDrawLayer->setLock(true);
    }
    return pDrawLayer;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
