/*************************************************************************
 *
 *  $RCSfile: opluginframefactory.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 15:46:26 $
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

#include "opluginframefactory.hxx"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::osl                           ;
using namespace ::rtl                           ;
using namespace ::cppu                          ;
using namespace ::com::sun::star::uno           ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::frame         ;

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________
#define SERVICENAME_DESKTOP                                 OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"               ))
#define SERVICENAME_PLUGINFRAME                             OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.mozilla.Plugin"              ))

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
OPlugInFrameFactory::OPlugInFrameFactory( const Reference< XMultiServiceFactory >& xFactory )
        //  Init baseclasses first
        //  Attention:
        //      Don't change order of initialization!
        //      OMutexMember is a struct with a mutex as member. We can't use a mutex as member directly, while
        //      we must garant right initialization and a valid value of this! First initialize
        //      baseclasses and then members. And we need the mutex for other baseclasses !!!
        :   OMutexMember(           )
        ,   OWeakObject (           )
        // Init member
        ,   m_xFactory  ( xFactory  )
{
}

//*****************************************************************************************************************
//  destructor (protected!)
//*****************************************************************************************************************
OPlugInFrameFactory::~OPlugInFrameFactory()
{
}

//*****************************************************************************************************************
//  XInterface
//*****************************************************************************************************************

void SAL_CALL OPlugInFrameFactory::acquire() throw()
{ OWeakObject::acquire(); }

void SAL_CALL OPlugInFrameFactory::release() throw()
{ OWeakObject::release(); }

Any SAL_CALL OPlugInFrameFactory::queryInterface( const Type& aType ) throw( RuntimeException )
{
    Any aReturn ( ::cppu::queryInterface( aType, static_cast< XSingleServiceFactory* >( this ) ) );
    if ( aReturn.hasValue() == sal_False )
    {
        aReturn = OWeakObject::queryInterface( aType );
    }
    return aReturn;
}
//*****************************************************************************************************************
//  XSingleServiceFactory
//*****************************************************************************************************************
Reference< XInterface > SAL_CALL OPlugInFrameFactory::createInstance() throw(   Exception       ,
                                                                                RuntimeException)
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Create new plugin.
    Reference< XInterface > xPlugIn( m_xFactory->createInstance( SERVICENAME_PLUGINFRAME ), UNO_QUERY );
    // Safe impossible cases
    OSL_ENSURE( !(xPlugIn.is()==sal_False), "OPlugInFrameFactory::createInstance()\nServicename of PlugIn is unknown!\n" );

    // Ouer plugin need information about his parent!
    // If someone dispatch anything to this plugin instance, he will create a new window and load the content.
    // Then he must append himself to ouer frame tree !!! => Thats why he need a valid reference to the desktop
    // as parent to do this.
    // Return result.
    return xPlugIn;
}

//*****************************************************************************************************************
//  XSingleServiceFactory
//*****************************************************************************************************************
Reference< XInterface > SAL_CALL OPlugInFrameFactory::createInstanceWithArguments( const Sequence< Any >& seqArguments ) throw( Exception       ,
                                                                                                                                RuntimeException)
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Safe impossible cases
    // This method is not defined for all incoming parameter.
    OSL_ENSURE( impldbg_checkParameter_createInstanceWithArguments( seqArguments ), "OPlugInFrameFactory::createInstanceWithArguments()\nInvalid parameter detected.\n" );

    // Create new PlugIn.
    Reference< XInterface > xPlugIn = createInstance();
    // Initialize it with given arguments.
    if ( xPlugIn.is()==sal_True )
    {
        // Before we must cast to right interface.
        Reference< XInitialization > xInit( xPlugIn, UNO_QUERY );
        // Safe impossible cases
        OSL_ENSURE( !(xInit.is()==sal_False), "OPlugInFrameFactory::createInstanceWithArguments()\nPlugIn don't support XInitialization ?!...\n" );
        xInit->initialize( seqArguments );
    }

    // Return result.
    return xPlugIn;
}

//_________________________________________________________________________________________________________________
//  debug methods
//_________________________________________________________________________________________________________________

/*-----------------------------------------------------------------------------------------------------------------
    The follow methods checks the parameter for other functions. If a parameter or his value is non valid,
    we return "sal_False". (else sal_True) This mechanism is used to throw an ASSERT!

    ATTENTION

        If you miss a test for one of this parameters, contact the autor or add it himself !(?)
        But ... look for right testing! See using of this methods!
-----------------------------------------------------------------------------------------------------------------*/

#if OSL_DEBUG_LEVEL > 0

//*****************************************************************************************************************
sal_Bool OPlugInFrameFactory::impldbg_checkParameter_createInstanceWithArguments( const Sequence< Any >& seqArguments )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &seqArguments             ==  NULL    )   ||
            ( seqArguments.getLength()  <   1       )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

#endif  //  #ifdef OSL_DEBUG_LEVEL
