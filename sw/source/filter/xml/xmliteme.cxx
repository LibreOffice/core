/*************************************************************************
 *
 *  $RCSfile: xmliteme.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:59 $
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
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _SHL_HXX //autogen wg. SHL_WRITER
#include <tools/shl.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
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
#ifndef _XMLOFF_XMLTABE_HXX
#include <xmloff/xmltabe.hxx>
#endif
#ifndef _XMLBRSHE_HXX
#include "xmlbrshe.hxx"
#endif

#ifndef _SVX_TSPTITEM_HXX
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_UNOMID_HXX
#include <svx/unomid.hxx>
#endif

#ifndef _VCL_FLDUNIT_HXX
#include <vcl/fldunit.hxx>
#endif

#ifndef _SWMODULE_HXX //autogen wg. SW_MOD
#include <swmodule.hxx>
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _FMTORNT_HXX
#include "fmtornt.hxx"
#endif
#ifndef _UNOMID_H
#include <unomid.h>
#endif
#ifndef _FRMFMT_HXX
#include "frmfmt.hxx"
#endif
#ifndef _FMTFSIZE_HXX
#include "fmtfsize.hxx"
#endif
#ifndef _SWRECT_HXX
#include "swrect.hxx"
#endif

#ifndef _XMLDROPE_HXX
#include "xmldrope.hxx"
#endif
#ifndef _XMLEXP_HXX
#include "xmlexp.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#ifdef XML_CORE_API
extern SvXMLItemMapEntry aXMLParaItemMap[];
#endif
extern SvXMLItemMapEntry aXMLTableItemMap[];
extern SvXMLItemMapEntry aXMLTableRowItemMap[];
extern SvXMLItemMapEntry aXMLTableCellItemMap[];

#ifdef XML_CORE_API
class SwXMLTextItemMapper_Impl: public SvXMLExportItemMapper
{
protected:
    SvXMLUnitConverter& mrUnitConverter;
    const Reference< xml::sax::XDocumentHandler > & mrHandler;
    SvxXMLTabStopExport maTabStopExport;

public:

    SwXMLTextItemMapper_Impl(
            SvXMLItemMapEntriesRef rMapEntries,
            SvXMLUnitConverter& rUnitConverter,
            const Reference< xml::sax::XDocumentHandler > & rHandler );

    virtual ~SwXMLTextItemMapper_Impl();

    virtual void handleElementItem(
            const Reference< xml::sax::XDocumentHandler > & rHandler,
            const SvXMLItemMapEntry& rEntry,
            const SfxPoolItem& rItem,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            const SfxItemSet& rSet,
            sal_uInt16 nFlags ) const;
};

SwXMLTextItemMapper_Impl::SwXMLTextItemMapper_Impl(
                SvXMLItemMapEntriesRef rMapEntries,
                SvXMLUnitConverter& rUnitConverter,
                const Reference< xml::sax::XDocumentHandler > & rHandler ) :
    SvXMLExportItemMapper( rMapEntries ),
    mrUnitConverter(rUnitConverter),
    mrHandler( rHandler ),
    maTabStopExport( rHandler, rUnitConverter )
{
}

SwXMLTextItemMapper_Impl::~SwXMLTextItemMapper_Impl()
{
}

/** this method is called for every item that has the
    MID_FLAG_ELEMENT_EXPORT flag set */
void SwXMLTextItemMapper_Impl::handleElementItem(
    const Reference< xml::sax::XDocumentHandler > & rHandler,
    const SvXMLItemMapEntry& rEntry,
    const SfxPoolItem& rItem,
    const SvXMLUnitConverter& rUnitConverter,
    const SvXMLNamespaceMap& rNamespaceMap,
    const SfxItemSet&,
    sal_uInt16 ) const
{
    switch( rEntry.nWhichId )
    {
    case RES_PARATR_TABSTOP:
        {
            SvxXMLTabStopExport aTabStopExport( rHandler, rUnitConverter );

            Any aAny;
            rItem.QueryValue( aAny, 0 );
            aTabStopExport.Export( aAny, rNamespaceMap );
        }
        break;
    case RES_PARATR_DROP:
        {
            SwXMLFmtDropExport aFmtDropExport( rHandler, rUnitConverter );
            aFmtDropExport.exportXML( (const SwFmtDrop&)rItem, rNamespaceMap );
        }
        break;
    case RES_BACKGROUND:
        {
            SwXMLBrushItemExport aBrushItemExport( rHandler, rUnitConverter );
            aBrushItemExport.exportXML( (const SvxBrushItem&)rItem,
                                         rNamespaceMap );
        }
        break;
    }
}
#endif

// ----------------------------------------------------------------------------

class SwXMLTableItemMapper_Impl: public SvXMLExportItemMapper
{
    const OUString sCDATA;

protected:
    SvXMLUnitConverter& mrUnitConverter;
    const Reference< xml::sax::XDocumentHandler > & mrHandler;

    sal_uInt32 nAbsWidth;

    void AddAttribute( sal_uInt16 nPrefix, const sal_Char *pLName,
                       const OUString& rValue,
                       const SvXMLNamespaceMap& rNamespaceMap,
                       SvXMLAttributeList& rAttrList ) const;

public:

    SwXMLTableItemMapper_Impl(
            SvXMLItemMapEntriesRef rMapEntries,
            SvXMLUnitConverter& rUnitConverter,
            const Reference< xml::sax::XDocumentHandler > & rHandler );

    virtual ~SwXMLTableItemMapper_Impl();

    virtual void handleSpecialItem( SvXMLAttributeList& rAttrList,
                                    const SvXMLItemMapEntry& rEntry,
                                    const SfxPoolItem& rItem,
                                    const SvXMLUnitConverter& rUnitConverter,
                                    const SvXMLNamespaceMap& rNamespaceMap,
                                    const SfxItemSet *pSet = NULL ) const;

    virtual void handleElementItem(
            const Reference< xml::sax::XDocumentHandler > & rHandler,
            const SvXMLItemMapEntry& rEntry,
            const SfxPoolItem& rItem,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            const SfxItemSet& rSet,
            sal_uInt16 nFlags ) const;

    inline void SetAbsWidth( sal_uInt32 nAbs );
};

SwXMLTableItemMapper_Impl::SwXMLTableItemMapper_Impl(
        SvXMLItemMapEntriesRef rMapEntries,
        SvXMLUnitConverter& rUnitConverter,
        const Reference< xml::sax::XDocumentHandler > & rHandler ) :
    SvXMLExportItemMapper( rMapEntries ),
    sCDATA( OUString::createFromAscii( sXML_CDATA ) ),
    mrUnitConverter(rUnitConverter),
    mrHandler( rHandler ),
    nAbsWidth( USHRT_MAX )
{
}

SwXMLTableItemMapper_Impl::~SwXMLTableItemMapper_Impl()
{
}

void SwXMLTableItemMapper_Impl::AddAttribute( sal_uInt16 nPrefix,
        const sal_Char *pLName,
        const OUString& rValue,
        const SvXMLNamespaceMap& rNamespaceMap,
        SvXMLAttributeList& rAttrList ) const
{
    OUString sLName( OUString::createFromAscii(pLName) );
    OUString sName( rNamespaceMap.GetQNameByKey( nPrefix, sLName ) );
    rAttrList.AddAttribute( sName, sCDATA, rValue );
}

void SwXMLTableItemMapper_Impl::handleSpecialItem(
        SvXMLAttributeList& rAttrList,
        const SvXMLItemMapEntry& rEntry,
        const SfxPoolItem& rItem,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        const SfxItemSet *pSet ) const
{
    switch( rEntry.nWhichId )
    {
    case RES_LR_SPACE:
        {
            const SfxPoolItem *pItem;
            if( pSet &&
                SFX_ITEM_SET == pSet->GetItemState( RES_HORI_ORIENT, sal_True,
                                                    &pItem ) )
            {
                SwHoriOrient eHoriOrient =
                    ((const SwFmtHoriOrient *)pItem)->GetHoriOrient();
                sal_Bool bExport = sal_False;
                sal_uInt32 nMemberId = rEntry.nMemberId & MID_FLAG_MASK;
                switch( nMemberId )
                {
                case MID_L_MARGIN:
                    bExport = HORI_NONE == eHoriOrient ||
                              HORI_LEFT_AND_WIDTH == eHoriOrient;
                    break;
                case MID_R_MARGIN:
                    bExport = HORI_NONE == eHoriOrient;
                    break;
                }
                OUString sValue;
                if( bExport && rItem.exportXML( sValue, nMemberId,
                                                rUnitConverter ) )
                {
                    AddAttribute( rEntry.nNameSpace, rEntry.pLocalName, sValue,
                                  rNamespaceMap, rAttrList );
                }
            }
        }
        break;

    case RES_FRM_SIZE:
        {
            sal_uInt32 nMemberId = rEntry.nMemberId & MID_FLAG_MASK;
            switch( nMemberId )
            {
            case MID_FRMSIZE_WIDTH:
                if( nAbsWidth )
                {
                    OUStringBuffer sBuffer;
                    rUnitConverter.convertMeasure( sBuffer, nAbsWidth );
                    AddAttribute( rEntry.nNameSpace, rEntry.pLocalName,
                                  sBuffer.makeStringAndClear(),
                                  rNamespaceMap, rAttrList );
                }
                break;
            case MID_FRMSIZE_REL_WIDTH:
                {
                    OUString sValue;
                    if( rItem.exportXML( sValue, nMemberId, rUnitConverter ) )
                    {
                        AddAttribute( rEntry.nNameSpace, rEntry.pLocalName,
                                      sValue, rNamespaceMap, rAttrList );
                    }
                }
                break;
            }
        }
        break;
    }
}

/** this method is called for every item that has the
    MID_FLAG_ELEMENT_EXPORT flag set */
void SwXMLTableItemMapper_Impl::handleElementItem(
        const Reference< xml::sax::XDocumentHandler > & rHandler,
        const SvXMLItemMapEntry& rEntry,
        const SfxPoolItem& rItem,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        const SfxItemSet&,
        sal_uInt16 ) const
{
    switch( rEntry.nWhichId )
    {
    case RES_BACKGROUND:
        {
            SwXMLBrushItemExport aBrushItemExport( rHandler, rUnitConverter );
            aBrushItemExport.exportXML( (const SvxBrushItem&)rItem,
                                         rNamespaceMap );
        }
        break;
    }
}

inline void SwXMLTableItemMapper_Impl::SetAbsWidth( sal_uInt32 nAbs )
{
    nAbsWidth = nAbs;
}

// ----------------------------------------------------------------------------

void SwXMLExport::_InitItemExport()
{
    pTwipUnitConv = new SvXMLUnitConverter( MAP_TWIP,
                                GetMM100UnitConverter().getXMLMeasureUnit() );

#ifdef XML_CORE_API
    SvXMLItemMapEntriesRef xParaItemMap =
        new SvXMLItemMapEntries( aXMLParaItemMap );

    pParaItemMapper = new SwXMLTextItemMapper_Impl( xParaItemMap,
                                                      *pTwipUnitConv,
                                                      GetDocHandler() );
#endif

    xTableItemMap = new SvXMLItemMapEntries( aXMLTableItemMap );
    xTableRowItemMap = new SvXMLItemMapEntries( aXMLTableRowItemMap );
    xTableCellItemMap = new SvXMLItemMapEntries( aXMLTableCellItemMap );

    pTableItemMapper = new SwXMLTableItemMapper_Impl( xTableItemMap,
                                                       *pTwipUnitConv,
                                                       GetDocHandler() );
}

void SwXMLExport::_FinitItemExport()
{
#ifdef XML_CORE_API
    delete pParaItemMapper;
#endif
    delete pTableItemMapper;
    delete pTwipUnitConv;
}

void SwXMLExport::ExportTableFmt( const SwFrmFmt& rFmt, sal_uInt32 nAbsWidth )
{
    ((SwXMLTableItemMapper_Impl *)pTableItemMapper)
        ->SetAbsWidth( nAbsWidth );
    ExportFmt( rFmt, sXML_table );
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/xml/xmliteme.cxx,v 1.1.1.1 2000-09-18 17:14:59 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.10  2000/09/18 16:05:07  willem.vandorp
      OpenOffice header added.

      Revision 1.9  2000/08/02 14:52:39  mib
      text export continued

      Revision 1.8  2000/07/31 09:42:35  mib
      text export continued

      Revision 1.7  2000/06/08 09:45:54  aw
      changed to use functionality from xmloff project now

      Revision 1.6  2000/05/03 12:08:05  mib
      unicode

      Revision 1.5  2000/03/13 14:33:44  mib
      UNO3

      Revision 1.4  2000/02/11 14:41:45  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.3  2000/02/07 10:03:28  mib
      #70271#: tables

      Revision 1.2  2000/01/20 14:03:58  mib
      #70271#: deletion of unit converter

      Revision 1.1  1999/12/14 07:32:49  mib
      #70271#: XML import/export of drop cap/register/language item, splitted swxmlat


*************************************************************************/

