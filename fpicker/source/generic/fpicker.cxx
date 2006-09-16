/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fpicker.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 17:49:17 $
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
#include "precompiled_fpicker.hxx"

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include "cppuhelper/implementationentry.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTICOMPONENTFACTORY_HPP_
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#endif

#ifndef INCLUDED_SVTOOLS_MISCOPT_HXX
#include "svtools/miscopt.hxx"
#endif

#ifndef SVTOOLS_PICKERHISTORYACCESS_HXX
#include "svtools/pickerhistoryaccess.hxx"
#endif

#ifndef _SV_APP_HXX
#include "vcl/svapp.hxx"
#endif

namespace css = com::sun::star;

using css::uno::Reference;
using css::uno::Sequence;
using rtl::OUString;

/*
 * FilePicker implementation.
 */
static OUString FilePicker_getSystemPickerServiceName()
{
    OUString aDesktopEnvironment (Application::GetDesktopEnvironment());
    if (aDesktopEnvironment.equalsIgnoreAsciiCaseAscii ("gnome"))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.GtkFilePicker"));
    else if (aDesktopEnvironment.equalsIgnoreAsciiCaseAscii ("kde"))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.KDEFilePicker"));
    else
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.SystemFilePicker"));
}

static Reference< css::uno::XInterface > FilePicker_createInstance (
    Reference< css::uno::XComponentContext > const & rxContext)
{
    Reference< css::uno::XInterface > xResult;
    if (rxContext.is())
    {
        Reference< css::lang::XMultiComponentFactory > xFactory (rxContext->getServiceManager());
        if (xFactory.is())
        {
            if (SvtMiscOptions().UseSystemFileDialog())
            {
                try
                {
                    xResult = xFactory->createInstanceWithContext (
                        FilePicker_getSystemPickerServiceName(),
                        rxContext);
                }
                catch (css::uno::Exception const &)
                {
                    // Handled below (see @ fallback).
                }
            }
            if (!xResult.is())
            {
                // Always fall back to OfficeFilePicker.
                xResult = xFactory->createInstanceWithContext (
                    OUString (RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.OfficeFilePicker")),
                    rxContext);
            }
            if (xResult.is())
            {
                // Add to FilePicker history.
                svt::addFilePicker (xResult);
            }
        }
    }
    return xResult;
}

static OUString FilePicker_getImplementationName()
{
    return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.comp.fpicker.FilePicker"));
}

static Sequence< OUString > FilePicker_getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames(1);
    aServiceNames.getArray()[0] =
        OUString (RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FilePicker"));
    return aServiceNames;
}

/*
 * FolderPicker implementation.
 */
static OUString FolderPicker_getSystemPickerServiceName()
{
    OUString aDesktopEnvironment (Application::GetDesktopEnvironment());
    if (aDesktopEnvironment.equalsIgnoreAsciiCaseAscii ("gnome"))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.GtkFolderPicker"));
    else if (aDesktopEnvironment.equalsIgnoreAsciiCaseAscii ("kde"))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.KDEFolderPicker"));
    else
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.SystemFolderPicker"));
}

static Reference< css::uno::XInterface > FolderPicker_createInstance (
    Reference< css::uno::XComponentContext > const & rxContext)
{
    Reference< css::uno::XInterface > xResult;
    if (rxContext.is())
    {
        Reference< css::lang::XMultiComponentFactory > xFactory (rxContext->getServiceManager());
        if (xFactory.is())
        {
            if (SvtMiscOptions().UseSystemFileDialog())
            {
                try
                {
                    xResult = xFactory->createInstanceWithContext (
                        FolderPicker_getSystemPickerServiceName(),
                        rxContext);
                }
                catch (css::uno::Exception const &)
                {
                    // Handled below (see @ fallback).
                }
            }
            if (!xResult.is())
            {
                // Always fall back to OfficeFolderPicker.
                xResult = xFactory->createInstanceWithContext (
                    OUString (RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.OfficeFolderPicker")),
                    rxContext);
            }
            if (xResult.is())
            {
                // Add to FolderPicker history.
                svt::addFolderPicker (xResult);
            }
        }
    }
    return xResult;
}

static OUString FolderPicker_getImplementationName()
{
    return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.comp.fpicker.FolderPicker"));
}

static Sequence< OUString > FolderPicker_getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames(1);
    aServiceNames.getArray()[0] =
        OUString (RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FolderPicker"));
    return aServiceNames;
}

/*
 * Implementation entries.
 */
static cppu::ImplementationEntry g_entries[] =
{
    {
        FilePicker_createInstance,
        FilePicker_getImplementationName,
        FilePicker_getSupportedServiceNames,
        cppu::createSingleComponentFactory, 0, 0
    },
    {
        FolderPicker_createInstance,
        FolderPicker_getImplementationName,
        FolderPicker_getSupportedServiceNames,
        cppu::createSingleComponentFactory, 0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

/*
 * Public (exported) interface.
 */
extern "C"
{
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo (
    void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_writeInfoHelper (
        pServiceManager, pRegistryKey, g_entries);
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory (
    const sal_Char * pImplementationName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper (
        pImplementationName, pServiceManager, pRegistryKey, g_entries);
}

} // extern "C"
