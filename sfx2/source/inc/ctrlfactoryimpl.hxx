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

#ifndef INCLUDED_SFX2_CTRLFACTORYIMPL_HXX
#define INCLUDED_SFX2_CTRLFACTORYIMPL_HXX

#include <sfx2/mnuitem.hxx>
#include <sfx2/stbitem.hxx>
#include <sfx2/tbxctrl.hxx>

#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>

class SfxMenuCtrlFactArr_Impl
{
    typedef std::vector<SfxMenuCtrlFactory> DataType;
    DataType maData;

public:
    const SfxMenuCtrlFactory& operator []( size_t i ) const;
    SfxMenuCtrlFactory& operator []( size_t i );

    void push_back( const SfxMenuCtrlFactory& );

    size_t size() const;
};

class SfxStbCtrlFactArr_Impl
{
    typedef boost::ptr_vector<SfxStbCtrlFactory> DataType;
    DataType maData;

public:
    const SfxStbCtrlFactory& operator []( size_t i ) const;
    SfxStbCtrlFactory& operator []( size_t i );

    void push_back( SfxStbCtrlFactory* p );

    size_t size() const;
};

class SfxTbxCtrlFactArr_Impl
{
    typedef boost::ptr_vector<SfxTbxCtrlFactory> DataType;
    DataType maData;

public:
    const SfxTbxCtrlFactory& operator []( size_t i ) const;
    SfxTbxCtrlFactory& operator []( size_t i );

    void push_back( SfxTbxCtrlFactory* p );

    size_t size() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
