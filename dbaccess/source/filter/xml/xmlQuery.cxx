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

#include "xmlQuery.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include <stringconstants.hxx>
#include <strings.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;


OXMLQuery::OXMLQuery( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const css::uno::Reference< css::container::XNameAccess >& _xParentContainer
                ) :
    OXMLTable( rImport, nPrfx, _sLocalName,_xAttrList,_xParentContainer, "com.sun.star.sdb.CommandDefinition" )
        ,m_bEscapeProcessing(true)
{

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetQueryElemTokenMap();

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
                m_sCommand = sValue;
                break;
            case XML_TOK_ESCAPE_PROCESSING:
                m_bEscapeProcessing = sValue == "true";
                break;
        }
    }
}

OXMLQuery::~OXMLQuery()
{

}

SvXMLImportContextRef OXMLQuery::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContextRef xContext = OXMLTable::CreateChildContext(nPrefix, rLocalName,xAttrList );
    if (!xContext)
    {
        const SvXMLTokenMap& rTokenMap = GetOwnImport().GetQueryElemTokenMap();

        switch( rTokenMap.Get( nPrefix, rLocalName ) )
        {
            case XML_TOK_UPDATE_TABLE:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                OUString s1;
                fillAttributes(xAttrList,s1,m_sTable,m_sSchema,m_sCatalog);
                break;
            }
        }
    }

    if (!xContext)
        xContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return xContext;
}

void OXMLQuery::setProperties(Reference< XPropertySet > & _xProp )
{
    try
    {
        if ( _xProp.is() )
        {
            OXMLTable::setProperties(_xProp);

            _xProp->setPropertyValue(PROPERTY_COMMAND,makeAny(m_sCommand));
            _xProp->setPropertyValue(PROPERTY_ESCAPE_PROCESSING,makeAny(m_bEscapeProcessing));

            if ( !m_sTable.isEmpty() )
                _xProp->setPropertyValue(PROPERTY_UPDATE_TABLENAME,makeAny(m_sTable));
            if ( !m_sCatalog.isEmpty() )
                _xProp->setPropertyValue(PROPERTY_UPDATE_CATALOGNAME,makeAny(m_sCatalog));
            if ( !m_sSchema.isEmpty() )
                _xProp->setPropertyValue(PROPERTY_UPDATE_SCHEMANAME,makeAny(m_sSchema));

            const ODBFilter::TPropertyNameMap& rSettings = GetOwnImport().getQuerySettings();
            ODBFilter::TPropertyNameMap::const_iterator aFind = rSettings.find(m_sName);
            if ( aFind != rSettings.end() )
                _xProp->setPropertyValue(PROPERTY_LAYOUTINFORMATION,makeAny(aFind->second));
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("OXMLTable::EndElement -> exception caught");
    }
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
