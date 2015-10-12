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
#ifndef INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBACONTROLS_HXX
#define INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBACONTROLS_HXX

#include <ooo/vba/msforms/XControls.hpp>
#include <com/sun/star/awt/XControl.hpp>

#include <vbahelper/vbacollectionimpl.hxx>
#include <vbahelper/vbahelper.hxx>

typedef CollTestImplHelper< ov::msforms::XControls > ControlsImpl_BASE;

class ScVbaControls : public ControlsImpl_BASE
{
public:
    ScVbaControls(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::awt::XControl >& xDialog,
        const css::uno::Reference< css::frame::XModel >& xModel,
        double fOffsetX, double fOffsetY );
    // XControls
    virtual void SAL_CALL Move( double cx, double cy ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Add( const css::uno::Any& Object, const css::uno::Any& StringKey, const css::uno::Any& Before, const css::uno::Any& After ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Remove( const css::uno::Any& StringKeyOrIndex ) throw (css::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

private:
    css::uno::Reference< css::awt::XControl > mxDialog;
    css::uno::Reference< css::frame::XModel > mxModel;
    double mfOffsetX;
    double mfOffsetY;
};

#endif // INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBACONTROLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
