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
#ifndef INCLUDED_SD_SOURCE_UI_UNOIDL_UNOOBJ_HXX
#define INCLUDED_SD_SOURCE_UI_UNOIDL_UNOOBJ_HXX

#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <svx/unomaster.hxx>

namespace com { namespace sun { namespace star { namespace beans { class XPropertySetInfo; } } } }

class SdXImpressDocument;
class SdAnimationInfo;
class SvxItemPropertySet;
class SvxShape;
struct SfxItemPropertyMapEntry;

class SdXShape : public SvxShapeMaster,
                 public css::document::XEventsSupplier
{
    friend class SdUnoEventsAccess;

private:
    SvxShape* mpShape;
    const SvxItemPropertySet*   mpPropSet;
    const SfxItemPropertyMapEntry* mpMap;
    SdXImpressDocument* mpModel;

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    void SetStyleSheet( const css::uno::Any& rAny );
    /// @throws css::beans::UnknownPropertyException
    css::uno::Any GetStyleSheet() const;

    // Intern
    /// @throws std::exception
    SdAnimationInfo* GetAnimationInfo( bool bCreate = false ) const;
    /// @throws std::exception
    bool IsPresObj() const;

    bool IsEmptyPresObj() const;
    void SetEmptyPresObj(bool bEmpty);

    bool IsMasterDepend() const throw();
    void SetMasterDepend( bool bDepend ) throw();

    OUString GetPlaceholderText() const;

public:
    SdXShape(SvxShape* pShape, SdXImpressDocument* pModel);
    virtual ~SdXShape() throw();

    virtual bool queryAggregation( const css::uno::Type & rType, css::uno::Any& aAny ) override;
    virtual void dispose() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;

    //XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

    // XEventsSupplier
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents(  ) override;
};

struct SvEventDescription;
const SvEventDescription* ImplGetSupportedMacroItems();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
