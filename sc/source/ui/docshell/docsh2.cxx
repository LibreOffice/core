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
#include <unotools/configmgr.hxx>
#include <comphelper/processfactory.hxx>

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

    m_aDocument.MakeTable(0);

    //  Additional tables are created by the first View, if bIsEmpty is still sal_True
    if( bRet )
    {
        Size aSize( long( STD_COL_WIDTH           * HMM_PER_TWIPS * OLE_STD_CELLS_X ),
                    static_cast<long>( ScGlobal::nStdRowHeight * HMM_PER_TWIPS * OLE_STD_CELLS_Y ) );
        // Also adjust start here
        SetVisAreaOrSize( tools::Rectangle( Point(), aSize ) );
    }

    // InitOptions sets the document languages, must be called before CreateStandardStyles
    InitOptions(false);

    m_aDocument.GetStyleSheetPool()->CreateStandardStyles();
    m_aDocument.UpdStlShtPtrsFrmNms();

    if (!m_bUcalcTest)
    {
        /* Create styles that are imported through Orcus */

        OUString aURL("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/calc/styles.xml");
        rtl::Bootstrap::expandMacros(aURL);

        OUString aPath;
        osl::FileBase::getSystemPathFromFileURL(aURL, aPath);

        ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
        if (pOrcus)
        {
            pOrcus->importODS_Styles(m_aDocument, aPath);
            m_aDocument.GetStyleSheetPool()->setAllStandard();
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

    ScDrawLayer* pDrawLayer = m_aDocument.GetDrawLayer();
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
    else if (!utl::ConfigManager::IsFuzzing())
    {
        //  always use global color table instead of local copy
        PutItem( SvxColorListItem( XColorList::GetStdColorList(), SID_COLOR_TABLE ) );
    }

    if (!utl::ConfigManager::IsFuzzing() &&
        (!m_aDocument.GetForbiddenCharacters() || !m_aDocument.IsValidAsianCompression() || !m_aDocument.IsValidAsianKerning()))
    {
        //  get settings from SvxAsianConfig
        SvxAsianConfig aAsian;

        if (!m_aDocument.GetForbiddenCharacters())
        {
            // set forbidden characters if necessary
            uno::Sequence<lang::Locale> aLocales = aAsian.GetStartEndCharLocales();
            if (aLocales.getLength())
            {
                std::shared_ptr<SvxForbiddenCharactersTable> xForbiddenTable(
                    SvxForbiddenCharactersTable::makeForbiddenCharactersTable(comphelper::getProcessComponentContext()));

                const lang::Locale* pLocales = aLocales.getConstArray();
                for (sal_Int32 i = 0; i < aLocales.getLength(); i++)
                {
                    i18n::ForbiddenCharacters aForbidden;
                    aAsian.GetStartEndChars( pLocales[i], aForbidden.beginLine, aForbidden.endLine );
                    LanguageType eLang = LanguageTag::convertToLanguageType(pLocales[i]);

                    xForbiddenTable->SetForbiddenCharacters( eLang, aForbidden );
                }

                m_aDocument.SetForbiddenCharacters( xForbiddenTable );
            }
        }

        if ( !m_aDocument.IsValidAsianCompression() )
        {
            // set compression mode from configuration if not already set (e.g. XML import)
            m_aDocument.SetAsianCompression( aAsian.GetCharDistanceCompression() );
        }

        if ( !m_aDocument.IsValidAsianKerning() )
        {
            // set asian punctuation kerning from configuration if not already set (e.g. XML import)
            m_aDocument.SetAsianKerning( !aAsian.IsKerningWesternTextOnly() );    // reversed
        }
    }
}

void ScDocShell::ResetDrawObjectShell()
{
    ScDrawLayer* pDrawLayer = m_aDocument.GetDrawLayer();
    if (pDrawLayer)
        pDrawLayer->SetObjectShell( nullptr );
}

ScDrawLayer* ScDocShell::MakeDrawLayer()
{
    ScDrawLayer* pDrawLayer = m_aDocument.GetDrawLayer();
    if (!pDrawLayer)
    {
        m_aDocument.InitDrawLayer(this);
        pDrawLayer = m_aDocument.GetDrawLayer();
        InitItems(); // including Undo and Basic
        Broadcast( SfxHint( SfxHintId::ScDrawLayerNew ) );
        if (m_nDocumentLock)
            pDrawLayer->setLock(true);
    }
    return pDrawLayer;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
