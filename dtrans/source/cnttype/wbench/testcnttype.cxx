/*************************************************************************
 *
 *  $RCSfile: testcnttype.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: armin $ $Date: 2001-03-08 12:07:55 $
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


//_________________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPE_HPP_
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPEFACTORY_HPP_
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <stdio.h>

#include <vector>

//-------------------------------------------------------------
// my defines
//-------------------------------------------------------------

#define TEST_CLIPBOARD
#define RDB_SYSPATH  "d:\\projects\\src621\\dtrans\\wntmsci7\\bin\\applicat.rdb"

//------------------------------------------------------------
//  namesapces
//------------------------------------------------------------

using namespace ::rtl;
using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

//------------------------------------------------------------
//  globales
//------------------------------------------------------------

//################################################################

/*
void CheckMimeContentType( const OUString& aCntType, const OUString& aType, const OUString& aSubtype, sal_Int32 nParams )
{

    Reference< XMimeContentType > xMimeCntType = xMCntTypeFactory->createMimeContentType( aCntType );

    OSL_ASSERT( aType    == xMimeCntType->getMediaType ( ) );
    OSL_ASSERT( aSubtype == xMimeCntType->getMediaSubtype ( ) );

    try
    {
        Sequence< OUString > seqParam = xMimeCntType->getParameters( );
        OSL_ASSERT( seqParam.getLength( ) == nParams );

        OUString param;
        OUString pvalue;
        for ( sal_Int32 i = 0; i < seqParam.getLength( ); i++ )
        {
            param  = seqParam[i];
            OSL_ASSERT( xMimeCntType->hasParameter( param ) );

            pvalue = xMimeCntType->getParameterValue( param );
        }

        pvalue = xMimeCntType->getParameterValue( OUString::createFromAscii( "aparam" ) );
    }
    catch( IllegalArgumentException& )
    {
        printf( "FAILED: Invalid Mime Contenttype detected\n" );
    }
    catch( NoSuchElementException& )
    {

    }
}
*/

//----------------------------------------------------------------
//
//----------------------------------------------------------------

void ShutdownServiceMgr( Reference< XMultiServiceFactory >& SrvMgr )
{
    // Cast factory to XComponent
    Reference< XComponent > xComponent( SrvMgr, UNO_QUERY );

    if ( !xComponent.is() )
        OSL_ENSURE(sal_False, "Error shuting down");

    // Dispose and clear factory
    xComponent->dispose();
    SrvMgr.clear();
    SrvMgr = Reference< XMultiServiceFactory >();
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------

sal_Bool readCntTypesFromFileIntoVector( char* fname, vector< string >& vecData )
{
    FILE* fstream;

    fstream = fopen( fname, "r+" );
    if ( !fstream )
        return sal_False;

    // set pointer to file start
    fseek( fstream, 0L, SEEK_SET );

    char line[1024];
    while ( fscanf( fstream, "%[^\n]s", line ) != EOF )
    {
        vecData.push_back( line );
        fgetc( fstream );
    }

    fclose( fstream );

    return sal_True;
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------

sal_Bool processCntTypesAndWriteResultIntoFile( char* fname, vector< string >& vecData, Reference< XMimeContentTypeFactory > cnttypeFactory )
{
    FILE* fstream;

    fstream = fopen( fname, "w" );
    if ( !fstream )
        return sal_False;

    // set pointer to file start
    fseek( fstream, 0L, SEEK_SET );

    vector< string >::iterator iter_end = vecData.end( );
    const char* pStr = NULL;

    for ( vector< string >::iterator iter = vecData.begin( ); iter != iter_end; ++iter )
    {
        try
        {
            fprintf( fstream, "Gelesen: %s\n", iter->c_str( ) );

            Reference< XMimeContentType > xMCntTyp = cnttypeFactory->createMimeContentType( OUString::createFromAscii( iter->c_str( ) ) );

            fwprintf( fstream, OUString::createFromAscii( "Type: %s\n" ),  xMCntTyp->getMediaType( ).getStr( ) );
            fwprintf( fstream, OUString::createFromAscii( "Subtype: %s\n" ), xMCntTyp->getMediaSubtype( ).getStr( ) );

            Sequence< OUString > seqParam = xMCntTyp->getParameters( );
            sal_Int32 nParams = seqParam.getLength( );

            for ( sal_Int32 i = 0; i < nParams; i++ )
            {
                fwprintf( fstream, OUString::createFromAscii("PName: %s\n" ), seqParam[i].getStr( ) );
                fwprintf( fstream, OUString::createFromAscii("PValue: %s\n" ), xMCntTyp->getParameterValue( seqParam[i] ).getStr( ) );
            }
        }
        catch( IllegalArgumentException& ex )
        {
            fwprintf( fstream, OUString::createFromAscii( "Fehlerhafter Content-Type gelesen!!!\n\n" ) );
        }
        catch( NoSuchElementException& )
        {
            fwprintf( fstream, OUString::createFromAscii( "Parameterwert nicht vorhanden\n" ) );
        }
        catch( ... )
        {
            fwprintf( fstream, OUString::createFromAscii( "Unbekannter Fehler!!!\n\n" ) );
        }

        fwprintf( fstream, OUString::createFromAscii( "\n#############################################\n\n" ) );
    }

    fclose( fstream );

    return sal_True;
}

//----------------------------------------------------------------
//  main
//----------------------------------------------------------------

int SAL_CALL main( int nArgc, char* argv[] )
{
    if ( nArgc != 3 )
        printf( "Start with: testcnttype input-file output-file\n" );

    //-------------------------------------------------
    // get the global service-manager
    //-------------------------------------------------
    OUString rdbName = OUString( RTL_CONSTASCII_USTRINGPARAM( RDB_SYSPATH ) );
    Reference< XMultiServiceFactory > g_xFactory( createRegistryServiceFactory( rdbName ) );

    // Print a message if an error occured.
    if ( !g_xFactory.is( ) )
    {
        OSL_ENSURE(sal_False, "Can't create RegistryServiceFactory");
        return(-1);
    }

    vector< string > vecCntTypes;

    // open input-file and read the data
    if ( !readCntTypesFromFileIntoVector( argv[1], vecCntTypes ) )
    {
        printf( "Can't open input file" );
        ShutdownServiceMgr( g_xFactory );
    }

    Reference< XMimeContentTypeFactory >
        xMCntTypeFactory( g_xFactory->createInstance( OUString::createFromAscii( "com.sun.star.datatransfer.MimeContentTypeFactory" ) ), UNO_QUERY );

    if ( !xMCntTypeFactory.is( ) )
    {
        OSL_ENSURE( sal_False, "Error creating MimeContentTypeFactory Service" );
        return(-1);
    }

    if ( !processCntTypesAndWriteResultIntoFile( argv[2], vecCntTypes, xMCntTypeFactory ) )
    {
        printf( "Can't open output file" );
        ShutdownServiceMgr( g_xFactory );
    }

    //--------------------------------------------------
    // shutdown the service manager
    //--------------------------------------------------

    ShutdownServiceMgr( g_xFactory );

    return 0;
}
