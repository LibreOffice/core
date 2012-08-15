/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <TablePositionHandler.hxx>
#include <PropertyMap.hxx>
#include <doctok/resourceids.hxx>
#include <ConversionHelper.hxx>
#include <ooxml/resourceids.hxx>
#include <com/sun/star/text/SizeType.hpp>
#include "dmapperLoggers.hxx"

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;

TablePositionHandler::TablePositionHandler() :
LoggedProperties(dmapper_logger, "TablePositionHandler")
{
}

TablePositionHandler::~TablePositionHandler()
{
}


void TablePositionHandler::lcl_attribute(Id rName, Value& rVal)
{
    switch (rName)
    {
        case NS_ooxml::LN_CT_TblPPr_vertAnchor:
            m_aVertAnchor = rVal.getString();
        break;
        default:
#ifdef DEBUG_DOMAINMAPPER
            dmapper_logger->element("unhandled");
#endif
            break;
    }
}


void TablePositionHandler::lcl_sprm(Sprm& /*rSprm*/)
{
}


OUString TablePositionHandler::getVertAnchor() const
{
    return m_aVertAnchor;
}

} // namespace dmapper
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
