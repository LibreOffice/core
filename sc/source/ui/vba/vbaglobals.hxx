/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaglobals.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:06:51 $
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
#ifndef SC_VBA_GLOBALS
#define SC_VBA_GLOBALS

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <org/openoffice/vba/XGlobals.hpp>

#include <cppuhelper/implbase2.hxx>
#include "vbahelper.hxx"

    // =============================================================================
    // class ScVbaGlobals
    // =============================================================================

    typedef ::cppu::WeakImplHelper2<
        oo::vba::XGlobals,
        css::lang::XServiceInfo > ScVbaGlobals_BASE;


    class ScVbaGlobals : public ScVbaGlobals_BASE
    {
    private:
            css::uno::Reference< css::uno::XComponentContext > m_xContext;
            css::uno::Reference< oo::excel::XApplication > mxApplication;
    public:

        ScVbaGlobals(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
        virtual ~ScVbaGlobals();

        static  css::uno::Reference< oo::vba::XGlobals > getGlobalsImpl(css::uno::Reference< css::uno::XComponentContext >& ) throw (css::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  )
            throw (css::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
            throw (css::uno::RuntimeException);
        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw (css::uno::RuntimeException);

        // XGlobals
        virtual css::uno::Reference<
                        oo::excel::XApplication > SAL_CALL getApplication()
                        throw (css::uno::RuntimeException);
        virtual css::uno::Reference< oo::excel::XWorkbook > SAL_CALL getActiveWorkbook() throw (css::uno::RuntimeException);
        virtual css::uno::Reference< oo::excel::XWorksheet > SAL_CALL getActiveSheet() throw (css::uno::RuntimeException);
        virtual css::uno::Any SAL_CALL WorkSheets(const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
        virtual css::uno::Any SAL_CALL WorkBooks(const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Sheets( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getGlobals(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Range( const css::uno::Any& Cell1, const css::uno::Any& Cell2 ) throw (css::uno::RuntimeException);
    };
#endif //
