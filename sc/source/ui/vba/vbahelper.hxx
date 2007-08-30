/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbahelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-08-30 10:04:41 $
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
#ifndef SC_VBA_HELPER_HXX
#define SC_VBA_HELPER_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <docsh.hxx>

namespace css = ::com::sun::star;

namespace org
{
    namespace openoffice
    {
        void dispatchRequests (css::uno::Reference< css::frame::XModel>& xModel,rtl::OUString & aUrl) ;
        void dispatchRequests (css::uno::Reference< css::frame::XModel>& xModel,rtl::OUString & aUrl, css::uno::Sequence< css::beans::PropertyValue >& sProps ) ;
        void implnCopy();
        void implnPaste();
        void implnCut();
        void implnPasteSpecial(sal_uInt16 nFlags,sal_uInt16 nFunction,sal_Bool bSkipEmpty, sal_Bool bTranspose);
        bool isRangeShortCut( const ::rtl::OUString& sParam );
        css::uno::Reference< css::frame::XModel >
            getCurrentDocument() throw (css::uno::RuntimeException);
        ScTabViewShell* getBestViewShell( css::uno::Reference< css::frame::XModel>& xModel ) ;
        ScDocShell* getDocShell( css::uno::Reference< css::frame::XModel>& xModel ) ;
        ScTabViewShell* getCurrentBestViewShell();
        SfxViewFrame* getCurrentViewFrame();
        sal_Int32 OORGBToXLRGB( sal_Int32 );
        sal_Int32 XLRGBToOORGB( sal_Int32 );
        css::uno::Any OORGBToXLRGB( const css::uno::Any& );
        css::uno::Any XLRGBToOORGB( const css::uno::Any& );
        // provide a NULL object that can be passed as variant so that
        // the object when passed to IsNull will return true. aNULL
        // contains an empty object reference
        const css::uno::Any& aNULL();
        void PrintOutHelper( const css::uno::Any& From, const css::uno::Any& To, const css::uno::Any& Copies, const css::uno::Any& Preview, const css::uno::Any& ActivePrinter, const css::uno::Any& PrintToFile, const css::uno::Any& Collate, const css::uno::Any& PrToFileName, css::uno::Reference< css::frame::XModel >& xModel, sal_Bool bSelection  );
    }
}

namespace oo = org::openoffice;

#ifdef DEBUG
#  define SC_VBA_FIXME(a) OSL_TRACE( a )
#  define SC_VBA_STUB() SC_VBA_FIXME(( "%s - stubbed\n", __FUNCTION__ ))
#else
#  define SC_VBA_FIXME(a)
#  define SC_VBA_STUB()
#endif

#endif
