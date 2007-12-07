/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbatextframe.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:04:25 $
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
#ifndef SC_VBA_TEXTFRAME_HXX
#define SC_VBA_TEXTFRAME_HXX
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <org/openoffice/excel/XCharacters.hpp>
#include <org/openoffice/excel/XTextFrame.hpp>

#include "vbahelperinterface.hxx"
#include "vbapalette.hxx"

typedef InheritedHelperInterfaceImpl1< oo::excel::XTextFrame > ScVbaTextFrame_BASE;

class ScVbaTextFrame : public ScVbaTextFrame_BASE
{
private:
    css::uno::Reference< oo::excel::XCharacters > m_xCharacters;
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
protected:
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
    virtual void setAsMSObehavior();
    sal_Int32 getMargin( rtl::OUString sMarginType );
    void setMargin( rtl::OUString sMarginType, float fMargin );
public:
    ScVbaTextFrame( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext , css::uno::Reference< css::drawing::XShape > xShape);
    virtual ~ScVbaTextFrame() {}
    // Attributes
    virtual sal_Bool SAL_CALL getAutoSize() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAutoSize( sal_Bool _autosize ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getMarginBottom() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMarginBottom( float _marginbottom ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getMarginTop() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMarginTop( float _margintop ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getMarginLeft() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMarginLeft( float _marginleft ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getMarginRight() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMarginRight( float _marginright ) throw (css::uno::RuntimeException);

    // Methods
    virtual css::uno::Reference< oo::excel::XCharacters > SAL_CALL Characters(  ) throw (css::uno::RuntimeException);

};

#endif//SC_VBA_TEXTFRAME_HXX
