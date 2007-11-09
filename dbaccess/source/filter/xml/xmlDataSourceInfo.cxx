/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlDataSourceInfo.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-09 08:14:30 $
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
                ,const Reference< XAttributeList > & _xAttrList) :
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
        if ( !bFoundCharset )
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
