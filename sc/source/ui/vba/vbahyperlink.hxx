/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
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
#ifndef SC_VBA_HYPERLINK_HXX
#define SC_VBA_HYPERLINK_HXX

#include <ooo/vba/excel/XHyperlink.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/excel/XRange.hpp>

#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ov::excel::XHyperlink > HyperlinkImpl_BASE;

class ScVbaHyperlink : public HyperlinkImpl_BASE
{
    css::uno::Reference< css::table::XCell > mxCell;
    css::uno::Reference< css::beans::XPropertySet > mxTextField;

public:
    ScVbaHyperlink( css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext > const& xContext ) throw ( css::lang::IllegalArgumentException );
    virtual ~ScVbaHyperlink();

    // Attributes
    virtual ::rtl::OUString SAL_CALL getAddress() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAddress( const ::rtl::OUString &rAddress ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString &rName ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTextToDisplay() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTextToDisplay( const ::rtl::OUString &rTextToDisplay ) throw (css::uno::RuntimeException);

    // Methods
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Range() throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SC_VBA_HYPERLINK_HXX */

