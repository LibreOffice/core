/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StorageBridgeFactory.hxx,v $
 * $Revision: 1.8 $
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


#ifndef _FRAMEWORK_SCRIPT_STORAGEBRIDGEFACTORY_HXX_
#define _FRAMEWORK_SCRIPT_STORAGEBRIDGEFACTORY_HXX_

#include <rtl/ustring>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <drafts/com/sun/star/script/framework/storage/XScriptInfoAccess.hpp>

namespace scripting_runtimemgr
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

class StorageBridgeFactory
{

public:
    explicit StorageBridgeFactory(
        const css::uno::Reference < css::uno::XComponentContext > & xContext );
    css::uno::Reference < dcsssf::storage::XScriptInfoAccess >
        getStorageInstance( sal_Int32 sid );
private:
    StorageBridgeFactory(); // No definition for default ctor
    css::uno::Reference< css::uno::XComponentContext >  m_xContext;
};
} // scripting_runtimemgr
#endif //_FRAMEWORK_SCRIPT_NAMERESOLVER_HXX_
