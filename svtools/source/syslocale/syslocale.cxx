/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: syslocale.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:28:36 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#ifndef GCC
#endif

#include "syslocale.hxx"

#ifndef _SVT_BROADCAST_HXX
#include <broadcast.hxx>
#endif
#ifndef _SVT_LISTENER_HXX
#include <listener.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX
#include <smplhint.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALEOPTIONS_HXX
#include "syslocaleoptions.hxx"
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif


using namespace osl;
using namespace com::sun::star;


SvtSysLocale_Impl*  SvtSysLocale::pImpl = NULL;
sal_Int32           SvtSysLocale::nRefCount = 0;


class SvtSysLocale_Impl : public SvtListener
{
    friend class SvtSysLocale;

        SvtSysLocaleOptions     aSysLocaleOptions;
        LocaleDataWrapper*      pLocaleData;
        CharClass*              pCharClass;

public:
                                SvtSysLocale_Impl();
    virtual                     ~SvtSysLocale_Impl();

    virtual void                Notify( SvtBroadcaster& rBC, const SfxHint& rHint );

};


// -----------------------------------------------------------------------

SvtSysLocale_Impl::SvtSysLocale_Impl()
{
    const lang::Locale& rLocale = Application::GetSettings().GetLocale();
    pLocaleData = new LocaleDataWrapper(
        ::comphelper::getProcessServiceFactory(), rLocale );
    pCharClass = new CharClass(
        ::comphelper::getProcessServiceFactory(), rLocale );
    aSysLocaleOptions.AddListener( *this );
}


SvtSysLocale_Impl::~SvtSysLocale_Impl()
{
    aSysLocaleOptions.RemoveListener( *this );
    delete pCharClass;
    delete pLocaleData;
}


void SvtSysLocale_Impl::Notify( SvtBroadcaster&, const SfxHint& rHint )
{
    const SfxSimpleHint* p = PTR_CAST( SfxSimpleHint, &rHint );
    if( p && (p->GetId() & SYSLOCALEOPTIONS_HINT_LOCALE) )
    {
        MutexGuard aGuard( SvtSysLocale::GetMutex() );
        const lang::Locale& rLocale = Application::GetSettings().GetLocale();
        pLocaleData->setLocale( rLocale );
        pCharClass->setLocale( rLocale );
    }
}


// ====================================================================

SvtSysLocale::SvtSysLocale()
{
    MutexGuard aGuard( GetMutex() );
    if ( !pImpl )
        pImpl = new SvtSysLocale_Impl;
    ++nRefCount;
}


SvtSysLocale::~SvtSysLocale()
{
    MutexGuard aGuard( GetMutex() );
    if ( !--nRefCount )
    {
        delete pImpl;
        pImpl = NULL;
    }
}


// static
Mutex& SvtSysLocale::GetMutex()
{
    static Mutex* pMutex = NULL;
    if( !pMutex )
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if( !pMutex )
        {
            static Mutex aMutex;
            pMutex = &aMutex;
        }
    }
    return *pMutex;
}


const LocaleDataWrapper& SvtSysLocale::GetLocaleData() const
{
    return *(pImpl->pLocaleData);
}


const LocaleDataWrapper* SvtSysLocale::GetLocaleDataPtr() const
{
    return pImpl->pLocaleData;
}


const CharClass& SvtSysLocale::GetCharClass() const
{
    return *(pImpl->pCharClass);
}


const CharClass* SvtSysLocale::GetCharClassPtr() const
{
    return pImpl->pCharClass;
}
