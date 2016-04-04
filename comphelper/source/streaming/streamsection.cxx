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

#include <comphelper/streamsection.hxx>
#include <osl/diagnose.h>

namespace comphelper
{


OStreamSection::OStreamSection(const css::uno::Reference< css::io::XDataInputStream >& _rxInput)
    :m_xMarkStream(_rxInput, css::uno::UNO_QUERY)
    ,m_xInStream(_rxInput)
    ,m_nBlockStart(-1)
    ,m_nBlockLen(-1)
{
    OSL_ENSURE(m_xInStream.is() && m_xMarkStream.is(), "OStreamSection::OStreamSection : invalid argument !");
    if (m_xInStream.is() && m_xMarkStream.is())
    {
        m_nBlockLen = _rxInput->readLong();
        m_nBlockStart = m_xMarkStream->createMark();
    }
}


OStreamSection::OStreamSection(const css::uno::Reference< css::io::XDataOutputStream >& _rxOutput)
    :m_xMarkStream(_rxOutput, css::uno::UNO_QUERY)
    ,m_xOutStream(_rxOutput)
    ,m_nBlockStart(-1)
    ,m_nBlockLen(-1)
{
    OSL_ENSURE(m_xOutStream.is() && m_xMarkStream.is(), "OStreamSection::OStreamSection : invalid argument !");
    if (m_xOutStream.is() && m_xMarkStream.is())
    {
        m_nBlockStart = m_xMarkStream->createMark();
        m_nBlockLen = 0;
        m_xOutStream->writeLong(m_nBlockLen);
    }
}


OStreamSection::~OStreamSection()
{
    try
    {   // don't allow any exceptions to leave this block, this may be called during the stack unwinding of an exception
        // handling routing
        if (m_xInStream.is() &&  m_xMarkStream.is())
        {   // we're working on an input stream
            m_xMarkStream->jumpToMark(m_nBlockStart);
            m_xInStream->skipBytes(m_nBlockLen);
            m_xMarkStream->deleteMark(m_nBlockStart);
        }
        else if (m_xOutStream.is() && m_xMarkStream.is())
        {
            sal_Int32 nRealBlockLength = m_xMarkStream->offsetToMark(m_nBlockStart) - sizeof(m_nBlockLen);
            m_nBlockLen = nRealBlockLength;
            m_xMarkStream->jumpToMark(m_nBlockStart);
            m_xOutStream->writeLong(m_nBlockLen);
            m_xMarkStream->jumpToFurthest();
            m_xMarkStream->deleteMark(m_nBlockStart);
        }
    }
    catch(const css::uno::Exception&)
    {
    }
}


}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
