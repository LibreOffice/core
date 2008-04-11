/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: configaccess.hxx,v $
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

#ifndef __FRAMEWORK_CONFIG_CONFIGACCESS_HXX_
#define __FRAMEWORK_CONFIG_CONFIGACCESS_HXX_

//_______________________________________
// my own includes

#include <threadhelp/threadhelpbase.hxx>
#include <macros/debug.hxx>
#include <general.h>

//_______________________________________
// interface includes
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_______________________________________
// other includes
#include <rtl/ustring.hxx>

//_______________________________________
// namespace

namespace framework{

//_______________________________________
// public const

//_______________________________________
/**
    @short  implements a simple configuration access
    @descr  Sometimes it's better to have direct config access
            instead of using soecialize config items of the svtools
            project. This class can wrapp such configuration access.
 */
class ConfigAccess : public ThreadHelpBase
{
    //___________________________________
    // const

    public:

        /** represent the possible modes of the internal wrapped configuration access */
        enum EOpenMode
        {
            /// config isn't used yet
            E_CLOSED,
            /// config access is open for reading only
            E_READONLY,
            /// config access is open for reading/writing data
            E_READWRITE
        };

    //___________________________________
    // member

    private:

        /**
            reference to the uno service manager
            It's neccessary to instanciate own needed services.
         */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** hold an opened configuration alive */
        css::uno::Reference< css::uno::XInterface > m_xConfig;

        /** knows the root of the opened config access point */
        ::rtl::OUString m_sRoot;

        /** represent the current open mode */
        EOpenMode m_eMode;

    //___________________________________
    // native interface methods

    public:

                 ConfigAccess( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ,
                               const ::rtl::OUString&                                        sRoot );
        virtual ~ConfigAccess();

        virtual void      open   ( EOpenMode eMode );
        virtual void      close  (                 );
        virtual EOpenMode getMode(                 ) const;

        virtual const css::uno::Reference< css::uno::XInterface >& cfg();
};

} // namespace framework

#endif // __FRAMEWORK_CONFIG_CONFIGACCESS_HXX_
