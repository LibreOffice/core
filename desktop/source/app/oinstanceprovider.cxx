 /*************************************************************************
 *
 *  $RCSfile: oinstanceprovider.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 15:46:05 $
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

#include "oinstanceprovider.hxx"
#include "opluginframefactory.hxx"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
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
using namespace ::com::sun::star::bridge        ;
using namespace ::com::sun::star::container     ;

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________
#define INSTANCENAME_PLUGINFACTORY                          OUString(RTL_CONSTASCII_USTRINGPARAM("PluginFactory"))
#define INSTANCENAME_PLUGINFACTORY_SERVICEMANAGER           OUString(RTL_CONSTASCII_USTRINGPARAM("ServiceManager"))

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
OInstanceProvider::OInstanceProvider( const Reference< XMultiServiceFactory >& xFactory )
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
OInstanceProvider::~OInstanceProvider()
{
}

//*****************************************************************************************************************
//  XInterface
//*****************************************************************************************************************

void SAL_CALL OInstanceProvider::acquire() throw()
{ OWeakObject::acquire(); }

void SAL_CALL OInstanceProvider::release() throw()
{ OWeakObject::release(); }

Any SAL_CALL OInstanceProvider::queryInterface( const Type& aType ) throw( RuntimeException )
{
    Any aReturn ( ::cppu::queryInterface( aType, static_cast< XInstanceProvider* >( this ) ) );
    if ( aReturn.hasValue() == sal_False )
    {
        aReturn = OWeakObject::queryInterface( aType );
    }
    return aReturn;
}

//*****************************************************************************************************************
//  XInstanceProvider
//*****************************************************************************************************************
Reference< XInterface > SAL_CALL OInstanceProvider::getInstance( const OUString& sInstanceName ) throw( NoSuchElementException  ,
                                                                                                        RuntimeException        )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Safe impossible cases
    // This method is not defined for all incoming parameter.
    OSL_ENSURE( impldbg_checkParameter_getInstance( sInstanceName ), "OInstanceProvider::getInstance()\nInvalid parameter detected.\n" );

    // Set default return value
    Reference< XInterface > xReturn;

    // Attention: The created factories are helper services and not registered in registry!
    // We cant' instanciate it as service; we must do it dynamicly.

    // If a correct name for factory was given ...
    if ( sInstanceName == INSTANCENAME_PLUGINFACTORY )
    {
        // ... create this searched factory and set it for return.
        OPlugInFrameFactory* pPlugInFactory = new OPlugInFrameFactory( m_xFactory );
        xReturn = Reference< XInterface >( (OWeakObject*)pPlugInFactory, UNO_QUERY );
        // Safe impossible cases!
        // This operation can't failed.
        OSL_ENSURE( !(xReturn.is()==sal_False), "OInstanceProvider::getInstance()\nCreation of PlugInFactory failed!\n" );
    }
    else if (sInstanceName == INSTANCENAME_PLUGINFACTORY_SERVICEMANAGER)
    {
        xReturn = Reference< XInterface >( ::comphelper::getProcessServiceFactory(), UNO_QUERY );
    }
    else
    {
        // If an unknown name was given, we throw an exception!
        NoSuchElementException aTmpException;
        throw( aTmpException );
    }

    // Return result of this operation.
    return xReturn;
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
sal_Bool OInstanceProvider::impldbg_checkParameter_getInstance( const OUString& sInstanceName )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &sInstanceName    ==  NULL    )   ||
            (
                ( sInstanceName !=  INSTANCENAME_PLUGINFACTORY  && sInstanceName != INSTANCENAME_PLUGINFACTORY_SERVICEMANAGER )
            )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

#endif  //  #ifdef OSL_DEBUG_LEVEL
