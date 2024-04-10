/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace writerfilter::rtftok
{
class RTFListener;

/// Skips a destination after a not parsed control word if it was prefixed with \*
class RTFSkipDestination final
{
public:
    explicit RTFSkipDestination(RTFListener& rImport);
    ~RTFSkipDestination();
    void setParsed(bool bParsed);
    void setReset(bool bReset);

private:
    RTFListener& m_rImport;
    bool m_bParsed;
    /// If false, the destructor is a noop, required by the \* symbol itself.
    bool m_bReset;
};
} // namespace writerfilter::rtftok

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
