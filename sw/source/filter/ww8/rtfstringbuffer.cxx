/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rtfstringbuffer.hxx"
#include "rtfattributeoutput.hxx"
#include "rtfexport.hxx"

RtfStringBufferValue::RtfStringBufferValue() = default;

RtfStringBufferValue::RtfStringBufferValue(const SwFlyFrameFormat* pFlyFrameFormat,
                                           const SwGrfNode* pGrfNode)
    : m_pFlyFrameFormat(pFlyFrameFormat)
    , m_pGrfNode(pGrfNode)
{
}

void RtfStringBufferValue::makeStringAndClear(RtfAttributeOutput* pAttributeOutput)
{
    if (!isGraphic())
        pAttributeOutput->m_rExport.Strm().WriteOString(m_aBuffer.makeStringAndClear());
    else
        pAttributeOutput->FlyFrameGraphic(m_pFlyFrameFormat, m_pGrfNode);
}

OString RtfStringBufferValue::makeStringAndClear() { return m_aBuffer.makeStringAndClear(); }

bool RtfStringBufferValue::isGraphic() const
{
    return m_pFlyFrameFormat != nullptr && m_pGrfNode != nullptr;
}

RtfStringBuffer::RtfStringBuffer() = default;

sal_Int32 RtfStringBuffer::getLength() const
{
    sal_Int32 nRet = 0;
    for (const auto& rValue : m_aValues)
        if (!rValue.isGraphic())
            nRet += rValue.getBuffer().getLength();
    return nRet;
}

void RtfStringBuffer::makeStringAndClear(RtfAttributeOutput* pAttributeOutput)
{
    for (auto& rValue : m_aValues)
        rValue.makeStringAndClear(pAttributeOutput);
}

OString RtfStringBuffer::makeStringAndClear()
{
    OStringBuffer aBuf;
    for (auto& rValue : m_aValues)
        if (!rValue.isGraphic())
            aBuf.append(rValue.makeStringAndClear());
    return aBuf.makeStringAndClear();
}

OStringBuffer& RtfStringBuffer::getLastBuffer()
{
    if (m_aValues.empty() || m_aValues.back().isGraphic())
        m_aValues.emplace_back();
    return m_aValues.back().getBuffer();
}

OStringBuffer* RtfStringBuffer::operator->() { return &getLastBuffer(); }

void RtfStringBuffer::clear() { m_aValues.clear(); }

void RtfStringBuffer::append(const SwFlyFrameFormat* pFlyFrameFormat, const SwGrfNode* pGrfNode)
{
    m_aValues.emplace_back(pFlyFrameFormat, pGrfNode);
}

void RtfStringBuffer::appendAndClear(RtfStringBuffer& rBuf)
{
    for (const auto& rValue : rBuf.m_aValues)
        m_aValues.push_back(rValue);
    rBuf.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
