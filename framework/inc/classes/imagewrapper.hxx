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

#ifndef __FRAMEWORK_CLASSES_IMAGEWRAPPER_HXX_
#define __FRAMEWORK_CLASSES_IMAGEWRAPPER_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cppuhelper/implbase2.hxx>
#include <vcl/image.hxx>
#include <framework/fwedllapi.h>

namespace framework
{

class FWE_DLLPUBLIC ImageWrapper : public ThreadHelpBase                            ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                     public ::cppu::WeakImplHelper2< ::com::sun::star::awt::XBitmap,
                                                    ::com::sun::star::lang::XUnoTunnel >
{
    public:
        ImageWrapper( const Image& aImage );
        virtual ~ImageWrapper();

        const Image&    GetImage() const
        {
            return m_aImage;
        }

        static ::com::sun::star::uno::Sequence< sal_Int8 > GetUnoTunnelId();

        // XBitmap
        virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getDIB() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getMaskDIB() throw (::com::sun::star::uno::RuntimeException);

        // XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (::com::sun::star::uno::RuntimeException);

    private:
        Image   m_aImage;
};

}

#endif // __FRAMEWORK_CLASSES_IMAGEWRAPPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
