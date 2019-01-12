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
#include "xmlQuery.hxx"
#include "xmlColumn.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include <stringconstants.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <comphelper/propertysequence.hxx>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;

OXMLHierarchyCollection::OXMLHierarchyCollection( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const Reference< XNameAccess >& _xParentContainer
                ,const OUString& _sCollectionServiceName
                ,const OUString& _sComponentServiceName) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_sCollectionServiceName(_sCollectionServiceName)
    ,m_sComponentServiceName(_sComponentServiceName)
{

    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetComponentElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_COMPONENT_NAME:
                m_sName = sValue;
                break;
        }
    }
    if ( !m_sName.isEmpty() && _xParentContainer.is() )
    {
        try
        {
            Reference<XMultiServiceFactory> xORB(_xParentContainer,UNO_QUERY);
            if ( xORB.is() )
            {
                Sequence<Any> aArguments(comphelper::InitAnyPropertySequence(
                {
                    {"Name", Any(m_sName)}, // set as folder
                    {"Parent", Any(_xParentContainer)},
                }));
                m_xContainer.set(xORB->createInstanceWithArguments(_sCollectionServiceName,aArguments),UNO_QUERY);
                Reference<XNameContainer> xNameContainer(_xParentContainer,UNO_QUERY);
                if ( xNameContainer.is() && !xNameContainer->hasByName(m_sName) )
                    xNameContainer->insertByName(m_sName,makeAny(m_xContainer));
            }
        }
        catch(Exception&)
        {
            OSL_FAIL("OXMLHierarchyCollection::OXMLHierarchyCollection -> exception caught");
        }
    }
}

OXMLHierarchyCollection::OXMLHierarchyCollection( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const Reference< XNameAccess >& _xContainer
                ,const Reference< XPropertySet >& _xTable
            ) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_xContainer(_xContainer)
    ,m_xTable(_xTable)
{
}

OXMLHierarchyCollection::~OXMLHierarchyCollection()
{

}

SvXMLImportContextRef OXMLHierarchyCollection::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    const SvXMLTokenMap&    rTokenMap   = GetOwnImport().GetDocumentsElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_COMPONENT:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLComponent( GetOwnImport(), nPrefix, rLocalName,xAttrList,m_xContainer,m_sComponentServiceName );
            break;
        case XML_TOK_COLUMN:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLColumn( GetOwnImport(), nPrefix, rLocalName,xAttrList,m_xContainer,m_xTable);
            break;
        case XML_TOK_COMPONENT_COLLECTION:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLHierarchyCollection( GetOwnImport(), nPrefix, rLocalName,xAttrList,m_xContainer,m_sCollectionServiceName,m_sComponentServiceName);
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

ODBFilter& OXMLHierarchyCollection::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
