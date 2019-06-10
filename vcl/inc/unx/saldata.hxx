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

#ifndef INCLUDED_VCL_INC_UNX_SALDATA_HXX
#define INCLUDED_VCL_INC_UNX_SALDATA_HXX

#include <X11/Xlib.h>

#include <unx/saldisp.hxx>
#include <unx/gendata.hxx>
#include <vclpluginapi.h>

class SalXLib;
class SalDisplay;
class SalPrinter;

class VCLPLUG_GEN_PUBLIC X11SalData : public GenericUnixSalData
{
    struct XErrorStackEntry
    {
        bool            m_bIgnore;
        bool            m_bWas;
        XErrorHandler   m_aHandler;
    };
    std::vector< XErrorStackEntry > m_aXErrorHandlerStack;
    XIOErrorHandler m_aOrigXIOErrorHandler;

protected:
    std::unique_ptr<SalXLib>  pXLib_;

public:
             X11SalData( GenericUnixSalDataType t, SalInstance *pInstance );
    virtual ~X11SalData() override;

    virtual void            Init();
    virtual void            Dispose() override;

    virtual void            initNWF();
    virtual void            deInitNWF();

    void                    DeleteDisplay(); // for shutdown

    SalXLib*                GetLib() const { return pXLib_.get(); }

    static void             Timeout();

    // X errors
    virtual void            ErrorTrapPush() override;
    virtual bool            ErrorTrapPop( bool bIgnoreError = true ) override;
    void                    XError( Display *pDisp, XErrorEvent *pEvent );
    bool                    HasXErrorOccurred() const
                                { return m_aXErrorHandlerStack.back().m_bWas; }
    void                    ResetXErrorOccurred()
                                { m_aXErrorHandlerStack.back().m_bWas = false; }
    void                    PushXErrorLevel( bool bIgnore );
    void                    PopXErrorLevel();
};

X11SalData* GetX11SalData();

#endif // INCLUDED_VCL_INC_UNX_SALDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
