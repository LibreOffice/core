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

#include <memory>
#include <com/sun/star/util/XReplaceable.hpp>
#include <com/sun/star/util/XReplaceDescriptor.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <editeng/editdata.hxx>

namespace com::sun::star::drawing { class XDrawPage; }
namespace com::sun::star::drawing { class XShape; }
namespace com::sun::star::text { class XTextRange; }
namespace com::sun::star::util { class XSearchDescriptor; }

class SvxItemPropertySet;
class SdUnoSearchReplaceDescriptor;

/** this class implements a search or replace operation on a given page or a given sdrobj */
class SdUnoSearchReplaceShape : public css::util::XReplaceable
{
protected:
    css::drawing::XDrawPage* mpPage;

    css::uno::Reference< css::text::XTextRange >  Search( const css::uno::Reference< css::text::XTextRange >&  xText, SdUnoSearchReplaceDescriptor* pDescr );
    bool Search( const OUString& rText, sal_Int32& nStartPos, sal_Int32& nEndPos, SdUnoSearchReplaceDescriptor* pDescr ) throw();
    static ESelection GetSelection( const css::uno::Reference< css::text::XTextRange >&  xTextRange ) throw();
    static css::uno::Reference< css::drawing::XShape >  GetShape( const css::uno::Reference< css::text::XTextRange >&  xTextRange ) throw();
    css::uno::Reference< css::drawing::XShape >  GetNextShape( const css::uno::Reference< css::container::XIndexAccess >&  xShapes, const css::uno::Reference< css::drawing::XShape >&  xCurrentShape ) throw();
    css::uno::Reference< css::drawing::XShape >  GetCurrentShape() const throw();

public:
    // danger, this c'tor is only usable if the given shape or page is derived
    // from this class!!!
    SdUnoSearchReplaceShape( css::drawing::XDrawPage* xPage ) throw();
    virtual ~SdUnoSearchReplaceShape() throw();

    // XReplaceable
    virtual css::uno::Reference< css::util::XReplaceDescriptor > SAL_CALL createReplaceDescriptor(  ) override;
    virtual sal_Int32 SAL_CALL replaceAll( const css::uno::Reference< css::util::XSearchDescriptor >& xDesc ) override;

    // XSearchable
    virtual css::uno::Reference< css::util::XSearchDescriptor > SAL_CALL createSearchDescriptor(  ) override;
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL findAll( const css::uno::Reference< css::util::XSearchDescriptor >& xDesc ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL findFirst( const css::uno::Reference< css::util::XSearchDescriptor >& xDesc ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL findNext( const css::uno::Reference< css::uno::XInterface >& xStartAt, const css::uno::Reference< css::util::XSearchDescriptor >& xDesc ) override;
};

/* ================================================================= */

/** this class holds the parameters and status of a search or replace operation performed
    by class SdUnoSearchReplaceShape */

class SdUnoSearchReplaceDescriptor : public ::cppu::WeakImplHelper< css::lang::XUnoTunnel, css::util::XReplaceDescriptor > // public css::util::XSearchDescriptor, css::beans::XPropertySet
{
protected:
    std::unique_ptr<SvxItemPropertySet> mpPropSet;

    bool mbBackwards;
    bool mbCaseSensitive;
    bool mbWords;

    OUString maSearchStr;
    OUString maReplaceStr;

public:
    /// @throws css::uno::RuntimeException
    SdUnoSearchReplaceDescriptor();
    virtual ~SdUnoSearchReplaceDescriptor() throw() override;

    bool IsCaseSensitive() const { return mbCaseSensitive; }
    bool IsWords() const { return mbWords; }

    UNO3_GETIMPLEMENTATION_DECL( SdUnoSearchReplaceDescriptor )

    // XSearchDescriptor
    virtual OUString SAL_CALL getSearchString(  ) override;
    virtual void SAL_CALL setSearchString( const OUString& aString ) override;

    // XReplaceDescriptor
    virtual OUString SAL_CALL getReplaceString(  ) override;
    virtual void SAL_CALL setReplaceString( const OUString& aReplaceString ) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
};

/* ================================================================= */

/** this class holds a sequence that is a result from a find all and
    lets people access it through the XIndexAccess Interface. */
class SdUnoFindAllAccess final : public ::cppu::WeakImplHelper< css::container::XIndexAccess > // public css::container::XElementAccess
{
    css::uno::Sequence< css::uno::Reference< css::uno::XInterface >  > maSequence;

public:
    SdUnoFindAllAccess( css::uno::Sequence< css::uno::Reference< css::uno::XInterface >  > const & rSequence ) throw();
    virtual ~SdUnoFindAllAccess() throw() override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
