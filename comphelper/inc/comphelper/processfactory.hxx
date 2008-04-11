/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: processfactory.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#define _COMPHELPER_PROCESSFACTORY_HXX_

#if ! defined(_COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HXX_)
#include "com/sun/star/uno/XComponentContext.hpp"
#endif
#include <com/sun/star/uno/Sequence.hxx>
#include "comphelper/comphelperdllapi.h"

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
} } } }

namespace comphelper
{

/**
 * This function set the process service factory.
 *
 * @author Juergen Schmidt
 */
COMPHELPER_DLLPUBLIC void setProcessServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xSMgr);

/**
 * This function get the process service factory. If no service factory is set the function returns
 * a null interface.
 *
 * @author Juergen Schmidt
 */
COMPHELPER_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getProcessServiceFactory();

/** creates a component, using the process factory if set
    @see getProcessServiceFactory
    @see setProcessServiceFactory
*/
COMPHELPER_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    createProcessComponent(
        const ::rtl::OUString& _rServiceSpecifier
    ) SAL_THROW( ( ::com::sun::star::uno::RuntimeException ) );

/** creates a component with arguments, using the process factory if set

    @see getProcessServiceFactory
    @see setProcessServiceFactory
*/
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    createProcessComponentWithArguments(
        const ::rtl::OUString& _rServiceSpecifier,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArgs
    ) SAL_THROW( ( ::com::sun::star::uno::RuntimeException ) );

}

extern "C" {
/// @internal
COMPHELPER_DLLPUBLIC
::com::sun::star::uno::XComponentContext *
comphelper_getProcessComponentContext();
} // extern "C"

#endif // _COMPHELPER_PROCESSFACTORY_HXX_

