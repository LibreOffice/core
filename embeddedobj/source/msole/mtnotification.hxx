/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mtnotification.hxx,v $
 * $Revision: 1.5 $
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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <cppuhelper/weakref.hxx>

#include <rtl/ref.hxx>

class OleEmbeddedObject;

#define OLECOMP_ONVIEWCHANGE    1
#define OLECOMP_ONCLOSE         2

class MainThreadNotificationRequest
{
    OleEmbeddedObject* m_pObject;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::embed::XEmbeddedObject > m_xObject;

    sal_uInt16 m_nNotificationType;
    sal_uInt32 m_nAspect;

public:
    MainThreadNotificationRequest( const ::rtl::Reference< OleEmbeddedObject >& xObj, sal_uInt16 nNotificationType, sal_uInt32 nAspect = 0 );

    static long worker( MainThreadNotificationRequest*, MainThreadNotificationRequest* );

    static void mainThreadWorkerStart( MainThreadNotificationRequest* );
};


