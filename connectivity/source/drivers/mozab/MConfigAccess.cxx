/*************************************************************************
 *
 *  $RCSfile: MConfigAccess.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:38:28 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONNECTIVITY_MOZAB_MCONFIGACCESS_HXX
#include "MConfigAccess.hxx"
#endif
#ifndef CONNECTIVITY_MOZAB_MEXTCONFIGACCESS_HXX
#include "MExtConfigAccess.hxx"
#endif
#ifndef CONNECTIVITY_SCONNECTION_HXX
#include "MConnection.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

//.........................................................................
namespace connectivity
{
    namespace mozab
    {
        //-----------------------------------------------------------------
        Reference< XPropertySet > createDriverConfigNode( Reference< XMultiServiceFactory > _rxORB )
        {
            Reference< XPropertySet > xNode;
            try
            {
                //같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
                // create the config provider
                Reference< XMultiServiceFactory > xConfigProvider(
                    _rxORB->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.configuration.ConfigurationProvider" ) ),
                    UNO_QUERY
                );
                OSL_ENSURE( xConfigProvider.is(), "createDriverConfigNode: could not create the config provider!" );

                if ( xConfigProvider.is() )
                {
                    ::rtl::OUString sCompleteNodePath = ::rtl::OUString::createFromAscii ("/org.openoffice.Office.DataAccess/DriverSettings/" );
                    sCompleteNodePath += OConnection::getDriverImplementationName();

                    //같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
                    // arguments for creating the config access
                    Sequence< Any > aArguments(2);
                    // the path to the node to open
                    aArguments[0] <<= PropertyValue(
                        ::rtl::OUString::createFromAscii( "nodepath"),
                        0,
                        makeAny( sCompleteNodePath ),
                        PropertyState_DIRECT_VALUE
                    );
                    // the depth: -1 means unlimited
                    aArguments[1] <<= PropertyValue(
                        ::rtl::OUString::createFromAscii( "depth"),
                        0,
                        makeAny( (sal_Int32)-1 ),
                        PropertyState_DIRECT_VALUE
                    );

                    //같같같같같같같같같같같같같같같같같같같같같같같같같같같같�
                    // create the access
                    Reference< XInterface > xAccess = xConfigProvider->createInstanceWithArguments(
                        ::rtl::OUString::createFromAscii( "com.sun.star.configuration.ConfigurationAccess" ),
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
                        xDriverNode->getPropertyValue( ::rtl::OUString::createFromAscii( "MozillaPreferences" ) ) >>= xMozPrefsNode;
                    OSL_ENSURE( xMozPrefsNode.is(), "getPreferredProfileName: could not access the node for the mozilla preferences!" );
                    if ( xMozPrefsNode.is() )
                        xMozPrefsNode->getPropertyValue( ::rtl::OUString::createFromAscii( "ProfileName" ) ) >>= sPreferredName;
                }
                catch( const Exception& )
                {
                    OSL_ENSURE( sal_False, "getPreferredProfileName: caught an exception!" );
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
        sUserProfile = ::connectivity::mozab::getPreferredProfileName( );
        bReadConfig = sal_True;
    }

    return sUserProfile.getStr();
}

//-------------------------------------------------------------------------
extern "C" void SAL_CALL setMozabServiceFactory( void* _pFactory )
{
    Reference< XMultiServiceFactory > xFactory = static_cast< XMultiServiceFactory* >( _pFactory );
    ::connectivity::mozab::setMozabServiceFactory( xFactory );

    // by definition, the object behind the interface pointer has been acquired once for purpose
    // of safely transporting it
    xFactory->release();
}

