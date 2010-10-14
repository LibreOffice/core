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
#ifndef SW_VBA_APPLICATION_HXX
#define SW_VBA_APPLICATION_HXX

#include <ooo/vba/word/XApplication.hpp>
#include <ooo/vba/word/XDocument.hpp>
#include <ooo/vba/word/XWindow.hpp>
#include <ooo/vba/word/XSystem.hpp>
#include <ooo/vba/word/XOptions.hpp>
#include <ooo/vba/word/XSelection.hpp>
#include <ooo/vba/word/XAddins.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbaapplicationbase.hxx>
#include <cppuhelper/implbase1.hxx>

//typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XApplication > SwVbaApplication_BASE;
typedef cppu::ImplInheritanceHelper1< VbaApplicationBase, ooo::vba::word::XApplication > SwVbaApplication_BASE;

class SwVbaApplication : public SwVbaApplication_BASE
{
public:
    SwVbaApplication( css::uno::Reference< css::uno::XComponentContext >& m_xContext );
    virtual ~SwVbaApplication();

    virtual SfxObjectShell* GetDocShell( const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);

    // XApplication
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XSystem > SAL_CALL getSystem() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::word::XDocument > SAL_CALL getActiveDocument() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::word::XWindow > SAL_CALL getActiveWindow() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XOptions > SAL_CALL getOptions() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XSelection > SAL_CALL getSelection() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL CommandBars( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Documents( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Addins( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Dialogs( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL ListGalleries( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getDisplayAutoCompleteTips() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayAutoCompleteTips( sal_Bool _displayAutoCompleteTips ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getEnableCancelKey() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEnableCancelKey( sal_Int32 _enableCancelKey ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL CentimetersToPoints( float _Centimeters ) throw (css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
protected:
    virtual css::uno::Reference< css::frame::XModel > getCurrentDocument() throw (css::uno::RuntimeException);
};
#endif /* SW_VBA_APPLICATION_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
