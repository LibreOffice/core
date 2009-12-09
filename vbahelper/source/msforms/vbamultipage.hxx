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
#ifndef SC_VBA_MULTIPAGE_HXX
#define SC_VBA_MULTIPAGE_HXX
#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/msforms/XMultiPage.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include "vbacontrol.hxx"
#include <vbahelper/vbahelper.hxx>
//#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase1.hxx>

typedef cppu::ImplInheritanceHelper1< ScVbaControl, ov::msforms::XMultiPage > MultiPageImpl_BASE;

class ScVbaMultiPage : public MultiPageImpl_BASE
{
    css::uno::Reference< css::container::XIndexAccess > getPages( sal_Int32 nPages );
    css::uno::Reference< css::beans::XPropertySet > mxDialogProps;
public:
    ScVbaMultiPage( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::uno::XInterface >& xControl, css::uno::Reference< css::frame::XModel >& xModel, ov::AbstractGeometryAttributes* pGeomHelper, const css::uno::Reference< css::uno::XInterface >& xDialog );
   // Attributes
    virtual sal_Int32 SAL_CALL getValue() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setValue( sal_Int32 _value ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Pages( const css::uno::Any& index ) throw (css::uno::RuntimeException);

    //XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
    // XDefaultProperty
    rtl::OUString SAL_CALL getDefaultPropertyName(  ) throw (css::uno::RuntimeException) { return ::rtl::OUString::createFromAscii("Value"); }
};
#endif //SC_VBA_LABEL_HXX
