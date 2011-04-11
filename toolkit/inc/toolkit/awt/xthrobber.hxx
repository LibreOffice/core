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
#ifndef TOOLKIT_AWT_XTHROBBER_HXX
#define TOOLKIT_AWT_XTHROBBER_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/awt/XThrobber.hpp>

#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

//........................................................................
namespace toolkit
{
//........................................................................

    //====================================================================
    //= XThrobber
    //====================================================================
    typedef ::cppu::ImplInheritanceHelper1  <   VCLXWindow
                                            ,   ::com::sun::star::awt::XThrobber
                                            >   XThrobber_Base;

    class XThrobber :public XThrobber_Base
                    ,public ::boost::noncopyable
    {
    private:
        void SAL_CALL InitImageList() throw(::com::sun::star::uno::RuntimeException);

    public:
        XThrobber();

    protected:
        ~XThrobber();

        // XThrobber
        virtual void SAL_CALL start() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL stop() throw (::com::sun::star::uno::RuntimeException);

        // VCLXWindow
        virtual void    SetWindow( Window* pWindow );

    private:
        XThrobber( const XThrobber& );            // never implemented
        XThrobber& operator=( const XThrobber& ); // never implemented
    };

//........................................................................
} // namespacetoolkit
//........................................................................

#endif // TOOLKIT_AWT_XTHROBBER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
