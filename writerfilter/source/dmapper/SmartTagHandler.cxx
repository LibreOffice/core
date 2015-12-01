/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SmartTagHandler.hxx>
#include <ooxml/resourceids.hxx>

namespace writerfilter
{
namespace dmapper
{

using namespace ::com::sun::star;

SmartTagHandler::SmartTagHandler()
    : LoggedProperties("SmartTagHandler")
{
}

SmartTagHandler::~SmartTagHandler()
{
}

void SmartTagHandler::lcl_attribute(Id nName, Value& rValue)
{
    switch (nName)
    {
    case NS_ooxml::LN_CT_Attr_name:
        break;
    case NS_ooxml::LN_CT_Attr_val:
        break;
    default:
        SAL_WARN("writerfilter", "SmartTagHandler::lcl_attribute: unhandled attribute " << nName << " (string value: '"<<rValue.getString()<<"')");
        break;
    }
}

void SmartTagHandler::lcl_sprm(Sprm& rSprm)
{
    switch (rSprm.getId())
    {
    case NS_ooxml::LN_CT_SmartTagPr_attr:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties.get())
            pProperties->resolve(*this);
        break;
    }
    }
}

void SmartTagHandler::setURI(const OUString& rURI)
{
    m_aURI = rURI;
}

void SmartTagHandler::setElement(const OUString& rElement)
{
    m_aElement = rElement;
}

} // namespace dmapper
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
