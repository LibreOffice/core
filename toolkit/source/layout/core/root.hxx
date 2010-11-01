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

#ifndef LAYOUT_CORE_ROOT_HXX
#define LAYOUT_CORE_ROOT_HXX

#define _BACKWARD_BACKWARD_WARNING_H 1
#include <hash_map>

#include <com/sun/star/awt/XLayoutRoot.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <toolkit/dllapi.h>

#include <layout/core/proplist.hxx>

namespace layoutimpl
{

namespace css = ::com::sun::star;

css::uno::Reference< css::io::XInputStream > getFileAsStream( const rtl::OUString &rName );

/* Interface generation code -- to hook to a parser. */

/*
  TODO: (ricardo) I think we should cut on LayoutRoot, stripping out its widget
  proxy interface (just make it return the root widget). Would even make it easier
  if there was interest to support multiple toplevel widgets in the same file.

  We also need to make sure the code gets diposed well... There is no need to keep
  these objects around after initialization...
*/


class LayoutWidget;

class TOOLKIT_DLLPUBLIC LayoutRoot : public ::cppu::WeakImplHelper3<
    css::awt::XLayoutRoot,
    css::lang::XInitialization,
    css::lang::XComponent>
{
protected:
    ::osl::Mutex maMutex;

    typedef std::hash_map< rtl::OUString,
                           css::uno::Reference< css::awt::XLayoutConstrains >,
                           ::rtl::OUStringHash > ItemHash;
    ItemHash maItems;

    sal_Bool mbDisposed;
    css::uno::Reference< css::lang::XMultiServiceFactory > mxFactory;
    ::cppu::OInterfaceContainerHelper *mpListeners;

    css::uno::Reference< css::awt::XWindow >          mxWindow;
    css::uno::Reference< css::awt::XLayoutContainer > mxContainer;

    css::uno::Reference< css::awt::XToolkit > mxToolkit;
    LayoutWidget *mpToplevel;
    css::uno::Reference< css::awt::XLayoutUnit > mxLayoutUnit;

    void error( rtl::OUString const& message );

public:
    LayoutRoot( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
    virtual ~LayoutRoot();

    void addItem( const rtl::OUString &rName,
                  const css::uno::Reference< css::awt::XLayoutConstrains > &xRef );

    void setWindow(    css::uno::Reference< css::awt::XLayoutConstrains > xPeer )
    {
        mxWindow = css::uno::Reference< css::awt::XWindow >( xPeer, css::uno::UNO_QUERY );
    }

    // get XLayoutContainer
    virtual css::uno::Reference< css::awt::XLayoutContainer > SAL_CALL getLayoutContainer() throw (css::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException);

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException);
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException);

    // generator
    virtual LayoutWidget *create( rtl::OUString id, const rtl::OUString unoName, long attrbs, css::uno::Reference< css::awt::XLayoutContainer > xParent );
};

class TOOLKIT_DLLPUBLIC LayoutWidget
{
    friend class LayoutRoot;

public:
    LayoutWidget() {}
    LayoutWidget( css::uno::Reference< css::awt::XToolkit > xToolkit,
                  css::uno::Reference< css::awt::XLayoutContainer > xToplevel,
                  rtl::OUString unoName, long attrbs );
    virtual ~LayoutWidget();

    virtual void setProperties( const PropList &rProps );
    virtual void setProperty( rtl::OUString const& attr, rtl::OUString const& value );

    virtual bool addChild( LayoutWidget *pChild );
    virtual void setChildProperties( LayoutWidget *pChild, const PropList &rProps );

    inline css::uno::Reference< css::awt::XLayoutConstrains > getPeer()
    { return mxWidget; }
    inline css::uno::Reference< css::awt::XLayoutContainer > getContainer()
    { return mxContainer; }

protected:
    css::uno::Reference< css::awt::XLayoutConstrains > mxWidget;
    css::uno::Reference< css::awt::XLayoutContainer > mxContainer;
};

} // namespace layoutimpl

#endif /* LAYOUT_CORE_ROOT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
