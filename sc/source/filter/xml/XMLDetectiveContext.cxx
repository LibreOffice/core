/*************************************************************************
 *
 *  $RCSfile: XMLDetectiveContext.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dr $ $Date: 2000-11-10 16:56:12 $
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

#ifndef _SC_XMLDETECTIVECONTEXT_HXX
#include "XMLDetectiveContext.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif

#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;


//___________________________________________________________________

ScMyImpDetectiveObj::ScMyImpDetectiveObj() :
    aSourceRange(),
    eObjType( SC_DETOBJ_NONE ),
    bHasError( sal_False )
{
}


//___________________________________________________________________

ScXMLDetectiveContext::ScXMLDetectiveContext(
        ScXMLImport& rImport,
        USHORT nPrfx,
        const OUString& rLName,
        ScMyImpDetectiveObjVec& rNewDetectiveObjVec ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rDetectiveObjVec( rNewDetectiveObjVec )
{
}

ScXMLDetectiveContext::~ScXMLDetectiveContext()
{
}

SvXMLImportContext *ScXMLDetectiveContext::CreateChildContext(
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext*     pContext    = NULL;
    const SvXMLTokenMap&    rTokenMap   = GetScImport().GetDetectiveElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DETECTIVE_ELEM_HIGHLIGHTED:
            pContext = new ScXMLDetectiveHighlightedContext( GetScImport(), nPrefix, rLName, xAttrList, rDetectiveObjVec );
        break;
        case XML_TOK_DETECTIVE_ELEM_OPERATION:
            pContext = new ScXMLDetectiveOperationContext( GetScImport(), nPrefix, rLName, xAttrList );
        break;
    }
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDetectiveContext::EndElement()
{
}


//___________________________________________________________________

ScXMLDetectiveHighlightedContext::ScXMLDetectiveHighlightedContext(
        ScXMLImport& rImport,
        USHORT nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList,
        ScMyImpDetectiveObjVec& rNewDetectiveObjVec ):
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rDetectiveObjVec( rNewDetectiveObjVec ),
    aDetectiveObj(),
    bValid( sal_False )
{
    if( !xAttrList.is() ) return;

    sal_Int16               nAttrCount      = xAttrList->getLength();
    const SvXMLTokenMap&    rAttrTokenMap   = GetScImport().GetDetectiveHighlightedAttrTokenMap();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString sAttrName  = xAttrList->getNameByIndex( nIndex );
        OUString sValue     = xAttrList->getValueByIndex( nIndex );
        OUString aLocalName;
        USHORT nPrefix      = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_CELL_RANGE:
                ScXMLConverter::GetRangeFromString( aDetectiveObj.aSourceRange, sValue, GetScImport().GetDocument() );
                bValid = sal_True;
            break;
            case XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_DIRECTION:
                aDetectiveObj.eObjType = ScXMLConverter::GetDetObjTypeFromString( sValue );
            break;
            case XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_CONTAINS_ERROR:
                aDetectiveObj.bHasError = (sValue.compareToAscii( sXML_true ) == 0);
            break;
        }
    }
}

ScXMLDetectiveHighlightedContext::~ScXMLDetectiveHighlightedContext()
{
}

SvXMLImportContext *ScXMLDetectiveHighlightedContext::CreateChildContext(
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLDetectiveHighlightedContext::EndElement()
{
    switch( aDetectiveObj.eObjType )
    {
        case SC_DETOBJ_ARROW:
        case SC_DETOBJ_TOOTHERTAB:
        break;
        case SC_DETOBJ_FROMOTHERTAB:
            bValid = sal_True;
        break;
        default:
            bValid = sal_False;
    }
    if( bValid )
        rDetectiveObjVec.push_back( aDetectiveObj );
}


//___________________________________________________________________

ScXMLDetectiveOperationContext::ScXMLDetectiveOperationContext(
        ScXMLImport& rImport,
        USHORT nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    if( !xAttrList.is() ) return;

    sal_Int16               nAttrCount      = xAttrList->getLength();
    const SvXMLTokenMap&    rAttrTokenMap   = GetScImport().GetDetectiveOperationAttrTokenMap();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString sAttrName  = xAttrList->getNameByIndex( nIndex );
        OUString sValue     = xAttrList->getValueByIndex( nIndex );
        OUString aLocalName;
        USHORT nPrefix      = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
//          case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_NAME:
//              rRangeSource.sSourceStr = sValue;
//          break;
//          case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_FILTER_NAME:
//              rRangeSource.sFilterName = sValue;
//          break;
//          case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_FILTER_OPTIONS:
//              rRangeSource.sFilterOptions = sValue;
//          break;
//          case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_HREF:
//              rRangeSource.sURL = sValue;
//          break;
//          case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_LAST_COLUMN:
//          {
//              sal_Int32 nValue;
//              if( SvXMLUnitConverter::convertNumber( nValue, sValue, 1 ) )
//                  rRangeSource.nColumns = nValue;
//              else
//                  rRangeSource.nColumns = 1;
//          }
//          break;
//          case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_LAST_ROW:
//          {
//              sal_Int32 nValue;
//              if( SvXMLUnitConverter::convertNumber( nValue, sValue, 1 ) )
//                  rRangeSource.nRows = nValue;
//              else
//                  rRangeSource.nRows = 1;
//          }
//          break;
        }
    }
}

ScXMLDetectiveOperationContext::~ScXMLDetectiveOperationContext()
{
}

SvXMLImportContext *ScXMLDetectiveOperationContext::CreateChildContext(
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLDetectiveOperationContext::EndElement()
{
}

