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

#include "childwinimpl.hxx"

size_t SfxChildWinContextArr_Impl::size() const
{
    return maData.size();
}

const SfxChildWinContextFactory& SfxChildWinContextArr_Impl::operator []( size_t i ) const
{
    return *maData[i].get();
}

SfxChildWinContextFactory& SfxChildWinContextArr_Impl::operator []( size_t i )
{
    return *maData[i].get();
}

void SfxChildWinContextArr_Impl::push_back( SfxChildWinContextFactory* p )
{
    maData.push_back(std::unique_ptr<SfxChildWinContextFactory>(p));
}

size_t SfxChildWinFactArr_Impl::size() const
{
    return maData.size();
}

const SfxChildWinFactory& SfxChildWinFactArr_Impl::operator []( size_t i ) const
{
    return *maData[i].get();
}

SfxChildWinFactory& SfxChildWinFactArr_Impl::operator []( size_t i )
{
    return *maData[i].get();
}

void SfxChildWinFactArr_Impl::push_back( SfxChildWinFactory* p )
{
    maData.push_back(std::unique_ptr<SfxChildWinFactory>(p));
}

void SfxChildWinFactArr_Impl::erase( iterator it )
{
    maData.erase(it);
}

SfxChildWinFactArr_Impl::iterator SfxChildWinFactArr_Impl::begin()
{
    return maData.begin();
}

SfxFrameArr_Impl::iterator SfxFrameArr_Impl::begin()
{
    return maData.begin();
}

SfxFrameArr_Impl::iterator SfxFrameArr_Impl::end()
{
    return maData.end();
}

SfxFrame* SfxFrameArr_Impl::front()
{
    return maData.front();
}

void SfxFrameArr_Impl::erase( iterator it )
{
    maData.erase(it);
}

SfxFrame* SfxFrameArr_Impl::operator[] ( size_t i )
{
    return maData[i];
}

void SfxFrameArr_Impl::push_back( SfxFrame* p )
{
    maData.push_back(p);
}

size_t SfxFrameArr_Impl::size() const
{
    return maData.size();
}

bool SfxFrameArr_Impl::empty() const
{
    return maData.empty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
