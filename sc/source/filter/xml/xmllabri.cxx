/*************************************************************************
 *
 *  $RCSfile: xmllabri.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dr $ $Date: 2000-11-02 16:39:54 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//___________________________________________________________________

#ifndef SC_XMLLABRI_HXX
#include "xmllabri.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XLABELRANGES_HPP_
#include <com/sun/star/sheet/XLabelRanges.hpp>
#endif

#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif
#ifndef SC_UNONAMES_HXX
#include "unonames.hxx"
#endif
#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::rtl;


//___________________________________________________________________

ScXMLLabelRangesContext::ScXMLLabelRangesContext(
        ScXMLImport& rImport,
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList ):
    SvXMLImportContext( rImport, nPrefix, rLName )
{
}

ScXMLLabelRangesContext::~ScXMLLabelRangesContext()
{
}

SvXMLImportContext* ScXMLLabelRangesContext::CreateChildContext(
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext*     pContext    = NULL;
    const SvXMLTokenMap&    rTokenMap   = GetScImport().GetLabelRangesElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_LABEL_RANGE_ELEM:
            pContext = new ScXMLLabelRangeContext( GetScImport(), nPrefix, rLName, xAttrList );
        break;
    }
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLLabelRangesContext::EndElement()
{
}


//___________________________________________________________________

ScXMLLabelRangeContext::ScXMLLabelRangeContext(
        ScXMLImport& rImport,
        USHORT nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    bColumnOrientation( sal_False )
{
    sal_Int16               nAttrCount      = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap&    rAttrTokenMap   = GetScImport().GetLabelRangeAttrTokenMap();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString    sAttrName   = xAttrList->getNameByIndex( nIndex );
        OUString    sValue      = xAttrList->getValueByIndex( nIndex );
        OUString    aLocalName;
        USHORT      nPrefix     = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_LABEL_RANGE_ATTR_LABEL_RANGE:
                sLabelRangeStr = sValue;
            break;
            case XML_TOK_LABEL_RANGE_ATTR_DATA_RANGE:
                sDataRangeStr = sValue;
            break;
            case XML_TOK_LABEL_RANGE_ATTR_ORIENTATION:
                bColumnOrientation = (sValue.compareToAscii( sXML_column ) == 0);
            break;
        }
    }
}

ScXMLLabelRangeContext::~ScXMLLabelRangeContext()
{
}

SvXMLImportContext* ScXMLLabelRangeContext::CreateChildContext(
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLLabelRangeContext::EndElement()
{
    uno::Reference< beans::XPropertySet > xPropSet( GetScImport().GetModel(), uno::UNO_QUERY );
    if( xPropSet.is() )
    {
        uno::Any aAny = xPropSet->getPropertyValue( bColumnOrientation ?
            OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_COLLABELRNG ) ) :
            OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_ROWLABELRNG ) ) );
        uno::Reference< sheet::XLabelRanges > xLabelRanges;
        if( aAny >>= xLabelRanges )
        {
            table::CellRangeAddress aLabelRange;
            ScXMLConverter::GetRangeFromString( aLabelRange, sLabelRangeStr, GetScImport().GetDocument() );
            table::CellRangeAddress aDataRange;
            ScXMLConverter::GetRangeFromString( aDataRange, sDataRangeStr, GetScImport().GetDocument() );
            xLabelRanges->addNew( aLabelRange, aDataRange );
        }
    }
}

