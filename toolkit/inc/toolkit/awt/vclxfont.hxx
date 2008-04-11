/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxfont.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _TOOLKIT_AWT_VCLXFONT_HXX_
#define _TOOLKIT_AWT_VCLXFONT_HXX_

#include <toolkit/dllapi.h>
#include <com/sun/star/awt/XFont2.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <vcl/metric.hxx>

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

