/** -- C++ Source File -- **/
#include <stdio.h>
#include "helper.hxx"

#include "libxml/tree.h"
#include "libxml/parser.h"
#ifndef XMLSEC_NO_XSLT
#include "libxslt/xslt.h"
#endif

#include "securityenvironment_mscryptimpl.hxx"

#include "xmlsec/strings.h"
#include "xmlsec/xmltree.h"
#include "xmlsec/mscrypto/app.h"

#include <rtl/ustring.hxx>

using namespace ::rtl ;
using namespace ::cppu ;
using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::io ;
using namespace ::com::sun::star::ucb ;
using namespace ::com::sun::star::beans ;
using namespace ::com::sun::star::document ;
using namespace ::com::sun::star::lang ;
using namespace ::com::sun::star::security ;
using namespace ::com::sun::star::xml::wrapper ;
using namespace ::com::sun::star::xml::crypto ;

extern OUString bigIntegerToNumericString( Sequence< sal_Int8 > serial ) ;

int SAL_CALL main( int argc, char **argv )
{
    const char* n_pCertStore ;
    HCERTSTORE n_hStoreHandle ;

    if( argc != 3 && argc != 2 ) {
        fprintf( stderr, "Usage: %s <rdb file>\n" , argv[0] ) ;
        fprintf( stderr, "Or: \t%s <rdb file> < Cert Store Name >\n\n" , argv[0] ) ;
        return 1 ;
    }

    //Initialize the crypto engine
    if( argc == 3 ) {
        n_pCertStore = argv[2] ;
        n_hStoreHandle = CertOpenSystemStore( NULL, n_pCertStore ) ;
        if( n_hStoreHandle == NULL ) {
            fprintf( stderr, "Can not open the system cert store %s\n", n_pCertStore ) ;
            return 1 ;
        }
    } else {
        n_pCertStore = NULL ;
        n_hStoreHandle = NULL ;
    }
    //xmlSecMSCryptoAppInit( n_pCertStore ) ;

    try {
        Reference< XMultiComponentFactory > xManager = NULL ;
        Reference< XComponentContext > xContext = NULL ;

        xManager = serviceManager( xContext , OUString::createFromAscii( "local" ), OUString::createFromAscii( argv[1] ) ) ;
        OSL_ENSURE( xManager.is() ,
            "ServicesManager - "
            "Cannot get service manager" ) ;

        //Create security environment
        //Build Security Environment
        Reference< XInterface > xsecenv =
            xManager->createInstanceWithContext( OUString::createFromAscii("com.sun.star.xml.security.bridge.xmlsec.SecurityEnvironment_MSCryptImpl"), xContext ) ;
        OSL_ENSURE( xsecenv.is() ,
            "Signer - "
            "Cannot get service instance of \"xsec.SecurityEnvironment\"" ) ;

        Reference< XSecurityEnvironment > xSecEnv( xsecenv , UNO_QUERY ) ;
        OSL_ENSURE( xSecEnv.is() ,
            "Signer - "
            "Cannot get interface of \"XSecurityEnvironment\" from service \"xsec.SecurityEnvironment\"" ) ;

        Reference< XUnoTunnel > xEnvTunnel( xsecenv , UNO_QUERY ) ;
        OSL_ENSURE( xEnvTunnel.is() ,
            "Signer - "
            "Cannot get interface of \"XUnoTunnel\" from service \"xsec.SecurityEnvironment\"" ) ;

        SecurityEnvironment_MSCryptImpl* pSecEnv = ( SecurityEnvironment_MSCryptImpl* )xEnvTunnel->getSomething( SecurityEnvironment_MSCryptImpl::getUnoTunnelId() ) ;
        OSL_ENSURE( pSecEnv != NULL ,
            "Signer - "
            "Cannot get implementation of \"xsec.SecurityEnvironment\"" ) ;

        //Setup key slot and certDb
        if( n_hStoreHandle != NULL ) {
            pSecEnv->setCryptoSlot( n_hStoreHandle ) ;
            pSecEnv->setCertDb( n_hStoreHandle ) ;
        } else {
            pSecEnv->enableDefaultCrypt( sal_True ) ;
        }

        //Get personal certificate
        Sequence < Reference< XCertificate > > xPersonalCerts = pSecEnv->getPersonalCertificates() ;
        OSL_ENSURE( xPersonalCerts.hasElements() ,
            "getPersonalCertificates - "
            "No personal certificates found\n" ) ;

        Sequence < Reference< XCertificate > > xCertPath ;
        for( int i = 0; i < xPersonalCerts.getLength(); i ++ ) {
            //Print the certificate infomation.
            fprintf( stdout, "\nPersonal Certificate Info\n" ) ;
            fprintf( stdout, "\tCertificate Issuer[%s]\n", OUStringToOString( xPersonalCerts[i]->getIssuerName(), RTL_TEXTENCODING_ASCII_US ).getStr() ) ;
            fprintf( stdout, "\tCertificate Serial Number[%s]\n", OUStringToOString( bigIntegerToNumericString( xPersonalCerts[i]->getSerialNumber() ), RTL_TEXTENCODING_ASCII_US ).getStr() ) ;
            fprintf( stdout, "\tCertificate Subject[%s]\n", OUStringToOString( xPersonalCerts[i]->getSubjectName(), RTL_TEXTENCODING_ASCII_US ).getStr() ) ;

            //build the certificate path
            xCertPath = pSecEnv->buildCertificatePath( xPersonalCerts[i] ) ;
            //Print the certificate path.
            fprintf( stdout, "\tCertificate Path\n" ) ;
            for( int j = 0; j < xCertPath.getLength(); j ++ ) {
                fprintf( stdout, "\t\tCertificate Authority Subject[%s]\n", OUStringToOString( xCertPath[j]->getSubjectName(), RTL_TEXTENCODING_ASCII_US ).getStr() ) ;
            }

            //Get the certificate
            Sequence < sal_Int8 > serial = xPersonalCerts[i]->getSerialNumber() ;
            Reference< XCertificate > xcert = pSecEnv->getCertificate( xPersonalCerts[i]->getIssuerName(), xPersonalCerts[i]->getSerialNumber() ) ;
            if( !xcert.is() ) {
                fprintf( stdout, "The personal certificate is not in the certificate database\n" ) ;
            }

            //Get the certificate characters
            sal_Int32 chars = pSecEnv->getCertificateCharacters( xPersonalCerts[i] ) ;
            fprintf( stdout, "The certificate characters are %d\n", chars ) ;

            //Get the certificate status
            sal_Int32 validity = pSecEnv->verifyCertificate( xPersonalCerts[i] ) ;
            fprintf( stdout, "The certificate validities are %d\n", validity ) ;

        }
    } catch( Exception& e ) {
        fprintf( stderr , "Error Message: %s\n" , OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).getStr() ) ;
        goto done ;
    }

done:
    if( n_hStoreHandle != NULL )
        CertCloseStore( n_hStoreHandle, CERT_CLOSE_STORE_FORCE_FLAG ) ;

    //xmlSecMSCryptoAppShutdown() ;

    return 0;
}

