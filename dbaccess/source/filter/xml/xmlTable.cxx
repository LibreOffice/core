/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "xmlTable.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlStyleImport.hxx"
#include "xmlHierarchyCollection.hxx"
#include "xmlstrings.hrc"
#include <ucbhelper/content.hxx>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <comphelper/namecontainer.hxx>
#include <tools/debug.hxx>

namespace dbaxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLTable)

OXMLTable::OXMLTable( ODBFilter& _rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const uno::Reference< XAttributeList > & _xAttrList
                ,const uno::Reference< ::com::sun::star::container::XNameAccess >& _xParentContainer
                ,const OUString& _sServiceName
                )
    :SvXMLImportContext( _rImport, nPrfx, _sLocalName )
    ,m_xParentContainer(_xParentContainer)
    ,m_sServiceName(_sServiceName)
    ,m_bApplyFilter(sal_False)
    ,m_bApplyOrder(sal_False)
{
    DBG_CTOR(OXMLTable,NULL);

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = GetOwnImport().GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = GetOwnImport().GetQueryElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_QUERY_NAME:
                m_sName = sValue;
                break;
            case XML_TOK_CATALOG_NAME:
                m_sCatalog = sValue;
                break;
            case XML_TOK_SCHEMA_NAME:
                m_sSchema = sValue;
                break;
            case XML_TOK_STYLE_NAME:
                m_sStyleName = sValue;
                break;
            case XML_TOK_APPLY_FILTER:
                m_bApplyFilter = sValue == "true";
                break;
            case XML_TOK_APPLY_ORDER:
                m_bApplyOrder = sValue == "true";
                break;
        }
    }
    Sequence< Any > aArguments(2);
    PropertyValue aValue;
    // set as folder
    aValue.Name = OUString("Name");
    aValue.Value <<= m_sName;
    aArguments[0] <<= aValue;
    //parent
    aValue.Name = OUString("Parent");
    aValue.Value <<= m_xParentContainer;
    aArguments[1] <<= aValue;
    m_xTable.set(
        GetOwnImport().GetComponentContext()->getServiceManager()->createInstanceWithArgumentsAndContext(m_sServiceName,aArguments, GetOwnImport().GetComponentContext()),
        UNO_QUERY);
}

OXMLTable::~OXMLTable()
{

    DBG_DTOR(OXMLTable,NULL);
}

SvXMLImportContext* OXMLTable::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    const SvXMLTokenMap&    rTokenMap   = GetOwnImport().GetQueryElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_FILTER_STATEMENT:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                OUString s1,s2,s3;
                fillAttributes(nPrefix, rLocalName,xAttrList,m_sFilterStatement,s1,s2,s3);
            }
            break;
        case XML_TOK_ORDER_STATEMENT:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                OUString s1,s2,s3;
                fillAttributes(nPrefix, rLocalName,xAttrList,m_sOrderStatement,s1,s2,s3);
            }
            break;

        case XML_TOK_COLUMNS:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                uno::Reference< XColumnsSupplier > xColumnsSup(m_xTable,UNO_QUERY);
                uno::Reference< XNameAccess > xColumns;
                if ( xColumnsSup.is() )
                {
                    xColumns = xColumnsSup->getColumns();
                }
                pContext = new OXMLHierarchyCollection( GetOwnImport(), nPrefix, rLocalName ,xColumns,m_xTable);
            }
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

ODBFilter& OXMLTable::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}

void OXMLTable::setProperties(uno::Reference< XPropertySet > & _xProp )
{
    try
    {
        if ( _xProp.is() )
        {
            _xProp->setPropertyValue(PROPERTY_APPLYFILTER,makeAny(m_bApplyFilter));
            _xProp->setPropertyValue(PROPERTY_FILTER,makeAny(m_sFilterStatement));

            if ( _xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_APPLYORDER) )
                _xProp->setPropertyValue(PROPERTY_APPLYORDER,makeAny(m_bApplyOrder));
            _xProp->setPropertyValue(PROPERTY_ORDER,makeAny(m_sOrderStatement));
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("OXMLTable::EndElement -> exception catched");
    }
}

void OXMLTable::EndElement()
{
    uno::Reference<XNameContainer> xNameContainer(m_xParentContainer,UNO_QUERY);
    if ( xNameContainer.is() )
    {
        try
        {
            if ( m_xTable.is() )
            {
                setProperties(m_xTable);

                if ( !m_sStyleName.isEmpty() )
                {
                    const SvXMLStylesContext* pAutoStyles = GetOwnImport().GetAutoStyles();
                    if ( pAutoStyles )
                    {
                        OTableStyleContext* pAutoStyle = PTR_CAST(OTableStyleContext,pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_TABLE,m_sStyleName));
                        if ( pAutoStyle )
                        {
                            pAutoStyle->FillPropertySet(m_xTable);
                        }
                    }
                }

                xNameContainer->insertByName(m_sName,makeAny(m_xTable));
            }
        }
        catch(Exception&)
        {
            OSL_FAIL("OXMLQuery::EndElement -> exception catched");
        }
    }

}

void OXMLTable::fillAttributes(sal_uInt16 /*nPrfx*/
                                ,const OUString& /*_sLocalName*/
                                ,const uno::Reference< XAttributeList > & _xAttrList
                                ,OUString& _rsCommand
                                ,OUString& _rsTableName
                                ,OUString& _rsTableSchema
                                ,OUString& _rsTableCatalog
                                )
{
    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = GetOwnImport().GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = GetOwnImport().GetQueryElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_COMMAND:
                _rsCommand = sValue;
                break;
            case XML_TOK_CATALOG_NAME:
                _rsTableCatalog = sValue;
                break;
            case XML_TOK_SCHEMA_NAME:
                _rsTableSchema = sValue;
                break;
            case XML_TOK_QUERY_NAME:
                _rsTableName = sValue;
                break;
        }
    }
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
