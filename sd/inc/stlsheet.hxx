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

#ifndef _SD_STLSHEET_HXX
#define _SD_STLSHEET_HXX

#include <rtl/ref.hxx>

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/basemutex.hxx>

#include <svl/style.hxx>

#include <editeng/unoipset.hxx>

#include <boost/scoped_ptr.hpp>

class ModifyListenerForewarder;

typedef cppu::ImplInheritanceHelper5< SfxUnoStyleSheet,
                                    ::com::sun::star::beans::XPropertySet,
                                    ::com::sun::star::lang::XServiceInfo,
                                    ::com::sun::star::beans::XPropertyState,
                                    ::com::sun::star::util::XModifyBroadcaster,
                                    ::com::sun::star::lang::XComponent > SdStyleSheetBase ;

class SdStyleSheet : public SdStyleSheetBase, private ::cppu::BaseMutex
{
public:
    SdStyleSheet( const rtl::OUString& rDisplayName, SfxStyleSheetBasePool& rPool, SfxStyleFamily eFamily, sal_uInt16 nMask );
    SdStyleSheet( const SdStyleSheet& );

    virtual bool        SetParent (const String& rParentName);
    virtual SfxItemSet& GetItemSet();
    virtual bool        IsUsed() const;
    virtual bool        HasFollowSupport() const;
    virtual bool        HasParentSupport() const;
    virtual bool        HasClearParentSupport() const;
    virtual bool        SetName( const UniString& );
    virtual void        SetHelpId( const String& r, sal_uLong nId );

    void        AdjustToFontHeight(SfxItemSet& rSet, sal_Bool bOnlyMissingItems = sal_True);

    SdStyleSheet* GetRealStyleSheet() const;
    SdStyleSheet* GetPseudoStyleSheet() const;

    void SetApiName( const ::rtl::OUString& rApiName );
    rtl::OUString GetApiName() const;

    static rtl::OUString GetFamilyString( SfxStyleFamily eFamily );

    static SdStyleSheet* CreateEmptyUserStyle( SfxStyleSheetBasePool& rPool, SfxStyleFamily eFamily );

    // XInterface
    virtual void SAL_CALL release(  ) throw ();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XNamed
    virtual ::rtl::OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XStyle
    virtual sal_Bool SAL_CALL isUserDefined(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isInUse(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getParentStyle(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParentStyle( const ::rtl::OUString& aParentStyle ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    void notifyModifyListener();

protected:
    const SfxItemPropertySimpleEntry* getPropertyMapEntry( const ::rtl::OUString& rPropertyName ) const throw();

    virtual void Load (SvStream& rIn, sal_uInt16 nVersion);
    virtual void Store(SvStream& rOut);

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);
    virtual             ~SdStyleSheet();

    void throwIfDisposed() throw (::com::sun::star::uno::RuntimeException);

    virtual void disposing();

    rtl::OUString   msApiName;
    rtl::Reference< SfxStyleSheetBasePool > mxPool;

    /** boradcast helper for events */
    ::cppu::OBroadcastHelper mrBHelper;

    boost::scoped_ptr< ModifyListenerForewarder > mpModifyListenerForewarder;

private:
    SdStyleSheet& operator=( const SdStyleSheet& ); // not implemented
};

typedef rtl::Reference< SdStyleSheet > SdStyleSheetRef;
typedef std::vector< SdStyleSheetRef > SdStyleSheetVector;

#endif     // _SD_STLSHEET_HXX



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
