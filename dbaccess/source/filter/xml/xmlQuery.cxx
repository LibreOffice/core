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
#include "xmlQuery.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlstrings.hrc"
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <tools/debug.hxx>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;

DBG_NAME(OXMLQuery)

OXMLQuery::OXMLQuery( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xParentContainer
                ) :
    OXMLTable( rImport, nPrfx, _sLocalName,_xAttrList,_xParentContainer,SERVICE_SDB_COMMAND_DEFINITION )
        ,m_bEscapeProcessing(sal_True)
{
    DBG_CTOR(OXMLQuery,NULL);

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetQueryElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        ::rtl::OUString sLocalName;
        rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_COMMAND:
                m_sCommand = sValue;
                break;
            case XML_TOK_ESCAPE_PROCESSING:
                m_bEscapeProcessing = sValue.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("true"));
                break;
        }
    }
}
// -----------------------------------------------------------------------------

OXMLQuery::~OXMLQuery()
{

    DBG_DTOR(OXMLQuery,NULL);
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLQuery::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext* pContext = OXMLTable::CreateChildContext(nPrefix, rLocalName,xAttrList );
    if ( !pContext )
    {
        const SvXMLTokenMap& rTokenMap = GetOwnImport().GetQueryElemTokenMap();

        switch( rTokenMap.Get( nPrefix, rLocalName ) )
        {
            case XML_TOK_UPDATE_TABLE:
                {
                    GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                    ::rtl::OUString s1;
                    fillAttributes(nPrefix, rLocalName,xAttrList,s1,m_sTable,m_sSchema,m_sCatalog);
                }
                break;
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
void OXMLQuery::setProperties(Reference< XPropertySet > & _xProp )
{
    try
    {
        if ( _xProp.is() )
        {
            OXMLTable::setProperties(_xProp);

            _xProp->setPropertyValue(PROPERTY_COMMAND,makeAny(m_sCommand));
            _xProp->setPropertyValue(PROPERTY_ESCAPE_PROCESSING,makeAny(m_bEscapeProcessing));

            if ( m_sTable.getLength() )
                _xProp->setPropertyValue(PROPERTY_UPDATE_TABLENAME,makeAny(m_sTable));
            if ( m_sCatalog.getLength() )
                _xProp->setPropertyValue(PROPERTY_UPDATE_CATALOGNAME,makeAny(m_sCatalog));
            if ( m_sSchema.getLength() )
                _xProp->setPropertyValue(PROPERTY_UPDATE_SCHEMANAME,makeAny(m_sSchema));

            const ODBFilter::TPropertyNameMap& rSettings = GetOwnImport().getQuerySettings();
            ODBFilter::TPropertyNameMap::const_iterator aFind = rSettings.find(m_sName);
            if ( aFind != rSettings.end() )
                _xProp->setPropertyValue(PROPERTY_LAYOUTINFORMATION,makeAny(aFind->second));
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("OXMLTable::EndElement -> exception catched");
    }
}
//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
