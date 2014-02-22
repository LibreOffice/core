/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <rtfskipdestination.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

namespace writerfilter {
namespace rtftok {

RTFSkipDestination::RTFSkipDestination(RTFListener& rImport)
    : m_rImport(rImport),
    m_bParsed(true),
    m_bReset(true)
{
}

RTFSkipDestination::~RTFSkipDestination()
{
    if (m_rImport.getSkipUnknown() && m_bReset)
    {
        if (!m_bParsed)
        {
            SAL_INFO("writerfilter", OSL_THIS_FUNC << ": skipping destination");
            m_rImport.setDestinationState(DESTINATION_SKIP);
        }
        m_rImport.setSkipUnknown(false);
    }
}

void RTFSkipDestination::setParsed(bool bParsed)
{
    m_bParsed = bParsed;
}

void RTFSkipDestination::setReset(bool bReset)
{
    m_bReset = bReset;
}

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
