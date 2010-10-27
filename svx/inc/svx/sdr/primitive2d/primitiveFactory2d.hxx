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

#ifndef INCLUDED_SDR_PRIMITIVE2D_PRIMITIVEFACTORY2D_HXX
#define INCLUDED_SDR_PRIMITIVE2D_PRIMITIVEFACTORY2D_HXX

#include <com/sun/star/graphic/XPrimitiveFactory2D.hpp>
#include <cppuhelper/compbase1.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <svx/svxdllapi.h>

//////////////////////////////////////////////////////////////////////////////
// UNO API helper methods

namespace drawinglayer
{
    namespace primitive2d
    {
        SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL XPrimitiveFactory2DProvider_createInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr) throw( ::com::sun::star::uno::Exception );
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// PrimitiveFactory2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        // typedef for PrimitiveFactory2DImplBase
        typedef cppu::WeakComponentImplHelper1< ::com::sun::star::graphic::XPrimitiveFactory2D > PrimitiveFactory2DImplBase;

        // base class for C++ implementation of com::sun::star::graphic::XPrimitiveFactory2D
        class PrimitiveFactory2D
        :   protected comphelper::OBaseMutex,
            public PrimitiveFactory2DImplBase
        {
        private:
        protected:
        public:
            // constructor
            PrimitiveFactory2D();

            // Methods from XPrimitiveFactory2D
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XPrimitive2D > > SAL_CALL createPrimitivesFromXShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aParms ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XPrimitive2D > > SAL_CALL createPrimitivesFromXDrawPage( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xDrawPage, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aParms ) throw (::com::sun::star::uno::RuntimeException);

            // UNO API helpers
            SVX_DLLPUBLIC static rtl::OUString getImplementationName_Static();
            SVX_DLLPUBLIC static com::sun::star::uno::Sequence< rtl::OUString > getSupportedServiceNames_Static();
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SDR_PRIMITIVE2D_PRIMITIVEFACTORY2D_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
