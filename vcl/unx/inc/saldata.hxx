/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: saldata.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 11:56:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_SALDATA_HXX
#define _SV_SALDATA_HXX

// -=-= includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <signal.h>

#ifndef _SALSTD_HXX
#include <salstd.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALINST_H
#include <salinst.h>
#endif
#ifndef _OSL_MODULE_H
#include <osl/module.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <dllapi.h>
#endif

// -=-= forwards -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SalXLib;
class SalDisplay;
class SalPrinter;

// -=-= typedefs -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

DECLARE_LIST( SalDisplays, SalDisplay* )

#if defined SCO || defined LINUX || defined NETBSD || defined AIX || defined HPUX || defined FREEBSD || defined MACOSX
#include <pthread.h>
#else
typedef unsigned int pthread_t;
#endif

// -=-= SalData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class VCL_DLLPUBLIC SalData
{
protected:
            BOOL                bNoExceptions_;

            SalXLib            *pXLib_;

            SalDisplay         *m_pSalDisplay;

            pthread_t           hMainThread_;

public:
            SalInstance*        pInstance_;         // pointer to instance
            oslModule           m_pPlugin; // plugin library handle
public:
    SalData();
    virtual ~SalData();

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

    void                    StartTimer( ULONG nMS );
    inline  void            StopTimer();
    void                    Timeout() const;

    static int XErrorHdl( Display*, XErrorEvent* );
    static int XIOErrorHdl( Display* );

    // set helper functions to set class and res name in W_CLASS hint
    static const char* getFrameResName();
    static const char* getFrameClassName();
    static rtl::OString getFrameResName( SalExtStyle nStyle );

};

// -=-= inlines =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline void SetSalData( SalData* pData )
{ ImplGetSVData()->mpSalData = (void*)pData; }

inline SalData* GetSalData()
{ return (SalData*)ImplGetSVData()->mpSalData; }

#ifdef _SV_SALDISP_HXX
inline void SalData::XError( Display *pDisplay, XErrorEvent *pEvent ) const
{ pXLib_->XError( pDisplay, pEvent ); }
#endif

class YieldMutexReleaser
{
    ULONG               m_nYieldCount;
public:
    inline YieldMutexReleaser();
    inline ~YieldMutexReleaser();
};

inline YieldMutexReleaser::YieldMutexReleaser()
{
    m_nYieldCount = GetSalData()->pInstance_->ReleaseYieldMutex();
}

inline YieldMutexReleaser::~YieldMutexReleaser()
{
    GetSalData()->pInstance_->AcquireYieldMutex( m_nYieldCount );
}

#endif // _SV_SALDATA_HXX

