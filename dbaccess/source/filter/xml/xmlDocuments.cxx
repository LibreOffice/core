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

#include "xmlDocuments.hxx"
#include "xmlfilter.hxx"
#include <utility>
#include <xmloff/xmltoken.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlQuery.hxx"
#include "xmlTable.hxx"
#include "xmlComponent.hxx"
#include "xmlHierarchyCollection.hxx"
#include "xmlEnums.hxx"

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;

OXMLDocuments::OXMLDocuments( ODBFilter& rImport
                ,const Reference< XNameAccess >& _xContainer
                ,OUString _sCollectionServiceName
                ,OUString _sComponentServiceName) :
    SvXMLImportContext( rImport )
        ,m_xContainer(_xContainer)
        ,m_sCollectionServiceName(std::move(_sCollectionServiceName))
        ,m_sComponentServiceName(std::move(_sComponentServiceName))
{

}

OXMLDocuments::OXMLDocuments( ODBFilter& rImport
                ,const Reference< XNameAccess >& _xContainer
                ,OUString _sCollectionServiceName
                ) :
    SvXMLImportContext( rImport )
        ,m_xContainer(_xContainer)
        ,m_sCollectionServiceName(std::move(_sCollectionServiceName))
{
}

OXMLDocuments::~OXMLDocuments()
{

}

css::uno::Reference< css::xml::sax::XFastContextHandler > OXMLDocuments::createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    switch( nElement & TOKEN_MASK )
    {
        case XML_TABLE:
        case XML_TABLE_REPRESENTATION:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLTable( GetOwnImport(), xAttrList, m_xContainer, u"com.sun.star.sdb.TableDefinition"_ustr);
            break;
        case XML_QUERY:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLQuery( GetOwnImport(), xAttrList, m_xContainer );
            break;
        case XML_COMPONENT:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLComponent( GetOwnImport(), xAttrList, m_xContainer,m_sComponentServiceName );
            break;
        case XML_COMPONENT_COLLECTION:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLHierarchyCollection( GetOwnImport(), xAttrList, m_xContainer,m_sCollectionServiceName,m_sComponentServiceName );
            break;
    }

    return pContext;
}

ODBFilter& OXMLDocuments::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
