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
#ifndef _SD_UNOCPRES_HXX
#define _SD_UNOCPRES_HXX

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <osl/mutex.hxx>

#include <cppuhelper/implbase5.hxx>
#include <comphelper/servicehelper.hxx>

///////////////////////////////////////////////////////////////////////////////

class SdXImpressDocument;
class SdCustomShow;

class SdXCustomPresentation :   public ::cppu::WeakImplHelper5< ::com::sun::star::container::XIndexContainer,
                                                                ::com::sun::star::container::XNamed,
                                                                ::com::sun::star::lang::XUnoTunnel,
                                                                ::com::sun::star::lang::XComponent,
                                                                ::com::sun::star::lang::XServiceInfo >
{
private:
    SdCustomShow*       mpSdCustomShow;
    SdXImpressDocument* mpModel;

    // for xComponent
    ::osl::Mutex aDisposeContainerMutex;
    ::cppu::OInterfaceContainerHelper aDisposeListeners;
    sal_Bool bDisposing;

public:
    SdXCustomPresentation() throw();
    SdXCustomPresentation( SdCustomShow* mpSdCustomShow, SdXImpressDocument* pMyModel) throw();
    virtual ~SdXCustomPresentation() throw();

    // internal
    void Invalidate() { mpSdCustomShow = NULL; }
    SdCustomShow* GetSdCustomShow() const throw() { return mpSdCustomShow; }
    void SetSdCustomShow( SdCustomShow* pShow ) throw() { mpSdCustomShow = pShow; }
    SdXImpressDocument* GetModel() const throw() { return mpModel; }

    // uno helper
    UNO3_GETIMPLEMENTATION_DECL(SdXCustomPresentation)

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XIndexContainer
    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XNamed
    virtual ::rtl::OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
};

// --------------------------------------------------------------------------
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <cppuhelper/implbase3.hxx>

#include "unomodel.hxx"
#include "drawdoc.hxx"

class List;

class SdXCustomPresentationAccess : public ::cppu::WeakImplHelper3< ::com::sun::star::container::XNameContainer,
                                                                    ::com::sun::star::lang::XSingleServiceFactory,
                                                                    ::com::sun::star::lang::XServiceInfo >
{
private:
    SdXImpressDocument& mrModel;

    // intern
    inline SdCustomShowList* GetCustomShowList() const throw();
    SdCustomShow * getSdCustomShow( const ::rtl::OUString& Name ) const throw();

public:
    SdXCustomPresentationAccess(SdXImpressDocument& rMyModel) throw();
    ~SdXCustomPresentationAccess() throw();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XSingleServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance(  ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);
};

inline SdCustomShowList* SdXCustomPresentationAccess::GetCustomShowList() const throw()
{
    if(mrModel.GetDoc())
        return mrModel.GetDoc()->GetCustomShowList(sal_False);
    else
        return NULL;
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
