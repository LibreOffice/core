/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SW_VBA_XWRAPFORMAT_HXX
#define SW_VBA_XWRAPFORMAT_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <ooo/vba/word/XWrapFormat.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XWrapFormat > SwVbaWrapFormat_BASE;

class SwVbaWrapFormat : public SwVbaWrapFormat_BASE
{
private:
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
    sal_Int32 mnWrapFormatType;
    sal_Int32 mnSide;

private:
    void makeWrap() throw (css::uno::RuntimeException);
    float getDistance( const rtl::OUString& sName ) throw (css::uno::RuntimeException);
    void setDistance( const rtl::OUString& sName, float _distance ) throw (css::uno::RuntimeException);

public:
    SwVbaWrapFormat(  css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );

    virtual ::sal_Int32 SAL_CALL getType() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setType( ::sal_Int32 _type ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getSide() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSide( ::sal_Int32 _side ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getDistanceTop() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDistanceTop( float _distancetop ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getDistanceBottom() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDistanceBottom( float _distancebottom ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getDistanceLeft() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDistanceLeft( float _distanceleft ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getDistanceRight() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDistanceRight( float _distanceright ) throw (css::uno::RuntimeException);

    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif//SW_VBA_XWRAPFORMAT_HXX
