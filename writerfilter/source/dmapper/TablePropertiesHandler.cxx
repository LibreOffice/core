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

#include "BorderHandler.hxx"
#include "CellColorHandler.hxx"
#include "CellMarginHandler.hxx"
#include "ConversionHelper.hxx"
#include "MeasureHandler.hxx"
#include "TrackChangesHandler.hxx"
#include "TablePropertiesHandler.hxx"
#include "TDefTableHandler.hxx"
#include "DomainMapperTableManager.hxx"

#include <ooxml/resourceids.hxx>

#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <dmapperLoggers.hxx>
#include <ooxml/OOXMLFastTokens.hxx>


namespace writerfilter {
namespace dmapper {

    TablePropertiesHandler::TablePropertiesHandler( bool bOOXML ) :
        m_pCurrentInteropGrabBag(0),
        m_pTableManager( NULL ),
        m_bOOXML( bOOXML )
    {
    }


    TablePropertiesHandler::~TablePropertiesHandler( )
    {
        // Do not delete the table manager... this will be done somewhere else
        m_pTableManager = NULL;
    }

    bool TablePropertiesHandler::sprm(Sprm & rSprm)
    {
#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->startElement("TablePropertiesHandler.sprm");
        dmapper_logger->attribute("sprm", rSprm.toString());
#endif

        bool bRet = true;
        sal_uInt32 nSprmId = rSprm.getId();
        Value::Pointer_t pValue = rSprm.getValue();
        sal_Int32 nIntValue = ((pValue.get() != NULL) ? pValue->getInt() : 0);
        switch( nSprmId )
        {
            case NS_ooxml::LN_CT_TrPrBase_jc: //90706
            case NS_ooxml::LN_CT_TblPrBase_jc:
            case 0x5400: // sprmTJc
            {
                //table justification 0: left, 1: center, 2: right
                sal_Int16 nOrient = ConversionHelper::convertTableJustification( nIntValue );
                TablePropertyMapPtr pTableMap( new TablePropertyMap );
                pTableMap->setValue( TablePropertyMap::HORI_ORIENT, nOrient );
                insertTableProps( pTableMap );
            }
            break;
            case 0x9601: // sprmTDxaLeft
            break;
            case 0x9602: // sprmTDxaGapHalf
            {
                //m_nGapHalf = ConversionHelper::convertTwipToMM100( nIntValue );
                TablePropertyMapPtr pPropMap( new TablePropertyMap );
                pPropMap->setValue( TablePropertyMap::GAP_HALF, ConversionHelper::convertTwipToMM100( nIntValue ) );
                insertTableProps(pPropMap);
            }
            break;
            case NS_ooxml::LN_CT_TrPrBase_trHeight: //90703
            {
                //contains unit and value
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {   //contains attributes x2902 (LN_unit) and x17e2 (LN_trleft)
                    MeasureHandlerPtr pMeasureHandler( new MeasureHandler );
                    pProperties->resolve(*pMeasureHandler);
                    TablePropertyMapPtr pPropMap( new TablePropertyMap );

                    DomainMapperTableManager* pManager = dynamic_cast<DomainMapperTableManager*>(m_pTableManager);
                    // In case any of the cells has the btLr cell direction, then an explicit minimal size will just hide the whole row, don't do that.
                    const int MINLAY = 23; // sw/inc/swtypes.hxx, minimal possible size of frames.
                    if (!pManager || !pManager->HasBtlrCell() || pMeasureHandler->getMeasureValue() > ConversionHelper::convertTwipToMM100(MINLAY))
                    {
                        // In case a cell already wanted fixed size, we should not overwrite it here.
                        if (!pManager || !pManager->IsRowSizeTypeInserted())
                            pPropMap->Insert( PROP_SIZE_TYPE, uno::makeAny( pMeasureHandler->GetRowHeightSizeType() ), false);
                        else
                            pPropMap->Insert( PROP_SIZE_TYPE, uno::makeAny(text::SizeType::FIX), false);

                        pPropMap->Insert( PROP_HEIGHT, uno::makeAny(pMeasureHandler->getMeasureValue() ));
                    }
                    insertRowProps(pPropMap);
                }
            }
            break;
            case NS_ooxml::LN_CT_TrPr_ins:
            case NS_ooxml::LN_CT_TrPr_del:
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    sal_Int32 nToken = sal_Int32();
                    switch( nSprmId )
                    {
                        case NS_ooxml::LN_CT_TrPr_ins:
                            nToken = OOXML_tableRowInsert;
                            break;
                        case NS_ooxml::LN_CT_TrPr_del:
                            nToken = OOXML_tableRowDelete;
                            break;
                    };
                    TrackChangesHandlerPtr pTrackChangesHandler( new TrackChangesHandler( nToken ) );
                    pProperties->resolve(*pTrackChangesHandler);
                    TablePropertyMapPtr pPropMap( new TablePropertyMap );

                    // Add the 'track changes' properties to the 'table row' via UNO.
                    // This way - in the SW core - when it receives this - it will create a new 'Table Redline' object for that row
                    uno::Sequence<beans::PropertyValue> aTableRedlineProperties = pTrackChangesHandler->getRedlineProperties();
                    pPropMap->Insert( PROP_TABLE_REDLINE_PARAMS , uno::makeAny( aTableRedlineProperties ));
                    insertRowProps(pPropMap);
                }
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_cellIns:
            case NS_ooxml::LN_CT_TcPrBase_cellDel:
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    sal_Int32 nToken;
                    switch( nSprmId )
                    {
                        case NS_ooxml::LN_CT_TcPrBase_cellIns:
                            nToken = OOXML_tableCellInsert;
                            break;
                        case NS_ooxml::LN_CT_TcPrBase_cellDel:
                            nToken = OOXML_tableCellDelete;
                            break;
                        default:
                            throw ::com::sun::star::lang::IllegalArgumentException("illegal redline token type", NULL, 0);
                            break;
                    };
                    TrackChangesHandlerPtr pTrackChangesHandler( new TrackChangesHandler( nToken ) );
                    pProperties->resolve(*pTrackChangesHandler);
                    TablePropertyMapPtr pPropMap( new TablePropertyMap );

                    // Add the 'track changes' properties to the 'table row' via UNO.
                    // This way - in the SW core - when it receives this - it will create a new 'Table Redline' object for that row
                    uno::Sequence<beans::PropertyValue> aTableRedlineProperties = pTrackChangesHandler->getRedlineProperties();
                    pPropMap->Insert( PROP_TABLE_REDLINE_PARAMS , uno::makeAny( aTableRedlineProperties ));
                    cellProps(pPropMap);
                }
            }
            break;
            case NS_ooxml::LN_CT_TrPrBase_cantSplit:
            {
                //row can't break across pages if nIntValue == 1
                TablePropertyMapPtr pPropMap( new TablePropertyMap );
                pPropMap->Insert( PROP_IS_SPLIT_ALLOWED, uno::makeAny( nIntValue != 1 ) );
                insertRowProps(pPropMap);
            }
            break;
            case 0x9407: // sprmTDyaRowHeight
            {
                // table row height - negative values indicate 'exact height' - positive 'at least'
                TablePropertyMapPtr pPropMap( new TablePropertyMap );
                bool bMinHeight = true;
                sal_Int16 nHeight = static_cast<sal_Int16>( nIntValue );
                if( nHeight < 0 )
                {
                    bMinHeight = false;
                    nHeight *= -1;
                }
                pPropMap->Insert( PROP_SIZE_TYPE, uno::makeAny(bMinHeight ? text::SizeType::MIN : text::SizeType::FIX ));
                pPropMap->Insert( PROP_HEIGHT, uno::makeAny(ConversionHelper::convertTwipToMM100( nHeight )));
                insertRowProps(pPropMap);
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_vAlign://90694
            {
                sal_Int16 nVertOrient = text::VertOrientation::NONE;
                switch( nIntValue ) //0 - top 1 - center 3 - bottom (ST_VerticalJc)
                {
                    case 1: nVertOrient = text::VertOrientation::CENTER; break;
                    case 3: nVertOrient = text::VertOrientation::BOTTOM; break;
                    default:;
                };
                TablePropertyMapPtr pCellPropMap( new TablePropertyMap() );
                pCellPropMap->Insert( PROP_VERT_ORIENT, uno::makeAny( nVertOrient ) );
                //todo: in ooxml import the value of m_ncell is wrong
                cellProps( pCellPropMap );
                if (m_pCurrentInteropGrabBag)
                {
                    OUString aVertOrient;
                    switch( nIntValue )
                    {
                        case 0: aVertOrient = "top"; break;
                        case 1: aVertOrient = "center"; break;
                        case 2: aVertOrient = "both"; break;
                        case 3: aVertOrient = "bottom"; break;
                    };
                    if (!aVertOrient.isEmpty())
                    {
                        beans::PropertyValue aValue;
                        aValue.Name = "vAlign";
                        aValue.Value = uno::makeAny(aVertOrient);
                        m_pCurrentInteropGrabBag->push_back(aValue);
                    }
                }
            }
            break;
            case NS_ooxml::LN_CT_TblPrBase_tblBorders: //table borders, might be defined in table style
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    BorderHandlerPtr pBorderHandler( new BorderHandler(m_bOOXML) );
                    if (m_pCurrentInteropGrabBag)
                        pBorderHandler->enableInteropGrabBag("tblBorders");
                    pProperties->resolve(*pBorderHandler);
                    if (m_pCurrentInteropGrabBag)
                        m_pCurrentInteropGrabBag->push_back(pBorderHandler->getInteropGrabBag());
                    TablePropertyMapPtr pTablePropMap( new TablePropertyMap );
                    pTablePropMap->InsertProps(pBorderHandler->getProperties());

#ifdef DEBUG_DOMAINMAPPER
                    pTablePropMap->dumpXml( dmapper_logger );
#endif
                    insertTableProps( pTablePropMap );
                }
            }
            break;
            case NS_ooxml::LN_CT_TblPrBase_tblLayout:
            {
                DomainMapperTableManager* pManager = dynamic_cast<DomainMapperTableManager*>(m_pTableManager);
                if (pManager)
                    pManager->SetLayoutType(static_cast<sal_uInt32>(nIntValue));
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_tcBorders ://cell borders
            //contains CT_TcBorders_left, right, top, bottom
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    //in OOXML there's one set of borders at each cell (if there is any)
                    TDefTableHandlerPtr pTDefTableHandler( new TDefTableHandler( m_bOOXML ));
                    if (m_pCurrentInteropGrabBag)
                        pTDefTableHandler->enableInteropGrabBag("tcBorders");
                    pProperties->resolve( *pTDefTableHandler );
                    if (m_pCurrentInteropGrabBag)
                        m_pCurrentInteropGrabBag->push_back(pTDefTableHandler->getInteropGrabBag());
                    TablePropertyMapPtr pCellPropMap( new TablePropertyMap );
                    pTDefTableHandler->fillCellProperties( 0, pCellPropMap );
                    cellProps( pCellPropMap );
                }
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_tcMar:

                {
                    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                    if (pProperties.get())
                    {
                        CellMarginHandlerPtr pCellMarginHandler(new CellMarginHandler);
                        if (m_pCurrentInteropGrabBag)
                            pCellMarginHandler->enableInteropGrabBag("tcMar");
                        pProperties->resolve(*pCellMarginHandler);
                        if (m_pCurrentInteropGrabBag)
                            m_pCurrentInteropGrabBag->push_back(pCellMarginHandler->getInteropGrabBag());
                        TablePropertyMapPtr pCellProperties(new TablePropertyMap);
                        if (pCellMarginHandler->m_bTopMarginValid)
                            pCellProperties->Insert(PROP_TOP_BORDER_DISTANCE, uno::makeAny(pCellMarginHandler->m_nTopMargin));
                        if (pCellMarginHandler->m_bLeftMarginValid)
                            pCellProperties->Insert(PROP_LEFT_BORDER_DISTANCE, uno::makeAny(pCellMarginHandler->m_nLeftMargin));
                        if (pCellMarginHandler->m_bBottomMarginValid)
                            pCellProperties->Insert(PROP_BOTTOM_BORDER_DISTANCE, uno::makeAny(pCellMarginHandler->m_nBottomMargin));
                        if (pCellMarginHandler->m_bRightMarginValid)
                            pCellProperties->Insert(PROP_RIGHT_BORDER_DISTANCE, uno::makeAny(pCellMarginHandler->m_nRightMargin));
                        cellProps(pCellProperties);
                    }
                }
            break;
            case NS_ooxml::LN_CT_TblPrBase_shd:
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    CellColorHandlerPtr pCellColorHandler( new CellColorHandler);
                    pProperties->resolve( *pCellColorHandler );
                    TablePropertyMapPtr pTablePropMap( new TablePropertyMap );
                    insertTableProps( pCellColorHandler->getProperties() );
                }
            }
            break;
            case 0xd61a : // sprmTCellTopColor
            case 0xd61b : // sprmTCellLeftColor
            case 0xd61c : // sprmTCellBottomColor
            case 0xd61d : // sprmTCellRightColor
            case NS_ooxml::LN_CT_TcPrBase_shd:
            {
                // each color sprm contains as much colors as cells are in a row
                //LN_CT_TcPrBase_shd: cell shading contains: LN_CT_Shd_val, LN_CT_Shd_fill, LN_CT_Shd_color
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    CellColorHandlerPtr pCellColorHandler( new CellColorHandler );
                    pCellColorHandler->enableInteropGrabBag("shd"); //enable to store shd unsupported props in grab bag
                    pProperties->resolve( *pCellColorHandler );
                    TablePropertyMapPtr pPropertyMap = pCellColorHandler->getProperties();
                    beans::PropertyValue aGrabBag = pCellColorHandler->getInteropGrabBag();
                    if (m_pCurrentInteropGrabBag)
                        m_pCurrentInteropGrabBag->push_back(aGrabBag);
                    pPropertyMap->Insert( PROP_CELL_INTEROP_GRAB_BAG, aGrabBag.Value );
                    cellProps( pPropertyMap );
                }
            }
            break;
//OOXML table properties
            case NS_ooxml::LN_CT_TblPrBase_tblCellMar: //cell margins
            {
                //contains LN_CT_TblCellMar_top, LN_CT_TblCellMar_left, LN_CT_TblCellMar_bottom, LN_CT_TblCellMar_right
                // LN_CT_TblCellMar_start, LN_CT_TblCellMar_end
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    CellMarginHandlerPtr pCellMarginHandler( new CellMarginHandler );
                    if (m_pCurrentInteropGrabBag)
                        pCellMarginHandler->enableInteropGrabBag("tblCellMar");
                    pProperties->resolve( *pCellMarginHandler );
                    if (m_pCurrentInteropGrabBag)
                        m_pCurrentInteropGrabBag->push_back(pCellMarginHandler->getInteropGrabBag());
                    TablePropertyMapPtr pMarginProps( new TablePropertyMap );
                    if( pCellMarginHandler->m_bTopMarginValid )
                        pMarginProps->setValue( TablePropertyMap::CELL_MAR_TOP, pCellMarginHandler->m_nTopMargin );
                    if( pCellMarginHandler->m_bBottomMarginValid )
                        pMarginProps->setValue( TablePropertyMap::CELL_MAR_BOTTOM, pCellMarginHandler->m_nBottomMargin );
                    if( pCellMarginHandler->m_bLeftMarginValid )
                        pMarginProps->setValue( TablePropertyMap::CELL_MAR_LEFT, pCellMarginHandler->m_nLeftMargin );
                    if( pCellMarginHandler->m_bRightMarginValid )
                        pMarginProps->setValue( TablePropertyMap::CELL_MAR_RIGHT, pCellMarginHandler->m_nRightMargin );
                    insertTableProps(pMarginProps);
                }
            }
            break;
           case NS_ooxml::LN_CT_TblPrBase_tblInd:
           {
               writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
               if (pProperties.get())
               {
                   MeasureHandlerPtr pHandler(new MeasureHandler);
                   if (m_pCurrentInteropGrabBag)
                       pHandler->enableInteropGrabBag("tblInd");
                   pProperties->resolve( *pHandler );
                   if (m_pCurrentInteropGrabBag)
                       m_pCurrentInteropGrabBag->push_back(pHandler->getInteropGrabBag());
                   TablePropertyMapPtr pTblIndMap(new TablePropertyMap);
                   sal_uInt32 nTblInd = pHandler->getMeasureValue();
                   pTblIndMap->setValue( TablePropertyMap::LEFT_MARGIN, nTblInd);
                   insertTableProps(pTblIndMap);
               }
           }
            break;
            case NS_ooxml::LN_CT_TcPrBase_hideMark:
            if (nIntValue)
            {
                TablePropertyMapPtr pPropMap(new TablePropertyMap());
                pPropMap->Insert(PROP_CELL_HIDE_MARK, uno::makeAny(nIntValue));
                cellProps(pPropMap);
            }
            break;
            default:
            // Not handled here, give the next handler a chance.
            bRet = false;
            // However, these logically belong here, so save the value if necessary.
            switch (nSprmId)
            {
                case NS_ooxml::LN_CT_TblPrBase_tblStyleRowBandSize:
                case NS_ooxml::LN_CT_TblPrBase_tblStyleColBandSize:
                    if (m_pCurrentInteropGrabBag)
                    {
                        beans::PropertyValue aValue;
                        aValue.Name = (nSprmId == NS_ooxml::LN_CT_TblPrBase_tblStyleRowBandSize ? OUString("tblStyleRowBandSize") : OUString("tblStyleColBandSize"));
                        aValue.Value = uno::makeAny(nIntValue);
                        m_pCurrentInteropGrabBag->push_back(aValue);
                    }
                    break;
            }
            break;
        }

#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->endElement();
#endif

        return bRet;
    }

    void TablePropertiesHandler::SetInteropGrabBag(std::vector<beans::PropertyValue>& rValue)
    {
        m_pCurrentInteropGrabBag = &rValue;
    }
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
