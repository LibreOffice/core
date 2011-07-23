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
                sal_uInt16 nPrfx, const ::rtl::OUString& _sLocalName,
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
    ::rtl::OUString sType,sHostName,sPortNumber,sDatabaseName;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        ::rtl::OUString sLocalName;
        const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

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
                aProperty.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LocalSocket"));
                aProperty.Value <<= sValue;
                rImport.addInfo(aProperty);
                break;
            case XML_TOK_DATABASE_NAME:
                sDatabaseName = sValue;
                break;
        }
    }
    if ( sType.getLength() )
    {
        ::rtl::OUStringBuffer sURL;
        if  (   sType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "sdbc:mysql:jdbc" ) )
            ||  sType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "sdbc:mysqlc" ) )
            ||  sType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "sdbc:mysql:mysqlc" ) )
            )
        {
            sURL.append( sType );
            sURL.append( sal_Unicode( ':' ) );
            sURL.append(sHostName);
            if ( sPortNumber.getLength() )
            {
                sURL.appendAscii(":");
                sURL.append(sPortNumber);
            }
            if ( sDatabaseName.getLength() )
            {
                sURL.appendAscii("/");
                sURL.append(sDatabaseName);
            }
        }
        else if ( sType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "jdbc:oracle:thin" ) ) )
        {
            sURL.appendAscii("jdbc:oracle:thin:@");
            sURL.append(sHostName);
            if ( sPortNumber.getLength() )
            {
                sURL.appendAscii(":");
                sURL.append(sPortNumber);
            }
            if ( sDatabaseName.getLength() )
            {
                sURL.appendAscii(":");
                sURL.append(sDatabaseName);
            }
        }
        else if ( sType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "sdbc:address:ldap" ) ) )
        {
            sURL.appendAscii("sdbc:address:ldap:");
            sURL.append(sHostName);
            if ( sPortNumber.getLength() )
            {
                sURL.appendAscii(":");
                sURL.append(sPortNumber);
            }
        }
        else
        {
            sURL.append(sType);
            sURL.appendAscii(":");
            sURL.append(sHostName);
            if ( sPortNumber.getLength() )
            {
                sURL.appendAscii(":");
                sURL.append(sPortNumber);
            }
            if ( sDatabaseName.getLength() )
            {
                sURL.appendAscii(":");
                sURL.append(sDatabaseName);
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
// -----------------------------------------------------------------------------

OXMLServerDatabase::~OXMLServerDatabase()
{

    DBG_DTOR(OXMLServerDatabase,NULL);
}
// -----------------------------------------------------------------------------

//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
