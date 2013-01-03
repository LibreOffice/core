/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef UNOMODEL_HXX
#define UNOMODEL_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <sfx2/sfxbasemodel.hxx>

//-----------------------------------------------------------------------------
class SIDEModel : public SfxBaseModel,
                public com::sun::star::lang::XServiceInfo
{
    void notImplemented() throw ( ::com::sun::star::io::IOException );
public:
    SIDEModel( SfxObjectShell *pObjSh = 0 );
    virtual ~SIDEModel();

    //XInterface
    virtual     ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

    //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
        throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
            throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
            throw( ::com::sun::star::uno::RuntimeException );
    // XStorable2
    virtual void SAL_CALL storeSelf( const  ::com::sun::star::uno::Sequence< PROPERTYVALUE >& )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException) { notImplemented(); }
    //  XStorable
    virtual void SAL_CALL store() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeAsURL(   const   ::rtl::OUString& sURL,
        const   ::com::sun::star::uno::Sequence< PROPERTYVALUE >&   seqArguments    )
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeToURL(   const   ::rtl::OUString& sURL,
        const   ::com::sun::star::uno::Sequence< PROPERTYVALUE >&   seqArguments    )
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Sequence< rtl::OUString > getSupportedServiceNames_Static();
    static ::rtl::OUString getImplementationName_Static();
};

com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SIDEModel_createInstance(
                const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rSMgr )
                    throw( com::sun::star::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
