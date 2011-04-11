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
                ,const ::rtl::OUString& _sLocalName
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
        ::rtl::OUString sLocalName;
        rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        aProperty.Name = ::rtl::OUString();

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
        if ( aProperty.Name.getLength() )
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
                aProperty.Value <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(";"));
                rImport.addInfo(aProperty);
            }
            if ( !bFoundThousand )
            {
                aProperty.Name = INFO_THOUSANDSDELIMITER;
                aProperty.Value <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
                rImport.addInfo(aProperty);
            }
        }
        if ( XML_TOK_FONT_CHARSET == _nToken && !bFoundCharset )
        {
            aProperty.Name = INFO_CHARSET;
            aProperty.Value <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("utf8"));
            rImport.addInfo(aProperty);
        }
    }
}
// -----------------------------------------------------------------------------

OXMLDataSourceInfo::~OXMLDataSourceInfo()
{

    DBG_DTOR(OXMLDataSourceInfo,NULL);
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
