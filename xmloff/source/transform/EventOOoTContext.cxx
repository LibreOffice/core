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
#include "precompiled_xmloff.hxx"
#include "EventOOoTContext.hxx"
#include "EventMap.hxx"
#include "MutableAttrList.hxx"
#include "xmloff/xmlnmspe.hxx"
#include "ActionMapTypesOOo.hxx"
#include "AttrTransformerAction.hxx"
#include "TransformerActions.hxx"
#include "TransformerBase.hxx"
#include <comphelper/stl_types.hxx>
#include <rtl/ustrbuf.hxx>

#include <boost/unordered_map.hpp>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

class XMLTransformerOOoEventMap_Impl:
    public ::boost::unordered_map< ::rtl::OUString, NameKey_Impl,
                            ::rtl::OUStringHash, ::comphelper::UStringEqual >
{
public:

    void AddMap( XMLTransformerEventMapEntry *pInit );

    XMLTransformerOOoEventMap_Impl( XMLTransformerEventMapEntry *pInit,
                                       XMLTransformerEventMapEntry *pInit2  );
    ~XMLTransformerOOoEventMap_Impl();
};

void XMLTransformerOOoEventMap_Impl::AddMap( XMLTransformerEventMapEntry *pInit )
{
    XMLTransformerOOoEventMap_Impl::key_type aKey;
    XMLTransformerOOoEventMap_Impl::mapped_type aData;
    while( pInit->m_pOOoName )
    {
        aKey = OUString::createFromAscii(pInit->m_pOOoName);

        OSL_ENSURE( find( aKey ) == end(), "duplicate event map entry" );

        aData.m_nPrefix = pInit->m_nOASISPrefix;
        aData.m_aLocalName = OUString::createFromAscii(pInit->m_pOASISName);

        XMLTransformerOOoEventMap_Impl::value_type aVal( aKey, aData );

        if( !insert( aVal ).second )
        {
            OSL_FAIL( "duplicate OOo event name extry" );
        }

        ++pInit;
    }
}

XMLTransformerOOoEventMap_Impl::XMLTransformerOOoEventMap_Impl(
        XMLTransformerEventMapEntry *pInit,
        XMLTransformerEventMapEntry *pInit2 )
{
    if( pInit )
        AddMap( pInit );
    if( pInit )
        AddMap( pInit2 );
}

XMLTransformerOOoEventMap_Impl::~XMLTransformerOOoEventMap_Impl()
{
}

// -----------------------------------------------------------------------------

TYPEINIT1( XMLEventOOoTransformerContext, XMLPersElemContentTContext );

XMLEventOOoTransformerContext::XMLEventOOoTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_Bool bPersistent ) :
    XMLPersElemContentTContext( rImp, rQName,
        rImp.GetNamespaceMap().GetKeyByAttrName( rQName ), XML_EVENT_LISTENER ),
    m_bPersistent( bPersistent )
{
}

XMLEventOOoTransformerContext::~XMLEventOOoTransformerContext()
{
}

XMLTransformerOOoEventMap_Impl
    *XMLEventOOoTransformerContext::CreateEventMap()
{
    return new XMLTransformerOOoEventMap_Impl( aTransformerEventMap,
                                                  aFormTransformerEventMap );
}

void XMLEventOOoTransformerContext::FlushEventMap(
        XMLTransformerOOoEventMap_Impl *p )
{
    delete p;
}

sal_uInt16 XMLEventOOoTransformerContext::GetEventName(
        const OUString& rName,
        OUString& rNewName,
           XMLTransformerOOoEventMap_Impl& rMap )
{
    XMLTransformerOOoEventMap_Impl::key_type aKey( rName );
    XMLTransformerOOoEventMap_Impl::const_iterator aIter = rMap.find( aKey );
    if( aIter == rMap.end() )
    {
        rNewName = rName;
        return XML_NAMESPACE_UNKNOWN;
    }
    else
    {
        rNewName = (*aIter).second.m_aLocalName;
        return (*aIter).second.m_nPrefix;
    }
}


void XMLEventOOoTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OOO_EVENT_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    OUString aLocation, aMacroName;
    sal_Int16 nMacroName = -1;
    Reference< XAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList = 0;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                 &aLocalName );
        XMLTransformerActions::key_type aKey( nPrefix, aLocalName );
        XMLTransformerActions::const_iterator aIter =
            pActions->find( aKey );
        if( !(aIter == pActions->end() ) )
        {
            if( !pMutableAttrList )
            {
                pMutableAttrList =
                        new XMLMutableAttributeList( xAttrList );
                xAttrList = pMutableAttrList;
            }
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_HREF:
                // TODO
                break;
            case XML_ATACTION_EVENT_NAME:
                pMutableAttrList->SetValueByIndex( i,
                               GetTransformer().GetEventName( rAttrValue ) );
                break;
            case XML_ATACTION_ADD_NAMESPACE_PREFIX:
                {
                    OUString aAttrValue( rAttrValue );
                    sal_uInt16 nValPrefix =
                        static_cast<sal_uInt16>((*aIter).second.m_nParam1);
                    if( GetTransformer().AddNamespacePrefix( aAttrValue,
                                                             nValPrefix ) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_MACRO_LOCATION:
                aLocation = rAttrValue;
                pMutableAttrList->RemoveAttributeByIndex( i );
                --i;
                --nAttrCount;
                break;
            case XML_ATACTION_MACRO_NAME:
                aMacroName = rAttrValue;
                nMacroName = i;
                break;
            case XML_ATACTION_COPY:
                break;
            default:
                OSL_ENSURE( !this, "unknown action" );
                break;
            }
        }
    }

    if( nMacroName != -1 && aLocation.getLength() > 0 )
    {
        if( !IsXMLToken( aLocation, XML_APPLICATION ) )
            aLocation = GetXMLToken( XML_DOCUMENT );
        OUStringBuffer sTmp( aLocation.getLength() + aMacroName.getLength() + 1 );
        sTmp = aLocation;
        sTmp.append( sal_Unicode( ':' ) );
        sTmp.append( aMacroName );
        pMutableAttrList->SetValueByIndex( nMacroName,
                                           sTmp.makeStringAndClear() );
    }

    if( m_bPersistent )
        XMLPersElemContentTContext::StartElement( xAttrList );
    else
        GetTransformer().GetDocHandler()->startElement( GetExportQName(),
                                                        xAttrList );
}

void XMLEventOOoTransformerContext::EndElement()
{
    if( m_bPersistent )
        XMLPersElemContentTContext::EndElement();
    else
        GetTransformer().GetDocHandler()->endElement( GetExportQName() );
}

XMLTransformerContext * XMLEventOOoTransformerContext::CreateChildContext(
                            sal_uInt16 nPrefix,
                            const OUString& rLocalName,
                            const OUString& rQName,
                            const Reference< XAttributeList >& xAttrList )
{
    if( m_bPersistent )
        return XMLPersElemContentTContext::CreateChildContext(nPrefix, rLocalName, rQName, xAttrList);
    else
        return XMLTransformerContext::CreateChildContext(nPrefix, rLocalName, rQName, xAttrList);
}

sal_Bool XMLEventOOoTransformerContext::IsPersistent() const
{
    return m_bPersistent;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
