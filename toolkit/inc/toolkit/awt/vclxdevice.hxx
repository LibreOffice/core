/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxdevice.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:43:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _TOOLKIT_AWT_VCLXDEVICE_HXX_
#define _TOOLKIT_AWT_VCLXDEVICE_HXX_

#ifndef TOOLKIT_DLLAPI_H
#include <toolkit/dllapi.h>
#endif

#ifndef _COM_SUN_STAR_AWT_XDEVICE_HPP_
#include <com/sun/star/awt/XDevice.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

class OutputDevice;
class VirtualDevice;

//  ----------------------------------------------------
//  class VCLXDevice
//  ----------------------------------------------------

// For using nDummy, no incompatible update, add a BOOL bCreatedWithToolkitMember later...
#define FLAGS_CREATEDWITHTOOLKIT    0x00000001

class TOOLKIT_DLLPUBLIC VCLXDevice :    public ::com::sun::star::awt::XDevice,
                    public ::com::sun::star::lang::XTypeProvider,
                    public ::com::sun::star::lang::XUnoTunnel,
                    public ::cppu::OWeakObject
{
    friend class VCLXGraphics;

private:
    NAMESPACE_VOS(IMutex)&  mrMutex;    // Reference to SolarMutex
    OutputDevice*           mpOutputDevice;

public:
    void*                   pDummy;
    sal_uInt32              nFlags;

protected:
    NAMESPACE_VOS(IMutex)&  GetMutex() { return mrMutex; }
    void                    DestroyOutputDevice();

public:
                            VCLXDevice();
                            ~VCLXDevice();

    void                    SetOutputDevice( OutputDevice* pOutDev ) { mpOutputDevice = pOutDev; }
    OutputDevice*           GetOutputDevice() const { return mpOutputDevice; }

    void                    SetCreatedWithToolkit( sal_Bool bCreatedWithToolkit );
    sal_Bool                IsCreatedWithToolkit() const;

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
};

//  ----------------------------------------------------
//  class VCLXVirtualDevice
//  ----------------------------------------------------

class VCLXVirtualDevice : public VCLXDevice
{
private:
    VirtualDevice*  mpVDev;

public:
                    ~VCLXVirtualDevice();

    void            SetVirtualDevice( VirtualDevice* pVDev ) { SetOutputDevice( (OutputDevice*)pVDev ); }
};




#endif // _TOOLKIT_AWT_VCLXDEVICE_HXX_

