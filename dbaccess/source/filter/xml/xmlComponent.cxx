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

#include "xmlComponent.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <stringconstants.hxx>
#include <strings.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/propertysequence.hxx>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;

OXMLComponent::OXMLComponent( ODBFilter& rImport
                ,const Reference< XFastAttributeList > & _xAttrList
                ,const Reference< XNameAccess >& _xParentContainer
                ,const OUString& _sComponentServiceName
                ) :
    SvXMLImportContext( rImport )
    ,m_bAsTemplate(false)
{
    static const OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    sax_fastparser::FastAttributeList *pAttribList =
                    sax_fastparser::FastAttributeList::castToFastAttributeList( _xAttrList );
    for (auto &aIter : *pAttribList)
    {
        OUString sValue = aIter.toString();

        switch( aIter.getToken() )
        {
            case XML_ELEMENT(XLINK, XML_HREF):
                m_sHREF = sValue;
                break;
            case XML_ELEMENT(DB, XML_NAME):
            case XML_ELEMENT(DB_OASIS, XML_NAME):
                m_sName = sValue;
                // sanitize the name. Previously, we allowed to create forms/reports/queries which contain
                // a / in their name, which nowadays is forbidden. To not lose such objects if they're contained
                // in older files, we replace the slash with something less offending.
                m_sName = m_sName.replace( '/', '_' );
                break;
            case XML_ELEMENT(DB, XML_AS_TEMPLATE):
            case XML_ELEMENT(DB_OASIS, XML_AS_TEMPLATE):
                m_bAsTemplate = sValue == s_sTRUE;
                break;
            default:
                SAL_WARN("dbaccess", "unknown attribute " << SvXMLImport::getNameFromToken(aIter.getToken()) << " value=" << aIter.toString());
        }
    }
    if ( !m_sHREF.isEmpty() && !m_sName.isEmpty() && _xParentContainer.is() )
    {
        Sequence<Any> aArguments(comphelper::InitAnyPropertySequence(
        {
            {PROPERTY_NAME, Any(m_sName)}, // set as folder
            {PROPERTY_PERSISTENT_NAME, Any(m_sHREF.copy(m_sHREF.lastIndexOf('/')+1))},
            {PROPERTY_AS_TEMPLATE, Any(m_bAsTemplate)},
        }));
        try
        {
            Reference< XMultiServiceFactory > xORB( _xParentContainer, UNO_QUERY_THROW );
            Reference< XInterface > xComponent( xORB->createInstanceWithArguments( _sComponentServiceName, aArguments ) );
            Reference< XNameContainer > xNameContainer( _xParentContainer, UNO_QUERY_THROW );
            xNameContainer->insertByName( m_sName, makeAny( xComponent ) );
        }
        catch(Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }
}

OXMLComponent::~OXMLComponent()
{

}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
