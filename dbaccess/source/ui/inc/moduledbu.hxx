/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: moduledbu.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 08:01:59 $
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

#ifndef _DBAUI_MODULE_DBU_HXX_
#define _DBAUI_MODULE_DBU_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif

class ResMgr;

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= OModule
//=========================================================================
class OModuleImpl;
class OModule
{
    friend class OModuleClient;

private:
    OModule();
        // not implemented. OModule is a static class

protected:
    static ::osl::Mutex s_aMutex;       /// access safety
    static sal_Int32    s_nClients;     /// number of registered clients
    static OModuleImpl* s_pImpl;        /// impl class. lives as long as at least one client for the module is registered

public:
    /// get the vcl res manager of the module
    static ResMgr*  getResManager();

protected:
    /// register a client for the module
    static void registerClient();
    /// revoke a client for the module
    static void revokeClient();

private:
    /** ensure that the impl class exists
        @precond m_aMutex is guarded when this method gets called
    */
    static void ensureImpl();
};

//=========================================================================
//= OModuleClient
//=========================================================================
/** base class for objects which uses any global module-specific ressources
*/
class OModuleClient
{
public:
    OModuleClient()     { OModule::registerClient(); }
    ~OModuleClient()    { OModule::revokeClient(); }
};

//=========================================================================
//= ModuleRes
//=========================================================================
/** specialized ResId, using the ressource manager provided by the global module
*/
class ModuleRes : public ::ResId
{
public:
    ModuleRes(USHORT _nId) : ResId(_nId, *OModule::getResManager()) { }
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_MODULE_DBU_HXX_

