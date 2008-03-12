/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlmeta.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:42:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <hintids.hxx>

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTPROPERTIESSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif

#ifndef _XMLOFF_XMLMETAI_HXX
#include <xmloff/xmlmetai.hxx>
#endif

#ifndef _XMLOFF_XMLMETAE_HXX
#include <xmloff/xmlmetae.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef _DOCSTAT_HXX
#include "docstat.hxx"
#endif
#ifndef _SWDOCSH_HXX
#include "docsh.hxx"
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif

#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLEXP_HXX
#include "xmlexp.hxx"
#endif


using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

// ---------------------------------------------------------------------

SvXMLImportContext *SwXMLImport::CreateMetaContext(
                                       const OUString& rLocalName )
{
    SvXMLImportContext *pContext = 0;

    if( !(IsStylesOnlyMode() || IsInsertMode()) )
    {
        uno::Reference<xml::sax::XDocumentHandler> xDocBuilder(
            mxServiceFactory->createInstance(::rtl::OUString::createFromAscii(
                "com.sun.star.xml.dom.SAXDocumentBuilder")),
                uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            GetModel(), UNO_QUERY_THROW);
        pContext = new SvXMLMetaDocumentContext(*this,
                        XML_NAMESPACE_OFFICE, rLocalName,
                        xDPS->getDocumentProperties(), xDocBuilder);
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
    XML_TOK_META_STAT_END=XML_TOK_UNKNOWN
};

/*
static __FAR_DATA SvXMLTokenMapEntry aMetaStatAttrTokenMap[] =
{
    { XML_NAMESPACE_META, XML_TABLE_COUNT,      XML_TOK_META_STAT_TABLE },
    { XML_NAMESPACE_META, XML_IMAGE_COUNT,      XML_TOK_META_STAT_IMAGE },
    { XML_NAMESPACE_META, XML_OBJECT_COUNT,     XML_TOK_META_STAT_OLE   },
    { XML_NAMESPACE_META, XML_PARAGRAPH_COUNT,  XML_TOK_META_STAT_PARA  },
    { XML_NAMESPACE_META, XML_PAGE_COUNT,       XML_TOK_META_STAT_PAGE  },
    { XML_NAMESPACE_META, XML_WORD_COUNT,       XML_TOK_META_STAT_WORD  },
    { XML_NAMESPACE_META, XML_CHARACTER_COUNT,  XML_TOK_META_STAT_CHAR  },
    XML_TOKEN_MAP_END
};
*/

struct statistic {
    SvXMLTokenMapAttrs token;
    const char* name;
    USHORT SwDocStat::* target16;
    ULONG  SwDocStat::* target32; /* or 64, on LP64 platforms */
};

static const struct statistic s_stats [] = {
    { XML_TOK_META_STAT_TABLE, "TableCount",     &SwDocStat::nTbl, 0  },
    { XML_TOK_META_STAT_IMAGE, "ImageCount",     &SwDocStat::nGrf, 0  },
    { XML_TOK_META_STAT_OLE,   "ObjectCount",    &SwDocStat::nOLE, 0  },
    { XML_TOK_META_STAT_PAGE,  "PageCount",      0, &SwDocStat::nPage },
    { XML_TOK_META_STAT_PARA,  "ParagraphCount", 0, &SwDocStat::nPara },
    { XML_TOK_META_STAT_WORD,  "WordCount",      0, &SwDocStat::nWord },
    { XML_TOK_META_STAT_CHAR,  "CharacterCount", 0, &SwDocStat::nChar },
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
                    DBG_ERROR("SwXMLImport::SetStatistics: invalid entry");
                }
            }
        }
    }

    if( 127 == nTokens )
        aDocStat.bModified = sal_False;
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

