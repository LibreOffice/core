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

#include <toolkit/dllapi.h>
#include <com/sun/star/awt/XDevice.hpp>
#include <cppuhelper/implbase.hxx>
#include <vcl/virdev.hxx>
#include <vcl/vclptr.hxx>

#include <com/sun/star/awt/XUnitConversion.hpp>

/// A UNO wrapper for the VCL OutputDevice
class TOOLKIT_DLLPUBLIC VCLXDevice :
                    public cppu::WeakImplHelper<
                        css::awt::XDevice,
                        css::awt::XUnitConversion>
{
    friend class VCLXGraphics;
    friend class VCLXVirtualDevice;

private:
    VclPtr<OutputDevice>    mpOutputDevice;

public:
                            VCLXDevice();
                            virtual ~VCLXDevice() override;

    void                    SetOutputDevice( const VclPtr<OutputDevice> &pOutDev ) { mpOutputDevice = pOutDev; }
    const VclPtr<OutputDevice>& GetOutputDevice() const { return mpOutputDevice; }

    // css::awt::XDevice,
    cpo::uno::Reference< css::awt::XGraphics >    createGraphics(  ) override;
    css::awt::DeviceInfo                                       getInfo() override;
    cpo::uno::Reference< css::awt::XFont >        getFont( const css::awt::FontDescriptor& aDescriptor ) override;

    // css::awt::XUnitConversion
    css::awt::Point convertPointToLogic( const css::awt::Point& aPoint, ::sal_Int16 TargetUnit ) override;
    css::awt::Point convertPointToPixel( const css::awt::Point& aPoint, ::sal_Int16 SourceUnit ) override;
    css::awt::Size convertSizeToLogic( const css::awt::Size& aSize, ::sal_Int16 TargetUnit ) override;
    css::awt::Size convertSizeToPixel( const css::awt::Size& aSize, ::sal_Int16 SourceUnit ) override;


};




class VCLXVirtualDevice final : public VCLXDevice
{
public:
                    virtual ~VCLXVirtualDevice() override;

    void            SetVirtualDevice( VirtualDevice* pVDev ) { SetOutputDevice( pVDev ); }
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
