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
#include "xmlHierarchyCollection.hxx"
#include "xmlComponent.hxx"
#include "xmlQuery.hxx"
#include "xmlColumn.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlstrings.hrc"
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <tools/debug.hxx>


namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLHierarchyCollection)

OXMLHierarchyCollection::OXMLHierarchyCollection( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const Reference< XNameAccess >& _xParentContainer
                ,const ::rtl::OUString& _sCollectionServiceName
                ,const ::rtl::OUString& _sComponentServiceName) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_xParentContainer(_xParentContainer)
    ,m_sCollectionServiceName(_sCollectionServiceName)
    ,m_sComponentServiceName(_sComponentServiceName)
{
    DBG_CTOR(OXMLHierarchyCollection,NULL);

    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetComponentElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        ::rtl::OUString sLocalName;
        rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_COMPONENT_NAME:
                m_sName = sValue;
                break;
        }
    }
    if ( m_sName.getLength() && _xParentContainer.is() )
    {
        try
        {
            Sequence< Any > aArguments(2);
            PropertyValue aValue;
            // set as folder
            aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name"));
            aValue.Value <<= m_sName;
            aArguments[0] <<= aValue;
            //parent
            aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Parent"));
            aValue.Value <<= _xParentContainer;
            aArguments[1] <<= aValue;

            Reference<XMultiServiceFactory> xORB(_xParentContainer,UNO_QUERY);
            if ( xORB.is() )
            {
                m_xContainer.set(xORB->createInstanceWithArguments(_sCollectionServiceName,aArguments),UNO_QUERY);
                Reference<XNameContainer> xNameContainer(_xParentContainer,UNO_QUERY);
                if ( xNameContainer.is() && !xNameContainer->hasByName(m_sName) )
                    xNameContainer->insertByName(m_sName,makeAny(m_xContainer));
            }
        }
        catch(Exception&)
        {
            OSL_FAIL("OXMLHierarchyCollection::OXMLHierarchyCollection -> exception catched");
        }
    }
}
// -----------------------------------------------------------------------------
OXMLHierarchyCollection::OXMLHierarchyCollection( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XNameAccess >& _xContainer
                ,const Reference< XPropertySet >& _xTable
            ) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_xContainer(_xContainer)
    ,m_xTable(_xTable)
{
    DBG_CTOR(OXMLHierarchyCollection,NULL);
}
// -----------------------------------------------------------------------------

OXMLHierarchyCollection::~OXMLHierarchyCollection()
{

    DBG_DTOR(OXMLHierarchyCollection,NULL);
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLHierarchyCollection::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
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
// -----------------------------------------------------------------------------
ODBFilter& OXMLHierarchyCollection::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
