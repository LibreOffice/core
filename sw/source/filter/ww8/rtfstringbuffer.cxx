/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@suse.cz> (SUSE, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2012 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "rtfattributeoutput.hxx"
#include "rtfstringbuffer.hxx"

RtfStringBufferValue::RtfStringBufferValue()
    : m_aBuffer(),
    m_pFlyFrmFmt(0),
    m_pGrfNode(0)
{
}

RtfStringBufferValue::RtfStringBufferValue(const SwFlyFrmFmt* pFlyFrmFmt, const SwGrfNode* pGrfNode)
    : m_aBuffer(),
    m_pFlyFrmFmt(pFlyFrmFmt),
    m_pGrfNode(pGrfNode)
{
}

void RtfStringBufferValue::makeStringAndClear(RtfAttributeOutput* pAttributeOutput)
{
    if (!isGraphic())
        pAttributeOutput->m_rExport.Strm() << m_aBuffer.makeStringAndClear().getStr();
    else
        pAttributeOutput->FlyFrameGraphic(m_pFlyFrmFmt, m_pGrfNode);
}

OString RtfStringBufferValue::makeStringAndClear()
{
    return m_aBuffer.makeStringAndClear();
}

bool RtfStringBufferValue::isGraphic() const
{
    return m_pFlyFrmFmt != 0 && m_pGrfNode != 0;
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

void RtfStringBuffer::append(const SwFlyFrmFmt* pFlyFrmFmt, const SwGrfNode* pGrfNode)
{
    m_aValues.push_back(RtfStringBufferValue(pFlyFrmFmt, pGrfNode));
}

void RtfStringBuffer::appendAndClear(RtfStringBuffer& rBuf)
{
    for (RtfStringBuffer::Values_t::iterator i = rBuf.m_aValues.begin(); i != rBuf.m_aValues.end(); ++i)
        m_aValues.push_back(*i);
    rBuf.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
