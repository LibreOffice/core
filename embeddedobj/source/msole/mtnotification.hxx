/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mtnotification.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-23 07:33:07 $
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

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


