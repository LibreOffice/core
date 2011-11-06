/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "oemjob.hxx"
#include <rtl/bootstrap.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/file.hxx>
#include <unotools/bootstrap.hxx>
#include <tools/config.hxx>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

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
Any SAL_CALL OEMPreloadJob::execute(const Sequence<NamedValue>&)
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
