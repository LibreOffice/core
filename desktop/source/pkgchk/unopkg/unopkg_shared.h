/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unopkg_shared.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-25 16:50:05 $
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

#include "dp_misc.h"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/deployment/XPackage.hpp"
#include "tools/resmgr.hxx"



namespace css = ::com::sun::star;

namespace unopkg {

struct DeploymentResMgr :  public rtl::StaticWithInit< ResMgr *, DeploymentResMgr >
{
    ResMgr * operator () () {
        return ResMgr::CreateResMgr( "deployment" );
    }
};
// There is another gobal DeploymentGuiResMgr in the gui part of the Extension Manager.
//We need it here because we share a WarningBox resource (WARNINGBOX_CONCURRENTINSTANCE)
//the gui
struct DeploymentGuiResMgr :
    public ::rtl::StaticWithInit< ResMgr *, DeploymentGuiResMgr > {
        ResMgr * operator () () {
            return ResMgr::CreateResMgr( "deploymentgui" );
    }
};

struct OptionInfo
{
    char const * m_name;
    sal_uInt32 m_name_length;
    sal_Unicode m_short_option;
    bool m_has_argument;
};

//==============================================================================
::rtl::OString toString( OptionInfo const * info );

//==============================================================================
OptionInfo const * getOptionInfo(
    OptionInfo const * list,
    ::rtl::OUString const & opt, sal_Unicode copt = '\0' );

//==============================================================================
bool isOption( OptionInfo const * option_info, sal_uInt32 * pIndex );

//==============================================================================
bool readArgument(
    ::rtl::OUString * pValue, OptionInfo const * option_info,
    sal_uInt32 * pIndex );

//==============================================================================
inline bool readOption(
    bool * flag, OptionInfo const * option_info, sal_uInt32 * pIndex )
{
    if (isOption( option_info, pIndex )) {
        OSL_ASSERT( flag != 0 );
        *flag = true;
        return true;
    }
    return false;
}
//==============================================================================

/** checks if an argument is a bootstrap variable. These start with -env:. For example
    -env:UNO_JAVA_JFW_USER_DATA=file:///d:/user
*/
bool isBootstrapVariable(sal_uInt32 * pIndex);
//==============================================================================
::rtl::OUString const & getExecutableDir();

//==============================================================================
::rtl::OUString const & getProcessWorkingDir();

//==============================================================================
::rtl::OUString makeAbsoluteFileUrl(
    ::rtl::OUString const & sys_path, ::rtl::OUString const & base_url,
    bool throw_exc = true );

//##############################################################################

//==============================================================================
class DisposeGuard
{
    css::uno::Reference<css::lang::XComponent> m_xComp;

public:
    inline DisposeGuard() {}
    inline DisposeGuard(
        css::uno::Reference<css::lang::XComponent> const & xComp )
        : m_xComp( xComp ) {}

    inline ~DisposeGuard()
    {
        if (m_xComp.is())
            m_xComp->dispose();
    }

    inline void reset(
        css::uno::Reference<css::lang::XComponent> const & xComp )
    {
        m_xComp = xComp;
    }
};

//==============================================================================
css::uno::Reference<css::ucb::XCommandEnvironment> createCmdEnv(
    css::uno::Reference<css::uno::XComponentContext> const & xContext,
    ::rtl::OUString const & logFile,
    bool option_force_overwrite,
    bool option_verbose);

//==============================================================================
void printf_packages(
    css::uno::Sequence<
    css::uno::Reference<css::deployment::XPackage> > const & seq,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
    sal_Int32 level = 0 );

//##############################################################################

//==============================================================================
css::uno::Reference<css::uno::XComponentContext> getUNO(
    DisposeGuard & disposeGuard, bool verbose, bool shared, bool bGui,
    css::uno::Reference<css::uno::XComponentContext> & out_LocalComponentContext);

}

