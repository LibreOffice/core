/*************************************************************************
*
*  $RCSfile: ScriptStorage.cxx,v $
*
*  $Revision: 1.6 $
*  last change: $Author: jmrice $ $Date: 2002-09-30 12:57:02 $
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
#include <osl/file.hxx>
#include <osl/time.h>
#include <cppuhelper/implementationentry.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

#include <drafts/com/sun/star/script/framework/storage/ScriptImplInfo.hpp>
#include <util/util.hxx>

#include "ScriptInfo.hxx"
#include "ScriptStorage.hxx"
#include "ScriptMetadataImporter.hxx"
#include "ScriptElement.hxx"

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;

namespace scripting_impl
{

const sal_Char* const SERVICE_NAME = "drafts.com.sun.star.script.framework.storage.ScriptStorage";
const sal_Char* const IMPL_NAME = "drafts.com.sun.star.script.framework.storage.ScriptStorage";

const sal_Char * const SCRIPT_DIR = "/Scripts";
const sal_Char * const SCRIPT_PARCEL = "/parcel.xml";
const sal_Char * const SCRIPT_PARCEL_NAME_ONLY = "parcel";

static OUString ss_implName = OUString::createFromAscii( IMPL_NAME );
static OUString ss_serviceName = OUString::createFromAscii( SERVICE_NAME );
static Sequence< OUString > ss_serviceNames = Sequence< OUString >( &ss_serviceName, 1 );

const sal_uInt16 NUMBER_STORAGE_INITIALIZE_ARGS = 3;

extern ::rtl_StandardModuleCount s_moduleCount;

//*************************************************************************
ScriptStorage::ScriptStorage( const Reference <
                              XComponentContext > & xContext )
        : m_xContext( xContext ), m_bInitialised( false )
{
    OSL_TRACE( "< ScriptStorage ctor called >\n" );
    s_moduleCount.modCnt.acquire( &s_moduleCount.modCnt );

    validateXRef( m_xContext, "ScriptStorage::ScriptStorage : cannot get component context" );

    m_xMgr = m_xContext->getServiceManager();
    validateXRef( m_xMgr, "ScriptStorage::ScriptStorage : cannot get service manager" );
}

//*************************************************************************
ScriptStorage::~ScriptStorage() SAL_THROW( () )
{
    OSL_TRACE( "< ScriptStorage dtor called >\n" );
    s_moduleCount.modCnt.release( &s_moduleCount.modCnt );
}

//*************************************************************************
void
ScriptStorage::initialize(
    const Sequence <Any> & args )
throw ( RuntimeException, Exception )
{
    OSL_TRACE( "Entering ScriptStorage::initialize\n" );
    ::rtl::OUString xStringUri;

    // Should not be renitialised
    if ( m_bInitialised )
    {
        throw RuntimeException( OUSTR( "ScriptStorage::initalize already initialized" ),
                                Reference<XInterface> () );
    }

    {   // Protect member variable writes
        ::osl::Guard< osl::Mutex > aGuard( m_mutex );

        // Check args
        if ( args.getLength() != NUMBER_STORAGE_INITIALIZE_ARGS )
        {
            OSL_TRACE( "ScriptStorage::initialize: got no args\n" );
            throw lang::IllegalArgumentException(
                OUSTR( "Invalid number of arguments provided!" ),
                Reference< XInterface >(), 0 );
        }

        if ( sal_False == ( args[ 0 ] >>= m_xSimpleFileAccess ) )
        {
            throw lang::IllegalArgumentException(
                OUSTR( "Invalid XSimpleFileAccess argument provided!" ),
                Reference< XInterface >(), 0 );
        }

        if ( sal_False == ( args[ 1 ] >>= m_scriptStorageID ) )
        {
            throw lang::IllegalArgumentException(
                OUSTR( "Invalid ScriptStorage ID argument provided!" ),
                Reference< XInterface >(), 0 );
        }

        if ( sal_False == ( args[ 2 ] >>= xStringUri ) )
        {
            throw lang::IllegalArgumentException(
                OUSTR( "Invalid String Uri argument provided!" ),
                Reference< XInterface >(), 0 );
        }
    } // End - Protect member variable writes

#ifdef _DEBUG
    fprintf( stderr, "uri: %s\n", ::rtl::OUStringToOString( xStringUri, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
#endif

    try
    {
        ScriptMetadataImporter* SMI = new ScriptMetadataImporter( m_xContext );
        Reference<xml::sax::XExtendedDocumentHandler> xSMI( SMI );


        xStringUri = xStringUri.concat( ::rtl::OUString::createFromAscii( SCRIPT_DIR ) );

        // get the list of language folders under the Scripts directory
        Sequence< ::rtl::OUString > languageDirs =
            m_xSimpleFileAccess->getFolderContents( xStringUri, true );

        Reference< io::XInputStream > xInput;
        sal_Int32 languageDirsLength = languageDirs.getLength();
        for ( sal_Int32 i = 0; i < languageDirsLength ; ++i )
        {
#ifdef _DEBUG
            fprintf( stderr, "contains: %s\n", ::rtl::OUStringToOString( languageDirs[ i ],
                     RTL_TEXTENCODING_ASCII_US ).pData->buffer );
#endif

            if ( ! m_xSimpleFileAccess->isFolder( languageDirs[ i ] ) )
            {
                continue;
            }

            //get the list of parcel folders for each language folder
            // under Scripts
            Sequence< ::rtl::OUString > parcelDirs =
                m_xSimpleFileAccess->getFolderContents( languageDirs[ i ], true );

            sal_Int32 parcelDirsLength = parcelDirs.getLength();
            for ( sal_Int32 j = 0; j < parcelDirsLength ; ++j )
            {
#ifdef _DEBUG
                fprintf( stderr, "contains: %s\n",
                         ::rtl::OUStringToOString( parcelDirs[ j ],
                                                   RTL_TEXTENCODING_ASCII_US ).pData->buffer );
#endif

                OUString parcelFile = parcelDirs[ j ].concat(
                                          ::rtl::OUString::createFromAscii( SCRIPT_PARCEL ) );

                // Do not have a valid parcel.xml
                if ( !m_xSimpleFileAccess->exists( parcelFile ) ||
                        m_xSimpleFileAccess->isFolder( parcelFile ) )
                {
                    continue;
                }
#ifdef _DEBUG
                fprintf( stderr,
                         "parcel file: %s\n",
                         ::rtl::OUStringToOString( parcelFile, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
#endif

                xInput = m_xSimpleFileAccess->openFileRead( parcelFile );
                // Failed to get input stream
                if ( !xInput.is() )
                {
                    continue;
                }

                OSL_TRACE( "Parse the metadata \n" );
                Impls_vec vScriptImplInfos;
                try
                {
                    vScriptImplInfos = SMI->parseMetaData( xInput, parcelDirs[ j ] );
                }
                catch ( xml::sax::SAXException saxe )
                {
                    if ( xInput.is() )
                    {
                        xInput->closeInput();
                    }
                    OSL_TRACE( "caught com::sun::star::xml::sax::SAXException in ScriptStorage::initalize" );
                    continue;
                }
                catch ( io::IOException ioe )
                {
                    if ( xInput.is() )
                    {
                        xInput->closeInput();
                    }
                    OSL_TRACE( "caught com::sun::star::io::IOException in ScriptStorage::initalize" );
                    continue;
                }
                xInput->closeInput();

                updateMaps( vScriptImplInfos );
            }
        }
    }
    catch ( io::IOException ioe )
    {
        //From ScriptMetadata Importer
        OSL_TRACE( "caught com::sun::star::io::IOException in ScriptStorage::initalize" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::initalize IOException: " ).concat( ioe.Message ),
            Reference<XInterface> () );

    }
    catch ( ucb::CommandAbortedException cae )
    {
        OSL_TRACE( "caught com::sun::star::ucb::CommandAbortedException in ScriptStorage::initialize" );
        throw RuntimeException(
            OUSTR(
                "ScriptStorage::initalize CommandAbortedException: " ).concat( cae.Message ),
            Reference<XInterface> () );
    }
    catch ( RuntimeException re )
    {
        OSL_TRACE( "caught com::sun::star::uno::RuntimeException in ScriptStorage::initialize" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::initalize RuntimeException: " ).concat( re.Message ),
            Reference<XInterface> () );
    }
    catch ( Exception ue )
    {
        OSL_TRACE( "caught com::sun::star::uno::Exception in ScriptStorage::initialize" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::initalize Exception: " ).concat( ue.Message ),
            Reference<XInterface> () );
    }
#ifdef _DEBUG
    catch ( ... )
    {
        OSL_TRACE( "caught com::sun::star::uno::Exception in ScriptStorage::initialize" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::initalize unknown exception: " ),
            Reference<XInterface> () );
    }
#endif

    OSL_TRACE( "Parsed the XML\n" );

    m_bInitialised = true;
}

//*************************************************************************
// private method for updating hashmaps
void
ScriptStorage::updateMaps( const Impls_vec & vScriptImplInfos )
{
    ::osl::Guard< osl::Mutex > aGuard( m_mutex );

    Impls_vec::const_iterator it_end = vScriptImplInfos.end();
    // step through the vector of ScripImplInfos returned from parse
    for ( Impls_vec::const_iterator it = vScriptImplInfos.begin() ; it != it_end; ++it )
    {
        //find the Impls_vec for this logical name
        ScriptInfo_hash::iterator h_it = mh_implementations.find( it->logicalName );

        if ( h_it == mh_implementations.end() )
        {
            //if it's null, need to create a new Impls_vec
#ifdef _DEBUG
            fprintf( stderr,
                     "updateMaps: new logical name: %s\n", rtl::OUStringToOString(
                         it->logicalName, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            fprintf( stderr, "language name: %s\n",
                     rtl::OUStringToOString(
                         it->functionName, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
#endif

            Impls_vec v;
            v.push_back( *it );
            mh_implementations[ it->logicalName ] = v;
        }
        else
        {
#ifdef _DEBUG
            fprintf( stderr, "updateMaps: existing logical name: %s\n", rtl::OUStringToOString( it->logicalName, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            fprintf( stderr, "                    language name: %s\n", rtl::OUStringToOString( it->functionName, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
#endif

            h_it->second.push_back( *it );
        }
    }
}

//*************************************************************************
// Not part of the interface yet, ie. not in the idl, and it should be!!
void
ScriptStorage::save()
throw ( RuntimeException )
{
    ::osl::Guard< osl::Mutex > aGuard( m_mutex );
    Reference< io::XActiveDataSource > xSource;
    Reference<io::XOutputStream> xOS;
    // xScriptInvocation = Reference<XScriptInvocation>(xx, UNO_QUERY_THROW);
    Reference<xml::sax::XExtendedDocumentHandler> xHandler;

    OUString parcel_suffix = OUString::createFromAscii( SCRIPT_PARCEL );
    OUString ou_parcel = OUString( RTL_CONSTASCII_USTRINGPARAM( SCRIPT_PARCEL_NAME_ONLY ) );

    try
    {
        ScriptInfo_hash::iterator it_end = mh_implementations.end();
        for ( ScriptInfo_hash::iterator it = mh_implementations.begin() ; it != it_end; ++it )
        {
            ::rtl::OUString logName = it->first;
            Impls_vec::iterator it_impls_end = it->second.end();
            for ( Impls_vec::iterator it_impls = it->second.begin();
                    it_impls != it_impls_end ; ++it_impls )
            {
                ScriptOutput_hash::const_iterator it_parcels =
                    mh_parcels.find( it_impls->parcelURI );
                if ( it_parcels == mh_parcels.end() )
                {
                    //create new outputstream
                    OUString parcel_xml_path = it_impls->parcelURI.concat( parcel_suffix );
                    m_xSimpleFileAccess->kill( parcel_xml_path );
                    xOS = m_xSimpleFileAccess->openFileWrite( parcel_xml_path );
#ifdef _DEBUG

                    fprintf( stderr, "saving: %s\n", rtl::OUStringToOString( it_impls->parcelURI.concat( OUString::createFromAscii( "/parcel.xml" ) ), RTL_TEXTENCODING_ASCII_US ).pData->buffer );
#endif

                    Reference< XInterface > xInterface =
                        m_xMgr->createInstanceWithContext(
                            OUString::createFromAscii( "com.sun.star.xml.sax.Writer" ),
                            m_xContext );
                    validateXRef( xInterface, "ScriptStorage::save: cannot get sax.Writer" );
                    xHandler =
                        Reference<xml::sax::XExtendedDocumentHandler>( xInterface,
                                UNO_QUERY_THROW );
                    xSource = Reference< io::XActiveDataSource >( xHandler, UNO_QUERY_THROW );
                    xSource->setOutputStream( xOS );

                    writeMetadataHeader( xHandler );

                    xHandler->startElement( ou_parcel,
                                            Reference< xml::sax::XAttributeList >() );

                    mh_parcels[ it_impls->parcelURI ] = xHandler;
                }
                else
                {
                    xHandler = it_parcels->second;
                }

                ScriptElement* pSE = new ScriptElement( *it_impls );
                // this is to get pSE released correctly
                Reference <xml::sax::XAttributeList> xal( pSE );
                pSE->dump( xHandler );
            }
        }

        ScriptOutput_hash::const_iterator out_it_end = mh_parcels.end();

        for ( ScriptOutput_hash::const_iterator out_it = mh_parcels.begin();
                out_it != out_it_end;
                ++out_it )
        {
            out_it->second->ignorableWhitespace( ::rtl::OUString() );
            out_it->second->endElement( ou_parcel );
            out_it->second->endDocument();
            xSource.set( out_it->second, UNO_QUERY );
            Reference<io::XOutputStream> xOS = xSource->getOutputStream();
            xOS->closeOutput();

        }
    }
    // *** TODO - other exception handling IO etc.
    catch ( RuntimeException re )
    {
        OSL_TRACE( "caught com::sun::star::uno::RuntimeException in ScriptStorage::save" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::save RuntimeException: " ).concat( re.Message ),
            Reference<XInterface> () );
    }
}

//*************************************************************************
void
ScriptStorage::writeMetadataHeader( Reference <xml::sax::XExtendedDocumentHandler> & xHandler )
{
    xHandler->startDocument();
    OUString aDocTypeStr( RTL_CONSTASCII_USTRINGPARAM(
                              "<!DOCTYPE dlg:window PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\""
                              " \"parcel.dtd\">" ) );
    xHandler->unknown( aDocTypeStr );
    xHandler->ignorableWhitespace( OUString() );
}

//*************************************************************************
//Returns a sequence of XScriptInfo interfaces
//to give access to info on scripts
//Eh, method name is not initiutive, maybe it
//should be getScriptInfoInterfaces? (Need to change IDL)
Sequence < Reference< storage::XScriptInfo > >
ScriptStorage::getScriptInfoService( const OUString & name )
throw ( RuntimeException )
{
    ::osl::Guard< osl::Mutex > aGuard( m_mutex );

    Sequence< Reference< storage::XScriptInfo > > sr_xScriptInfo;

    //Get iterator over the hash_map
    ScriptInfo_hash::const_iterator h_iter = mh_implementations.find( name );

    if ( h_iter == mh_implementations.end() )
    {
        OSL_TRACE("ScriptStorage::getScriptInfoService: EMPTY STORAGE");
        return sr_xScriptInfo;
    }
    sr_xScriptInfo.realloc( h_iter->second.size() );

    try
    {
        //Get array of XScriptInfos
        //        Reference< storage::XScriptInfo >* pScriptInfo = sr_xScriptInfo.getArray();

        Impls_vec::const_iterator end_iter = h_iter->second.end();
        sal_Int32 count = 0;
        for ( Impls_vec::const_iterator it = h_iter->second.begin(); it != end_iter; ++it )
        {
            Any a( makeAny( *it ) );
            Reference< XInterface > xInterface =
                m_xMgr->createInstanceWithArgumentsAndContext(
                    OUString::createFromAscii(
                        "drafts.com.sun.star.script.framework.storage.ScriptInfo" ),
                    Sequence< Any >( &a, 1 ), m_xContext );
            validateXRef( xInterface,
                          "ScriptStorage::getScriptInfoService: cannot get drafts.com.sun.star.script.framework.storage.ScriptInfo" );
            //            pScriptInfo[ count ] = Reference< storage::XScriptInfo >(xInterface,
            //                                   UNO_QUERY_THROW );
            sr_xScriptInfo[ count ] = Reference< storage::XScriptInfo >( xInterface,
                                      UNO_QUERY_THROW );
            count++;
        }
    }
    catch ( RuntimeException re )
    {
        OSL_TRACE( "caught com::sun::star::uno::RuntimeException in ScriptStorage::getScriptInfoService" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::getScriptInfoService RuntimeException: " ).concat( re.Message ),
            Reference<XInterface> () );
    }
    return sr_xScriptInfo;
}

//XNamingAccess
/**
Reference<XInterface> ScriptStorage::getView(
    const ::rtl::OUString& viewName )
    throw (storage::NoSuchView,
    RuntimeException)
{
}
*/

//*************************************************************************
Sequence< Reference< scripturi::XScriptURI > >
ScriptStorage::getImplementations( const Reference <
                                   scripturi::XScriptURI > & queryURI )
throw ( lang::IllegalArgumentException,
        RuntimeException )
{

    Sequence< Reference< scripturi::XScriptURI > > results;
    ScriptInfo_hash::iterator h_it =
        mh_implementations.find( queryURI->getLogicalName() );

    if ( h_it == mh_implementations.end() )
    {
        OSL_TRACE("ScriptStorage::getImplementations: EMPTY STORAGE");
        return results;
    }
    results.realloc( h_it->second.size() );

    //find the implementations for the given logical name
    try
    {

        ::osl::Guard< osl::Mutex > aGuard( m_mutex );

        Impls_vec::const_iterator it_impls = h_it->second.begin();
        Impls_vec::const_iterator it_impls_end = h_it->second.end();
        for ( sal_Int32 count = 0; it_impls != it_impls_end ; ++it_impls )
        {
            Sequence<Any> aArgs( 2 );
            aArgs[ 0 ] <<= *it_impls;
            aArgs[ 1 ] <<= m_scriptStorageID;

            Reference< XInterface > xInterface =
                m_xMgr->createInstanceWithArgumentsAndContext(
                    OUString::createFromAscii(
                        "drafts.com.sun.star.script.framework.scripturi.ScriptURI" ), aArgs,
                    m_xContext );
            validateXRef( xInterface,
                          "ScriptStorage::getImplementations: cannot get drafts.com.sun.star.script.framework.storage.ScriptURI" );
            Reference<scripturi::XScriptURI> uri( xInterface, UNO_QUERY_THROW );

            results[ count++ ] = uri;
        }

    }
    catch ( RuntimeException re )
    {
        throw RuntimeException(
            OUSTR( "ScriptStorage::getImplementations RuntimeException: " ).concat( re.Message ),
            Reference<XInterface> () );
    }
    catch ( Exception e )
    {
        throw RuntimeException( OUSTR( "ScriptStorage::getImplementations Exception: " ).concat( e.Message ),
                                Reference<XInterface> () );
    }
    return results;
}

//*************************************************************************
/**
    copies a parcel to a temporary location

    @params parcelURI
    the location of the parcel (file URI) to be copied

    @return
    <type>::rtl::OUString</type> the new location of the parcel (file URI)
*/
OUString SAL_CALL
ScriptStorage::prepareForInvocation( const OUString& parcelURI )
throw ( RuntimeException )
{
    try
    {
        if ( ( m_xSimpleFileAccess->exists( parcelURI ) != sal_True ) ||
                ( m_xSimpleFileAccess->isFolder( parcelURI ) != sal_True ) )
        {
            throw RuntimeException(
                OUSTR( "Parcel URI is not valid" ),
                Reference<XInterface> () );
        }

        OUString dest;
        if ( ::osl::FileBase::E_None != ::osl::FileBase::getTempDirURL( dest ) )
        {
            throw RuntimeException(
                OUSTR( "getTempDirURL failed" ),
                Reference<XInterface> () );
        }

        // Add time value to tmp dir name to prvent name clashes
        TimeValue timeVal;
        OUString sTime;
        if ( sal_False != osl_getSystemTime( &timeVal ) )
        {
            sTime = sTime.valueOf( static_cast<sal_Int32>( timeVal.Seconds ) );
        }
        OSL_TRACE( ::rtl::OUStringToOString(
                       sTime.concat( OUString::createFromAscii( "  Time to add to parcel name." ) ), RTL_TEXTENCODING_ASCII_US ).pData->buffer );

        sal_Int32 idx = parcelURI.lastIndexOf( '/' );
        sal_Int32 len = parcelURI.getLength();
        if ( idx == ( len - 1 ) )
        {
            // deal with the trailing separator
            idx = parcelURI.lastIndexOf( '/', len - 2 );
            OUString parcel_base = parcelURI.copy( idx, len - idx - 1 );
            dest = dest.concat( parcel_base );
        }
        else
        {
            dest = dest.concat( parcelURI.copy( idx ) );
        }
        dest = dest.concat( OUString::valueOf( ( sal_Int32 ) m_scriptStorageID ) );

        dest = dest.concat( sTime );

#ifdef _DEBUG

        fprintf( stderr, "dest is: %s\n",
                 rtl::OUStringToOString( dest, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
#endif

        copyFolder( parcelURI, dest );

        return dest;
    }
    catch ( RuntimeException & e )
    {
        throw RuntimeException(
            OUSTR( "ScriptStorage::prepareForInvocation RuntimeException: " ).concat( e.Message ),
            Reference<XInterface> () );
    }
    catch ( Exception & e )
    {
        throw RuntimeException(
            OUSTR( "ScriptStorage::prepareForInvocation Exception: " ).concat( e.Message ),
            Reference<XInterface> () );
    }
#ifdef _DEBUG
    catch ( ... )
    {
        throw RuntimeException(
            OUSTR( "ScriptStorage::prepareForInvocation UnknownException: " ),
            Reference<XInterface> () );
    }
#endif
}

//*************************************************************************
/**
   This function copies the contents of the source folder into the
   destination folder. If the destination folder does not exist, it
   is created. If the destination folder exists, it is deleted and then
   created. All URIs supported by the relevant XSimpleFileAccess
   implementation are supported.

    @params src
        the source folder (file URI)

    @params dest
    the destination folder (file URI)
*/
void
ScriptStorage::copyFolder( const OUString &src, const OUString &dest )
throw ( RuntimeException )
{
    try
    {
        /* A mutex guard is needed to not write later into
         * a folder that has been deleted by an incoming thread.
         * Note that this calls into the SimpleFileAccess service
         * with a locked mutex, but the implementation of this method
         * will probably change soon.
         */
        ::osl::Guard< osl::Mutex > aGuard( m_mutex );

        if ( m_xSimpleFileAccess->isFolder( dest ) == sal_True )
        {
            m_xSimpleFileAccess->kill( dest );
        }
        m_xSimpleFileAccess->createFolder( dest );

        Sequence <OUString> seq = m_xSimpleFileAccess->getFolderContents(
                                      src, sal_True );
        OUString new_dest;
        sal_Int32 idx;
        sal_Int32 len = seq.getLength();
        for ( sal_Int32 i = 0; i < len; i++ )
        {
            new_dest = dest;
            idx = seq[ i ].lastIndexOf( '/' );
            new_dest = new_dest.concat( seq[ i ].copy( idx ) );

            if ( m_xSimpleFileAccess->isFolder( seq[ i ] ) == sal_True )
            {
                copyFolder( seq[ i ], new_dest );
            }
            else
            {
                m_xSimpleFileAccess->copy( seq[ i ], new_dest );
            }
        }
    }
    catch ( ucb::CommandAbortedException & e )
    {
        OUString temp = OUSTR(
                            "ScriptStorage::copyFolder CommandAbortedException: " );
        throw RuntimeException( temp.concat( e.Message ),
                                Reference<XInterface> () );
    }
    catch ( RuntimeException & e )
    {
        OUString temp = OUSTR( "ScriptStorage::copyFolder RuntimeException: " );
        throw RuntimeException( temp.concat( e.Message ),
                                Reference<XInterface> () );
    }
    catch ( Exception & e )
    {
        OUString temp = OUSTR( "ScriptStorage::copyFolder Exception: " );
        throw RuntimeException( temp.concat( e.Message ),
                                Reference<XInterface> () );
    }
#ifdef _DEBUG
    catch ( ... )
    {
        throw RuntimeException( OUSTR(
                                    "ScriptStorage::copyFolder UnknownException: " ),
                                Reference<XInterface> () );
    }
#endif
}


//*************************************************************************
OUString SAL_CALL ScriptStorage::getImplementationName( )
throw( RuntimeException )
{
    return ss_implName;
}

//*************************************************************************
sal_Bool SAL_CALL ScriptStorage::supportsService( const OUString& serviceName )
throw( RuntimeException )
{
    OUString const * pNames = ss_serviceNames.getConstArray();
    for ( sal_Int32 nPos = ss_serviceNames.getLength(); nPos--; )
    {
        if ( serviceName.equals( pNames[ nPos ] ) )
        {
            return sal_True;
        }
    }
    return sal_False;
}

//*************************************************************************
Sequence<OUString> SAL_CALL ScriptStorage::getSupportedServiceNames( )
throw( RuntimeException )
{
    return ss_serviceNames;
}
//*************************************************************************
Reference<XInterface> SAL_CALL ss_create(
    const Reference< XComponentContext > & xCompC )
{
    return ( cppu::OWeakObject * ) new ScriptStorage( xCompC );
}

//*************************************************************************
Sequence<OUString> ss_getSupportedServiceNames( )
SAL_THROW( () )
{
    return ss_serviceNames;
}

//*************************************************************************
OUString ss_getImplementationName( )
SAL_THROW( () )
{
    return ss_implName;
}
}
