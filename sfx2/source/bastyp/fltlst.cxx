/*************************************************************************
 *
 *  $RCSfile: fltlst.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 07:58:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "fltlst.hxx"

//*****************************************************************************************************************
//  includes
//*****************************************************************************************************************
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include "sfxuno.hxx"
#include "docfac.hxx"

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
        uno::Reference< util::XFlushable > xNotifier( xSmgr->createInstance( DEFINE_CONST_OUSTRING("com.sun.star.document.FilterFactory") ), uno::UNO_QUERY );
        if( xNotifier.is() == sal_True )
        {
            m_xFilterCache = xNotifier;
            m_xFilterCache->addFlushListener( this );
        }

        xNotifier = uno::Reference< util::XFlushable >( xSmgr->createInstance( DEFINE_CONST_OUSTRING("com.sun.star.document.TypeDetection") ), uno::UNO_QUERY );
        if( xNotifier.is() == sal_True )
        {
            m_xTypeCache = xNotifier;
            m_xTypeCache->addFlushListener( this );
        }
    }
}

SfxFilterListener::~SfxFilterListener()
{
}

void SAL_CALL SfxFilterListener::flushed( const lang::EventObject& aSource ) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    uno::Reference< util::XFlushable > xContainer( aSource.Source, uno::UNO_QUERY );
    if(
        (xContainer.is()                                       ) &&
        (xContainer==m_xTypeCache || xContainer==m_xFilterCache)
      )
    {
        SfxFilterContainer::ReadFilters_Impl();
    }
}

void SAL_CALL SfxFilterListener::disposing( const lang::EventObject& aSource ) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    uno::Reference< util::XFlushable > xNotifier( aSource.Source, uno::UNO_QUERY );
    if (!xNotifier.is())
        return;

    if (xNotifier == m_xTypeCache)
    {
        m_xTypeCache->removeFlushListener( this );
        m_xTypeCache = uno::Reference< util::XFlushable >();
    }
    else
    if (xNotifier == m_xFilterCache)
    {
        m_xFilterCache->removeFlushListener( this );
        m_xFilterCache = uno::Reference< util::XFlushable >();
    }
}
