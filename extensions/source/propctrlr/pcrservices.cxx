/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pcrservices.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:12:07 $
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
#include "precompiled_extensions.hxx"

#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif

//---------------------------------------------------------------------------------------

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

//---------------------------------------------------------------------------------------

extern "C" void SAL_CALL createRegistryInfo_OPropertyBrowserController();
extern "C" void SAL_CALL createRegistryInfo_FormController();
extern "C" void SAL_CALL createRegistryInfo_DefaultFormComponentInspectorModel();
extern "C" void SAL_CALL createRegistryInfo_DefaultHelpProvider();
extern "C" void SAL_CALL createRegistryInfo_OControlFontDialog();
extern "C" void SAL_CALL createRegistryInfo_OTabOrderDialog();
extern "C" void SAL_CALL createRegistryInfo_CellBindingPropertyHandler();
extern "C" void SAL_CALL createRegistryInfo_ButtonNavigationHandler();
extern "C" void SAL_CALL createRegistryInfo_EditPropertyHandler();
extern "C" void SAL_CALL createRegistryInfo_FormComponentPropertyHandler();
extern "C" void SAL_CALL createRegistryInfo_EFormsPropertyHandler();
extern "C" void SAL_CALL createRegistryInfo_XSDValidationPropertyHandler();
extern "C" void SAL_CALL createRegistryInfo_EventHandler();
extern "C" void SAL_CALL createRegistryInfo_GenericPropertyHandler();
extern "C" void SAL_CALL createRegistryInfo_ObjectInspectorModel();
extern "C" void SAL_CALL createRegistryInfo_SubmissionPropertyHandler();
extern "C" void SAL_CALL createRegistryInfo_StringRepresentation();
extern "C" void SAL_CALL createRegistryInfo_MasterDetailLinkDialog();

//---------------------------------------------------------------------------------------

extern "C" void SAL_CALL pcr_initializeModule()
{
    static sal_Bool s_bInit = sal_False;
    if (!s_bInit)
    {
        createRegistryInfo_OPropertyBrowserController();
        createRegistryInfo_FormController();
        createRegistryInfo_DefaultFormComponentInspectorModel();
        createRegistryInfo_DefaultHelpProvider();
        createRegistryInfo_OControlFontDialog();
        createRegistryInfo_OTabOrderDialog();
        createRegistryInfo_CellBindingPropertyHandler();
        createRegistryInfo_ButtonNavigationHandler();
        createRegistryInfo_EditPropertyHandler();
        createRegistryInfo_FormComponentPropertyHandler();
        createRegistryInfo_EFormsPropertyHandler();
        createRegistryInfo_XSDValidationPropertyHandler();
        createRegistryInfo_EventHandler();
        createRegistryInfo_GenericPropertyHandler();
        createRegistryInfo_ObjectInspectorModel();
        createRegistryInfo_SubmissionPropertyHandler();
        createRegistryInfo_StringRepresentation();
        createRegistryInfo_MasterDetailLinkDialog();
        s_bInit = sal_True;
    }
}

//---------------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
                const sal_Char  **ppEnvTypeName,
                uno_Environment ** /*ppEnv*/
            )
{
    pcr_initializeModule();
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//---------------------------------------------------------------------------------------
extern "C" sal_Bool SAL_CALL component_writeInfo(
                void* pServiceManager,
                void* pRegistryKey
            )
{
    if (pRegistryKey)
    try
    {
        return ::pcr::PcrModule::getInstance().writeComponentInfos(
            static_cast<XMultiServiceFactory*>(pServiceManager),
            static_cast<XRegistryKey*>(pRegistryKey));
    }
    catch (InvalidRegistryException& )
    {
        OSL_ASSERT("pcr::component_writeInfo: could not create a registry key (InvalidRegistryException) !");
    }

    return sal_False;
}

//---------------------------------------------------------------------------------------
extern "C" void* SAL_CALL component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void* /*pRegistryKey*/)
{
    Reference< XInterface > xRet;
    if (pServiceManager && pImplementationName)
    {
        xRet = ::pcr::PcrModule::getInstance().getComponentFactory(
            ::rtl::OUString::createFromAscii(pImplementationName),
            static_cast< XMultiServiceFactory* >(pServiceManager));
    }

    if (xRet.is())
        xRet->acquire();
    return xRet.get();
};

