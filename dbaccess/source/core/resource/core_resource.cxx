/*************************************************************************
 *
 *  $RCSfile: core_resource.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:15:41 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "core_resource.hxx"

#ifndef _TOOLS_SIMPLERESMGR_HXX_
#include <tools/simplerm.hxx>
#endif

// ---- needed as long as we have no contexts for components ---
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif
//---------------------------------------------------

#ifndef _SOLAR_HRC
#include <svtools/solar.hrc>
#endif

//.........................................................................
namespace dba
{

    //==================================================================
    //= ResourceManager
    //==================================================================
    SimpleResMgr* ResourceManager::m_pImpl = NULL;

    //------------------------------------------------------------------
    ResourceManager::EnsureDelete::~EnsureDelete()
    {
        delete ResourceManager::m_pImpl;
    }

    //------------------------------------------------------------------
    void ResourceManager::ensureImplExists()
    {
        if (m_pImpl)
            return;

        LanguageType nType = Application::GetAppInternational().GetLanguage();

        ByteString sFileName("dbac");
        sFileName += ByteString::CreateFromInt32( (sal_Int32)SOLARUPD );

        m_pImpl = SimpleResMgr::Create(sFileName.GetBuffer(), nType);

        if (m_pImpl)
        {
            // now that we have an impl class make sure it's deleted on unloading the library
            static ResourceManager::EnsureDelete    s_aDeleteTheImplClass;
        }
    }

    //------------------------------------------------------------------
    String ResourceManager::loadString(sal_uInt16 _nResId)
    {
        String sReturn;

        ensureImplExists();
        if (m_pImpl)
            sReturn = m_pImpl->ReadString(_nResId);

        return sReturn;
    }

//.........................................................................
}
//.........................................................................

