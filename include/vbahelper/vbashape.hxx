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

#include <exception>
#include <memory>

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <ooo/vba/msforms/XShape.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vbahelper/vbadllapi.h>
#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbahelperinterface.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace drawing { class XShape; }
    namespace drawing { class XShapes; }
    namespace frame { class XModel; }
    namespace uno { class XComponentContext; }
} } }

namespace ooo { namespace vba {
    class ShapeHelper;
    class XHelperInterface;
    namespace msforms { class XFillFormat; }
    namespace msforms { class XLineFormat; }
    namespace msforms { class XPictureFormat; }
} }

typedef  ::cppu::WeakImplHelper< ov::msforms::XShape, css::lang::XEventListener > ListeningShape;

typedef InheritedHelperInterfaceImpl< ListeningShape > ScVbaShape_BASE;

class VBAHELPER_DLLPUBLIC ScVbaShape : public ScVbaShape_BASE
{
protected:
    std::unique_ptr< ov::ShapeHelper > m_pShapeHelper;
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::drawing::XShapes > m_xShapes;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
    sal_Int32 const m_nType;
    css::uno::Reference< css::frame::XModel > m_xModel;
    void addListeners();
    /// @throws css::uno::RuntimeException
    void removeShapeListener();
    /// @throws css::uno::RuntimeException
    void removeShapesListener();
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
public:
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    ScVbaShape( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape >& xShape, const css::uno::Reference< css::drawing::XShapes >& xShapes, const css::uno::Reference< css::frame::XModel >& xModel, sal_Int32 nType );
    virtual ~ScVbaShape() override;

    /// @throws css::uno::RuntimeException
    static sal_Int32 getType( const css::uno::Reference< css::drawing::XShape >& rShape );

    static sal_Int32 getAutoShapeType( const css::uno::Reference< css::drawing::XShape >& rShape );

    // Attributes
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName( const OUString& _name ) override;
    virtual OUString SAL_CALL getAlternativeText() override;
    virtual void SAL_CALL setAlternativeText( const OUString& _name ) override;
    virtual double SAL_CALL getHeight() override;
    virtual void SAL_CALL setHeight(double _height) override;
    virtual double SAL_CALL getWidth() override;
    virtual void SAL_CALL setWidth(double _width) override;
    virtual double SAL_CALL getLeft() override;
    virtual void SAL_CALL setLeft( double _left ) override;
    virtual double SAL_CALL getTop() override;
    virtual void SAL_CALL setTop( double _top ) override;
    virtual sal_Bool SAL_CALL getVisible() override;
    virtual void SAL_CALL setVisible( sal_Bool _visible ) override;
    virtual sal_Int32 SAL_CALL getZOrderPosition() override;
    virtual sal_Int32 SAL_CALL getType() override;
    virtual double SAL_CALL getRotation() override;
    virtual void SAL_CALL setRotation( double _rotation ) override;
    virtual css::uno::Reference< ov::msforms::XLineFormat > SAL_CALL getLine() override;
    virtual css::uno::Reference< ov::msforms::XFillFormat > SAL_CALL getFill() override;
    virtual css::uno::Reference< ov::msforms::XPictureFormat > SAL_CALL getPictureFormat() override;
    virtual sal_Bool SAL_CALL getLockAspectRatio() override;
    virtual void SAL_CALL setLockAspectRatio( sal_Bool _lockaspectratio ) override;
    virtual sal_Bool SAL_CALL getLockAnchor() override;
    virtual void SAL_CALL setLockAnchor( sal_Bool _lockanchor ) override;
    virtual ::sal_Int32 SAL_CALL getRelativeHorizontalPosition() override;
    virtual void SAL_CALL setRelativeHorizontalPosition(::sal_Int32 _relativehorizontalposition) override;
    virtual ::sal_Int32 SAL_CALL getRelativeVerticalPosition() override;
    virtual void SAL_CALL setRelativeVerticalPosition(::sal_Int32 _relativeverticalposition) override;

    // Methods
    virtual css::uno::Any SAL_CALL TextFrame(  ) override;
    virtual css::uno::Any SAL_CALL WrapFormat(  ) override;
    virtual void SAL_CALL Delete() override;
    virtual void SAL_CALL ZOrder( sal_Int32 ZOrderCmd ) override;
    virtual void SAL_CALL IncrementRotation( double Increment ) override;
    virtual void SAL_CALL IncrementLeft( double Increment ) override;
    virtual void SAL_CALL IncrementTop( double Increment ) override;
    virtual void SAL_CALL ScaleHeight( double Factor, sal_Bool RelativeToOriginalSize, sal_Int32 Scale ) override;
    virtual void SAL_CALL ScaleWidth( double Factor, sal_Bool RelativeToOriginalSize, sal_Int32 Scale ) override;
    // Replace??
    virtual void SAL_CALL Select( const css::uno::Any& Replace ) override;
    virtual css::uno::Any SAL_CALL ShapeRange( const css::uno::Any& index ) override;
    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& rEventObject ) override;
};
#endif // INCLUDED_VBAHELPER_VBASHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
