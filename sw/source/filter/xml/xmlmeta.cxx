/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <hintids.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlmetai.hxx>
#include <xmloff/xmlmetae.hxx>
#include <editeng/langitem.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include "docstat.hxx"
#include "docsh.hxx"
#include <doc.hxx>
#include "xmlimp.hxx"
#include "xmlexp.hxx"


using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

// ---------------------------------------------------------------------

uno::Reference<document::XDocumentProperties>
SwXMLImport::GetDocumentProperties() const
{
    if (IsOrganizerMode() || IsStylesOnlyMode() ||
        IsBlockMode() || IsInsertMode())
    {
        return 0;
    }
    uno::Reference<document::XDocumentPropertiesSupplier> const xDPS(
        GetModel(), UNO_QUERY_THROW);
    return xDPS->getDocumentProperties();
}

SvXMLImportContext *SwXMLImport::CreateMetaContext(
                                       const OUString& rLocalName )
{
    SvXMLImportContext *pContext = 0;

    if (getImportFlags() & IMPORT_META)
    {
        uno::Reference<document::XDocumentProperties> const xDocProps(
                GetDocumentProperties());
        pContext = new SvXMLMetaDocumentContext(*this,
                    XML_NAMESPACE_OFFICE, rLocalName, xDocProps);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( *this,
                        XML_NAMESPACE_OFFICE, rLocalName );

    return pContext;
}

// ---------------------------------------------------------------------

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
    SvXMLTokenMapAttrs token;
    const char* name;
    sal_uInt16 SwDocStat::* target16;
    sal_uLong  SwDocStat::* target32; /* or 64, on LP64 platforms */
};

static const struct statistic s_stats [] = {
    { XML_TOK_META_STAT_TABLE, "TableCount",     &SwDocStat::nTbl, 0  },
    { XML_TOK_META_STAT_IMAGE, "ImageCount",     &SwDocStat::nGrf, 0  },
    { XML_TOK_META_STAT_OLE,   "ObjectCount",    &SwDocStat::nOLE, 0  },
    { XML_TOK_META_STAT_PAGE,  "PageCount",      0, &SwDocStat::nPage },
    { XML_TOK_META_STAT_PARA,  "ParagraphCount", 0, &SwDocStat::nPara },
    { XML_TOK_META_STAT_WORD,  "WordCount",      0, &SwDocStat::nWord },
    { XML_TOK_META_STAT_CHAR,  "CharacterCount", 0, &SwDocStat::nChar },
    { XML_TOK_META_STAT_NON_WHITE_SPACE_CHAR,  "NonWhitespaceCharacterCount", 0, &SwDocStat::nCharExcludingSpaces },
    { XML_TOK_META_STAT_END,   0,                0, 0                 }
};

void SwXMLImport::SetStatistics(
        const Sequence< beans::NamedValue > & i_rStats)
{
    if( IsStylesOnlyMode() || IsInsertMode() )
        return;

    SvXMLImport::SetStatistics(i_rStats);

    SwDoc *pDoc = SwImport::GetDocFromXMLImport( *this );
    SwDocStat aDocStat( pDoc->GetDocStat() );

    sal_uInt32 nTokens = 0;

    for (sal_Int32 i = 0; i < i_rStats.getLength(); ++i) {
        for (struct statistic const* pStat = s_stats; pStat->name != 0;
                ++pStat) {
            if (i_rStats[i].Name.equalsAscii(pStat->name)) {
                sal_Int32 val = 0;
                if (i_rStats[i].Value >>= val) {
                    if (pStat->target16 != 0) {
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
        pDoc->SetDocStat( aDocStat );

    // set progress bar reference to #paragraphs. If not available,
    // use #pages*10, or guesstimate 250 paragraphs. Additionally
    // guesstimate PROGRESS_BAR_STEPS each for meta+settings, styles,
    // and autostyles.
    sal_Int32 nProgressReference = 250;
    if( nTokens & XML_TOK_META_STAT_PARA )
        nProgressReference = (sal_Int32)aDocStat.nPara;
    else if ( nTokens & XML_TOK_META_STAT_PAGE )
        nProgressReference = 10 * (sal_Int32)aDocStat.nPage;
    ProgressBarHelper* pProgress = GetProgressBarHelper();
    pProgress->SetReference( nProgressReference + 3*PROGRESS_BAR_STEP );
    pProgress->SetValue( 0 );
}

// ---------------------------------------------------------------------

void SwXMLExport::_ExportMeta()
{
    SvXMLExport::_ExportMeta();

    if( !IsBlockMode() )
    {

        if( IsShowProgress() )
        {
            ProgressBarHelper *pProgress = GetProgressBarHelper();
            pProgress->SetValue( pProgress->GetValue() + 2 );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
