/*************************************************************************
 *
 *  $RCSfile: unosrch.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREPLACEABLE_HPP_
#include <com/sun/star/util/XReplaceable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSEARCHABLE_HPP_
#include <com/sun/star/util/XSearchable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSEARCHDESCRIPTOR_HPP_
#include <com/sun/star/util/XSearchDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREPLACEDESCRIPTOR_HPP_
#include <com/sun/star/util/XReplaceDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#include <unotools/servicehelper.hxx>

#ifndef _MyEDITDATA_HXX //autogen
#include <svx/editdata.hxx>
#endif

#ifndef _SVX_UNOIPSET_HXX_
#include <svx/unoipset.hxx>
#endif

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
    SdUnoSearchReplaceShape( ::com::sun::star::drawing::XDrawPage* xPage ) throw(); // danger, these c'tors are only usable if
    SdUnoSearchReplaceShape( ::com::sun::star::drawing::XShape* xShape ) throw();   // the given shape or page is derived from
    virtual ~SdUnoSearchReplaceShape() throw();                                     // this class!!!

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

