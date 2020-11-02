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
#include <strings.hxx>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include "xmlStyleImport.hxx"
#include <osl/diagnose.h>
#include <sal/log.hxx>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;

OXMLColumn::OXMLColumn( ODBFilter& rImport
                ,const Reference< XFastAttributeList > & _xAttrList
                ,const Reference< XNameAccess >& _xParentContainer
                ,const Reference< XPropertySet >& _xTable
                ) :
    SvXMLImportContext( rImport )
    ,m_xParentContainer(_xParentContainer)
    ,m_xTable(_xTable)
    ,m_bHidden(false)
{
    OUString sType;
    for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
    {
        OUString sValue = aIter.toString();

        switch( aIter.getToken() & TOKEN_MASK )
        {
            case XML_NAME:
                m_sName = sValue;
                break;
            case XML_STYLE_NAME:
                m_sStyleName = sValue;
                break;
            case XML_HELP_MESSAGE:
                m_sHelpMessage = sValue;
                break;
            case XML_VISIBILITY:
                m_bHidden = sValue != "visible";
                break;
            case XML_TYPE_NAME:
                sType = sValue;
                OSL_ENSURE(!sType.isEmpty(),"No type name set");
                break;
            case XML_DEFAULT_VALUE:
                if ( !(sValue.isEmpty() || sType.isEmpty()) )
                    m_aDefaultValue <<= sValue;
                break;
            case XML_VISIBLE:
                m_bHidden = sValue == "false";
                break;
            case XML_DEFAULT_CELL_STYLE_NAME:
                m_sCellStyleName = sValue;
                break;
            default:
                XMLOFF_WARN_UNKNOWN_ATTR("dbaccess", aIter.getToken(), aIter.toString());
        }
    }
}

OXMLColumn::~OXMLColumn()
{

}

void OXMLColumn::endFastElement(sal_Int32 )
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
                    OTableStyleContext* pAutoStyle = const_cast<OTableStyleContext*>(
                        dynamic_cast< const OTableStyleContext* >(pAutoStyles->FindStyleChildContext(XmlStyleFamily::TABLE_COLUMN,m_sStyleName)));
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
                    OTableStyleContext* pAutoStyle = const_cast<OTableStyleContext*>(dynamic_cast<const OTableStyleContext* >(pAutoStyles->FindStyleChildContext(XmlStyleFamily::TABLE_CELL,m_sCellStyleName)));
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
            OTableStyleContext* pAutoStyle = const_cast<OTableStyleContext*>(dynamic_cast< const OTableStyleContext* >(pAutoStyles->FindStyleChildContext(XmlStyleFamily::TABLE_CELL,m_sCellStyleName)));
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
