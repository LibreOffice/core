/*************************************************************************
 *
 *  $RCSfile: xmlfmte.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: dvo $ $Date: 2001-03-09 14:42:26 $
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

#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_ATTRLIST_HXX
#include <xmloff/attrlist.hxx>
#endif
#ifndef _XMLOFF_XMLITMPR_HXX
#include <xmloff/xmlexpit.hxx>
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX
#include <xmloff/XMLTextListAutoStylePool.hxx>
#endif
#ifndef _XMLOFF_XMLTEXTMASTERPAGEEXPORT
#include <xmloff/XMLTextMasterPageExport.hxx>
#endif

#ifndef _XMLOFF_TXTPRMAP_HXX
#include <xmloff/txtprmap.hxx>
#endif
#ifndef _XMLOFF_XMLASTPLP_HXX
#include <xmloff/xmlaustp.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif
#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include <xmloff/ProgressBarHelper.hxx>
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

#ifndef _XMLEXP_HXX
#include "xmlexp.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;

void SwXMLExport::ExportFmt( const SwFmt& rFmt, const char *pFamily )
{
    // <style:style ...>
    CheckAttrList();

    // style:family="..."
    const sal_Char *pStr = pFamily;
    DBG_ASSERT( RES_FRMFMT==rFmt.Which(), "frame format expected" );
    if( RES_FRMFMT != rFmt.Which() )
        return;
    DBG_ASSERT( pStr, "family must be specified" );
    // style:name="..."
    AddAttribute( XML_NAMESPACE_STYLE, sXML_name, rFmt.GetName() );

    if( pStr )
        AddAttributeASCII( XML_NAMESPACE_STYLE, sXML_family, pStr );

    // style:parent-style-name="..." (if its not the default only)
    const SwFmt* pParent = rFmt.DerivedFrom();
    // Parent-Namen nur uebernehmen, wenn kein Default
    ASSERT( !pParent || pParent->IsDefault(), "unexpected parent" );

    ASSERT( USHRT_MAX == rFmt.GetPoolFmtId(), "pool ids arent'supported" );
    ASSERT( UCHAR_MAX == rFmt.GetPoolHlpFileId(), "help ids aren't supported" );

    // style:master-page-name
    if( RES_FRMFMT == rFmt.Which() && sXML_table == pStr )
    {
        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == rFmt.GetAttrSet().GetItemState( RES_PAGEDESC,
                                                            sal_False, &pItem ) )
        {
            String sName;
            const SwPageDesc *pPageDesc =
                ((const SwFmtPageDesc *)pItem)->GetPageDesc();
            if( pPageDesc )
                sName = SwXStyleFamilies::GetProgrammaticName(
                                    pPageDesc->GetName(),
                                    SFX_STYLE_FAMILY_PAGE );
            AddAttribute( XML_NAMESPACE_STYLE, sXML_master_page_name, sName );
        }
    }

    if( sXML_table_cell == pStr )
    {
        DBG_ASSERT(RES_FRMFMT == rFmt.Which(), "only frame format");

        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET ==
            rFmt.GetAttrSet().GetItemState( RES_BOXATR_FORMAT,
                                            sal_False, &pItem ) )
        {
            sal_Int32 nFormat = (sal_Int32)
                ((const SwTblBoxNumFormat *)pItem)->GetValue();

            if (-1 != nFormat)
            {
                // if we have a format, register and then export
                // (Careful: here we assume that data styles will be
                // written after cell styles)
                addDataStyle(nFormat);
                AddAttribute( XML_NAMESPACE_STYLE, sXML_data_style_name,
                              getDataStyleName(nFormat) );
            }
        }
    }

    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_STYLE, sXML_style,
                                  sal_True, sal_True );

        SvXMLItemMapEntriesRef xItemMap;
        if( sXML_table == pStr )
            xItemMap = xTableItemMap;
        else if( sXML_table_row == pStr )
            xItemMap = xTableRowItemMap;
        else if( sXML_table_cell == pStr )
            xItemMap = xTableCellItemMap;

        if( xItemMap.Is() )
        {
            SvXMLExportItemMapper& rItemMapper = GetTableItemMapper();
            rItemMapper.setMapEntries( xItemMap );

            GetTableItemMapper().exportXML( GetDocHandler(),
                                           rFmt.GetAttrSet(),
                                           GetTwipUnitConverter(),
                                           GetNamespaceMap(),
                                           XML_EXPORT_FLAG_IGN_WS );
        }
    }
}


void SwXMLExport::_ExportStyles( sal_Bool bUsed )
{
    SvXMLExport::_ExportStyles( bUsed );
    GetTextParagraphExport()->exportTextStyles( bUsed );
}

void SwXMLExport::_ExportAutoStyles()
{
    if( bShowProgress )
    {
        ProgressBarHelper *pProgress = GetProgressBarHelper();
        pProgress->SetValue( nContentProgressStart );
        GetTextParagraphExport()->SetProgress( nContentProgressStart );
    }

    // The order in which styles are collected *MUST* be the same as
    // the order in which they are exported. Otherwise, caching will
    // fail.
    // exported in _ExportMasterStyles
    if( (getExportFlags() & EXPORT_MASTERSTYLES) != 0 )
        GetPageExport()->collectAutoStyles( sal_False );

    // exported in _ExportContent
    if( (getExportFlags() & EXPORT_CONTENT) != 0 )
    {
    GetTextParagraphExport()->exportUsedDeclarations( sal_False );
        GetTextParagraphExport()->exportTrackedChanges( sal_True );
        Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
        Reference < XText > xText = xTextDoc->getText();

        GetTextParagraphExport()->collectFrameBoundToPageAutoStyles( bShowProgress );
        GetTextParagraphExport()->collectTextAutoStyles( xText, bShowProgress );

        // collect form autostyle
        Reference<XDrawPageSupplier> xDrawPageSupplier( GetModel(), UNO_QUERY );
        if (xDrawPageSupplier.is() && GetFormExport().is())
        {
            Reference<XDrawPage> xPage = xDrawPageSupplier->getDrawPage();
            if (xPage.is())
                GetFormExport()->examineForms(xPage);
        }
    }

    GetTextParagraphExport()->exportTextAutoStyles();
    GetShapeExport()->exportAutoStyles();
    if( (getExportFlags() & EXPORT_MASTERSTYLES) != 0 )
        GetPageExport()->exportAutoStyles();

    // we rely on data styles being written after cell styles in the
    // ExportFmt() method; so be careful when changing order.
    exportAutoDataStyles();
}

XMLPageExport* SwXMLExport::CreatePageExport()
{
    return new XMLTextMasterPageExport( *this );
}

void SwXMLExport::_ExportMasterStyles()
{
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
            const SvXMLExportPropertyMapper& rPropExp,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap) const;
public:

    SwXMLAutoStylePoolP( SvXMLExport& rExport );
    virtual ~SwXMLAutoStylePoolP();
};

void SwXMLAutoStylePoolP::exportStyleAttributes(
            SvXMLAttributeList& rAttrList,
            sal_Int32 nFamily,
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap) const
{
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
                        OUString sName( rNamespaceMap.GetQNameByKey(
                                XML_NAMESPACE_STYLE, sListStyleName ) );
                        rAttrList.AddAttribute( sName, sCDATA, sStyleName );
                    }
                }
                break;
            case CTF_PAGEDESCNAME:
                {
                    OUString sStyleName;
                    aProperty->maValue >>= sStyleName;
                    OUString sName( rNamespaceMap.GetQNameByKey(
                                XML_NAMESPACE_STYLE, sMasterPageName ) );
                    rAttrList.AddAttribute( sName, sCDATA, sStyleName );
                }
                break;
            }
        }
    }
}

SwXMLAutoStylePoolP::SwXMLAutoStylePoolP(SvXMLExport& rExp ) :
    SvXMLAutoStylePoolP(),
    rExport( rExp ),
    sListStyleName( RTL_CONSTASCII_USTRINGPARAM( sXML_list_style_name) ),
    sMasterPageName( RTL_CONSTASCII_USTRINGPARAM( sXML_master_page_name) ),
    sCDATA( RTL_CONSTASCII_USTRINGPARAM( sXML_CDATA) )
{
}


SwXMLAutoStylePoolP::~SwXMLAutoStylePoolP()
{
}

SvXMLAutoStylePoolP* SwXMLExport::CreateAutoStylePool()
{
    return new SwXMLAutoStylePoolP( *this );
}
