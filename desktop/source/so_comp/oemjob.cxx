/*************************************************************************
 *
 *  $RCSfile: oemjob.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 15:00:19 $
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

#include "oemjob.hxx"
#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;

namespace desktop{

char const OEM_PRELOAD_SECTION[] = "Bootstrap";
char const OEM_PRELOAD[]         = "Preload";
char const STR_TRUE[]            = "1";
char const STR_FALSE[]           = "0";

const char* OEMPreloadJob::interfaces[] =
{
    "com.sun.star.task.XJob",
    NULL,
};
const char* OEMPreloadJob::implementationName = "com.sun.star.comp.desktop.OEMPreloadJob";
const char* OEMPreloadJob::serviceName = "com.sun.star.office.OEMPreloadJob";

OUString OEMPreloadJob::GetImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( implementationName));
}

Sequence< OUString > OEMPreloadJob::GetSupportedServiceNames()
{
    sal_Int32 nSize = (sizeof( interfaces ) / sizeof( const char *)) - 1;
    Sequence< OUString > aResult( nSize );

    for( sal_Int32 i = 0; i < nSize; i++ )
        aResult[i] = OUString::createFromAscii( interfaces[i] );
    return aResult;
}

Reference< XInterface >  SAL_CALL OEMPreloadJob::CreateInstance(
    const Reference< XMultiServiceFactory >& rSMgr )
{
    static osl::Mutex   aMutex;
        osl::MutexGuard guard( aMutex );
        return (XComponent*) ( new OEMPreloadJob( rSMgr ) );
}

OEMPreloadJob::OEMPreloadJob( const Reference< XMultiServiceFactory >& xFactory ) :
    m_aListeners( m_aMutex ),
    m_xServiceManager( xFactory )
{
}

OEMPreloadJob::~OEMPreloadJob()
{
}

// XComponent
void SAL_CALL OEMPreloadJob::dispose() throw ( RuntimeException )
{
    EventObject aObject;
    aObject.Source = (XComponent*)this;
    m_aListeners.disposeAndClear( aObject );
}

void SAL_CALL OEMPreloadJob::addEventListener( const Reference< XEventListener > & aListener) throw ( RuntimeException )
{
    m_aListeners.addInterface( aListener );
}

void SAL_CALL OEMPreloadJob::removeEventListener( const Reference< XEventListener > & aListener ) throw ( RuntimeException )
{
    m_aListeners.removeInterface( aListener );
}

// XServiceInfo
::rtl::OUString SAL_CALL OEMPreloadJob::getImplementationName()
throw ( RuntimeException )
{
    return OEMPreloadJob::GetImplementationName();
}

sal_Bool SAL_CALL OEMPreloadJob::supportsService( const ::rtl::OUString& rServiceName )
throw ( RuntimeException )
{
    sal_Int32 nSize = sizeof( interfaces ) / sizeof( const char *);

    for( sal_Int32 i = 0; i < nSize; i++ )
        if ( rServiceName.equalsAscii( interfaces[i] ))
            return sal_True;
    return sal_False;
}

Sequence< ::rtl::OUString > SAL_CALL OEMPreloadJob::getSupportedServiceNames()
throw ( RuntimeException )
{
    return OEMPreloadJob::GetSupportedServiceNames();
}

// XJob
Any SAL_CALL OEMPreloadJob::execute(const Sequence<NamedValue>& args)
throw ( RuntimeException )
{
    sal_Bool bCont = sal_False;
    // are we an OEM version at all?
    if (checkOEMPreloadFlag())
    {
        // create OEM preload service dialog
        Reference <XExecutableDialog> xDialog( m_xServiceManager->createInstance(
            OUString::createFromAscii("org.openoffice.comp.preload.OEMPreloadWizard")),
            UNO_QUERY );
        if ( xDialog.is() ){
            // execute OEM preload dialog and check return value
            if ( xDialog->execute() == ExecutableDialogResults::OK ) {
                // user accepted.
                // make sure the job does not get called again.
                bCont = sal_True;
                disableOEMPreloadFlag();
            } else {
                // user declined...
                // terminate.
                /*
                Reference< XDesktop > xDesktop( m_xServiceManager->createInstance(
                    OUString::createFromAscii("com.sun.star.frame.Desktop")),
                    UNO_QUERY );
                xDesktop->terminate();
                */
                /*
                OUString aName;
                OUString aEnvType;
                Reference<XFrame> rFrame;
                Reference<XModel> rModel;
                Reference<XCloseable> rClose;
                for (int i=0; i<args.getLength(); i++)
                {
                    if (args[i].Name.equalsAscii("EnvType"))
                        args[i].Value >>= aEnvType;
                    else if (args[i].Name.equalsAscii("Frame")) {
                        args[i].Value >>= rFrame;
                        rClose = Reference<XCloseable>(rFrame, UNO_QUERY);
                    }
                    else if (args[i].Name.equalsAscii("Model")) {
                        args[i].Value >>= rModel;
                        rClose = Reference<XCloseable>(rModel, UNO_QUERY);
                    }
                }
                if (rClose.is()) rClose->close(sal_True);
                */
                bCont = sal_False;
            }
        }
    } else {
        // don't try again
        bCont = sal_True;
    }
    /*
    NamedValue nv;
    nv.Name  = OUString::createFromAscii("Deactivate");
    nv.Value <<=  bDeactivate;
    Sequence<NamedValue> s(1);
    s[0] = nv;
    */
    Any r;
    r <<= bCont;
    return r;
}


static sal_Bool existsURL( OUString const& _sURL )
{
    using namespace osl;
    DirectoryItem aDirItem;

    if (_sURL.getLength() != 0)
        return ( DirectoryItem::get( _sURL, aDirItem ) == DirectoryItem::E_None );

    return sal_False;
}


// locate soffice.ini/.rc file
static OUString locateIniFile()
{
    OUString aUserDataPath;
    OUString aSofficeIniFileURL;

    // Retrieve the default file URL for the soffice.ini/rc
    Bootstrap().getIniName( aSofficeIniFileURL );

    if ( utl::Bootstrap::locateUserData( aUserDataPath ) == utl::Bootstrap::PATH_EXISTS )
    {
        const char CONFIG_DIR[] = "/config";

        sal_Int32 nIndex = aSofficeIniFileURL.lastIndexOf( '/');
        if ( nIndex > 0 )
        {
            OUString        aUserSofficeIniFileURL;
            OUStringBuffer  aBuffer( aUserDataPath );
            aBuffer.appendAscii( CONFIG_DIR );
            aBuffer.append( aSofficeIniFileURL.copy( nIndex ));
            aUserSofficeIniFileURL = aBuffer.makeStringAndClear();

            if ( existsURL( aUserSofficeIniFileURL ))
                return aUserSofficeIniFileURL;
        }
    }
    // Fallback try to use the soffice.ini/rc from program folder
    return aSofficeIniFileURL;
}

// check whether the OEMPreload flag was set in soffice.ini/.rc
sal_Bool OEMPreloadJob::checkOEMPreloadFlag()
{
    OUString aSofficeIniFileURL;
    aSofficeIniFileURL = locateIniFile();
    Config aConfig(aSofficeIniFileURL);
    aConfig.SetGroup( OEM_PRELOAD_SECTION );
    ByteString sResult = aConfig.ReadKey( OEM_PRELOAD );
    if ( sResult == STR_TRUE )
        return sal_True;
    else
        return sal_False;
}

void OEMPreloadJob::disableOEMPreloadFlag()
{
    OUString aSofficeIniFileURL = locateIniFile();
    if ( aSofficeIniFileURL.getLength() > 0 )
    {
        Config aConfig(aSofficeIniFileURL);
        aConfig.SetGroup( OEM_PRELOAD_SECTION );
        aConfig.WriteKey( OEM_PRELOAD, STR_FALSE );
        aConfig.Flush();
    }
}

} // namespace desktop
