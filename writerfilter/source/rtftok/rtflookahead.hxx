/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>
#include <tools/ref.hxx>
#include "rtflistener.hxx"

class SvStream;

namespace writerfilter::rtftok
{
class RTFTokenizer;
/**
 * This acts like an importer, but used for looking ahead, e.g. to
 * determine if the current group contains a table, etc.
 */
class RTFLookahead : public RTFListener
{
public:
    RTFLookahead(SvStream& rStream, sal_uInt64 nGroupStart);
    ~RTFLookahead() override;
    RTFError dispatchDestination(RTFKeyword nKeyword) override;
    RTFError dispatchFlag(RTFKeyword nKeyword) override;
    RTFError dispatchSymbol(RTFKeyword nKeyword) override;
    RTFError dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam) override;
    RTFError dispatchValue(RTFKeyword nKeyword, int nParam) override;
    RTFError resolveChars(char ch) override;
    RTFError pushState() override;
    RTFError popState() override;
    Destination getDestination() override;
    void setDestination(Destination eDestination) override;
    RTFInternalState getInternalState() override;
    void setInternalState(RTFInternalState nInternalState) override;
    bool getSkipUnknown() override;
    void setSkipUnknown(bool bSkipUnknown) override;
    void finishSubstream() override;
    bool isSubstream() const override;
    bool hasTable() const { return m_bHasTable; }
    bool hasColumns() const { return m_bHasColumns; }

private:
    tools::SvRef<RTFTokenizer> m_pTokenizer;
    SvStream& m_rStream;
    bool m_bHasTable;
    bool m_bHasColumns;
};
} // namespace writerfilter::rtftok

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
