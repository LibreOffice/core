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

#ifndef INCLUDED_SD_SOURCE_UI_INC_UNOSRCH_HXX
#define INCLUDED_SD_SOURCE_UI_INC_UNOSRCH_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/util/XReplaceable.hpp>
#include <com/sun/star/util/XSearchable.hpp>
#include <com/sun/star/util/XSearchDescriptor.hpp>
#include <com/sun/star/util/XReplaceDescriptor.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <editeng/editdata.hxx>
#include <editeng/unoipset.hxx>

class SvxItemPropertySet;
class SdUnoSearchReplaceDescriptor;

/** this class implements a search or replace operation on a given page or a given sdrobj */
class SdUnoSearchReplaceShape : public ::com::sun::star::util::XReplaceable
{
protected:
    ::com::sun::star::drawing::XShape* mpShape;
    ::com::sun::star::drawing::XDrawPage* mpPage;

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  Search( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  xText, SdUnoSearchReplaceDescriptor* pDescr ) throw();
    bool Search( const OUString& rText, sal_Int32& nStartPos, sal_Int32& nEndPos, SdUnoSearchReplaceDescriptor* pDescr ) throw();
    static ESelection GetSelection( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  xTextRange ) throw();
    static ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  GetShape( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  xTextRange ) throw();
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  GetNextShape( ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xShapes, ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  xCurrentShape ) throw();
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  GetCurrentShape() const throw();

public:
    // danger, this c'tor is only usable if the given shape or page is derived
    // from this class!!!
    SdUnoSearchReplaceShape( ::com::sun::star::drawing::XDrawPage* xPage ) throw();
    virtual ~SdUnoSearchReplaceShape() throw();

    // XReplaceable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XReplaceDescriptor > SAL_CALL createReplaceDescriptor(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL replaceAll( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor >& xDesc ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XSearchable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor > SAL_CALL createSearchDescriptor(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL findAll( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor >& xDesc ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL findFirst( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor >& xDesc ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL findNext( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xStartAt, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor >& xDesc ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

/* ================================================================= */

/** this class holds the parameters and status of a search or replace operation performed
    by class SdUnoSearchReplaceShape */

class SdUnoSearchReplaceDescriptor : public ::cppu::WeakImplHelper< ::com::sun::star::lang::XUnoTunnel, ::com::sun::star::util::XReplaceDescriptor > // public ::com::sun::star::util::XSearchDescriptor, ::com::sun::star::beans::XPropertySet
{
protected:
    SvxItemPropertySet* mpPropSet;

    bool mbBackwards;
    bool mbCaseSensitive;
    bool mbWords;

    bool mbReplace;

    OUString maSearchStr;
    OUString maReplaceStr;

public:
    SdUnoSearchReplaceDescriptor(bool bReplace) throw (css::uno::RuntimeException);
    virtual ~SdUnoSearchReplaceDescriptor() throw();

    bool IsCaseSensitive() const { return mbCaseSensitive; }
    bool IsWords() const { return mbWords; }

    UNO3_GETIMPLEMENTATION_DECL( SdUnoSearchReplaceDescriptor )

    // XSearchDescriptor
    virtual OUString SAL_CALL getSearchString(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSearchString( const OUString& aString ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XReplaceDescriptor
    virtual OUString SAL_CALL getReplaceString(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setReplaceString( const OUString& aReplaceString ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
};

/* ================================================================= */

/** this class holds a sequence that is a result from a find all and
    lets people access it through the XIndexAccess Interface. */
class SdUnoFindAllAccess : public ::cppu::WeakImplHelper< ::com::sun::star::container::XIndexAccess > // public ::com::sun::star::container::XElementAccess
{
protected:
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  > maSequence;

public:
    SdUnoFindAllAccess( ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  >& rSequence ) throw();
    virtual ~SdUnoFindAllAccess() throw();

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) override ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_SD_SOURCE_UI_INC_UNOSRCH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
