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

#ifndef SC_STYLEUNO_HXX
#define SC_STYLEUNO_HXX

#include <svl/itemprop.hxx>
#include <svl/lstner.hxx>
#include <rsc/rscsfx.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleLoader.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase7.hxx>

class SfxStyleSheetBase;
class ScDocShell;

class ScStyleFamilyObj;
class ScStyleObj;

class ScStyleFamiliesObj : public ::cppu::WeakImplHelper4<
                            ::com::sun::star::container::XIndexAccess,
                            ::com::sun::star::container::XNameAccess,
                            ::com::sun::star::style::XStyleLoader,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

    ScStyleFamilyObj*       GetObjectByType_Impl(sal_uInt16 Type) const;
    ScStyleFamilyObj*       GetObjectByIndex_Impl(sal_uInt32 nIndex) const;
    ScStyleFamilyObj*       GetObjectByName_Impl(const OUString& aName) const;

public:
                            ScStyleFamiliesObj(ScDocShell* pDocSh);
    virtual                 ~ScStyleFamiliesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception);

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception);

                            // XStyleLoader
    virtual void SAL_CALL   loadStylesFromURL( const OUString& URL,
                                const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::beans::PropertyValue >& aOptions )
                                        throw(::com::sun::star::io::IOException,
                                            ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            getStyleLoaderOptions() throw(::com::sun::star::uno::RuntimeException, std::exception);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
};


class ScStyleFamilyObj : public ::cppu::WeakImplHelper4<
                            ::com::sun::star::container::XNameContainer,
                            ::com::sun::star::container::XIndexAccess,
                            ::com::sun::star::beans::XPropertySet,
                            ::com::sun::star::lang::XServiceInfo >,
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

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XNameContainer
    virtual void SAL_CALL   insertByName( const OUString& aName,
                                const ::com::sun::star::uno::Any& aElement )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::container::ElementExistException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL   removeByName( const OUString& Name )
                                throw(::com::sun::star::container::NoSuchElementException,
                                      ::com::sun::star::lang::WrappedTargetException,
                                      ::com::sun::star::uno::RuntimeException,
                                      std::exception);

                            // XNameReplace
    virtual void SAL_CALL   replaceByName( const OUString& aName,
                                const ::com::sun::star::uno::Any& aElement )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception);

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
                                throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
                                throw (::com::sun::star::beans::UnknownPropertyException,
                                       ::com::sun::star::beans::PropertyVetoException,
                                       ::com::sun::star::lang::IllegalArgumentException,
                                       ::com::sun::star::lang::WrappedTargetException,
                                       ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
                                throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
};


class ScStyleObj : public ::cppu::WeakImplHelper7<
                    ::com::sun::star::style::XStyle,
                    ::com::sun::star::beans::XPropertySet,
                    ::com::sun::star::beans::XMultiPropertySet,
                    ::com::sun::star::beans::XPropertyState,
                    ::com::sun::star::beans::XMultiPropertyStates,
                    ::com::sun::star::lang::XUnoTunnel,
                    ::com::sun::star::lang::XServiceInfo >,
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
                                                    const ::com::sun::star::uno::Any* pValue )
                                            throw(::com::sun::star::lang::IllegalArgumentException,
                                                  ::com::sun::star::uno::RuntimeException,
                                                  std::exception);

    ScStyleObj(); // disabled
public:
                            ScStyleObj(ScDocShell* pDocSh, SfxStyleFamily eFam, const OUString& rName);
    virtual                 ~ScStyleObj();

                            // created by getImplementation:
    bool                    IsInserted() const      { return pDocShell != NULL; }
    SfxStyleFamily          GetFamily() const       { return eFamily; }
    void                    InitDoc( ScDocShell* pNewDocSh, const OUString& rNewName );

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    static ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace >
                            CreateEmptyNumberingRules();

                            // XStyle
    virtual sal_Bool SAL_CALL isUserDefined() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL isInUse() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getParentStyle() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL   setParentStyle( const OUString& aParentStyle )
                                throw(::com::sun::star::container::NoSuchElementException,
                                      ::com::sun::star::uno::RuntimeException,
                                      std::exception);

                            // XNamed
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException,
                                    std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception);

                            // XMultiPropertySet
    virtual void SAL_CALL   setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames,
                                    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues )
                                throw (::com::sun::star::beans::PropertyVetoException,
                                       ::com::sun::star::lang::IllegalArgumentException,
                                       ::com::sun::star::lang::WrappedTargetException,
                                       ::com::sun::star::uno::RuntimeException,
                                       std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL
                            getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames )
                                throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
                                throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
                                throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
                                throw (::com::sun::star::uno::RuntimeException, std::exception);

                            // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL
                            getPropertyStates( const ::com::sun::star::uno::Sequence<
                                        OUString >& aPropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL   setPropertyToDefault( const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                      ::com::sun::star::uno::RuntimeException,
                                      std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault(
                                    const OUString& aPropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception);

                            // XMultiPropertyStates
    // getPropertyStates already defined for XPropertyState
    virtual void SAL_CALL   setAllPropertiesToDefault()
                                throw (::com::sun::star::uno::RuntimeException,
                                       std::exception);
    virtual void SAL_CALL   setPropertiesToDefault( const ::com::sun::star::uno::Sequence<
                                        OUString >& aPropertyNames )
                                throw (::com::sun::star::beans::UnknownPropertyException,
                                       ::com::sun::star::uno::RuntimeException,
                                       std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL
                            getPropertyDefaults( const ::com::sun::star::uno::Sequence<
                                        OUString >& aPropertyNames )
                                throw (::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception);

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException, std::exception);

    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScStyleObj* getImplementation( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::uno::XInterface> xObj );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
