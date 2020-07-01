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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXDEVICE_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXDEVICE_HXX

#include <toolkit/dllapi.h>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <vcl/virdev.hxx>
#include <vcl/vclptr.hxx>

#include <com/sun/star/awt/XUnitConversion.hpp>

/// A UNO wrapper for the VCL OutputDevice
class TOOLKIT_DLLPUBLIC VCLXDevice :
                    public cppu::WeakImplHelper<
                        css::awt::XDevice,
                        css::lang::XUnoTunnel,
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

    // css::lang::XUnoTunnel
    UNO3_GETIMPLEMENTATION_DECL(VCLXDevice)

    // css::awt::XDevice,
    css::uno::Reference< css::awt::XGraphics >    SAL_CALL createGraphics(  ) override;
    css::uno::Reference< css::awt::XDevice >      SAL_CALL createDevice( sal_Int32 nWidth, sal_Int32 nHeight ) override;
    css::awt::DeviceInfo                                       SAL_CALL getInfo() override;
    css::uno::Sequence< css::awt::FontDescriptor > SAL_CALL getFontDescriptors(  ) override;
    css::uno::Reference< css::awt::XFont >        SAL_CALL getFont( const css::awt::FontDescriptor& aDescriptor ) override;
    css::uno::Reference< css::awt::XBitmap >      SAL_CALL createBitmap( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight ) override;
    css::uno::Reference< css::awt::XDisplayBitmap > SAL_CALL createDisplayBitmap( const css::uno::Reference< css::awt::XBitmap >& Bitmap ) override;

    // css::awt::XUnitConversion
    css::awt::Point SAL_CALL convertPointToLogic( const css::awt::Point& aPoint, ::sal_Int16 TargetUnit ) override;
    css::awt::Point SAL_CALL convertPointToPixel( const css::awt::Point& aPoint, ::sal_Int16 SourceUnit ) override;
    css::awt::Size SAL_CALL convertSizeToLogic( const css::awt::Size& aSize, ::sal_Int16 TargetUnit ) override;
    css::awt::Size SAL_CALL convertSizeToPixel( const css::awt::Size& aSize, ::sal_Int16 SourceUnit ) override;


};




class VCLXVirtualDevice : public VCLXDevice
{
public:
                    virtual ~VCLXVirtualDevice() override;

    void            SetVirtualDevice( VirtualDevice* pVDev ) { SetOutputDevice( pVDev ); }
};


#endif // INCLUDED_TOOLKIT_AWT_VCLXDEVICE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
