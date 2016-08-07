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
#ifndef DBA_XMLTABLEFILTERLIST_HXX
#include "xmlTableFilterList.hxx"
#endif
#ifndef DBA_XMLTABLEFILTERPATTERN_HXX
#include "xmlTableFilterPattern.hxx"
#endif
#ifndef DBA_XMLENUMS_HXX
#include "xmlEnums.hxx"
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
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef DBACCESS_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef DBA_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

namespace dbaxml
{
    using namespace ::xmloff::token;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLTableFilterList)

OXMLTableFilterList::OXMLTableFilterList( SvXMLImport& rImport, sal_uInt16 nPrfx, const ::rtl::OUString& _sLocalName )
    :SvXMLImportContext( rImport, nPrfx, _sLocalName )
{
    DBG_CTOR(OXMLTableFilterList,NULL);

}
// -----------------------------------------------------------------------------

OXMLTableFilterList::~OXMLTableFilterList()
{
    DBG_DTOR(OXMLTableFilterList,NULL);
}
// -----------------------------------------------------------------------------

SvXMLImportContext* OXMLTableFilterList::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & /*xAttrList*/ )
{
    SvXMLImportContext *pContext = 0;

    if ( XML_NAMESPACE_DB == nPrefix )
    {
        GetImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
        if ( IsXMLToken( rLocalName, XML_TABLE_FILTER_PATTERN ) )
            pContext = new OXMLTableFilterPattern( GetImport(), nPrefix, rLocalName,sal_True,*this);
        else if ( IsXMLToken( rLocalName, XML_TABLE_TYPE ) )
            pContext = new OXMLTableFilterPattern( GetImport(), nPrefix, rLocalName,sal_False,*this);
        else if ( IsXMLToken( rLocalName, XML_TABLE_INCLUDE_FILTER ) )
            pContext = new OXMLTableFilterList( GetImport(), nPrefix, rLocalName );
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
ODBFilter& OXMLTableFilterList::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}

// -----------------------------------------------------------------------------
void OXMLTableFilterList::EndElement()
{
    Reference<XPropertySet> xDataSource(GetOwnImport().getDataSource());
    if ( xDataSource.is() )
    {
        if ( !m_aPatterns.empty() )
            xDataSource->setPropertyValue(PROPERTY_TABLEFILTER,makeAny(Sequence< ::rtl::OUString>(&(*m_aPatterns.begin()),m_aPatterns.size())));
        if ( !m_aTypes.empty() )
            xDataSource->setPropertyValue(PROPERTY_TABLETYPEFILTER,makeAny(Sequence< ::rtl::OUString>(&(*m_aTypes.begin()),m_aTypes.size())));
    }
}
//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
