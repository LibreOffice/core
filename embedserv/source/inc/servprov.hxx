/*************************************************************************
 *
 *  $RCSfile: servprov.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2003-04-10 14:55:58 $
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

#ifndef _SERVPROV_HXX
#define _SERVPROV_HXX
#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#undef _DEBUG
#endif

#include "common.h"
#include <oleidl.h>

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef __COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif


class EmbedProviderFactory_Impl;

class EmbedServer_Impl : public ::cppu::OWeakObject, ::com::sun::star::lang::XTypeProvider
{
public:
    EmbedServer_Impl( const ::com::sun::star::uno::Reference<::com::sun::star::lang::XMultiServiceFactory> &xFactory );
    ~EmbedServer_Impl();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL release() throw (::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes( )
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw(::com::sun::star::uno::RuntimeException);

protected:

    CComPtr< EmbedProviderFactory_Impl > m_pOLEFactories[ SUPPORTED_FACTORIES_NUM ];
    ::com::sun::star::uno::Reference<::com::sun::star::lang::XMultiServiceFactory> m_xFactory;
};

class EmbedProviderFactory_Impl : public IClassFactory
{
public:

    EmbedProviderFactory_Impl( const ::com::sun::star::uno::Reference<::com::sun::star::lang::XMultiServiceFactory>& xFactory, const GUID* pGuid);
    ~EmbedProviderFactory_Impl();

    sal_Bool registerClass();
    sal_Bool deregisterClass();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    /* IClassFactory methods */
    STDMETHOD(CreateInstance)(IUnknown FAR* punkOuter, REFIID riid, void FAR* FAR* ppv);
    STDMETHOD(LockServer)(int fLock);

protected:

    oslInterlockedCount m_refCount;
    GUID                m_guid;
    DWORD               m_factoryHandle;

    ::com::sun::star::uno::Reference<::com::sun::star::lang::XMultiServiceFactory> m_xFactory;
};

#endif

