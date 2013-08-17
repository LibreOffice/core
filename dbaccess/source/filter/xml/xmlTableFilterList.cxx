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

#include "xmlTableFilterList.hxx"
#include "xmlTableFilterPattern.hxx"
#include "xmlEnums.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
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

OXMLTableFilterList::OXMLTableFilterList( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& _sLocalName )
    :SvXMLImportContext( rImport, nPrfx, _sLocalName )
{
    DBG_CTOR(OXMLTableFilterList,NULL);

}

OXMLTableFilterList::~OXMLTableFilterList()
{
    DBG_DTOR(OXMLTableFilterList,NULL);
}

SvXMLImportContext* OXMLTableFilterList::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
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

ODBFilter& OXMLTableFilterList::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}

void OXMLTableFilterList::EndElement()
{
    Reference<XPropertySet> xDataSource(GetOwnImport().getDataSource());
    if ( xDataSource.is() )
    {
        if ( !m_aPatterns.empty() )
            xDataSource->setPropertyValue(PROPERTY_TABLEFILTER,makeAny(Sequence< OUString>(&(*m_aPatterns.begin()),m_aPatterns.size())));
        if ( !m_aTypes.empty() )
            xDataSource->setPropertyValue(PROPERTY_TABLETYPEFILTER,makeAny(Sequence< OUString>(&(*m_aTypes.begin()),m_aTypes.size())));
    }
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
