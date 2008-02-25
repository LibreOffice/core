/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ModuleHelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-25 16:26:39 $
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
#ifndef _RPTUI_MODULE_HELPER_RPT_HXX_
#include "ModuleHelper.hxx"
#endif
#ifndef _COMPHELPER_CONFIGURATIONHELPER_HXX_
#include <comphelper/configurationhelper.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMACROEXPANDER_HPP_
#include <com/sun/star/util/XMacroExpander.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SOLAR_HRC
#include <svtools/solar.hrc>
#endif

#define EXPAND_PROTOCOL     "vnd.sun.star.expand:"
#define ENTER_MOD_METHOD()  \
    ::osl::MutexGuard aGuard(s_aMutex); \
    ensureImpl()

//.........................................................................
namespace rptui
{
//.........................................................................
    using namespace ::com::sun::star;
//=========================================================================
//= OModuleImpl
//=========================================================================
/** implementation for <type>OModule</type>. not threadsafe, has to be guarded by it's owner
*/
class OModuleImpl
{
    ResMgr* m_pRessources;

public:
    /// ctor
    OModuleImpl();
    ~OModuleImpl();

    /// get the manager for the ressources of the module
    ResMgr* getResManager();
};

DBG_NAME( rpt_OModuleImpl )
//-------------------------------------------------------------------------
OModuleImpl::OModuleImpl()
    :m_pRessources(NULL)
{
    DBG_CTOR( rpt_OModuleImpl,NULL);

}

//-------------------------------------------------------------------------
OModuleImpl::~OModuleImpl()
{
    if (m_pRessources)
        delete m_pRessources;

    DBG_DTOR( rpt_OModuleImpl,NULL);
}

//-------------------------------------------------------------------------
ResMgr* OModuleImpl::getResManager()
{
    // note that this method is not threadsafe, which counts for the whole class !

    if (!m_pRessources)
    {
        // create a manager with a fixed prefix
        rtl::OString sName = rtl::OString( "rptui" );
        m_pRessources = ResMgr::CreateResMgr(sName);
    }
    return m_pRessources;
}

//=========================================================================
//= OModule
//=========================================================================
::osl::Mutex    OModule::s_aMutex;
sal_Int32       OModule::s_nClients = 0;
OModuleImpl*    OModule::s_pImpl = NULL;
//-------------------------------------------------------------------------
ResMgr* OModule::getResManager()
{
    ENTER_MOD_METHOD();
    return s_pImpl->getResManager();
}

//-------------------------------------------------------------------------
void OModule::registerClient()
{
    ::osl::MutexGuard aGuard(s_aMutex);
    ++s_nClients;
}

//-------------------------------------------------------------------------
void OModule::revokeClient()
{
    ::osl::MutexGuard aGuard(s_aMutex);
    if (!--s_nClients && s_pImpl)
    {
        delete s_pImpl;
        s_pImpl = NULL;
    }
}

//-------------------------------------------------------------------------
void OModule::ensureImpl()
{
    if (s_pImpl)
        return;
    s_pImpl = new OModuleImpl();
}

//.........................................................................
}   // namespace dbaui
//.........................................................................
