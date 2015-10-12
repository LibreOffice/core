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

#ifndef INCLUDED_SD_INC_STLSHEET_HXX
#define INCLUDED_SD_INC_STLSHEET_HXX

#include <rtl/ref.hxx>

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <svl/style.hxx>

#include <editeng/unoipset.hxx>

#include <memory>

#include "prlayout.hxx"

class ModifyListenerForewarder;

typedef cppu::ImplInheritanceHelper< SfxUnoStyleSheet,
                                    ::com::sun::star::beans::XPropertySet,
                                    ::com::sun::star::lang::XServiceInfo,
                                    ::com::sun::star::beans::XPropertyState,
                                    ::com::sun::star::util::XModifyBroadcaster,
                                    ::com::sun::star::lang::XComponent > SdStyleSheetBase ;

class SdStyleSheet : public SdStyleSheetBase, private ::cppu::BaseMutex
{
public:
    SdStyleSheet( const OUString& rDisplayName, SfxStyleSheetBasePool& rPool, SfxStyleFamily eFamily, sal_uInt16 nMask );
    SdStyleSheet( const SdStyleSheet& );

    virtual bool        SetParent (const OUString& rParentName) SAL_OVERRIDE;
    virtual SfxItemSet& GetItemSet() SAL_OVERRIDE;
    virtual bool        IsUsed() const SAL_OVERRIDE;
    virtual bool        HasFollowSupport() const SAL_OVERRIDE;
    virtual bool        HasParentSupport() const SAL_OVERRIDE;
    virtual bool        HasClearParentSupport() const SAL_OVERRIDE;
    virtual bool        SetName(const OUString& rNewName, bool bReindexNow = true) SAL_OVERRIDE;
    virtual void        SetHelpId( const OUString& r, sal_uLong nId ) SAL_OVERRIDE;

    void        AdjustToFontHeight(SfxItemSet& rSet, bool bOnlyMissingItems = true);

    SdStyleSheet* GetRealStyleSheet() const;
    SdStyleSheet* GetPseudoStyleSheet() const;

    void SetApiName( const OUString& rApiName );
    OUString GetApiName() const;

    static OUString GetFamilyString( SfxStyleFamily eFamily );

    static SdStyleSheet* CreateEmptyUserStyle( SfxStyleSheetBasePool& rPool, SfxStyleFamily eFamily );

    //Broadcast that a SdStyleSheet has changed, taking into account outline sublevels
    //which need to be explicitly broadcast as changing if their parent style was
    //the one that changed
    static void BroadcastSdStyleSheetChange(SfxStyleSheetBase* pStyleSheet, PresentationObjects ePO,
        SfxStyleSheetBasePool* pSSPool);

    // XInterface
    virtual void SAL_CALL release(  ) throw () SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XNamed
    virtual OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XStyle
    virtual sal_Bool SAL_CALL isUserDefined(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isInUse(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getParentStyle(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setParentStyle( const OUString& aParentStyle ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    void notifyModifyListener();

protected:
    static const SfxItemPropertySimpleEntry* getPropertyMapEntry( const OUString& rPropertyName ) throw (css::uno::RuntimeException);

    virtual void Load (SvStream& rIn, sal_uInt16 nVersion) SAL_OVERRIDE;
    virtual void Store(SvStream& rOut) SAL_OVERRIDE;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) SAL_OVERRIDE;
    virtual             ~SdStyleSheet();

    void throwIfDisposed() throw (::com::sun::star::uno::RuntimeException);

    void disposing();

    OUString   msApiName;
    rtl::Reference< SfxStyleSheetBasePool > mxPool;

    /** broadcast helper for events */
    ::cppu::OBroadcastHelper mrBHelper;

    std::unique_ptr< ModifyListenerForewarder > mpModifyListenerForewarder;

private:
    SdStyleSheet& operator=( const SdStyleSheet& ) = delete;
};

typedef rtl::Reference< SdStyleSheet > SdStyleSheetRef;
typedef std::vector< SdStyleSheetRef > SdStyleSheetVector;

#endif // INCLUDED_SD_INC_STLSHEET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
