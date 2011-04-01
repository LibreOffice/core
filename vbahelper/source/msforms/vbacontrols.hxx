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
#ifndef SC_VBA_CONTROLS_HXX
#define SC_VBA_CONTROLS_HXX

#include <cppuhelper/implbase1.hxx>
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
    virtual void SAL_CALL Move( double cx, double cy ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Add( const css::uno::Any& Object, const css::uno::Any& StringKey, const css::uno::Any& Before, const css::uno::Any& After ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Remove( const css::uno::Any& StringKeyOrIndex ) throw (css::uno::RuntimeException);

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

private:
    css::uno::Reference< css::awt::XControl > mxDialog;
    css::uno::Reference< css::frame::XModel > mxModel;
    double mfOffsetX;
    double mfOffsetY;
};

#endif //SC_VBA_OLEOBJECTS_HXX

