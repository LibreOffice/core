/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: core_resource.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-25 16:26:22 $
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

#include "core_resource.hxx"

#ifndef _TOOLS_SIMPLERESMGR_HXX_
#include <tools/simplerm.hxx>
#endif

// ---- needed as long as we have no contexts for components ---
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
//---------------------------------------------------
#ifndef _COMPHELPER_CONFIGURATIONHELPER_HXX_
#include <comphelper/configurationhelper.hxx>
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

#ifndef _SOLAR_HRC
#include <svtools/solar.hrc>
#endif
#ifndef _RPTUI_MODULE_HELPER_RPT_HXX_
#include "ModuleHelper.hxx"
#endif
//.........................................................................
namespace reportdesign
{
    using namespace ::com::sun::star;
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
    void ResourceManager::ensureImplExists(const uno::Reference< lang::XMultiComponentFactory >& /* _rM */)
    {
        if (!m_pImpl)
        {
            // now that we have an impl class make sure it's deleted on unloading the library
            static ResourceManager::EnsureDelete    s_aDeleteTheImplClass;

            ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();

            rtl::OString sResLibName = rtl::OString( "rpt" );
            m_pImpl = SimpleResMgr::Create(sResLibName, aLocale);
        }
    }

    //------------------------------------------------------------------
    ::rtl::OUString ResourceManager::loadString(sal_uInt16 _nResId,const uno::Reference< lang::XMultiComponentFactory >& _rM)
    {
        ::rtl::OUString sReturn;

        ensureImplExists(_rM);
        if (m_pImpl)
            sReturn = m_pImpl->ReadString(_nResId);

        return sReturn;
    }

//.........................................................................
}
//.........................................................................

