/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlTableFilterList.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-09 08:18:39 $
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
