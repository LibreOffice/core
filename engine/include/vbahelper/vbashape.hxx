/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <ooo/vba/msforms/XShape.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vbahelper/vbadllapi.h>
#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbahelperinterface.hxx>

namespace com::sun::star {
    namespace beans { class XPropertySet; }
    namespace drawing { class XShape; }
    namespace drawing { class XShapes; }
    namespace frame { class XModel; }
}
namespace cpo::uno { class XComponentContext; }

namespace ooo::vba {
    class ShapeHelper;
    class XHelperInterface;
    namespace msforms { class XFillFormat; }
    namespace msforms { class XLineFormat; }
    namespace msforms { class XPictureFormat; }
}

typedef  ::cppu::WeakImplHelper< ov::msforms::XShape, css::lang::XEventListener > ListeningShape;

typedef InheritedHelperInterfaceImpl< ListeningShape > ScVbaShape_BASE;

class VBAHELPER_DLLPUBLIC ScVbaShape : public ScVbaShape_BASE
{
protected:
    ov::ShapeHelper m_aShapeHelper;
    css::uno::Reference< css::drawing::XShape > m_xShape;
    css::uno::Reference< css::drawing::XShapes > m_xShapes;
    css::uno::Reference< css::beans::XPropertySet > m_xPropertySet;
    sal_Int32 m_nType;
    css::uno::Reference< css::frame::XModel > m_xModel;
    void addListeners();
    /// @throws cpo::uno::RuntimeException
    void removeShapeListener();
    /// @throws cpo::uno::RuntimeException
    void removeShapesListener();
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
public:
    /// @throws css::lang::IllegalArgumentException
    /// @throws cpo::uno::RuntimeException
    ScVbaShape( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< cpo::uno::XComponentContext >& xContext, css::uno::Reference< css::drawing::XShape >  xShape, css::uno::Reference< css::drawing::XShapes > xShapes, css::uno::Reference< css::frame::XModel > xModel, sal_Int32 nType );
    virtual ~ScVbaShape() override;

    /// @throws cpo::uno::RuntimeException
    static sal_Int32 getType( const css::uno::Reference< css::drawing::XShape >& rShape );

    static sal_Int32 getAutoShapeType( const css::uno::Reference< css::drawing::XShape >& rShape );

    // Attributes
    virtual OUString getName() override;
    virtual void setName( const OUString& _name ) override;
    virtual OUString getAlternativeText() override;
    virtual void setAlternativeText( const OUString& _name ) override;
    virtual double getHeight() override;
    virtual void setHeight(double _height) override;
    virtual double getWidth() override;
    virtual void setWidth(double _width) override;
    virtual double getLeft() override;
    virtual void setLeft( double _left ) override;
    virtual double getTop() override;
    virtual void setTop( double _top ) override;
    virtual bool getVisible() override;
    virtual void setVisible( bool _visible ) override;
    virtual sal_Int32 getZOrderPosition() override;
    virtual sal_Int32 getType() override;
    virtual double getRotation() override;
    virtual void setRotation( double _rotation ) override;
    virtual css::uno::Reference< ov::msforms::XLineFormat > getLine() override;
    virtual css::uno::Reference< ov::msforms::XFillFormat > getFill() override;
    virtual css::uno::Reference< ov::msforms::XPictureFormat > getPictureFormat() override;
    virtual bool getLockAspectRatio() override;
    virtual void setLockAspectRatio( bool _lockaspectratio ) override;
    virtual bool getLockAnchor() override;
    virtual void setLockAnchor( bool _lockanchor ) override;
    virtual ::sal_Int32 getRelativeHorizontalPosition() override;
    virtual void setRelativeHorizontalPosition(::sal_Int32 _relativehorizontalposition) override;
    virtual ::sal_Int32 getRelativeVerticalPosition() override;
    virtual void setRelativeVerticalPosition(::sal_Int32 _relativeverticalposition) override;

    // Methods
    virtual cpo::uno::Any TextFrame(  ) override;
    virtual cpo::uno::Any WrapFormat(  ) override;
    virtual void Delete() override;
    virtual void ZOrder( sal_Int32 ZOrderCmd ) override;
    virtual void IncrementRotation( double Increment ) override;
    virtual void IncrementLeft( double Increment ) override;
    virtual void IncrementTop( double Increment ) override;
    virtual void ScaleHeight( double Factor, bool RelativeToOriginalSize, sal_Int32 Scale ) override;
    virtual void ScaleWidth( double Factor, bool RelativeToOriginalSize, sal_Int32 Scale ) override;
    // Replace??
    virtual void Select( const cpo::uno::Any& Replace ) override;
    virtual cpo::uno::Any ShapeRange( const cpo::uno::Any& index ) override;
    // XEventListener
    virtual void disposing( const css::lang::EventObject& rEventObject ) override;
};
#endif // INCLUDED_VBAHELPER_VBASHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
