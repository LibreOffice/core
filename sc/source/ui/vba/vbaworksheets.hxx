/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaworksheets.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:07:32 $
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
#ifndef SC_VBA_WORKSHEETS_HXX
#define SC_VBA_WORKSHEETS_HXX

#include <cppuhelper/implbase1.hxx>

#include <org/openoffice/excel/XWorksheets.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <org/openoffice/vba/XGlobals.hpp>

#include "vbacollectionimpl.hxx"

class ScModelObj;


typedef CollTestImplHelper< oo::excel::XWorksheets > ScVbaWorksheets_BASE;

class ScVbaWorksheets : public ScVbaWorksheets_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::sheet::XSpreadsheets > m_xSheets;
protected:
    // ScVbaWorksheets_BASE
    virtual css::uno::Any getItemByStringIndex( const rtl::OUString& sIndex ) throw (css::uno::RuntimeException);
public:
    ScVbaWorksheets( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::container::XIndexAccess >& xSheets, const css::uno::Reference< css::frame::XModel >& xModel );
    ScVbaWorksheets( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::container::XEnumerationAccess >& xEnum,  const css::uno::Reference< css::frame::XModel >& xModel );
    virtual ~ScVbaWorksheets() {}

    bool isSelectedSheets();

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);


    // XWorksheets
    virtual css::uno::Any SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( const css::uno::Any& _visible ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Add( const css::uno::Any& Before, const css::uno::Any& After, const css::uno::Any& Count, const css::uno::Any& Type ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Delete(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL PrintOut( const css::uno::Any& From, const css::uno::Any& To, const css::uno::Any& Copies, const css::uno::Any& Preview, const css::uno::Any& ActivePrinter, const css::uno::Any& PrintToFile, const css::uno::Any& Collate, const css::uno::Any& PrToFileName ) throw (css::uno::RuntimeException);
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );
    virtual void SAL_CALL Select( const css::uno::Any& Replace ) throw (css::uno::RuntimeException);
    // ScVbaWorksheets_BASE
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index1, const css::uno::Any& Index2 ) throw
(css::uno::RuntimeException);
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();

};

#endif /* SC_VBA_WORKSHEETS_HXX */
