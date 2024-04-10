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

#include "TblStylePrHandler.hxx"
#include "TagLogger.hxx"
#include "CellMarginHandler.hxx"
#include "PropertyMap.hxx"
#include "MeasureHandler.hxx"
#include <ooxml/resourceids.hxx>
#include <comphelper/sequence.hxx>


using namespace css;

namespace writerfilter::dmapper {

TblStylePrHandler::TblStylePrHandler( DomainMapper & rDMapper ) :
LoggedProperties("TblStylePrHandler"),
m_rDMapper( rDMapper ),
m_pTablePropsHandler(new TablePropertiesHandler()),
m_nType( TBL_STYLE_UNKNOWN ),
m_pProperties( new PropertyMap )
{
}

TblStylePrHandler::~TblStylePrHandler( )
{
}

OUString TblStylePrHandler::getTypeString() const
{
    switch (m_nType)
    {
        case TBL_STYLE_WHOLETABLE: return "wholeTable";
        case TBL_STYLE_FIRSTROW: return "firstRow";
        case TBL_STYLE_LASTROW: return "lastRow";
        case TBL_STYLE_FIRSTCOL: return "firstCol";
        case TBL_STYLE_LASTCOL: return "lastCol";
        case TBL_STYLE_BAND1VERT: return "band1Vert";
        case TBL_STYLE_BAND2VERT: return "band2Vert";
        case TBL_STYLE_BAND1HORZ: return "band1Horz";
        case TBL_STYLE_BAND2HORZ: return "band2Horz";
        case TBL_STYLE_NECELL: return "neCell";
        case TBL_STYLE_NWCELL: return "nwCell";
        case TBL_STYLE_SECELL: return "seCell";
        case TBL_STYLE_SWCELL: return "swCell";
        default: break;
    }
    return OUString();
}

void TblStylePrHandler::lcl_attribute(Id rName, Value & rVal)
{

    switch ( rName )
    {
        case NS_ooxml::LN_CT_TblStyleOverrideType:
            {
                switch (rVal.getInt())
                {
                case NS_ooxml::LN_Value_ST_TblStyleOverrideType_wholeTable:
                    m_nType = TBL_STYLE_WHOLETABLE;
                    break;
                case NS_ooxml::LN_Value_ST_TblStyleOverrideType_firstRow:
                    m_nType = TBL_STYLE_FIRSTROW;
                    break;
                case NS_ooxml::LN_Value_ST_TblStyleOverrideType_lastRow:
                    m_nType = TBL_STYLE_LASTROW;
                    break;
                case NS_ooxml::LN_Value_ST_TblStyleOverrideType_firstCol:
                    m_nType = TBL_STYLE_FIRSTCOL;
                    break;
                case NS_ooxml::LN_Value_ST_TblStyleOverrideType_lastCol:
                    m_nType = TBL_STYLE_LASTCOL;
                    break;
                case NS_ooxml::LN_Value_ST_TblStyleOverrideType_band1Vert:
                    m_nType = TBL_STYLE_BAND1VERT;
                    break;
                case NS_ooxml::LN_Value_ST_TblStyleOverrideType_band2Vert:
                    m_nType = TBL_STYLE_BAND2VERT;
                    break;
                case NS_ooxml::LN_Value_ST_TblStyleOverrideType_band1Horz:
                    m_nType = TBL_STYLE_BAND1HORZ;
                    break;
                case NS_ooxml::LN_Value_ST_TblStyleOverrideType_band2Horz:
                    m_nType = TBL_STYLE_BAND2HORZ;
                    break;
                case NS_ooxml::LN_Value_ST_TblStyleOverrideType_neCell:
                    m_nType = TBL_STYLE_NECELL;
                    break;
                case NS_ooxml::LN_Value_ST_TblStyleOverrideType_nwCell:
                    m_nType = TBL_STYLE_NWCELL;
                    break;
                case NS_ooxml::LN_Value_ST_TblStyleOverrideType_seCell:
                    m_nType = TBL_STYLE_SECELL;
                    break;
                case NS_ooxml::LN_Value_ST_TblStyleOverrideType_swCell:
                    m_nType = TBL_STYLE_SWCELL;
                    break;
                }
            }
            break;
    }
}

void TblStylePrHandler::lcl_sprm(Sprm & rSprm)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("TblStylePrHandler.sprm");
    TagLogger::getInstance().attribute("sprm", rSprm.toString());
#endif

    switch ( rSprm.getId( ) )
    {
        case NS_ooxml::LN_CT_PPrBase:
        case NS_ooxml::LN_EG_RPrBase:
        case NS_ooxml::LN_CT_TblPrBase:
        case NS_ooxml::LN_CT_TrPrBase:
        case NS_ooxml::LN_CT_TcPrBase:
        {
            std::vector<beans::PropertyValue> aSavedGrabBag;
            bool bGrabBag = rSprm.getId() == NS_ooxml::LN_CT_PPrBase ||
                rSprm.getId() == NS_ooxml::LN_EG_RPrBase ||
                rSprm.getId() == NS_ooxml::LN_CT_TblPrBase ||
                rSprm.getId() == NS_ooxml::LN_CT_TrPrBase ||
                rSprm.getId() == NS_ooxml::LN_CT_TcPrBase;
            if (bGrabBag)
            {
                std::swap(aSavedGrabBag, m_aInteropGrabBag);
            }
            resolveSprmProps( rSprm );
            if (bGrabBag)
            {
                if (rSprm.getId() == NS_ooxml::LN_CT_PPrBase)
                    aSavedGrabBag.push_back(getInteropGrabBag("pPr"));
                else if (rSprm.getId() == NS_ooxml::LN_EG_RPrBase)
                    aSavedGrabBag.push_back(getInteropGrabBag("rPr"));
                else if (rSprm.getId() == NS_ooxml::LN_CT_TblPrBase)
                    aSavedGrabBag.push_back(getInteropGrabBag("tblPr"));
                else if (rSprm.getId() == NS_ooxml::LN_CT_TrPrBase)
                    aSavedGrabBag.push_back(getInteropGrabBag("trPr"));
                else if (rSprm.getId() == NS_ooxml::LN_CT_TcPrBase)
                    aSavedGrabBag.push_back(getInteropGrabBag("tcPr"));
                std::swap(m_aInteropGrabBag, aSavedGrabBag);
            }
        }
            break;
        case NS_ooxml::LN_CT_TrPrBase_tblHeader:
        {
            m_pProperties->Insert( PROP_HEADER_ROW_COUNT, uno::Any(sal_Int32(1)));
            beans::PropertyValue aValue;
            aValue.Name = "tblHeader";
            aValue.Value <<= true;
            m_aInteropGrabBag.push_back(aValue);
        }
            break;
        case NS_ooxml::LN_CT_TblPrBase_tblInd:
        {
            //contains unit and value
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties )
            {
                MeasureHandlerPtr pMeasureHandler( new MeasureHandler );
                pProperties->resolve(*pMeasureHandler);
                TablePropertyMapPtr pPropMap( new TablePropertyMap );
                pPropMap->setValue( TablePropertyMap::LEFT_MARGIN, pMeasureHandler->getMeasureValue() );
                m_pProperties->Insert( PROP_LEFT_MARGIN, uno::Any(pMeasureHandler->getMeasureValue()) );
            }
        }
            break;
        case NS_ooxml::LN_CT_TblPrBase_tblCellMar:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if ( pProperties )
            {
                auto pCellMarginHandler = std::make_shared<CellMarginHandler>();
                pCellMarginHandler->enableInteropGrabBag("tblCellMar");
                pProperties->resolve( *pCellMarginHandler );
                m_aInteropGrabBag.push_back(pCellMarginHandler->getInteropGrabBag());

                if( pCellMarginHandler->m_bTopMarginValid )
                    m_pProperties->Insert( META_PROP_CELL_MAR_TOP, uno::Any(pCellMarginHandler->m_nTopMargin) );
                if( pCellMarginHandler->m_bBottomMarginValid )
                    m_pProperties->Insert( META_PROP_CELL_MAR_BOTTOM, uno::Any(pCellMarginHandler->m_nBottomMargin) );
                if( pCellMarginHandler->m_bLeftMarginValid )
                    m_pProperties->Insert( META_PROP_CELL_MAR_LEFT, uno::Any(pCellMarginHandler->m_nLeftMargin) );
                if( pCellMarginHandler->m_bRightMarginValid )
                    m_pProperties->Insert( META_PROP_CELL_MAR_RIGHT, uno::Any(pCellMarginHandler->m_nRightMargin) );
            }
        }
        break;
        default:
            // Tables specific properties have to handled here
            m_pTablePropsHandler->SetProperties( m_pProperties );
            m_pTablePropsHandler->SetInteropGrabBag(m_aInteropGrabBag);
            bool bRet = m_pTablePropsHandler->sprm( rSprm );

            if ( !bRet )
            {
                // The DomainMapper can handle some of the properties
                m_rDMapper.PushStyleSheetProperties( m_pProperties, true );
                // Just pass a non-empty string, the array will have a single element anyway.
                m_rDMapper.enableInteropGrabBag("TblStylePrHandler");
                m_rDMapper.sprm( rSprm );
                uno::Sequence<beans::PropertyValue> aGrabBag = m_rDMapper.getInteropGrabBag().Value.get< uno::Sequence<beans::PropertyValue> >();
                if (aGrabBag.hasElements())
                    m_aInteropGrabBag.push_back(aGrabBag[0]);
                m_rDMapper.PopStyleSheetProperties( true );
            }
    }

#ifdef DBG_UTIL
    TagLogger::getInstance().endElement();
#endif
}

void TblStylePrHandler::resolveSprmProps(Sprm & rSprm)
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
    if( pProperties )
        pProperties->resolve(*this);
}

void TblStylePrHandler::appendInteropGrabBag(const OUString& aKey, const OUString& aValue)
{
    beans::PropertyValue aProperty;
    aProperty.Name = aKey;
    aProperty.Value <<= aValue;
    m_aInteropGrabBag.push_back(aProperty);
}

beans::PropertyValue TblStylePrHandler::getInteropGrabBag(const OUString& aName)
{
    beans::PropertyValue aRet;
    aRet.Name = aName;

    aRet.Value <<= comphelper::containerToSequence(m_aInteropGrabBag);
    return aRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
