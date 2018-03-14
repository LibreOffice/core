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

#include "PageBordersHandler.hxx"

#include <ooxml/resourceids.hxx>

namespace writerfilter {
namespace dmapper {

PgBorder::PgBorder( ) :
    m_nDistance( 0 ),
    m_ePos( BORDER_RIGHT ),
    m_bShadow(false)
{
}

PgBorder::~PgBorder( )
{
}

PageBordersHandler::PageBordersHandler( ) :
LoggedProperties("PageBordersHandler"),
m_eBorderApply(SectionPropertyMap::BorderApply::ToAllInSection),
m_eOffsetFrom(SectionPropertyMap::BorderOffsetFrom::Text)
{
}

PageBordersHandler::~PageBordersHandler( )
{
}

void PageBordersHandler::lcl_attribute( Id eName, Value& rVal )
{
    int nIntValue = rVal.getInt( );
    switch ( eName )
    {
        case NS_ooxml::LN_CT_PageBorders_display:
        {
            switch ( nIntValue )
            {
                default:
                case NS_ooxml::LN_Value_doc_ST_PageBorderDisplay_allPages:
                    m_eBorderApply = SectionPropertyMap::BorderApply::ToAllInSection;
                    break;
                case NS_ooxml::LN_Value_doc_ST_PageBorderDisplay_firstPage:
                    m_eBorderApply = SectionPropertyMap::BorderApply::ToFirstPageInSection;
                    break;
                case NS_ooxml::LN_Value_doc_ST_PageBorderDisplay_notFirstPage:
                    m_eBorderApply = SectionPropertyMap::BorderApply::ToAllButFirstInSection;
                    break;
            }
        }
        break;
        case NS_ooxml::LN_CT_PageBorders_offsetFrom:
        {
            switch ( nIntValue )
            {
                default:
                case NS_ooxml::LN_Value_doc_ST_PageBorderOffset_page:
                    m_eOffsetFrom = SectionPropertyMap::BorderOffsetFrom::Edge;
                    break;
                case NS_ooxml::LN_Value_doc_ST_PageBorderOffset_text:
                    m_eOffsetFrom = SectionPropertyMap::BorderOffsetFrom::Text;
                    break;
            }
        }
        break;
        default:;
    }
}

void PageBordersHandler::lcl_sprm( Sprm& rSprm )
{
    switch ( rSprm.getId( ) )
    {
        case NS_ooxml::LN_CT_PageBorders_top:
        case NS_ooxml::LN_CT_PageBorders_left:
        case NS_ooxml::LN_CT_PageBorders_bottom:
        case NS_ooxml::LN_CT_PageBorders_right:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
            {
                BorderHandlerPtr pBorderHandler( new BorderHandler( true ) );
                pProperties->resolve(*pBorderHandler);
                BorderPosition ePos = BorderPosition( 0 );
                switch( rSprm.getId( ) )
                {
                    case NS_ooxml::LN_CT_PageBorders_top:
                        ePos = BORDER_TOP;
                    break;
                    case NS_ooxml::LN_CT_PageBorders_left:
                        ePos = BORDER_LEFT;
                    break;
                    case NS_ooxml::LN_CT_PageBorders_bottom:
                        ePos = BORDER_BOTTOM;
                    break;
                    case NS_ooxml::LN_CT_PageBorders_right:
                        ePos = BORDER_RIGHT;
                    break;
                    default:;
                }

                PgBorder aPgBorder;
                aPgBorder.m_rLine = pBorderHandler->getBorderLine( );
                aPgBorder.m_nDistance = pBorderHandler->getLineDistance( );
                aPgBorder.m_ePos = ePos;
                aPgBorder.m_bShadow = pBorderHandler->getShadow();
                m_aBorders.push_back( aPgBorder );
            }
        }
        break;
        default:;
    }
}

void PageBordersHandler::SetBorders( SectionPropertyMap* pSectContext )
{
    for (const PgBorder& rBorder : m_aBorders)
    {
        pSectContext->SetBorder( rBorder.m_ePos, rBorder.m_nDistance, rBorder.m_rLine, rBorder.m_bShadow );
    }
    pSectContext->SetBorderApply(m_eBorderApply);
    pSectContext->SetBorderOffsetFrom(m_eOffsetFrom);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
