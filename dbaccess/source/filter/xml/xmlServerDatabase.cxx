/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xmlServerDatabase.cxx,v $
 * $Revision: 1.5 $
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
#ifndef DBA_XMLSERVERDATABASE_HXX_INCLUDED
#include "xmlServerDatabase.hxx"
#endif
#ifndef DBA_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef DBA_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef DBACCESS_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif

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
        catch(Exception)
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
