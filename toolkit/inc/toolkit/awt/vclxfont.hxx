/*************************************************************************
 *
 *  $RCSfile: vclxfont.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLKIT_AWT_VCLXFONT_HXX_
#define _TOOLKIT_AWT_VCLXFONT_HXX_


#ifndef _COM_SUN_STAR_AWT_XFONT_HPP_
#include <com/sun/star/awt/XFont.hpp>
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

class VCLXFont :    public ::com::sun::star::awt::XFont,
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
    void                                        SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)   { OWeakObject::release(); }

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
};



#endif // _TOOLKIT_AWT_VCLXFONT_HXX_

