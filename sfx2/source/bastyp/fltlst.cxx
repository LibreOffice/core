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


#include "fltlst.hxx"

//*****************************************************************************************************************
//  includes
//*****************************************************************************************************************
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <comphelper/processfactory.hxx>

#include <sfx2/sfxuno.hxx>
#include <sfx2/docfac.hxx>

#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

//*****************************************************************************************************************
//  namespaces
//*****************************************************************************************************************
using namespace ::com::sun::star;


class SfxRefreshListener : public ::cppu::WeakImplHelper1<com::sun::star::util::XRefreshListener>
{
    private:
        SfxFilterListener *m_pOwner;

    public:
        SfxRefreshListener(SfxFilterListener *pOwner)
            : m_pOwner(pOwner)
        {
        }

        virtual ~SfxRefreshListener()
        {
        }

        // util.XRefreshListener
        virtual void SAL_CALL refreshed( const ::com::sun::star::lang::EventObject& rEvent )
            throw(com::sun::star::uno::RuntimeException)
        {
            m_pOwner->refreshed(rEvent);
        }

        // lang.XEventListener
        virtual void SAL_CALL disposing(const com::sun::star::lang::EventObject& rEvent)
            throw(com::sun::star::uno::RuntimeException)
        {
            m_pOwner->disposing(rEvent);
        }
};



//*****************************************************************************************************************
//  definitions
//*****************************************************************************************************************

/*-************************************************************************************************************//**
    @short          ctor
    @descr          These initialize an instance of a SfxFilterListener class. Created object listen automaticly
                    on right FilterFactory-Service for all changes and synchronize right SfxFilterContainer with
                    corresponding framework-cache.
                    We use given "sFactory" value to decide which query must be used to fill "pContainer" with new values.
                    Given "pContainer" hold us alive as uno reference and we use it to syschronize it with framework caches.
                    We will die, if he die! see dtor for further informations.

    @seealso        dtor
    @seealso        class framework::FilterCache
    @seealso        service ::document::FilterFactory

    @param          "sFactory"  , short name of module which contains filter container
    @param          "pContainer", pointer to filter container which will be informed
    @return         -

    @onerror        We show some assertions in non product version.
                    Otherwise we do nothing!
    @threadsafe     yes
*//*-*************************************************************************************************************/
SfxFilterListener::SfxFilterListener()
{
    uno::Reference< lang::XMultiServiceFactory > xSmgr = ::comphelper::getProcessServiceFactory();
    if( xSmgr.is() == sal_True )
    {
        uno::Reference< util::XRefreshable > xNotifier( xSmgr->createInstance( DEFINE_CONST_OUSTRING("com.sun.star.document.FilterConfigRefresh") ), uno::UNO_QUERY );
        if( xNotifier.is() == sal_True )
        {
            m_xFilterCache = xNotifier;
            m_xFilterCacheListener = new SfxRefreshListener(this);
            m_xFilterCache->addRefreshListener( m_xFilterCacheListener );
        }
    }
}

SfxFilterListener::~SfxFilterListener()
{
}

void SAL_CALL SfxFilterListener::refreshed( const lang::EventObject& aSource ) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< util::XRefreshable > xContainer( aSource.Source, uno::UNO_QUERY );
    if(
        (xContainer.is()           ) &&
        (xContainer==m_xFilterCache)
      )
    {
        SfxFilterContainer::ReadFilters_Impl( sal_True );
    }
}

void SAL_CALL SfxFilterListener::disposing( const lang::EventObject& aSource ) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< util::XRefreshable > xNotifier( aSource.Source, uno::UNO_QUERY );
    if (!xNotifier.is())
        return;

    if (xNotifier == m_xFilterCache)
        m_xFilterCache.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
