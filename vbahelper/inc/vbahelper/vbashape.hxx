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
#ifndef SC_VBA_SHAPE_HXX
#define SC_VBA_SHAPE_HXX

#include <ooo/vba/office/MsoShapeType.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/msforms/XShape.hpp>
#include <ooo/vba/msforms/XLineFormat.hpp>
#include <cppuhelper/implbase2.hxx>

#include <vbahelper/vbahelperinterface.hxx>

typedef  ::cppu::WeakImplHelper2< ov::msforms::XShape, css::lang::XEventListener > ListeningShape;

typedef InheritedHelperInterfaceImpl< ListeningShape > ScVbaShape_BASE;

class VBAHELPER_DLLPUBLIC ScVbaShape : public ScVbaShape_BASE
{
private:
protected:
    std::auto_ptr< ov::ShapeHelper > m_pShapeHelper;
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::drawing::XShapes > m_xShapes;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
    sal_Int32 m_nType;
    css::uno::Reference< css::frame::XModel > m_xModel;
    css::uno::Any m_aRange;
    virtual void addListeners();
    virtual void removeShapeListener() throw( css::uno::RuntimeException );
    virtual void removeShapesListener() throw( css::uno::RuntimeException );
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
public:
    ScVbaShape( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape >& xShape, const css::uno::Reference< css::drawing::XShapes >& xShapes, const css::uno::Reference< css::frame::XModel >& xModel, sal_Int32 nType ) throw ( css::lang::IllegalArgumentException );
    ScVbaShape( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape >& xShape, const css::uno::Reference< css::frame::XModel >& xModel ) throw ( css::lang::IllegalArgumentException );
    virtual ~ScVbaShape();
    css::uno::Any getRange() { return m_aRange; };
    void setRange( css::uno::Any aRange ) { m_aRange = aRange; };

    static sal_Int32 getType( const css::uno::Reference< css::drawing::XShape > xShape ) throw (css::uno::RuntimeException);

    // Attributes
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
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getZOrderPosition() throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getType() throw (css::uno::RuntimeException);
    virtual double SAL_CALL getRotation() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRotation( double _rotation ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XLineFormat > SAL_CALL getLine() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XFillFormat > SAL_CALL getFill() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XPictureFormat > SAL_CALL getPictureFormat() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getLockAspectRatio() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLockAspectRatio( ::sal_Bool _lockaspectratio ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getLockAnchor() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLockAnchor( ::sal_Bool _lockanchor ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getRelativeHorizontalPosition() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRelativeHorizontalPosition( ::sal_Int32 _relativehorizontalposition ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getRelativeVerticalPosition() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRelativeVerticalPosition( ::sal_Int32 _relativeverticalposition ) throw (css::uno::RuntimeException);

    // Methods
    virtual css::uno::Any SAL_CALL SAL_CALL TextFrame(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL SAL_CALL WrapFormat(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Delete() throw (css::uno::RuntimeException);
    virtual void SAL_CALL ZOrder( sal_Int32 ZOrderCmd ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL IncrementRotation( double Increment ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL IncrementLeft( double Increment ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL IncrementTop( double Increment ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL ScaleHeight( double Factor, sal_Bool RelativeToOriginalSize, sal_Int32 Scale ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL ScaleWidth( double Factor, sal_Bool RelativeToOriginalSize, sal_Int32 Scale ) throw (css::uno::RuntimeException);
    // Replace??
    virtual void SAL_CALL Select( const css::uno::Any& Replace ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL ShapeRange( const css::uno::Any& index ) throw ( css::uno::RuntimeException );
    virtual void SAL_CALL Copy(  ) throw (css::uno::RuntimeException);
    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& rEventObject ) throw( css::uno::RuntimeException );
};
#endif//SC_VBA_SHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
