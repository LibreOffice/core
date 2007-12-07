/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbapictureformat.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:58:37 $
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
#ifndef SC_VBA_PICTUREFORMAT_HXX
#define SC_VBA_PICTUREFORMAT_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <org/openoffice/msforms/XPictureFormat.hpp>
#include "vbahelperinterface.hxx"

typedef InheritedHelperInterfaceImpl1< oo::msforms::XPictureFormat > ScVbaPictureFormat_BASE;

class ScVbaPictureFormat : public ScVbaPictureFormat_BASE
{
private:
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
protected:
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
private:
    void checkParameterRangeInDouble( double nRange, double nMin, double nMax ) throw (css::uno::RuntimeException);
public:
    ScVbaPictureFormat( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, css::uno::Reference< css::drawing::XShape > xShape ) throw( css::lang::IllegalArgumentException );

    // Attributes
    virtual double SAL_CALL getBrightness() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBrightness( double _brightness ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getContrast() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setContrast( double _contrast ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL IncrementBrightness( double increment ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL IncrementContrast( double increment ) throw (css::uno::RuntimeException);
};

#endif//SC_VBA_PICTUREFORMAT_HXX
