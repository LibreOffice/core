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
#include "xmlTable.hxx"
#include "xmlfilter.hxx"
#include <o3tl/safeint.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <RptDef.hxx>
#include "xmlHelper.hxx"
#include "xmlEnums.hxx"
#include "xmlColumn.hxx"
#include <com/sun/star/report/ForceNewPage.hpp>
#include "xmlCondPrtExpr.hxx"
#include <strings.hxx>
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/report/XFixedLine.hpp>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>

#include <numeric>

#define MIN_WIDTH   80
#define MIN_HEIGHT  20

namespace rptxml
{
    using namespace ::xmloff;
    using namespace ::com::sun::star;
    using ::com::sun::star::uno::Reference;
    using namespace ::com::sun::star::xml::sax;

    static sal_Int16 lcl_getForceNewPageOption(std::string_view _sValue)
    {
        sal_Int16 nRet = report::ForceNewPage::NONE;
        const SvXMLEnumMapEntry<sal_Int16>* aXML_EnumMap = OXMLHelper::GetForceNewPageOptions();
        (void)SvXMLUnitConverter::convertEnum( nRet,_sValue,aXML_EnumMap );
        return nRet;
    }

OXMLTable::OXMLTable( ORptFilter& rImport
                ,const Reference< XFastAttributeList > & _xAttrList
                ,const uno::Reference< report::XSection >& _xSection
                )
:SvXMLImportContext( rImport )
,m_xSection(_xSection)
,m_nColSpan(1)
,m_nRowSpan(0)
,m_nRowIndex(0)
,m_nColumnIndex(0)
{

    if (!m_xSection.is())
        return;
    try
    {
        for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
        {
            switch( aIter.getToken() )
            {
                case XML_ELEMENT(REPORT, XML_VISIBLE):
                    m_xSection->setVisible(IsXMLToken(aIter, XML_TRUE));
                    break;
                case XML_ELEMENT(REPORT, XML_FORCE_NEW_PAGE):
                    m_xSection->setForceNewPage(lcl_getForceNewPageOption(aIter.toView()));
                    break;
                case XML_ELEMENT(REPORT, XML_FORCE_NEW_COLUMN):
                    m_xSection->setNewRowOrCol(lcl_getForceNewPageOption(aIter.toView()));
                    break;
                case XML_ELEMENT(REPORT, XML_KEEP_TOGETHER):
                    m_xSection->setKeepTogether(IsXMLToken(aIter, XML_TRUE));
                    break;
                case XML_ELEMENT(TABLE, XML_NAME):
                    m_xSection->setName(aIter.toString());
                    break;
                case XML_ELEMENT(TABLE, XML_STYLE_NAME):
                    m_sStyleName = aIter.toString();
                    break;
                default:
                    XMLOFF_WARN_UNKNOWN("reportdesign", aIter);
                    break;
            }
        }
    }
    catch(Exception&)
    {
        TOOLS_WARN_EXCEPTION( "reportdesign", "Exception caught while filling the section props");
    }
}

OXMLTable::~OXMLTable()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > OXMLTable::createFastChildContext(
        sal_Int32 nElement,
        const Reference< XFastAttributeList > & xAttrList )
{
    css::uno::Reference< css::xml::sax::XFastContextHandler > xContext;
    ORptFilter& rImport = GetOwnImport();

    switch( nElement )
    {
        case XML_ELEMENT(TABLE, XML_TABLE_COLUMNS):
        case XML_ELEMENT(TABLE, XML_TABLE_ROWS):
            xContext = new OXMLRowColumn( rImport,xAttrList ,this);
            break;
        case XML_ELEMENT(TABLE, XML_TABLE_ROW):
            incrementRowIndex();
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            xContext = new OXMLRowColumn( rImport,xAttrList,this);
            break;
        case XML_ELEMENT(TABLE, XML_TABLE_COLUMN):
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            xContext = new OXMLRowColumn( rImport,xAttrList,this);
            break;
        case XML_ELEMENT(REPORT, XML_CONDITIONAL_PRINT_EXPRESSION):
            xContext = new OXMLCondPrtExpr( rImport,xAttrList,m_xSection);
            break;
        default:
                break;
    }

    return xContext;
}

ORptFilter& OXMLTable::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}

void OXMLTable::endFastElement(sal_Int32 )
{
    try
    {
        if ( m_xSection.is() )
        {
            if ( !m_sStyleName.isEmpty() )
            {
                const SvXMLStylesContext* pAutoStyles = GetImport().GetAutoStyles();
                if ( pAutoStyles )
                {
                    XMLPropStyleContext* pAutoStyle = const_cast<XMLPropStyleContext*>(dynamic_cast< const XMLPropStyleContext *>(pAutoStyles->FindStyleChildContext(XmlStyleFamily::TABLE_TABLE,m_sStyleName)));
                    if ( pAutoStyle )
                    {
                        pAutoStyle->FillPropertySet(m_xSection);
                    }
                }
            }
            // set height
            sal_Int32 nHeight = std::accumulate(m_aHeight.begin(), m_aHeight.end(), sal_Int32(0),
                [](const sal_Int32& rSum, const sal_Int32& rHeight) { return rSum + rHeight; });
            m_xSection->setHeight( nHeight );
            // set positions, widths, and heights
            sal_Int32 nLeftMargin = rptui::getStyleProperty<sal_Int32>(m_xSection->getReportDefinition(),PROPERTY_LEFTMARGIN);
            sal_Int32 nPosY = 0;
            ::std::vector< ::std::vector<TCell> >::iterator aRowIter = m_aGrid.begin();
            ::std::vector< ::std::vector<TCell> >::const_iterator aRowEnd = m_aGrid.end();
            for (sal_Int32 i = 0; aRowIter != aRowEnd; ++aRowIter,++i)
            {
                sal_Int32 nPosX = nLeftMargin;
                ::std::vector<TCell>::iterator aColIter = (*aRowIter).begin();
                ::std::vector<TCell>::const_iterator aColEnd = (*aRowIter).end();
                for (sal_Int32 j = 0; aColIter != aColEnd; ++aColIter,++j)
                {
                    TCell& rCell = *aColIter;
                    for (const auto& rxElement : rCell.xElements)
                    {
                        uno::Reference<report::XShape> xShape(rxElement,uno::UNO_QUERY);
                        if ( xShape.is() )
                        {
                            xShape->setPositionX(xShape->getPositionX() + nLeftMargin);
                        }
                        else
                        {
                            sal_Int32 nWidth = rCell.nWidth;
                            sal_Int32 nColSpan = rCell.nColSpan;
                            if ( nColSpan > 1 )
                            {
                                ::std::vector<TCell>::const_iterator aWidthIter = aColIter + 1;
                                while ( nColSpan > 1 )
                                {
                                    nWidth += (aWidthIter++)->nWidth;
                                    --nColSpan;
                                }
                            }
                            nHeight = rCell.nHeight;
                            sal_Int32 nRowSpan = rCell.nRowSpan;
                            if ( nRowSpan > 1 )
                            {
                                ::std::vector< ::std::vector<TCell> >::const_iterator aHeightIter = aRowIter + 1;
                                while( nRowSpan > 1)
                                {
                                    nHeight += (*aHeightIter)[j].nHeight;
                                    ++aHeightIter;
                                    --nRowSpan;
                                }
                            }
                            Reference<XFixedLine> xFixedLine(rxElement,uno::UNO_QUERY);
                            if ( xFixedLine.is() )
                            {
                                if ( xFixedLine->getOrientation() == 1 ) // vertical
                                {
                                    OSL_ENSURE(o3tl::make_unsigned(j+1) < m_aWidth.size(),"Illegal pos of col iter. There should be an empty cell for the next line part.");
                                    nWidth += m_aWidth[j+1];
                                    if ( nWidth < MIN_WIDTH )
                                        nWidth = MIN_WIDTH;
                                }
                                else if ( nHeight < MIN_HEIGHT )
                                    nHeight = MIN_HEIGHT;
                            }
                            try
                            {
                                rxElement->setSize(awt::Size(nWidth,nHeight));
                                rxElement->setPosition(awt::Point(nPosX,nPosY));
                                rxElement->setAutoGrow(rCell.bAutoHeight);
                            }
                            catch(const beans::PropertyVetoException &)
                            {
                                OSL_FAIL("Could not set the correct position or size!");
                            }
                        }
                    }
                    nPosX += m_aWidth[j];
                }
                nPosY += m_aHeight[i];
            }
        }
    }
    catch(Exception&)
    {
        TOOLS_WARN_EXCEPTION( "reportdesign", "OXMLTable::EndElement");
    }
}

void OXMLTable::addCell(const Reference<XReportComponent>& _xElement)
{
    uno::Reference<report::XShape> xShape(_xElement,uno::UNO_QUERY);
    OSL_ENSURE(o3tl::make_unsigned(m_nRowIndex-1 ) < m_aGrid.size() && o3tl::make_unsigned( m_nColumnIndex-1 ) < m_aGrid[m_nRowIndex-1].size(),
        "OXMLTable::addCell: Invalid column and row index");
    if ( o3tl::make_unsigned(m_nRowIndex-1 ) < m_aGrid.size() && o3tl::make_unsigned( m_nColumnIndex-1 ) < m_aGrid[m_nRowIndex-1].size() )
    {
        TCell& rCell = m_aGrid[m_nRowIndex-1][m_nColumnIndex-1];
        if ( _xElement.is() )
            rCell.xElements.push_back( _xElement );
        if ( !xShape.is() )
        {
            rCell.nWidth   = m_aWidth[m_nColumnIndex-1];
            rCell.nHeight  = m_aHeight[m_nRowIndex-1];
            rCell.bAutoHeight = m_aAutoHeight[m_nRowIndex-1];
            rCell.nColSpan = m_nColSpan;
            rCell.nRowSpan = m_nRowSpan;
        }
    }

    if ( !xShape.is() )
        m_nColSpan = m_nRowSpan = 1;
}

void OXMLTable::incrementRowIndex()
{
    ++m_nRowIndex;
    m_nColumnIndex = 0;
    m_aGrid.push_back(::std::vector<TCell>(m_aWidth.size()));
}

} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
