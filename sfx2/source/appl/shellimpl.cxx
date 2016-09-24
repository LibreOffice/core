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

#include <shellimpl.hxx>

SfxObjectShellArr_Impl::iterator SfxObjectShellArr_Impl::begin()
{
    return maData.begin();
}

SfxObjectShellArr_Impl::iterator SfxObjectShellArr_Impl::end()
{
    return maData.end();
}

const SfxObjectShell* SfxObjectShellArr_Impl::operator[] ( size_t i ) const
{
    return maData[i];
}

SfxObjectShell* SfxObjectShellArr_Impl::operator[] ( size_t i )
{
    return maData[i];
}

void SfxObjectShellArr_Impl::erase( const iterator& it )
{
    maData.erase(it);
}

void SfxObjectShellArr_Impl::push_back( SfxObjectShell* p )
{
    maData.push_back(p);
}

size_t SfxObjectShellArr_Impl::size() const
{
    return maData.size();
}

SfxViewFrameArr_Impl::iterator SfxViewFrameArr_Impl::begin()
{
    return maData.begin();
}

SfxViewFrameArr_Impl::iterator SfxViewFrameArr_Impl::end()
{
    return maData.end();
}

const SfxViewFrame* SfxViewFrameArr_Impl::operator[] ( size_t i ) const
{
    return maData[i];
}

SfxViewFrame* SfxViewFrameArr_Impl::operator[] ( size_t i )
{
    return maData[i];
}

void SfxViewFrameArr_Impl::erase( const iterator& it )
{
    maData.erase(it);
}

void SfxViewFrameArr_Impl::push_back( SfxViewFrame* p )
{
    maData.push_back(p);
}

size_t SfxViewFrameArr_Impl::size() const
{
    return maData.size();
}

SfxViewShellArr_Impl::iterator SfxViewShellArr_Impl::begin()
{
    return maData.begin();
}

SfxViewShellArr_Impl::iterator SfxViewShellArr_Impl::end()
{
    return maData.end();
}

const SfxViewShell* SfxViewShellArr_Impl::operator[] ( size_t i ) const
{
    return maData[i];
}

SfxViewShell* SfxViewShellArr_Impl::operator[] ( size_t i )
{
    return maData[i];
}

void SfxViewShellArr_Impl::erase( const iterator& it )
{
    maData.erase(it);
}

void SfxViewShellArr_Impl::push_back( SfxViewShell* p )
{
    maData.push_back(p);
}

size_t SfxViewShellArr_Impl::size() const
{
    return maData.size();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
