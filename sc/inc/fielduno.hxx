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

#ifndef INCLUDED_SC_INC_FIELDUNO_HXX
#define INCLUDED_SC_INC_FIELDUNO_HXX

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
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>

#include <boost/noncopyable.hpp>
#include <memory>

class ScEditSource;
class SvxFieldItem;
class SvxFieldData;
class ScEditFieldObj;
class ScDocShell;
class ScHeaderFooterTextData;

class ScCellFieldsObj : public cppu::WeakImplHelper<
                            css::container::XEnumerationAccess,
                            css::container::XIndexAccess,
                            css::container::XContainer,
                            css::util::XRefreshable,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    css::uno::Reference<css::text::XTextRange> mxContent;
    ScDocShell*             pDocShell;
    ScAddress               aCellPos;
    ScEditSource* mpEditSource;
    /// List of refresh listeners.
    cppu::OInterfaceContainerHelper* mpRefreshListeners;
    /// mutex to lock the InterfaceContainerHelper
    osl::Mutex              aMutex;

    css::uno::Reference<css::text::XTextField>
            GetObjectByIndex_Impl(sal_Int32 Index) const;

public:
    ScCellFieldsObj(
        const css::uno::Reference<css::text::XTextRange>& xContent,
        ScDocShell* pDocSh, const ScAddress& rPos);
    virtual ~ScCellFieldsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XContainer
    virtual void SAL_CALL   addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XRefreshable
    virtual void SAL_CALL refresh(  )
                                    throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
                                    throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
                                    throw (css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScHeaderFieldsObj : public cppu::WeakImplHelper<
                            css::container::XEnumerationAccess,
                            css::container::XIndexAccess,
                            css::container::XContainer,
                            css::util::XRefreshable,
                            css::lang::XServiceInfo >
{
private:
    ScHeaderFooterTextData& mrData;
    ScEditSource* mpEditSource;

    /// List of refresh listeners.
    cppu::OInterfaceContainerHelper* mpRefreshListeners;
    /// mutex to lock the InterfaceContainerHelper
    osl::Mutex                  aMutex;

    css::uno::Reference< css::text::XTextField>
            GetObjectByIndex_Impl(sal_Int32 Index) const;

public:
    ScHeaderFieldsObj(ScHeaderFooterTextData& rData);
    virtual                 ~ScHeaderFieldsObj();

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XContainer
    virtual void SAL_CALL   addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XRefreshable
    virtual void SAL_CALL refresh(  )
                                    throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
                                    throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l )
                                    throw (css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

/**
 * Generic UNO wrapper for edit engine's field item in cells, headers, and
 * footers.
 */
class ScEditFieldObj : public cppu::WeakImplHelper<
                            css::text::XTextField,
                            css::beans::XPropertySet,
                            css::lang::XUnoTunnel,
                            css::lang::XServiceInfo>,
                        public ScMutexHelper,
                        public ::cppu::OComponentHelper,
                        private boost::noncopyable
{
    const SfxItemPropertySet* pPropSet;
    ScEditSource* mpEditSource;
    ESelection aSelection;

    sal_Int32 meType;
    std::unique_ptr<SvxFieldData> mpData;
    css::uno::Reference<css::text::XTextRange> mpContent;

    css::util::DateTime maDateTime;
    sal_Int32 mnNumFormat;
    bool mbIsDate:1;
    bool mbIsFixed:1;

private:
    ScEditFieldObj(); // disabled

    SvxFieldData* getData();

    void setPropertyValueURL(const OUString& rName, const css::uno::Any& rVal);
    css::uno::Any getPropertyValueURL(const OUString& rName);

    void setPropertyValueFile(const OUString& rName, const css::uno::Any& rVal);
    css::uno::Any getPropertyValueFile(const OUString& rName);

    void setPropertyValueDateTime(const OUString& rName, const css::uno::Any& rVal);
    css::uno::Any getPropertyValueDateTime(const OUString& rName);

    void setPropertyValueSheet(const OUString& rName, const css::uno::Any& rVal);

public:
    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScEditFieldObj* getImplementation(const css::uno::Reference<css::text::XTextContent>& xObj);

    ScEditFieldObj(
        const css::uno::Reference<css::text::XTextRange>& rContent,
        ScEditSource* pEditSrc, sal_Int32 eType, const ESelection& rSel);
    virtual ~ScEditFieldObj();

    sal_Int32 GetFieldType() const { return meType;}
    void DeleteField();
    bool IsInserted() const;
    SvxFieldItem CreateFieldItem();
    void InitDoc(
        const css::uno::Reference<css::text::XTextRange>& rContent,
        ScEditSource* pEditSrc, const ESelection& rSel);

                            // XTextField
    virtual OUString SAL_CALL getPresentation( sal_Bool bShowCommand )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XTextContent
    virtual void SAL_CALL attach( const css::uno::Reference< css::text::XTextRange >& xTextRange )
                                    throw(css::lang::IllegalArgumentException,
                                            css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getAnchor() throw(css::uno::RuntimeException, std::exception) override;

                            // XComponent
    virtual void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw (css::beans::UnknownPropertyException,
                                       css::beans::PropertyVetoException,
                                       css::lang::IllegalArgumentException,
                                       css::lang::WrappedTargetException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
