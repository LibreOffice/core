/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaxml.hxx"
#ifndef DBA_XMLDATASOURCEINFO_HXX
#include "xmlDataSourceInfo.hxx"
#endif
#ifndef DBA_XMLDATASOURCE_HXX
#include "xmlDataSource.hxx"
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
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
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
            } // if ( !bFoundThousand )
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
