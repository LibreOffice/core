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
#include "xmlTableFilterList.hxx"
#include "xmlTableFilterPattern.hxx"
#include "xmlEnums.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmlstrings.hrc"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <xmloff/xmlimp.hxx>
#include "xmlfilter.hxx"
#include <tools/debug.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
