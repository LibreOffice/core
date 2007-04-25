/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaapplication.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:01:04 $
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
#ifndef SC_VBA_APPLICATION_HXX
#define SC_VBA_APPLICATION_HXX

#include <cppuhelper/implbase1.hxx>

#include <org/openoffice/excel/XWorksheetFunction.hpp>
#include <org/openoffice/excel/XApplication.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "vbahelper.hxx"

typedef ::cppu::WeakImplHelper1< oo::excel::XApplication > ScVbaApplication_BASE;

class ScVbaApplication : public ScVbaApplication_BASE
{
private:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    sal_Int32 m_xCalculation;
public:
    ScVbaApplication( css::uno::Reference< css::uno::XComponentContext >& m_xContext );
    virtual ~ScVbaApplication();

    // XApplication
    virtual ::sal_Int32 SAL_CALL getCalculation() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCalculation( ::sal_Int32 _calculation ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL getSelection() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XWorkbook > SAL_CALL getActiveWorkbook() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL getActiveCell() throw ( css::uno::RuntimeException);
 virtual css::uno::Reference< oo::excel::XWindow > SAL_CALL getActiveWindow() throw (::com::sun::star::uno::RuntimeException);
 virtual css::uno::Reference< oo::excel::XWorksheet > SAL_CALL getActiveSheet() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getScreenUpdating() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setScreenUpdating(sal_Bool bUpdate) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getDisplayStatusBar() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayStatusBar(sal_Bool bDisplayStatusBar) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XWorkbook > SAL_CALL getThisWorkbook() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Workbooks( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Worksheets( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL WorksheetFunction( ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Evaluate( const ::rtl::OUString& Name ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Dialogs( const css::uno::Any& DialogIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getCutCopyMode() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCutCopyMode( const css::uno::Any& _cutcopymode ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getVersion() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getStatusBar() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStatusBar( const css::uno::Any& _statusbar ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getCursor() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCursor( ::sal_Int32 _cursor ) throw (css::uno::RuntimeException);

    virtual double SAL_CALL CountA( const css::uno::Any& arg1 ) throw (css::uno::RuntimeException) ;

    virtual css::uno::Any SAL_CALL Windows( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL wait( double time ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Range( const css::uno::Any& Cell1, const css::uno::Any& Cell2 ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL GoTo( const css::uno::Any& Reference, const css::uno::Any& Scroll ) throw (css::uno::RuntimeException);
};
#endif /* SC_VBA_APPLICATION_HXX */
