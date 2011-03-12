/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_connectivity.hxx"
#include "LConfigAccess.hxx"
#include "LDriver.hxx"
#include "LDebug.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

//.........................................................................
namespace connectivity
{
    namespace evoab
    {
        //-----------------------------------------------------------------
        Reference< XPropertySet > createDriverConfigNode( Reference< XMultiServiceFactory > _rxORB, ::rtl::OUString _sDriverImplementationName )
        {
            OSL_TRACE("createDriverConfigNode()entered");

            Reference< XPropertySet > xNode;
            try
            {
                //=============================================================
                // create the config provider
                Reference< XMultiServiceFactory > xConfigProvider(
                    _rxORB->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationProvider")) ),
                    UNO_QUERY
                );
                OSL_ENSURE( xConfigProvider.is(), "createDriverConfigNode: could not create the config provider!" );

                if ( xConfigProvider.is() )
                {
                    ::rtl::OUString sCompleteNodePath(RTL_CONSTASCII_USTRINGPARAM ("/org.openoffice.Office.DataAccess/DriverSettings/" ));
                    sCompleteNodePath += _sDriverImplementationName;
                    //sCompleteNodePath += OEvoabConnection::getDriverImplementationName();
                    //sCompleteNodePath += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.comp.sdbc.MozabDriver"));
                    EVO_TRACE_STRING("createDriverConfigNode()::sCompleteNodePath = %s\n", sCompleteNodePath );

                    //=========================================================
                    // arguments for creating the config access
                    Sequence< Any > aArguments(2);
                    // the path to the node to open
                    aArguments[0] <<= PropertyValue(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("nodepath")),
                        0,
                        makeAny( sCompleteNodePath ),
                        PropertyState_DIRECT_VALUE
                    );
                    // the depth: -1 means unlimited
                    aArguments[1] <<= PropertyValue(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("depth")),
                        0,
                        makeAny( (sal_Int32)-1 ),
                        PropertyState_DIRECT_VALUE
                    );

                    //=========================================================
                    // create the access
                    Reference< XInterface > xAccess = xConfigProvider->createInstanceWithArguments(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationAccess")),
                        aArguments
                    );
                    OSL_ENSURE( xAccess.is(), "createDriverConfigNode: invalid access returned (should throw an exception instead)!" );

                    xNode = xNode.query( xAccess );
                }
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "createDriverConfigNode: caught an exception while accessing the driver's config node!" );
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
                static Reference< XMultiServiceFactory > xEvoabServiceFactory;
                return xEvoabServiceFactory;
            }
        }

        //-----------------------------------------------------------------
        void setEvoabServiceFactory( const Reference< XMultiServiceFactory >& _rxFactory )
        {
            accessFactoryStorage( ) = _rxFactory;
        }

        //-----------------------------------------------------------------
        const Reference< XMultiServiceFactory >& getEvoabServiceFactory( )
        {
            return accessFactoryStorage( );
        }

        //-----------------------------------------------------------------
        ::rtl::OUString getFullPathExportingCommand( Reference< XMultiServiceFactory > _rxORB )
        {
            ::rtl::OUString sFullPathExportingCommand;

            //Reference< XMultiServiceFactory > xFactory = getEvoabServiceFactory();
            //OSL_ENSURE( xFactory.is(), "getPreferredProfileName: invalid service factory!" );
            OSL_ENSURE( _rxORB.is(), "getFullPathExportingCommand: invalid service factory!" );
            if ( _rxORB.is() )
            {
                try
                {
                    Reference< XPropertySet > xDriverNode = createDriverConfigNode( _rxORB, OEvoabDriver::getImplementationName_Static() );
                    Reference< XPropertySet > xEvoPrefsNode;
                    if ( xDriverNode.is() )
                        xDriverNode->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EvolutionPreferences")) ) >>= xEvoPrefsNode;
                    OSL_ENSURE( xEvoPrefsNode.is(), "getFullPathExportingCommand: could not access the node for the evolution preferences!" );
                    if ( xEvoPrefsNode.is() )
                        xEvoPrefsNode->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FullPathExportingCommand")) ) >>= sFullPathExportingCommand;
                }
                catch( const Exception& )
                {
                    OSL_ENSURE( sal_False, "getFullPathExportingCommand: caught an exception!" );
                }
            }
            return sFullPathExportingCommand;
        }
    }
}

//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
