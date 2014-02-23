/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_TOOLKIT_AWT_XTHROBBER_HXX
#define INCLUDED_TOOLKIT_AWT_XTHROBBER_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/awt/XThrobber.hpp>

#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>


namespace toolkit
{



    //= XThrobber

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


} // namespacetoolkit


#endif // INCLUDED_TOOLKIT_AWT_XTHROBBER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
