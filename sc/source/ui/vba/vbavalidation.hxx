/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbavalidation.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:04:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SC_VBA_VALIDATION_HXX
#define SC_VBA_VALIDATION_HXX
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <org/openoffice/excel/XValidation.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include "vbahelperinterface.hxx"

typedef InheritedHelperInterfaceImpl1<oo::excel::XValidation > ValidationImpl_BASE;

class ScVbaValidation : public ValidationImpl_BASE
{
    css::uno::Reference< css::table::XCellRange > m_xRange;

public:
    ScVbaValidation( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::table::XCellRange >& xRange ) : ValidationImpl_BASE( xParent, xContext ), m_xRange( xRange) {}
    // Attributes
    virtual ::sal_Bool SAL_CALL getIgnoreBlank() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setIgnoreBlank( ::sal_Bool _ignoreblank ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getInCellDropdown() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setInCellDropdown( ::sal_Bool _incelldropdown ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getShowInput() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setShowInput( ::sal_Bool _showinput ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getShowError() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setShowError( ::sal_Bool _showerror ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getInputTitle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setInputTitle( const ::rtl::OUString& _inputtitle ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getErrorTitle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setErrorTitle( const ::rtl::OUString& _errortitle ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getInputMessage() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setInputMessage( const ::rtl::OUString& _inputmessage ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getErrorMessage() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setErrorMessage( const ::rtl::OUString& _errormessage ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getFormula1() throw (css::uno::RuntimeException) ;
    virtual ::rtl::OUString SAL_CALL getFormula2() throw (css::uno::RuntimeException);
    // Methods
    virtual void SAL_CALL Delete(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Add( const css::uno::Any& Type, const css::uno::Any& AlertStyle, const css::uno::Any& Operator, const css::uno::Any& Formula1, const css::uno::Any& Formula2 ) throw (css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();

};

#endif
