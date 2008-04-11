/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: login.cxx,v $
 * $Revision: 1.9 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_SERVICES_LOGINDIALOG_HXX_
#include <services/logindialog.hxx>
#endif
#include <classes/servicemanager.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <services.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <vos/process.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>

#include <stdio.h>

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#define TEMPFILE_ENCODING           RTL_TEXTENCODING_UTF8           // encoding of written temp. ascii file
#define LOGIN_RDB                   DECLARE_ASCII("login.rdb")      // name of our own registry file - neccessary to create own servicemanager
#define SEPERATOR                   "\n"                            // used to seperate parts in temp. file

#define MINARGUMENTCOUNT            1                               // count of min. required arguments
#define ARGUMENTFOUND               0                               // OUString::compareTo returns 0 if searched string match given one
#define ARGUMENTLENGTH              3                               // length of fixed part of any argument to detect it easier!

#define ARGUMENT_TEMPFILE           DECLARE_ASCII("-f=")            // we support "-f=c:\temp\test.txt" to write dialog data in temp. file
#define ARGUMENT_DIALOGPARENT       DECLARE_ASCII("-p=")            // we support "-p=36748322" as window handle of parent for vcl dialog

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl                       ;
using namespace ::vos                       ;
using namespace ::comphelper                ;
using namespace ::framework                 ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::lang      ;
using namespace ::com::sun::star::awt       ;
using namespace ::com::sun::star::beans     ;

//_________________________________________________________________________________________________________________
//  defines
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short      implement command application to show login dialog and save his information in temp. file!
    @descr      We need this temp. file to share informations between our dialog and different processes, which
                can't use vcl directly. Caller of this executable give us the file name as an argument - we save
                all informations in it - caller can read it and MUST delete temp. file.
                This is neccessary for example; to hide the password!

    @implements -

    @base       Application
*//*-*************************************************************************************************************/
class LoginApplication : public Application
{
    //*************************************************************************************************************
    //  public methods
    //*************************************************************************************************************
    public:
        void Main();

    //*************************************************************************************************************
    //  private methods
    //*************************************************************************************************************
    private:
        void impl_parseCommandline();       // search supported arguments on command line

    //*************************************************************************************************************
    //  private variables
    //*************************************************************************************************************
    private:
        OString     m_sTempFile     ;   // name of temp. file in system notation
        sal_Int32   m_nParentHandle ;   // a parent window handle for used vcl dialog

};  //  class LoginApplication

//_________________________________________________________________________________________________________________
//  global variables
//_________________________________________________________________________________________________________________

LoginApplication    gLoginApplication;

//_________________________________________________________________________________________________________________
//  main
//_________________________________________________________________________________________________________________

void LoginApplication::Main()
{
    // Init global uno servicemanager.
    ServiceManager aManager;
    Reference< XMultiServiceFactory > xServiceManager = aManager.getSharedUNOServiceManager( DECLARE_ASCII("login.rdb") );
    LOG_ASSERT( !(xServiceManager.is()==sal_False), "LoginApplication::Main()\nCould not create uno service manager!\n" )

    // Parse command line and set found arguments on application member.
    impl_parseCommandline();
    LOG_ASSERT( !(m_sTempFile.getLength()<1), "LoginApplication::Main()\nWrong or missing argument for temp. file detected!\n" )

    // Try to get neccessary dialog service.
    // By the way - cast it to interface XPropertySet too - we need it later.
    // (define SERVICENAME... comes from defines.hxx!)
    Reference< XDialog >        xLoginDialog( xServiceManager->createInstance( SERVICENAME_LOGINDIALOG ), UNO_QUERY );
    Reference< XPropertySet >   xPropertySet( xLoginDialog                                              , UNO_QUERY );

    // Work with valid ressources only!
    // Otherwise do nothing ...
    if  (
            ( xLoginDialog.is()         ==  sal_True    )   &&
            ( xPropertySet.is()         ==  sal_True    )   &&
            ( m_sTempFile.getLength()   >   0           )
        )
    {
        // Exist a parent window? YES => set right property.
        if( m_nParentHandle != 0 )
        {
            Any aParentWindow;
            aParentWindow <<= m_nParentHandle;
            xPropertySet->setPropertyValue( PROPERTYNAME_PARENTWINDOW, aParentWindow );
        }

        Any aConnectionType;
        aConnectionType <<= PROPERTYNAME_HTTPS;
        xPropertySet->setPropertyValue( PROPERTYNAME_CONNECTIONTYPE, aConnectionType );

        // Show login dialog and get decision of user.
        sal_Bool bDecision = (sal_Bool)(xLoginDialog->execute());

        OUString    sUserName       ;
        OUString    sPassword       ;
        OUString    sServer         ;
        OUString    sConnectionType ;
        sal_Int32   nPort=0         ;   // We need this default if follow "if"-statement "failed"!

        // If user say "OK" ... get values from dialog.
        // If user say "NO" ... leave it. Then we save empty informations later ...
        if( bDecision == sal_True )
        {
            // defines PROPERTYNAME... comes from logindialog.hxx!
            xPropertySet->getPropertyValue( PROPERTYNAME_USERNAME       ) >>= sUserName         ;
            xPropertySet->getPropertyValue( PROPERTYNAME_PASSWORD       ) >>= sPassword         ;
            xPropertySet->getPropertyValue( PROPERTYNAME_SERVER         ) >>= sServer           ;
            xPropertySet->getPropertyValue( PROPERTYNAME_CONNECTIONTYPE ) >>= sConnectionType   ;
            if( sConnectionType.getLength() > 0 )
            {
                xPropertySet->getPropertyValue( sConnectionType ) >>= nPort;
            }
        }

        // Build string for output.
        // At this point it doesnt matter if information exist or not!
        // Format of output: "<decision>    [0|1]       SEPERATOR
        //                    <username>    [string]    SEPERATOR
        //                    <password>    [string]    SEPERATOR
        //                    <servername>  [string]    SEPERATOR
        //                    <port>        [int]       SEPERATOR"
        OUStringBuffer sBuffer( 1000 );

        if( bDecision == sal_True )
        {
            sBuffer.appendAscii( "1" );
        }
        else
        {
            sBuffer.appendAscii( "0" );
        }
        sBuffer.appendAscii ( SEPERATOR         );
        sBuffer.append      ( sUserName         );
        sBuffer.appendAscii ( SEPERATOR         );
        sBuffer.append      ( sPassword         );
        sBuffer.appendAscii ( SEPERATOR         );
        sBuffer.append      ( sServer           );
        sBuffer.appendAscii ( SEPERATOR         );
        sBuffer.append      ( sConnectionType   );
        sBuffer.appendAscii ( SEPERATOR         );
        sBuffer.append      ( nPort             );
        sBuffer.appendAscii ( SEPERATOR         );

        // Write informations in temp. file.
        // If given file name isnt valid ... caller will have a problem!!!
        // If fil already exist (That's out of specification!!!) we overwrite it everytime.
        FILE* pFile = fopen( m_sTempFile.getStr(), "w" );
        LOG_ASSERT( !(pFile==NULL), "LoginApplication::Main()\nCould not open file!\n" );
        if( pFile != NULL )
        {
            OString sEncodedOut = U2B_ENC( sBuffer.makeStringAndClear(), TEMPFILE_ENCODING );
            fprintf( pFile, sEncodedOut.getStr()    );
            fclose ( pFile                          );
        }
    }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
void LoginApplication::impl_parseCommandline()
{
    // Use vos::OStartupInfo for access to command line.
    // Step over all arguments, search for supported ones and try to get his values.
    // Set it on our member. Caller of this method must control setted values.
    OStartupInfo aInfo;

    sal_uInt32  nCount      =   aInfo.getCommandArgCount()  ;
    sal_uInt32  nArgument   =   0                           ;
    OUString    sArgument                                   ;
    OUString    sValue                                      ;

    // Warn programmer if argument count isnt ok!
    LOG_ASSERT( !(nCount!=MINARGUMENTCOUNT), "LoginApplication::impl_parseCommandline()\nWrong argument count detected!\n" )

    // Reset all possible argument variables to defaults if someone is missing.
    m_sTempFile     = OString();
    m_nParentHandle = 0        ;

    // Step over all arguments ...
    for( nArgument=0; nArgument<nCount; ++nArgument )
    {
        // .. but work with valid ones only!
        // Don't check values here. Caller of this method must decide between wrong and allowed values!
        aInfo.getCommandArg( nArgument, sArgument );

        //_____________________________________________________________________________________________________
        // Look for "-f=<temp. file name>"
        if( sArgument.compareTo( ARGUMENT_TEMPFILE, ARGUMENTLENGTH ) == ARGUMENTFOUND )
        {
            sValue      = sArgument.copy( ARGUMENTLENGTH );
            m_sTempFile = U2B(sValue);
        }
        else
        //_____________________________________________________________________________________________________
        // Look for "-p=<parent window handle>"
        if( sArgument.compareTo( ARGUMENT_DIALOGPARENT, ARGUMENTLENGTH ) == ARGUMENTFOUND )
        {
            sValue          = sArgument.copy( ARGUMENTLENGTH );
            m_nParentHandle = sValue.toInt32();
        }
    }

    // Parent window handle is an optional argument ... but should be used mostly!
    // Warn programmer.
    LOG_ASSERT( !(m_nParentHandle==0), "Login.exe\nYou should give me a parent window handle!\n" )
}
