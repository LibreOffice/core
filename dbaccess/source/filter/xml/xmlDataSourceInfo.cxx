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
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <strings.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

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
    for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
    {
        aProperty.Name.clear();

        switch( aIter.getToken() & TOKEN_MASK )
        {
            case XML_ADDITIONAL_COLUMN_STATEMENT:
                aProperty.Name = PROPERTY_AUTOINCREMENTCREATION;
                bAutoEnabled = true;
                break;
            case XML_ROW_RETRIEVING_STATEMENT:
                aProperty.Name = INFO_AUTORETRIEVEVALUE;
                bAutoEnabled = true;
                break;
            case XML_STRING:
                aProperty.Name = INFO_TEXTDELIMITER;
                break;
            case XML_FIELD:
                aProperty.Name = INFO_FIELDDELIMITER;
                bFoundField = true;
                break;
            case XML_DECIMAL:
                aProperty.Name = INFO_DECIMALDELIMITER;
                break;
            case XML_THOUSAND:
                aProperty.Name = INFO_THOUSANDSDELIMITER;
                bFoundThousand = true;
                break;
            case XML_ENCODING:
                aProperty.Name = INFO_CHARSET;
                bFoundCharset = true;
                break;
            default:
                XMLOFF_WARN_UNKNOWN("dbaccess", aIter);
        }
        if ( !aProperty.Name.isEmpty() )
        {
            aProperty.Value <<= aIter.toString();
            rImport.addInfo(aProperty);
        }
    }
    if ( bAutoEnabled )
    {
        aProperty.Name = INFO_AUTORETRIEVEENABLED;
        aProperty.Value <<= true;
        rImport.addInfo(aProperty);
    }
    if ( !rImport.isNewFormat() )
        return;

    if ( (nElement & TOKEN_MASK) == XML_DELIMITER )
    {
        if ( !bFoundField )
        {
            aProperty.Name = INFO_FIELDDELIMITER;
            aProperty.Value <<= u";"_ustr;
            rImport.addInfo(aProperty);
        }
        if ( !bFoundThousand )
        {
            aProperty.Name = INFO_THOUSANDSDELIMITER;
            aProperty.Value <<= u","_ustr;
            rImport.addInfo(aProperty);
        }
    }
    if ( (nElement & TOKEN_MASK) == XML_FONT_CHARSET && !bFoundCharset )
    {
        aProperty.Name = INFO_CHARSET;
        aProperty.Value <<= u"utf8"_ustr;
        rImport.addInfo(aProperty);
    }
}

OXMLDataSourceInfo::~OXMLDataSourceInfo()
{

}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
