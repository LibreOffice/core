/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaworkbook.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:13:02 $
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

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <org/openoffice/excel/XWorkbook.hpp>
#include "vbahelper.hxx"

class ScModelObj;

typedef ::cppu::WeakImplHelper1< oo::excel::XWorkbook > ScVbaWorkbook_BASE;

class ScVbaWorkbook : public ScVbaWorkbook_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
protected:
    virtual css::uno::Reference< css::frame::XModel >  getModel() { return mxModel; }
    ScVbaWorkbook(  css::uno::Reference< css::uno::XComponentContext >& xContext) : mxModel(NULL), m_xContext( xContext ){}
public:
    ScVbaWorkbook(  css::uno::Reference< css::uno::XComponentContext >& xContext,
            css::uno::Reference< css::frame::XModel > xModel ) :
             mxModel( xModel ), m_xContext( xContext ){}
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
};

#endif /* SC_VBA_WORKBOOK_HXX */
