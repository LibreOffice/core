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
#include "xmlComponent.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlstrings.hrc"
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLComponent)

OXMLComponent::OXMLComponent( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const Reference< XNameAccess >& _xParentContainer
                ,const ::rtl::OUString& _sComponentServiceName
                ) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_bAsTemplate(sal_False)
{
    DBG_CTOR(OXMLComponent,NULL);

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetComponentElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        ::rtl::OUString sLocalName;
        rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_HREF:
                m_sHREF = sValue;
                break;
            case XML_TOK_COMPONENT_NAME:
                m_sName = sValue;
                // sanitize the name. Previously, we allowed to create forms/reports/queries which contain
                // a / in their name, which nowadays is forbidden. To not lose such objects if they're contained
                // in older files, we replace the slash with something less offending.
                m_sName = m_sName.replace( '/', '_' );
                break;
            case XML_TOK_AS_TEMPLATE:
                m_bAsTemplate = (sValue == s_sTRUE ? sal_True : sal_False);
                break;
        }
    }
    if ( m_sHREF.getLength() && m_sName.getLength() && _xParentContainer.is() )
    {
        Sequence< Any > aArguments(3);
        PropertyValue aValue;
        // set as folder
        aValue.Name = PROPERTY_NAME;
        aValue.Value <<= m_sName;
        aArguments[0] <<= aValue;

        aValue.Name = PROPERTY_PERSISTENT_NAME;
        sal_Int32 nIndex = m_sHREF.lastIndexOf('/')+1;
        aValue.Value <<= m_sHREF.getToken(0,'/',nIndex);
        aArguments[1] <<= aValue;

        aValue.Name = PROPERTY_AS_TEMPLATE;
        aValue.Value <<= m_bAsTemplate;
        aArguments[2] <<= aValue;

        try
        {
            Reference< XMultiServiceFactory > xORB( _xParentContainer, UNO_QUERY_THROW );
            Reference< XInterface > xComponent( xORB->createInstanceWithArguments( _sComponentServiceName, aArguments ) );
            Reference< XNameContainer > xNameContainer( _xParentContainer, UNO_QUERY_THROW );
            xNameContainer->insertByName( m_sName, makeAny( xComponent ) );
        }
        catch(Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}
// -----------------------------------------------------------------------------

OXMLComponent::~OXMLComponent()
{

    DBG_DTOR(OXMLComponent,NULL);
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
