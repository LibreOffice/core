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

#include <cppuhelper/implbase.hxx>
#include <ooo/vba/msforms/XCommandButton.hpp>

#include "vbacontrol.hxx"
#include <vbahelper/vbahelper.hxx>

typedef cppu::ImplInheritanceHelper< ScVbaControl, ov::msforms::XCommandButton > ButtonImpl_BASE;

class VbaButton : public ButtonImpl_BASE
{
public:
    VbaButton( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< cpo::uno::XComponentContext >& xContext, const css::uno::Reference< cpo::uno::XInterface >& xControl, const css::uno::Reference< css::frame::XModel >& xModel, std::unique_ptr<ov::AbstractGeometryAttributes> pGeomHelper  );
   // Attributes
    virtual OUString getCaption() override;
    virtual void setCaption( const OUString& _caption ) override;
    virtual bool getAutoSize() override;
    virtual void setAutoSize( bool bAutoSize ) override;
    virtual bool getCancel() override;
    virtual void setCancel( bool bCancel ) override;
    virtual bool getDefault() override;
    virtual void setDefault( bool bDefault ) override;
    virtual sal_Int32 getBackColor() override;
    virtual void setBackColor( sal_Int32 nBackColor ) override;
    virtual sal_Int32 getForeColor() override;
    virtual void setForeColor( sal_Int32 nForeColor ) override;
    virtual bool getLocked() override;
    virtual void setLocked( bool bAutoSize ) override;
    virtual css::uno::Reference< ov::msforms::XNewFont > getFont() override;
    //XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
