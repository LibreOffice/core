/*************************************************************************
 *
 *  $RCSfile: xmlfilterjar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 16:33:42 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

#ifndef _COM_SUN_STAR_IO_XACTIVEDATACONTROL_HPP_
#include <com/sun/star/io/XActiveDataControl.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONFIGMANAGER_HPP_
#include <com/sun/star/frame/XConfigManager.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif


#ifndef _COMPHELPER_STREAM_OSLFILEWRAPPER_HXX_
#include <comphelper/oslfile2streamwrap.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#include <rtl/uri.hxx>

#include "xmlfilterjar.hxx"
#include "xmlfilterdialogstrings.hrc"
#include "xmlfiltersettingsdialog.hxx"
#include "typedetectionexport.hxx"
#include "typedetectionimport.hxx"

using namespace rtl;
using namespace osl;
using namespace comphelper;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::io;

XMLFilterJarHelper::XMLFilterJarHelper( Reference< XMultiServiceFactory >& xMSF )
: mxMSF( xMSF ),
    sVndSunStarPackage( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.Package:" ) ),
    sXSLTPath( RTL_CONSTASCII_USTRINGPARAM( "$(user)/xslt/" ) ),
    sDTDPath( RTL_CONSTASCII_USTRINGPARAM( "$(user)/dtd/" ) ),
    sTemplatePath( RTL_CONSTASCII_USTRINGPARAM( "$(user)/template/") ),
    sSpecialConfigManager( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.config.SpecialConfigManager" ) ),
    sPump( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.Pump" ) ),
    sProgPath( RTL_CONSTASCII_USTRINGPARAM( "$(prog)/" ) )
{
    try
    {
        Reference< XConfigManager > xCfgMgr( xMSF->createInstance(OUString::createFromAscii("com.sun.star.config.SpecialConfigManager")), UNO_QUERY );
        if( xCfgMgr.is() )
        {
            sProgPath = xCfgMgr->substituteVariables( sProgPath );
            sXSLTPath = xCfgMgr->substituteVariables( sXSLTPath );
            sDTDPath = xCfgMgr->substituteVariables( sDTDPath );
            sTemplatePath = xCfgMgr->substituteVariables( sTemplatePath );
        }
    }
    catch(Exception&)
    {
    }
}

static OUString encodeZipUri( const OUString& rURI )
{
    return Uri::encode( rURI, rtl_UriCharClassUric, rtl_UriEncodeCheckEscapes, RTL_TEXTENCODING_UTF8 );
}

static Reference< XInterface > addFolder( Reference< XInterface >& xRootFolder, Reference< XSingleServiceFactory >& xFactory, const OUString& rName ) throw( Exception )
{
    Sequence< Any > aArgs(1);
    aArgs[0] <<= (sal_Bool)sal_True;

    Reference< XInterface > xFolder( xFactory->createInstanceWithArguments(aArgs) );
    Reference< XNamed > xNamed( xFolder, UNO_QUERY );
    Reference< XChild > xChild( xFolder, UNO_QUERY );

    if( xNamed.is() && xChild.is() )
    {
        OUString aName( encodeZipUri( rName ) );
        xNamed->setName( aName );
        xChild->setParent( xRootFolder );
    }

    return xFolder;
}

static void _addFile( Reference< XInterface >& xRootFolder, Reference< XSingleServiceFactory >& xFactory, Reference< XInputStream >& xInput, OUString aName ) throw( Exception )
{
    Reference< XActiveDataSink > xSink( xFactory->createInstance(), UNO_QUERY );
    Reference< XNamed > xNamed( xSink, UNO_QUERY );
    Reference< XChild > xChild( xSink, UNO_QUERY );

    aName = encodeZipUri( aName );

    if( xSink.is() && xNamed.is() && xChild.is() )
    {
        xChild->setParent( xRootFolder );
        xNamed->setName( aName );
        xSink->setInputStream( xInput );
    }
}

static void addFile( Reference< XInterface > xRootFolder, Reference< XSingleServiceFactory > xFactory, const OUString& rSourceFile, const OUString& rName ) throw( Exception )
{
    Reference< XInputStream > xInput(  new utl::OSeekableInputStreamWrapper( new SvFileStream(rSourceFile, STREAM_READ ), true ) );
    _addFile( xRootFolder, xFactory, xInput, rName );
}

void XMLFilterJarHelper::addFile( Reference< XInterface > xRootFolder, Reference< XSingleServiceFactory > xFactory, const OUString& rSourceFile ) throw( Exception )
{
    if( rSourceFile.getLength() &&
        (rSourceFile.compareToAscii( RTL_CONSTASCII_STRINGPARAM("http:") ) != 0) &&
        (rSourceFile.compareToAscii( RTL_CONSTASCII_STRINGPARAM("shttp:") ) != 0) &&
        (rSourceFile.compareToAscii( RTL_CONSTASCII_STRINGPARAM("jar:") ) != 0) &&
        (rSourceFile.compareToAscii( RTL_CONSTASCII_STRINGPARAM("ftp:") ) != 0))
    {
        OUString aFileURL( rSourceFile );

        if( !aFileURL.matchIgnoreAsciiCase( OUString( RTL_CONSTASCII_USTRINGPARAM("file://") ) ) )
        {
            aFileURL = URIHelper::SmartRel2Abs( String(sProgPath), String( aFileURL ), Link(), false );
        }

        INetURLObject aURL( aFileURL );
        OUString aName( aURL.getName() );

        SvFileStream* pStream = new SvFileStream(aFileURL, STREAM_READ );
        Reference< XInputStream > xInput(  new utl::OSeekableInputStreamWrapper( pStream, true ) );
        _addFile( xRootFolder, xFactory, xInput, aName );
    }
}

bool XMLFilterJarHelper::savePackage( const OUString& rPackageURL, const XMLFilterVector& rFilters )
{
    try
    {
        osl::File::remove( rPackageURL );

        // create the package jar file

        Sequence< Any > aArguments( 1 );
        aArguments[ 0 ] <<= rPackageURL;

        Reference< XHierarchicalNameAccess > xIfc(
            mxMSF->createInstanceWithArguments(
                rtl::OUString::createFromAscii(
                                "com.sun.star.packages.comp.ZipPackage" ),
                aArguments ), UNO_QUERY );

        if( xIfc.is() )
        {
            Reference< XSingleServiceFactory > xFactory( xIfc, UNO_QUERY );

            // get root zip folder
            Reference< XInterface > xRootFolder;
            OUString szRootFolder( RTL_CONSTASCII_USTRINGPARAM("/") );
            xIfc->getByHierarchicalName( szRootFolder ) >>= xRootFolder;

            // export filters files
            XMLFilterVector::const_iterator aIter( rFilters.begin() );
            while( aIter != rFilters.end() )
            {
                const filter_info_impl* pFilter = (*aIter);

                Reference< XInterface > xFilterRoot( addFolder( xRootFolder, xFactory, pFilter->maFilterName ) );

                if( xFilterRoot.is() )
                {
                    if( pFilter->maDTD.getLength() )
                        addFile( xFilterRoot, xFactory, pFilter->maDTD );

                    if( pFilter->maExportXSLT.getLength() )
                        addFile( xFilterRoot, xFactory, pFilter->maExportXSLT );

                    if( pFilter->maImportXSLT.getLength() )
                        addFile( xFilterRoot, xFactory, pFilter->maImportXSLT );

                    if( pFilter->maImportTemplate.getLength() )
                        addFile( xFilterRoot, xFactory, pFilter->maImportTemplate );
                }

                aIter++;
            }

            // create TypeDetection.xcu
            utl::TempFile aTempFile;
            aTempFile.EnableKillingFile();
            OUString aTempFileURL( aTempFile.GetURL() );

            {
                osl::File aOutputFile( aTempFileURL );
                osl::File::RC rc = aOutputFile.open( OpenFlag_Write );
                Reference< XOutputStream > xOS( new OSLOutputStreamWrapper( aOutputFile ) );

                TypeDetectionExporter aExporter( mxMSF );
                aExporter.doExport(xOS,rFilters);
            }

            Reference< XInputStream > XIS(  new utl::OSeekableInputStreamWrapper( new SvFileStream(aTempFileURL, STREAM_READ ), true ) );
            OUString szTypeDetection( RTL_CONSTASCII_USTRINGPARAM( "TypeDetection.xcu" ) );
            _addFile( xRootFolder, xFactory,  XIS, szTypeDetection );

            Reference< XChangesBatch > xBatch( xIfc, UNO_QUERY );
            if( xBatch.is() )
                xBatch->commitChanges();

            return true;
        }
    }
    catch( Exception& e )
    {
        DBG_ERROR( "XMLFilterJarHelper::savePackage exception catched!" );
    }

    osl::File::remove( rPackageURL );

    return false;
}



//

void XMLFilterJarHelper::openPackage( const OUString& rPackageURL, XMLFilterVector& rFilters )
{
    try
    {
        // create the package jar file

        Sequence< Any > aArguments( 1 );
        aArguments[ 0 ] <<= rPackageURL;

        Reference< XHierarchicalNameAccess > xIfc(
            mxMSF->createInstanceWithArguments(
                rtl::OUString::createFromAscii(
                                "com.sun.star.packages.comp.ZipPackage" ),
                aArguments ), UNO_QUERY );

        if( xIfc.is() )
        {
            Reference< XSingleServiceFactory > xFactory( xIfc, UNO_QUERY );

            // get root zip folder
            Reference< XInterface > xRootFolder;
            OUString szRootFolder( RTL_CONSTASCII_USTRINGPARAM("/") );
            xIfc->getByHierarchicalName( szRootFolder ) >>= xRootFolder;

            OUString szTypeDetection( RTL_CONSTASCII_USTRINGPARAM("TypeDetection.xcu") );
            if( xIfc->hasByHierarchicalName( szTypeDetection ) )
            {
                Reference< XActiveDataSink > xTypeDetection;
                xIfc->getByHierarchicalName( szTypeDetection ) >>= xTypeDetection;

                if( xTypeDetection.is() )
                {
                    Reference< XInputStream > xIS( xTypeDetection->getInputStream() );

                    XMLFilterVector aFilters;
                    TypeDetectionImporter::doImport( mxMSF, xIS, aFilters );

                    // copy all files used by the filters imported from the
                    // typedetection to office/user/xslt
                    XMLFilterVector::iterator aIter( aFilters.begin() );
                    while( aIter != aFilters.end() )
                    {
                        if( copyFiles( xIfc, (*aIter) ) )
                        {
                            rFilters.push_back( (*aIter) );
                        }
                        else
                        {
                            // failed to copy all files
                            delete (*aIter);
                        }
                        aIter++;
                    }
                }
            }
        }
    }
    catch( Exception& e )
    {
        DBG_ERROR( "XMLFilterJarHelper::savePackage exception catched!" );
    }
}

bool XMLFilterJarHelper::copyFiles( Reference< XHierarchicalNameAccess > xIfc, filter_info_impl* pFilter )
{
    bool bOk = copyFile( xIfc, pFilter->maDTD, sDTDPath );

    if( bOk )
        bOk = copyFile( xIfc, pFilter->maExportXSLT, sXSLTPath );

    if( bOk )
        bOk = copyFile( xIfc, pFilter->maImportXSLT, sXSLTPath );

    if( bOk )
        bOk = copyFile( xIfc, pFilter->maImportTemplate, sTemplatePath );

    return bOk;
}

bool XMLFilterJarHelper::copyFile( Reference< XHierarchicalNameAccess > xIfc, OUString& rURL, const OUString& rTargetURL )
{
    if( !rURL.matchIgnoreAsciiCase( sVndSunStarPackage ) )
        return true;

    try
    {
        OUString szPackagePath( encodeZipUri( rURL.copy( sVndSunStarPackage.getLength() ) ) );

        if( xIfc->hasByHierarchicalName( szPackagePath ) )
        {
            Reference< XActiveDataSink > xFileEntry;
            xIfc->getByHierarchicalName( szPackagePath ) >>= xFileEntry;

            if( xFileEntry.is() )
            {
                Reference< XInputStream > xIS( xFileEntry->getInputStream() );

                INetURLObject aBaseURL( rTargetURL );

                rURL = URIHelper::SmartRel2Abs( aBaseURL, szPackagePath, Link(), false );

                if( rURL.getLength() )
                {
                    // create output directory if needed
                    if( !createDirectory( rURL ) )
                        return false;

                    SvFileStream aOutputStream(rURL, STREAM_WRITE );
                    Reference< XOutputStream > xOS(  new utl::OOutputStreamWrapper( aOutputStream ) );

                    return copyStreams( xIS, xOS );
                }
            }
        }
    }
    catch( Exception& )
    {
        DBG_ERROR( "XMLFilterJarHelper::copyFile exception catched" );
    }
    return false;
}
