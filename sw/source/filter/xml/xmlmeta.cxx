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
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <osl/diagnose.h>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlmetai.hxx>
#include <xmloff/xmlmetae.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <editeng/langitem.hxx>
#include <o3tl/safeint.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include <docstat.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <IDocumentStatistics.hxx>
#include "xmlimp.hxx"
#include "xmlexp.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

uno::Reference<document::XDocumentProperties>
SwXMLImport::GetDocumentProperties() const
{
    if (m_bOrganizerMode || IsStylesOnlyMode() ||
        IsBlockMode() || IsInsertMode())
    {
        return nullptr;
    }
    uno::Reference<document::XDocumentPropertiesSupplier> const xDPS(
        GetModel(), UNO_QUERY_THROW);
    return xDPS->getDocumentProperties();
}

SvXMLImportContext *SwXMLImport::CreateMetaContext(
                                       const sal_Int32 /*nElement*/ )
{
    SvXMLImportContext *pContext = nullptr;

    if (getImportFlags() & SvXMLImportFlags::META)
    {
        uno::Reference<document::XDocumentProperties> const xDocProps(
                GetDocumentProperties());
        pContext = new SvXMLMetaDocumentContext(*this, xDocProps);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( *this );

    return pContext;
}

enum SvXMLTokenMapAttrs
{
    XML_TOK_META_STAT_TABLE = 1,
    XML_TOK_META_STAT_IMAGE = 2,
    XML_TOK_META_STAT_OLE = 4,
    XML_TOK_META_STAT_PAGE = 8,
    XML_TOK_META_STAT_PARA = 16,
    XML_TOK_META_STAT_WORD = 32,
    XML_TOK_META_STAT_CHAR = 64,
    XML_TOK_META_STAT_NON_WHITE_SPACE_CHAR = 128,
    XML_TOK_META_STAT_END=XML_TOK_UNKNOWN
};

struct statistic {
    SvXMLTokenMapAttrs const token;
    const char* name;
    sal_uInt16 SwDocStat::* target16;
    sal_uLong  SwDocStat::* target32; /* or 64, on LP64 platforms */
};

static const struct statistic s_stats [] = {
    { XML_TOK_META_STAT_TABLE, "TableCount",     &SwDocStat::nTable, nullptr  },
    { XML_TOK_META_STAT_IMAGE, "ImageCount",     &SwDocStat::nGrf, nullptr  },
    { XML_TOK_META_STAT_OLE,   "ObjectCount",    &SwDocStat::nOLE, nullptr  },
    { XML_TOK_META_STAT_PAGE,  "PageCount",      nullptr, &SwDocStat::nPage },
    { XML_TOK_META_STAT_PARA,  "ParagraphCount", nullptr, &SwDocStat::nPara },
    { XML_TOK_META_STAT_WORD,  "WordCount",      nullptr, &SwDocStat::nWord },
    { XML_TOK_META_STAT_CHAR,  "CharacterCount", nullptr, &SwDocStat::nChar },
    { XML_TOK_META_STAT_NON_WHITE_SPACE_CHAR,  "NonWhitespaceCharacterCount", nullptr, &SwDocStat::nCharExcludingSpaces },
    { XML_TOK_META_STAT_END,   nullptr,                nullptr, nullptr                 }
};

void SwXMLImport::SetStatistics(
        const Sequence< beans::NamedValue > & i_rStats)
{
    if( IsStylesOnlyMode() || IsInsertMode() )
        return;

    SvXMLImport::SetStatistics(i_rStats);

    SwDoc *pDoc = SwImport::GetDocFromXMLImport( *this );
    SwDocStat aDocStat( pDoc->getIDocumentStatistics().GetDocStat() );

    sal_uInt32 nTokens = 0;

    for (sal_Int32 i = 0; i < i_rStats.getLength(); ++i) {
        for (struct statistic const* pStat = s_stats; pStat->name != nullptr;
                ++pStat) {
            if (i_rStats[i].Name.equalsAscii(pStat->name)) {
                sal_Int32 val = 0;
                if (i_rStats[i].Value >>= val) {
                    if (pStat->target16 != nullptr) {
                        aDocStat.*(pStat->target16)
                            = static_cast<sal_uInt16> (val);
                    } else {
                        aDocStat.*(pStat->target32)
                            = static_cast<sal_uInt32> (val);
                    }
                    nTokens |= pStat->token;
                } else {
                    OSL_FAIL("SwXMLImport::SetStatistics: invalid entry");
                }
            }
        }
    }

    if( nTokens )
        pDoc->getIDocumentStatistics().SetDocStat( aDocStat );

    // set progress bar reference to #paragraphs. If not available,
    // use #pages*10, or guesstimate 250 paragraphs. Additionally
    // guesstimate PROGRESS_BAR_STEPS each for meta+settings, styles,
    // and autostyles.
    bool bSetFallback = true;
    sal_Int32 nProgressReference = sal_Int32(); // silence C4701
    if (nTokens & XML_TOK_META_STAT_PARA)
    {
        nProgressReference = static_cast<sal_Int32>(aDocStat.nPara);
        bSetFallback = false;
    }
    else if (nTokens & XML_TOK_META_STAT_PAGE)
        bSetFallback = o3tl::checked_multiply<sal_Int32>(aDocStat.nPage, 10, nProgressReference);
    if (bSetFallback)
        nProgressReference = 250;
    ProgressBarHelper* pProgress = GetProgressBarHelper();
    pProgress->SetReference( nProgressReference + 3*PROGRESS_BAR_STEP );
    pProgress->SetValue( 0 );
}

void SwXMLExport::ExportMeta_()
{
    SvXMLExport::ExportMeta_();

    if( !m_bBlock && IsShowProgress() )
    {
        ProgressBarHelper *pProgress = GetProgressBarHelper();
        pProgress->SetValue( pProgress->GetValue() + 2 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
