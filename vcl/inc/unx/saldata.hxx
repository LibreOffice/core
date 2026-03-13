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

#pragma once

#include <X11/Xlib.h>

#include <unx/gendata.hxx>

class SalX11Display;

class SalXLib;

class X11SalData final : public GenericUnixSalData
{
    struct XErrorStackEntry
    {
        bool            m_bIgnore;
        bool            m_bWas;
        XErrorHandler   m_aHandler;
    };

    SalX11Display* m_pDisplay;

    // cached hostname to avoid slow lookup
    OUString m_aHostname;

    // for transient storage of unicode strings eg. 'u123' by input methods
    OUString m_aUnicodeEntry;

    std::vector< XErrorStackEntry > m_aXErrorHandlerStack;
    XIOErrorHandler m_aOrigXIOErrorHandler;

    std::unique_ptr<SalXLib>  pXLib_;

public:
    X11SalData();
    virtual ~X11SalData() override;

    const OUString& GetHostname()
    {
        if (m_aHostname.isEmpty())
            osl_getLocalHostname(&m_aHostname.pData);
        return m_aHostname;
    }

    OUString& GetUnicodeCommand() { return m_aUnicodeEntry; }

    void Dispose();

    SalX11Display* GetDisplay() const { return m_pDisplay; }
    void SetDisplay(SalX11Display* pDisp) { m_pDisplay = pDisp; }
    void                    DeleteDisplay(); // for shutdown

    SalXLib*                GetLib() const { return pXLib_.get(); }

    static void             Timeout();

    // X errors
    void ErrorTrapPush();
    // true on error
    bool ErrorTrapPop(bool bIgnoreError = true);
    void                    XError( Display *pDisp, XErrorEvent *pEvent );
    bool                    HasXErrorOccurred() const
                                { return m_aXErrorHandlerStack.back().m_bWas; }
    void                    ResetXErrorOccurred()
                                { m_aXErrorHandlerStack.back().m_bWas = false; }
    void                    PushXErrorLevel( bool bIgnore );
    void                    PopXErrorLevel();
};

X11SalData* GetX11SalData();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
