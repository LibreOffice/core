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
#include <TrackChangesHandler.hxx>
#include <PropertyMap.hxx>
#include <doctok/resourceids.hxx>
#include <ConversionHelper.hxx>
#include <ooxml/resourceids.hxx>
#include <ooxml/OOXMLFastTokens.hxx>
#include "dmapperLoggers.hxx"
#include <filter/msfilter/util.hxx>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;


TrackChangesHandler::TrackChangesHandler( sal_Int32 nToken ) :
LoggedProperties(dmapper_logger, "TrackChangesHandler")
{
    m_pRedlineParams = RedlineParamsPtr( new RedlineParams() );
    m_pRedlineParams->m_nToken = nToken;
}


TrackChangesHandler::~TrackChangesHandler()
{
}


void TrackChangesHandler::lcl_attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    OUString sStringValue = rVal.getString();
    (void)rName;
    switch( rName )
    {
        case NS_ooxml::LN_CT_TrackChange_author:
        {
            m_pRedlineParams->m_sAuthor = sStringValue;
        }
        break;
        case NS_ooxml::LN_CT_TrackChange_date:
        {
            m_pRedlineParams->m_sDate = sStringValue;
        }
        break;
        case NS_ooxml::LN_CT_Markup_id:
        {
            m_pRedlineParams->m_nId = nIntValue;
        }
        break;
        default:
            OSL_FAIL( "unknown attribute");
    }
}

uno::Sequence<beans::PropertyValue> TrackChangesHandler::getRedlineProperties() const
{
    uno::Sequence< beans::PropertyValue > aRedlineProperties(3);
    beans::PropertyValue* pRedlineProperties = aRedlineProperties.getArray();

    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

    OUString sType;
    switch ( m_pRedlineParams->m_nToken & 0xffff )
    {
        case ooxml::OOXML_tableRowInsert:
            sType = rPropNameSupplier.GetName( PROP_TABLE_ROW_INSERT );
            break;
        case ooxml::OOXML_tableRowDelete:
            sType = rPropNameSupplier.GetName( PROP_TABLE_ROW_DELETE );
            break;
        case ooxml::OOXML_tableCellInsert:
            sType = rPropNameSupplier.GetName( PROP_TABLE_CELL_INSERT );
            break;
        case ooxml::OOXML_tableCellDelete:
            sType = rPropNameSupplier.GetName( PROP_TABLE_CELL_DELETE );
            break;
    }

    pRedlineProperties[0].Name = rPropNameSupplier.GetName( PROP_REDLINE_TYPE );
    pRedlineProperties[0].Value <<= sType;
    pRedlineProperties[1].Name = rPropNameSupplier.GetName( PROP_REDLINE_AUTHOR );
    pRedlineProperties[1].Value <<= m_pRedlineParams->m_sAuthor;
    pRedlineProperties[2].Name = rPropNameSupplier.GetName( PROP_REDLINE_DATE_TIME );
    pRedlineProperties[2].Value <<= ConversionHelper::ConvertDateStringToDateTime( m_pRedlineParams->m_sDate );
    //pRedlineProperties[3].Name = rPropNameSupplier.GetName( PROP_REDLINE_REVERT_PROPERTIES );
    //pRedlineProperties[3].Value <<= pRedline->m_aRevertProperties;

    return aRedlineProperties;
}

void TrackChangesHandler::lcl_sprm(Sprm & rSprm)
{
    (void)rSprm;
}

} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
