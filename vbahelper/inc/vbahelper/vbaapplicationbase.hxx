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
#ifndef VBA_APPLICATION_BASE_HXX
#define VBA_APPLICATION_BASE_HXX

#include <ooo/vba/XHelperInterface.hpp>
#include <ooo/vba/XApplicationBase.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <sfx2/objsh.hxx>

typedef InheritedHelperInterfaceImpl1< ov::XApplicationBase > ApplicationBase_BASE;

struct VbaApplicationBase_Impl;

class VBAHELPER_DLLPUBLIC VbaApplicationBase : public ApplicationBase_BASE
{
    VbaApplicationBase_Impl* m_pImpl;

protected:
    VbaApplicationBase( const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~VbaApplicationBase();

    virtual css::uno::Reference< css::frame::XModel > getCurrentDocument() throw (css::uno::RuntimeException) = 0;
public:
    // XHelperInterface ( parent is itself )
    virtual css::uno::Reference< ov::XHelperInterface > SAL_CALL getParent(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException) { return this; }

    virtual sal_Bool SAL_CALL getScreenUpdating() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setScreenUpdating(sal_Bool bUpdate) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getDisplayStatusBar() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayStatusBar(sal_Bool bDisplayStatusBar) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getInteractive() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setInteractive( ::sal_Bool bInteractive ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( ::sal_Bool bVisible ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL CommandBars( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getVersion() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getVBE() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getVBProjects() throw (css::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL Run( const ::rtl::OUString& MacroName, const css::uno::Any& varg1, const css::uno::Any& varg2, const css::uno::Any& varg3, const css::uno::Any& varg4, const css::uno::Any& varg5, const css::uno::Any& varg6, const css::uno::Any& varg7, const css::uno::Any& varg8, const css::uno::Any& varg9, const css::uno::Any& varg10, const css::uno::Any& varg11, const css::uno::Any& varg12, const css::uno::Any& varg13, const css::uno::Any& varg14, const css::uno::Any& varg15, const css::uno::Any& varg16, const css::uno::Any& varg17, const css::uno::Any& varg18, const css::uno::Any& varg19, const css::uno::Any& varg20, const css::uno::Any& varg21, const css::uno::Any& varg22, const css::uno::Any& varg23, const css::uno::Any& varg24, const css::uno::Any& varg25, const css::uno::Any& varg26, const css::uno::Any& varg27, const css::uno::Any& varg28, const css::uno::Any& varg29, const css::uno::Any& varg30 ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL OnTime( const css::uno::Any& aEarliestTime, const ::rtl::OUString& aFunction, const css::uno::Any& aLatestTime, const css::uno::Any& aSchedule ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL CentimetersToPoints( float _Centimeters ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Undo() throw (css::uno::RuntimeException);
    virtual void SAL_CALL Quit() throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
