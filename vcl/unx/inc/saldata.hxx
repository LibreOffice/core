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

// -=-= includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <signal.h>
#include <salstd.hxx>
#include <vcl/salframe.hxx>
#include <salinst.h>
#include <vcl/saldatabasic.hxx>
#ifndef _OSL_MODULE_H
#include <osl/module.h>
#endif
#include <vcl/dllapi.h>

// -=-= forwards -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SalXLib;
class SalDisplay;
class SalPrinter;

// -=-= typedefs -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

DECLARE_LIST( SalDisplays, SalDisplay* )

#if defined SCO || defined LINUX || defined NETBSD || defined AIX || defined HPUX || defined FREEBSD
#include <pthread.h>
#else
typedef unsigned int pthread_t;
#endif

// -=-= SalData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class VCL_DLLPUBLIC X11SalData : public SalData
{
protected:
            sal_Bool                bNoExceptions_;
            SalXLib            *pXLib_;
            SalDisplay         *m_pSalDisplay;
            pthread_t           hMainThread_;
            rtl::OUString       maLocalHostName;
            rtl::OUString       maUnicodeAccumulator;

public:
    X11SalData();
    virtual ~X11SalData();

    virtual void            Init();
    virtual void            initNWF();
    virtual void            deInitNWF();

    inline  void            XError( Display     *pDisplay, XErrorEvent *pEvent ) const;

    SalDisplay*             GetDisplay() const
    { return m_pSalDisplay; }
    void                    SetSalDisplay( SalDisplay* pDisplay )
    { m_pSalDisplay = pDisplay; }

    void                    DeleteDisplay(); // for shutdown

    inline  SalXLib*        GetLib() const { return pXLib_; }
    inline  pthread_t       GetMainThread() const { return hMainThread_; }

    void                    StartTimer( sal_uLong nMS );
    inline  void            StopTimer();
    void                    Timeout() const;

    const rtl::OUString&    GetLocalHostName() const
    { return maLocalHostName; }
    rtl::OUString&          GetUnicodeAccumulator() { return maUnicodeAccumulator; }

    static int XErrorHdl( Display*, XErrorEvent* );
    static int XIOErrorHdl( Display* );

    // set helper functions to set class and res name in W_CLASS hint
    static const char* getFrameResName();
    static const char* getFrameClassName();
    static rtl::OString getFrameResName( SalExtStyle nStyle );

};

inline X11SalData* GetX11SalData()
{ return (X11SalData*)ImplGetSVData()->mpSalData; }


#ifdef _SV_SALDISP_HXX
inline void X11SalData::XError( Display *pDisplay,  XErrorEvent *pEvent ) const
{ pXLib_->XError( pDisplay, pEvent ); }
#endif

class YieldMutexReleaser
{
    sal_uLong               m_nYieldCount;
public:
    inline YieldMutexReleaser();
    inline ~YieldMutexReleaser();
};

inline YieldMutexReleaser::YieldMutexReleaser()
{
    m_nYieldCount = GetSalData()->m_pInstance->ReleaseYieldMutex();
}

inline YieldMutexReleaser::~YieldMutexReleaser()
{
    GetSalData()->m_pInstance->AcquireYieldMutex( m_nYieldCount );
}

#endif // _SV_SALDATA_HXX

