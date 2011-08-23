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
#ifndef _SD_UNOGSFM_HXX_
#define _SD_UNOGSFM_HXX_

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#include <comphelper/servicehelper.hxx>

#ifndef _SFXLSTNER_HXX
#include <bf_svtools/lstner.hxx>
#endif

#include <cppuhelper/implbase5.hxx>

#include <unomodel.hxx>
namespace binfilter {

class SfxStyleSheetBase;
class SvUnoWeakContainer;
class SdPage;
class SdUnoGraphicStyle;

class SdUnoGraphicStyleFamily : public ::cppu::WeakImplHelper5<
                                             ::com::sun::star::container::XNameContainer,
                                             ::com::sun::star::container::XIndexAccess,
                                             ::com::sun::star::lang::XSingleServiceFactory,
                                             ::com::sun::star::lang::XUnoTunnel,
                                             ::com::sun::star::lang::XServiceInfo >
{
private:
    SvUnoWeakContainer*	mpStyles;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > mxModel;
    SdXImpressDocument*	mpModel;

    SdUnoGraphicStyle* getStyleFromAny( const ::com::sun::star::uno::Any& Element ) throw( ::com::sun::star::lang::IllegalArgumentException  );

public:
    SdUnoGraphicStyleFamily( SdXImpressDocument* pModel ) throw();
    ~SdUnoGraphicStyleFamily() throw();

    void createStyle( SfxStyleSheetBase* pStyleSheet, ::com::sun::star::uno::Any &rAny ) const throw();

    static String getInternalStyleName( const ::rtl::OUString& rName ) throw();
    static ::rtl::OUString getExternalStyleName( const String& rName ) throw();

    UNO3_GETIMPLEMENTATION_DECL( SdUnoGraphicStyleFamily )

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

    // XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException); 
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException); 
    
    // XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XSingleServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance(  ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
};

} //namespace binfilter
#endif

