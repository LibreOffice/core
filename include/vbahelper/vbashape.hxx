/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_VBAHELPER_VBASHAPE_HXX
#define INCLUDED_VBAHELPER_VBASHAPE_HXX

#include <ooo/vba/office/MsoShapeType.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/msforms/XShape.hpp>
#include <ooo/vba/msforms/XLineFormat.hpp>
#include <cppuhelper/implbase.hxx>

#include <vbahelper/vbahelperinterface.hxx>

typedef  ::cppu::WeakImplHelper< ov::msforms::XShape, css::lang::XEventListener > ListeningShape;

typedef InheritedHelperInterfaceImpl< ListeningShape > ScVbaShape_BASE;

class VBAHELPER_DLLPUBLIC ScVbaShape : public ScVbaShape_BASE
{
protected:
    std::unique_ptr< ov::ShapeHelper > m_pShapeHelper;
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::drawing::XShapes > m_xShapes;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
    sal_Int32 m_nType;
    css::uno::Reference< css::frame::XModel > m_xModel;
    css::uno::Any m_aRange;
    void addListeners();
    void removeShapeListener() throw( css::uno::RuntimeException );
    void removeShapesListener() throw( css::uno::RuntimeException );
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
public:
    ScVbaShape( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape >& xShape, const css::uno::Reference< css::drawing::XShapes >& xShapes, const css::uno::Reference< css::frame::XModel >& xModel, sal_Int32 nType )
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);
    virtual ~ScVbaShape();
    void setRange( css::uno::Any aRange ) { m_aRange = aRange; };

    static sal_Int32 getType( const css::uno::Reference< css::drawing::XShape >& rShape ) throw (css::uno::RuntimeException);

    // Attributes
    virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setName( const OUString& _name ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getAlternativeText() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setAlternativeText( const OUString& _name ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getHeight() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setHeight(double _height)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getWidth() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setWidth(double _width)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getLeft() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setLeft( double _left ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getTop() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setTop( double _top ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getZOrderPosition() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getRotation() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRotation( double _rotation ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< ov::msforms::XLineFormat > SAL_CALL getLine() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< ov::msforms::XFillFormat > SAL_CALL getFill() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< ov::msforms::XPictureFormat > SAL_CALL getPictureFormat() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getLockAspectRatio() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setLockAspectRatio( sal_Bool _lockaspectratio ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getLockAnchor() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setLockAnchor( sal_Bool _lockanchor ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getRelativeHorizontalPosition() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRelativeHorizontalPosition(::sal_Int32 _relativehorizontalposition)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getRelativeVerticalPosition() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRelativeVerticalPosition(::sal_Int32 _relativeverticalposition)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods
    virtual css::uno::Any SAL_CALL SAL_CALL TextFrame(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL SAL_CALL WrapFormat(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL Delete() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL ZOrder( sal_Int32 ZOrderCmd ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL IncrementRotation( double Increment ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL IncrementLeft( double Increment ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL IncrementTop( double Increment ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL ScaleHeight( double Factor, sal_Bool RelativeToOriginalSize, sal_Int32 Scale ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL ScaleWidth( double Factor, sal_Bool RelativeToOriginalSize, sal_Int32 Scale ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // Replace??
    virtual void SAL_CALL Select( const css::uno::Any& Replace ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL ShapeRange( const css::uno::Any& index ) throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& rEventObject ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
};
#endif // INCLUDED_VBAHELPER_VBASHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
