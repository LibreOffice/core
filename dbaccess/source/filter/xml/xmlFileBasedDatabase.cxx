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
#include "xmlFileBasedDatabase.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlstrings.hrc"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/sequence.hxx>
#include <svl/filenotation.hxx>
#include <unotools/pathoptions.hxx>
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
                        const sal_Int32 nFileNameLength = sFileName.getLength();
                        if ( ( nFileNameLength > 0 ) && ( sFileName.getStr()[ nFileNameLength - 1 ] == '/' ) )
                            sFileName = sFileName.copy( 0, nFileNameLength - 1 );

                        sLocation = ::svt::OFileNotation( rImport.GetAbsoluteReference( sFileName ) ).get( ::svt::OFileNotation::N_SYSTEM );
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
        catch(const Exception&)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
