/*************************************************************************
 *
 *  $RCSfile: urltransformer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: as $ $Date: 2001-03-29 13:17:15 $
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

#ifndef __FRAMEWORK_SERVICES_URLTRANSFORMER_HXX_
#include <services/urltransformer.hxx>
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

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

using namespace ::osl                           ;
using namespace ::rtl                           ;
using namespace ::cppu                          ;
using namespace ::com::sun::star::uno           ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::util          ;

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
URLTransformer::URLTransformer( const Reference< XMultiServiceFactory >& xFactory )
        //  Init baseclasses first
        //  Attention:
        //      Don't change order of initialization!
        //      OMutexMember is a struct with a mutex as member. We can't use a mutex as member, while
        //      we must garant right initialization and a valid value of this! First initialize
        //      baseclasses and then members. And we need the mutex for other baseclasses !!!
        :   OMutexMember    (           )
        ,   OWeakObject     (           )
        // Init member
        ,   m_xFactory      ( xFactory  )
{
    // Safe impossible cases.
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_URLTransformer( xFactory ), "URLTransformer::URLTransformer()\nInvalid parameter detected!\n" )
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
URLTransformer::~URLTransformer()
{
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************

DEFINE_XINTERFACE_3                 (   URLTransformer                      ,
                                        OWeakObject                         ,
                                        DIRECT_INTERFACE(XTypeProvider      ),
                                        DIRECT_INTERFACE(XServiceInfo       ),
                                        DIRECT_INTERFACE(XURLTransformer    )
                                    )

DEFINE_XTYPEPROVIDER_3              (   URLTransformer  ,
                                        XTypeProvider   ,
                                        XServiceInfo    ,
                                        XURLTransformer
                                    )

DEFINE_XSERVICEINFO_MULTISERVICE    (   URLTransformer                      ,
                                        SERVICENAME_URLTRANSFORMER          ,
                                        IMPLEMENTATIONNAME_URLTRANSFORMER
                                    )

// Attention: If you have enabled any testmode different from TEST_NOTHING => you have declared XDebugging-interface automaticly!
// Bhere is no macro to define and implement the right methods. You are the only one, who know - how you will use this mechanism.
// It exist a macro to switch on or off your implementation only.
// SERVICEDEBUG_ENABLED must be defined, to activate your code!

//#ifdef SERVICEDEBUG_ENABLED

    // Debug mechanism for services is not used in this implementation.
    // This define will expand to nothing if SERVICEDEBUG_ENABLED unknown!
    // But if you will implement your own code - activate this define before ... and deactivate follow line.
    //DEFINE_EMPTY_XSPECIALDEBUGINTERFACE( URLTransformer )

//#endif // #ifdef SERVICEDEBUG_ENABLED

//*****************************************************************************************************************
//  XURLTransformer
//*****************************************************************************************************************
sal_Bool SAL_CALL URLTransformer::parseStrict( URL& aURL ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "URLTransformer::parseStrict()" )
    // Safe impossible cases.
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_parseStrict( aURL ), "URLTransformer::parseStrict()\nInvalid parameter detected!\n" )

    // Initialize parser with given URL.
    INetURLObject aParser( aURL.Complete );

    // Get all information about this URL.
    aURL.Protocol   = INetURLObject::GetScheme( aParser.GetProtocol() );
    aURL.User       = aParser.GetUser   ( INetURLObject::DECODE_WITH_CHARSET );
    aURL.Password   = aParser.GetPass   ( INetURLObject::DECODE_WITH_CHARSET );
    aURL.Server     = aParser.GetHost   ( INetURLObject::DECODE_WITH_CHARSET );
    aURL.Port       = (sal_Int16)aParser.GetPort();
    aURL.Path       = aParser.GetURLPath( INetURLObject::DECODE_WITH_CHARSET );
//  aURL.Name       = aParser.GetName   (                                    );
    aURL.Arguments  = aParser.GetParam  ( INetURLObject::DECODE_WITH_CHARSET );
    aURL.Mark       = aParser.GetMark   ( INetURLObject::DECODE_WITH_CHARSET );

    aParser.SetMark ( OUString() );
    aParser.SetParam( OUString() );

    aURL.Main       = aParser.GetMainURL( INetURLObject::DECODE_WITH_CHARSET );

    // Return "URL is parsed".
    return sal_True;
}

//*****************************************************************************************************************
//  XURLTransformer
//*****************************************************************************************************************
sal_Bool SAL_CALL URLTransformer::parseSmart(           URL&        aURL            ,
                                                const   OUString&   sSmartProtocol  ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "URLTransformer::parseSmart()" )
    // Safe impossible cases.
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_parseSmart( aURL, sSmartProtocol ), "URLTransformer::parseSmart()\nInvalid parameter detected!\n" )

    // Initialize parser with given URL.
    INetURLObject aParser;

    aParser.SetSmartProtocol( INetURLObject::CompareProtocolScheme( sSmartProtocol ));
    aParser.SetSmartURL     ( aURL.Complete                                         );

    // Get all information about this URL.
    aURL.Protocol   = INetURLObject::GetScheme( aParser.GetProtocol() );
    aURL.User       = aParser.GetUser   ( INetURLObject::DECODE_WITH_CHARSET );
    aURL.Password   = aParser.GetPass   ( INetURLObject::DECODE_WITH_CHARSET );
    aURL.Server     = aParser.GetHost   ( INetURLObject::DECODE_WITH_CHARSET );
    aURL.Port       = (sal_Int16)aParser.GetPort();
    aURL.Path       = aParser.GetURLPath( INetURLObject::DECODE_WITH_CHARSET );
//  aURL.Name       = aParser.GetName   ();
    aURL.Arguments  = aParser.GetParam  ( INetURLObject::DECODE_WITH_CHARSET );
    aURL.Mark       = aParser.GetMark   ( INetURLObject::DECODE_WITH_CHARSET );

    aParser.SetMark ( OUString() );
    aParser.SetParam( OUString() );

    aURL.Main       = aParser.GetMainURL( INetURLObject::DECODE_WITH_CHARSET );

    // Return "URL is parsed".
    return sal_True;
}

//*****************************************************************************************************************
//  XURLTransformer
//*****************************************************************************************************************
sal_Bool SAL_CALL URLTransformer::assemble( URL& aURL ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "URLTransformer::assemble()" )
    // Safe impossible cases.
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_assemble( aURL ), "URLTransformer::assemble()\nInvalid parameter detected!\n" )

    // Initialize parser.
    INetURLObject aParser;

    aParser.ConcatData( INetURLObject::CompareProtocolScheme( aURL.Protocol )   ,
                         aURL.User                                              ,
                        aURL.Password                                           ,
                        aURL.Server                                             ,
                         aURL.Port                                              ,
                        aURL.Path                                               );

    // First parse URL WITHOUT ...
    aURL.Main = aParser.GetMainURL( INetURLObject::DECODE_WITH_CHARSET );
    // ...and then WITH parameter and mark.
    aParser.SetParam( aURL.Arguments);
    aParser.SetMark ( aURL.Mark     );
    aURL.Complete = aParser.GetMainURL( INetURLObject::DECODE_WITH_CHARSET );

    // Return "URL is assembled".
    return sal_True;
}

//*****************************************************************************************************************
//  XURLTransformer
//*****************************************************************************************************************
OUString SAL_CALL URLTransformer::getPresentation(  const   URL&        aURL            ,
                                                            sal_Bool    bWithPassword   ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "URLTransformer::getPresentation()" )
    // Safe impossible cases.
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_getPresentation( aURL, bWithPassword ), "URLTransformer::getPresentation()\nInvalid parameter detected!\n" )

    // Convert internal URLs to "praesentation"-URLs!
    UniString   sPraesentationURL;
    UniString   sURL = aURL.Complete;
    INetURLObject::translateToExternal( sURL, sPraesentationURL );

    return sPraesentationURL;
}

//_________________________________________________________________________________________________________________
//  debug methods
//_________________________________________________________________________________________________________________

/*-----------------------------------------------------------------------------------------------------------------
    The follow methods checks the parameter for other functions. If a parameter or his value is non valid,
    we return "sal_False". (else sal_True) This mechanism is used to throw an ASSERT!

    ATTENTION

        If you miss a test for one of this parameters, contact Andreas Schluens [as] or add it himself !(?)
        But ... look for right testing! See using of this methods!
-----------------------------------------------------------------------------------------------------------------*/

#ifdef ENABLE_ASSERTIONS

//*****************************************************************************************************************
// We need a valid factory to work with her.
sal_Bool URLTransformer::impldbg_checkParameter_URLTransformer( const Reference< XMultiServiceFactory >& xFactory )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xFactory     ==  NULL        )   ||
            ( xFactory.is() ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// We need a valid "Complete" value for parsing.
sal_Bool URLTransformer::impldbg_checkParameter_parseStrict( URL& aURL )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &aURL                     ==  NULL    )   ||
            ( aURL.Complete.getLength() <   1       )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// We need a valid "Complete" value for URL and a non empty smart protocol description.
sal_Bool URLTransformer::impldbg_checkParameter_parseSmart(         URL&        aURL            ,
                                                            const   OUString&   sSmartProtocol  )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &aURL                         ==  NULL    )   ||
            ( &sSmartProtocol               ==  NULL    )   ||
            ( aURL.Complete.getLength()     <   1       )   ||
            ( sSmartProtocol.getLength()    <   1       )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
sal_Bool URLTransformer::impldbg_checkParameter_assemble( URL& aURL )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &aURL == NULL )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
sal_Bool URLTransformer::impldbg_checkParameter_getPresentation (   const   URL&        aURL            ,
                                                                            sal_Bool    bWithPassword   )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &aURL                     ==  NULL        )   ||
            ( aURL.Complete.getLength() <   1           )   ||
            (
                ( bWithPassword         !=  sal_True    )   &&
                ( bWithPassword         !=  sal_False   )
            )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

#endif  //  #ifdef ENABLE_ASSERTIONS

}       //  namespace framework
