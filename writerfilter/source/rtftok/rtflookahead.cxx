/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <boost/shared_ptr.hpp>

#include <com/sun/star/task/XStatusIndicator.hpp>
#include <tools/stream.hxx>

#include <rtflookahead.hxx>

using namespace com::sun::star;

namespace writerfilter {
namespace rtftok {

RTFLookahead::RTFLookahead(SvStream& rStream, sal_Size nGroupStart)
    : m_rStream(rStream),
    m_bHasTable(false)
{
    sal_Size nPos = m_rStream.Tell();
    m_rStream.Seek(nGroupStart);
    uno::Reference<task::XStatusIndicator> xStatusIndicator;
    m_pTokenizer.reset(new RTFTokenizer(*this, &m_rStream, xStatusIndicator));
    m_pTokenizer->resolveParse();
    m_rStream.Seek(nPos);
}

RTFLookahead::~RTFLookahead()
{
}

int RTFLookahead::dispatchDestination(RTFKeyword /*nKeyword*/)
{
    return 0;
}

int RTFLookahead::dispatchFlag(RTFKeyword nKeyword)
{
    if (nKeyword == RTF_INTBL)
        m_bHasTable = true;
    return 0;
}

int RTFLookahead::dispatchSymbol(RTFKeyword /*nKeyword*/)
{
    return 0;
}

int RTFLookahead::dispatchToggle(RTFKeyword /*nKeyword*/, bool /*bParam*/, int /*nParam*/)
{
    return 0;
}

int RTFLookahead::dispatchValue(RTFKeyword /*nKeyword*/, int /*nParam*/)
{
    return 0;
}

int RTFLookahead::resolveChars(char ch)
{
    while(!m_rStream.IsEof() && (ch != '{' && ch != '}' && ch != '\\'))
        m_rStream >> ch;
    if (!m_rStream.IsEof())
        m_rStream.SeekRel(-1);
    return 0;
}

int RTFLookahead::pushState()
{
    m_pTokenizer->pushGroup();
    return 0;
}

int RTFLookahead::popState()
{
    m_pTokenizer->popGroup();
    return 0;
}

RTFDestinationState RTFLookahead::getDestinationState()
{
    return DESTINATION_NORMAL;
}

void RTFLookahead::setDestinationState(RTFDestinationState /*nDestinationState*/)
{
}

RTFInternalState RTFLookahead::getInternalState()
{
    return INTERNAL_NORMAL;
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

bool RTFLookahead::hasTable()
{
    return m_bHasTable;
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
