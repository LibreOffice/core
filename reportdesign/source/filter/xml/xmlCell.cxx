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
#include "xmlCell.hxx"
#include "xmlHelper.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/report/XFixedLine.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>
#include <strings.hxx>
#include "xmlTable.hxx"
#include "xmlFormattedField.hxx"
#include "xmlImage.hxx"
#include "xmlFixedContent.hxx"
#include "xmlSubDocument.hxx"

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace uno;
    using namespace beans;
    using namespace xml::sax;


OXMLCell::OXMLCell( ORptFilter& rImport
                ,const Reference< XFastAttributeList > & _xAttrList
                ,OXMLTable* _pContainer
                ,OXMLCell* _pCell) :
    SvXMLImportContext( rImport )
    ,m_pContainer(_pContainer)
    ,m_pCell(_pCell)
    ,m_nCurrentCount(0)
    ,m_bContainsShape(false)
{
    if ( !m_pCell )
        m_pCell = this;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
    {
        switch( aIter.getToken() )
        {
            case XML_ELEMENT(TABLE, XML_STYLE_NAME):
                m_sStyleName = aIter.toString();
                break;
            case XML_ELEMENT(TABLE, XML_NUMBER_COLUMNS_SPANNED):
                m_pContainer->setColumnSpanned(aIter.toInt32());
                break;
            case XML_ELEMENT(TABLE, XML_NUMBER_ROWS_SPANNED):
                m_pContainer->setRowSpanned(aIter.toInt32());
                break;
            default:
                XMLOFF_WARN_UNKNOWN("reportdesign", aIter);
                break;
        }
    }
}

OXMLCell::~OXMLCell()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > OXMLCell::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    css::uno::Reference< css::xml::sax::XFastContextHandler > xContext;
    ORptFilter& rImport = GetOwnImport();
    Reference<XMultiServiceFactory> xFactor(rImport.GetModel(),uno::UNO_QUERY);
    static constexpr char16_t s_sStringConcat[] = u" & ";

    switch( nElement )
    {
        case XML_ELEMENT(REPORT, XML_FIXED_CONTENT):
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                xContext = new OXMLFixedContent( rImport,*m_pCell,m_pContainer);
            }
            break;
        case XML_ELEMENT(TEXT, XML_PAGE_NUMBER):
            m_sText += OUString::Concat(s_sStringConcat) + " PageNumber()";
            break;
        case XML_ELEMENT(TEXT, XML_PAGE_COUNT):
            m_sText += OUString::Concat(s_sStringConcat) + " PageCount()";
            break;
        case XML_ELEMENT(REPORT, XML_FORMATTED_TEXT):
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                uno::Reference< uno::XInterface> xInt = xFactor->createInstance(SERVICE_FORMATTEDFIELD);
                Reference< report::XFormattedField > xControl(xInt,uno::UNO_QUERY);

                OSL_ENSURE(xControl.is(),"Could not create FormattedField!");
                setComponent(xControl);
                if ( xControl.is() )
                    xContext = new OXMLFormattedField( rImport,xAttrList,xControl,m_pContainer, false);
            }
            break;
        case XML_ELEMENT(REPORT, XML_IMAGE):
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                Reference< XImageControl > xControl(xFactor->createInstance(SERVICE_IMAGECONTROL),uno::UNO_QUERY);

                OSL_ENSURE(xControl.is(),"Could not create ImageControl!");
                setComponent(xControl);
                if ( xControl.is() )
                    xContext = new OXMLImage( rImport,xAttrList,xControl,m_pContainer);
            }
            break;
        case XML_ELEMENT(REPORT, XML_SUB_DOCUMENT):
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                if ( !m_bContainsShape )
                    m_nCurrentCount = m_pContainer->getSection()->getCount();
                uno::Reference< uno::XInterface> xInt = xFactor->createInstance(SERVICE_FORMATTEDFIELD);
                Reference< report::XFormattedField > xControl(xInt,uno::UNO_QUERY);
                xContext = new OXMLSubDocument( rImport,xControl,m_pContainer, this /* give the current cell as parent*/ );
            }
            break;

        case XML_ELEMENT(TEXT, XML_P):
            xContext = new OXMLCell( rImport,xAttrList ,m_pContainer,this);
            break;

        case XML_ELEMENT(DRAW, XML_CUSTOM_SHAPE):
        case XML_ELEMENT(DRAW, XML_FRAME):
            {
                if ( !m_bContainsShape )
                    m_nCurrentCount = m_pContainer->getSection()->getCount();
                uno::Reference< drawing::XShapes > xShapes = m_pContainer->getSection();
                xContext = XMLShapeImportHelper::CreateGroupChildContext(rImport,nElement,xAttrList,xShapes);
                m_bContainsShape = true;
            }
            break;

        default:
            SAL_WARN("reportdesign", "unknown element " << SvXMLImport::getPrefixAndNameFromToken(nElement));
    }

    if ( m_xComponent.is() )
        m_pContainer->addCell(m_xComponent);

    return xContext;
}

void OXMLCell::endFastElement(sal_Int32)
{
    if ( m_bContainsShape )
    {
        const sal_Int32 nCount = m_pContainer->getSection()->getCount();
        for (sal_Int32 i = m_nCurrentCount; i < nCount; ++i)
        {
            uno::Reference<report::XShape> xShape(m_pContainer->getSection()->getByIndex(i),uno::UNO_QUERY);
            if ( xShape.is() )
                m_pContainer->addCell(xShape);
        }
    }
    if ( m_pCell != this && !m_sText.isEmpty() )
    {
        ORptFilter& rImport = GetOwnImport();
        Reference<XMultiServiceFactory> xFactor(rImport.GetModel(),uno::UNO_QUERY);
        uno::Reference< uno::XInterface> xInt = xFactor->createInstance(SERVICE_FORMATTEDFIELD);
        Reference< report::XFormattedField > xControl(xInt,uno::UNO_QUERY);
        xControl->setDataField("rpt:" + m_sText);

        OSL_ENSURE(xControl.is(),"Could not create FormattedField!");
        setComponent(xControl);
        m_xComponent = xControl.get();
        m_pContainer->getSection()->add(m_xComponent);
        m_pContainer->addCell(m_xComponent);
    }
    // check if we have a FixedLine
    else if ( !m_sStyleName.isEmpty() && !m_xComponent.is() && m_pCell == this )
    {
        ORptFilter& rImport = GetOwnImport();
        Reference<XMultiServiceFactory> xFactor(rImport.GetModel(),uno::UNO_QUERY);
        Reference<XFixedLine> xFixedLine(xFactor->createInstance(SERVICE_FIXEDLINE),uno::UNO_QUERY);
        m_xComponent = xFixedLine.get();
        m_pContainer->getSection()->add(m_xComponent);
        m_pContainer->addCell(m_xComponent);
        XMLPropStyleContext* pAutoStyle = const_cast<XMLPropStyleContext*>(dynamic_cast< const XMLPropStyleContext* >(GetImport().GetAutoStyles()->FindStyleChildContext(XmlStyleFamily::TABLE_CELL,m_sStyleName)));
        if ( pAutoStyle )
        {
            uno::Reference<beans::XPropertySet> xBorderProp = OXMLHelper::createBorderPropertySet();
            try
            {
                pAutoStyle->FillPropertySet(xBorderProp);
                table::BorderLine2 aRight,aLeft;
                xBorderProp->getPropertyValue(PROPERTY_BORDER_RIGHT) >>= aRight;
                xBorderProp->getPropertyValue(PROPERTY_BORDER_LEFT) >>= aLeft;
                const sal_Int16 rWidth = (aRight.LineWidth == 0) ? aRight.OuterLineWidth : aRight.LineWidth;
                const sal_Int16 lWidth = (aLeft.LineWidth  == 0) ? aLeft.OuterLineWidth  : aLeft.LineWidth;
                xFixedLine->setOrientation( (rWidth != 0 || lWidth != 0) ? 1 : 0);
               }
            catch(uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION( "reportdesign", "OXMLCell::EndElement -> exception caught");
            }
        }
    }
    else
        OXMLHelper::copyStyleElements(GetOwnImport().isOldFormat(),m_sStyleName,GetImport().GetAutoStyles(),m_xComponent);
}

ORptFilter& OXMLCell::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}

void OXMLCell::setComponent(const uno::Reference< report::XReportComponent >& _xComponent)
{
    m_pCell->m_xComponent = _xComponent;
    m_xComponent = _xComponent;
}

void OXMLCell::characters( const OUString& rChars )
{
    if ( !rChars.isEmpty() )
    {
        static const char s_Quote[] = "\"";
        if ( !m_sText.isEmpty() )
        {
            m_sText += " & ";
        }

        m_sText += s_Quote + rChars + s_Quote;
    }
}

void OXMLCell::setContainsShape(bool _bContainsShape)
{
    m_bContainsShape = _bContainsShape;
}


} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
