/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <LatentStyleHandler.hxx>
#include <ooxml/resourceids.hxx>

namespace writerfilter
{
namespace dmapper
{

using namespace ::com::sun::star;

LatentStyleHandler::LatentStyleHandler() :
    LoggedProperties("LatentStyleHandler")
{
}

LatentStyleHandler::~LatentStyleHandler()
{
}

void LatentStyleHandler::lcl_attribute(Id rName, Value& rVal)
{
    beans::PropertyValue aValue;
    bool bFound = true;
    switch (rName)
    {
    case NS_ooxml::LN_CT_LsdException_name:
        aValue.Name = "name";
        break;
    case NS_ooxml::LN_CT_LsdException_locked:
        aValue.Name = "locked";
        break;
    case NS_ooxml::LN_CT_LsdException_uiPriority:
        aValue.Name = "uiPriority";
        break;
    case NS_ooxml::LN_CT_LsdException_semiHidden:
        aValue.Name = "semiHidden";
        break;
    case NS_ooxml::LN_CT_LsdException_unhideWhenUsed:
        aValue.Name = "unhideWhenUsed";
        break;
    case NS_ooxml::LN_CT_LsdException_qFormat:
        aValue.Name = "qFormat";
        break;
    default:
        bFound = false;
#ifdef DEBUG_WRITERFILTER
        TagLogger::getInstance().element("unhandled");
#endif
        break;
    }
    if (bFound)
    {
        aValue.Value = uno::makeAny(rVal.getString());
        m_aAttributes.push_back(aValue);
    }
}


void LatentStyleHandler::lcl_sprm(Sprm& /*rSprm*/)
{
}

const std::vector<beans::PropertyValue>& LatentStyleHandler::getAttributes() const
{
    return m_aAttributes;
}

} // namespace dmapper
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
