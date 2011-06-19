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
#ifndef SC_VBA_SHAPES_HXX
#define SC_VBA_SHAPES_HXX

#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <ooo/vba/msforms/XShapes.hpp>

#include <vbahelper/vbahelperinterface.hxx>

#include <vbahelper/vbacollectionimpl.hxx>

typedef CollTestImplHelper< ov::msforms::XShapes > ScVbaShapes_BASE;

class VBAHELPER_DLLPUBLIC ScVbaShapes : public ScVbaShapes_BASE
{
private:
    css::uno::Reference< css::drawing::XShapes > m_xShapes;
    css::uno::Reference< css::drawing::XDrawPage > m_xDrawPage;
    sal_Int32 m_nNewShapeCount;
    void initBaseCollection();
protected:
    css::uno::Reference< css::frame::XModel > m_xModel;
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
    virtual css::uno::Reference< css::container::XIndexAccess > getShapesByArrayIndices( const css::uno::Any& Index ) throw (css::uno::RuntimeException);
    css::uno::Reference< css::drawing::XShape > createShape( rtl::OUString service ) throw (css::uno::RuntimeException);
    css::uno::Any AddRectangle( sal_Int32 startX, sal_Int32 startY, sal_Int32 nLineWidth, sal_Int32 nLineHeight, css::uno::Any aRange ) throw (css::uno::RuntimeException);
    css::uno::Any AddEllipse( sal_Int32 startX, sal_Int32 startY, sal_Int32 nLineWidth, sal_Int32 nLineHeight, css::uno::Any aRange ) throw (css::uno::RuntimeException);
    css::uno::Any AddTextboxInWriter( sal_Int32 _nOrientation, sal_Int32 _nLeft, sal_Int32 _nTop, sal_Int32 _nWidth, sal_Int32 _nHeight ) throw (css::uno::RuntimeException);
    rtl::OUString createName( rtl::OUString sName );
    css::uno::Any AddShape( const rtl::OUString& sService, const rtl::OUString& sName, sal_Int32 _nLeft, sal_Int32 _nTop, sal_Int32 _nWidth, sal_Int32 _nHeight ) throw (css::uno::RuntimeException);
    //TODO helperapi using a writer document
    //css::awt::Point calculateTopLeftMargin( css::uno::Reference< ov::XHelperInterface > xDocument );

public:
    ScVbaShapes( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess > xShapes, const css::uno::Reference< css::frame::XModel >& xModel );
    static void setDefaultShapeProperties( css::uno::Reference< css::drawing::XShape > xShape ) throw (css::uno::RuntimeException);
    static void setShape_NameProperty( css::uno::Reference< css::drawing::XShape > xShape, rtl::OUString sName );
    //XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

    virtual void SAL_CALL SelectAll() throw (css::uno::RuntimeException);
    //helper::calc
    virtual css::uno::Any SAL_CALL AddLine( sal_Int32 StartX, sal_Int32 StartY, sal_Int32 endX, sal_Int32 endY ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL AddShape( sal_Int32 _nType, sal_Int32 _nLeft, sal_Int32 _nTop, sal_Int32 _nWidth, sal_Int32 _nHeight ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL AddTextbox( sal_Int32 _nOrientation, sal_Int32 _nLeft, sal_Int32 _nTop, sal_Int32 _nWidth, sal_Int32 _nHeight ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XShapeRange > SAL_CALL Range( const css::uno::Any& shapes ) throw (css::uno::RuntimeException);
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index1, const css::uno::Any& Index2 ) throw (css::uno::RuntimeException);
};

#endif//SC_VBA_SHAPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
