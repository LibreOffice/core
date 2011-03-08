/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <editeng/editdata.hxx>
#include <editeng/unoipset.hxx>

class SdrObject;
class SvxItemPropertySet;
class SdUnoSearchReplaceDescriptor;

/** this class implements a search or replace operation on a given page or a given sdrobj */
class SdUnoSearchReplaceShape : public ::com::sun::star::util::XReplaceable
{
protected:
    ::com::sun::star::drawing::XShape* mpShape;
    ::com::sun::star::drawing::XDrawPage* mpPage;

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  Search( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  xText, SdUnoSearchReplaceDescriptor* pDescr ) throw();
    sal_Bool Search( const ::rtl::OUString& rText, sal_Int32& nStartPos, sal_Int32& nEndPos, SdUnoSearchReplaceDescriptor* pDescr ) throw();
    ESelection GetSelection( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  xTextRange ) throw();
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  GetShape( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  xTextRange ) throw();
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  GetNextShape( ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xShapes, ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  xCurrentShape ) throw();
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  GetCurrentShape() const throw();

public:
    // danger, this c'tor is only usable if the given shape or page is derived
    // from this class!!!
    SdUnoSearchReplaceShape( ::com::sun::star::drawing::XDrawPage* xPage ) throw();
    virtual ~SdUnoSearchReplaceShape() throw();

    // XReplaceable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XReplaceDescriptor > SAL_CALL createReplaceDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL replaceAll( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor >& xDesc ) throw(::com::sun::star::uno::RuntimeException);

    // XSearchable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor > SAL_CALL createSearchDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL findAll( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor >& xDesc ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL findFirst( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor >& xDesc ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL findNext( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xStartAt, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor >& xDesc ) throw(::com::sun::star::uno::RuntimeException);
};

/* ================================================================= */

/** this class holds the parameters and status of a search or replace operation performed
    by class SdUnoSearchReplaceShape */

#include <cppuhelper/implbase2.hxx>

class SdUnoSearchReplaceDescriptor : public ::cppu::WeakImplHelper2< ::com::sun::star::lang::XUnoTunnel, ::com::sun::star::util::XReplaceDescriptor > // public ::com::sun::star::util::XSearchDescriptor, ::com::sun::star::beans::XPropertySet
{
protected:
    SvxItemPropertySet* mpPropSet;

    sal_Bool mbBackwards;
    sal_Bool mbCaseSensitive;
    sal_Bool mbWords;

    sal_Bool mbReplace;

    ::rtl::OUString maSearchStr;
    ::rtl::OUString maReplaceStr;

public:
    SdUnoSearchReplaceDescriptor( sal_Bool bReplace ) throw();
    virtual ~SdUnoSearchReplaceDescriptor() throw();

    sal_Bool IsBackwards() const throw() { return mbBackwards; }
    sal_Bool IsCaseSensitive() const throw() { return mbCaseSensitive; }
    sal_Bool IsWords() const throw() { return mbWords; }

    UNO3_GETIMPLEMENTATION_DECL( SdUnoSearchReplaceDescriptor )

    // XSearchDescriptor
    virtual ::rtl::OUString SAL_CALL getSearchString(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSearchString( const ::rtl::OUString& aString ) throw(::com::sun::star::uno::RuntimeException);

    // XReplaceDescriptor
    virtual ::rtl::OUString SAL_CALL getReplaceString(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setReplaceString( const ::rtl::OUString& aReplaceString ) throw(::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};

/* ================================================================= */

#include <cppuhelper/implbase1.hxx>

/** this class holds a sequence that is a result from a find all and
    lets people access it through the XIndexAccess Interface. */
class SdUnoFindAllAccess : public ::cppu::WeakImplHelper1< ::com::sun::star::container::XIndexAccess > // public ::com::sun::star::container::XElementAccess
{
protected:
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  > maSequence;

public:
    SdUnoFindAllAccess( ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  >& rSequence ) throw();
    virtual ~SdUnoFindAllAccess() throw();

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
