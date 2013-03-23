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

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <svx/svdpool.hxx>
#include <svx/unomaster.hxx>

#include <editeng/unoipset.hxx>

#include <cppuhelper/implbase2.hxx>

class SdrObject;
class SdXImpressDocument;
class SdAnimationInfo;

class SdXShape : public SvxShapeMaster,
                 public ::com::sun::star::document::XEventsSupplier
{
    friend class SdUnoEventsAccess;

private:
    SvxShape* mpShape;
    const SvxItemPropertySet*   mpPropSet;
    const SfxItemPropertyMapEntry* mpMap;
    SdXImpressDocument* mpModel;

    void SetStyleSheet( const ::com::sun::star::uno::Any& rAny ) throw( ::com::sun::star::lang::IllegalArgumentException );
    ::com::sun::star::uno::Any GetStyleSheet() const throw( ::com::sun::star::beans::UnknownPropertyException  );

    // Intern
    SdAnimationInfo* GetAnimationInfo( sal_Bool bCreate = sal_False ) const throw();
    sal_Bool IsPresObj() const throw();
    void SetPresObj( sal_Bool bPresObj ) throw();

    bool IsEmptyPresObj() const throw();
    void SetEmptyPresObj( bool bEmpty ) throw();

    sal_Bool IsMasterDepend() const throw();
    void SetMasterDepend( sal_Bool bDepend ) throw();

    SdrObject* GetSdrObject() const throw();
    ::rtl::OUString GetPlaceholderText() const;

    com::sun::star::uno::Sequence< sal_Int8 >* mpImplementationId;

public:
    SdXShape(SvxShape* pShape, SdXImpressDocument* pModel) throw();
    virtual ~SdXShape() throw();

    virtual sal_Bool queryAggregation( const com::sun::star::uno::Type & rType, com::sun::star::uno::Any& aAny );
    virtual void dispose();
    virtual void modelChanged( SdrModel* pNewModel );
    virtual void pageChanged( SdrPage* pNewPage );
    virtual void objectChanged( SdrObject* pNewObj );

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    // XEventsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw(::com::sun::star::uno::RuntimeException);

private:
    void setOldEffect( const com::sun::star::uno::Any& aValue );
    void setOldTextEffect( const com::sun::star::uno::Any& aValue );
    void setOldSpeed( const com::sun::star::uno::Any& aValue );
    void setOldDimColor( const com::sun::star::uno::Any& aValue );
    void setOldDimHide( const com::sun::star::uno::Any& aValue );
    void setOldDimPrevious( const com::sun::star::uno::Any& aValue );
    void setOldPresOrder( const com::sun::star::uno::Any& aValue );
    void updateOldSoundEffect( SdAnimationInfo* pInfo );

    void getOldEffect( com::sun::star::uno::Any& rValue ) const;
    void getOldTextEffect( com::sun::star::uno::Any& rValue ) const;
    void getOldSpeed( com::sun::star::uno::Any& rValue ) const;
    void getOldSoundFile( com::sun::star::uno::Any& rValue ) const;
    void getOldSoundOn( com::sun::star::uno::Any& rValue ) const;
    void getOldDimColor( com::sun::star::uno::Any& rValue ) const;
    void getOldDimHide( com::sun::star::uno::Any& rValue ) const;
    void getOldDimPrev( com::sun::star::uno::Any& rValue ) const;
    void getOldPresOrder( com::sun::star::uno::Any& rValue ) const;
};

struct SvEventDescription;
const SvEventDescription* ImplGetSupportedMacroItems();

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
