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
                                    css::beans::XPropertySet,
                                    css::lang::XServiceInfo,
                                    css::beans::XPropertyState,
                                    css::util::XModifyBroadcaster,
                                    css::lang::XComponent > SdStyleSheetBase ;

class SdStyleSheet : public SdStyleSheetBase, private ::cppu::BaseMutex
{
public:
    SdStyleSheet( const OUString& rDisplayName, SfxStyleSheetBasePool& rPool, SfxStyleFamily eFamily, sal_uInt16 nMask );

    virtual bool        SetParent (const OUString& rParentName) override;
    virtual SfxItemSet& GetItemSet() override;
    virtual bool        IsUsed() const override;
    virtual bool        HasFollowSupport() const override;
    virtual bool        HasParentSupport() const override;
    virtual bool        HasClearParentSupport() const override;
    virtual void        SetHelpId( const OUString& r, sal_uLong nId ) override;

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
    virtual void SAL_CALL release(  ) throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XNamed
    virtual OUString SAL_CALL getName(  ) override;
    virtual void SAL_CALL setName( const OUString& aName ) override;

    // XStyle
    virtual sal_Bool SAL_CALL isUserDefined(  ) override;
    virtual sal_Bool SAL_CALL isInUse(  ) override;
    virtual OUString SAL_CALL getParentStyle(  ) override;
    virtual void SAL_CALL setParentStyle( const OUString& aParentStyle ) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    void notifyModifyListener();

protected:
    /// @throws css::uno::RuntimeException
    static const SfxItemPropertySimpleEntry* getPropertyMapEntry( const OUString& rPropertyName );

    virtual void Load (SvStream& rIn, sal_uInt16 nVersion) override;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;
    virtual             ~SdStyleSheet() override;

    /// @throws css::uno::RuntimeException
    void throwIfDisposed();

    void disposing();

    OUString   msApiName;
    rtl::Reference< SfxStyleSheetBasePool > mxPool;

    /** broadcast helper for events */
    ::cppu::OBroadcastHelper mrBHelper;

    std::unique_ptr< ModifyListenerForewarder > mpModifyListenerForewarder;

private:
    SdStyleSheet( const SdStyleSheet& ) = delete;
    SdStyleSheet& operator=( const SdStyleSheet& ) = delete;
};

typedef rtl::Reference< SdStyleSheet > SdStyleSheetRef;
typedef std::vector< SdStyleSheetRef > SdStyleSheetVector;

#endif // INCLUDED_SD_INC_STLSHEET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
