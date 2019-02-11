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
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <RptDef.hxx>
#include "xmlHelper.hxx"
#include "xmlEnums.hxx"
#include "xmlColumn.hxx"
#include <com/sun/star/report/ForceNewPage.hpp>
#include "xmlCondPrtExpr.hxx"
#include "xmlStyleImport.hxx"
#include <strings.hxx>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/report/XFixedLine.hpp>

#include <numeric>

#define MIN_WIDTH   80
#define MIN_HEIGHT  20

namespace rptxml
{
    using namespace ::xmloff;
    using namespace ::com::sun::star;
    using ::com::sun::star::uno::Reference;
    using namespace ::com::sun::star::xml::sax;
    using ::com::sun::star::xml::sax::XAttributeList;

    static sal_Int16 lcl_getForceNewPageOption(const OUString& _sValue)
    {
        sal_Int16 nRet = report::ForceNewPage::NONE;
        const SvXMLEnumMapEntry<sal_Int16>* aXML_EnumMap = OXMLHelper::GetForceNewPageOptions();
        (void)SvXMLUnitConverter::convertEnum( nRet,_sValue,aXML_EnumMap );
        return nRet;
    }

OXMLTable::OXMLTable( ORptFilter& rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const uno::Reference< report::XSection >& _xSection
                )
:SvXMLImportContext( rImport, nPrfx, _sLocalName )
,m_xSection(_xSection)
,m_nColSpan(1)
,m_nRowSpan(0)
,m_nRowIndex(0)
,m_nColumnIndex(0)
{
    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetSectionElemTokenMap();

    const sal_Int16 nLength = (m_xSection.is() && _xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    try
    {
        for(sal_Int16 i = 0; i < nLength; ++i)
        {
            OUString sLocalName;
            const OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
            const OUString sValue = _xAttrList->getValueByIndex( i );

            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_VISIBLE:
                    m_xSection->setVisible(sValue == s_sTRUE);
                    break;
                case XML_TOK_FORCE_NEW_PAGE:
                    m_xSection->setForceNewPage(lcl_getForceNewPageOption(sValue));
                    break;
                case XML_TOK_FORCE_NEW_COLUMN:
                    m_xSection->setNewRowOrCol(lcl_getForceNewPageOption(sValue));
                    break;
                case XML_TOK_KEEP_TOGETHER:
                    m_xSection->setKeepTogether(sValue == s_sTRUE);
                    break;
                case XML_TOK_SECTION_NAME:
                    m_xSection->setName(sValue);
                    break;
                case XML_TOK_SECT_STYLE_NAME:
                    m_sStyleName = sValue;
                    break;
                default:
                    break;
            }
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("Exception caught while filling the section props");
    }
}

OXMLTable::~OXMLTable()
{
}

SvXMLImportContextRef OXMLTable::CreateChildContext(
        sal_uInt16 _nPrefix,
        const OUString& _rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    ORptFilter& rImport = GetOwnImport();
    const SvXMLTokenMap&    rTokenMap   = rImport.GetColumnTokenMap();

    switch( rTokenMap.Get( _nPrefix, _rLocalName ) )
    {
        case XML_TOK_TABLE_COLUMNS:
        case XML_TOK_TABLE_ROWS:
            pContext = new OXMLRowColumn( rImport, _nPrefix, _rLocalName,xAttrList ,this);
            break;
        case XML_TOK_ROW:
            incrementRowIndex();
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLRowColumn( rImport, _nPrefix, _rLocalName,xAttrList,this);
            break;
        case XML_TOK_COLUMN:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLRowColumn( rImport, _nPrefix, _rLocalName,xAttrList,this);
            break;
        case XML_TOK_CONDITIONAL_PRINT_EXPRESSION:
            pContext = new OXMLCondPrtExpr( rImport, _nPrefix, _rLocalName,xAttrList,m_xSection.get());
            break;
        default:
                break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( rImport, _nPrefix, _rLocalName );

    return pContext;
}

ORptFilter& OXMLTable::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}

void OXMLTable::EndElement()
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
                    XMLPropStyleContext* pAutoStyle = const_cast<XMLPropStyleContext*>(dynamic_cast< const XMLPropStyleContext *>(pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_TABLE,m_sStyleName)));
                    if ( pAutoStyle )
                    {
                        pAutoStyle->FillPropertySet(m_xSection.get());
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
                                    OSL_ENSURE(static_cast<sal_uInt32>(j+1) < m_aWidth.size(),"Illegal pos of col iter. There should be an empty cell for the next line part.");
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
        OSL_FAIL("OXMLTable::EndElement -> exception caught");
    }
}

void OXMLTable::addCell(const Reference<XReportComponent>& _xElement)
{
    uno::Reference<report::XShape> xShape(_xElement,uno::UNO_QUERY);
    OSL_ENSURE(static_cast<sal_uInt32>(m_nRowIndex-1 ) < m_aGrid.size() && static_cast<sal_uInt32>( m_nColumnIndex-1 ) < m_aGrid[m_nRowIndex-1].size(),
        "OXMLTable::addCell: Invalid column and row index");
    if ( static_cast<sal_uInt32>(m_nRowIndex-1 ) < m_aGrid.size() && static_cast<sal_uInt32>( m_nColumnIndex-1 ) < m_aGrid[m_nRowIndex-1].size() )
    {
        TCell& rCell = m_aGrid[m_nRowIndex-1][m_nColumnIndex-1];
        if ( _xElement.is() )
            rCell.xElements.push_back( _xElement );
        if ( !xShape.is() )
        {
            rCell.nWidth   = m_aWidth[m_nColumnIndex-1];
            rCell.nHeight  = m_aHeight[m_nRowIndex-1];
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
