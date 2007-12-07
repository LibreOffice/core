/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaapplication.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:43:17 $
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


#include <org/openoffice/excel/XWorksheetFunction.hpp>
#include <org/openoffice/excel/XApplication.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "vbahelperinterface.hxx"

typedef InheritedHelperInterfaceImpl1< oo::excel::XApplication > ScVbaApplication_BASE;

class ScVbaApplication : public ScVbaApplication_BASE
{
private:
    sal_Int32 m_xCalculation;
    rtl::OUString getOfficePath( const rtl::OUString& sPath ) throw ( css::uno::RuntimeException );
public:
    ScVbaApplication( css::uno::Reference< css::uno::XComponentContext >& m_xContext );
    virtual ~ScVbaApplication();

    // XHelperInterface ( parent is itself )
    virtual css::uno::Reference< oo::vba::XHelperInterface > SAL_CALL getParent(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException) { return this; }

    // XApplication
    virtual ::rtl::OUString SAL_CALL PathSeparator(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setDefaultFilePath( const ::rtl::OUString& DefaultFilePath ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDefaultFilePath(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL LibraryPath(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL TemplatesPath(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getDisplayAlerts() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayAlerts( sal_Bool displayAlerts ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getCalculation() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCalculation( ::sal_Int32 _calculation ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getSelection() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XWorkbook > SAL_CALL getActiveWorkbook() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL getActiveCell() throw ( css::uno::RuntimeException);
 virtual css::uno::Reference< oo::excel::XWindow > SAL_CALL getActiveWindow() throw (css::uno::RuntimeException);
 virtual css::uno::Reference< oo::excel::XWorksheet > SAL_CALL getActiveSheet() throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getScreenUpdating() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setScreenUpdating(sal_Bool bUpdate) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getDisplayStatusBar() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayStatusBar(sal_Bool bDisplayStatusBar) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getDisplayFormulaBar() throw ( css::uno::RuntimeException );
    virtual void SAL_CALL setDisplayFormulaBar( ::sal_Bool _displayformulabar ) throw ( css::uno::RuntimeException );

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
    virtual css::uno::Any SAL_CALL Names( ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL GoTo( const css::uno::Any& Reference, const css::uno::Any& Scroll ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Calculate() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XRange > SAL_CALL Intersect( const css::uno::Reference< oo::excel::XRange >& Arg1, const css::uno::Reference< oo::excel::XRange >& Arg2, const css::uno::Any& Arg3, const css::uno::Any& Arg4, const css::uno::Any& Arg5, const css::uno::Any& Arg6, const css::uno::Any& Arg7, const css::uno::Any& Arg8, const css::uno::Any& Arg9, const css::uno::Any& Arg10, const css::uno::Any& Arg11, const css::uno::Any& Arg12, const css::uno::Any& Arg13, const css::uno::Any& Arg14, const css::uno::Any& Arg15, const css::uno::Any& Arg16, const css::uno::Any& Arg17, const css::uno::Any& Arg18, const css::uno::Any& Arg19, const css::uno::Any& Arg20, const css::uno::Any& Arg21, const css::uno::Any& Arg22, const css::uno::Any& Arg23, const css::uno::Any& Arg24, const css::uno::Any& Arg25, const css::uno::Any& Arg26, const css::uno::Any& Arg27, const css::uno::Any& Arg28, const css::uno::Any& Arg29, const css::uno::Any& Arg30 ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL Volatile( const css::uno::Any& Volatile ) throw (css::uno::RuntimeException );
    virtual void SAL_CALL DoEvents() throw (css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SC_VBA_APPLICATION_HXX */
