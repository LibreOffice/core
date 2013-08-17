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

#include "xmlFileBasedDatabase.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlstrings.hrc"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
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
                sal_uInt16 nPrfx, const OUString& _sLocalName,
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
    OUString sLocation,sMediaType,sFileTypeExtension;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        const OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        const OUString sValue = _xAttrList->getValueByIndex( i );

        aProperty.Name = OUString();
        aProperty.Value = Any();

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_DB_HREF:
                {
                    SvtPathOptions aPathOptions;
                    OUString sFileName = aPathOptions.SubstituteVariable(sValue);
                    if ( sValue == sFileName )
                    {
                        const sal_Int32 nFileNameLength = sFileName.getLength();
                        if ( ( nFileNameLength > 0 ) && ( sFileName.getStr()[ nFileNameLength - 1 ] == '/' ) )
                            sFileName = sFileName.copy( 0, nFileNameLength - 1 );

                        sLocation = ::svt::OFileNotation( rImport.GetAbsoluteReference( sFileName ) ).get( ::svt::OFileNotation::N_SYSTEM );
                    }

                    if ( sLocation.isEmpty() )
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
        if ( !aProperty.Name.isEmpty() )
        {
            if ( !aProperty.Value.hasValue() )
                aProperty.Value <<= sValue;
            rImport.addInfo(aProperty);
        }
    }
    if ( !(sLocation.isEmpty() || sMediaType.isEmpty()) )
    {
        ::dbaccess::ODsnTypeCollection aTypeCollection(rImport.GetComponentContext());
        OUString sURL(aTypeCollection.getDatasourcePrefixFromMediaType(sMediaType,sFileTypeExtension));
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

OXMLFileBasedDatabase::~OXMLFileBasedDatabase()
{

    DBG_DTOR(OXMLFileBasedDatabase,NULL);
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
