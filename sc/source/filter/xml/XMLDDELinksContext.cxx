/*************************************************************************
 *
 *  $RCSfile: XMLDDELinksContext.cxx,v $
 *
 *  $Revision: 1.5 $
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

#ifndef _SC_XMLDDELINKSCONTEXT_HXX
#include "XMLDDELinksContext.hxx"
#endif
#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLDDELinksContext::ScXMLDDELinksContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    // here are no attributes
}

ScXMLDDELinksContext::~ScXMLDDELinksContext()
{
}

SvXMLImportContext *ScXMLDDELinksContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_TABLE && rLName.compareToAscii(sXML_dde_link) == 0)
        pContext = new ScXMLDDELinkContext(GetScImport(), nPrefix, rLName, xAttrList);

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDDELinksContext::EndElement()
{
}

ScXMLDDELinkContext::ScXMLDDELinkContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    aDDELinkTable(),
    aDDELinkRow(),
    sApplication(),
    sTopic(),
    sItem(),
    nMode(SC_DDE_DEFAULT),
    nPosition(-1),
    nColumns(0),
    nRows(0)
{
    // here are no attributes
}

ScXMLDDELinkContext::~ScXMLDDELinkContext()
{
}

SvXMLImportContext *ScXMLDDELinkContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_OFFICE && rLName.compareToAscii(sXML_dde_source) == 0)
        pContext = new ScXMLDDESourceContext(GetScImport(), nPrefix, rLName, xAttrList, this);
    else if (nPrefix == XML_NAMESPACE_TABLE && rLName.compareToAscii(sXML_table) == 0)
        pContext = new ScXMLDDETableContext(GetScImport(), nPrefix, rLName, xAttrList, this);

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDDELinkContext::CreateDDELink()
{
    if (sApplication.getLength() &&
        sTopic.getLength() &&
        sItem.getLength())
    {
        String sAppl(sApplication);
        String sTop(sTopic);
        String sIt(sItem);
        GetScImport().GetDocument()->CreateDdeLink(sAppl, sTop, sIt, nMode);
        sal_uInt16 nPos;
        if(GetScImport().GetDocument()->FindDdeLink(sAppl, sTop, sIt, nMode, nPos))
            nPosition = nPos;
        else
            nPosition = -1;
        DBG_ASSERT(nPosition > -1, "DDE Link not inserted");
    }
}

void ScXMLDDELinkContext::AddCellToRow(const ScDDELinkCell& aCell)
{
    aDDELinkRow.push_back(aCell);
}

void ScXMLDDELinkContext::AddRowsToTable(const sal_Int32 nRows)
{
    for (sal_Int32 i = 0; i < nRows; i++)
        aDDELinkTable.insert(aDDELinkTable.end(), aDDELinkRow.begin(), aDDELinkRow.end());
    aDDELinkRow.clear();
}

void ScXMLDDELinkContext::EndElement()
{
    if (nPosition > -1 && nColumns && nRows)
    {
        ScMatrix* pMatrix;
        if (GetScImport().GetDocument()->CreateDdeLinkResultDimension(static_cast<USHORT>(nPosition),
            static_cast<USHORT>(nColumns), static_cast<USHORT>(nRows), pMatrix))
        {
            if (pMatrix)
            {
                DBG_ASSERT(static_cast<sal_uInt32>(nColumns * nRows) == aDDELinkTable.size(), "there is a wrong cells count");
                sal_Int32 nCol(0);
                sal_Int32 nRow(-1);
                sal_Int32 nIndex(0);
                for (ScDDELinkCells::iterator aItr = aDDELinkTable.begin(); aItr != aDDELinkTable.end(); aItr++)
                {
                    if (nIndex % nColumns == 0)
                    {
                        nRow++;
                        nCol = 0;
                    }
                    else
                        nCol++;
                    String sValue(aItr->sValue);
                    GetScImport().GetDocument()->SetDdeLinkResult(pMatrix, static_cast<USHORT>(nCol), static_cast<USHORT>(nRow),
                        sValue, aItr->fValue, aItr->bString, aItr->bEmpty);
                    nIndex++;
                }
            }
        }
    }
}

ScXMLDDESourceContext::ScXMLDDESourceContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDDELinkContext* pTempDDELink) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDDELink(pTempDDELink)
{
    if( !xAttrList.is() ) return;

    sal_Int16               nAttrCount      = xAttrList->getLength();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString sAttrName  = xAttrList->getNameByIndex( nIndex );
        OUString sValue     = xAttrList->getValueByIndex( nIndex );
        OUString aLocalName;
        USHORT nPrefix      = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if (nPrefix == XML_NAMESPACE_OFFICE)
        {
            if (aLocalName.compareToAscii(sXML_dde_application) == 0)
                pDDELink->SetApplication(sValue);
            else if (aLocalName.compareToAscii(sXML_dde_topic) == 0)
                pDDELink->SetTopic(sValue);
            else if (aLocalName.compareToAscii(sXML_dde_item) == 0)
                pDDELink->SetItem(sValue);
        }
        else if (nPrefix == XML_NAMESPACE_TABLE && aLocalName.compareToAscii(sXML_conversion_mode) == 0)
            if (sValue.compareToAscii(sXML_into_english_number) == 0)
                pDDELink->SetMode(SC_DDE_ENGLISH);
            else if (sValue.compareToAscii(sXML_let_text) == 0)
                pDDELink->SetMode(SC_DDE_TEXT);
            else
                pDDELink->SetMode(SC_DDE_DEFAULT);
    }
}

ScXMLDDESourceContext::~ScXMLDDESourceContext()
{
}

SvXMLImportContext *ScXMLDDESourceContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDDESourceContext::EndElement()
{
    pDDELink->CreateDDELink();
}

ScXMLDDETableContext::ScXMLDDETableContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDDELinkContext* pTempDDELink) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDDELink(pTempDDELink)
{
    // here are no attributes
}

ScXMLDDETableContext::~ScXMLDDETableContext()
{
}

SvXMLImportContext *ScXMLDDETableContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = NULL;

    if (nPrefix == XML_NAMESPACE_TABLE)
        if (rLName.compareToAscii(sXML_table_column) == 0)
            pContext = new ScXMLDDEColumnContext(GetScImport(), nPrefix, rLName, xAttrList, pDDELink);
        else if (rLName.compareToAscii(sXML_table_row) == 0)
            pContext = new ScXMLDDERowContext(GetScImport(), nPrefix, rLName, xAttrList, pDDELink);

    if (!pContext)
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDDETableContext::EndElement()
{
}

ScXMLDDEColumnContext::ScXMLDDEColumnContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDDELinkContext* pTempDDELink) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDDELink(pTempDDELink)
{
    if( !xAttrList.is() ) return;
    sal_Int32 nCols(1);

    sal_Int16               nAttrCount      = xAttrList->getLength();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString sAttrName  = xAttrList->getNameByIndex( nIndex );
        OUString sValue     = xAttrList->getValueByIndex( nIndex );
        OUString aLocalName;
        USHORT nPrefix      = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if (nPrefix == XML_NAMESPACE_TABLE)
            if (aLocalName.compareToAscii(sXML_number_columns_repeated) == 0)
                GetScImport().GetMM100UnitConverter().convertNumber(nCols, sValue);
    }
    pDDELink->AddColumns(nCols);
}

ScXMLDDEColumnContext::~ScXMLDDEColumnContext()
{
}

SvXMLImportContext *ScXMLDDEColumnContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDDEColumnContext::EndElement()
{
}

ScXMLDDERowContext::ScXMLDDERowContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDDELinkContext* pTempDDELink) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDDELink(pTempDDELink),
    nRows(1)
{
    if( !xAttrList.is() ) return;

    sal_Int16               nAttrCount      = xAttrList->getLength();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString sAttrName  = xAttrList->getNameByIndex( nIndex );
        OUString sValue     = xAttrList->getValueByIndex( nIndex );
        OUString aLocalName;
        USHORT nPrefix      = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if (nPrefix == XML_NAMESPACE_TABLE)
            if (aLocalName.compareToAscii(sXML_number_rows_repeated) == 0)
                GetScImport().GetMM100UnitConverter().convertNumber(nRows, sValue);
    }
    pDDELink->AddRows(nRows);
}

ScXMLDDERowContext::~ScXMLDDERowContext()
{
}

SvXMLImportContext *ScXMLDDERowContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = NULL;

    if (nPrefix == XML_NAMESPACE_TABLE)
        if (rLName.compareToAscii(sXML_table_cell) == 0)
            pContext = new ScXMLDDECellContext(GetScImport(), nPrefix, rLName, xAttrList, pDDELink);

    if (!pContext)
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDDERowContext::EndElement()
{
    pDDELink->AddRowsToTable(nRows);
}

ScXMLDDECellContext::ScXMLDDECellContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDDELinkContext* pTempDDELink) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDDELink(pTempDDELink),
    sValue(),
    fValue(),
    nCells(1),
    bString(sal_True),
    bString2(sal_True),
    bEmpty(sal_True)
{
    if( !xAttrList.is() ) return;

    sal_Int16               nAttrCount      = xAttrList->getLength();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString sAttrName  = xAttrList->getNameByIndex( nIndex );
        OUString sTempValue     = xAttrList->getValueByIndex( nIndex );
        OUString aLocalName;
        USHORT nPrefix      = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if (nPrefix == XML_NAMESPACE_TABLE)
            if (aLocalName.compareToAscii(sXML_value_type) == 0)
            {
                if (sTempValue.compareToAscii(sXML_string) == 0)
                    bString = sal_True;
                else
                    bString = sal_False;
            }
            else if (aLocalName.compareToAscii(sXML_string_value) == 0)
            {
                sValue = sTempValue;
                bEmpty = sal_False;
                bString2 = sal_True;
            }
            else if (aLocalName.compareToAscii(sXML_value) == 0)
            {
                GetScImport().GetMM100UnitConverter().convertNumber(fValue, sTempValue);
                bEmpty = sal_False;
                bString2 = sal_False;
            }
            else if (aLocalName.compareToAscii(sXML_number_columns_repeated) == 0)
                GetScImport().GetMM100UnitConverter().convertNumber(nCells, sTempValue);
    }
}

ScXMLDDECellContext::~ScXMLDDECellContext()
{
}

SvXMLImportContext *ScXMLDDECellContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDDECellContext::EndElement()
{
    DBG_ASSERT(bString == bString2, "something wrong with this type");
    ScDDELinkCell aCell;
    aCell.sValue = sValue;
    aCell.fValue = fValue;
    aCell.bEmpty = bEmpty;
    aCell.bString = bString2;
    for(sal_Int32 i = 0; i < nCells; i++)
        pDDELink->AddCellToRow(aCell);
}
