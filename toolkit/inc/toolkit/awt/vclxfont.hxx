/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxfont.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 12:20:03 $
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

#ifndef _TOOLKIT_AWT_VCLXFONT_HXX_
#define _TOOLKIT_AWT_VCLXFONT_HXX_

#ifndef TOOLKIT_DLLAPI_H
#include <toolkit/dllapi.h>
#endif

#ifndef _COM_SUN_STAR_AWT_XFONT2_HPP_
#include <com/sun/star/awt/XFont2.hpp>
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

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif

//  ----------------------------------------------------
//  class VCLXFont
//  ----------------------------------------------------

class TOOLKIT_DLLPUBLIC VCLXFont :  public ::com::sun::star::awt::XFont2,
                    public ::com::sun::star::lang::XTypeProvider,
                    public ::com::sun::star::lang::XUnoTunnel,
                    public ::cppu::OWeakObject
{
private:
    ::osl::Mutex    maMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice> mxDevice;
    Font            maFont;
    FontMetric*     mpFontMetric;

protected:
    BOOL            ImplAssertValidFontMetric();
    ::osl::Mutex&   GetMutex() { return maMutex; }

public:
                    VCLXFont();
                    ~VCLXFont();

    void            Init( ::com::sun::star::awt::XDevice& rxDev, const Font& rFont );
    const Font&     GetFont() const { return maFont; }

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw()  { OWeakObject::release(); }

    // ::com::sun::star::lang::XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    static VCLXFont*                                            GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw();
    sal_Int64                                                   SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XFont
    ::com::sun::star::awt::FontDescriptor           SAL_CALL getFontDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::SimpleFontMetric         SAL_CALL getFontMetric(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16                                       SAL_CALL getCharWidth( sal_Unicode c ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int16 >    SAL_CALL getCharWidths( sal_Unicode nFirst, sal_Unicode nLast ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32                                       SAL_CALL getStringWidth( const ::rtl::OUString& str ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int32                                       SAL_CALL getStringWidthArray( const ::rtl::OUString& str, ::com::sun::star::uno::Sequence< sal_Int32 >& rDXArray ) throw(::com::sun::star::uno::RuntimeException);
    void                                            SAL_CALL getKernPairs( ::com::sun::star::uno::Sequence< sal_Unicode >& rnChars1, ::com::sun::star::uno::Sequence< sal_Unicode >& rnChars2, ::com::sun::star::uno::Sequence< sal_Int16 >& rnKerns ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XFont2
    sal_Bool                                        SAL_CALL hasGlyphs( const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException);
};



#endif // _TOOLKIT_AWT_VCLXFONT_HXX_

