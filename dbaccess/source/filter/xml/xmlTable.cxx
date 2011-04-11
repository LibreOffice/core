/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
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
                ,const ::rtl::OUString& _sLocalName
                ,const uno::Reference< XAttributeList > & _xAttrList
                ,const uno::Reference< ::com::sun::star::container::XNameAccess >& _xParentContainer
                ,const ::rtl::OUString& _sServiceName
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
        ::rtl::OUString sLocalName;
        rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        rtl::OUString sValue = _xAttrList->getValueByIndex( i );

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
                m_bApplyFilter = sValue.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("true"));
                break;
            case XML_TOK_APPLY_ORDER:
                m_bApplyOrder = sValue.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("true"));
                break;
        }
    }
    Sequence< Any > aArguments(2);
    PropertyValue aValue;
    // set as folder
    aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name"));
    aValue.Value <<= m_sName;
    aArguments[0] <<= aValue;
    //parent
    aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Parent"));
    aValue.Value <<= m_xParentContainer;
    aArguments[1] <<= aValue;
    m_xTable.set(GetOwnImport().getORB()->createInstanceWithArguments(m_sServiceName,aArguments),UNO_QUERY);
}
// -----------------------------------------------------------------------------

OXMLTable::~OXMLTable()
{

    DBG_DTOR(OXMLTable,NULL);
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLTable::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const uno::Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    const SvXMLTokenMap&    rTokenMap   = GetOwnImport().GetQueryElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_FILTER_STATEMENT:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                ::rtl::OUString s1,s2,s3;
                fillAttributes(nPrefix, rLocalName,xAttrList,m_sFilterStatement,s1,s2,s3);
            }
            break;
        case XML_TOK_ORDER_STATEMENT:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                ::rtl::OUString s1,s2,s3;
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
// -----------------------------------------------------------------------------
ODBFilter& OXMLTable::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
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

                if ( m_sStyleName.getLength() )
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
// -----------------------------------------------------------------------------
void OXMLTable::fillAttributes(sal_uInt16 /*nPrfx*/
                                ,const ::rtl::OUString& /*_sLocalName*/
                                ,const uno::Reference< XAttributeList > & _xAttrList
                                , ::rtl::OUString& _rsCommand
                                ,::rtl::OUString& _rsTableName
                                ,::rtl::OUString& _rsTableSchema
                                ,::rtl::OUString& _rsTableCatalog
                                )
{
    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = GetOwnImport().GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = GetOwnImport().GetQueryElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        ::rtl::OUString sLocalName;
        rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        rtl::OUString sValue = _xAttrList->getValueByIndex( i );

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
//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
