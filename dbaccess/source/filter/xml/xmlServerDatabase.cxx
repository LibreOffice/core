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

#include "xmlServerDatabase.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlstrings.hrc"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLServerDatabase)

OXMLServerDatabase::OXMLServerDatabase( ODBFilter& rImport,
                sal_uInt16 nPrfx, const OUString& _sLocalName,
                const Reference< XAttributeList > & _xAttrList) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
{
    DBG_CTOR(OXMLServerDatabase,NULL);

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetDataSourceElemTokenMap();

    Reference<XPropertySet> xDataSource = rImport.getDataSource();

    PropertyValue aProperty;

    const sal_Int16 nLength = (xDataSource.is() && _xAttrList.is()) ? _xAttrList->getLength() : 0;
    OUString sType,sHostName,sPortNumber,sDatabaseName;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        const OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        const OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_DB_TYPE:
                sType = sValue;
                break;
            case XML_TOK_HOSTNAME:
                sHostName = sValue;
                break;
            case XML_TOK_PORT:
                sPortNumber = sValue;
                break;
            case XML_TOK_LOCAL_SOCKET:
                aProperty.Name = OUString("LocalSocket");
                aProperty.Value <<= sValue;
                rImport.addInfo(aProperty);
                break;
            case XML_TOK_DATABASE_NAME:
                sDatabaseName = sValue;
                break;
        }
    }
    if ( !sType.isEmpty() )
    {
        OUStringBuffer sURL;
        if  ( sType == "sdbc:mysql:jdbc" || sType == "sdbc:mysqlc" || sType == "sdbc:mysql:mysqlc" )
        {
            sURL.append( sType + ":" + sHostName);
            if ( !sPortNumber.isEmpty() )
            {
                sURL.append(":" + sPortNumber);
            }
            if ( !sDatabaseName.isEmpty() )
            {
                sURL.append("/" + sDatabaseName);
            }
        }
        else if ( sType == "jdbc:oracle:thin" )
        {
            sURL.append("jdbc:oracle:thin:@" + sHostName);
            if ( !sPortNumber.isEmpty() )
            {
                sURL.append(":" + sPortNumber);
            }
            if ( !sDatabaseName.isEmpty() )
            {
                sURL.append(":" + sDatabaseName);
            }
        }
        else if ( sType == "sdbc:address:ldap" )
        {
            sURL.append("sdbc:address:ldap:" + sHostName);
            if ( !sPortNumber.isEmpty() )
            {
                sURL.append(":" + sPortNumber);
            }
        }
        else
        {
            sURL.append(sType + ":" + sHostName);
            if ( !sPortNumber.isEmpty() )
            {
                sURL.append(":" + sPortNumber);
            }
            if ( !sDatabaseName.isEmpty() )
            {
                sURL.append(":" + sDatabaseName);
            }
        }
        try
        {
            xDataSource->setPropertyValue(PROPERTY_URL,makeAny(sURL.makeStringAndClear()));
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

OXMLServerDatabase::~OXMLServerDatabase()
{

    DBG_DTOR(OXMLServerDatabase,NULL);
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
