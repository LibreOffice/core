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

#include "xmlDataSourceInfo.hxx"
#include "xmlDataSource.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include <stringconstants.hxx>
#include <strings.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vector>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

OXMLDataSourceInfo::OXMLDataSourceInfo( ODBFilter& rImport
                ,sal_Int32 nElement
                ,const Reference< XFastAttributeList > & _xAttrList) :
    SvXMLImportContext( rImport )
{
    PropertyValue aProperty;
    bool bAutoEnabled = false;
    bool bFoundField = false,bFoundThousand = false, bFoundCharset = false;
    sax_fastparser::FastAttributeList *pAttribList =
                sax_fastparser::FastAttributeList::castToFastAttributeList( _xAttrList );
    for (auto &aIter : *pAttribList)
    {
        OUString sValue = aIter.toString();

        aProperty.Name.clear();

        switch( aIter.getToken() )
        {
            case XML_ELEMENT(DB, XML_ADDITIONAL_COLUMN_STATEMENT):
                aProperty.Name = PROPERTY_AUTOINCREMENTCREATION;
                bAutoEnabled = true;
                break;
            case XML_ELEMENT(DB, XML_ROW_RETRIEVING_STATEMENT):
                aProperty.Name = INFO_AUTORETRIEVEVALUE;
                bAutoEnabled = true;
                break;
            case XML_ELEMENT(DB, XML_STRING):
                aProperty.Name = INFO_TEXTDELIMITER;
                break;
            case XML_ELEMENT(DB, XML_FIELD):
                aProperty.Name = INFO_FIELDDELIMITER;
                bFoundField = true;
                break;
            case XML_ELEMENT(DB, XML_DECIMAL):
                aProperty.Name = INFO_DECIMALDELIMITER;
                break;
            case XML_ELEMENT(DB, XML_THOUSAND):
                aProperty.Name = INFO_THOUSANDSDELIMITER;
                bFoundThousand = true;
                break;
            case XML_ELEMENT(DB, XML_ENCODING):
                aProperty.Name = INFO_CHARSET;
                bFoundCharset = true;
                break;
        }
        if ( !aProperty.Name.isEmpty() )
        {
            aProperty.Value <<= sValue;
            rImport.addInfo(aProperty);
        }
    }
    if ( bAutoEnabled )
    {
        aProperty.Name = INFO_AUTORETRIEVEENABLED;
        aProperty.Value <<= true;
        rImport.addInfo(aProperty);
    }
    if ( rImport.isNewFormat() )
    {
        if ( XML_ELEMENT(DB, XML_DELIMITER) == nElement )
        {
            if ( !bFoundField )
            {
                aProperty.Name = INFO_FIELDDELIMITER;
                aProperty.Value <<= OUString(";");
                rImport.addInfo(aProperty);
            }
            if ( !bFoundThousand )
            {
                aProperty.Name = INFO_THOUSANDSDELIMITER;
                aProperty.Value <<= OUString(",");
                rImport.addInfo(aProperty);
            }
        }
        if ( XML_ELEMENT(DB, XML_FONT_CHARSET) == nElement && !bFoundCharset )
        {
            aProperty.Name = INFO_CHARSET;
            aProperty.Value <<= OUString("utf8");
            rImport.addInfo(aProperty);
        }
    }
}

OXMLDataSourceInfo::~OXMLDataSourceInfo()
{

}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
