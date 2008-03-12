/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLCellRangeSourceContext.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 13:14:46 $
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
#include "precompiled_sc.hxx"



//___________________________________________________________________

#ifndef _SC_XMLCELLRANGESOURCECONTEXT_HXX
#include "XMLCellRangeSourceContext.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif

using ::rtl::OUString;
using namespace ::com::sun::star;


//___________________________________________________________________

ScMyImpCellRangeSource::ScMyImpCellRangeSource() :
    nColumns( 0 ),
    nRows( 0 ),
    nRefresh( 0 )
{
}


//___________________________________________________________________

ScXMLCellRangeSourceContext::ScXMLCellRangeSourceContext(
        ScXMLImport& rImport,
        USHORT nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList,
        ScMyImpCellRangeSource* pCellRangeSource ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    if( !xAttrList.is() ) return;

    sal_Int16               nAttrCount      = xAttrList->getLength();
    const SvXMLTokenMap&    rAttrTokenMap   = GetScImport().GetTableCellRangeSourceAttrTokenMap();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; ++nIndex )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( nIndex ));
        const OUString& sValue(xAttrList->getValueByIndex( nIndex ));
        OUString aLocalName;
        USHORT nPrefix      = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_NAME:
                pCellRangeSource->sSourceStr = sValue;
            break;
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_FILTER_NAME:
                pCellRangeSource->sFilterName = sValue;
            break;
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_FILTER_OPTIONS:
                pCellRangeSource->sFilterOptions = sValue;
            break;
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_HREF:
                pCellRangeSource->sURL = GetScImport().GetAbsoluteReference(sValue);
            break;
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_LAST_COLUMN:
            {
                sal_Int32 nValue;
                if( SvXMLUnitConverter::convertNumber( nValue, sValue, 1 ) )
                    pCellRangeSource->nColumns = nValue;
                else
                    pCellRangeSource->nColumns = 1;
            }
            break;
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_LAST_ROW:
            {
                sal_Int32 nValue;
                if( SvXMLUnitConverter::convertNumber( nValue, sValue, 1 ) )
                    pCellRangeSource->nRows = nValue;
                else
                    pCellRangeSource->nRows = 1;
            }
            break;
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_REFRESH_DELAY:
            {
                double fTime;
                if( SvXMLUnitConverter::convertTime( fTime, sValue ) )
                    pCellRangeSource->nRefresh = Max( (sal_Int32)(fTime * 86400.0), (sal_Int32)0 );
            }
            break;
        }
    }
}

ScXMLCellRangeSourceContext::~ScXMLCellRangeSourceContext()
{
}

SvXMLImportContext *ScXMLCellRangeSourceContext::CreateChildContext(
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLCellRangeSourceContext::EndElement()
{
}

