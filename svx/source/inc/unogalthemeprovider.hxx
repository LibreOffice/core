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

#ifndef INCLUDED_SVX_SOURCE_INC_UNOGALTHEMEPROVIDER_HXX
#define INCLUDED_SVX_SOURCE_INC_UNOGALTHEMEPROVIDER_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/gallery/XGalleryThemeProvider.hpp>

class Gallery;

namespace {

class GalleryThemeProvider : public ::cppu::WeakImplHelper< ::com::sun::star::lang::XInitialization,
                                                             ::com::sun::star::gallery::XGalleryThemeProvider,
                                                             ::com::sun::star::lang::XServiceInfo >
{
public:

    GalleryThemeProvider();
    virtual ~GalleryThemeProvider();

protected:

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw ( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XGalleryThemeProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::gallery::XGalleryTheme > SAL_CALL insertNewByName( const OUString& ThemeName ) throw (::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByName( const OUString& ThemeName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception) override;

private:

    Gallery*    mpGallery;
    bool    mbHiddenThemes;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
