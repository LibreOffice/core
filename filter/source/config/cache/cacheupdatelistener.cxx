/*************************************************************************
 *
 *  $RCSfile: cacheupdatelistener.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-29 13:39:12 $
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

#include "cacheupdatelistener.hxx"

//_______________________________________________
// includes

#ifndef _COM_SUN_STAR_UTIL_XCHANGESNOTIFIER_HPP_
#include <com/sun/star/util/XChangesNotifier.hpp>
#endif

#ifndef _COMPHELPER_SINGLETONREF_HXX_
#include <comphelper/singletonref.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//_______________________________________________
// namespace

namespace filter{
    namespace config{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

/*-----------------------------------------------
    05.03.2004 08:36
-----------------------------------------------*/
CacheUpdateListener::CacheUpdateListener(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : BaseLock(     )
    , m_xSMGR (xSMGR)
    , m_rCache(     )
{
}

/*-----------------------------------------------
    05.03.2004 08:37
-----------------------------------------------*/
CacheUpdateListener::~CacheUpdateListener()
{
}

/*-----------------------------------------------
    07.03.2004 07:59
-----------------------------------------------*/
void CacheUpdateListener::startListening(const css::uno::Reference< css::uno::XInterface >& xConfigAccess)
{
    css::uno::Reference< css::util::XChangesNotifier > xNotifier(xConfigAccess, css::uno::UNO_QUERY);
    if (!xNotifier.is())
        return;

    css::uno::Reference< css::util::XChangesListener > xThis(static_cast< css::util::XChangesListener* >(this), css::uno::UNO_QUERY);
    xNotifier->addChangesListener(xThis);

/*
    css::uno::Reference< css::container::XContainer > xNotifier(xConfigAccess, css::uno::UNO_QUERY);
    if (!xNotifier.is())
        return;

    css::uno::Reference< css::container::XContainerListener > xThis(static_cast< css::container::XContainerListener* >(this), css::uno::UNO_QUERY);
    xNotifier->addContainerListener(xThis);
*/
}

/*-----------------------------------------------
    07.03.2004 07:59
-----------------------------------------------*/
void CacheUpdateListener::stopListening(const css::uno::Reference< css::uno::XInterface >& xConfigAccess)
{
    css::uno::Reference< css::util::XChangesNotifier > xNotifier(xConfigAccess, css::uno::UNO_QUERY);
    if (!xNotifier.is())
        return;

    css::uno::Reference< css::util::XChangesListener > xThis(static_cast< css::util::XChangesListener* >(this), css::uno::UNO_QUERY);
    xNotifier->removeChangesListener(xThis);

/*
    css::uno::Reference< css::container::XContainer > xNotifier(xConfigAccess, css::uno::UNO_QUERY);
    if (!xNotifier.is())
        return;

    css::uno::Reference< css::container::XContainerListener > xThis(static_cast< css::container::XContainerListener* >(this), css::uno::UNO_QUERY);
    xNotifier->removeContainerListener(xThis);
*/
}

/*-----------------------------------------------
    07.03.2004 08:17
-----------------------------------------------*/
void SAL_CALL  CacheUpdateListener::changesOccurred(const css::util::ChangesEvent& aEvent)
    throw(css::uno::RuntimeException)
{
}

/*-----------------------------------------------
    05.03.2004 08:44
-----------------------------------------------*/
void SAL_CALL CacheUpdateListener::disposing(const css::lang::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    // We dont know the configuration access. Because it was given from outside and its lifetime
    // is controlled there too. We live, till this external config access lives ...
    // If we are registered for multiple access points, we live a little bit longer :-)
}

    } // namespace config
} // namespace filter
