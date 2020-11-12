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

#ifndef INCLUDED_SFX2_SHELLIMPL_HXX
#define INCLUDED_SFX2_SHELLIMPL_HXX

#include <cstdlib>
#include <vector>

class SfxObjectShell;
class SfxViewFrame;
class SfxViewShell;

class SfxObjectShellArr_Impl
{
    typedef std::vector<SfxObjectShell*> DataType;
    DataType maData;

public:
    typedef DataType::iterator iterator;

    iterator begin();
    iterator end();

    const SfxObjectShell* operator[](size_t i) const;
    SfxObjectShell* operator[](size_t i);

    void erase(const iterator& it);

    void push_back(SfxObjectShell* p);

    size_t size() const;
};

class SfxViewFrameArr_Impl
{
    typedef std::vector<SfxViewFrame*> DataType;
    DataType maData;

public:
    typedef DataType::iterator iterator;
    iterator begin();
    iterator end();

    const SfxViewFrame* operator[](size_t i) const;
    SfxViewFrame* operator[](size_t i);

    void erase(const iterator& it);

    void push_back(SfxViewFrame* p);

    size_t size() const;
};

class SfxViewShellArr_Impl
{
    typedef std::vector<SfxViewShell*> DataType;
    DataType maData;

public:
    typedef DataType::iterator iterator;

    iterator begin();
    iterator end();

    const SfxViewShell* operator[](size_t i) const;
    SfxViewShell* operator[](size_t i);

    void erase(const iterator& it);

    void push_back(SfxViewShell* p);

    size_t size() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
