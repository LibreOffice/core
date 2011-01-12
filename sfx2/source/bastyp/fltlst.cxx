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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

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
#include <vos/mutex.hxx>

//*****************************************************************************************************************
//  namespaces
//*****************************************************************************************************************
using namespace ::com::sun::star;

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

    @last_change    17.10.2001 10:27
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
            m_xFilterCache->addRefreshListener( this );
        }
    }
}

SfxFilterListener::~SfxFilterListener()
{
}

void SAL_CALL SfxFilterListener::refreshed( const lang::EventObject& aSource ) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    uno::Reference< util::XRefreshable > xNotifier( aSource.Source, uno::UNO_QUERY );
    if (!xNotifier.is())
        return;

    if (xNotifier == m_xFilterCache)
        m_xFilterCache.clear();
}
