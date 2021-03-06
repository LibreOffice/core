/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "rtfcontrolwords.hxx"

namespace writerfilter::rtftok
{
enum class RTFInternalState
{
    NORMAL,
    BIN,
    HEX
};

enum class RTFError
{
    OK,
    GROUP_UNDER,
    GROUP_OVER,
    UNEXPECTED_EOF,
    HEX_INVALID,
    CHAR_OVER,
    CLASSIFICATION
};

/**
 * RTFTokenizer needs a class implementing this interface. While
 * RTFTokenizer separates control words (and their arguments) from
 * text, the class implementing this interface is expected to map the
 * raw RTF tokens to dmapper tokens.
 */
class RTFListener
{
public:
    virtual ~RTFListener() = default;
    // Dispatching of control words and characters.
    virtual RTFError dispatchDestination(RTFKeyword nKeyword) = 0;
    virtual RTFError dispatchFlag(RTFKeyword nKeyword) = 0;
    virtual RTFError dispatchSymbol(RTFKeyword nKeyword) = 0;
    virtual RTFError dispatchToggle(RTFKeyword nKeyword, bool bParam, int nParam) = 0;
    virtual RTFError dispatchValue(RTFKeyword nKeyword, int nParam) = 0;
    virtual RTFError resolveChars(char ch) = 0;

    // State handling.
    virtual RTFError pushState() = 0;
    virtual RTFError popState() = 0;

    virtual Destination getDestination() = 0;
    virtual void setDestination(Destination eDestination) = 0;
    virtual RTFInternalState getInternalState() = 0;
    virtual void setInternalState(RTFInternalState nInternalState) = 0;
    virtual bool getSkipUnknown() = 0;
    virtual void setSkipUnknown(bool bSkipUnknown) = 0;

    // Substream handling.
    virtual void finishSubstream() = 0;
    virtual bool isSubstream() const = 0;
};
} // namespace writerfilter::rtftok

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
