/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "TrackChangesHandler.hxx"
#include "PropertyMap.hxx"
#include "ConversionHelper.hxx"
#include <ooxml/resourceids.hxx>
#include <oox/token/tokens.hxx>
#include <osl/diagnose.h>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
using namespace oox;


TrackChangesHandler::TrackChangesHandler( sal_Int32 nToken ) :
LoggedProperties("TrackChangesHandler")
{
    m_pRedlineParams = std::make_shared<RedlineParams>( );
    m_pRedlineParams->m_nToken = nToken;
}


TrackChangesHandler::~TrackChangesHandler()
{
}


void TrackChangesHandler::lcl_attribute(Id rName, Value & rVal)
{
    OUString sStringValue = rVal.getString();
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
        break;
        default:
            OSL_FAIL( "unknown attribute");
    }
}

uno::Sequence<beans::PropertyValue> TrackChangesHandler::getRedlineProperties() const
{
    uno::Sequence< beans::PropertyValue > aRedlineProperties(3);
    beans::PropertyValue* pRedlineProperties = aRedlineProperties.getArray();

    OUString sType;
    switch ( m_pRedlineParams->m_nToken & 0xffff )
    {
        case XML_tableRowInsert:
            sType = getPropertyName( PROP_TABLE_ROW_INSERT );
            break;
        case XML_tableRowDelete:
            sType = getPropertyName( PROP_TABLE_ROW_DELETE );
            break;
        case XML_tableCellInsert:
            sType = getPropertyName( PROP_TABLE_CELL_INSERT );
            break;
        case XML_tableCellDelete:
            sType = getPropertyName( PROP_TABLE_CELL_DELETE );
            break;
    }

    pRedlineProperties[0].Name = getPropertyName( PROP_REDLINE_TYPE );
    pRedlineProperties[0].Value <<= sType;
    pRedlineProperties[1].Name = getPropertyName( PROP_REDLINE_AUTHOR );
    pRedlineProperties[1].Value <<= m_pRedlineParams->m_sAuthor;
    pRedlineProperties[2].Name = getPropertyName( PROP_REDLINE_DATE_TIME );
    pRedlineProperties[2].Value <<= ConversionHelper::ConvertDateStringToDateTime( m_pRedlineParams->m_sDate );
    //pRedlineProperties[3].Name = getPropertyName( PROP_REDLINE_REVERT_PROPERTIES );
    //pRedlineProperties[3].Value <<= pRedline->m_aRevertProperties;

    return aRedlineProperties;
}

void TrackChangesHandler::lcl_sprm(Sprm &) {}

} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
