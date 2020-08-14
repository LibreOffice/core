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
#include <xmloff/xmluconv.hxx>
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <sal/log.hxx>
#include "xmlEnums.hxx"
#include "xmlCell.hxx"
#include "xmlTable.hxx"
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <strings.hxx>

#define PROPERTY_ID_WIDTH      1
#define PROPERTY_ID_HEIGHT     2
#define PROPERTY_ID_MINHEIGHT  3

namespace rptxml
{
    using namespace ::comphelper;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xml::sax;


OXMLRowColumn::OXMLRowColumn( ORptFilter& rImport
                ,const Reference< XFastAttributeList > & _xAttrList
                ,OXMLTable* _pContainer
                ) :
    SvXMLImportContext( rImport )
    ,m_pContainer(_pContainer)
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
    {
        OUString sValue = aIter.toString();

        switch( aIter.getToken() )
        {
            case XML_ELEMENT(TABLE, XML_STYLE_NAME):
                fillStyle(sValue);
                break;
            default:
                SAL_WARN("reportdesign", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << " = " << sValue);
                break;
        }
    }
}


OXMLRowColumn::~OXMLRowColumn()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > OXMLRowColumn::createFastChildContext(
        sal_Int32 nElement,
        const Reference< XFastAttributeList > & xAttrList )
{
    css::uno::Reference< css::xml::sax::XFastContextHandler > xContext;
    ORptFilter& rImport = GetOwnImport();

    switch( nElement )
    {
        case XML_ELEMENT(TABLE, XML_TABLE_COLUMN):
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            xContext = new OXMLRowColumn( rImport,xAttrList,m_pContainer);
            break;
        case XML_ELEMENT(TABLE, XML_TABLE_ROW):
            m_pContainer->incrementRowIndex();
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            xContext = new OXMLRowColumn( rImport,xAttrList,m_pContainer);
            break;
        case XML_ELEMENT(TABLE, XML_TABLE_CELL):
            m_pContainer->incrementColumnIndex();
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            xContext = new OXMLCell( rImport,xAttrList,m_pContainer);
            break;
        case XML_ELEMENT(TABLE, XML_COVERED_TABLE_CELL):
            m_pContainer->incrementColumnIndex();
            m_pContainer->addCell(nullptr);
            break;
        default:
            break;
    }

    return xContext;
}

void OXMLRowColumn::fillStyle(const OUString& _sStyleName)
{
    if ( _sStyleName.isEmpty() )
        return;

    const SvXMLStylesContext* pAutoStyles = GetOwnImport().GetAutoStyles();
    if ( !pAutoStyles )
        return;

    PropertySetInfo* pInfo = new PropertySetInfo();
    static PropertyMapEntry const pMap[] =
    {
        {OUString(PROPERTY_WIDTH),    PROPERTY_ID_WIDTH,        ::cppu::UnoType<sal_Int32>::get()       ,PropertyAttribute::BOUND,0},
        {OUString(PROPERTY_HEIGHT),   PROPERTY_ID_HEIGHT,       ::cppu::UnoType<sal_Int32>::get()       ,PropertyAttribute::BOUND,0 },
        {OUString(PROPERTY_MINHEIGHT), PROPERTY_ID_MINHEIGHT,    ::cppu::UnoType<sal_Int32>::get()       ,PropertyAttribute::BOUND,0 },
        {OUString(), 0, css::uno::Type(), 0, 0 }
    };
    pInfo->add(pMap);
    Reference<XPropertySet> xProp = GenericPropertySet_CreateInstance(pInfo);
    XMLPropStyleContext* pAutoStyle = const_cast<XMLPropStyleContext*>(dynamic_cast< const XMLPropStyleContext*>(pAutoStyles->FindStyleChildContext(XmlStyleFamily::TABLE_COLUMN,_sStyleName)));
    if ( pAutoStyle )
    {
        pAutoStyle->FillPropertySet(xProp);
        sal_Int32 nWidth = 0;
        xProp->getPropertyValue(PROPERTY_WIDTH) >>= nWidth;
        m_pContainer->addWidth(nWidth);
    }
    else
    {
        pAutoStyle = const_cast<XMLPropStyleContext*>(dynamic_cast< const XMLPropStyleContext* >(pAutoStyles->FindStyleChildContext(XmlStyleFamily::TABLE_ROW,_sStyleName)));
        if ( pAutoStyle )
        {
            pAutoStyle->FillPropertySet(xProp);
            sal_Int32 nHeight = 0;
            sal_Int32 nMinHeight = 0;
            xProp->getPropertyValue(PROPERTY_HEIGHT) >>= nHeight;
            xProp->getPropertyValue(PROPERTY_MINHEIGHT) >>= nMinHeight;
            if (nHeight == 0 && nMinHeight > 0)
            {
                m_pContainer->addHeight(nMinHeight);
                m_pContainer->addAutoHeight(true);
            }
            else
            {
                m_pContainer->addHeight(nHeight);
                m_pContainer->addAutoHeight(false);
            }
        }
    }
}

ORptFilter& OXMLRowColumn::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}

void OXMLRowColumn::endFastElement(sal_Int32 )
{
}

} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
