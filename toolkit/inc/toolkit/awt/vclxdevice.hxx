/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _TOOLKIT_AWT_VCLXDEVICE_HXX_
#define _TOOLKIT_AWT_VCLXDEVICE_HXX_

#include <toolkit/dllapi.h>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>

#include <com/sun/star/awt/XUnitConversion.hpp>

class OutputDevice;
class VirtualDevice;

//  ----------------------------------------------------
//  class VCLXDevice
//  ----------------------------------------------------

// For using nDummy, no incompatible update, add a sal_Bool bCreatedWithToolkitMember later...
#define FLAGS_CREATEDWITHTOOLKIT    0x00000001

class TOOLKIT_DLLPUBLIC VCLXDevice :    public ::com::sun::star::awt::XDevice,
                    public ::com::sun::star::lang::XTypeProvider,
                    public ::com::sun::star::lang::XUnoTunnel,
                    public ::com::sun::star::awt::XUnitConversion,
                    public ::cppu::OWeakObject
{
    friend class VCLXGraphics;

private:
    OutputDevice*           mpOutputDevice;

public:
    void*                   pDummy;
    sal_uInt32              nFlags;

protected:
    void                    DestroyOutputDevice();

public:
                            VCLXDevice();
                            ~VCLXDevice();

    void                    SetOutputDevice( OutputDevice* pOutDev ) { mpOutputDevice = pOutDev; }
    OutputDevice*           GetOutputDevice() const { return mpOutputDevice; }

    void                    SetCreatedWithToolkit( sal_Bool bCreatedWithToolkit );

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw()  { OWeakObject::release(); }

    // ::com::sun::star::lang::XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    static VCLXDevice*                                          GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw();
    sal_Int64                                                   SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XDevice,
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >    SAL_CALL createGraphics(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >      SAL_CALL createDevice( sal_Int32 nWidth, sal_Int32 nHeight ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::DeviceInfo                                       SAL_CALL getInfo() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::awt::FontDescriptor > SAL_CALL getFontDescriptors(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont >        SAL_CALL getFont( const ::com::sun::star::awt::FontDescriptor& aDescriptor ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >      SAL_CALL createBitmap( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayBitmap > SAL_CALL createDisplayBitmap( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >& Bitmap ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XUnitConversion
    ::com::sun::star::awt::Point SAL_CALL convertPointToLogic( const ::com::sun::star::awt::Point& aPoint, ::sal_Int16 TargetUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Point SAL_CALL convertPointToPixel( const ::com::sun::star::awt::Point& aPoint, ::sal_Int16 SourceUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL convertSizeToLogic( const ::com::sun::star::awt::Size& aSize, ::sal_Int16 TargetUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL convertSizeToPixel( const ::com::sun::star::awt::Size& aSize, ::sal_Int16 SourceUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);


};

//  ----------------------------------------------------
//  class VCLXVirtualDevice
//  ----------------------------------------------------

class VCLXVirtualDevice : public VCLXDevice
{
public:
                    ~VCLXVirtualDevice();

    void            SetVirtualDevice( VirtualDevice* pVDev ) { SetOutputDevice( (OutputDevice*)pVDev ); }
};




#endif // _TOOLKIT_AWT_VCLXDEVICE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
