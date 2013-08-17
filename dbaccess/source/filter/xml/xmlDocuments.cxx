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
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include "xmlQuery.hxx"
#include "xmlTable.hxx"
#include "xmlComponent.hxx"
#include "xmlHierarchyCollection.hxx"
#include "xmlEnums.hxx"
#include "xmlstrings.hrc"
#include <tools/debug.hxx>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLDocuments)

OXMLDocuments::OXMLDocuments( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                , const OUString& rLName
                ,const Reference< XNameAccess >& _xContainer
                ,const OUString& _sCollectionServiceName
                ,const OUString& _sComponentServiceName) :
    SvXMLImportContext( rImport, nPrfx, rLName )
        ,m_xContainer(_xContainer)
        ,m_sCollectionServiceName(_sCollectionServiceName)
        ,m_sComponentServiceName(_sComponentServiceName)
{
    DBG_CTOR(OXMLDocuments,NULL);

}

OXMLDocuments::OXMLDocuments( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                , const OUString& rLName
                ,const Reference< XNameAccess >& _xContainer
                ,const OUString& _sCollectionServiceName
                ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
        ,m_xContainer(_xContainer)
        ,m_sCollectionServiceName(_sCollectionServiceName)
{
    DBG_CTOR(OXMLDocuments,NULL);
}

OXMLDocuments::~OXMLDocuments()
{

    DBG_DTOR(OXMLDocuments,NULL);
}

SvXMLImportContext* OXMLDocuments::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    const SvXMLTokenMap&    rTokenMap   = GetOwnImport().GetDocumentsElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_TABLE:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLTable( GetOwnImport(), nPrefix, rLocalName, xAttrList, m_xContainer, "com.sun.star.sdb.TableDefinition");
            break;
        case XML_TOK_QUERY:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLQuery( GetOwnImport(), nPrefix, rLocalName,xAttrList,m_xContainer );
            break;
        case XML_TOK_COMPONENT:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLComponent( GetOwnImport(), nPrefix, rLocalName,xAttrList,m_xContainer,m_sComponentServiceName );
            break;
        case XML_TOK_COMPONENT_COLLECTION:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLHierarchyCollection( GetOwnImport(), nPrefix, rLocalName,xAttrList,m_xContainer,m_sCollectionServiceName,m_sComponentServiceName );
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

ODBFilter& OXMLDocuments::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
