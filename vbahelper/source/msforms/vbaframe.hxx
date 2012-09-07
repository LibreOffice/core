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
#ifndef SC_VBA_FRAME_HXX
#define SC_VBA_FRAME_HXX
#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/msforms/XFrame.hpp>

#include "vbacontrol.hxx"
#include <vbahelper/vbahelper.hxx>

typedef cppu::ImplInheritanceHelper1< ScVbaControl, ov::msforms::XFrame > FrameImpl_BASE;

class ScVbaFrame : public FrameImpl_BASE
{
public:
    ScVbaFrame(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::uno::XInterface >& xControl,
        const css::uno::Reference< css::frame::XModel >& xModel,
        ov::AbstractGeometryAttributes* pGeomHelper,
        const css::uno::Reference< css::awt::XControl >& xDialog );

    // XFrame attributes
    virtual OUString SAL_CALL getCaption() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCaption( const OUString& _caption ) throw (css::uno::RuntimeException);

    virtual ::sal_Int32 SAL_CALL getForeColor() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setForeColor( ::sal_Int32 _forecolor ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSpecialEffect() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSpecialEffect( sal_Int32 nSpecialEffect ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBorderStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBorderStyle( sal_Int32 nBorderStyle ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XNewFont > SAL_CALL getFont() throw (css::uno::RuntimeException);
    // XFrame methods
    css::uno::Any SAL_CALL Controls( const css::uno::Any& rIndex ) throw (css::uno::RuntimeException);
    //XHelperInterface
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();

private:
    css::uno::Reference< css::awt::XControl > mxDialog;
};
#endif //SC_VBA_LABEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
