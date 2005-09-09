/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: module.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:07:37 $
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


#include <vos/module.hxx>
#include <vos/diagnose.hxx>

#ifdef _USE_NAMESPACE
using namespace vos;
#endif

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
    VOS_ASSERT(ustrModuleName);

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
    VOS_ASSERT(strSymbolName);
    VOS_ASSERT(m_Module);
    return ( osl_getSymbol( m_Module, strSymbolName.pData ) );
}

