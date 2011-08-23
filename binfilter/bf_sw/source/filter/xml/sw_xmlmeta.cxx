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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <hintids.hxx>

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_ 
#include <com/sun/star/text/XTextDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_ 
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <bf_xmloff/xmlnmspe.hxx>
#endif

#ifndef _XMLOFF_XMLMETAI_HXX
#include <bf_xmloff/xmlmetai.hxx>
#endif

#ifndef _XMLOFF_XMLMETAE_HXX
#include <bf_xmloff/xmlmetae.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <bf_svx/langitem.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX 
#include <bf_xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX 
#include <bf_xmloff/nmspmap.hxx>
#endif

#ifndef _DOCSTAT_HXX
#include "docstat.hxx"
#endif
#ifndef _SWDOCSH_HXX
#include "docsh.hxx"
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif

#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include <bf_xmloff/ProgressBarHelper.hxx>
#endif
#ifndef _XMLEXP_HXX
#include "xmlexp.hxx"
#endif
namespace binfilter {


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::binfilter::xmloff::token;

// ---------------------------------------------------------------------

#if 0
class SwXMLMetaContext_Impl : public SfxXMLMetaContext
{
public:
    SwXMLMetaContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< document::XDocumentInfo > & rInfo ) :
        SfxXMLMetaContext( rImport, nPrfx, rLName, rInfo )
    {}

    virtual void SetDocLanguage( LanguageType eLang );
};

void SwXMLMetaContext_Impl::SetDocLanguage( LanguageType eLang )
{
    SwDoc& rDoc = ((SwXMLImport&)GetImport()).GetDoc();

    rDoc.SetDefault( SvxLanguageItem( eLang, RES_CHRATR_LANGUAGE ) );
}
#endif

// ---------------------------------------------------------------------

SvXMLImportContext *SwXMLImport::CreateMetaContext(
                                       const OUString& rLocalName )
{
    SvXMLImportContext *pContext = 0;

    if( !(IsStylesOnlyMode() || IsInsertMode()) )
    {
        pContext = new SfxXMLMetaContext( *this,
                                    XML_NAMESPACE_OFFICE, rLocalName,
                                    GetModel() );
    }
    
    if( !pContext )
        pContext = new SvXMLImportContext( *this, XML_NAMESPACE_OFFICE,
                                              rLocalName );

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

static __FAR_DATA SvXMLTokenMapEntry aMetaStatAttrTokenMap[] =
{
    { XML_NAMESPACE_META, XML_TABLE_COUNT,      XML_TOK_META_STAT_TABLE	},
    { XML_NAMESPACE_META, XML_IMAGE_COUNT,      XML_TOK_META_STAT_IMAGE	},
    { XML_NAMESPACE_META, XML_OBJECT_COUNT,     XML_TOK_META_STAT_OLE	},
    { XML_NAMESPACE_META, XML_PARAGRAPH_COUNT,  XML_TOK_META_STAT_PARA	},
    { XML_NAMESPACE_META, XML_PAGE_COUNT,       XML_TOK_META_STAT_PAGE	},
    { XML_NAMESPACE_META, XML_WORD_COUNT,       XML_TOK_META_STAT_WORD	},
    { XML_NAMESPACE_META, XML_CHARACTER_COUNT,  XML_TOK_META_STAT_CHAR	},
    XML_TOKEN_MAP_END 
};
void SwXMLImport::SetStatisticAttributes(
        const Reference< xml::sax::XAttributeList > & xAttrList)
{
    if( IsStylesOnlyMode() || IsInsertMode() )
        return;

    SvXMLImport::SetStatisticAttributes(xAttrList);

    Reference<XUnoTunnel> xCrsrTunnel( GetTextImport()->GetCursor(),
                                       UNO_QUERY);
    ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
    OTextCursorHelper *pTxtCrsr = (OTextCursorHelper*)xCrsrTunnel->getSomething(
                                        OTextCursorHelper::getUnoTunnelId() );
    ASSERT( pTxtCrsr, "SwXTextCursor missing" );
    SwDoc *pDoc = pTxtCrsr->GetDoc();
    SwDocStat aDocStat( pDoc->GetDocStat() );

    SvXMLTokenMap aTokenMap( aMetaStatAttrTokenMap );

    sal_uInt32 nTokens = 0;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rValue = xAttrList->getValueByIndex( i );
        sal_Int32 nValue;
        if( !GetMM100UnitConverter().convertNumber( nValue, rValue ) )
            continue;

        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );

        sal_uInt32 nToken = aTokenMap.Get( nPrefix, aLocalName );
        switch( nToken )
        {
        case XML_TOK_META_STAT_TABLE:
            aDocStat.nTbl = (sal_uInt16)nValue;
            break;
        case XML_TOK_META_STAT_IMAGE:
            aDocStat.nGrf = (sal_uInt16)nValue;
            break;
        case XML_TOK_META_STAT_OLE:
            aDocStat.nOLE = (sal_uInt16)nValue;
            break;
        case XML_TOK_META_STAT_PAGE:
            aDocStat.nPage = (sal_uInt32)nValue;
            break;
        case XML_TOK_META_STAT_PARA:
            aDocStat.nPara = (sal_uInt32)nValue;
            break;
        case XML_TOK_META_STAT_WORD:
            aDocStat.nWord = (sal_uInt32)nValue;
            break;
        case XML_TOK_META_STAT_CHAR:
            aDocStat.nChar = (sal_uInt32)nValue;
            break;
        default:
            nToken = 0;
        }
        nTokens |= nToken;
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
        OUStringBuffer aOut(16);
        
        Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
        Reference < XText > xText = xTextDoc->getText();
        Reference<XUnoTunnel> xTextTunnel( xText, UNO_QUERY);
        ASSERT( xTextTunnel.is(), "missing XUnoTunnel for Cursor" );
        if( !xTextTunnel.is() )
            return;

        SwXText *pText = (SwXText *)xTextTunnel->getSomething(
                                            SwXText::getUnoTunnelId() );
        ASSERT( pText, "SwXText missing" );
        if( !pText )
            return;

        SwDocStat aDocStat( pText->GetDoc()->GetDocStat() );
        aOut.append( (sal_Int32)aDocStat.nTbl );
        AddAttribute( XML_NAMESPACE_META, XML_TABLE_COUNT,
                      aOut.makeStringAndClear() );
        aOut.append( (sal_Int32)aDocStat.nGrf );
        AddAttribute( XML_NAMESPACE_META, XML_IMAGE_COUNT,
                      aOut.makeStringAndClear() );
        aOut.append( (sal_Int32)aDocStat.nOLE );
        AddAttribute( XML_NAMESPACE_META, XML_OBJECT_COUNT,
                      aOut.makeStringAndClear() );
        if( aDocStat.nPage )
        {
            aOut.append( (sal_Int32)aDocStat.nPage );
            AddAttribute( XML_NAMESPACE_META, XML_PAGE_COUNT,
                          aOut.makeStringAndClear() );
        }
        aOut.append( (sal_Int32)aDocStat.nPara );
        AddAttribute( XML_NAMESPACE_META, XML_PARAGRAPH_COUNT,
                      aOut.makeStringAndClear() );
        aOut.append( (sal_Int32)aDocStat.nWord );
        AddAttribute( XML_NAMESPACE_META, XML_WORD_COUNT,
                      aOut.makeStringAndClear() );
        aOut.append( (sal_Int32)aDocStat.nChar );
        AddAttribute( XML_NAMESPACE_META, XML_CHARACTER_COUNT,
                      aOut.makeStringAndClear() );
        SvXMLElementExport aElem( *this, XML_NAMESPACE_META,
                                  XML_DOCUMENT_STATISTIC,
                                  sal_True, sal_True );

        if( IsShowProgress() )
        {
            ProgressBarHelper *pProgress = GetProgressBarHelper();
            pProgress->SetValue( pProgress->GetValue() + 2 );
        }
    }
}

}
