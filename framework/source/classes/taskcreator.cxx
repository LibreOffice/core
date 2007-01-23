/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: taskcreator.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-23 07:31:12 $
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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_TASKCREATOR_HXX_
#include <classes/taskcreator.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_TARGETHELPER_HXX_
#include <loadenv/targethelper.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _COMPHELPER_CONFIGURATIONHELPER_HXX_
#include <comphelper/configurationhelper.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

//_________________________________________________________________________________________________________________
//  includes of my own project
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-****************************************************************************************************//**
    @short      initialize instance with neccessary informations
    @descr      We need a valid uno service manager to create or instanciate new services.
                All other informations to create frames or tasks come in on right interface methods.

    @param      xSMGR
                    points to the valid uno service manager

    @modified   16.05.2002 09:25, as96863
*//*-*****************************************************************************************************/
TaskCreator::TaskCreator( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
    : ThreadHelpBase(       )
    , m_xSMGR       ( xSMGR )
{
}

/*-****************************************************************************************************//**
    @short      deinitialize instance
    @descr      We should release all used ressource which are not needed any longer.

    @modified   16.05.2002 09:33, as96863
*//*-*****************************************************************************************************/
TaskCreator::~TaskCreator()
{
    m_xSMGR.clear();
}

/*-****************************************************************************************************//**
    TODO document me
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFrame > TaskCreator::createTask( const ::rtl::OUString& sName    ,
                                                                         sal_Bool         bVisible )
{
    static ::rtl::OUString PACKAGE = ::rtl::OUString::createFromAscii("org.openoffice.Office.TabBrowse");
    static ::rtl::OUString RELPATH = ::rtl::OUString::createFromAscii("TaskCreatorService"             );
    static ::rtl::OUString KEY     = ::rtl::OUString::createFromAscii("ImplementationName"             );

    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    /* } SAFE */

    css::uno::Reference< css::lang::XSingleServiceFactory > xCreator;
    ::rtl::OUString sCreator = IMPLEMENTATIONNAME_FWK_TASKCREATOR;

    try
    {
        if (
            ( TargetHelper::matchSpecialTarget(sName, TargetHelper::E_BLANK  ) ) ||
            ( TargetHelper::matchSpecialTarget(sName, TargetHelper::E_DEFAULT) )
           )
        {
            css::uno::Any aVal = ::comphelper::ConfigurationHelper::readDirectKey(xSMGR, PACKAGE, RELPATH, KEY, ::comphelper::ConfigurationHelper::E_READONLY);
            aVal >>= sCreator;
        }

        xCreator = css::uno::Reference< css::lang::XSingleServiceFactory >(
                    xSMGR->createInstance(sCreator), css::uno::UNO_QUERY_THROW);
    }
    catch(const css::uno::Exception&)
    {}

    // no catch here ... without an task creator service we cant open ANY document window within the office.
    // Thats IMHO not a good idea. Then we should accept the stacktrace showing us the real problem.
    // BTW: The used fallback creator service (IMPLEMENTATIONNAME_FWK_TASKCREATOR) is implemented in the same
    // library then these class here ... Why we should not be able to create it ?
    if ( ! xCreator.is())
        xCreator = css::uno::Reference< css::lang::XSingleServiceFactory >(
                    xSMGR->createInstance(IMPLEMENTATIONNAME_FWK_TASKCREATOR), css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::frame::XFrame > xTask(xCreator->createInstance(), css::uno::UNO_QUERY_THROW);

    if (
        (bVisible  ) &&
        (xTask.is())
       )
    {
        css::uno::Reference< css::awt::XWindow > xWindow = xTask->getContainerWindow();
        if (xWindow.is())
            xWindow->setVisible(sal_True);
    }

    return xTask;
}

} // namespace framework
