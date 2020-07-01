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

#ifndef INCLUDED_VCL_WINDOWSTATE_HXX
#define INCLUDED_VCL_WINDOWSTATE_HXX

#include <vcl/dllapi.h>
#include <vcl/vclenum.hxx>
#include <rtl/string.hxx>

class VCL_PLUGIN_PUBLIC WindowStateData
{
private:
    WindowStateMask mnValidMask;
    int mnX;
    int mnY;
    unsigned int mnWidth;
    unsigned int mnHeight;
    int mnMaximizedX;
    int mnMaximizedY;
    unsigned int mnMaximizedWidth;
    unsigned int mnMaximizedHeight;
    WindowStateState mnState;

public:
    WindowStateData()
        : mnValidMask(WindowStateMask::NONE)
        , mnX(0)
        , mnY(0)
        , mnWidth(0)
        , mnHeight(0)
        , mnMaximizedX(0)
        , mnMaximizedY(0)
        , mnMaximizedWidth(0)
        , mnMaximizedHeight(0)
        , mnState(WindowStateState::NONE)
    {
    }

    void SetMask(WindowStateMask nValidMask) { mnValidMask = nValidMask; }
    WindowStateMask GetMask() const { return mnValidMask; }

    void SetX(int nX) { mnX = nX; }
    int GetX() const { return mnX; }
    void SetY(int nY) { mnY = nY; }
    int GetY() const { return mnY; }
    void SetWidth(unsigned int nWidth) { mnWidth = nWidth; }
    unsigned int GetWidth() const { return mnWidth; }
    void SetHeight(unsigned int nHeight) { mnHeight = nHeight; }
    unsigned int GetHeight() const { return mnHeight; }
    void SetState(WindowStateState nState) { mnState = nState; }
    WindowStateState GetState() const { return mnState; }
    void SetMaximizedX(int nRX) { mnMaximizedX = nRX; }
    int GetMaximizedX() const { return mnMaximizedX; }
    void SetMaximizedY(int nRY) { mnMaximizedY = nRY; }
    int GetMaximizedY() const { return mnMaximizedY; }
    void SetMaximizedWidth(unsigned int nRWidth) { mnMaximizedWidth = nRWidth; }
    unsigned int GetMaximizedWidth() const { return mnMaximizedWidth; }
    void SetMaximizedHeight(unsigned int nRHeight) { mnMaximizedHeight = nRHeight; }
    unsigned int GetMaximizedHeight() const { return mnMaximizedHeight; }

    OString ToStr() const;
};

#endif // INCLUDED_VCL_WINDOWSTATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
