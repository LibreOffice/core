/*************************************************************************
 *
 *  $RCSfile: xmlsceni.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-15 09:29:26 $
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

// INCLUDE ---------------------------------------------------------------

#include "document.hxx"
#include "xmlimprt.hxx"
#include "xmlsceni.hxx"
#include "docuno.hxx"
#include "attrib.hxx"

#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlkywd.hxx>
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLTableScenarioContext::ScXMLTableScenarioContext(
        ScXMLImport& rImport,
        USHORT nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList ):
    SvXMLImportContext( rImport, nPrfx, rLName ),
    aBorderColor( COL_BLACK ),
    bDisplayBorder( sal_True ),
    bCopyBack( sal_True ),
    bCopyStyles( sal_True ),
    bCopyFormulas( sal_True ),
    bIsActive( sal_False )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableScenarioAttrTokenMap();
    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_TABLE_SCENARIO_ATTR_DISPLAY_BORDER:
            {
                bDisplayBorder = (sValue.compareToAscii(sXML_true) == 0);
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_BORDER_COLOR:
            {
                SvXMLUnitConverter::convertColor(aBorderColor, sValue);
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_COPY_BACK:
            {
                bCopyBack = (sValue.compareToAscii(sXML_true) == 0);
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_COPY_STYLES:
            {
                bCopyStyles = (sValue.compareToAscii(sXML_true) == 0);
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_COPY_FORMULAS:
            {
                bCopyFormulas = (sValue.compareToAscii(sXML_true) == 0);
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_IS_ACTIVE:
            {
                bIsActive = (sValue.compareToAscii(sXML_true) == 0);
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_SCENARIO_RANGES:
            {
                ScXMLConverter::GetRangeListFromString(
                    aScenarioRanges, sValue, GetScImport().GetDocument() );
            }
            break;
            case XML_TOK_TABLE_SCENARIO_ATTR_COMMENT:
            {
                sComment = sValue;
            }
            break;
        }
    }
}

ScXMLTableScenarioContext::~ScXMLTableScenarioContext()
{
}

SvXMLImportContext *ScXMLTableScenarioContext::CreateChildContext(
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLTableScenarioContext::EndElement()
{
    sal_Int16   nCurrTable( GetScImport().GetTables().GetCurrentSheet() );
    ScDocument* pDoc = GetScImport().GetDocument();

    pDoc->SetScenario( nCurrTable, TRUE );
    USHORT nFlags( 0 );
    if( bDisplayBorder )
        nFlags |= SC_SCENARIO_SHOWFRAME;
    if( bCopyBack )
        nFlags |= SC_SCENARIO_TWOWAY;
    if( bCopyStyles )
        nFlags |= SC_SCENARIO_ATTRIB;
    if( !bCopyFormulas )
        nFlags |= SC_SCENARIO_VALUE;
    pDoc->SetScenarioData( nCurrTable, String( sComment ), aBorderColor, nFlags );
    for( sal_Int32 i = 0; i < static_cast<sal_Int32>(aScenarioRanges.Count()); i++ )
    {
        ScRange* pRange = aScenarioRanges.GetObject( i );
        if( pRange )
            pDoc->ApplyFlagsTab( pRange->aStart.Col(), pRange->aStart.Row(),
                pRange->aEnd.Col(), pRange->aEnd.Row(), nCurrTable, SC_MF_SCENARIO );
    }
    pDoc->SetActiveScenario( nCurrTable, bIsActive );
}

