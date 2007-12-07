/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbalineformat.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:55:05 $
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
#ifndef SC_VBA_XLINEFORMAT_HXX
#define SC_VBA_XLINEFORMAT_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <org/openoffice/msforms/XLineFormat.hpp>
#include "vbahelperinterface.hxx"

typedef InheritedHelperInterfaceImpl1< oo::msforms::XLineFormat > ScVbaLineFormat_BASE;

class ScVbaLineFormat : public ScVbaLineFormat_BASE
{
private:
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
    sal_Int32 m_nLineDashStyle;
    double m_nLineWeight;
protected:
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
    sal_Int32 convertLineStartEndNameToArrowheadStyle( rtl::OUString sLineName );
    rtl::OUString convertArrowheadStyleToLineStartEndName( sal_Int32 nArrowheadStyle ) throw (css::uno::RuntimeException);
    sal_Int32 calculateArrowheadSize();
public:
    ScVbaLineFormat( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape > xShape );

    // Attributes
    virtual sal_Int32 SAL_CALL getBeginArrowheadStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBeginArrowheadStyle( sal_Int32 _beginarrowheadstyle ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBeginArrowheadLength() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBeginArrowheadLength( sal_Int32 _beginarrowheadlength ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBeginArrowheadWidth() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBeginArrowheadWidth( sal_Int32 _beginarrowheadwidth ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getEndArrowheadStylel() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEndArrowheadStylel( sal_Int32 _endarrowheadstylel ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getEndArrowheadLength() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEndArrowheadLength( sal_Int32 _endarrowheadlength ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getEndArrowheadWidth() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEndArrowheadWidth( sal_Int32 _endarrowheadwidth ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getWeight() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setWeight( double _weight ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getTransparency() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTransparency( double _transparency ) throw (css::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStyle( sal_Int16 _style ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getDashStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDashStyle( sal_Int32 _dashstyle ) throw (css::uno::RuntimeException);

    // Methods
    virtual css::uno::Reference< oo::msforms::XColorFormat > SAL_CALL BackColor() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::msforms::XColorFormat > SAL_CALL ForeColor() throw (css::uno::RuntimeException);
};

#endif//SC_VBA_XLINEFORMAT_HXX
