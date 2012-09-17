/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "com/sun/star/configuration/theDefaultProvider.hpp"
#include "comphelper/processfactory.hxx"

#include "MConfigAccess.hxx"
#include "MExtConfigAccess.hxx"
#include "MConnection.hxx"
#include "MDriver.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

//.........................................................................
namespace connectivity
{
    namespace mork
    {
        //-----------------------------------------------------------------
        Reference< XPropertySet > createDriverConfigNode( Reference< XMultiServiceFactory > _rxORB )
        {
            Reference< XPropertySet > xNode;
            try
            {
                //=============================================================
                // create the config provider
                Reference< XMultiServiceFactory > xConfigProvider(
                    com::sun::star::configuration::theDefaultProvider::get(
                        comphelper::getComponentContext( _rxORB ) ) );

                ::rtl::OUString sCompleteNodePath(RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.DataAccess/DriverSettings/" ));
                sCompleteNodePath += MorkDriver::getImplementationName_Static(  );

                //=========================================================
                // arguments for creating the config access
                Sequence< Any > aArguments(2);
                // the path to the node to open
                aArguments[0] <<= PropertyValue(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("nodepath")),
                    0,
                    makeAny( sCompleteNodePath ),
                    PropertyState_DIRECT_VALUE
                );
                // the depth: -1 means unlimited
                aArguments[1] <<= PropertyValue(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("depth")),
                    0,
                    makeAny( (sal_Int32)-1 ),
                    PropertyState_DIRECT_VALUE
                );

                //=========================================================
                // create the access
                Reference< XInterface > xAccess = xConfigProvider->createInstanceWithArguments(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationAccess" )),
                    aArguments
                );
                OSL_ENSURE( xAccess.is(), "createDriverConfigNode: invalid access returned (should throw an exception instead)!" );

                xNode = xNode.query( xAccess );
            }
            catch( const Exception& )
            {
                OSL_FAIL( "createDriverConfigNode: caught an exception while accessing the driver's config node!" );
            }

            // outta here
            return xNode;
        }

        //-----------------------------------------------------------------
        namespace
        {
            // a private helper to accessing the point where we store the reference
            // to the factory
            Reference< XMultiServiceFactory >& accessFactoryStorage( )
            {
                static Reference< XMultiServiceFactory > xMozabServiceFactory;
                return xMozabServiceFactory;
            }
        }

        //-----------------------------------------------------------------
        void setMozabServiceFactory( const Reference< XMultiServiceFactory >& _rxFactory )
        {
            accessFactoryStorage( ) = _rxFactory;
        }

        //-----------------------------------------------------------------
        const Reference< XMultiServiceFactory >& getMozabServiceFactory( )
        {
            return accessFactoryStorage( );
        }

        //-----------------------------------------------------------------
        ::rtl::OUString getDescription(const sal_Char* sNode,const ::rtl::OUString & sDefault)
        {
            ::rtl::OUString sPreferredName;
            ::rtl::OUString sDescription;

            Reference< XMultiServiceFactory > xFactory = getMozabServiceFactory();
            OSL_ENSURE( xFactory.is(), "getPreferredProfileName: invalid service factory!" );
            if ( xFactory.is() )
            {
                try
                {
                    Reference< XPropertySet > xDriverNode = createDriverConfigNode( xFactory );
                    Reference< XPropertySet > xMozPrefsNode;
                    if ( xDriverNode.is() )
                        xDriverNode->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("MozillaPreferences" )) ) >>= xMozPrefsNode;
                    OSL_ENSURE( xMozPrefsNode.is(), "getPreferredProfileName: could not access the node for the mozilla preferences!" );
                    if ( xMozPrefsNode.is() )
                        xMozPrefsNode->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ProfileName" )) ) >>= sPreferredName;
                    if ( xMozPrefsNode.is() )
                        xMozPrefsNode->getPropertyValue( ::rtl::OUString::createFromAscii(sNode) ) >>= sDescription;
                    if (sDescription.getLength() == 0)
                        sDescription = sDefault;
                }
                catch( const Exception& )
                {
                    OSL_FAIL( "getDescription: caught an exception!" );
                }
            }
            if (sDescription.getLength() == 0)
                sDescription = sDefault;
            return sDescription;
        }
        //-----------------------------------------------------------------
        ::rtl::OUString getPreferredProfileName( )
        {
            ::rtl::OUString sPreferredName;

            Reference< XMultiServiceFactory > xFactory = getMozabServiceFactory();
            OSL_ENSURE( xFactory.is(), "getPreferredProfileName: invalid service factory!" );
            if ( xFactory.is() )
            {
                try
                {
                    Reference< XPropertySet > xDriverNode = createDriverConfigNode( xFactory );
                    Reference< XPropertySet > xMozPrefsNode;
                    if ( xDriverNode.is() )
                        xDriverNode->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("MozillaPreferences" )) ) >>= xMozPrefsNode;
                    OSL_ENSURE( xMozPrefsNode.is(), "getPreferredProfileName: could not access the node for the mozilla preferences!" );
                    if ( xMozPrefsNode.is() )
                        xMozPrefsNode->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ProfileName" )) ) >>= sPreferredName;
                }
                catch( const Exception& )
                {
                    OSL_FAIL( "getPreferredProfileName: caught an exception!" );
                }
            }
            return sPreferredName;
        }
    }
}

//.........................................................................

//-------------------------------------------------------------------------
extern "C" const sal_Unicode* SAL_CALL getUserProfile( void )
{
    static sal_Bool         bReadConfig = sal_False;
    static ::rtl::OUString  sUserProfile;
    if ( !bReadConfig )
    {
        sUserProfile = ::connectivity::mork::getPreferredProfileName( );
        bReadConfig = sal_True;
    }

    return sUserProfile.getStr();
}
//------------------------------------------------------------------------
extern "C" const sal_Char* SAL_CALL getPabDescription( void )
{
    static sal_Bool         bReadConfig = sal_False;
    static ::rtl::OUString  usPabDescription;
    static ::rtl::OString   sPabDescription;

    if ( !bReadConfig )
    {
        usPabDescription = ::connectivity::mork::getDescription(
                            "PabDescription" ,
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Personal Address Book" )));
        sPabDescription = ::rtl::OUStringToOString( usPabDescription,
                                                 RTL_TEXTENCODING_UTF8);
        bReadConfig = sal_True;
    }

    return sPabDescription.getStr();
}

//-------------------------------------------------------------------------
extern "C" const sal_Char* SAL_CALL getHisDescription( void )
{
    static sal_Bool         bReadConfig = sal_False;
    static ::rtl::OUString  usHisDescription;
    static ::rtl::OString   sHisDescription;

    if ( !bReadConfig )
    {
        usHisDescription = ::connectivity::mork::getDescription(
                            "HisDescription" ,
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Collected Addresses" )));
        sHisDescription = ::rtl::OUStringToOString( usHisDescription,
                                                 RTL_TEXTENCODING_UTF8);
        bReadConfig = sal_True;
    }

    return sHisDescription.getStr();
}

//-------------------------------------------------------------------------
// MConfigAccess was invented to allow non-UNO parts access to the configuration.
// Unfortunately, configuration access requires a XMultiServiceFactory - which the
// mozilla side does not have.
// So we create a "library-local" service factory here: Every need for a service
// factory can be fullfilled by this factory (similar to the get/setProcessServiceFactory
// in comphelper).
// This is halfway valid, as usually, the mozabdrv library is invoked from the mozab library
// only. The latter contains the driver class (and only this class and nothing more), and
// the driver class is a singleton. The driver itself is created with a service factory,
// which (by definition) can and should be used for all subsequent service requests.
// And this is exactly what we're allowing with the following functions ....

/** _pFactory must point to an XMultiServiceFactory, which must be aquired once
    for purpose of safely transfering it. The callee will release this interface
    when it has stored the pointer somewhere else.
*/
extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL setMozabServiceFactory(
    void* _pFactory )
{
    Reference< XMultiServiceFactory > xFactory = static_cast< XMultiServiceFactory* >( _pFactory );
  //  ::connectivity::mozab::setMozabServiceFactory( xFactory );

    // by definition, the object behind the interface pointer has been acquired once for purpose
    // of safely transporting it
    xFactory->release();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
