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

#include <svl/lstner.hxx>
#include <svl/style.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleLoader2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>

namespace com::sun::star::container { class XIndexReplace; }
namespace com::sun::star::lang { class XComponent; }

class ScDocShell;
class SfxItemPropertySet;
class SfxItemSet;

class ScStyleFamilyObj;
class ScStyleObj;
struct SfxItemPropertySimpleEntry;

class ScStyleFamiliesObj final : public ::cppu::WeakImplHelper<
                            css::container::XIndexAccess,
                            css::container::XNameAccess,
                            css::style::XStyleLoader2,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

    rtl::Reference<ScStyleFamilyObj> GetObjectByType_Impl(SfxStyleFamily nType) const;
    rtl::Reference<ScStyleFamilyObj> GetObjectByIndex_Impl(sal_uInt32 nIndex) const;
    rtl::Reference<ScStyleFamilyObj> GetObjectByName_Impl(std::u16string_view aName) const;

public:
                            ScStyleFamiliesObj(ScDocShell* pDocSh);
    virtual                 ~ScStyleFamiliesObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XStyleLoader
    virtual void SAL_CALL   loadStylesFromURL( const OUString& URL,
                                const css::uno::Sequence< css::beans::PropertyValue >& aOptions ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                            getStyleLoaderOptions() override;

                            //XStyleLoader2
    virtual void SAL_CALL   loadStylesFromDocument( const css::uno::Reference < css::lang::XComponent > & aSourceComponent,
                                const css::uno::Sequence< css::beans::PropertyValue >& aOptions ) override;
                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
private:
    /// @throws css::io::IOException
    /// @throws css::uno::RuntimeException
    void loadStylesFromDocShell( ScDocShell* pSource,
                              const css::uno::Sequence< css::beans::PropertyValue>& aOptions );
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

    rtl::Reference<ScStyleObj> GetObjectByIndex_Impl(sal_Int32 nIndex);
    rtl::Reference<ScStyleObj> GetObjectByName_Impl(const OUString& Name);

public:
                            ScStyleFamilyObj(ScDocShell* pDocSh, SfxStyleFamily eFam);
    virtual                 ~ScStyleFamilyObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XNameContainer
    virtual void SAL_CALL   insertByName( const OUString& aName,
                                const css::uno::Any& aElement ) override;
    virtual void SAL_CALL   removeByName( const OUString& Name ) override;

                            // XNameReplace
    virtual void SAL_CALL   replaceByName( const OUString& aName,
                                const css::uno::Any& aElement ) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
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
    SfxStyleSheetBase*      pStyle_cached;

    SfxStyleSheetBase*      GetStyle_Impl( bool bUseCachedValue = false );
    const SfxItemSet*       GetStyleItemSet_Impl( std::u16string_view rPropName, const SfxItemPropertySimpleEntry*& rpEntry );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    css::beans::PropertyState getPropertyState_Impl( std::u16string_view PropertyName );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    css::uno::Any           getPropertyDefault_Impl( std::u16string_view aPropertyName );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    css::uno::Any           getPropertyValue_Impl( std::u16string_view aPropertyName );
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    void                    setPropertyValue_Impl( std::u16string_view rPropertyName,
                                                 const SfxItemPropertySimpleEntry* pEntry,
                                                 const css::uno::Any* pValue );

public:
                            ScStyleObj() = delete;
                            ScStyleObj(ScDocShell* pDocSh, SfxStyleFamily eFam, const OUString& rName);
    virtual                 ~ScStyleObj() override;

                            // created by getImplementation:
    bool                    IsInserted() const      { return pDocShell != nullptr; }
    SfxStyleFamily          GetFamily() const       { return eFamily; }
    void                    InitDoc( ScDocShell* pNewDocSh, const OUString& rNewName );

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    static css::uno::Reference< css::container::XIndexReplace >
                            CreateEmptyNumberingRules();

                            // XStyle
    virtual sal_Bool SAL_CALL isUserDefined() override;
    virtual sal_Bool SAL_CALL isInUse() override;
    virtual OUString SAL_CALL getParentStyle() override;
    virtual void SAL_CALL   setParentStyle( const OUString& aParentStyle ) override;

                            // XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL   setName( const OUString& aName ) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference<
                                        css::beans::XVetoableChangeListener >& aListener ) override;

                            // XMultiPropertySet
    virtual void SAL_CALL   setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames,
                                    const css::uno::Sequence< css::uno::Any >& aValues ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL
                            getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames,
                                    const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames,
                                    const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

                            // XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState(
                                    const OUString& PropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL
                            getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void SAL_CALL   setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

                            // XMultiPropertyStates
    // getPropertyStates already defined for XPropertyState
    virtual void SAL_CALL   setAllPropertiesToDefault() override;
    virtual void SAL_CALL   setPropertiesToDefault( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL
                            getPropertyDefaults( const css::uno::Sequence< OUString >& aPropertyNames ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

                            // XUnoTunnel
    UNO3_GETIMPLEMENTATION_DECL(ScStyleObj)
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
