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

#include <ctrlfactoryimpl.hxx>

const SfxMenuCtrlFactory& SfxMenuCtrlFactArr_Impl::operator []( size_t i ) const
{
    return maData[i];
}

SfxMenuCtrlFactory& SfxMenuCtrlFactArr_Impl::operator []( size_t i )
{
    return maData[i];
}

void SfxMenuCtrlFactArr_Impl::push_back( const SfxMenuCtrlFactory& p )
{
    maData.push_back(p);
}

size_t SfxMenuCtrlFactArr_Impl::size() const
{
    return maData.size();
}

const SfxStbCtrlFactory& SfxStbCtrlFactArr_Impl::operator []( size_t i ) const
{
    return maData[i];
}

SfxStbCtrlFactory& SfxStbCtrlFactArr_Impl::operator []( size_t i )
{
    return maData[i];
}

void SfxStbCtrlFactArr_Impl::push_back( SfxStbCtrlFactory* p )
{
    maData.push_back(p);
}

size_t SfxStbCtrlFactArr_Impl::size() const
{
    return maData.size();
}

const SfxTbxCtrlFactory& SfxTbxCtrlFactArr_Impl::operator []( size_t i ) const
{
    return maData[i];
}

SfxTbxCtrlFactory& SfxTbxCtrlFactArr_Impl::operator []( size_t i )
{
    return maData[i];
}

void SfxTbxCtrlFactArr_Impl::push_back( SfxTbxCtrlFactory* p )
{
    maData.push_back(p);
}

size_t SfxTbxCtrlFactArr_Impl::size() const
{
    return maData.size();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
