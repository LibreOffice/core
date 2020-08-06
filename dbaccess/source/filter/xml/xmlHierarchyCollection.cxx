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

#include "xmlHierarchyCollection.hxx"
#include "xmlComponent.hxx"
#include "xmlColumn.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include <com/sun/star/container/XNameContainer.hpp>
#include <comphelper/propertysequence.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;

OXMLHierarchyCollection::OXMLHierarchyCollection( ODBFilter& rImport
                ,const Reference< XFastAttributeList > & _xAttrList
                ,const Reference< XNameAccess >& _xParentContainer
                ,const OUString& _sCollectionServiceName
                ,const OUString& _sComponentServiceName) :
    SvXMLImportContext( rImport )
    ,m_sCollectionServiceName(_sCollectionServiceName)
    ,m_sComponentServiceName(_sComponentServiceName)
{
    OUString sName;
    for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
    {
        OUString sValue = aIter.toString();

        switch( aIter.getToken() & TOKEN_MASK )
        {
            case XML_NAME:
                sName = sValue;
                break;
            default:
                SAL_WARN("dbaccess", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << aIter.toString());
        }
    }
    if ( sName.isEmpty() || !_xParentContainer.is() )
        return;

    try
    {
        Reference<XMultiServiceFactory> xORB(_xParentContainer,UNO_QUERY);
        if ( xORB.is() )
        {
            Sequence<Any> aArguments(comphelper::InitAnyPropertySequence(
            {
                {"Name", Any(sName)}, // set as folder
                {"Parent", Any(_xParentContainer)},
            }));
            m_xContainer.set(xORB->createInstanceWithArguments(_sCollectionServiceName,aArguments),UNO_QUERY);
            Reference<XNameContainer> xNameContainer(_xParentContainer,UNO_QUERY);
            if ( xNameContainer.is() && !xNameContainer->hasByName(sName) )
                xNameContainer->insertByName(sName,makeAny(m_xContainer));
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("OXMLHierarchyCollection::OXMLHierarchyCollection -> exception caught");
    }
}

OXMLHierarchyCollection::OXMLHierarchyCollection( ODBFilter& rImport
                ,const Reference< XNameAccess >& _xContainer
                ,const Reference< XPropertySet >& _xTable
            ) :
    SvXMLImportContext( rImport )
    ,m_xContainer(_xContainer)
    ,m_xTable(_xTable)
{
}

OXMLHierarchyCollection::~OXMLHierarchyCollection()
{

}

css::uno::Reference< css::xml::sax::XFastContextHandler > OXMLHierarchyCollection::createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    switch( nElement & TOKEN_MASK )
    {
        case XML_COMPONENT:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLComponent( GetOwnImport(), xAttrList,m_xContainer,m_sComponentServiceName );
            break;
        case XML_COLUMN:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLColumn( GetOwnImport(), xAttrList,m_xContainer,m_xTable);
            break;
        case XML_COMPONENT_COLLECTION:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLHierarchyCollection( GetOwnImport(), xAttrList,m_xContainer,m_sCollectionServiceName,m_sComponentServiceName);
            break;
    }

    return pContext;
}

ODBFilter& OXMLHierarchyCollection::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
