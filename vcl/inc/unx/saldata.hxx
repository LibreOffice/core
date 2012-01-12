/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SV_SALDATA_HXX
#define _SV_SALDATA_HXX

// -=-= includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <signal.h>
#include <unx/salstd.hxx>
#include <salframe.hxx>
#include <unx/salinst.h>
#include <saldatabasic.hxx>
#ifndef _OSL_MODULE_H
#include <osl/module.h>
#endif
#include <vclpluginapi.h>

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
class VCLPLUG_GEN_PUBLIC X11SalData : public SalData
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

