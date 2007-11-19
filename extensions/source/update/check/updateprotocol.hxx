/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updateprotocol.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-19 16:50:14 $
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

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_DEPLOYMENT_UPDATEINFORMATIONPROVIDER_HPP_
#include <com/sun/star/deployment/UpdateInformationProvider.hpp>
#endif

#include <vector>
#include "updateinfo.hxx"

// Returns 'true' if successfully connected to the update server
bool checkForUpdates(
    UpdateInfo& o_rUpdateInfo,
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& rxInteractionHandler,
    const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XUpdateInformationProvider >& rxProvider
);

// Returns 'true' if there are updates for any extension
bool checkForExtensionUpdates(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
);

bool checkForPendingUpdates(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
);

bool storeExtensionUpdateInfos(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< rtl::OUString > > &rUpdateInfos
);

