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
#ifndef INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBACONTROL_HXX
#define INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBACONTROL_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/script/ScriptEvent.hpp>
#include <ooo/vba/msforms/XControl.hpp>

#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbahelperinterface.hxx>
#include <memory>

typedef InheritedHelperInterfaceWeakImpl< ov::msforms::XControl > ControlImpl_BASE;

class ScVbaControl : public ControlImpl_BASE
{
private:
    css::uno::Reference< css::lang::XEventListener > m_xEventListener;
    css::uno::Reference< css::awt::XControl > m_xEmptyFormControl;
protected:
    // awt control has nothing similar to Tag property of Mso controls,
    // whether it is necessary is another question
    OUString m_aControlTag;

    OUString m_sLibraryAndCodeName;
    std::unique_ptr< ov::AbstractGeometryAttributes > mpGeometryHelper;
    css::uno::Reference< css::beans::XPropertySet > m_xProps;
    css::uno::Reference< cpo::uno::XInterface > m_xControl;
    css::uno::Reference< css::frame::XModel > m_xModel;

    /// @throws cpo::uno::RuntimeException
    css::uno::Reference< css::awt::XVclWindowPeer > getWindowPeer();
    void fireChangeEvent();
    void fireClickEvent();
public:
    ScVbaControl( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< cpo::uno::XComponentContext >& xContext,
                    css::uno::Reference< cpo::uno::XInterface >  xControl, css::uno::Reference< css::frame::XModel > xModel, std::unique_ptr<ov::AbstractGeometryAttributes> pHelper );
    virtual ~ScVbaControl() override;
    // This class will own the helper, so make sure it is allocated from
    // the heap
    void setGeometryHelper( std::unique_ptr<ov::AbstractGeometryAttributes> pHelper );
    // sets the name of the associated library ( used for UserForm controls )
    void setLibraryAndCodeName( const OUString& sLibCodeName ) { m_sLibraryAndCodeName = sLibCodeName; }

    // XControl
    virtual bool getEnabled() override;
    virtual void setEnabled( bool _enabled ) override;
    virtual bool getVisible() override;
    virtual void setVisible( bool _visible ) override;
    virtual double getHeight() override;
    virtual void setHeight( double _height ) override;
    virtual double getWidth() override;
    virtual void setWidth( double _width ) override;
    virtual double getLeft() override;
    virtual void setLeft( double _left ) override;
    virtual double getTop() override;
    virtual void setTop( double _top ) override;
    virtual void SetFocus(  ) override;
    virtual void Move( double Left, double Top, const cpo::uno::Any& Width, const cpo::uno::Any& Height ) override;
    virtual void fireEvent( const css::script::ScriptEvent& evt ) override;

    virtual css::uno::Reference< cpo::uno::XInterface > getObject() override;
    virtual OUString getControlSource() override;
    virtual void setControlSource( const OUString& _controlsource ) override;
    virtual OUString getRowSource() override;
    virtual void setRowSource( const OUString& _rowsource ) override;
    virtual OUString getName() override;
    virtual void setName( const OUString& _name ) override;
    virtual OUString getControlTipText() override;
    virtual void setControlTipText( const OUString& ) override;
    virtual OUString getTag() override;
    virtual void setTag( const OUString& aTag ) override;
    virtual sal_Int32 getTabIndex() override;
    virtual void setTabIndex( sal_Int32 nTabIndex ) override;
    virtual ::sal_Int32 getMousePointer() override;
    virtual void setMousePointer( ::sal_Int32 _mousepointer ) override;
    //remove resource because ooo.vba.excel.XControl is a wrapper of com.sun.star.drawing.XControlShape
    /// @throws cpo::uno::RuntimeException
    void removeResource();
    /// @throws cpo::uno::RuntimeException
    virtual ::sal_Int32 getForeColor();
    //XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
    //General helper methods for properties (may or may not be relevant for all
    //controls)
    /// @throws cpo::uno::RuntimeException
    sal_Int32 getBackColor();
    /// @throws cpo::uno::RuntimeException
    void setBackColor( sal_Int32 nBackColor );
    /// @throws cpo::uno::RuntimeException
    bool getAutoSize() const;
    /// @throws cpo::uno::RuntimeException
    void setAutoSize( bool bAutoSize );
    /// @throws cpo::uno::RuntimeException
    bool getLocked();
    /// @throws cpo::uno::RuntimeException
    void setLocked( bool bAutoSize );
};


namespace ScVbaControlFactory
{
    /// @throws cpo::uno::RuntimeException
    css::uno::Reference< ov::msforms::XControl > createShapeControl(
        const css::uno::Reference< cpo::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::drawing::XControlShape >& xControlShape,
        const css::uno::Reference< css::frame::XModel >& xModel );

    /// @throws cpo::uno::RuntimeException
    css::uno::Reference< ov::msforms::XControl > createUserformControl(
        const css::uno::Reference< cpo::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::awt::XControl >& xControl,
        const css::uno::Reference< css::awt::XControl >& xDialog,
        const css::uno::Reference< css::frame::XModel >& xModel,
        double fOffsetX, double fOffsetY );
}

#endif // INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBACONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
