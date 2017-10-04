/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rtflookahead.hxx"
#include <tools/stream.hxx>

using namespace com::sun::star;

namespace writerfilter
{
namespace rtftok
{

RTFLookahead::RTFLookahead(SvStream& rStream, sal_uInt64 nGroupStart)
    : m_rStream(rStream),
      m_bHasTable(false)
{
    sal_uInt64 const nPos = m_rStream.Tell();
    m_rStream.Seek(nGroupStart);
    uno::Reference<task::XStatusIndicator> xStatusIndicator;
    m_pTokenizer.reset(new RTFTokenizer(*this, &m_rStream, xStatusIndicator));
    m_pTokenizer->resolveParse();
    m_rStream.Seek(nPos);
}

RTFLookahead::~RTFLookahead() = default;

RTFError RTFLookahead::dispatchDestination(RTFKeyword /*nKeyword*/)
{
    return RTFError::OK;
}

RTFError RTFLookahead::dispatchFlag(RTFKeyword nKeyword)
{
    if (nKeyword == RTF_INTBL)
        m_bHasTable = true;
    return RTFError::OK;
}

RTFError RTFLookahead::dispatchSymbol(RTFKeyword /*nKeyword*/)
{
    return RTFError::OK;
}

RTFError RTFLookahead::dispatchToggle(RTFKeyword /*nKeyword*/, bool /*bParam*/, int /*nParam*/)
{
    return RTFError::OK;
}

RTFError RTFLookahead::dispatchValue(RTFKeyword /*nKeyword*/, int /*nParam*/)
{
    return RTFError::OK;
}

RTFError RTFLookahead::resolveChars(char ch)
{
    while (!m_rStream.IsEof() && (ch != '{' && ch != '}' && ch != '\\'))
        m_rStream.ReadChar(ch);
    if (!m_rStream.IsEof())
        m_rStream.SeekRel(-1);
    return RTFError::OK;
}

RTFError RTFLookahead::pushState()
{
    m_pTokenizer->pushGroup();
    return RTFError::OK;
}

RTFError RTFLookahead::popState()
{
    m_pTokenizer->popGroup();
    return RTFError::OK;
}

Destination RTFLookahead::getDestination()
{
    return Destination::NORMAL;
}

void RTFLookahead::setDestination(Destination /*eDestination*/)
{
}

RTFInternalState RTFLookahead::getInternalState()
{
    return RTFInternalState::NORMAL;
}

void RTFLookahead::setInternalState(RTFInternalState /*nInternalState*/)
{
}

bool RTFLookahead::getSkipUnknown()
{
    return false;
}

void RTFLookahead::setSkipUnknown(bool /*bSkipUnknown*/)
{
}

void RTFLookahead::finishSubstream()
{
}

bool RTFLookahead::isSubstream() const
{
    return false;
}


} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
