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
#ifndef SC_VBA_CONTROL_HXX
#define SC_VBA_CONTROL_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/script/ScriptEvent.hpp>
#include <ooo/vba/msforms/XControl.hpp>

#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbahelperinterface.hxx>
#include <memory>

//typedef ::cppu::WeakImplHelper1< ov::msforms::XControl > ControlImpl_BASE;
//template SAL_DLLPUBLIC_IMPORT InheritedHelperInterfaceImpl1< ov::msforms::XControl >;
typedef InheritedHelperInterfaceImpl1< ov::msforms::XControl > ControlImpl_BASE;

class ScVbaControl : public ControlImpl_BASE
{
private:
    com::sun::star::uno::Reference< com::sun::star::lang::XEventListener > m_xEventListener;
    com::sun::star::uno::Reference< com::sun::star::awt::XControl > m_xEmptyFormControl;
protected:
    // awt control has nothing similar to Tag property of Mso controls,
    // whether it is necessary is another question
    OUString m_aControlTag;

    bool bIsDialog;
    OUString m_sLibraryAndCodeName;
    std::auto_ptr< ov::AbstractGeometryAttributes > mpGeometryHelper;
    css::uno::Reference< css::beans::XPropertySet > m_xProps;
    css::uno::Reference< css::uno::XInterface > m_xControl;
    css::uno::Reference< css::frame::XModel > m_xModel;

    virtual css::uno::Reference< css::awt::XWindowPeer > getWindowPeer() throw (css::uno::RuntimeException);
    void fireChangeEvent();
    void fireClickEvent();
    void fireEvent( css::script::ScriptEvent& evt );
public:
    ScVbaControl( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    const css::uno::Reference< css::uno::XInterface >& xControl, const css::uno::Reference< css::frame::XModel >& xModel, ov::AbstractGeometryAttributes* pHelper );
    virtual ~ScVbaControl();
    // This class will own the helper, so make sure it is allocated from
    // the heap
    void setGeometryHelper( ov::AbstractGeometryAttributes* pHelper );
    // sets the name of the associated library ( used for UserForm controls )
    void setLibraryAndCodeName( const OUString& sLibCodeName ) { m_sLibraryAndCodeName = sLibCodeName; }
    OUString getLibraryAndCodeName() const { return m_sLibraryAndCodeName; }

    // XControl
    virtual sal_Bool SAL_CALL getEnabled() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEnabled( sal_Bool _enabled ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getHeight() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHeight( double _height ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getWidth() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setWidth( double _width ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getLeft() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLeft( double _left ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getTop() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTop( double _top ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL SetFocus(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Move( double Left, double Top, const ::com::sun::star::uno::Any& Width, const ::com::sun::star::uno::Any& Height ) throw (::com::sun::star::uno::RuntimeException);

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getObject() throw (css::uno::RuntimeException);
    virtual OUString SAL_CALL getControlSource() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setControlSource( const OUString& _controlsource ) throw (css::uno::RuntimeException);
    virtual OUString SAL_CALL getRowSource() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRowSource( const OUString& _rowsource ) throw (css::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const OUString& _name ) throw (css::uno::RuntimeException);
    virtual OUString SAL_CALL getControlTipText() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setControlTipText( const OUString& ) throw (css::uno::RuntimeException);
    virtual OUString SAL_CALL getTag() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTag( const OUString& aTag ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getTabIndex() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTabIndex( sal_Int32 nTabIndex ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getMousePointer() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMousePointer( ::sal_Int32 _mousepointer ) throw (::com::sun::star::uno::RuntimeException);
    //remove resouce because ooo.vba.excel.XControl is a wrapper of com.sun.star.drawing.XControlShape
    virtual void removeResouce() throw( css::uno::RuntimeException );
    virtual ::sal_Int32 SAL_CALL getForeColor() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setForeColor( ::sal_Int32 _forecolor ) throw (::com::sun::star::uno::RuntimeException);
    //XHelperInterface
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();
    //General helper methods for properties ( may or maynot be relevant for all
    //controls
    sal_Int32 getBackColor() throw (css::uno::RuntimeException);
    void setBackColor( sal_Int32 nBackColor ) throw (css::uno::RuntimeException);
    sal_Bool getAutoSize() throw (css::uno::RuntimeException);
    void setAutoSize( sal_Bool bAutoSize ) throw (css::uno::RuntimeException);
    sal_Bool getLocked() throw (css::uno::RuntimeException);
    void setLocked( sal_Bool bAutoSize ) throw (css::uno::RuntimeException);
};


class ScVbaControlFactory
{
public:
    static css::uno::Reference< ov::msforms::XControl > createShapeControl(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::drawing::XControlShape >& xControlShape,
        const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);

    static css::uno::Reference< ov::msforms::XControl > createUserformControl(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::awt::XControl >& xControl,
        const css::uno::Reference< css::awt::XControl >& xDialog,
        const css::uno::Reference< css::frame::XModel >& xModel,
        double fOffsetX, double fOffsetY ) throw (css::uno::RuntimeException);

private:
    ScVbaControlFactory();
    ~ScVbaControlFactory();
};

#endif//SC_VBA_CONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
