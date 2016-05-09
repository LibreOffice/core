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

#ifndef INCLUDED_SC_INC_STYLEUNO_HXX
#define INCLUDED_SC_INC_STYLEUNO_HXX

#include <svl/itemprop.hxx>
#include <svl/lstner.hxx>
#include <rsc/rscsfx.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleLoader2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XComponent.hpp>


class SfxStyleSheetBase;
class ScDocShell;
class SfxObjectShell;

class ScStyleFamilyObj;
class ScStyleObj;

class ScStyleFamiliesObj : public ::cppu::WeakImplHelper<
                            css::container::XIndexAccess,
                            css::container::XNameAccess,
                            css::style::XStyleLoader2,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

    ScStyleFamilyObj*       GetObjectByType_Impl(SfxStyleFamily nType) const;
    ScStyleFamilyObj*       GetObjectByIndex_Impl(sal_uInt32 nIndex) const;
    ScStyleFamilyObj*       GetObjectByName_Impl(const OUString& aName) const;

public:
                            ScStyleFamiliesObj(ScDocShell* pDocSh);
    virtual                 ~ScStyleFamiliesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XStyleLoader
    virtual void SAL_CALL   loadStylesFromURL( const OUString& URL,
                                const css::uno::Sequence< css::beans::PropertyValue >& aOptions )
                                        throw(css::io::IOException,
                                            css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                            getStyleLoaderOptions() throw(css::uno::RuntimeException, std::exception) override;

                            //XStyleLoader2
    virtual void SAL_CALL   loadStylesFromDocument( const css::uno::Reference < css::lang::XComponent > & aSourceComponent,
                                const css::uno::Sequence< css::beans::PropertyValue >& aOptions )
                                        throw(css::io::IOException,
                                            css::uno::RuntimeException, std::exception) override;
                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
private:
    void loadStylesFromDocShell( ScDocShell* pSource,
                              const css::uno::Sequence< css::beans::PropertyValue>& aOptions )
                                throw(css::io::IOException,
                                            css::uno::RuntimeException, std::exception);
};

class ScStyleFamilyObj : public ::cppu::WeakImplHelper<
                            css::container::XNameContainer,
                            css::container::XIndexAccess,
                            css::beans::XPropertySet,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;
    SfxStyleFamily          eFamily;        // Family

    ScStyleObj*             GetObjectByIndex_Impl(sal_uInt32 nIndex);
    ScStyleObj*             GetObjectByName_Impl(const OUString& Name);

public:
                            ScStyleFamilyObj(ScDocShell* pDocSh, SfxStyleFamily eFam);
    virtual                 ~ScStyleFamilyObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XNameContainer
    virtual void SAL_CALL   insertByName( const OUString& aName,
                                const css::uno::Any& aElement )
                                    throw(css::lang::IllegalArgumentException,
                                        css::container::ElementExistException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeByName( const OUString& Name )
                                throw(css::container::NoSuchElementException,
                                      css::lang::WrappedTargetException,
                                      css::uno::RuntimeException,
                                      std::exception) override;

                            // XNameReplace
    virtual void SAL_CALL   replaceByName( const OUString& aName,
                                const css::uno::Any& aElement )
                                    throw(css::lang::IllegalArgumentException,
                                        css::container::NoSuchElementException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue )
                                throw (css::beans::UnknownPropertyException,
                                       css::beans::PropertyVetoException,
                                       css::lang::IllegalArgumentException,
                                       css::lang::WrappedTargetException,
                                       css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
                                throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScStyleObj : public ::cppu::WeakImplHelper<
                    css::style::XStyle,
                    css::beans::XPropertySet,
                    css::beans::XMultiPropertySet,
                    css::beans::XPropertyState,
                    css::beans::XMultiPropertyStates,
                    css::lang::XUnoTunnel,
                    css::lang::XServiceInfo >,
                public SfxListener
{
private:
    const SfxItemPropertySet* pPropSet;
    ScDocShell*             pDocShell;
    SfxStyleFamily          eFamily;        // Family
    OUString                aStyleName;

    SfxStyleSheetBase*      GetStyle_Impl();
    const SfxItemSet*       GetStyleItemSet_Impl( const OUString& rPropName, const SfxItemPropertySimpleEntry*& rpEntry );
    void                    SetOnePropertyValue(    const OUString& rPropertyName,
                                                    const SfxItemPropertySimpleEntry* pEntry,
                                                    const css::uno::Any* pValue )
                                            throw(css::lang::IllegalArgumentException,
                                                  css::uno::RuntimeException,
                                                  std::exception);

public:
                            ScStyleObj() = delete;
                            ScStyleObj(ScDocShell* pDocSh, SfxStyleFamily eFam, const OUString& rName);
    virtual                 ~ScStyleObj();

                            // created by getImplementation:
    bool                    IsInserted() const      { return pDocShell != nullptr; }
    SfxStyleFamily          GetFamily() const       { return eFamily; }
    void                    InitDoc( ScDocShell* pNewDocSh, const OUString& rNewName );

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    static css::uno::Reference< css::container::XIndexReplace >
                            CreateEmptyNumberingRules();

                            // XStyle
    virtual sal_Bool SAL_CALL isUserDefined() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isInUse() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getParentStyle() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setParentStyle( const OUString& aParentStyle )
                                throw(css::container::NoSuchElementException,
                                      css::uno::RuntimeException,
                                      std::exception) override;

                            // XNamed
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException,
                                    std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XMultiPropertySet
    virtual void SAL_CALL   setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames,
                                    const css::uno::Sequence< css::uno::Any >& aValues )
                                throw (css::beans::PropertyVetoException,
                                       css::lang::IllegalArgumentException,
                                       css::lang::WrappedTargetException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL
                            getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames )
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames,
                                    const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener )
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener )
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames,
                                    const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener )
                                throw (css::uno::RuntimeException, std::exception) override;

                            // XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState(
                                    const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL
                            getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyToDefault( const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                      css::uno::RuntimeException,
                                      std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XMultiPropertyStates
    // getPropertyStates already defined for XPropertyState
    virtual void SAL_CALL   setAllPropertiesToDefault()
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   setPropertiesToDefault( const css::uno::Sequence< OUString >& aPropertyNames )
                                throw (css::beans::UnknownPropertyException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL
                            getPropertyDefaults( const css::uno::Sequence< OUString >& aPropertyNames )
                                throw (css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
                                throw(css::uno::RuntimeException, std::exception) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScStyleObj* getImplementation(const css::uno::Reference<css::uno::XInterface>& rObj);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
