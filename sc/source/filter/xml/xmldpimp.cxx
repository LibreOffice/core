/*************************************************************************
 *
 *  $RCSfile: xmldpimp.cxx,v $
 *
 *  $Revision: 1.4 $
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

// INCLUDE ---------------------------------------------------------------

#include "xmldpimp.hxx"
#include "xmlimprt.hxx"
#include "xmlfilti.hxx"
#include "xmlsorti.hxx"
#include "document.hxx"
#include "docuno.hxx"
#include "dpshttab.hxx"
#include "dpsdbtab.hxx"
#include "attrib.hxx"

#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlkywd.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLDataPilotTablesContext::ScXMLDataPilotTablesContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    // has no Attributes
}

ScXMLDataPilotTablesContext::~ScXMLDataPilotTablesContext()
{
}

SvXMLImportContext *ScXMLDataPilotTablesContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDataPilotTablesElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATA_PILOT_TABLE :
        {
            pContext = new ScXMLDataPilotTableContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotTablesContext::EndElement()
{
}

ScXMLDataPilotTableContext::ScXMLDataPilotTableContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sDataPilotTableName(),
    sApplicationData(),
    sGrandTotal(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_both))),
    bIsNative(sal_True),
    bIgnoreEmptyRows(sal_False),
    bIdentifyCategories(sal_False),
    pDoc(NULL),
    pDPObject(NULL),
    pDPSave(NULL)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataPilotTableAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DATA_PILOT_TABLE_ATTR_NAME :
            {
                sDataPilotTableName = sValue;
            }
            break;
            case XML_TOK_DATA_PILOT_TABLE_ATTR_APPLICATION_DATA :
            {
                sApplicationData = sValue;
            }
            break;
            case XML_TOK_DATA_PILOT_TABLE_ATTR_GRAND_TOTAL :
            {
                sGrandTotal = sValue;
            }
            break;
            case XML_TOK_DATA_PILOT_TABLE_ATTR_IGNORE_EMPTY_ROWS :
            {
                if (sValue.compareToAscii(sXML_true) == 0)
                    bIgnoreEmptyRows = sal_True;
            }
            case XML_TOK_DATA_PILOT_TABLE_ATTR_IDENTIFY_CATEGORIES :
            {
                if (sValue.compareToAscii(sXML_true) == 0)
                    bIdentifyCategories = sal_True;
            }
            case XML_TOK_DATA_PILOT_TABLE_ATTR_TARGET_RANGE_ADDRESS :
            {
                ScXMLImport& rXMLImport = GetScImport();
                ScModelObj* pDocObj = ScModelObj::getImplementation( rXMLImport.GetModel() );
                if ( pDocObj )
                {
                    ScDocument* pDoc = pDocObj->GetDocument();
                    ScAddress aStartCellAddress;
                    ScAddress aEndCellAddress;
                    sal_Int16 i = 0;
                    while ((sValue[i] != ':') && (i < sValue.getLength()))
                        i++;
                    rtl::OUString sStartCellAddress = sValue.copy(0, i);
                    rtl::OUString sEndCellAddress = sValue.copy(i + 1);
                    aStartCellAddress.Parse(sStartCellAddress, pDoc);
                    aEndCellAddress.Parse(sEndCellAddress, pDoc);
                    aTargetRangeAddress = ScRange(aStartCellAddress, aEndCellAddress);
                }
            }
            case XML_TOK_DATA_PILOT_TABLE_ATTR_BUTTONS :
            {
                sButtons = sValue;
            }
        }
    }
    ScModelObj* pDocObj = ScModelObj::getImplementation( GetScImport().GetModel() );
    if ( pDocObj )
    {
        pDoc = pDocObj->GetDocument();
        if (pDoc)
        {
            pDPObject = new ScDPObject(pDoc);
             pDPSave = new ScDPSaveData();
        }
    }
}

ScXMLDataPilotTableContext::~ScXMLDataPilotTableContext()
{
}

SvXMLImportContext *ScXMLDataPilotTableContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDataPilotTableElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_SQL :
        {
            pContext = new ScXMLDPSourceSQLContext(GetScImport(), nPrefix, rLName, xAttrList, this);
            nSourceType = SQL;
        }
        break;
        case XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_TABLE :
        {
            pContext = new ScXMLDPSourceTableContext(GetScImport(), nPrefix, rLName, xAttrList, this);
            nSourceType = TABLE;
        }
        break;
        case XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_QUERY :
        {
            pContext = new ScXMLDPSourceQueryContext(GetScImport(), nPrefix, rLName, xAttrList, this);
            nSourceType = QUERY;
        }
        break;
        case XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_SERVICE :
        {
            pContext = new ScXMLSourceServiceContext(GetScImport(), nPrefix, rLName, xAttrList, this);
            nSourceType = SERVICE;
        }
        break;
        case XML_TOK_DATA_PILOT_TABLE_ELEM_SOURCE_CELL_RANGE :
        {
            pContext = new ScXMLSourceCellRangeContext(GetScImport(), nPrefix, rLName, xAttrList, this);
            nSourceType = CELLRANGE;
        }
        break;
        case XML_TOK_DATA_PILOT_TABLE_ELEM_DATA_PILOT_FIELD :
            pContext = new ScXMLDataPilotFieldContext(GetScImport(), nPrefix, rLName, xAttrList, this);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotTableContext::SetButtons()
{
    sal_Int32 nCount = sButtons.getLength();
    if (nCount)
    {
        sal_Bool bIn(sal_False);
        sal_Int32 nPos = 0;
        sal_Int32 nOldPos = 0;
        while(nPos < nCount)
        {
            if (sButtons[nPos] == '\'')
                bIn = !bIn;
            if ((!bIn && sButtons[nPos] == ' ') || (nPos == nCount - 1 && nPos > nOldPos))
            {
                if (nPos == nCount - 1)
                    nPos++;
                rtl::OUString sCellAddress = sButtons.copy(nOldPos, nPos - nOldPos);
                ScAddress aCellAddress;
                aCellAddress.Parse(sCellAddress, pDoc);
                ScMergeFlagAttr aAttr(SC_MF_BUTTON);
                pDoc->ApplyAttr(aCellAddress.Col(), aCellAddress.Row(), aCellAddress.Tab(), aAttr);
                nOldPos = nPos + 1;
            }
            nPos++;
        }
    }
}

void ScXMLDataPilotTableContext::EndElement()
{
    pDPObject->SetName(sDataPilotTableName);
    pDPObject->SetTag(sApplicationData);
    pDPObject->SetOutRange(aTargetRangeAddress);
    switch (nSourceType)
    {
        case SQL :
        {
            ScImportSourceDesc aImportDesc;
            aImportDesc.aDBName = sDatabaseName;
            aImportDesc.aObject = sSourceObject;
            aImportDesc.nType = sheet::DataImportMode_SQL;
            aImportDesc.bNative = bIsNative;
            pDPObject->SetImportDesc(aImportDesc);
        }
        break;
        case TABLE :
        {
            ScImportSourceDesc aImportDesc;
            aImportDesc.aDBName = sDatabaseName;
            aImportDesc.aObject = sSourceObject;
            aImportDesc.nType = sheet::DataImportMode_TABLE;
            pDPObject->SetImportDesc(aImportDesc);
        }
        break;
        case QUERY :
        {
            ScImportSourceDesc aImportDesc;
            aImportDesc.aDBName = sDatabaseName;
            aImportDesc.aObject = sSourceObject;
            aImportDesc.nType = sheet::DataImportMode_QUERY;
            pDPObject->SetImportDesc(aImportDesc);
        }
        break;
        case SERVICE :
        {
            ScDPServiceDesc aServiceDesk(sServiceName, sServiceSourceObject, sServiceSourceName,
                                sServiceUsername, sServicePassword);
            pDPObject->SetServiceData(aServiceDesk);
        }
        break;
        case CELLRANGE :
        {
            ScSheetSourceDesc aSheetDesc;
            aSheetDesc.aSourceRange = aSourceCellRangeAddress;
            aSheetDesc.aQueryParam = aSourceQueryParam;
            pDPObject->SetSheetDesc(aSheetDesc);
        }
        break;
    }
    if (sGrandTotal.compareToAscii(sXML_both) == 0)
    {
        pDPSave->SetRowGrand(sal_True);
        pDPSave->SetColumnGrand(sal_True);
    }
    else if (sGrandTotal.compareToAscii(sXML_row) == 0)
    {
        pDPSave->SetRowGrand(sal_True);
        pDPSave->SetColumnGrand(sal_False);
    }
    else if (sGrandTotal.compareToAscii(sXML_column) == 0)
    {
        pDPSave->SetRowGrand(sal_False);
        pDPSave->SetColumnGrand(sal_True);
    }
    else
    {
        pDPSave->SetRowGrand(sal_False);
        pDPSave->SetColumnGrand(sal_False);
    }
    pDPSave->SetIgnoreEmptyRows(bIgnoreEmptyRows);
    pDPSave->SetRepeatIfEmpty(bIdentifyCategories);
    pDPObject->SetSaveData(*pDPSave);
    ScDPCollection* pDPCollection = pDoc->GetDPCollection();
    pDPObject->SetAlive(sal_True);
    pDPCollection->Insert(pDPObject);
    SetButtons();
}

ScXMLDPSourceSQLContext::ScXMLDPSourceSQLContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotTableContext* pTempDataPilotTable) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDataPilotTable = pTempDataPilotTable;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeSourceSQLAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_SQL_ATTR_DATABASE_NAME :
            {
                pDataPilotTable->SetDatabaseName(sValue);
            }
            break;
            case XML_TOK_SOURCE_SQL_ATTR_SQL_STATEMENT :
            {
                pDataPilotTable->SetSourceObject(sValue);
            }
            break;
            case XML_TOK_SOURCE_SQL_ATTR_PARSE_SQL_STATEMENT :
            {
                if (sValue == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true)))
                    pDataPilotTable->SetNative(sal_False);
                else
                    pDataPilotTable->SetNative(sal_True);
            }
            break;
        }
    }
}

ScXMLDPSourceSQLContext::~ScXMLDPSourceSQLContext()
{
}

SvXMLImportContext *ScXMLDPSourceSQLContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDPSourceSQLContext::EndElement()
{
}

ScXMLDPSourceTableContext::ScXMLDPSourceTableContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotTableContext* pTempDataPilotTable) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDataPilotTable = pTempDataPilotTable;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeSourceTableAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_TABLE_ATTR_DATABASE_NAME :
            {
                pDataPilotTable->SetDatabaseName(sValue);
            }
            break;
            case XML_TOK_SOURCE_TABLE_ATTR_TABLE_NAME :
            {
                pDataPilotTable->SetSourceObject(sValue);
            }
            break;
        }
    }
}

ScXMLDPSourceTableContext::~ScXMLDPSourceTableContext()
{
}

SvXMLImportContext *ScXMLDPSourceTableContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDPSourceTableContext::EndElement()
{
}

ScXMLDPSourceQueryContext::ScXMLDPSourceQueryContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotTableContext* pTempDataPilotTable) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDataPilotTable = pTempDataPilotTable;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeSourceQueryAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_QUERY_ATTR_DATABASE_NAME :
            {
                pDataPilotTable->SetDatabaseName(sValue);
            }
            break;
            case XML_TOK_SOURCE_QUERY_ATTR_QUERY_NAME :
            {
                pDataPilotTable->SetSourceObject(sValue);
            }
            break;
        }
    }
}

ScXMLDPSourceQueryContext::~ScXMLDPSourceQueryContext()
{
}

SvXMLImportContext *ScXMLDPSourceQueryContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDPSourceQueryContext::EndElement()
{
}

ScXMLSourceServiceContext::ScXMLSourceServiceContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotTableContext* pTempDataPilotTable) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDataPilotTable = pTempDataPilotTable;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataPilotTableSourceServiceAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_SERVICE_ATTR_NAME :
            {
                pDataPilotTable->SetServiceName(sValue);
            }
            break;
            case XML_TOK_SOURCE_SERVICE_ATTR_SOURCE_NAME :
            {
                pDataPilotTable->SetServiceSourceName(sValue);
            }
            break;
            case XML_TOK_SOURCE_SERVICE_ATTR_OBJECT_NAME :
            {
                pDataPilotTable->SetServiceSourceObject(sValue);
            }
            break;
            case XML_TOK_SOURCE_SERVICE_ATTR_USERNAME :
            {
                pDataPilotTable->SetServiceUsername(sValue);
            }
            break;
            case XML_TOK_SOURCE_SERVICE_ATTR_PASSWORD :
            {
                pDataPilotTable->SetServicePassword(sValue);
            }
            break;
        }
    }
}

ScXMLSourceServiceContext::~ScXMLSourceServiceContext()
{
}

SvXMLImportContext *ScXMLSourceServiceContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSourceServiceContext::EndElement()
{
}

ScXMLSourceCellRangeContext::ScXMLSourceCellRangeContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotTableContext* pTempDataPilotTable) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDataPilotTable = pTempDataPilotTable;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataPilotTableSourceCellRangeAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_CELL_RANGE_ATTR_CELL_RANGE_ADDRESS :
            {
                ScXMLImport& rXMLImport = GetScImport();
                ScModelObj* pDocObj = ScModelObj::getImplementation( rXMLImport.GetModel() );
                if ( pDocObj )
                {
                    ScDocument* pDoc = pDocObj->GetDocument();
                    ScAddress aStartCellAddress;
                    ScAddress aEndCellAddress;
                    sal_Int16 i = 0;
                    while ((sValue[i] != ':') && (i < sValue.getLength()))
                        i++;
                    rtl::OUString sStartCellAddress = sValue.copy(0, i);
                    rtl::OUString sEndCellAddress = sValue.copy(i + 1);
                    aStartCellAddress.Parse(sStartCellAddress, pDoc);
                    aEndCellAddress.Parse(sEndCellAddress, pDoc);
                    ScRange aSourceRangeAddress(aStartCellAddress, aEndCellAddress);
                    pDataPilotTable->SetSourceCellRangeAddress(aSourceRangeAddress);
                }
            }
            break;
        }
    }
}

ScXMLSourceCellRangeContext::~ScXMLSourceCellRangeContext()
{
}

SvXMLImportContext *ScXMLSourceCellRangeContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDataPilotTableSourceCellRangeElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_SOURCE_CELL_RANGE_ELEM_FILTER :
            pContext = new ScXMLDPFilterContext(GetScImport(), nPrefix, rLName, xAttrList, pDataPilotTable);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSourceCellRangeContext::EndElement()
{
}

ScXMLDataPilotFieldContext::ScXMLDataPilotFieldContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotTableContext* pTempDataPilotTable) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDim(NULL)
{
    pDataPilotTable = pTempDataPilotTable;
    rtl::OUString sName;
    sal_Bool bHasName(sal_False);
    sal_Bool bDataLayout(sal_False);
    nUsedHierarchy = 1;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataPilotFieldAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DATA_PILOT_FIELD_ATTR_SOURCE_FIELD_NAME :
            {
                sName = sValue;
                bHasName = sal_True;
            }
            break;
            case XML_TOK_DATA_PILOT_FIELD_ATTR_IS_DATA_LAYOUT_FIELD :
            {
                if (sValue.compareToAscii(sXML_true) == 0)
                    bDataLayout = sal_True;
            }
            break;
            case XML_TOK_DATA_PILOT_FIELD_ATTR_FUNCTION :
            {
                nFunction = (sal_Int16) ScXMLConverter::GetFunctionFromString( sValue );
            }
            break;
            case XML_TOK_DATA_PILOT_FIELD_ATTR_ORIENTATION :
            {
                nOrientation = GetOrientation(sValue);
            }
            break;
            case XML_TOK_DATA_PILOT_FIELD_ATTR_USED_HIERARCHY :
            {
                nUsedHierarchy = sValue.toInt32();
            }
            break;
        }
    }
    if (bHasName)
        pDim = new ScDPSaveDimension(String(sName), bDataLayout);
}

ScXMLDataPilotFieldContext::~ScXMLDataPilotFieldContext()
{
}

SvXMLImportContext *ScXMLDataPilotFieldContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDataPilotFieldElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATA_PILOT_FIELD_ELEM_DATA_PILOT_LEVEL :
            pContext = new ScXMLDataPilotLevelContext(GetScImport(), nPrefix, rLName, xAttrList, this);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

sal_Int16 ScXMLDataPilotFieldContext::GetOrientation(const rtl::OUString& sOrientation)
{
    if (sOrientation.compareToAscii(sXML_row) == 0)
        return sheet::DataPilotFieldOrientation_ROW;
    else if (sOrientation.compareToAscii(sXML_column) == 0)
        return sheet::DataPilotFieldOrientation_COLUMN;
    else if (sOrientation.compareToAscii(sXML_data) == 0)
        return sheet::DataPilotFieldOrientation_DATA;
    else if (sOrientation.compareToAscii(sXML_page) == 0)
        return sheet::DataPilotFieldOrientation_PAGE;
    else if (sOrientation.compareToAscii(sXML_hidden) == 0)
        return sheet::DataPilotFieldOrientation_HIDDEN;
    else
        return 0;
}

void ScXMLDataPilotFieldContext::EndElement()
{
    if (pDim)
    {
        pDim->SetUsedHierarchy(nUsedHierarchy);
        pDim->SetFunction(nFunction);
        pDim->SetOrientation(nOrientation);
        pDataPilotTable->AddDimension(pDim);
    }
}

ScXMLDataPilotLevelContext::ScXMLDataPilotLevelContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotFieldContext* pTempDataPilotField) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDataPilotField = pTempDataPilotField;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataPilotLevelAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DATA_PILOT_LEVEL_ATTR_DISPLAY_EMPTY :
            {
                if (sValue.compareToAscii(sXML_true) == 0)
                    pDataPilotField->SetShowEmpty(sal_True);
                else if (sValue.compareToAscii(sXML_false) == 0)
                    pDataPilotField->SetShowEmpty(sal_False);
            }
            break;
        }
    }
}

ScXMLDataPilotLevelContext::~ScXMLDataPilotLevelContext()
{
}

SvXMLImportContext *ScXMLDataPilotLevelContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDataPilotLevelElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATA_PILOT_LEVEL_ELEM_DATA_PILOT_SUBTOTALS :
            pContext = new ScXMLDataPilotSubTotalsContext(GetScImport(), nPrefix, rLName, xAttrList, pDataPilotField);
        break;
        case XML_TOK_DATA_PILOT_LEVEL_ELEM_DATA_PILOT_MEMBERS :
            pContext = new ScXMLDataPilotMembersContext(GetScImport(), nPrefix, rLName, xAttrList, pDataPilotField);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotLevelContext::EndElement()
{
}

ScXMLDataPilotSubTotalsContext::ScXMLDataPilotSubTotalsContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotFieldContext* pTempDataPilotField) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pFunctions(NULL),
    nFunctionCount(0)
{
    pDataPilotField = pTempDataPilotField;

    // has no attributes
}

ScXMLDataPilotSubTotalsContext::~ScXMLDataPilotSubTotalsContext()
{
}

SvXMLImportContext *ScXMLDataPilotSubTotalsContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDataPilotSubTotalsElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATA_PILOT_SUBTOTALS_ELEM_DATA_PILOT_SUBTOTAL :
            pContext = new ScXMLDataPilotSubTotalContext(GetScImport(), nPrefix, rLName, xAttrList, this);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotSubTotalsContext::EndElement()
{
    pDataPilotField->SetSubTotals(pFunctions, nFunctionCount);
}

void ScXMLDataPilotSubTotalsContext::AddFunction(sal_Int16 nFunction)
{
    if (nFunctionCount)
    {
        nFunctionCount++;
        sal_uInt16* pTemp = new sal_uInt16[nFunctionCount];
        for (sal_Int16 i = 0; i < nFunctionCount - 1; i++)
            pTemp[i] = pFunctions[i];
        pTemp[nFunctionCount - 1] = nFunction;
        delete[] pFunctions;
        pFunctions = pTemp;
    }
    else
    {
        nFunctionCount = 1;
        pFunctions = new sal_uInt16[nFunctionCount];
        pFunctions[0] = nFunction;
    }
}

ScXMLDataPilotSubTotalContext::ScXMLDataPilotSubTotalContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotSubTotalsContext* pTempDataPilotSubTotals) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDataPilotSubTotals = pTempDataPilotSubTotals;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataPilotSubTotalAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DATA_PILOT_SUBTOTAL_ATTR_FUNCTION :
            {
                pDataPilotSubTotals->AddFunction( ScXMLConverter::GetFunctionFromString( sValue ) );
            }
            break;
        }
    }
}

ScXMLDataPilotSubTotalContext::~ScXMLDataPilotSubTotalContext()
{
}

SvXMLImportContext *ScXMLDataPilotSubTotalContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotSubTotalContext::EndElement()
{
}

ScXMLDataPilotMembersContext::ScXMLDataPilotMembersContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotFieldContext* pTempDataPilotField) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDataPilotField = pTempDataPilotField;

    // has no attributes
}

ScXMLDataPilotMembersContext::~ScXMLDataPilotMembersContext()
{
}

SvXMLImportContext *ScXMLDataPilotMembersContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDataPilotMembersElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATA_PILOT_MEMBERS_ELEM_DATA_PILOT_MEMBER :
            pContext = new ScXMLDataPilotMemberContext(GetScImport(), nPrefix, rLName, xAttrList, pDataPilotField);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotMembersContext::EndElement()
{
}

ScXMLDataPilotMemberContext::ScXMLDataPilotMemberContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotFieldContext* pTempDataPilotField) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDataPilotField = pTempDataPilotField;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDataPilotMemberAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DATA_PILOT_MEMBER_ATTR_NAME :
            {
                sName = sValue;
            }
            break;
            case XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY :
            {
                if (sValue.compareToAscii(sXML_true) == 0)
                    bDisplay = sal_True;
                else if (sValue.compareToAscii(sXML_false) == 0)
                    bDisplay = sal_False;
            }
            break;
            case XML_TOK_DATA_PILOT_MEMBER_ATTR_DISPLAY_DETAILS :
            {
                if (sValue.compareToAscii(sXML_true) == 0)
                    bDisplayDetails = sal_True;
                else if (sValue.compareToAscii(sXML_false) == 0)
                    bDisplayDetails = sal_False;
            }
            break;
        }
    }
}

ScXMLDataPilotMemberContext::~ScXMLDataPilotMemberContext()
{
}

SvXMLImportContext *ScXMLDataPilotMemberContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDataPilotMemberContext::EndElement()
{
    if (sName.getLength())
    {
        ScDPSaveMember* pMember = new ScDPSaveMember(String(sName));
        pMember->SetIsVisible(bDisplay);
        pMember->SetShowDetails(bDisplayDetails);
        pDataPilotField->AddMember(pMember);
    }
}


