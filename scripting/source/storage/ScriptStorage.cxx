/*************************************************************************
*
*  $RCSfile: ScriptStorage.cxx,v $
*
*  $Revision: 1.16 $
*  last change: $Author: npower $ $Date: 2003-02-12 16:21:43 $
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

#include <util/util.hxx>

#include "ScriptData.hxx"
#include "ScriptInfo.hxx"
#include "ScriptStorage.hxx"
#include "ScriptElement.hxx"
#include "ScriptMetadataImporter.hxx"
#include "ScriptURI.hxx"

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;

namespace scripting_impl
{

const sal_Char* const SERVICE_NAME =
    "drafts.com.sun.star.script.framework.storage.ScriptStorage";
const sal_Char* const IMPL_NAME =
    "drafts.com.sun.star.script.framework.storage.ScriptStorage";

const sal_Char * const SCRIPT_DIR = "/Scripts";
const sal_Char * const SCRIPT_PARCEL = "/parcel-descriptor.xml";
const sal_Char * const SCRIPT_PARCEL_NAME_ONLY = "parcel-descriptor";

static OUString ss_implName = OUString::createFromAscii( IMPL_NAME );
static OUString ss_serviceName = OUString::createFromAscii( SERVICE_NAME );
static Sequence< OUString > ss_serviceNames =
    Sequence< OUString >( &ss_serviceName, 1 );

const sal_uInt16 NUMBER_STORAGE_INITIALIZE_ARGS = 3;

extern ::rtl_StandardModuleCount s_moduleCount;



//*************************************************************************
ScriptStorage::ScriptStorage( const Reference <
                              XComponentContext > & xContext )
        : m_xContext( xContext ), m_bInitialised( false )
{
    OSL_TRACE( "< ScriptStorage ctor called >\n" );

    validateXRef( m_xContext,
        "ScriptStorage::ScriptStorage : cannot get component context" );

    m_xMgr = m_xContext->getServiceManager();
    validateXRef( m_xMgr,
        "ScriptStorage::ScriptStorage : cannot get service manager" );

    s_moduleCount.modCnt.acquire( &s_moduleCount.modCnt );
}

//*************************************************************************
ScriptStorage::~ScriptStorage() SAL_THROW( () )
{
    OSL_TRACE( "< ScriptStorage dtor called >\n" );
    s_moduleCount.modCnt.release( &s_moduleCount.modCnt );
}

//*************************************************************************
void
ScriptStorage::initialize( const Sequence <Any> & args )
throw ( RuntimeException, Exception )
{
    OSL_TRACE( "Entering ScriptStorage::initialize\n" );

    // Should not be renitialised
    if ( m_bInitialised )
    {
        throw RuntimeException(
            OUSTR( "ScriptStorage::initalize already initialized" ),
            Reference<XInterface> () );
    }

    {   // Protect member variable writes
        ::osl::Guard< osl::Mutex > aGuard( m_mutex );

        // Check args
        if ( args.getLength() != NUMBER_STORAGE_INITIALIZE_ARGS )
        {
            OSL_TRACE( "ScriptStorage::initialize: got wrong number of args\n" );
            throw RuntimeException(
                OUSTR( "Invalid number of arguments provided!" ),
                Reference< XInterface >() );
        }

        if ( sal_False == ( args[ 0 ] >>= m_xSimpleFileAccess ) )
        {
            throw RuntimeException(
                OUSTR( "Invalid XSimpleFileAccess argument provided!" ),
                Reference< XInterface >() );
        }

        if ( sal_False == ( args[ 1 ] >>= m_scriptStorageID ) )
        {
            throw RuntimeException(
                OUSTR( "Invalid ScriptStorage ID argument provided!" ),
                Reference< XInterface >() );

        }
        if ( sal_False == ( args[ 2 ] >>= m_stringUri ) )
        {
            throw RuntimeException(
                OUSTR( "Invalid String Uri argument provided!" ),
                Reference< XInterface >() );
        }
    } // End - Protect member variable writes

    OSL_TRACE(  "uri: %s\n", ::rtl::OUStringToOString(
        m_stringUri, RTL_TEXTENCODING_ASCII_US ).pData->buffer );

    try
    {
        create();
    }
    catch ( RuntimeException & re )
    {
        OSL_TRACE( "caught com::sun::star::uno::RuntimeException in ScriptStorage::initialize" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::initalize RuntimeException: " ).concat( re.Message ),
            Reference< XInterface > () );
    }
    catch ( Exception & ue )
    {
        OSL_TRACE( "caught com::sun::star::uno::Exception in ScriptStorage::initialize" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::initalize Exception: " ).concat( ue.Message ),
            Reference< XInterface > () );
    }
#ifdef _DEBUG
    catch ( ... )
    {
        OSL_TRACE( "caught unknown Exception in ScriptStorage::initialize" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::initalize unknown exception: " ),
            Reference< XInterface > () );
    }
#endif

    OSL_TRACE( "Parsed the XML\n" );

    m_bInitialised = true;
}

void
ScriptStorage::create()
throw ( RuntimeException, Exception )
{
    ::osl::Guard< osl::Mutex > aGuard( m_mutex );
    try
    {
        // clear existing hashmap - rebuilding from scratch to avoid having
        // to search for deleted elements on refresh
        mh_implementations.clear();

        OUString xStringUri(m_stringUri);

        ScriptMetadataImporter* SMI = new ScriptMetadataImporter( m_xContext );
        Reference< xml::sax::XExtendedDocumentHandler > xSMI( SMI );

        validateXRef( xSMI, "ScriptStorage::create: failed to obtain valid XExtendedDocumentHandler" );

        xStringUri = xStringUri.concat( ::rtl::OUString::createFromAscii(
            SCRIPT_DIR ) );

       // No Scripts directory - just return
       if ( ! m_xSimpleFileAccess->isFolder( xStringUri ) )
       {
            OSL_TRACE( "ScriptStorage::initialize: no Scripts dir for this storage - install problem\n" );
           return;
       }

        // get the list of language folders under the Scripts directory
        Sequence< ::rtl::OUString > languageDirs =
            m_xSimpleFileAccess->getFolderContents( xStringUri, true );

        Reference< io::XInputStream > xInput;
        sal_Int32 languageDirsLength = languageDirs.getLength();
        for ( sal_Int32 i = 0; i < languageDirsLength ; ++i )
        {
            OSL_TRACE(  "contains: %s\n", ::rtl::OUStringToOString(
                languageDirs[ i ], RTL_TEXTENCODING_ASCII_US ).pData->buffer );

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
                OSL_TRACE(  "contains: %s\n",
                    ::rtl::OUStringToOString( parcelDirs[ j ],
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );

                OUString parcelFile = parcelDirs[ j ].concat(
                    ::rtl::OUString::createFromAscii( SCRIPT_PARCEL ) );

                // Do not have a valid parcel.xml
                if ( !m_xSimpleFileAccess->exists( parcelFile ) ||
                        m_xSimpleFileAccess->isFolder( parcelFile ) )
                {
                    continue;
                }
                OSL_TRACE(  "parcel file: %s\n",
                    ::rtl::OUStringToOString( parcelFile,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );

                xInput = m_xSimpleFileAccess->openFileRead( parcelFile );
                // Failed to get input stream
                if ( !xInput.is() )
                {
                    continue;
                }

                OSL_TRACE( "Parse the metadata \n" );
                Datas_vec vScriptDatas;
                try
                {
                    SMI->parseMetaData( xInput, parcelDirs[ j ], vScriptDatas );
                }
                catch ( xml::sax::SAXException & saxe )
                {
                    if ( xInput.is() )
                    {
                        xInput->closeInput();
                    }
                    OSL_TRACE(
                        "caught com::sun::star::xml::sax::SAXException in ScriptStorage::create %s",
                        ::rtl::OUStringToOString( saxe.Message,
                        RTL_TEXTENCODING_ASCII_US ).pData->buffer  );

                    continue;
                }
                catch ( io::IOException & ioe )
                {
                    if ( xInput.is() )
                    {
                        xInput->closeInput();
                    }
                    OSL_TRACE(
                        "caught com::sun::star::io::IOException in ScriptStorage::create" );
                    continue;
                }
                xInput->closeInput();

                updateMaps( vScriptDatas );
            }
        }
    }
    catch ( io::IOException & ioe )
    {
        //From ScriptMetadata Importer
        OSL_TRACE( "caught com::sun::star::io::IOException in ScriptStorage::create" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::create IOException: " ).concat( ioe.Message ),
            Reference< XInterface > () );

    }
    catch ( ucb::CommandAbortedException & cae )
    {
        OSL_TRACE( "caught com::sun::star::ucb::CommandAbortedException in ScriptStorage::create" );
        throw RuntimeException(
            OUSTR(
                "ScriptStorage::create CommandAbortedException: " ).concat( cae.Message ),
            Reference< XInterface > () );
    }
    catch ( RuntimeException & re )
    {
        OSL_TRACE( "caught com::sun::star::uno::RuntimeException in ScriptStorage::create" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::create RuntimeException: " ).concat( re.Message ),
            Reference< XInterface > () );
    }
    catch ( Exception & ue )
    {
        OSL_TRACE( "caught com::sun::star::uno::Exception in ScriptStorage::create" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::create Exception: " ).concat( ue.Message ),
            Reference< XInterface > () );
    }
#ifdef _DEBUG
    catch ( ... )
    {
        OSL_TRACE( "caught unknown Exception in ScriptStorage::create" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::initalize unknown exception: " ),
            Reference< XInterface > () );
    }
#endif

    OSL_TRACE( "Parsed the XML\n" );

    m_bInitialised = true;
}


//*************************************************************************
// private method for updating hashmaps
void
ScriptStorage::updateMaps( const Datas_vec & vScriptDatas )
{

    Datas_vec::const_iterator it_end = vScriptDatas.end();
    // step through the vector of ScripImplInfos returned from parse
    for ( Datas_vec::const_iterator it = vScriptDatas.begin() ; it != it_end; ++it )
    {
        //find the Datas_vec for this logical name
        ScriptInfo_hash::iterator h_it = mh_implementations.find( it->logicalname );

        if ( h_it == mh_implementations.end() )
        {
            //if it's null, need to create a new Datas_vec
            OSL_TRACE(
                     "updateMaps: new logical name: %s\n", rtl::OUStringToOString(
                         it->logicalname, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            OSL_TRACE(  "language name: %s\n",
                     rtl::OUStringToOString(
                         it->functionname, RTL_TEXTENCODING_ASCII_US ).pData->buffer );

            Datas_vec v;
            v.push_back( *it );
            mh_implementations[ it->logicalname ] = v;
        }
        else
        {
            OSL_TRACE(  "updateMaps: existing logical name: %s\n",
                rtl::OUStringToOString( it->logicalname,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            OSL_TRACE(  "                    language name: %s\n",
                rtl::OUStringToOString( it->functionname,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );

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
    Reference< io::XOutputStream > xOS;

    // xScriptInvocation = Reference<XScriptInvocation>(xx, UNO_QUERY_THROW);
    Reference< xml::sax::XExtendedDocumentHandler > xHandler;

    OUString parcel_suffix = OUString::createFromAscii( SCRIPT_PARCEL );
    OUString ou_parcel = OUString(
        RTL_CONSTASCII_USTRINGPARAM( SCRIPT_PARCEL_NAME_ONLY ) );

    try
    {
        ScriptInfo_hash::iterator it_end = mh_implementations.end();
        for ( ScriptInfo_hash::iterator it = mh_implementations.begin() ; it != it_end; ++it )
        {
            ::rtl::OUString logName = it->first;
            Datas_vec::iterator it_datas_end = it->second.end();
            for ( Datas_vec::iterator it_datas = it->second.begin();
                    it_datas != it_datas_end ; ++it_datas )
            {
                ScriptOutput_hash::const_iterator it_parcels =
                    mh_parcels.find( it_datas->parcelURI );
                if ( it_parcels == mh_parcels.end() )
                {
                    //create new outputstream
                    OUString parcel_xml_path = it_datas->parcelURI.concat(
                        parcel_suffix );
                    m_xSimpleFileAccess->kill( parcel_xml_path );
                    xOS = m_xSimpleFileAccess->openFileWrite( parcel_xml_path );

                    OSL_TRACE(  "saving: %s\n", rtl::OUStringToOString(
                        it_datas->parcelURI.concat( OUString::createFromAscii(
                        "/parcel.xml" ) ),
                        RTL_TEXTENCODING_ASCII_US ).pData->buffer );

                    Reference< XInterface > xInterface =
                        m_xMgr->createInstanceWithContext(
                        OUString::createFromAscii( "com.sun.star.xml.sax.Writer" ),
                        m_xContext );
                    validateXRef( xInterface, "ScriptStorage::save: cannot get sax.Writer" );
                    xHandler = Reference<xml::sax::XExtendedDocumentHandler>(
                        xInterface, UNO_QUERY_THROW );
                    xSource = Reference< io::XActiveDataSource >(
                        xHandler, UNO_QUERY_THROW );
                    xSource->setOutputStream( xOS );

                    writeMetadataHeader( xHandler );

                    xHandler->startElement( ou_parcel,
                        Reference< xml::sax::XAttributeList >() );

                    mh_parcels[ it_datas->parcelURI ] = xHandler;
                }
                else
                {
                    xHandler = it_parcels->second;
                }

                ScriptElement* pSE = new ScriptElement( *it_datas );
                // this is to get pSE released correctly
                Reference < xml::sax::XAttributeList > xal( pSE );
                pSE->dump( xHandler );
            }
        }

        ScriptOutput_hash::const_iterator out_it_end = mh_parcels.end();

        for ( ScriptOutput_hash::const_iterator out_it = mh_parcels.begin();
                out_it != out_it_end; ++out_it )
        {
            out_it->second->ignorableWhitespace( ::rtl::OUString() );
            out_it->second->endElement( ou_parcel );
            out_it->second->endDocument();
            xSource.set( out_it->second, UNO_QUERY );
            Reference< io::XOutputStream > xOS = xSource->getOutputStream();
            xOS->closeOutput();

        }
    }
    // *** TODO - other exception handling IO etc.
    catch ( RuntimeException & re )
    {
        OSL_TRACE( "caught com::sun::star::uno::RuntimeException in ScriptStorage::save" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::save RuntimeException: " ).concat(
            re.Message ),
            Reference< XInterface > () );
    }
}

//*************************************************************************
void
ScriptStorage::refresh()
throw (RuntimeException)
{
    OSL_TRACE("** => ScriptStorage: in refresh()\n");

    // guard against concurrent refreshes
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );

    try
    {
        create();

    }
    catch ( RuntimeException & re )
    {
        OSL_TRACE( "caught com::sun::star::uno::RuntimeException in ScriptStorage::refresh" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::refresh RuntimeException: " ).concat( re.Message ),
            Reference< XInterface > () );
    }
    catch ( Exception & ue )
    {
        OSL_TRACE( "caught com::sun::star::uno::Exception in ScriptStorage::refresh" );
        throw RuntimeException(
            OUSTR( "ScriptStorage::refresh Exception: " ).concat( ue.Message ),
            Reference< XInterface > () );
    }
}

//*************************************************************************
void
ScriptStorage::writeMetadataHeader(
    Reference <xml::sax::XExtendedDocumentHandler> & xHandler )
{
    xHandler->startDocument();
    OUString aDocTypeStr( RTL_CONSTASCII_USTRINGPARAM(
        "<!DOCTYPE dlg:window PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\""
                              " \"parcel.dtd\">" ) );
    xHandler->unknown( aDocTypeStr );
    xHandler->ignorableWhitespace( OUString() );
}


//*************************************************************************
Sequence< ::rtl::OUString >
ScriptStorage::getScriptLogicalNames()
throw ( lang::IllegalArgumentException,
        RuntimeException )
{
    Sequence< ::rtl::OUString  > results;
    ScriptInfo_hash::iterator h_it = mh_implementations.begin();
    ScriptInfo_hash::iterator h_itEnd =  mh_implementations.end();
    if ( h_it == h_itEnd  )
    {
        OSL_TRACE( "ScriptStorage::getImplementations: EMPTY STORAGE");
        return results;
    }
    results.realloc( mh_implementations.size() );

    //find the implementations for the given logical name
    try
    {

        ::osl::Guard< osl::Mutex > aGuard( m_mutex );

        for ( sal_Int32 count = 0; h_it != h_itEnd ; ++h_it )
        {
            ::rtl::OUString logicalName = h_it->first;
            OSL_TRACE( "Adding %s at index %d ", ::rtl::OUStringToOString(
                logicalName, RTL_TEXTENCODING_ASCII_US ).pData->buffer, count);
            results[ count++ ] = logicalName;
        }

    }
    catch ( RuntimeException & re )
    {
        throw RuntimeException(
            OUSTR( "ScriptStorage::getScriptLogicalNames RuntimeException: " ).concat( re.Message ),
            Reference< XInterface > () );
    }
    catch ( Exception & e )
    {
        throw RuntimeException( OUSTR(
            "ScriptStorage::getScriptLogicalNames Exception: " ).concat(
            e.Message ), Reference< XInterface > () );
    }
    return results;
}

//*************************************************************************
Sequence< Reference< storage::XScriptInfo > >
ScriptStorage::getImplementations( const ::rtl::OUString & queryURI )
throw ( lang::IllegalArgumentException,
        RuntimeException )
{
    ::osl::Guard< osl::Mutex > aGuard( m_mutex );

    Sequence< Reference< storage::XScriptInfo > > results;
    ScriptURI scriptURI( queryURI );
    OSL_TRACE( "getting impl for logical name: %s",
        ::rtl::OUStringToOString( scriptURI.getLogicalName(),
        RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    ScriptInfo_hash::iterator h_itEnd =  mh_implementations.end();
    ScriptInfo_hash::iterator h_it = mh_implementations.begin();
    if ( h_it == h_itEnd )
    {
        OSL_TRACE( "ScriptStorage::getImplementations: EMPTY STORAGE" );
        return results;
    }

    h_it = mh_implementations.find( scriptURI.getLogicalName() );

    if ( h_it == h_itEnd )
    {
        OSL_TRACE( "ScriptStorage::getImplementations: no impls found for %s",
            ::rtl::OUStringToOString( scriptURI.getLogicalName(),
            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        return results;
    }


    results.realloc( h_it->second.size() );

    //find the implementations for the given logical name
    Datas_vec::const_iterator it_datas = h_it->second.begin();
    Datas_vec::const_iterator it_datas_end = h_it->second.end();
    for ( sal_Int32 count = 0; it_datas != it_datas_end ; ++it_datas )
    {
        OSL_TRACE( "Adding to sequence of impls " );
        Reference< storage::XScriptInfo > xScriptInfo = new ScriptInfo (
            *it_datas, m_scriptStorageID );

        results[ count++ ] = xScriptInfo;
    }
    OSL_TRACE( "returning from ScriptStorage::getImplementations with %d entries",
    results.getLength() );
    return results;
}

//*************************************************************************
Sequence< Reference< storage::XScriptInfo > > SAL_CALL
ScriptStorage::getAllImplementations() throw ( RuntimeException )
{
    ::osl::Guard< osl::Mutex > aGuard( m_mutex );
    Sequence< Reference< storage::XScriptInfo > > results;
    ScriptInfo_hash::iterator h_itEnd =  mh_implementations.end();
    ScriptInfo_hash::iterator h_it = mh_implementations.begin();
    if ( h_it == h_itEnd )
    {
        OSL_TRACE( "ScriptStorage::getImplementations: EMPTY STORAGE" );
        return results;
    }


    //iterater through each logical name and gather each implementation
    //for that name
    for ( sal_Int32 count = 0; h_it !=  h_itEnd; ++h_it )
    {
        results.realloc( h_it->second.size() + count );
        OSL_TRACE( "Adding implementations for %s",
            ::rtl::OUStringToOString( h_it->first,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        Datas_vec::const_iterator it_datas = h_it->second.begin();
        Datas_vec::const_iterator it_datas_end = h_it->second.end();
        OSL_TRACE( "Adding %d to sequence of impls ", h_it->second.size() );
        for ( ; it_datas != it_datas_end ; ++it_datas )
        {
            Reference< storage::XScriptInfo > xScriptInfo = new ScriptInfo (
            *it_datas, m_scriptStorageID );

            results[ count++ ] = xScriptInfo;
        }
    }
    return results;

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
} // namespace scripting_impl
