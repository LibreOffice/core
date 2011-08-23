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

#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_ 
#include <com/sun/star/text/XTextDocument.hpp>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <bf_xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_ATTRLIST_HXX
#include <bf_xmloff/attrlist.hxx>
#endif
#ifndef _XMLITMPR_HXX
#include "xmlexpit.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <bf_xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX 
#include <bf_xmloff/XMLTextListAutoStylePool.hxx>
#endif
#ifndef _XMLOFF_XMLTEXTMASTERPAGEEXPORT
#include <bf_xmloff/XMLTextMasterPageExport.hxx>
#endif

#ifndef _XMLOFF_TXTPRMAP_HXX
#include <bf_xmloff/txtprmap.hxx>
#endif
#ifndef _XMLOFF_XMLASTPLP_HXX 
#include <bf_xmloff/xmlaustp.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include <bf_xmloff/families.hxx>
#endif
#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include <bf_xmloff/ProgressBarHelper.hxx>
#endif

#ifndef _FORMAT_HXX //autogen wg. SwFmt
#include <format.hxx>
#endif
#ifndef _FMTPDSC_HXX
#include <fmtpdsc.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif

#ifndef _UNOSTYLE_HXX
#include <unostyle.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_ 
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_ 
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_ 
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::binfilter::xmloff::token;

void SwXMLExport::ExportFmt( const SwFmt& rFmt, enum XMLTokenEnum eFamily )
{
    // <style:style ...>
    CheckAttrList();

    // style:family="..."
    DBG_ASSERT( RES_FRMFMT==rFmt.Which(), "frame format expected" );
    if( RES_FRMFMT != rFmt.Which() )
        return;
    DBG_ASSERT( eFamily != XML_TOKEN_INVALID, "family must be specified" );
    // style:name="..."
    AddAttribute( XML_NAMESPACE_STYLE, XML_NAME, rFmt.GetName() );

    if( eFamily != XML_TOKEN_INVALID )
        AddAttribute( XML_NAMESPACE_STYLE, XML_FAMILY, eFamily );
    
    // style:parent-style-name="..." (if its not the default only)
    const SwFmt* pParent = rFmt.DerivedFrom();
    // Parent-Namen nur uebernehmen, wenn kein Default
    ASSERT( !pParent || pParent->IsDefault(), "unexpected parent" );

    ASSERT( USHRT_MAX == rFmt.GetPoolFmtId(), "pool ids arent'supported" );
    ASSERT( USHRT_MAX == rFmt.GetPoolHelpId(), "help ids arent'supported" );
    ASSERT( USHRT_MAX == rFmt.GetPoolHelpId() ||
            UCHAR_MAX == rFmt.GetPoolHlpFileId(), "help file ids aren't supported" );

    // style:master-page-name
    if( RES_FRMFMT == rFmt.Which() && XML_TABLE == eFamily )
    {
        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == rFmt.GetAttrSet().GetItemState( RES_PAGEDESC,
                                                            sal_False, &pItem ) )
        {
            String sName;
            const SwPageDesc *pPageDesc =
                ((const SwFmtPageDesc *)pItem)->GetPageDesc();
            if( pPageDesc )
                SwStyleNameMapper::FillProgName(
                                    pPageDesc->GetName(),
                                    sName,
                                    GET_POOLID_PAGEDESC,
                                    sal_True);
            AddAttribute( XML_NAMESPACE_STYLE, XML_MASTER_PAGE_NAME, sName );
        }
    }

    if( XML_TABLE_CELL == eFamily )
    {
        DBG_ASSERT(RES_FRMFMT == rFmt.Which(), "only frame format");

        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == 
            rFmt.GetAttrSet().GetItemState( RES_BOXATR_FORMAT,
                                            sal_False, &pItem ) )
        {
            sal_Int32 nFormat = (sal_Int32)
                ((const SwTblBoxNumFormat *)pItem)->GetValue();

            if ( (nFormat != -1) && (nFormat != NUMBERFORMAT_TEXT) )
            {
                // if we have a format, register and then export
                // (Careful: here we assume that data styles will be
                // written after cell styles)
                addDataStyle(nFormat);
                OUString sDataStyleName = getDataStyleName(nFormat);
                if( sDataStyleName.getLength() > 0 )
                    AddAttribute( XML_NAMESPACE_STYLE, XML_DATA_STYLE_NAME, 
                                  sDataStyleName );
            }
        }
    }

    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_STYLE, XML_STYLE,
                                  sal_True, sal_True );

        SvXMLItemMapEntriesRef xItemMap;
        if( XML_TABLE == eFamily )
            xItemMap = xTableItemMap;
        else if( XML_TABLE_ROW == eFamily )
            xItemMap = xTableRowItemMap;
        else if( XML_TABLE_CELL == eFamily )
            xItemMap = xTableCellItemMap;

        if( xItemMap.Is() )
        {
            SvXMLExportItemMapper& rItemMapper = GetTableItemMapper();
            rItemMapper.setMapEntries( xItemMap );

            GetTableItemMapper().exportXML( *this,
                                           rFmt.GetAttrSet(),
                                           GetTwipUnitConverter(),
                                           XML_EXPORT_FLAG_IGN_WS );
        }
    }
}


void SwXMLExport::_ExportStyles( sal_Bool bUsed )
{
    SvXMLExport::_ExportStyles( bUsed );

    // drawing defaults
    GetShapeExport()->ExportGraphicDefaults();

    GetTextParagraphExport()->exportTextStyles( bUsed
                                             ,IsShowProgress()
                                              );
}

void SwXMLExport::_ExportAutoStyles()
{
    // The order in which styles are collected *MUST* be the same as
    // the order in which they are exported. Otherwise, caching will
    // fail.

    if( (getExportFlags() & (EXPORT_MASTERSTYLES|EXPORT_CONTENT)) != 0 )
    {
        if( (getExportFlags() & EXPORT_CONTENT) == 0 )
        {
            // only master pages are exported => styles for frames bound
            // to frames (but none for frames bound to pages) need to be
            // collected.
            GetTextParagraphExport()->collectFramesBoundToFrameAutoStyles(
                                                bShowProgress );
        }
        else
        {
            // content (and optional master pages) are exported => styles
            // for frames bound to frame or to pages need to be
            // collected.
            GetTextParagraphExport()->collectFramesBoundToPageOrFrameAutoStyles(
                                                bShowProgress );
        }

    }

    // exported in _ExportMasterStyles
    if( (getExportFlags() & EXPORT_MASTERSTYLES) != 0 )
        GetPageExport()->collectAutoStyles( sal_False );

    // if we don't export styles (i.e. in content stream only, but not
    // in single-stream case), then we can save ourselves a bit of
    // work and memory by not collecting field masters
    if( (getExportFlags() & EXPORT_STYLES ) == 0 )
        GetTextParagraphExport()->exportUsedDeclarations( sal_False );

    // exported in _ExportContent
    if( (getExportFlags() & EXPORT_CONTENT) != 0 )
    {
        GetTextParagraphExport()->exportTrackedChanges( sal_True );
        Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
        Reference < XText > xText = xTextDoc->getText();

        // collect form autostyle
        // (do this before collectTextAutoStyles, 'cause the shapes need the results of the work
        // done by examineForms)
        Reference<XDrawPageSupplier> xDrawPageSupplier( GetModel(), UNO_QUERY );
        if (xDrawPageSupplier.is() && GetFormExport().is())
        {
            Reference<XDrawPage> xPage = xDrawPageSupplier->getDrawPage();
            if (xPage.is())
                GetFormExport()->examineForms(xPage);
        }

        GetTextParagraphExport()->collectTextAutoStyles( xText, bShowProgress );
    }

    GetTextParagraphExport()->exportTextAutoStyles();
    GetShapeExport()->exportAutoStyles();
    if( (getExportFlags() & EXPORT_MASTERSTYLES) != 0 )
        GetPageExport()->exportAutoStyles();

    // we rely on data styles being written after cell styles in the
    // ExportFmt() method; so be careful when changing order.
    exportAutoDataStyles();

    sal_uInt16 nContentAutostyles = EXPORT_CONTENT | EXPORT_AUTOSTYLES;
    if ( ( getExportFlags() & nContentAutostyles ) == nContentAutostyles )
        GetFormExport()->exportAutoStyles();
}

XMLPageExport* SwXMLExport::CreatePageExport()
{
    return new XMLTextMasterPageExport( *this );
}

void SwXMLExport::_ExportMasterStyles()
{
    // export master styles
    GetPageExport()->exportMasterStyles( sal_False );
}

// ---------------------------------------------------------------------
class SwXMLAutoStylePoolP : public SvXMLAutoStylePoolP
{
    SvXMLExport& rExport;
    const OUString sListStyleName;
    const OUString sMasterPageName;
    const OUString sCDATA;

protected:

    virtual void exportStyleAttributes(
            SvXMLAttributeList& rAttrList,
            sal_Int32 nFamily, 
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp
            , const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap
            ) const;
public:

    SwXMLAutoStylePoolP( SvXMLExport& rExport );
    virtual ~SwXMLAutoStylePoolP();
};

void SwXMLAutoStylePoolP::exportStyleAttributes(
            SvXMLAttributeList& rAttrList,
            sal_Int32 nFamily, 
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp
            , const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap
            ) const
{
    SvXMLAutoStylePoolP::exportStyleAttributes( rAttrList, nFamily, rProperties, rPropExp, rUnitConverter, rNamespaceMap);

    if( XML_STYLE_FAMILY_TEXT_PARAGRAPH == nFamily )
    {
        for( ::std::vector< XMLPropertyState >::const_iterator
                    aProperty = rProperties.begin();
             aProperty != rProperties.end();
              aProperty++ )
        {
            switch( rPropExp.getPropertySetMapper()->
                        GetEntryContextId( aProperty->mnIndex ) )
            {
            case CTF_NUMBERINGSTYLENAME:
                {
                    OUString sStyleName;
                    aProperty->maValue >>= sStyleName;
                    if( sStyleName.getLength() )
                    {
                        OUString sTmp = rExport.GetTextParagraphExport()->GetListAutoStylePool().Find( sStyleName );
                        if( sTmp.getLength() )
                            sStyleName = sTmp;
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                  sListStyleName, sStyleName );
                    }
                }
                break;
            case CTF_PAGEDESCNAME:
                {
                    OUString sStyleName;
                    aProperty->maValue >>= sStyleName;
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                              sMasterPageName, sStyleName );
                }
                break;
            }
        }
    }
}

SwXMLAutoStylePoolP::SwXMLAutoStylePoolP(SvXMLExport& rExp ) :
    SvXMLAutoStylePoolP( rExp ),
    rExport( rExp ),
    sListStyleName( GetXMLToken( XML_LIST_STYLE_NAME ) ),
    sMasterPageName( GetXMLToken( XML_MASTER_PAGE_NAME ) ),
    sCDATA( GetXMLToken( XML_CDATA ) )
{
}


SwXMLAutoStylePoolP::~SwXMLAutoStylePoolP()
{
}

SvXMLAutoStylePoolP* SwXMLExport::CreateAutoStylePool()
{
    return new SwXMLAutoStylePoolP( *this );
}
}
