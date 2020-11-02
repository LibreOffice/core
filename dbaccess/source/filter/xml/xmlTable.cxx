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
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include "xmlStyleImport.hxx"
#include "xmlHierarchyCollection.hxx"
#include <strings.hxx>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/propertysequence.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

namespace dbaxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::xml::sax;

OXMLTable::OXMLTable( ODBFilter& _rImport
                ,const uno::Reference< XFastAttributeList > & _xAttrList
                ,const uno::Reference< css::container::XNameAccess >& _xParentContainer
                ,const OUString& _sServiceName
                )
    :SvXMLImportContext( _rImport )
    ,m_xParentContainer(_xParentContainer)
    ,m_bApplyFilter(false)
    ,m_bApplyOrder(false)
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
    {
        OUString sValue = aIter.toString();

        switch( aIter.getToken() & TOKEN_MASK )
        {
            case XML_NAME:
                m_sName = sValue;
                break;
            case XML_CATALOG_NAME:
                m_sCatalog = sValue;
                break;
            case XML_SCHEMA_NAME:
                m_sSchema = sValue;
                break;
            case XML_STYLE_NAME:
                m_sStyleName = sValue;
                break;
            case XML_APPLY_FILTER:
                m_bApplyFilter = sValue == "true";
                break;
            case XML_APPLY_ORDER:
                m_bApplyOrder = sValue == "true";
                break;
            default:
                XMLOFF_WARN_UNKNOWN("dbaccess", aIter);
        }
    }
    uno::Sequence<uno::Any> aArguments(comphelper::InitAnyPropertySequence(
    {
        {"Name", uno::Any(m_sName)}, // set as folder
        {"Parent", uno::Any(m_xParentContainer)}
    }));
    m_xTable.set(
        GetOwnImport().GetComponentContext()->getServiceManager()->createInstanceWithArgumentsAndContext(_sServiceName,aArguments, GetOwnImport().GetComponentContext()),
        UNO_QUERY);
}

OXMLTable::~OXMLTable()
{

}

css::uno::Reference< css::xml::sax::XFastContextHandler > OXMLTable::createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    switch( nElement & TOKEN_MASK )
    {
        case XML_FILTER_STATEMENT:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                OUString s1,s2,s3;
                fillAttributes(xAttrList,m_sFilterStatement,s1,s2,s3);
            }
            break;
        case XML_ORDER_STATEMENT:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                OUString s1,s2,s3;
                fillAttributes(xAttrList,m_sOrderStatement,s1,s2,s3);
            }
            break;

        case XML_COLUMNS:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                uno::Reference< XColumnsSupplier > xColumnsSup(m_xTable,UNO_QUERY);
                uno::Reference< XNameAccess > xColumns;
                if ( xColumnsSup.is() )
                {
                    xColumns = xColumnsSup->getColumns();
                }
                pContext = new OXMLHierarchyCollection( GetOwnImport(), xColumns,m_xTable);
            }
            break;
    }

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
        OSL_FAIL("OXMLTable::EndElement -> exception caught");
    }
}

void OXMLTable::endFastElement(sal_Int32 )
{
    uno::Reference<XNameContainer> xNameContainer(m_xParentContainer,UNO_QUERY);
    if ( !xNameContainer.is() )
        return;

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
                    OTableStyleContext* pAutoStyle = const_cast<OTableStyleContext*>(dynamic_cast< const OTableStyleContext* >(pAutoStyles->FindStyleChildContext(XmlStyleFamily::TABLE_TABLE,m_sStyleName)));
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
        OSL_FAIL("OXMLQuery::EndElement -> exception caught");
    }

}

void OXMLTable::fillAttributes(const uno::Reference< XFastAttributeList > & _xAttrList
                                ,OUString& _rsCommand
                                ,OUString& _rsTableName
                                ,OUString& _rsTableSchema
                                ,OUString& _rsTableCatalog
                                )
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
    {
        OUString sValue = aIter.toString();

        switch( aIter.getToken() & TOKEN_MASK )
        {
            case XML_COMMAND:
                _rsCommand = sValue;
                break;
            case XML_CATALOG_NAME:
                _rsTableCatalog = sValue;
                break;
            case XML_SCHEMA_NAME:
                _rsTableSchema = sValue;
                break;
            case XML_QUERY_NAME:
                _rsTableName = sValue;
                break;
            default:
                XMLOFF_WARN_UNKNOWN("dbaccess", aIter);
        }
    }
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
