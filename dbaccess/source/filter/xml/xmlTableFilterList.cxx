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
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <strings.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/sequence.hxx>
#include <xmloff/xmlimp.hxx>
#include "xmlfilter.hxx"

namespace dbaxml
{
    using namespace ::xmloff::token;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xml::sax;

OXMLTableFilterList::OXMLTableFilterList( SvXMLImport& rImport)
    :SvXMLImportContext( rImport )
{

}

OXMLTableFilterList::~OXMLTableFilterList()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > OXMLTableFilterList::createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    SvXMLImportContext *pContext = nullptr;

    if ( IsTokenInNamespace(nElement, XML_NAMESPACE_DB) ||
         IsTokenInNamespace(nElement, XML_NAMESPACE_DB_OASIS) )
    {
        GetImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
        switch (nElement & TOKEN_MASK)
        {
            case XML_TABLE_FILTER_PATTERN:
                pContext = new OXMLTableFilterPattern( GetImport(), true,*this);
                break;
            case XML_TABLE_TYPE:
                pContext = new OXMLTableFilterPattern( GetImport(), false,*this);
                break;
            case XML_TABLE_INCLUDE_FILTER:
                pContext = new OXMLTableFilterList( GetImport() );
                break;
            default: break;
        }
    }

    return pContext;
}

ODBFilter& OXMLTableFilterList::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}

void OXMLTableFilterList::endFastElement(sal_Int32 )
{
    Reference<XPropertySet> xDataSource(GetOwnImport().getDataSource());
    if ( xDataSource.is() )
    {
        if ( !m_aPatterns.empty() )
            xDataSource->setPropertyValue(PROPERTY_TABLEFILTER,makeAny(comphelper::containerToSequence(m_aPatterns)));
        if ( !m_aTypes.empty() )
            xDataSource->setPropertyValue(PROPERTY_TABLETYPEFILTER,makeAny(comphelper::containerToSequence(m_aTypes)));
    }
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
