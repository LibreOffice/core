/*************************************************************************
 *
 *  $RCSfile: xmlfmte.cxx,v $
 *
 *  $Revision: 1.9 $
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

#ifndef _HINTS_HXX //autogen wg. SwFmtChg
#include <hints.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _SVSTDARR_STRINGSSORT_DECL
#define _SVSTDARR_STRINGSSORT
#include <svtools/svstdarr.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif

#ifndef _XMLOFF_ATTRLIST_HXX
#include <xmloff/attrlist.hxx>
#endif

#ifndef _XMLOFF_XMLASTPL_HXX
#include <xmloff/xmlastpl.hxx>
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

#ifndef _XMLOFF_TXTSTYLE_HXX
#include <xmloff/txtstyle.hxx>
#endif

#ifndef _XMLOFF_XMLPROPERTYSETMAPPER_HXX
#include <xmloff/xmlprmap.hxx>
#endif
#ifndef _XMLOFF_XMLPROPMAPPINGTYPES_HXX
#include <xmloff/maptype.hxx>
#endif
#ifndef _XMLOFF_XMLTYPES_HXX
#include <xmloff/xmltypes.hxx>
#endif
#ifndef _XMLOFF_TXTPARAE_HXX
#include <xmloff/txtparae.hxx>
#endif
#ifndef _XMLOFF_TXTPRMAP_HXX
#include <xmloff/txtprmap.hxx>
#endif
#ifndef _XMLOFF_XMLEXPPR_HXX
#include <xmloff/xmlexppr.hxx>
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
#ifndef _FMTCOL_HXX //autogen wg. SwTxtFmtColl
#include <fmtcol.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen wg. SwCharFmt
#include <charfmt.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _FMTPDSC_HXX
#include <fmtpdsc.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif

#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#include "docary.hxx"
#ifndef _NDTXT_HXX //autogen wg. SwTxtNode
#include <ndtxt.hxx>
#endif
#ifndef _SECTION_HXX //autogen wg. SwSection
#include <section.hxx>
#endif
#ifndef _PAM_HXX //autogen wg. SwPaM
#include <pam.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _XMLTEXTE_HXX
#include "xmltexte.hxx"
#endif

#ifndef _SWDOCSH_HXX
#include "docsh.hxx"
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX
#include <xmloff/XMLTextListAutoStylePool.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELD_HPP_
#include <com/sun/star/text/XTextField.hpp>
#endif

#ifndef _UNOFIELD_HXX
#include <unofield.hxx>
#endif

#ifndef _XMLOFF_TXTFLDE_HXX
#include <xmloff/txtflde.hxx>
#endif

#ifndef _XMLOFF_XMLNUMFE_HXX
#include <xmloff/xmlnumfe.hxx>
#endif
#ifndef _XMLOFF_XMLTEXTMASTERPAGEEXPORT
#include <xmloff/XMLTextMasterPageExport.hxx>
#endif

#ifndef _UNOSTYLE_HXX
#include "unostyle.hxx"
#endif

#ifndef _CELLATR_HXX
#include "cellatr.hxx"
#endif


using namespace rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;

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

SwXMLTextParagraphExport::SwXMLTextParagraphExport(
        SwXMLExport& rExp,
         SvXMLAutoStylePoolP& rAutoStylePool ) :
    XMLTextParagraphExport( rExp, rAutoStylePool ),
    sTextTable( RTL_CONSTASCII_USTRINGPARAM( "TextTable" ) )
{
}

SwXMLTextParagraphExport::~SwXMLTextParagraphExport()
{
}

void SwXMLTextParagraphExport::exportStyleContent(
        const Reference< XStyle > & rStyle )
{

    const SwXStyle* pStyle = 0;
    Reference<XUnoTunnel> xStyleTunnel( rStyle, UNO_QUERY);
    if( xStyleTunnel.is() )
    {
        pStyle = (SwXStyle*)xStyleTunnel->getSomething(
                                            SwXStyle::getUnoTunnelId() );
    }
    if( pStyle && SFX_STYLE_FAMILY_PARA == pStyle->GetFamily() )
    {
        const SwDoc& rDoc = ((SwXMLExport&)GetExport()).GetDoc();
        const SwTxtFmtColl *pColl =
            rDoc.FindTxtFmtCollByName( SwXStyleFamilies::GetUIName( pStyle->GetStyleName(), SFX_STYLE_FAMILY_PARA ) );
        ASSERT( pColl, "There is the text collection?" );
        if( pColl && RES_CONDTXTFMTCOLL == pColl->Which() )
        {
            const SwFmtCollConditions& rConditions =
                ((const SwConditionTxtFmtColl *)pColl)->GetCondColls();
            for( sal_uInt16 i=0; i < rConditions.Count(); i++ )
            {
                const SwCollCondition& rCond = *rConditions[i];

                const sal_Char *pFunc = 0;
                OUStringBuffer sBuffer( 20 );
                switch( rCond.GetCondition() )
                {
                case PARA_IN_LIST:
                    pFunc = sXML_list_level;
                    sBuffer.append( (sal_Int32)(rCond.GetSubCondition()+1) );
                    break;
                case PARA_IN_OUTLINE:
                    pFunc = sXML_outline_level;
                    sBuffer.append( (sal_Int32)(rCond.GetSubCondition()+1) );
                    break;
                case PARA_IN_FRAME:
                    pFunc = sXML_text_box;
                    break;
                case PARA_IN_TABLEHEAD:
                    pFunc = sXML_table_header;
                    break;
                case PARA_IN_TABLEBODY:
                    pFunc = sXML_table;
                    break;
                case PARA_IN_SECTION:
                    pFunc = sXML_section;
                    break;
                case PARA_IN_FOOTENOTE:
                    pFunc = sXML_footnote;
                    break;
                case PARA_IN_FOOTER:
                    pFunc = sXML_footer;
                    break;
                case PARA_IN_HEADER:
                    pFunc = sXML_header;
                    break;
                case PARA_IN_ENDNOTE:
                    pFunc = sXML_endnote;
                    break;
                }
                OUString sVal( sBuffer.makeStringAndClear() );

                DBG_ASSERT( pFunc, "SwXMLExport::ExportFmt: unknon condition" );
                if( pFunc )
                {
                    sBuffer.appendAscii( pFunc );
                    sBuffer.append( (sal_Unicode)'(' );
                    sBuffer.append( (sal_Unicode)')' );
                    if( sVal.getLength() )
                    {
                        sBuffer.append( (sal_Unicode)'=' );
                        sBuffer.append( sVal );
                    }

                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                sXML_condition,
                                sBuffer.makeStringAndClear() );
                    const String& rName =
                        SwXStyleFamilies::GetProgrammaticName(
                                    rCond.GetTxtFmtColl()->GetName(),
                                    SFX_STYLE_FAMILY_PARA );
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                sXML_apply_style_name, rName );
                    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                              sXML_map, sal_True, sal_True );
                }
            }
        }
    }
}

void SwXMLTextParagraphExport::exportTextAutoStyles()
{
    XMLTextParagraphExport::exportTextAutoStyles();
    GetAutoStylePool().exportXML( XML_STYLE_FAMILY_SD_GRAPHICS_ID,
                                   GetExport().GetDocHandler(),
                                   GetExport().GetMM100UnitConverter(),
                                   GetExport().GetNamespaceMap() );
}

void SwXMLExport::_ExportStyles( sal_Bool bUsed )
{
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
    Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
    Reference < XText > xText = xTextDoc->getText();

    GetTextParagraphExport()->collectFrameBoundToPageAutoStyles( bShowProgress );
    GetTextParagraphExport()->collectTextAutoStyles( xText, bShowProgress );
    GetPageExport()->collectAutoStyles( sal_False );

    GetTextParagraphExport()->exportTextAutoStyles();
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
