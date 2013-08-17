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

#include "xmlColumn.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlstrings.hrc"
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XChild.hpp>
#include "xmlStyleImport.hxx"
#include <tools/debug.hxx>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLColumn)

OXMLColumn::OXMLColumn( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const Reference< XNameAccess >& _xParentContainer
                ,const Reference< XPropertySet >& _xTable
                ) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_xParentContainer(_xParentContainer)
    ,m_xTable(_xTable)
    ,m_bHidden(sal_False)
{
    DBG_CTOR(OXMLColumn,NULL);

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetColumnElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    OUString sType;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_COLUMN_NAME:
                m_sName = sValue;
                break;
            case XML_TOK_COLUMN_STYLE_NAME:
                m_sStyleName = sValue;
                break;
            case XML_TOK_COLUMN_HELP_MESSAGE:
                m_sHelpMessage = sValue;
                break;
            case XML_TOK_COLUMN_VISIBILITY:
                m_bHidden = sValue != "visible";
                break;
            case XML_TOK_COLUMN_TYPE_NAME:
                sType = sValue;
                OSL_ENSURE(!sType.isEmpty(),"No type name set");
                break;
            case XML_TOK_COLUMN_DEFAULT_VALUE:
                if ( !(sValue.isEmpty() || sType.isEmpty()) )
                    m_aDefaultValue <<= sValue;
                break;
            case XML_TOK_COLUMN_VISIBLE:
                m_bHidden = sValue == "false";
                break;
            case XML_TOK_DEFAULT_CELL_STYLE_NAME:
                m_sCellStyleName = sValue;
                break;
        }
    }
}

OXMLColumn::~OXMLColumn()
{

    DBG_DTOR(OXMLColumn,NULL);
}

void OXMLColumn::EndElement()
{
    Reference<XDataDescriptorFactory> xFac(m_xParentContainer,UNO_QUERY);
    if ( xFac.is() && !m_sName.isEmpty() )
    {
        Reference<XPropertySet> xProp(xFac->createDataDescriptor());
        if ( xProp.is() )
        {
            xProp->setPropertyValue(PROPERTY_NAME,makeAny(m_sName));
            xProp->setPropertyValue(PROPERTY_HIDDEN,makeAny(m_bHidden));
            if ( !m_sHelpMessage.isEmpty() )
                xProp->setPropertyValue(PROPERTY_HELPTEXT,makeAny(m_sHelpMessage));

            if ( m_aDefaultValue.hasValue() )
                xProp->setPropertyValue(PROPERTY_CONTROLDEFAULT,m_aDefaultValue);

            Reference<XAppend> xAppend(m_xParentContainer,UNO_QUERY);
            if ( xAppend.is() )
                xAppend->appendByDescriptor(xProp);
            m_xParentContainer->getByName(m_sName) >>= xProp;

            if ( !m_sStyleName.isEmpty() )
            {
                const SvXMLStylesContext* pAutoStyles = GetOwnImport().GetAutoStyles();
                if ( pAutoStyles )
                {
                    OTableStyleContext* pAutoStyle = PTR_CAST(OTableStyleContext,pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_COLUMN,m_sStyleName));
                    if ( pAutoStyle )
                    {
                        pAutoStyle->FillPropertySet(xProp);
                    }
                }
            }
            if ( !m_sCellStyleName.isEmpty() )
            {
                const SvXMLStylesContext* pAutoStyles = GetOwnImport().GetAutoStyles();
                if ( pAutoStyles )
                {
                    OTableStyleContext* pAutoStyle = PTR_CAST(OTableStyleContext,pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_CELL,m_sCellStyleName));
                    if ( pAutoStyle )
                    {
                        pAutoStyle->FillPropertySet(xProp);
                        // we also have to do this on the table to import text-properties
                        pAutoStyle->FillPropertySet(m_xTable);
                    }
                }
            }

        }
    }
    else if ( !m_sCellStyleName.isEmpty() )
    {
        const SvXMLStylesContext* pAutoStyles = GetOwnImport().GetAutoStyles();
        if ( pAutoStyles )
        {
            OTableStyleContext* pAutoStyle = PTR_CAST(OTableStyleContext,pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_CELL,m_sCellStyleName));
            if ( pAutoStyle )
            {
                // we also have to do this on the table to import text-properties
                pAutoStyle->FillPropertySet(m_xTable);
            }
        }
    }
}

ODBFilter& OXMLColumn::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
