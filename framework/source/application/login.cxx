/*************************************************************************
 *
 *  $RCSfile: login.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-03-29 13:17:12 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_SERVICES_LOGINDIALOG_HXX_
#include <services/logindialog.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_SERVICEMANAGER_HXX_
#include <classes/servicemanager.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XDIALOG_HPP_
#include <com/sun/star/awt/XDialog.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _SV_EVENT_HXX
#include <vcl/event.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#include <stdio.h>

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#define TEMPFILE_ENCODING           RTL_TEXTENCODING_UTF8           // encoding of written temp. ascii file
#define ARGUMENTFOUND               0                               // OUString::compareTo returns 0 if searched string match given one
#define SEPERATOR                   (sal_Unicode)';'                // seperator for temp. file values
#define ENCODESIGN                  (sal_Unicode)'\\'               // special character to encode SEPERATOR. These sign must be encoded too!!!

#define ARGUMENT_TEMPFILE           DECLARE_ASCII("-f=")            // we support "-f=c:\temp\test.txt" as argument
#define ARGUMENTLENGTH_TEMPFILE     3                               // length of ARGUMENT_TEMPFILE to find it in argumentlist

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
        void        impl_parseCommandline   (                           );      // search supported arguments on command line
        OUString    impl_encodeString       ( const OUString& sValue    );      // encode SEPERATOR and "\" with an additional "\"! zB "\" => "\\"

    //*************************************************************************************************************
    //  private variables
    //*************************************************************************************************************
    private:
        OString m_sTempFile ;   // name of temp. file in system notation

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
        // User can't set used connection type in dialog directly!
        // And if our setup has written wrong value for it ...
        // we must set right type before to get right value after showing dialog!!!
        Any aConnectionType;
        aConnectionType <<= PROPERTYNAME_COMPRESSEDSECURE;
        xPropertySet->setPropertyValue( PROPERTYNAME_CONNECTIONTYPE, aConnectionType );

        // Show login dialog and get decision of user.
        sal_Bool bDecision = (sal_Bool)(xLoginDialog->execute());

        OUString    sUserName       ;
        OUString    sPassword       ;
        OUString    sServer         ;
        OUString    sConnectionType ;
        sal_Int32   nPort=0         ;   // We need this default if follow "if"-statement "failed"!
                                        // Strings before has "" as default.

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
        // Format of output: "<decision[0|1]>;<username[string]>;<password[string]>;<servername[string]>;<port[int]>"
        OUStringBuffer sBuffer( 1000 );

        if( bDecision == sal_True )
        {
            sBuffer.appendAscii( "1" );
        }
        else
        {
            sBuffer.appendAscii( "0" );
        }
        sBuffer.append      ( SEPERATOR                         );
        sBuffer.append      ( impl_encodeString(sUserName)      );
        sBuffer.append      ( SEPERATOR                         );
        sBuffer.append      ( impl_encodeString(sPassword)      );
        sBuffer.append      ( SEPERATOR                         );
        sBuffer.append      ( impl_encodeString(sServer)        );
        sBuffer.append      ( SEPERATOR                         );
        sBuffer.append      ( impl_encodeString(sConnectionType));
        sBuffer.append      ( SEPERATOR                         );
        sBuffer.append      ( nPort                             );
        sBuffer.append      ( SEPERATOR                         );
        sBuffer.appendAscii ( "\n"                              );

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

    // Warn programmer if argument count isnt ok!
    LOG_ASSERT( !(nCount!=1), "LoginApplication::impl_parseCommandline()\nWrong argument count detected!\n" )

    // Step over all arguments ...
    for( nArgument=0; nArgument<nCount; ++nArgument )
    {
        // .. but work with valid ones only!
        // Don't check values here. Caller of this method must decide between wrong and allowed values!
        aInfo.getCommandArg( nArgument, sArgument );

        //_____________________________________________________________________________________________________
        // Look for "-f<temp. file name>
        if( sArgument.compareTo( ARGUMENT_TEMPFILE, ARGUMENTLENGTH_TEMPFILE ) == ARGUMENTFOUND )
        {
            m_sTempFile = U2B(sArgument.copy( ARGUMENTLENGTH_TEMPFILE ));
        }
    }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
OUString LoginApplication::impl_encodeString( const OUString& sValue )
{
    // Read all signs from source buffer into destination buffer
    // and change all ";" and "\" to "\;" and "\\"!

    sal_Int32       nCount      = sValue.getLength();
    OUStringBuffer  sSource     ( sValue    )       ;
    OUStringBuffer  sDestination( nCount*2  )       ;   // Reserve destination buffer with enough free space for changes! Sometimes we must add signs ...
    sal_Unicode     cLetter                         ;

    for( sal_Int32 nLetter=0; nLetter<nCount; ++nLetter )
    {
        cLetter = sSource.charAt(nLetter);
        // If sign a special one ...
        // add escape letter before ...
        // and special one then!
        // Otherwise letter will copied normaly.
        if  (
                ( cLetter   ==  SEPERATOR   )   ||
                ( cLetter   ==  ENCODESIGN  )
            )
        {
            sDestination.append( ENCODESIGN );
        }
        sDestination.append( cLetter );
    }

    return sDestination.makeStringAndClear();
}
