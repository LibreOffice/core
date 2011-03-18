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
#ifndef SC_VBA_COMMANDBAR_HXX
#define SC_VBA_COMMANDBAR_HXX

#include <ooo/vba/XCommandBar.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>

#include <vbahelper/vbahelperinterface.hxx>
#include "vbacommandbarhelper.hxx"

#include <map>

typedef InheritedHelperInterfaceImpl1< ov::XCommandBar > CommandBar_BASE;

class ScVbaCommandBar : public CommandBar_BASE
{
private:
    VbaCommandBarHelperRef pCBarHelper;
    css::uno::Reference< css::container::XIndexAccess > m_xBarSettings;
    rtl::OUString   m_sResourceUrl;
    sal_Bool        m_bIsMenu;
    sal_Bool        m_bTemporary;

public:
    ScVbaCommandBar( const css::uno::Reference< ov::XHelperInterface > xParent, const css::uno::Reference< css::uno::XComponentContext > xContext, VbaCommandBarHelperRef pHelper, const css::uno::Reference< css::container::XIndexAccess >& xBarSettings, const rtl::OUString& sResourceUrl, sal_Bool bIsMenu, sal_Bool bTemporary = sal_True ) throw( css::uno::RuntimeException );

    sal_Bool IsMenu() const { return m_bIsMenu; }

    // Attributes
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& _name ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( ::sal_Bool _visible ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getEnabled() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEnabled( ::sal_Bool _enabled ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Controls( const css::uno::Any& aIndex ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL Type(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL FindControl( const css::uno::Any& aType, const css::uno::Any& aId, const css::uno::Any& aTag, const css::uno::Any& aVisible, const css::uno::Any& aRecursive ) throw (css::script::BasicErrorException, css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

/** Dummy command bar implementation. Does nothing but provide its name. */
class VbaDummyCommandBar : public CommandBar_BASE
{
public:
    VbaDummyCommandBar(
            const css::uno::Reference< ov::XHelperInterface > xParent,
            const css::uno::Reference< css::uno::XComponentContext > xContext,
            const ::rtl::OUString& rName,
            sal_Int32 nType ) throw( css::uno::RuntimeException );

    // Attributes
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& _name ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( ::sal_Bool _visible ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getEnabled() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEnabled( ::sal_Bool _enabled ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Controls( const css::uno::Any& aIndex ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL Type(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL FindControl( const css::uno::Any& aType, const css::uno::Any& aId, const css::uno::Any& aTag, const css::uno::Any& aVisible, const css::uno::Any& aRecursive ) throw (css::script::BasicErrorException, css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();

private:
    ::rtl::OUString maName;
    sal_Int32 mnType;
};

#endif//SC_VBA_COMMANDBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
