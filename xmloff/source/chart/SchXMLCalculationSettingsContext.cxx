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
#include "precompiled_xmloff.hxx"
#include <SchXMLCalculationSettingsContext.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>


using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

SchXMLCalculationSettingsContext::SchXMLCalculationSettingsContext( SvXMLImport& rImport,
                                    sal_uInt16 p_nPrefix,
                                    const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                         ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
: SvXMLImportContext ( rImport, p_nPrefix, rLocalName )
{
    const SvXMLNamespaceMap& rMap = GetImport().GetNamespaceMap();
    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName(sAttrName, &aLocalName );
        if ( nPrefix == XML_NAMESPACE_TABLE && IsXMLToken( aLocalName, XML_DATE_VALUE ) )
        {
            util::DateTime aNullDate;
            const rtl::OUString sValue = xAttrList->getValueByIndex( i );
            GetImport().GetMM100UnitConverter().convertDateTime(aNullDate, sValue);
            m_aNullDate <<= aNullDate;
        }
    }
}
SvXMLImportContext* SchXMLCalculationSettingsContext::CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
{
    return new SchXMLCalculationSettingsContext(GetImport(),nPrefix,rLocalName,xAttrList);
}

void SchXMLCalculationSettingsContext::EndElement()
{
    if ( m_aNullDate.hasValue() )
    {
        Reference < XPropertySet > xPropSet ( GetImport().GetModel(), UNO_QUERY );
        ::rtl::OUString sNullDate( RTL_CONSTASCII_USTRINGPARAM ( "NullDate" ) );
        xPropSet->setPropertyValue ( sNullDate, m_aNullDate );
    }
}
