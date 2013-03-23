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

#pragma once
#if 1

#include "address.hxx"
#include "mutexhlp.hxx"

#include <svl/lstner.hxx>
#include <svl/itemprop.hxx>
#include <editeng/editdata.hxx>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <cppuhelper/component.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/implbase4.hxx>
#include <osl/mutex.hxx>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

class ScEditSource;
class SvxFieldItem;
class SvxFieldData;
class ScEditFieldObj;
class ScDocShell;
class ScHeaderFooterTextData;

class ScCellFieldsObj : public cppu::WeakImplHelper5<
                            com::sun::star::container::XEnumerationAccess,
                            com::sun::star::container::XIndexAccess,
                            com::sun::star::container::XContainer,
                            com::sun::star::util::XRefreshable,
                            com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    com::sun::star::uno::Reference<com::sun::star::text::XTextRange> mxContent;
    ScDocShell*             pDocShell;
    ScAddress               aCellPos;
    ScEditSource* mpEditSource;
    /// List of refresh listeners.
    cppu::OInterfaceContainerHelper* mpRefreshListeners;
    /// mutex to lock the InterfaceContainerHelper
    osl::Mutex              aMutex;

    com::sun::star::uno::Reference<
        com::sun::star::text::XTextField>
            GetObjectByIndex_Impl(sal_Int32 Index) const;

public:
    ScCellFieldsObj(
        const com::sun::star::uno::Reference<com::sun::star::text::XTextRange>& xContent,
        ScDocShell* pDocSh, const ScAddress& rPos);
    virtual ~ScCellFieldsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XContainer
    virtual void SAL_CALL   addContainerListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::container::XContainerListener >& xListener )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeContainerListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::container::XContainerListener >& xListener )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XRefreshable
    virtual void SAL_CALL refresh(  )
                                    throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addRefreshListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::util::XRefreshListener >& l )
                                    throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRefreshListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::util::XRefreshListener >& l )
                                    throw (::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};

class ScHeaderFieldsObj : public cppu::WeakImplHelper5<
                            com::sun::star::container::XEnumerationAccess,
                            com::sun::star::container::XIndexAccess,
                            com::sun::star::container::XContainer,
                            com::sun::star::util::XRefreshable,
                            com::sun::star::lang::XServiceInfo >
{
private:
    ScHeaderFooterTextData& mrData;
    ScEditSource* mpEditSource;

    /// List of refresh listeners.
    cppu::OInterfaceContainerHelper* mpRefreshListeners;
    /// mutex to lock the InterfaceContainerHelper
    osl::Mutex                  aMutex;

    com::sun::star::uno::Reference<
        com::sun::star::text::XTextField>
            GetObjectByIndex_Impl(sal_Int32 Index) const;

public:
    ScHeaderFieldsObj(ScHeaderFooterTextData& rData);
    virtual                 ~ScHeaderFieldsObj();

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XContainer
    virtual void SAL_CALL   addContainerListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::container::XContainerListener >& xListener )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeContainerListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::container::XContainerListener >& xListener )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XRefreshable
    virtual void SAL_CALL refresh(  )
                                    throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addRefreshListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::util::XRefreshListener >& l )
                                    throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRefreshListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::util::XRefreshListener >& l )
                                    throw (::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};

/**
 * Generic UNO wrapper for edit engine's field item in cells, headers, and
 * footers.
 */
class ScEditFieldObj : public cppu::WeakImplHelper4<
                            com::sun::star::text::XTextField,
                            com::sun::star::beans::XPropertySet,
                            com::sun::star::lang::XUnoTunnel,
                            com::sun::star::lang::XServiceInfo>,
                        public ScMutexHelper,
                        public ::cppu::OComponentHelper,
                        private boost::noncopyable
{
    const SfxItemPropertySet* pPropSet;
    ScEditSource* mpEditSource;
    ESelection aSelection;

    sal_Int32 meType;
    boost::scoped_ptr<SvxFieldData> mpData;
    com::sun::star::uno::Reference<com::sun::star::text::XTextRange> mpContent;

    com::sun::star::util::DateTime maDateTime;
    sal_Int32 mnNumFormat;
    bool mbIsDate:1;
    bool mbIsFixed:1;

private:
    ScEditFieldObj(); // disabled

    SvxFieldData* getData();

    void setPropertyValueURL(const rtl::OUString& rName, const com::sun::star::uno::Any& rVal);
    com::sun::star::uno::Any getPropertyValueURL(const rtl::OUString& rName);

    void setPropertyValueFile(const rtl::OUString& rName, const com::sun::star::uno::Any& rVal);
    com::sun::star::uno::Any getPropertyValueFile(const rtl::OUString& rName);

    void setPropertyValueDateTime(const rtl::OUString& rName, const com::sun::star::uno::Any& rVal);
    com::sun::star::uno::Any getPropertyValueDateTime(const rtl::OUString& rName);

    void setPropertyValueSheet(const rtl::OUString& rName, const com::sun::star::uno::Any& rVal);

public:
    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScEditFieldObj* getImplementation(const com::sun::star::uno::Reference<com::sun::star::text::XTextContent>& xObj);

    ScEditFieldObj(
        const com::sun::star::uno::Reference<com::sun::star::text::XTextRange>& rContent,
        ScEditSource* pEditSrc, sal_Int32 eType, const ESelection& rSel);
    virtual ~ScEditFieldObj();

    sal_Int32 GetFieldType() const;
    void DeleteField();
    bool IsInserted() const;
    SvxFieldItem CreateFieldItem();
    void InitDoc(
        const com::sun::star::uno::Reference<com::sun::star::text::XTextRange>& rContent,
        ScEditSource* pEditSrc, const ESelection& rSel);

                            // XTextField
    virtual ::rtl::OUString SAL_CALL getPresentation( sal_Bool bShowCommand )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XTextContent
    virtual void SAL_CALL attach( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::text::XTextRange >& xTextRange )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                            ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getAnchor() throw(::com::sun::star::uno::RuntimeException);

                            // XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::lang::XEventListener >& xListener )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::lang::XEventListener >& aListener )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
