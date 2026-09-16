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
#pragma once

#include <com/sun/star/drawing/XControlShape.hpp>
#include <ooo/vba/excel/XOLEObject.hpp>
#include <ooo/vba/msforms/XControl.hpp>

#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XOLEObject > OLEObjectImpl_BASE;

class ScVbaOLEObject final : public OLEObjectImpl_BASE
{
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
    cpo::uno::Reference< ov::msforms::XControl> m_xControl;
public:
    ScVbaOLEObject( const cpo::uno::Reference< ov::XHelperInterface >& xParent, const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext,
            cpo::uno::Reference< css::drawing::XControlShape > const & xControlShape );

    // XOLEObject Attributes
    virtual cpo::uno::Reference< cpo::uno::XInterface > getObject() override;
    virtual bool getEnabled() override;
    virtual void setEnabled( bool _enabled ) override;
    virtual bool getVisible() override;
    virtual void setVisible( bool _visible ) override;

    virtual double getLeft() override;
    virtual void setLeft( double _left ) override;
    virtual double getTop() override;
    virtual void setTop( double _top ) override;
    virtual double getHeight() override;
    virtual void setHeight( double _height ) override;
    virtual double getWidth() override;
    virtual void setWidth( double _width ) override;
    virtual OUString getLinkedCell() override;
    virtual void setLinkedCell( const OUString& _linkedcell ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
