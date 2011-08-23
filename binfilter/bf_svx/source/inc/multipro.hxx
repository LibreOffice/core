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
#ifndef _MULTIPRO_HXX
#define _MULTIPRO_HXX

#include <com/sun/star/form/XFormComponent.hpp>
#include <comphelper/composedprops.hxx>
#include <svdmark.hxx>
namespace binfilter {

//.........................................................................
namespace svxform
{
//.........................................................................

    //==================================================================
    // FmXMultiSet
    //==================================================================
    typedef ::comphelper::OComposedPropertySet	FmXMultiSet_Base;
    class FmXMultiSet
        :public FmXMultiSet_Base
        ,public ::com::sun::star::form::XFormComponent
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >	m_xParent;

    public:
        FmXMultiSet(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> > & _rPropertySets);

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire(  ) throw();
        virtual void SAL_CALL release(  ) throw();

        // XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

        static FmXMultiSet* Create(const SdrMarkList& rMarkList);
    };

//.........................................................................
}	// namespace svxform
//.........................................................................

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
