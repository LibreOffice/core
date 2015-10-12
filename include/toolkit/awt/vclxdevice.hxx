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
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <vcl/virdev.hxx>
#include <vcl/vclptr.hxx>

#include <com/sun/star/awt/XUnitConversion.hpp>

class OutputDevice;
class VirtualDevice;

// For using nDummy, no incompatible update, add a sal_Bool bCreatedWithToolkitMember later...
#define FLAGS_CREATEDWITHTOOLKIT    0x00000001

/// An UNO wrapper for the VCL OutputDevice
class TOOLKIT_DLLPUBLIC VCLXDevice :
                    public ::com::sun::star::awt::XDevice,
                    public ::com::sun::star::lang::XTypeProvider,
                    public ::com::sun::star::lang::XUnoTunnel,
                    public ::com::sun::star::awt::XUnitConversion,
                    public ::cppu::OWeakObject
{
    friend class VCLXGraphics;
    friend class VCLXVirtualDevice;

private:
    VclPtr<OutputDevice>    mpOutputDevice;

public:
    void*                   pDummy;
    sal_uInt32              nFlags;

public:
                            VCLXDevice();
                            virtual ~VCLXDevice();

    void                    SetOutputDevice( const VclPtr<OutputDevice> &pOutDev ) { mpOutputDevice = pOutDev; }
    VclPtr<OutputDevice>    GetOutputDevice() const { return mpOutputDevice; }

    void                    SetCreatedWithToolkit( bool bCreatedWithToolkit );

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // ::com::sun::star::lang::XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    static VCLXDevice*                                          GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace );
    sal_Int64                                                   SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XDevice,
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >    SAL_CALL createGraphics(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >      SAL_CALL createDevice( sal_Int32 nWidth, sal_Int32 nHeight ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::awt::DeviceInfo                                       SAL_CALL getInfo() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< ::com::sun::star::awt::FontDescriptor > SAL_CALL getFontDescriptors(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont >        SAL_CALL getFont( const ::com::sun::star::awt::FontDescriptor& aDescriptor ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >      SAL_CALL createBitmap( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayBitmap > SAL_CALL createDisplayBitmap( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >& Bitmap ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XUnitConversion
    ::com::sun::star::awt::Point SAL_CALL convertPointToLogic( const ::com::sun::star::awt::Point& aPoint, ::sal_Int16 TargetUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::awt::Point SAL_CALL convertPointToPixel( const ::com::sun::star::awt::Point& aPoint, ::sal_Int16 SourceUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::awt::Size SAL_CALL convertSizeToLogic( const ::com::sun::star::awt::Size& aSize, ::sal_Int16 TargetUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::awt::Size SAL_CALL convertSizeToPixel( const ::com::sun::star::awt::Size& aSize, ::sal_Int16 SourceUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;


};


//  class VCLXVirtualDevice


class VCLXVirtualDevice : public VCLXDevice
{
public:
                    virtual ~VCLXVirtualDevice();

    void            SetVirtualDevice( VirtualDevice* pVDev ) { SetOutputDevice( pVDev ); }
};




#endif // INCLUDED_TOOLKIT_AWT_VCLXDEVICE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
