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
#ifndef SC_VBA_PAGES_HXX
#define SC_VBA_PAGES_HXX

#include <ooo/vba/office/MsoShapeType.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/msforms/XPages.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase1.hxx>

#include <vbahelper/vbacollectionimpl.hxx>
typedef CollTestImplHelper<
ov::msforms::XPages > ScVbaPages_BASE;

class ScVbaPages : public ScVbaPages_BASE
{
protected:
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
public:
    ScVbaPages( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xPages ) throw ( css::lang::IllegalArgumentException );
    virtual ~ScVbaPages() {}
        // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
     // ScVbaPages_BASE
     virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

};
#endif//SC_VBA_SHAPE_HXX
