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

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <bf_svtools/lstner.hxx>

#include <comphelper/servicehelper.hxx>

#include <cppuhelper/implbase4.hxx>

#include "unomodel.hxx"
#include "prlayout.hxx"
namespace binfilter {

class SfxStyleSheet;
class SfxStyleSheetBase;

class SvUnoWeakContainer;
class SdPage;

class SdUnoPseudoStyleFamily : public ::cppu::WeakImplHelper4<
                                    ::com::sun::star::container::XNameAccess,
                                    ::com::sun::star::container::XIndexAccess,
                                    ::com::sun::star::lang::XUnoTunnel,
                                    ::com::sun::star::lang::XServiceInfo >,
                               public SfxListener
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > mxModel;
    SdXImpressDocument*   mpModel;
    SdPage*				  mpPage;

    SvUnoWeakContainer*	mpStyles;

public:
    SdUnoPseudoStyleFamily( SdXImpressDocument* pModel, SdPage* pPage ) throw();
    ~SdUnoPseudoStyleFamily() throw();

    static ::rtl::OUString getExternalStyleName( const String& rStyleName ) throw();

    void createStyle( sal_uInt16 nIndex, ::com::sun::star::uno::Any& rAny ) throw();
    void createStyle( SfxStyleSheetBase* pStyleSheet, ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle > & rxRef ) throw();
    void createStyle( SfxStyleSheetBase* pStyleSheet, PresentationObjects eObject, ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle > & rxRef ) throw();

    sal_Bool isValid() const throw() { return mxModel.is() && mpModel->GetDoc() != NULL; }
    SdPage* getPage() const throw() { return mpPage; }

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    UNO3_GETIMPLEMENTATION_DECL( SdUnoPseudoStyleFamily )

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};

} //namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
