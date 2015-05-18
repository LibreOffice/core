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

RTFReferenceProperties::~RTFReferenceProperties()
{
}

void RTFReferenceProperties::resolve(Properties& rHandler)
{
    for (RTFSprms::Iterator_t i = m_aAttributes.begin(); i != m_aAttributes.end(); ++i)
        rHandler.attribute(i->first, *i->second.get());
    for (RTFSprms::Iterator_t i = m_aSprms.begin(); i != m_aSprms.end(); ++i)
    {
        RTFSprm aSprm(i->first, i->second);
        rHandler.sprm(aSprm);
    }
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
