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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAOLEOBJECT_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAOLEOBJECT_HXX

#include <com/sun/star/drawing/XControlShape.hpp>
#include <ooo/vba/excel/XOLEObject.hpp>
#include <ooo/vba/msforms/XControl.hpp>

#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XOLEObject > OLEObjectImpl_BASE;

class ScVbaOLEObject : public OLEObjectImpl_BASE
{
protected:
    css::uno::Reference< css::drawing::XControlShape > m_xControlShape;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
    css::uno::Reference< ov::msforms::XControl> m_xControl;
public:
    ScVbaOLEObject( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext,
            css::uno::Reference< css::drawing::XControlShape > xControlShape );

    // XOLEObject Attributes
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getObject() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getEnabled() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setEnabled( sal_Bool _enabled ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException, std::exception) override;

    virtual double SAL_CALL getLeft() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLeft( double _left ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getTop() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTop( double _top ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getHeight() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHeight( double _height ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getWidth() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setWidth( double _width ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getLinkedCell() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLinkedCell( const OUString& _linkedcell ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};
#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAOLEOBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
