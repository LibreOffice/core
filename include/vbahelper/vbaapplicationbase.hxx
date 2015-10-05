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
#ifndef INCLUDED_VBAHELPER_VBAAPPLICATIONBASE_HXX
#define INCLUDED_VBAHELPER_VBAAPPLICATIONBASE_HXX

#include <ooo/vba/XHelperInterface.hpp>
#include <ooo/vba/XApplicationBase.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <sfx2/objsh.hxx>

typedef InheritedHelperInterfaceWeakImpl< ov::XApplicationBase > ApplicationBase_BASE;

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
    virtual css::uno::Reference< ov::XHelperInterface > SAL_CALL getParent(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException) SAL_OVERRIDE { return this; }

    virtual sal_Bool SAL_CALL getScreenUpdating() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setScreenUpdating(sal_Bool bUpdate) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getDisplayStatusBar() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDisplayStatusBar(sal_Bool bDisplayStatusBar) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getInteractive() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setInteractive( sal_Bool bInteractive ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL OnKey( const OUString& Key, const ::com::sun::star::uno::Any& Procedure ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL CommandBars( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getVersion() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL getVBE() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Any SAL_CALL Run( const OUString& MacroName, const css::uno::Any& varg1, const css::uno::Any& varg2, const css::uno::Any& varg3, const css::uno::Any& varg4, const css::uno::Any& varg5, const css::uno::Any& varg6, const css::uno::Any& varg7, const css::uno::Any& varg8, const css::uno::Any& varg9, const css::uno::Any& varg10, const css::uno::Any& varg11, const css::uno::Any& varg12, const css::uno::Any& varg13, const css::uno::Any& varg14, const css::uno::Any& varg15, const css::uno::Any& varg16, const css::uno::Any& varg17, const css::uno::Any& varg18, const css::uno::Any& varg19, const css::uno::Any& varg20, const css::uno::Any& varg21, const css::uno::Any& varg22, const css::uno::Any& varg23, const css::uno::Any& varg24, const css::uno::Any& varg25, const css::uno::Any& varg26, const css::uno::Any& varg27, const css::uno::Any& varg28, const css::uno::Any& varg29, const css::uno::Any& varg30 ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL OnTime( const css::uno::Any& aEarliestTime, const OUString& aFunction, const css::uno::Any& aLatestTime, const css::uno::Any& aSchedule ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual float SAL_CALL CentimetersToPoints( float _Centimeters ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL Undo() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL Quit() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
