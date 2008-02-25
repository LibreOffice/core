/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: frm_resource.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-25 15:55:24 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#include "frm_resource.hxx"

#ifndef _TOOLS_SIMPLERESMGR_HXX_
#include <tools/simplerm.hxx>
#endif

// ---- needed as long as we have no contexts for components ---
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
//---------------------------------------------------

#ifndef _SOLAR_HRC
#include <svtools/solar.hrc>
#endif

//.........................................................................
namespace frm
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

        ByteString sFileName("frm");

        m_pImpl = SimpleResMgr::Create(sFileName.GetBuffer(), Application::GetSettings().GetUILocale());

        if (m_pImpl)
        {
            // no that we have a impl class make sure it's deleted on unloading the library
            static ResourceManager::EnsureDelete    s_aDeleteTheImplClas;
        }
    }

    //------------------------------------------------------------------
    ::rtl::OUString ResourceManager::loadString(sal_uInt16 _nResId)
    {
        ::rtl::OUString sReturn;

        ensureImplExists();
        if (m_pImpl)
            sReturn = m_pImpl->ReadString(_nResId);

        return sReturn;
    }

//.........................................................................
}
//.........................................................................

