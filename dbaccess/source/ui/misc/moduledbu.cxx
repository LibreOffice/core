/*************************************************************************
 *
 *  $RCSfile: moduledbu.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-05 10:09:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif

#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif
#ifndef _SOLAR_HRC
#include <svtools/solar.hrc>
#endif

#define ENTER_MOD_METHOD()  \
    ::osl::MutexGuard aGuard(s_aMutex); \
    ensureImpl()

//.........................................................................
namespace dbaui
{
//.........................................................................

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

//-------------------------------------------------------------------------
OModuleImpl::OModuleImpl()
    :m_pRessources(NULL)
{
}

//-------------------------------------------------------------------------
OModuleImpl::~OModuleImpl()
{
    if (m_pRessources)
        delete m_pRessources;
}

//-------------------------------------------------------------------------
ResMgr* OModuleImpl::getResManager()
{
    // note that this method is not threadsafe, which counts for the whole class !

    if (!m_pRessources)
    {
        // create a manager with a fixed prefix
        ByteString aMgrName = ByteString( "dbu" );
        aMgrName += ByteString::CreateFromInt32(SOLARUPD); // current build number
        m_pRessources = ResMgr::CreateResMgr(aMgrName.GetBuffer());
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

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 20.09.00 16:16:54  fs
 ************************************************************************/

