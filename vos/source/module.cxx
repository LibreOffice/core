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


#include <vos/module.hxx>
#include <osl/diagnose.h>

using namespace vos;

VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OModule, vos),
                        VOS_NAMESPACE(OModule, vos),
                        VOS_NAMESPACE(OObject, vos), 0);


OModule::OModule()
    :m_Module(0)
{
}

OModule::OModule(const rtl::OUString& ustrModuleName, sal_Int32 nRtldMode) : m_Module(0)
{
    if (ustrModuleName)
        load(ustrModuleName, nRtldMode);
}

OModule::~OModule()
{
    if (m_Module)
        osl_unloadModule(m_Module);
}

sal_Bool OModule::load(const rtl::OUString& ustrModuleName, sal_Int32 nRtldMode)
{
    OSL_ASSERT(ustrModuleName);

    unload();

    m_Module = osl_loadModule( ustrModuleName.pData, nRtldMode );

    return (m_Module != 0);
}

void OModule::unload()
{
    if (m_Module)
    {
        osl_unloadModule(m_Module);
        m_Module = 0;
    }
}

sal_Bool OModule::isLoaded()
{
    return m_Module != NULL;
}

void *OModule::getSymbol(const rtl::OUString& strSymbolName)
{
    OSL_ASSERT(strSymbolName);
    OSL_ASSERT(m_Module);
    return ( osl_getSymbol( m_Module, strSymbolName.pData ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
