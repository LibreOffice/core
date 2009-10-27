/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xmlFileBasedDatabase.cxx,v $
 * $Revision: 1.6.68.1 $
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
#include <svtools/filenotation.hxx>
#include <svtools/pathoptions.hxx>
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
                {
                    SvtPathOptions aPathOptions;
                    rtl::OUString sFileName = aPathOptions.SubstituteVariable(sValue);
                    if ( sValue == sFileName )
                    {
                        sLocation = ::svt::OFileNotation(rImport.GetAbsoluteReference(sValue)).get( ::svt::OFileNotation::N_SYSTEM );
                    }

                    if ( sLocation.getLength() == 0 )
                        sLocation = sValue;
                }
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
        ::dbaccess::ODsnTypeCollection aTypeCollection(rImport.getORB());
        ::rtl::OUString sURL(aTypeCollection.getDatasourcePrefixFromMediaType(sMediaType,sFileTypeExtension));
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
