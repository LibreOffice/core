/*************************************************************************
 *
 *  $RCSfile: dispatchwatcher.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cd $ $Date: 2001-11-05 07:16:26 $
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

#include "dispatchwatcher.hxx"

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XTASKSSUPPLIER_HPP_
#include <com/sun/star/frame/XTasksSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;

namespace desktop
{

// Create or get the dispatch watcher implementation
Reference< XStatusListener > DispatchWatcher::GetDispatchWatcher()
{
    static DispatchWatcher* pDispatchWatcher = NULL;

    if ( !pDispatchWatcher )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if ( !pDispatchWatcher )
            pDispatchWatcher = new DispatchWatcher( ::comphelper::getProcessServiceFactory() );
    }

    return pDispatchWatcher;
}


DispatchWatcher::DispatchWatcher( const com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rServiceManager ) :
    m_xServiceManager( rServiceManager )
{
}


DispatchWatcher::~DispatchWatcher()
{
}


void SAL_CALL DispatchWatcher::disposing( const ::com::sun::star::lang::EventObject& )
throw(::com::sun::star::uno::RuntimeException)
{
}


void SAL_CALL DispatchWatcher::statusChanged( const com::sun::star::frame::FeatureStateEvent& rEvent )
throw(::com::sun::star::uno::RuntimeException)
{
    if ( !rEvent.IsEnabled )
    {
        // There was an error dispatching slot:5500 which means start office template&document wizard.
        // We have to be sure that there is a task open so the user can use the office. If we
        // don't have a task open we have to shutdown the office otherwise the user cannot
        // use his/her office installation until office gets killed!!!
        Reference< XTasksSupplier > xTasksSupplier( m_xServiceManager->createInstance(
                                                    OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                                                UNO_QUERY );

        Reference< XEnumeration > xList = xTasksSupplier->getTasks()->createEnumeration();

        if ( !xList->hasMoreElements() )
        {
            // We don't have any task open so we have to shutdown ourself!!
            Reference< XDesktop > xDesktop( xTasksSupplier, UNO_QUERY );
            if ( xDesktop.is() )
                xDesktop->terminate();
        }
    }
}

}
