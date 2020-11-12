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

#include <string.h>
#include <stdlib.h>
#include "hstream.hxx"

HStream::HStream()
    : pos(0)
{
}

void HStream::addData(const byte* buf, size_t aToAdd) { seq.insert(seq.end(), buf, buf + aToAdd); }

size_t HStream::readBytes(byte* buf, size_t aToRead)
{
    auto size = seq.size();
    if (aToRead >= (size - pos))
        aToRead = size - pos;
    for (size_t i = 0; i < aToRead; ++i)
        buf[i] = seq[pos++];
    return aToRead;
}

size_t HStream::skipBytes(size_t aToSkip)
{
    auto size = seq.size();
    if (aToSkip >= (size - pos))
        aToSkip = size - pos;
    pos += aToSkip;
    return aToSkip;
}

size_t HStream::available() const { return seq.size() - pos; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
