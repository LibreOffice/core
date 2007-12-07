/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaworkbook.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:06:17 $
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
#ifndef SC_VBA_WORKBOOK_HXX
#define SC_VBA_WORKBOOK_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <org/openoffice/excel/XWorkbook.hpp>
#include "vbahelperinterface.hxx"

class ScModelObj;

typedef InheritedHelperInterfaceImpl1< oo::excel::XWorkbook > ScVbaWorkbook_BASE;

class ScVbaWorkbook : public ScVbaWorkbook_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    static css::uno::Sequence< sal_Int32 > ColorData;
    void initColorData( const css::uno::Sequence< sal_Int32 >& sColors );
    void init();
protected:

    virtual css::uno::Reference< css::frame::XModel >  getModel() { return mxModel; }
    ScVbaWorkbook(  const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext);
public:
    ScVbaWorkbook(  const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext,
            css::uno::Reference< css::frame::XModel > xModel );
    ScVbaWorkbook(  css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );
    virtual ~ScVbaWorkbook() {}

    // Attributes
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getPath() throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getFullName() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getProtectStructure() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XWorksheet > SAL_CALL getActiveSheet() throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getSaved() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSaved( sal_Bool bSave ) throw (css::uno::RuntimeException);

    // Methods
    virtual css::uno::Any SAL_CALL Worksheets( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Sheets( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Windows( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Close( const css::uno::Any &bSaveChanges,
                                 const css::uno::Any &aFileName,
                                 const css::uno::Any &bRouteWorkbook ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Protect( const css::uno::Any & aPassword ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Unprotect( const css::uno::Any &aPassword ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Save() throw (css::uno::RuntimeException);
    virtual void SAL_CALL Activate() throw (css::uno::RuntimeException);
    // Amelia Wang
    virtual css::uno::Any SAL_CALL Names( ) throw (css::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL Styles( const css::uno::Any& Item ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL ResetColors(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Colors( const css::uno::Any& Index ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL FileFormat(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL SaveCopyAs( const rtl::OUString& Filename ) throw ( css::uno::RuntimeException);
    // code name
    virtual ::rtl::OUString SAL_CALL getCodeName() throw ( css::uno::RuntimeException);
    virtual void SAL_CALL setCodeName( const ::rtl::OUString& sGlobCodeName ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif /* SC_VBA_WORKBOOK_HXX */
