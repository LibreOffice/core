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
#ifndef TOOLKIT_AWT_XSIMPLEANIMATION_HXX
#define TOOLKIT_AWT_XSIMPLEANIMATION_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/awt/XSimpleAnimation.hpp>

#include <boost/scoped_ptr.hpp>

//........................................................................
namespace toolkit
{
//........................................................................

    //====================================================================
    //= XSimpleAnimation
    //====================================================================
    typedef ::cppu::ImplInheritanceHelper1  <   VCLXWindow
                                            ,   ::com::sun::star::awt::XSimpleAnimation
                                            >   XSimpleAnimation_Base;

    class XSimpleAnimation : public XSimpleAnimation_Base
    {
    public:
        XSimpleAnimation();

    protected:
        ~XSimpleAnimation();

        // XSimpleAnimation
        virtual void SAL_CALL start() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL stop() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setImageList( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > >& ImageList )
                                            throw (::com::sun::star::uno::RuntimeException);
        // VclWindowPeer
        virtual void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

    private:
        XSimpleAnimation( const XSimpleAnimation& );            // never implemented
        XSimpleAnimation& operator=( const XSimpleAnimation& ); // never implemented
    };

//........................................................................
} // namespacetoolkit
//........................................................................

#endif // TOOLKIT_INC_TOOLKIT_AWT_XSIMPLEANIMATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
