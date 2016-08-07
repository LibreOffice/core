/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaxml.hxx"
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
