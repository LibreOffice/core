/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <comphelper/propertyvalue.hxx>
#include "TrackChangesHandler.hxx"
#include "PropertyMap.hxx"
#include "ConversionHelper.hxx"
#include <ooxml/resourceids.hxx>
#include <oox/token/tokens.hxx>
#include <osl/diagnose.h>

namespace writerfilter::dmapper {

using namespace ::com::sun::star;
using namespace oox;


TrackChangesHandler::TrackChangesHandler( sal_Int32 nToken ) :
    LoggedProperties("TrackChangesHandler"),
    m_pRedlineParams(new RedlineParams)
{
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
    return {
               comphelper::makePropertyValue(getPropertyName(PROP_REDLINE_TYPE ), uno::Any(sType)),
               comphelper::makePropertyValue(getPropertyName(PROP_REDLINE_AUTHOR), uno::Any(m_pRedlineParams->m_sAuthor)),
               comphelper::makePropertyValue(getPropertyName(PROP_REDLINE_DATE_TIME), uno::Any(ConversionHelper::ConvertDateStringToDateTime( m_pRedlineParams->m_sDate )))
               //comphelper::makePropertyValue(getPropertyName(PROP_REDLINE_REVERT_PROPERTIES), uno::Any(pRedline->m_aRevertProperties))
           };
}

void TrackChangesHandler::lcl_sprm(Sprm &) {}

} //namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
