/*************************************************************************
 *
 *  $RCSfile: xmlmeta.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mib $ $Date: 2000-11-23 14:42:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include <hintids.hxx>

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
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
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include <xmloff/ProgressBarHelper.hxx>
#endif
#ifndef _XMLEXP_HXX
#include "xmlexp.hxx"
#endif


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

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
    { XML_NAMESPACE_META, sXML_table_count, XML_TOK_META_STAT_TABLE },
    { XML_NAMESPACE_META, sXML_image_count, XML_TOK_META_STAT_IMAGE },
    { XML_NAMESPACE_META, sXML_object_count,    XML_TOK_META_STAT_OLE   },
    { XML_NAMESPACE_META, sXML_paragraph_count, XML_TOK_META_STAT_PARA  },
    { XML_NAMESPACE_META, sXML_page_count,  XML_TOK_META_STAT_PAGE  },
    { XML_NAMESPACE_META, sXML_word_count,  XML_TOK_META_STAT_WORD  },
    { XML_NAMESPACE_META, sXML_character_count, XML_TOK_META_STAT_CHAR  },
    XML_TOKEN_MAP_END
};
void SwXMLImport::SetStatisticAttributes(
        const Reference< xml::sax::XAttributeList > & xAttrList)
{
    if( IsStylesOnlyMode() || IsInsertMode() )
        return;

    Reference<XUnoTunnel> xCrsrTunnel( GetTextImport()->GetCursor(),
                                       UNO_QUERY);
    ASSERT( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
    SwXTextCursor *pTxtCrsr = (SwXTextCursor*)xCrsrTunnel->getSomething(
                                        SwXTextCursor::getUnoTunnelId() );
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
    if( nTokens & XML_TOK_META_STAT_PARA )
        SetProgressRef( (sal_Int32)aDocStat.nPara );
    else if ( nTokens & XML_TOK_META_STAT_PAGE )
        SetProgressRef( (sal_Int32)aDocStat.nPage * 10 );
}

// ---------------------------------------------------------------------

void SwXMLExport::_ExportMeta()
{
    SvXMLExport::_ExportMeta();

    OUStringBuffer aOut(16);

    SwDocStat aDocStat( GetDoc().GetDocStat() );
    aOut.append( (sal_Int32)aDocStat.nTbl );
    AddAttribute( XML_NAMESPACE_META, sXML_table_count,
                  aOut.makeStringAndClear() );
    aOut.append( (sal_Int32)aDocStat.nGrf );
    AddAttribute( XML_NAMESPACE_META, sXML_image_count,
                  aOut.makeStringAndClear() );
    aOut.append( (sal_Int32)aDocStat.nOLE );
    AddAttribute( XML_NAMESPACE_META, sXML_object_count,
                  aOut.makeStringAndClear() );
    if( aDocStat.nPage )
    {
        aOut.append( (sal_Int32)aDocStat.nPage );
        AddAttribute( XML_NAMESPACE_META, sXML_page_count,
                      aOut.makeStringAndClear() );
    }
    aOut.append( (sal_Int32)aDocStat.nPara );
    AddAttribute( XML_NAMESPACE_META, sXML_paragraph_count,
                  aOut.makeStringAndClear() );
    aOut.append( (sal_Int32)aDocStat.nWord );
    AddAttribute( XML_NAMESPACE_META, sXML_word_count,
                  aOut.makeStringAndClear() );
    aOut.append( (sal_Int32)aDocStat.nChar );
    AddAttribute( XML_NAMESPACE_META, sXML_character_count,
                  aOut.makeStringAndClear() );
    SvXMLElementExport aElem( *this, XML_NAMESPACE_META,
                              sXML_document_statistic,
                              sal_True, sal_True );
}

