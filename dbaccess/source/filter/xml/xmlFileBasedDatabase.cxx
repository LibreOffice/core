/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlFileBasedDatabase.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:48:31 $
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
#ifndef DBA_XMLFILEBASEDDATABASE_HXX_INCLUDED
#include "xmlFileBasedDatabase.hxx"
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
#include <comphelper/sequence.hxx>
#include "dsntypes.hxx"
namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLFileBasedDatabase)

OXMLFileBasedDatabase::OXMLFileBasedDatabase( ODBFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& _sLocalName,
                const Reference< XAttributeList > & _xAttrList) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
{
    DBG_CTOR(OXMLFileBasedDatabase,NULL);

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetDataSourceElemTokenMap();

    Reference<XPropertySet> xDataSource = rImport.getDataSource();

    PropertyValue aProperty;

    const sal_Int16 nLength = (xDataSource.is() && _xAttrList.is()) ? _xAttrList->getLength() : 0;
    ::rtl::OUString sLocation,sMediaType,sFileTypeExtension;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        ::rtl::OUString sLocalName;
        const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        aProperty.Name = ::rtl::OUString();
        aProperty.Value = Any();

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_DB_HREF:
                sLocation = sValue;
                break;
            case XML_TOK_MEDIA_TYPE:
                sMediaType = sValue;
                break;
            case XML_TOK_EXTENSION:
                aProperty.Name = INFO_TEXTFILEEXTENSION;
                sFileTypeExtension = sValue;
                break;
        }
        if ( aProperty.Name.getLength() )
        {
            if ( !aProperty.Value.hasValue() )
                aProperty.Value <<= sValue;
            rImport.addInfo(aProperty);
        }
    }
    if ( sLocation.getLength() && sMediaType.getLength() )
    {
        ::rtl::OUString sURL(dbaui::ODsnTypeCollection::getDatasourcePrefixFromMediaType(sMediaType,sFileTypeExtension));
        sURL += sLocation;
        try
        {
            xDataSource->setPropertyValue(PROPERTY_URL,makeAny(sURL));
        }
        catch(Exception)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}
// -----------------------------------------------------------------------------

OXMLFileBasedDatabase::~OXMLFileBasedDatabase()
{

    DBG_DTOR(OXMLFileBasedDatabase,NULL);
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
