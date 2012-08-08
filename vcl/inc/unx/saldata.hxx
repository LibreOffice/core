/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_SALDATA_HXX
#define _SV_SALDATA_HXX

#include <unx/salunx.h>
#include <vcl/salgtype.hxx>
#include <unx/salstd.hxx>
#include <salframe.hxx>
#include <unx/salinst.h>
#include <generic/gendata.hxx>
#include <osl/module.h>
#include <vclpluginapi.h>

// -=-= forwards -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SalXLib;
class SalDisplay;
class SalPrinter;

// -=-= typedefs -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#if defined LINUX || defined NETBSD || defined AIX || \
    defined FREEBSD || defined OPENBSD || defined DRAGONFLY || \
    defined ANDROID
#include <pthread.h>
#else
typedef unsigned int pthread_t;
#endif

// -=-= SalData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class VCLPLUG_GEN_PUBLIC X11SalData : public SalGenericData
{
    static int XErrorHdl( Display*, XErrorEvent* );
    static int XIOErrorHdl( Display* );

    struct XErrorStackEntry
    {
        bool            m_bIgnore;
        bool            m_bWas;
        unsigned int    m_nLastErrorRequest;
        XErrorHandler   m_aHandler;
    };
    std::vector< XErrorStackEntry > m_aXErrorHandlerStack;
    XIOErrorHandler m_aOrigXIOErrorHandler;

protected:
    SalXLib      *pXLib_;
    pthread_t     hMainThread_;

public:
             X11SalData( SalGenericDataType t, SalInstance *pInstance );
    virtual ~X11SalData();

    virtual void            Init();
    virtual void            Dispose();

    virtual void            initNWF();
    virtual void            deInitNWF();

    SalDisplay*             GetX11Display() const { return GetSalDisplay(); }
    void                    DeleteDisplay(); // for shutdown

    inline  SalXLib*        GetLib() const { return pXLib_; }
    inline  pthread_t       GetMainThread() const { return hMainThread_; }

    void                    StartTimer( sal_uLong nMS );
    inline  void            StopTimer();
    void                    Timeout() const;

    // X errors
    virtual void            ErrorTrapPush();
    virtual bool            ErrorTrapPop( bool bIgnoreError );
    void                    XError( Display *pDisp, XErrorEvent *pEvent );
    bool                    HasXErrorOccurred() const
                                { return m_aXErrorHandlerStack.back().m_bWas; }
    unsigned int            GetLastXErrorRequestCode() const
                                { return m_aXErrorHandlerStack.back().m_nLastErrorRequest; }
    void                    ResetXErrorOccurred()
                                { m_aXErrorHandlerStack.back().m_bWas = false; }
    void                    PushXErrorLevel( bool bIgnore );
    void                    PopXErrorLevel();
};

X11SalData* GetX11SalData();

#endif // _SV_SALDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
