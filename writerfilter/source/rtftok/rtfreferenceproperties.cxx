/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtfreferenceproperties.hxx>

namespace writerfilter
{
namespace rtftok
{

RTFReferenceProperties::RTFReferenceProperties(RTFSprms aAttributes, RTFSprms aSprms)
    : m_aAttributes(aAttributes),
      m_aSprms(aSprms)
{
}

RTFReferenceProperties::RTFReferenceProperties(RTFSprms aAttributes)
    : m_aAttributes(aAttributes),
      m_aSprms()
{
}

RTFReferenceProperties::~RTFReferenceProperties() = default;

void RTFReferenceProperties::resolve(Properties& rHandler)
{
    for (auto& rAttribute : m_aAttributes)
        rHandler.attribute(rAttribute.first, *rAttribute.second.get());
    for (auto& rSprm : m_aSprms)
    {
        RTFSprm aSprm(rSprm.first, rSprm.second);
        rHandler.sprm(aSprm);
    }
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
