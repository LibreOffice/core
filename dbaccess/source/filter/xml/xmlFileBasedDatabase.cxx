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
#include <xmloff/xmlnamespace.hxx>
#include <strings.hxx>
#include <tools/diagnose_ex.h>
#include <svl/filenotation.hxx>
#include <unotools/pathoptions.hxx>
#include <dsntypes.hxx>
namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

OXMLFileBasedDatabase::OXMLFileBasedDatabase( ODBFilter& rImport,
                const Reference< XFastAttributeList > & _xAttrList) :
    SvXMLImportContext( rImport )
{
    Reference<XPropertySet> xDataSource = rImport.getDataSource();

    PropertyValue aProperty;

    OUString sLocation,sMediaType,sFileTypeExtension;
    if (xDataSource.is())
    {
        for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
        {
            OUString sValue = aIter.toString();

            aProperty.Name.clear();
            aProperty.Value = Any();

            switch( aIter.getToken() )
            {
                case XML_ELEMENT(XLINK, XML_HREF):
                    {
                        SvtPathOptions aPathOptions;
                        OUString sFileName = aPathOptions.SubstituteVariable(sValue);
                        if ( sValue == sFileName )
                        {
                            const sal_Int32 nFileNameLength = sFileName.getLength();
                            if ( sFileName.endsWith("/") )
                                sFileName = sFileName.copy( 0, nFileNameLength - 1 );

                            sLocation = ::svt::OFileNotation( rImport.GetAbsoluteReference( sFileName ) ).get( ::svt::OFileNotation::N_SYSTEM );
                        }

                        if ( sLocation.isEmpty() )
                            sLocation = sValue;
                    }
                    break;
                case XML_ELEMENT(DB, XML_MEDIA_TYPE):
                case XML_ELEMENT(DB_OASIS, XML_MEDIA_TYPE):
                    sMediaType = sValue;
                    break;
                case XML_ELEMENT(DB, XML_EXTENSION):
                case XML_ELEMENT(DB_OASIS, XML_EXTENSION):
                    aProperty.Name = INFO_TEXTFILEEXTENSION;
                    sFileTypeExtension = sValue;
                    break;
                default:
                    SAL_WARN("dbaccess", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << aIter.toString());
            }
            if ( !aProperty.Name.isEmpty() )
            {
                if ( !aProperty.Value.hasValue() )
                    aProperty.Value <<= sValue;
                rImport.addInfo(aProperty);
            }
        }
    }
    if ( sLocation.isEmpty() || sMediaType.isEmpty() )
        return;

    ::dbaccess::ODsnTypeCollection aTypeCollection(rImport.GetComponentContext());
    OUString sURL = aTypeCollection.getDatasourcePrefixFromMediaType(sMediaType,sFileTypeExtension) + sLocation;
    try
    {
        xDataSource->setPropertyValue(PROPERTY_URL,makeAny(sURL));
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

OXMLFileBasedDatabase::~OXMLFileBasedDatabase()
{

}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
