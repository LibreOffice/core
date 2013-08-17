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
#include "xmlstrings.hrc"
#include <com/sun/star/beans/PropertyValue.hpp>
#include <tools/debug.hxx>
#include <vector>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLDataSourceInfo)

OXMLDataSourceInfo::OXMLDataSourceInfo( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const sal_uInt16 _nToken) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
{
    DBG_CTOR(OXMLDataSourceInfo,NULL);

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetDataSourceInfoElemTokenMap();

    PropertyValue aProperty;
    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    bool bAutoEnabled = false;
    bool bFoundField = false,bFoundThousand = false, bFoundCharset = false;
    ::std::vector< sal_uInt16 > aTokens;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        OUString sValue = _xAttrList->getValueByIndex( i );

        aProperty.Name = OUString();

        sal_uInt16 nToken = rTokenMap.Get( nPrefix, sLocalName );
        aTokens.push_back(nToken);
        switch( nToken )
        {
            case XML_TOK_ADDITIONAL_COLUMN_STATEMENT:
                aProperty.Name = PROPERTY_AUTOINCREMENTCREATION;
                bAutoEnabled = true;
                break;
            case XML_TOK_ROW_RETRIEVING_STATEMENT:
                aProperty.Name = INFO_AUTORETRIEVEVALUE;
                bAutoEnabled = true;
                break;
            case XML_TOK_STRING:
                aProperty.Name = INFO_TEXTDELIMITER;
                break;
            case XML_TOK_FIELD:
                aProperty.Name = INFO_FIELDDELIMITER;
                bFoundField = true;
                break;
            case XML_TOK_DECIMAL:
                aProperty.Name = INFO_DECIMALDELIMITER;
                break;
            case XML_TOK_THOUSAND:
                aProperty.Name = INFO_THOUSANDSDELIMITER;
                bFoundThousand = true;
                break;
            case XML_TOK_ENCODING:
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
        aProperty.Value <<= sal_True;
        rImport.addInfo(aProperty);
    }
    if ( rImport.isNewFormat() )
    {
        if ( XML_TOK_DELIMITER == _nToken )
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
        if ( XML_TOK_FONT_CHARSET == _nToken && !bFoundCharset )
        {
            aProperty.Name = INFO_CHARSET;
            aProperty.Value <<= OUString("utf8");
            rImport.addInfo(aProperty);
        }
    }
}

OXMLDataSourceInfo::~OXMLDataSourceInfo()
{

    DBG_DTOR(OXMLDataSourceInfo,NULL);
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
