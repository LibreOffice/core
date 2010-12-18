/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef SC_VBA_SHAPERANGE_HXX
#define SC_VBA_SHAPERANGE_HXX

#include <com/sun/star/drawing/XShapes.hpp>
#include <ooo/vba/msforms/XShapeRange.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/msforms/XLineFormat.hpp>
#include <ooo/vba/msforms/XFillFormat.hpp>

typedef CollTestImplHelper< ov::msforms::XShapeRange > ScVbaShapeRange_BASE;

class VBAHELPER_DLLPUBLIC ScVbaShapeRange : public ScVbaShapeRange_BASE
{
private:
    css::uno::Reference< css::drawing::XDrawPage > m_xDrawPage;
    css::uno::Reference< css::drawing::XShapes > m_xShapes;
    sal_Int32 m_nShapeGroupCount;
protected:
    css::uno::Reference< css::frame::XModel > m_xModel;
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
    css::uno::Reference< css::drawing::XShapes > getShapes() throw (css::uno::RuntimeException) ;
public:
    ScVbaShapeRange( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xShapes, const css::uno::Reference< css::drawing::XDrawPage>& xDrawShape, const css::uno::Reference< css::frame::XModel >& xModel );

    // Methods
    virtual void SAL_CALL Select(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ::ooo::vba::msforms::XShape > SAL_CALL Group() throw (css::uno::RuntimeException);
    virtual void SAL_CALL IncrementRotation( double Increment ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL IncrementLeft( double Increment ) throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL IncrementTop( double Increment ) throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const rtl::OUString& _name ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getHeight() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHeight( double _height ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getWidth() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setWidth( double _width ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getLeft() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLeft( double _left ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getTop() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTop( double _top ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XLineFormat > SAL_CALL getLine() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XFillFormat > SAL_CALL getFill() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getLockAspectRatio() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLockAspectRatio( ::sal_Bool _lockaspectratio ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getLockAnchor() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLockAnchor( ::sal_Bool _lockanchor ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getRelativeHorizontalPosition() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRelativeHorizontalPosition( ::sal_Int32 _relativehorizontalposition ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getRelativeVerticalPosition() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRelativeVerticalPosition( ::sal_Int32 _relativeverticalposition ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL SAL_CALL TextFrame(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL SAL_CALL WrapFormat(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL ZOrder( sal_Int32 ZOrderCmd ) throw (css::uno::RuntimeException);
    //XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );
};

#endif//SC_VBA_SHAPERANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
