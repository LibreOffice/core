/*************************************************************************
 *
 *  $RCSfile: seinitializer_nssimpl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-21 14:31:25 $
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

/*
 * Turn off DEBUG Assertions
 */
#ifdef _DEBUG
    #define _DEBUG_WAS_DEFINED _DEBUG
    #undef _DEBUG
#else
    #undef _DEBUG_WAS_DEFINED
#endif

/*
 * and turn off the additional virtual methods which are part of some interfaces when compiled
 * with debug
 */
#ifdef DEBUG
    #define DEBUG_WAS_DEFINED DEBUG
    #undef DEBUG
#else
    #undef DEBUG_WAS_DEFINED
#endif

/*
 * header files needed for getCurrentProfilePath
 */
#include "nsIServiceManager.h"
#include "nsIProfileInternal.h"
#include "nsString.h"
#include "nsEmbedAPI.h"

#include <sal/types.h>


#include "seinitializer_nssimpl.hxx"

#include "securityenvironment_nssimpl.hxx"

#include "nspr.h"
#include "prtypes.h"
#include "pk11func.h"
#include "cert.h"
#include "cryptohi.h"
#include "certdb.h"
#include "nss.h"

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;

using namespace com::sun::star;

#define SERVICE_NAME "com.sun.star.xml.crypto.SEInitializer"
#define IMPLEMENTATION_NAME "com.sun.star.xml.security.bridge.xmlsec.SEInitializer_NssImpl"
#define SECURITY_ENVIRONMENT "com.sun.star.xml.crypto.SecurityEnvironment"
#define SECURITY_CONTEXT "com.sun.star.xml.crypto.XMLSecurityContext"

/*
 * MM : get the current user profile
 */

// MM : By now, the XPCOM is initialized only once in the current thread, and it will
//      not be shutdown until StarOffice exits.
//      This is a bug, because any other component who will initialize the XPCOM afterward
//      will always fail.
//      This bug will be fixed when there is solution.
static nsIServiceManager           *sServiceManager = nsnull;
static nsIDirectoryServiceProvider *appFileLocProvider = nsnull;
static NS_DEFINE_CID(kProfileCID, NS_PROFILE_CID);

char* getCurrentProfilePath( )
{
/*
        nsCOMPtr<nsILocalFile> binDir;

        // Note: if getenv() returns NULL, mozilla will default to using MOZILLA_FIVE_HOME in the NS_InitXPCOM2()
        // The NS_NewNativeLocalFile() will accept NULL as its first parameter.
        char * env = getenv("OPENOFFICE_MOZILLA_FIVE_HOME");
        if (env)
        {
        nsDependentCString sPath(env);
        nsresult rv = NS_NewNativeLocalFile(sPath, PR_TRUE, getter_AddRefs(binDir));
        if (NS_FAILED(rv))
            return NULL;
        }

    if (sServiceManager == nsnull)
    {
        NS_InitXPCOM2(&sServiceManager, binDir, appFileLocProvider);
    }

    if (!sServiceManager)
        return NULL;

    nsresult rv;
    nsCOMPtr< nsIProfile > theProfile = do_GetService( kProfileCID, &rv );
    if (NS_SUCCEEDED(rv))
    {
        nsXPIDLString profileName;
        rv = theProfile->GetCurrentProfile(getter_Copies(profileName));
        if (NS_SUCCEEDED(rv))
        {
            nsCOMPtr<nsIFile> curProfileDir;
            PRBool exists = PR_FALSE;
            nsCOMPtr<nsIProfileInternal> profileInternal=do_QueryInterface(theProfile);
            if (NS_SUCCEEDED(rv))
            {
                rv = profileInternal->GetProfileDir(profileName, getter_AddRefs(curProfileDir));
                if (NS_SUCCEEDED(rv))
                {
                    nsCOMPtr<nsILocalFile> localFile(do_QueryInterface(curProfileDir));

                    nsAutoString path;
                    rv = localFile->GetPath(path);
                    if (NS_SUCCEEDED(rv))
                    {
                        char cs[1024];
                        path.ToCString(cs, 1024);

                        // MM : I can't shutdown, because the XPCom can't be initialized twice in
                        //      one program
                        //NS_RELEASE(sServiceManager);
                        //NS_ShutdownXPCOM(sServiceManager);

                        return (strdup(cs));
                    }
                }
            }
        }
    }

    // MM : I can't shutdown, because the XPCom can't be initialized twice in
    //      one program
    //NS_RELEASE(sServiceManager);
    //NS_ShutdownXPCOM(sServiceManager);
    */

    return NULL;
}

/*
 * get the current user profile (end)
 */

SEInitializer_NssImpl::SEInitializer_NssImpl(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > &rxMSF)
    :mxMSF( rxMSF )
{
}

SEInitializer_NssImpl::~SEInitializer_NssImpl()
{
}

/* XSEInitializer */
cssu::Reference< cssxc::XXMLSecurityContext > SAL_CALL
    SEInitializer_NssImpl::createSecurityContext(
    const rtl::OUString& sCertDB )
    throw (cssu::RuntimeException)
{
    CERTCertDBHandle*   pCertHandle = NULL ;
    PK11SlotInfo*       pSlot = NULL ;

    rtl::OString sCertDir;
    if( sCertDB.getLength() > 0 )
    {
        sCertDir = rtl::OString(sCertDB, sCertDB.getLength(), RTL_TEXTENCODING_ASCII_US);
    }
    else
    {
        char *pCurrentProfilePath = getCurrentProfilePath();

        if (pCurrentProfilePath == NULL)
        {
            return NULL;
        }
        else
        {
            sCertDir = rtl::OString(pCurrentProfilePath);
            free(pCurrentProfilePath);
        }
    }

    /* Initialize NSPR and NSS */
    PR_Init( PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1 ) ;

    if (NSS_Init(sCertDir.getStr()) != SECSuccess )
    {
        PK11_LogoutAll();
        return NULL;
    }

    pCertHandle = CERT_GetDefaultCertDB() ;
    pSlot = PK11_GetInternalKeySlot() ;

    if (pSlot == NULL)
    {
        PK11_LogoutAll();
        NSS_Shutdown();
        return NULL;
    }

    PK11SymKey* pSymKey = PK11_KeyGen( pSlot , CKM_DES3_CBC, NULL, 128, NULL ) ;
    if( pSymKey == NULL )
    {
        PK11_FreeSlot( pSlot ) ;
        PK11_LogoutAll();
        NSS_Shutdown();
        return NULL;
    }

    try
    {
        /* Build Security Environment */
        const rtl::OUString sSecyrutyEnvironment ( RTL_CONSTASCII_USTRINGPARAM( SECURITY_ENVIRONMENT ) );
        cssu::Reference< cssxc::XSecurityEnvironment > xSecEnv( mxMSF->createInstance ( sSecyrutyEnvironment ), cssu::UNO_QUERY );
        if( !xSecEnv.is() )
        {
            PK11_FreeSymKey( pSymKey ) ;
            PK11_FreeSlot( pSlot ) ;
            PK11_LogoutAll();
            NSS_Shutdown();
            return NULL;
        }

        /* Setup key slot and certDb */
        cssu::Reference< cssl::XUnoTunnel > xEnvTunnel( xSecEnv , cssu::UNO_QUERY ) ;
        if( !xEnvTunnel.is() )
        {
            PK11_FreeSymKey( pSymKey ) ;
            PK11_FreeSlot( pSlot ) ;
            PK11_LogoutAll();
            NSS_Shutdown();
            return NULL;
        }

        SecurityEnvironment_NssImpl* pSecEnv = ( SecurityEnvironment_NssImpl* )xEnvTunnel->getSomething( SecurityEnvironment_NssImpl::getUnoTunnelId() ) ;
        if( pSecEnv == NULL )
        {
            PK11_FreeSymKey( pSymKey ) ;
            PK11_FreeSlot( pSlot ) ;
            PK11_LogoutAll();
            NSS_Shutdown();
            return NULL;
        }

        pSecEnv->setCryptoSlot( pSlot ) ;
        PK11_FreeSlot( pSlot ) ;
        pSlot = NULL;

        pSecEnv->setCertDb( pCertHandle ) ;

        pSecEnv->adoptSymKey( pSymKey ) ;
        PK11_FreeSymKey( pSymKey ) ;
        pSymKey = NULL;

        /* Build XML Security Context */
        const rtl::OUString sSecyrutyContext ( RTL_CONSTASCII_USTRINGPARAM( SECURITY_CONTEXT ) );
        cssu::Reference< cssxc::XXMLSecurityContext > xSecCtx( mxMSF->createInstance ( sSecyrutyContext ), cssu::UNO_QUERY );
        if( !xSecCtx.is() )
        {
            PK11_LogoutAll();
            NSS_Shutdown();
            return NULL;
        }

        xSecCtx->setSecurityEnvironment( xSecEnv ) ;
        return xSecCtx;
    }
    catch( cssu::Exception& )
    {
        if (pSymKey != NULL)
        {
            PK11_FreeSymKey( pSymKey ) ;
        }

        if (pSlot != NULL)
        {
            PK11_FreeSlot( pSlot ) ;
        }

        PK11_LogoutAll();
        NSS_Shutdown();
        return NULL;
    }
}

void SAL_CALL SEInitializer_NssImpl::freeSecurityContext( const cssu::Reference< cssxc::XXMLSecurityContext >& securityContext )
    throw (cssu::RuntimeException)
{
    /*
     * because the security context will free all its content when it
     * is destructed, so here no free process for the security context
     * is needed.
     */
    PK11_LogoutAll();
    NSS_Shutdown();
}

rtl::OUString SEInitializer_NssImpl_getImplementationName ()
    throw (cssu::RuntimeException)
{
    return rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( IMPLEMENTATION_NAME ) );
}

sal_Bool SAL_CALL SEInitializer_NssImpl_supportsService( const rtl::OUString& ServiceName )
    throw (cssu::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ));
}

cssu::Sequence< rtl::OUString > SAL_CALL SEInitializer_NssImpl_getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    cssu::Sequence < rtl::OUString > aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}
#undef SERVICE_NAME

cssu::Reference< cssu::XInterface > SAL_CALL SEInitializer_NssImpl_createInstance( const cssu::Reference< cssl::XMultiServiceFactory > & rSMgr)
    throw( cssu::Exception )
{
    return (cppu::OWeakObject*) new SEInitializer_NssImpl(rSMgr);
}

/* XServiceInfo */
rtl::OUString SAL_CALL SEInitializer_NssImpl::getImplementationName(  )
    throw (cssu::RuntimeException)
{
    return SEInitializer_NssImpl_getImplementationName();
}
sal_Bool SAL_CALL SEInitializer_NssImpl::supportsService( const rtl::OUString& rServiceName )
    throw (cssu::RuntimeException)
{
    return SEInitializer_NssImpl_supportsService( rServiceName );
}
cssu::Sequence< rtl::OUString > SAL_CALL SEInitializer_NssImpl::getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    return SEInitializer_NssImpl_getSupportedServiceNames();
}

