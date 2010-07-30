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

#ifndef _GSTMANAGER_HXX
#define _GSTMANAGER_HXX

#include "gstcommon.hxx"

#ifndef _COM_SUN_STAR_MEDIA_XMANAGER_HDL_
#include "com/sun/star/media/XManager.hdl"
#endif

// -----------
// - Manager -
// -----------

namespace avmedia
{
namespace gst
{
class Manager : public ::cppu::WeakImplHelper2< ::com::sun::star::media::XManager,
                                                ::com::sun::star::lang::XServiceInfo >
{
public: Manager( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMgr );
    ~Manager();

    // XManager
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > SAL_CALL createPlayer(
        const ::rtl::OUString& aURL )
     throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
     throw( ::com::sun::star::uno::RuntimeException );


private:

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMgr;
};
} // namespace gst
} // namespace avmedia

#endif // _GSTMANAGER_HXX
