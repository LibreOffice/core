/*************************************************************************
 *
 *  $RCSfile: xmlfmte.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-12 17:30:28 $
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
#ifndef _XMLECTXT_HXX
#include "xmlectxt.hxx"
#endif
#ifndef _XMLFMTE_HXX
#include "xmlfmte.hxx"
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
#ifndef XML_CORE_API
#ifndef _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX
#include <xmloff/XMLTextListAutoStylePool.hxx>
#endif
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
#ifdef XML_CORE_API
    SfxStyleFamily eFamily = SFX_STYLE_FAMILY_ALL;
    switch( rFmt.Which() )
    {
    case RES_CHRFMT:
        if( !pStr )
            pStr = sXML_text;
        eFamily = SFX_STYLE_FAMILY_CHAR;
        break;
    case RES_FRMFMT:
        if( !pStr )
            pStr = sXML_frame;
        eFamily = SFX_STYLE_FAMILY_FRAME;
        break;
    case RES_TXTFMTCOLL:
    case RES_CONDTXTFMTCOLL:
        if( !pStr )
            pStr = sXML_paragraph;
        eFamily = SFX_STYLE_FAMILY_PARA;
        break;
    case RES_FLYFRMFMT:
    case RES_GRFFMTCOLL:
        DBG_ASSERT( pStr, "SwXMLExport::ExportFmt: which id unexpected" );
        break;
    default:
        DBG_ASSERT( pStr, "SwXMLExport::ExportFmt: which id unknown" );
        break;
    }
    // style:name="..."
    AddAttribute( XML_NAMESPACE_STYLE, sXML_name,
            SwXStyleFamilies::GetProgrammaticName( rFmt.GetName(), eFamily ) );
#else
    DBG_ASSERT( RES_FRMFMT==rFmt.Which(), "frame format expected" );
    if( RES_FRMFMT != rFmt.Which() )
        return;
    DBG_ASSERT( pStr, "family must be specified" );
    // style:name="..."
    AddAttribute( XML_NAMESPACE_STYLE, sXML_name, rFmt.GetName() );
#endif

    if( pStr )
        AddAttributeASCII( XML_NAMESPACE_STYLE, sXML_family, pStr );

#ifdef XML_CORE_API
    // register name to prevent it from beeing reused as an automtic
    // style name
    if( SFX_STYLE_FAMILY_ALL != eFamily )
        pItemSetAutoStylePool->RegisterName( eFamily, rFmt.GetName() );
#endif

    // style:parent-style-name="..." (if its not the default only)
    const SwFmt* pParent = rFmt.DerivedFrom();
    // Parent-Namen nur uebernehmen, wenn kein Default
    if( pParent && !pParent->IsDefault() )
    {
#ifdef XML_CORE_API
        AddAttribute( XML_NAMESPACE_STYLE, sXML_parent_style_name,
            SwXStyleFamilies::GetProgrammaticName( pParent->GetName(),
                                                   eFamily ) );
#else
        AddAttribute( XML_NAMESPACE_STYLE, sXML_parent_style_name,
                      pParent->GetName() );
#endif
    }

#ifdef XML_CORE_API
    // style:next-style-name="..."
    // (if its not the default and not the same style only)
    if( RES_TXTFMTCOLL == rFmt.Which() || RES_CONDTXTFMTCOLL == rFmt.Which() )
    {
        const SwTxtFmtColl& rFollow =
            ((const SwTxtFmtColl&)rFmt).GetNextTxtFmtColl();
        if( !rFollow.IsDefault() && &rFollow != &rFmt )
        {
            AddAttribute( XML_NAMESPACE_STYLE, sXML_next_style_name,
                SwXStyleFamilies::GetProgrammaticName( rFollow.GetName(),
                                                       eFamily ) );
        }
    }

#if 0
    // style:pool-id="..."
    if( USHRT_MAX != rFmt.GetPoolFmtId() )
    {
        OUStringBuffer sTmp( 5L );
        sTmp.append( (sal_Int32)rFmt.GetPoolFmtId() );
        AddAttribute( XML_NAMESPACE_STYLE, sXML_pool_id,
                      sTmp.makeStringAndClear() );
    }

    // style:help-file-name="..." and style:help-id="..."
    if( UCHAR_MAX != rFmt.GetPoolHlpFileId() )
    {
        AddAttribute( XML_NAMESPACE_STYLE, sXML_help_file_name,
                      *pDoc->GetDocPattern( rFmt.GetPoolHlpFileId() ) );
        OUStringBuffer sTmp( 5L );
        sTmp.append( (sal_Int32)rFmt.GetPoolHelpId() );
        AddAttribute( XML_NAMESPACE_STYLE, sXML_help_id,
                      sTmp.makeStringAndClear() );
    }
#endif

    // style:auto-update="..."
    if( rFmt.IsAutoUpdateFmt() )
    {
        AddAttributeASCII( XML_NAMESPACE_STYLE, sXML_auto_update, sXML_true );
    }

    // text:list-style-name
    if( RES_TXTFMTCOLL == rFmt.Which() || RES_CONDTXTFMTCOLL == rFmt.Which() )
    {
        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == rFmt.GetAttrSet().GetItemState( RES_PARATR_NUMRULE,
                                                            sal_False, &pItem ) )
        {
            AddAttribute( XML_NAMESPACE_STYLE, sXML_list_style_name,
                SwXStyleFamilies::GetProgrammaticName(
                    ((const SwNumRuleItem *)pItem)->GetValue(),
                    SFX_STYLE_FAMILY_PSEUDO ) );
        }
    }
#else
    ASSERT( USHRT_MAX == rFmt.GetPoolFmtId(), "pool ids arent'supported" );
    ASSERT( UCHAR_MAX == rFmt.GetPoolHlpFileId(), "help ids aren't supported" );
#endif

    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_STYLE, sXML_style,
                                  sal_True, sal_True );

#ifdef XML_CORE_API
        switch( rFmt.Which() )
        {
        case RES_TXTFMTCOLL:
        case RES_CONDTXTFMTCOLL:
        case RES_CHRFMT:
            GetParaItemMapper().exportXML( GetDocHandler(),
                                           rFmt.GetAttrSet(),
                                           GetTwipUnitConverter(),
                                           GetNamespaceMap(),
                                           XML_EXPORT_FLAG_IGN_WS );
            break;
        case RES_FRMFMT:
            {
#endif
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
#ifdef XML_CORE_API
            }
            break;
        default:
            DBG_ASSERT( !this,
                        "SwXMLExport::ExportFmt: attribute export missing" );
            break;
        }

        if( RES_CONDTXTFMTCOLL == rFmt.Which() )
        {
            const SwFmtCollConditions& rConditions =
                ((SwConditionTxtFmtColl&)rFmt). GetCondColls();
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

                    AddAttribute( XML_NAMESPACE_STYLE, sXML_condition,
                                  sBuffer.makeStringAndClear() );
                    const String& rName =
                        SwXStyleFamilies::GetProgrammaticName(
                                    rCond.GetTxtFmtColl()->GetName(),
                                    SFX_STYLE_FAMILY_PARA );
                    AddAttribute( XML_NAMESPACE_STYLE, sXML_apply_style_name,
                                  rName );
                    SvXMLElementExport aElem( *this, XML_NAMESPACE_STYLE,
                                              sXML_map, sal_True, sal_True );
                }
            }
        }
#endif
    }
}

#ifndef XML_CORE_API

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
                                   *GetAutoFramePropMapper().get(),
                                   GetExport().GetDocHandler(),
                                   GetExport().GetMM100UnitConverter(),
                                   GetExport().GetNamespaceMap() );
}
#endif

void SwXMLExport::_ExportStyles( sal_Bool bUsed )
{
#ifdef XML_CORE_API
    // text collections
    SvStringsSort *pCollNames = bUsed ? new SvStringsSort : 0;
    sal_uInt16 nCount = pDoc->GetTxtFmtColls()->Count();
    sal_uInt16 i;
    for( i=0; i<nCount; i++ )
    {
        const SwTxtFmtColl *pColl = (*pDoc->GetTxtFmtColls())[i];
        if( ( !bUsed || pDoc->IsUsed( *pColl ) ) && !pColl->IsDefault() )
        {
            ExportFmt( *pColl );
            if( pCollNames )
                pCollNames->Insert( (String *)&pColl->GetName() );
        }
    }
    if( bUsed )
    {
        // Even if some next styles have not been used, they have to be
        // exported.
        for( i = 0; i < nCount; i++ )
        {
            const SwTxtFmtColl* pColl = (*pDoc->GetTxtFmtColls())[ i ];
            if( pColl != &pColl->GetNextTxtFmtColl() &&
                !pCollNames->Seek_Entry(
                    (String *)&pColl->GetNextTxtFmtColl().GetName() ) )
            {
                ExportFmt( pColl->GetNextTxtFmtColl() );
                pCollNames->Insert(
                    (String *)&pColl->GetNextTxtFmtColl().GetName() );
            }
        }
        delete pCollNames;
    }

    // char fmts
    nCount = pDoc->GetCharFmts()->Count();
    for( i=0; i<nCount; i++ )
    {
        const SwCharFmt *pFmt = (*pDoc->GetCharFmts())[i];
        if( ( !bUsed || pDoc->IsUsed( *pFmt ) ) && !pFmt->IsDefault() )
            ExportFmt( *pFmt );
    }

    // num rules
    ExportNumRules( sal_False, bUsed );
#else
    GetTextParagraphExport()->exportTextStyles( bUsed );
#endif
}

// ---------------------------------------------------------------------

#ifdef XML_CORE_API
void SwXMLExport::AddTextAutoStyle( const SfxPoolItem& rItem )
{
    SfxItemSet aItemSet( pDoc->GetAttrPool(),
                         RES_CHRATR_BEGIN,      RES_CHRATR_END - 1,
                         RES_UNKNOWNATR_BEGIN,  RES_UNKNOWNATR_END - 1, 0 );
    aItemSet.Put( rItem );
    pItemSetAutoStylePool->Add( SFX_STYLE_FAMILY_CHAR, aItemSet );
}

void SwXMLExport::AddParaAutoStyle( const OUString& rParent,
                                    const SfxItemSet& rItemSet )
{
    pItemSetAutoStylePool->Add( SFX_STYLE_FAMILY_PARA,
        SwXStyleFamilies::GetProgrammaticName(rParent, SFX_STYLE_FAMILY_PARA ),
           rItemSet );
}

OUString SwXMLExport::FindTextAutoStyle( const SfxPoolItem& rItem )
{
    SfxItemSet aItemSet( pDoc->GetAttrPool(),
                         RES_CHRATR_BEGIN,      RES_CHRATR_END - 1,
                         RES_UNKNOWNATR_BEGIN,  RES_UNKNOWNATR_END - 1, 0 );
    aItemSet.Put( rItem );
    return pItemSetAutoStylePool->Find( SFX_STYLE_FAMILY_CHAR, aItemSet );
}

OUString SwXMLExport::FindParaAutoStyle( const OUString& rParent,
                                         const SfxItemSet& rItemSet )
{
    return pItemSetAutoStylePool->Find(
        SFX_STYLE_FAMILY_PARA,
        SwXStyleFamilies::GetProgrammaticName(rParent, SFX_STYLE_FAMILY_PARA ),
         rItemSet );
}

void SwXMLExport::ExportTxtNodeAutoStyles( const SwTxtNode& rTxtNd,
                                           xub_StrLen nStart, xub_StrLen nEnd,
                                           sal_Bool bExportWholeNode )
{
    const SfxItemSet *pItemSet = rTxtNd.GetpSwAttrSet();
    if( pItemSet )
    {
        const SwFmtColl& rColl = rTxtNd.GetAnyFmtColl();
        OUString sParent( rColl.GetName() );
        AddParaAutoStyle( sParent, *pItemSet );

        if( rTxtNd.GetCondFmtColl() &&
             &rColl != rTxtNd.GetFmtColl() )
        {
            const SwFmtColl *pColl = rTxtNd.GetFmtColl();
            sParent = pColl->GetName();
            AddParaAutoStyle( sParent, *pItemSet );
        }
    }

    xub_StrLen nPos = pCurPaM->GetPoint()->nContent.GetIndex();
    xub_StrLen nEndPos = rTxtNd.GetTxt().Len();
    if( pCurPaM->GetPoint()->nNode == pCurPaM->GetMark()->nNode )
        nEndPos = pCurPaM->GetMark()->nContent.GetIndex();

    const SwpHints *pHints = rTxtNd.GetpSwpHints();
    sal_uInt16 nHintCount = pHints ? pHints->Count() : 0;
    sal_uInt16 nHintPos = 0;
    const SwTxtAttr * pTxtAttr = 0;

    if( nHintCount && nPos > *( pTxtAttr = (*pHints)[0] )->GetStart() )
    {
        do
        {
            nHintPos++;
            sal_uInt16 nWhich = pTxtAttr->Which();
            if( pTxtAttr->GetEnd() &&
                ( (nWhich >= RES_CHRATR_BEGIN && nWhich < RES_CHRATR_END) ||
                  (nWhich >= RES_UNKNOWNATR_BEGIN &&
                                             nWhich < RES_UNKNOWNATR_END) ) )
            {
                xub_StrLen nHtEnd = *pTxtAttr->GetEnd(),
                       nHtStt = *pTxtAttr->GetStart();
                if( (bExportWholeNode || nHtEnd > nPos) &&
                    nHtEnd != nHtStt )
                {
                    AddTextAutoStyle( pTxtAttr->GetAttr() );
                }
            }

        }
        while( nHintPos < nHintCount &&
               nPos > *( pTxtAttr = (*pHints)[nHintPos ] )->GetStart() );
    }

    for( ; nPos < nEndPos; nPos++ )
    {
        if( nHintPos < nHintCount && *pTxtAttr->GetStart() == nPos
            && nPos != nEndPos )
        {
            do
            {
                sal_uInt16 nWhich = pTxtAttr->Which();
                if( pTxtAttr->GetEnd() && *pTxtAttr->GetEnd() != nPos &&
                    ( (nWhich >= RES_CHRATR_BEGIN && nWhich < RES_CHRATR_END) ||
                      (nWhich >= RES_UNKNOWNATR_BEGIN &&
                                             nWhich < RES_UNKNOWNATR_END) ) )
                {
                    AddTextAutoStyle( pTxtAttr->GetAttr() );
                } else if( nWhich == RES_TXTATR_FIELD ) {
                    // text field found: export AutoStyle
#ifdef XML_CORE_API
//                  Reference < text::XTextField > xFld = new SwXTextField(
//                      ((const SwFmtFld&)pTxtAttr->GetAttr()), &GetDoc() );
//
//                  GetTextFieldExport().ExportFieldAutoStyle( xFld );
#endif
                }
            }
            while( ++nHintPos < nHintCount &&
                  nPos == *( pTxtAttr = (*pHints)[nHintPos] )->GetStart() );
        }
    }
}

void SwXMLExport::ExportSectionAutoStyles( const SwSectionNode& rSectNd )
{
    SwNode *pStartNd = pDoc->GetNodes()[rSectNd.GetIndex() + 1];
    SwNode *pEndNd = pDoc->GetNodes()[rSectNd.EndOfSectionIndex() - 1];

    {
        SwXMLExpContext aContext( *this, *pStartNd, *pEndNd, 0, STRING_LEN );
        ExportCurPaMAutoStyles();
    }

    pCurPaM->GetPoint()->nNode = *rSectNd.EndOfSectionNode();
}

void SwXMLExport::ExportCurPaMAutoStyles( sal_Bool bExportWholePaM )
{
    sal_Bool bFirstNode = sal_True;
    sal_Bool bExportWholeNode = bExportWholePaM;

    while( pCurPaM->GetPoint()->nNode.GetIndex() <
                                pCurPaM->GetMark()->nNode.GetIndex() ||
           ( pCurPaM->GetPoint()->nNode.GetIndex() ==
                                pCurPaM->GetMark()->nNode.GetIndex() &&
             pCurPaM->GetPoint()->nContent.GetIndex() <=
                                pCurPaM->GetMark()->nContent.GetIndex() ) )
    {
        SwNode *pNd = pCurPaM->GetNode();

        ASSERT( !(pNd->IsGrfNode() || pNd->IsOLENode()),
                "SwXMLExport::exportCurPaM: grf or OLE node unexpected" );
        if( pNd->IsTxtNode() )
        {
            SwTxtNode* pTxtNd = pNd->GetTxtNode();

            if( !bFirstNode )
                pCurPaM->GetPoint()->nContent.Assign( pTxtNd, 0 );

            ExportTxtNodeAutoStyles( *pTxtNd, 0, STRING_LEN, bExportWholeNode );
        }
        else if( pNd->IsTableNode() )
        {
            ExportTableAutoStyles( *pNd->GetTableNode() );
        }
        else if( pNd->IsSectionNode() )
        {
            ExportSectionAutoStyles( *pNd->GetSectionNode() );
        }
        else if( pNd == &pDoc->GetNodes().GetEndOfContent() )
            break;

        pCurPaM->GetPoint()->nNode++;   // next node

        // if not everything should be exported, the WriteAll flag must be
        // set for all but the first and last node anyway.
        bExportWholeNode = bExportWholePaM ||
                           pCurPaM->GetPoint()->nNode.GetIndex() !=
                           pCurPaM->GetMark()->nNode.GetIndex();
        bFirstNode = sal_False;
    }
}
#endif

void SwXMLExport::_ExportAutoStyles()
{
#ifdef XML_CORE_API
    // export all PaMs
    SwPaM *pPaM = pOrigPaM;
    sal_Bool bContinue = sal_True;
    do
    {
        // export PaM content
        ExportCurPaMAutoStyles( bExportWholeDoc );

        bContinue = pPaM->GetNext() != pOrigPaM;

        // the current PaM must be at the start if this loop finishes
        pPaM = (SwPaM *)pPaM->GetNext();
        SetCurPaM( *pPaM, bExportWholeDoc, bExportFirstTableOnly );

    } while( bContinue );

    // text collections
    ((SvXMLAutoStylePool *)pItemSetAutoStylePool)->exportXML( SFX_STYLE_FAMILY_PARA,
                                                       GetParaItemMapper(),
                                                       GetDocHandler(),
                                                       GetTwipUnitConverter(),
                                                       GetNamespaceMap() );

    // char fmts
    ((SvXMLAutoStylePool *)pItemSetAutoStylePool)->exportXML( SFX_STYLE_FAMILY_CHAR,
                                                       GetParaItemMapper(),
                                                       GetDocHandler(),
                                                       GetTwipUnitConverter(),
                                                       GetNamespaceMap() );

    // num rules
    ExportNumRules( sal_True, sal_False );

    // text field styles
//  GetNumberFormatExport().Export( GetNamespaceMap() );

#else
    Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
    Reference < XText > xText = xTextDoc->getText();

    GetTextParagraphExport()->collectFrameBoundToPageAutoStyles();
    GetTextParagraphExport()->collectTextAutoStyles( xText );
    xMasterPageExport->collectAutoStyles( sal_False );
    GetTextParagraphExport()->exportTextAutoStyles();
    exportAutoDataStyles();

#endif

}

void SwXMLExport::_ExportMasterStyles()
{
    xMasterPageExport->exportMasterStyles( sal_False );
}

// ---------------------------------------------------------------------

#ifdef XML_CORE_API
void SwXMLAutoStylePool::exportXML( SvXMLAttributeList& rAttrList,
                            SfxStyleFamily eFamily, const SfxItemSet& rItemSet,
                            const SvXMLUnitConverter& rUnitConverter,
                            const SvXMLNamespaceMap& rNamespaceMap ) const
{
    if( SFX_STYLE_FAMILY_PARA == eFamily )
    {
        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( RES_PARATR_NUMRULE, sal_False,
                                                   &pItem ) )
        {
            OUString sName( rNamespaceMap.GetQNameByKey( XML_NAMESPACE_STYLE,
                    OUString::createFromAscii(sXML_list_style_name) ) );
            rAttrList.AddAttribute( sName,
                                    OUString::createFromAscii( sXML_CDATA ),
                                      ((const SwNumRuleItem *)pItem)->GetValue() );
        }
    }
}

SwXMLAutoStylePool::~SwXMLAutoStylePool()
{
}
#else

class SwXMLAutoStylePoolP : public SvXMLAutoStylePoolP
{
    SvXMLExport& rExport;
    const OUString sListStyleName;
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
            }
        }
    }
}

SwXMLAutoStylePoolP::SwXMLAutoStylePoolP(SvXMLExport& rExp ) :
    SvXMLAutoStylePoolP(),
    rExport( rExp ),
    sListStyleName( RTL_CONSTASCII_USTRINGPARAM( sXML_list_style_name) ),
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
#endif

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/xml/xmlfmte.cxx,v 1.3 2000-10-12 17:30:28 mib Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.2  2000/09/29 10:54:05  mib
      export graphics styles again

      Revision 1.1.1.1  2000/09/18 17:15:00  hr
      initial import

      Revision 1.23  2000/09/18 16:05:06  willem.vandorp
      OpenOffice header added.

      Revision 1.22  2000/09/18 11:58:02  mib
      text frames/graphics import and export continued

      Revision 1.21  2000/09/07 05:29:00  mib
      #78555#: Search paragraph styles by programmatic name instead of UI name

      Revision 1.20  2000/08/24 11:16:41  mib
      text import continued

      Revision 1.19  2000/08/21 11:02:50  dvo
      - fixed data style export (for textfields)

      Revision 1.18  2000/08/10 10:22:16  mib
      #74404#: Adeptions to new XSL/XLink working draft

      Revision 1.17  2000/08/02 14:52:39  mib
      text export continued

      Revision 1.16  2000/07/31 09:42:35  mib
      text export continued

      Revision 1.15  2000/07/27 08:06:34  mib
      text import continued

      Revision 1.14  2000/07/26 05:11:20  mib
      text import/export continued

      Revision 1.13  2000/07/24 10:19:02  dvo
      - textfield export for XML_CORE_API

      Revision 1.12  2000/07/21 12:55:15  mib
      text import/export using StarOffice API

      Revision 1.11  2000/07/10 06:58:49  mib
      text styles

      Revision 1.10  2000/07/07 13:58:36  mib
      text styles using StarOffice API

      Revision 1.9  2000/06/08 09:45:54  aw
      changed to use functionality from xmloff project now

      Revision 1.8  2000/05/03 12:08:05  mib
      unicode

      Revision 1.7  2000/03/13 14:33:44  mib
      UNO3

      Revision 1.6  2000/03/06 10:46:11  mib
      #72585#: toInt32

      Revision 1.5  2000/02/11 14:41:57  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.4  2000/02/07 10:03:28  mib
      #70271#: tables

      Revision 1.3  2000/01/20 10:03:16  mib
      #70271#: Lists reworked

      Revision 1.2  2000/01/12 15:00:23  mib
      #70271#: lists

      Revision 1.1  2000/01/06 15:03:40  mib
      #70271#:separation of text/layout, cond. styles, adaptions to wd-xlink-19991229

      Revision 1.23  1999/12/13 08:28:25  mib
      #70271#: Support for element items added

      Revision 1.22  1999/12/06 11:41:33  mib
      #70258#: Container item for unkown attributes

      Revision 1.21  1999/11/26 11:13:57  mib
      loading of styles only and insert mode

      Revision 1.20  1999/11/22 15:52:34  os
      headers added

      Revision 1.19  1999/11/17 20:08:49  nn
      document language

      Revision 1.18  1999/11/12 14:50:28  mib
      meta import and export reactivated

      Revision 1.17  1999/11/12 11:43:03  mib
      using item mapper, part iii

      Revision 1.16  1999/11/10 15:08:09  mib
      Import now uses XMLItemMapper

      Revision 1.15  1999/11/09 15:40:08  mib
      Using XMLItemMapper for export

      Revision 1.14  1999/11/05 19:44:11  nn
      handle office:meta

      Revision 1.13  1999/11/01 11:38:50  mib
      List style import

      Revision 1.12  1999/10/25 10:41:48  mib
      Using new OUString ASCII methods

      Revision 1.11  1999/10/22 09:49:16  mib
      List style export

      Revision 1.10  1999/10/15 12:37:05  mib
      integrated SvXMLStyle into SvXMLStyleContext

      Revision 1.9  1999/10/08 11:47:45  mib
      moved some file to SVTOOLS/SVX

      Revision 1.8  1999/10/01 13:02:51  mib
      no comparisons between OUString and char* any longer

      Revision 1.7  1999/09/28 10:47:58  mib
      char fmts again

      Revision 1.6  1999/09/28 08:31:15  mib
      char fmts, hints

      Revision 1.5  1999/09/23 11:53:58  mib
      i18n, token maps and hard paragraph attributes

      Revision 1.4  1999/09/22 11:56:57  mib
      string -> wstring

      Revision 1.3  1999/08/19 12:57:42  MIB
      attribute import added


      Rev 1.2   19 Aug 1999 14:57:42   MIB
   attribute import added

      Rev 1.1   18 Aug 1999 17:03:36   MIB
   Style import

      Rev 1.0   13 Aug 1999 16:18:10   MIB
   Initial revision.


*************************************************************************/

