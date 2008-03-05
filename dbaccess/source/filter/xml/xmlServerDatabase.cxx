/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlServerDatabase.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:51:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
        if ( sType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "sdbc:mysql:jdbc" ) ) )
        {
            sURL.appendAscii("sdbc:mysql:jdbc:");
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
