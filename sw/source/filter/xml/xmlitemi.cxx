/*************************************************************************
 *
 *  $RCSfile: xmlitemi.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:15:00 $
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

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RSCSFX_HXX
#include <rsc/rscsfx.hxx>
#endif

#ifndef _XMLOFF_XMLITMAP_HXX
#include <xmloff/xmlitmap.hxx>
#endif
#ifndef _XMLOFF_XMLIMPIT_HXX
#include <xmloff/xmlimpit.hxx>
#endif
#ifndef _XMLOFF_XMLITEM_HXX
#include <xmloff/xmlitem.hxx>
#endif
#ifndef _XMLOFF_I18NMAP_HXX
#include <xmloff/i18nmap.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX
#include <xmloff/families.hxx>
#endif

#ifndef _SVX_UNOMID_HXX
#include <svx/unomid.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif

#ifndef _XMLOFF_XMLTABI_HXX
//#include <xmloff/xmltabi.hxx>
#endif

#ifndef _XMLBRSHI_HXX
#include "xmlbrshi.hxx"
#endif

#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _UNOMID_H
#include <unomid.h>
#endif

#ifndef _XMDROPI_HXX
#include "xmldropi.hxx"
#endif
#ifdef XML_CORE_API
#ifndef _XMLTBLI_HXX
#include "xmltbli.hxx"
#endif
#endif
#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#ifdef XML_CORE_API
extern SvXMLItemMapEntry aXMLParaItemMap[];
#endif
extern SvXMLItemMapEntry aXMLTableItemMap[];
extern SvXMLItemMapEntry aXMLTableColItemMap[];
extern SvXMLItemMapEntry aXMLTableRowItemMap[];
extern SvXMLItemMapEntry aXMLTableCellItemMap[];

#ifdef XML_CORE_API
class SwXMLImportTextItemMapper_Impl: public SvXMLImportItemMapper
{
    SvI18NMap&  rI18NMap;
    SwDoc& rDoc;

public:

    SwXMLImportTextItemMapper_Impl( SvXMLItemMapEntriesRef rMapEntries,
                                SwDoc& rD, SvI18NMap& rI18NMap );
    virtual ~SwXMLImportTextItemMapper_Impl();

    virtual sal_Bool handleSpecialItem( const SvXMLItemMapEntry& rEntry,
                                SfxPoolItem& rItem,
                                SfxItemSet& rSet,
                                const OUString& rValue,
                                const SvXMLUnitConverter& rUnitConverter,
                                const SvXMLNamespaceMap& rNamespaceMap ) const;

    virtual void finished( SfxItemSet& rSet ) const;
};

SwXMLImportTextItemMapper_Impl::SwXMLImportTextItemMapper_Impl(
                                        SvXMLItemMapEntriesRef rMapEntries,
                                        SwDoc& rD, SvI18NMap& rI18NM ) :
    SvXMLImportItemMapper( rMapEntries, RES_UNKNOWNATR_CONTAINER),
    rDoc( rD ),
    rI18NMap( rI18NM )
{
}

SwXMLImportTextItemMapper_Impl::~SwXMLImportTextItemMapper_Impl()
{
}

sal_Bool SwXMLImportTextItemMapper_Impl::handleSpecialItem(
                                        const SvXMLItemMapEntry& rEntry,
                                        SfxPoolItem& rItem,
                                        SfxItemSet& rItemSet,
                                        const OUString& rValue,
                                        const SvXMLUnitConverter& rUnitConv,
                                        const SvXMLNamespaceMap& ) const
{
    sal_Bool bRet = sal_False;
    switch( rItem.Which() )
    {
    case RES_TXTATR_CHARFMT:
        DBG_ASSERT( !this,
    "SwXMLImportTextItemMapper_Impl::handleSpecialItem: char fmt unexpected" );
        break;

    case RES_PARATR_NUMRULE:
        if( rValue.getLength() )
        {
            OUString sName( rI18NMap.Get( SFX_STYLE_FAMILY_PSEUDO, rValue ) );
            ((SwNumRuleItem&)rItem).SetValue( sName );
            bRet = sal_True;
        }
        break;

    case RES_CHRATR_LANGUAGE:
        // If the item is not contained in the item set, its the default
        // value. To set language and contry individually, we have to clear
        // its value.
        if( SFX_ITEM_SET != rItemSet.GetItemState( RES_CHRATR_LANGUAGE,
                                                   sal_False ) )
        {
            ((SvxLanguageItem &)rItem).SetLanguage( LANGUAGE_NONE );
        }
        bRet = rItem.importXML( rValue, rEntry.nMemberId, rUnitConv );
        break;
    }

    return bRet;
}

void SwXMLImportTextItemMapper_Impl::finished( SfxItemSet& rSet ) const
{
    const SfxPoolItem *pItem;

    if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_FONT, sal_False,
                                           &pItem ) )
    {
        const SvxFontItem *pFont = (const SvxFontItem *)pItem;
        // delete font items that have no family name
         if( 0 == pFont->GetFamilyName().Len() )
            rSet.ClearItem( RES_CHRATR_FONT );
        else if( RTL_TEXTENCODING_DONTKNOW == pFont->GetCharSet() )
        {
            SvxFontItem aFont( *pFont );
            aFont.GetCharSet() = gsl_getSystemTextEncoding();
            rSet.Put( aFont );
        }
    }

    // ensure that box item have a distance to a border.
    if( SFX_ITEM_SET == rSet.GetItemState( RES_BOX, sal_False, &pItem ) )
    {
        const SvxBoxItem *pBox = (const SvxBoxItem *)pItem;
        sal_uInt16 aLines[4] = { BOX_LINE_TOP, BOX_LINE_BOTTOM,
                             BOX_LINE_LEFT, BOX_LINE_RIGHT };
        sal_uInt16 i;
        for( i=0; i<4; i++ )
        {
            if( pBox->GetLine( aLines[i] ) &&
                pBox->GetDistance( aLines[i] ) < MIN_BORDER_DIST )
                break;
        }
        if( i < 4 )
        {
            SvxBoxItem aBox( *pBox );
            for( /*i=0*/; i<4; i++ )    // i points to the mod. line
            {
                if( aBox.GetLine( aLines[i] ) &&
                    aBox.GetDistance( aLines[i] ) < MIN_BORDER_DIST )
                    aBox.SetDistance( MIN_BORDER_DIST, aLines[i] );
            }
            rSet.Put( aBox );
        }
    }
}
#endif

// ---------------------------------------------------------------------

class SwXMLImportTableItemMapper_Impl: public SvXMLImportItemMapper
{

public:

    SwXMLImportTableItemMapper_Impl( SvXMLItemMapEntriesRef rMapEntries );
    virtual ~SwXMLImportTableItemMapper_Impl();

    virtual sal_Bool handleSpecialItem( const SvXMLItemMapEntry& rEntry,
                                SfxPoolItem& rItem,
                                SfxItemSet& rSet,
                                const OUString& rValue,
                                const SvXMLUnitConverter& rUnitConverter,
                                const SvXMLNamespaceMap& rNamespaceMap ) const;
    virtual void finished( SfxItemSet& rSet ) const;
};

SwXMLImportTableItemMapper_Impl::SwXMLImportTableItemMapper_Impl(
                                        SvXMLItemMapEntriesRef rMapEntries ) :
    SvXMLImportItemMapper( rMapEntries, RES_UNKNOWNATR_CONTAINER)
{
}

SwXMLImportTableItemMapper_Impl::~SwXMLImportTableItemMapper_Impl()
{
}

sal_Bool SwXMLImportTableItemMapper_Impl::handleSpecialItem(
                                        const SvXMLItemMapEntry& rEntry,
                                        SfxPoolItem& rItem,
                                        SfxItemSet& rItemSet,
                                        const OUString& rValue,
                                        const SvXMLUnitConverter& rUnitConv,
                                        const SvXMLNamespaceMap& ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nMemberId = rEntry.nMemberId & MID_FLAG_MASK;
    switch( rItem.Which() )
    {
    case RES_FRM_SIZE:
        switch( nMemberId )
        {
        case MID_FRMSIZE_COL_WIDTH:
            // If the item is existing already, a relative value has been set
            // already that must be preserved.
            if( SFX_ITEM_SET != rItemSet.GetItemState( RES_FRM_SIZE,
                                                       sal_False ) )
                bRet = rItem.importXML( rValue, nMemberId, rUnitConv );
            break;
        }
    }

    return bRet;
}

void SwXMLImportTableItemMapper_Impl::finished( SfxItemSet& rSet ) const
{
#if 0
    const SfxPoolItem *pItem;

    // ensure that box item have a distance to a border.
    if( SFX_ITEM_SET == rSet.GetItemState( RES_BOX, sal_False, &pItem ) )
    {
        const SvxBoxItem *pBox = (const SvxBoxItem *)pItem;
        sal_uInt16 aLines[4] = { BOX_LINE_TOP, BOX_LINE_BOTTOM,
                             BOX_LINE_LEFT, BOX_LINE_RIGHT };
        sal_uInt16 i;
        for( i=0; i<4; i++ )
        {
            if( pBox->GetLine( aLines[i] ) &&
                pBox->GetDistance( aLines[i] ) < MIN_BORDER_DIST )
                break;
        }
        if( i < 4 )
        {
            SvxBoxItem aBox( *pBox );
            for( /*i=0*/; i<4; i++ )    // i points to the mod. line
            {
                if( aBox.GetLine( aLines[i] ) &&
                    aBox.GetDistance( aLines[i] ) < MIN_BORDER_DIST )
                    aBox.SetDistance( MIN_BORDER_DIST, aLines[i] );
            }
            rSet.Put( aBox );
        }
    }
#endif
}

// ---------------------------------------------------------------------

class SwXMLItemSetContext_Impl : public SvXMLItemSetContext
{
    SvXMLImportContextRef xBackground;
#ifdef XML_CORE_API
    SvXMLImportContextRef xTabStop;
    SvXMLImportContextRef xDropCap;
#endif

public:
    SwXMLItemSetContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
                  const OUString& rLName,
                  const Reference< xml::sax::XAttributeList > & xAttrList,
                  SfxItemSet&  rItemSet,
                  const SvXMLImportItemMapper& rIMapper,
                  const SvXMLUnitConverter& rUnitConv );
    virtual ~SwXMLItemSetContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                   const OUString& rLocalName,
                   const Reference< xml::sax::XAttributeList > & xAttrList,
                   SfxItemSet&  rItemSet,
                   const SvXMLItemMapEntry& rEntry,
                   const SvXMLUnitConverter& rUnitConv );
};

SwXMLItemSetContext_Impl::SwXMLItemSetContext_Impl(
                 SwXMLImport& rImport, sal_uInt16 nPrfx,
                 const OUString& rLName,
                 const Reference< xml::sax::XAttributeList > & xAttrList,
                 SfxItemSet&  rItemSet,
                 const SvXMLImportItemMapper& rIMapper,
                 const SvXMLUnitConverter& rUnitConv ) :
    SvXMLItemSetContext( rImport, nPrfx, rLName, xAttrList,
                         rItemSet, rIMapper, rUnitConv )
{
}

SwXMLItemSetContext_Impl::~SwXMLItemSetContext_Impl()
{
#ifdef XML_CORE_API
    if( xTabStop.Is() )
    {
        Any aAny;
        ((SvxXMLTabStopImportContext*)&xTabStop)->fillTabStops( aAny );
        SvxTabStopItem aTabStop( RES_PARATR_TABSTOP );
        ((SfxPoolItem *)&aTabStop)->PutValue( aAny, 0 );
        rItemSet.Put( aTabStop );
    }
    if( xDropCap.Is() )
    {
        const SwFmtDrop& rItem =
            ((SwXMLFmtDropImportContext*)&xDropCap)->GetItem();
        rItemSet.Put( rItem );
    }
#endif
    if( xBackground.Is() )
    {
        const SvxBrushItem& rItem =
            ((SwXMLBrushItemImportContext*)&xBackground)->GetItem();
        rItemSet.Put( rItem );
    }
}

SvXMLImportContext *SwXMLItemSetContext_Impl::CreateChildContext(
                   sal_uInt16 nPrefix,
                   const OUString& rLocalName,
                   const Reference< xml::sax::XAttributeList > & xAttrList,
                   SfxItemSet&  rItemSet,
                   const SvXMLItemMapEntry& rEntry,
                   const SvXMLUnitConverter& rUnitConv )
{
    SvXMLImportContext *pContext = 0;

    switch( rEntry.nWhichId )
    {
#ifdef XML_CORE_API
    case RES_PARATR_TABSTOP:
        pContext = new SvxXMLTabStopImportContext( GetImport(), nPrefix,
                                                   rLocalName, rUnitConv );
        xTabStop = pContext;
        break;

    case RES_PARATR_DROP:
        pContext = new SwXMLFmtDropImportContext( (SwXMLImport&)GetImport(),
                                                  nPrefix, rLocalName,
                                                  xAttrList, rUnitConv,
                                                  RES_PARATR_DROP );
        xDropCap = pContext;
        break;
#endif

    case RES_BACKGROUND:
        {
            const SfxPoolItem *pItem;
            if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BACKGROUND,
                                                       sal_False, &pItem ) )
            {
                pContext = new SwXMLBrushItemImportContext(
                                GetImport(), nPrefix, rLocalName, xAttrList,
                                rUnitConv, *(const SvxBrushItem *)pItem );
            }
            else
            {
                pContext = new SwXMLBrushItemImportContext(
                                GetImport(), nPrefix, rLocalName, xAttrList,
                                rUnitConv, RES_BACKGROUND );
            }
            xBackground = pContext;
        }
        break;
    }

    if( !pContext )
        pContext = SvXMLItemSetContext::CreateChildContext( nPrefix, rLocalName,
                                                            xAttrList, rItemSet,
                                                            rEntry, rUnitConv );

    return pContext;
}

// ---------------------------------------------------------------------

void SwXMLImport::_InitItemImport()
{
    pTwipUnitConv = new SvXMLUnitConverter( MAP_TWIP, MAP_TWIP );

#ifdef XML_CORE_API
    SvXMLItemMapEntriesRef xParaItemMapEntries =
        new SvXMLItemMapEntries( aXMLParaItemMap );

    pParaItemMapper = new SwXMLImportTextItemMapper_Impl( xParaItemMapEntries,
                                                      GetDoc(), GetI18NMap() );
#endif

    xTableItemMap = new SvXMLItemMapEntries( aXMLTableItemMap );
    xTableColItemMap = new SvXMLItemMapEntries( aXMLTableColItemMap );
    xTableRowItemMap = new SvXMLItemMapEntries( aXMLTableRowItemMap );
    xTableCellItemMap = new SvXMLItemMapEntries( aXMLTableCellItemMap );

    pTableItemMapper = new SwXMLImportTableItemMapper_Impl( xTableItemMap );
}

void SwXMLImport::_FinitItemImport()
{
#ifdef XML_CORE_API
    delete pParaItemMapper;
#endif
    delete pTableItemMapper;
    delete pTwipUnitConv;
}

#ifdef XML_CORE_API
SvXMLImportContext *SwXMLImport::CreateParaItemImportContext(
                  sal_uInt16 nPrefix,
                  const OUString& rLocalName,
                  const Reference< xml::sax::XAttributeList > & xAttrList,
                  SfxItemSet& rItemSet )
{
    return new SwXMLItemSetContext_Impl( *this, nPrefix, rLocalName,
                                            xAttrList, rItemSet,
                                            GetParaItemMapper(),
                                            GetTwipUnitConverter() );
}
#endif

SvXMLImportContext *SwXMLImport::CreateTableItemImportContext(
                  sal_uInt16 nPrefix,
                  const OUString& rLocalName,
                  const Reference< xml::sax::XAttributeList > & xAttrList,
#ifdef XML_CORE_API
                  sal_uInt16 nSubFamily,
#else
                  sal_uInt16 nFamily,
#endif
                  SfxItemSet& rItemSet )
{
    SvXMLItemMapEntriesRef xItemMap;

#ifdef XML_CORE_API
    switch( nSubFamily )
#else
    switch( nFamily )
#endif
    {
#ifdef XML_CORE_API
    case SW_STYLE_SUBFAMILY_TABLE:
#else
    case XML_STYLE_FAMILY_TABLE_TABLE:
#endif
        xItemMap = xTableItemMap;
        break;
#ifdef XML_CORE_API
    case SW_STYLE_SUBFAMILY_TABLE_COL:
#else
    case XML_STYLE_FAMILY_TABLE_COLUMN:
#endif
        xItemMap = xTableColItemMap;
        break;
#ifdef XML_CORE_API
    case SW_STYLE_SUBFAMILY_TABLE_LINE:
#else
    case XML_STYLE_FAMILY_TABLE_ROW:
#endif
        xItemMap = xTableRowItemMap;
        break;
#ifdef XML_CORE_API
    case SW_STYLE_SUBFAMILY_TABLE_BOX:
#else
    case XML_STYLE_FAMILY_TABLE_CELL:
#endif
        xItemMap = xTableCellItemMap;
        break;
    }

    pTableItemMapper->setMapEntries( xItemMap );

    return new SwXMLItemSetContext_Impl( *this, nPrefix, rLocalName,
                                            xAttrList, rItemSet,
                                            GetTableItemMapper(),
                                            GetTwipUnitConverter() );
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/xml/xmlitemi.cxx,v 1.1.1.1 2000-09-18 17:15:00 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.11  2000/09/18 16:05:07  willem.vandorp
      OpenOffice header added.

      Revision 1.10  2000/08/24 11:16:41  mib
      text import continued

      Revision 1.9  2000/08/02 14:52:39  mib
      text export continued

      Revision 1.8  2000/06/08 09:45:54  aw
      changed to use functionality from xmloff project now

      Revision 1.7  2000/05/03 12:08:05  mib
      unicode

      Revision 1.6  2000/03/13 14:33:44  mib
      UNO3

      Revision 1.5  2000/02/17 14:40:30  mib
      #70271#: XML table import

      Revision 1.3  2000/01/20 14:03:57  mib
      #70271#: deletion of unit converter

      Revision 1.2  2000/01/06 15:08:28  mib
      #70271#:separation of text/layout, cond. styles, adaptions to wd-xlink-19991229

      Revision 1.1  1999/12/14 07:32:58  mib
      #70271#: XML import/export of drop cap/register/language item, splitted swxmlat


*************************************************************************/

