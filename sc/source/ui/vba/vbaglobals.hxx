/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vbaglobals.hxx,v $
 * $Revision: 1.4 $
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
#ifndef SC_VBA_GLOBALS
#define SC_VBA_GLOBALS

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <ooo/vba/XGlobals.hpp>

#include <cppuhelper/implbase1.hxx>
#include "vbahelper.hxx"

    // =============================================================================
    // class ScVbaGlobals
    // =============================================================================

    typedef ::cppu::WeakImplHelper1<
 ov::XGlobals > ScVbaGlobals_BASE;


    class ScVbaGlobals : public ScVbaGlobals_BASE
    {
    private:
            css::uno::Reference< css::uno::XComponentContext > m_xContext;
            css::uno::Reference< ov::excel::XApplication > mxApplication;
    public:

        ScVbaGlobals(
            css::uno::Reference< css::uno::XComponentContext >const& rxContext );
        virtual ~ScVbaGlobals();

        static  css::uno::Reference< ov::XGlobals > getGlobalsImpl(const css::uno::Reference< css::uno::XComponentContext >& ) throw (css::uno::RuntimeException);

        // XGlobals
        virtual css::uno::Reference<
 ov::excel::XApplication > SAL_CALL getApplication()
                        throw (css::uno::RuntimeException);
        virtual css::uno::Reference< ov::excel::XWorkbook > SAL_CALL getActiveWorkbook() throw (css::uno::RuntimeException);
        virtual css::uno::Reference< ov::excel::XWorksheet > SAL_CALL getActiveSheet() throw (css::uno::RuntimeException);
        virtual css::uno::Any SAL_CALL WorkSheets(const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
        virtual css::uno::Any SAL_CALL WorkBooks(const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Sheets( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getGlobals(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Range( const css::uno::Any& Cell1, const css::uno::Any& Cell2 ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Names( const css::uno::Any& aIndex ) throw ( css::uno::RuntimeException );
    };
#endif //
