/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#ifndef _VOS_MODULE_HXX_
#define _VOS_MODULE_HXX_

#   include <vos/object.hxx>
#   include <rtl/ustring.hxx>
#   include <osl/module.h>

namespace vos
{

class OModule : public OObject
{
    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OModule, vos));

public:

    /// default c'tor
    OModule();

    /// this c'tor is a combination of the default c'tor and load()
    OModule(const ::rtl::OUString& strModuleName, sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT);
    virtual ~OModule();

    /// loads the specified module
    sal_Bool SAL_CALL load(const ::rtl::OUString& strModuleName, sal_Int32 nRtldMode = SAL_LOADMODULE_DEFAULT);

    /// unloads the currently loaded module
    void SAL_CALL unload();

    /// returns sal_True, if a module is loaded, sal_False otherwise
    sal_Bool SAL_CALL isLoaded();

#if 0
    // not implemented yet
    // returns the name of the currently loaded module or an empty string if none.
    ::rtl::OUString SAL_CALL getModuleName();
#endif

    /// returns a pointer to the specified Symbol if found, NULL otherwise
    void* SAL_CALL getSymbol(const ::rtl::OUString& strSymbolName);

protected:

    oslModule m_Module;
};

}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
