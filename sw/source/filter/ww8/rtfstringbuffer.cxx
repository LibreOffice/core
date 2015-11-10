/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rtfstringbuffer.hxx"
#include "rtfattributeoutput.hxx"

RtfStringBufferValue::RtfStringBufferValue()
    : m_aBuffer(),
      m_pFlyFrameFormat(nullptr),
      m_pGrfNode(nullptr)
{
}

RtfStringBufferValue::RtfStringBufferValue(const SwFlyFrameFormat* pFlyFrameFormat, const SwGrfNode* pGrfNode)
    : m_aBuffer(),
      m_pFlyFrameFormat(pFlyFrameFormat),
      m_pGrfNode(pGrfNode)
{
}

void RtfStringBufferValue::makeStringAndClear(RtfAttributeOutput* pAttributeOutput)
{
    if (!isGraphic())
        pAttributeOutput->m_rExport.Strm().WriteCharPtr(m_aBuffer.makeStringAndClear().getStr());
    else
        pAttributeOutput->FlyFrameGraphic(m_pFlyFrameFormat, m_pGrfNode);
}

OString RtfStringBufferValue::makeStringAndClear()
{
    return m_aBuffer.makeStringAndClear();
}

bool RtfStringBufferValue::isGraphic() const
{
    return m_pFlyFrameFormat != nullptr && m_pGrfNode != nullptr;
}

RtfStringBuffer::RtfStringBuffer()
    : m_aValues()
{
}

sal_Int32 RtfStringBuffer::getLength() const
{
    sal_Int32 nRet = 0;
    for (RtfStringBuffer::Values_t::const_iterator i = m_aValues.begin(); i != m_aValues.end(); ++i)
        if (!i->isGraphic())
            nRet += i->m_aBuffer.getLength();
    return nRet;
}

void RtfStringBuffer::makeStringAndClear(RtfAttributeOutput* pAttributeOutput)
{
    for (RtfStringBuffer::Values_t::iterator i = m_aValues.begin(); i != m_aValues.end(); ++i)
        i->makeStringAndClear(pAttributeOutput);
}

OString RtfStringBuffer::makeStringAndClear()
{
    OStringBuffer aBuf;
    for (RtfStringBuffer::Values_t::iterator i = m_aValues.begin(); i != m_aValues.end(); ++i)
        if (!i->isGraphic())
            aBuf.append(i->makeStringAndClear());
    return aBuf.makeStringAndClear();
}

OStringBuffer& RtfStringBuffer::getLastBuffer()
{
    if (m_aValues.empty() || m_aValues.back().isGraphic())
        m_aValues.push_back(RtfStringBufferValue());
    return m_aValues.back().m_aBuffer;
}

OStringBuffer* RtfStringBuffer::operator->()
{
    return &getLastBuffer();
}

void RtfStringBuffer::clear()
{
    m_aValues.clear();
}

void RtfStringBuffer::append(const SwFlyFrameFormat* pFlyFrameFormat, const SwGrfNode* pGrfNode)
{
    m_aValues.push_back(RtfStringBufferValue(pFlyFrameFormat, pGrfNode));
}

void RtfStringBuffer::appendAndClear(RtfStringBuffer& rBuf)
{
    for (RtfStringBuffer::Values_t::iterator i = rBuf.m_aValues.begin(); i != rBuf.m_aValues.end(); ++i)
        m_aValues.push_back(*i);
    rBuf.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
