/*************************************************************************
 *
 *  $RCSfile: ZipPackage.cxx,v $
 *
 *  $Revision: 1.38 $
 *
 *  last change: $Author: mtg $ $Date: 2001-04-19 14:16:31 $
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
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/
#ifndef _ZIP_PACKAGE_HXX
#include <ZipPackage.hxx>
#endif
#ifndef _ZIP_PACKAGE_SINK_HXX
#include <ZipPackageSink.hxx>
#endif
#ifndef _ZIP_PACKAGE_STREAM_HXX
#include <ZipPackageStream.hxx>
#endif
#ifndef _ZIP_PACKAGE_FOLDER_HXX
#include <ZipPackageFolder.hxx>
#endif
#ifndef _ZIP_OUTPUT_STREAM_HXX
#include <ZipOutputStream.hxx>
#endif
#ifndef _ZIP_FILE_HXX
#include <ZipFile.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIPCONSTANTS_HPP_
#include <com/sun/star/packages/ZipConstants.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_MANIFEST_XMANIFESTREADER_HPP_
#include <com/sun/star/packages/manifest/XManifestReader.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_MANIFEST_XMANIFESTWRITER_HPP_
#include <com/sun/star/packages/manifest/XManifestWriter.hpp>
#endif

using namespace rtl;
using namespace std;
using namespace com::sun::star::io;
using namespace com::sun::star::registry;
using namespace com::sun::star::util;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::packages;
using namespace com::sun::star::packages::manifest;
using namespace com::sun::star::packages::ZipConstants;

ZipPackage::ZipPackage (const Reference < XMultiServiceFactory > &xNewFactory)
: pContent(NULL)
, pZipFile(NULL)
, pRootFolder(NULL)
, xContentStream (NULL)
, xContentSeek (NULL)
, xRootFolder (NULL)
, xZipFile (NULL)
, xFactory(xNewFactory)
{
    pRootFolder = new ZipPackageFolder();
    xRootFolder = Reference < XNameContainer > (pRootFolder );
}

ZipPackage::~ZipPackage( void )
{
    if (pContent)
        delete pContent;
    // As all folders and streams contain references to their parents,
    // we must remove these references so that they will be deleted when
    // the hash_map of the root folder is cleared, releasing all subfolders
    // and substreams which in turn release theirs, etc. When xRootFolder is
    // released when this destructor completes, the folder tree should be
    // deleted fully (and automagically).

    pRootFolder->releaseUpwardRef();
}

void ZipPackage::getZipFileContents()
{
    Reference< XEnumeration > xEnum = pZipFile->entries();
    Reference< XNameContainer > xCurrent;
    ZipPackageStream *pPkgStream;
    ZipPackageFolder *pPkgFolder;
    ZipEntry aEntry;
    Any aAny;
    while (xEnum->hasMoreElements())
    {
        xCurrent  = xRootFolder;
        sal_Int32 nOldIndex =0,nIndex = 0;
        aAny = xEnum->nextElement();
        aAny >>= aEntry;
        OUString &rName = aEntry.sName;

        if (rName.lastIndexOf('/') == rName.getLength()-1)
        {
            while ((nIndex = rName.indexOf('/', nOldIndex)) != -1)
            {
                OUString sTemp = rName.copy (nOldIndex, nIndex - nOldIndex);
                if (nIndex == nOldIndex)
                    break;
                if (!xCurrent->hasByName(sTemp))
                {
                    pPkgFolder = new ZipPackageFolder();
                    pPkgFolder->setName(sTemp);
                    try
                    {
                        pPkgFolder->setParent( Reference < XInterface > (xCurrent, UNO_QUERY) );
                    }
                    catch ( NoSupportException& )
                    {
                        VOS_ENSURE(0, "setParent threw an exception: attempted to set Parent to non-existing interface!");
                    }
                    xCurrent = Reference < XNameContainer > (pPkgFolder);
                }
                else
                {
                    Reference < XUnoTunnel> xRef;
                    aAny = xCurrent->getByName(sTemp);
                    aAny >>= xRef;
                    xCurrent = Reference < XNameContainer > (xRef, UNO_QUERY);
                }

                nOldIndex = nIndex+1;
            }
        }
        else
        {
            while ((nIndex = rName.indexOf('/', nOldIndex)) != -1)
            {
                OUString sTemp = rName.copy (nOldIndex, nIndex - nOldIndex);
                if (nIndex == nOldIndex)
                    break;
                if (!xCurrent->hasByName(sTemp))
                {
                    pPkgFolder = new ZipPackageFolder();
                    pPkgFolder->setName(sTemp);
                    try
                    {
                        pPkgFolder->setParent( Reference < XInterface >(xCurrent, UNO_QUERY));
                    }
                    catch ( NoSupportException& )
                    {
                        VOS_ENSURE( 0, "setParent threw an exception: attempted to set Parent to non-existing interface!");
                    }
                    xCurrent = Reference < XNameContainer > (pPkgFolder);
                }
                else
                {
                    Reference < XUnoTunnel> xRef;
                    aAny = xCurrent->getByName(sTemp);
                    aAny >>= xRef;
                    xCurrent = Reference < XNameContainer > (xRef, UNO_QUERY);
                }
                nOldIndex = nIndex+1;
            }
            OUString sStreamName = rName.copy( nOldIndex, rName.getLength() - nOldIndex);
            pPkgStream = new ZipPackageStream( pZipFile );
            pPkgStream->bPackageMember = sal_True;
            pPkgStream->setZipEntry( aEntry );
            pPkgStream->setName( sStreamName );
            try
            {
                pPkgStream->setParent( Reference < XInterface > (xCurrent, UNO_QUERY));
            }
            catch ( NoSupportException& )
            {
                VOS_ENSURE( 0, "setParent threw an exception: attempted to set Parent to non-existing interface!");
            }
        }
    }
    const OUString sManifest (RTL_CONSTASCII_USTRINGPARAM( "META-INF/manifest.xml") );
    if (hasByHierarchicalName( sManifest ) )
    {
        Reference < XUnoTunnel > xTunnel;
        aAny = getByHierarchicalName( sManifest );
        aAny >>= xTunnel;
        Reference < XActiveDataSink > xSink (xTunnel, UNO_QUERY);
        if (xSink.is())
        {
            OUString sManifestReader ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.manifest.ManifestReader" ) );
            Reference < XManifestReader > xReader (xFactory->createInstance( sManifestReader ), UNO_QUERY );
            if ( xReader.is() )
            {
                const OUString sFullPath ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );
                const OUString sMediaType ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) );
                Sequence < Sequence < PropertyValue > > aManifestSequence = xReader->readManifestSequence ( xSink->getInputStream() );
                sal_Int32 nLength = aManifestSequence.getLength();
                const Sequence < PropertyValue > *pSequence = aManifestSequence.getConstArray();


                for (sal_Int32 i = 0; i < nLength ; i++, pSequence++)
                {
                    OUString sPath;
                    Any aValueAny;
                    const PropertyValue *pValue = pSequence->getConstArray();
                    for (sal_Int32 j = 0, nNum = pSequence->getLength(); j < nNum; j++ )
                    {
                        if (pValue[j].Name.equals( sFullPath ) )
                            pValue[j].Value >>= sPath;
                        else if (pValue[j].Name.equals( sMediaType ) )
                            aValueAny = pValue[j].Value;
                    }
                    if (sPath.getLength() && aValueAny.getValueTypeClass() == TypeClass_STRING)
                    {
                        if ( hasByHierarchicalName ( sPath ) )
                        {
                            Any aAny = getByHierarchicalName( sPath );
                            Reference < XUnoTunnel > xTunnel;
                            aAny >>= xTunnel;
                            Reference < XPropertySet > xProps ( xTunnel, UNO_QUERY );
                            xProps->setPropertyValue ( sMediaType, aValueAny );
                        }
                    }
                }
            }
        }
    }
}
// XInitialization
void SAL_CALL ZipPackage::initialize( const Sequence< Any >& aArguments )
        throw(Exception, RuntimeException)
{
    OUString sURL;
    aArguments[0] >>= sURL;
    pContent = new ::ucb::Content(sURL, Reference < com::sun::star::ucb::XCommandEnvironment >() );
    Reference < XActiveDataSink > xSink = new ZipPackageSink;
    try
    {
        if (pContent->openStream ( xSink) )
            xContentStream = xSink->getInputStream();
        xContentSeek = Reference < XSeekable > (xContentStream, UNO_QUERY);
        try
        {
            pZipFile    = new ZipFile(xContentStream, sal_True);
            xZipFile    = Reference < XZipFile > ( pZipFile );
            getZipFileContents();
        }
        catch (ZipException&)// rException)
        {
            // clean up the memory, throw an assertion, and tell the UCB about the error
            delete pZipFile; pZipFile = NULL;
            VOS_ENSURE( 0, "ZipException thrown - bad Zip File"); //rException.Message);
            throw;
        }
    }
    catch (::com::sun::star::ucb::CommandAbortedException&)
    {
        // File doesn't exist, we'll create it at commitChanges time
    }
}

Any SAL_CALL ZipPackage::getByHierarchicalName( const OUString& aName )
        throw(NoSuchElementException, RuntimeException)
{
    OUString sTemp, sRoot( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );
    sal_Int32 nOldIndex =0, nIndex;
    Any aAny;
    Reference < XNameContainer > xCurrent  (xRootFolder);
    Reference < XNameContainer > xPrevious  (NULL);

    if (aName[nOldIndex] == '/')
        nOldIndex++;

    if (aName == sRoot)
        aAny <<= Reference < XUnoTunnel > (pRootFolder);
    else if (aName.lastIndexOf('/') == (nIndex = aName.getLength()-1))
    {
        if ( aRecent.count(aName) && (nOldIndex = aName.lastIndexOf('/', nIndex)) != -1)
        {
            sTemp = aName.copy(++nOldIndex, nIndex-nOldIndex);
            if (aRecent[aName]->hasByName(sTemp) )
                return aRecent[aName]->getByName(sTemp);
            else
                aRecent.erase(aName);
        }
        nOldIndex=0;
        while ((nIndex = aName.indexOf('/', nOldIndex)) != -1)
        {
            sTemp = aName.copy (nOldIndex, nIndex - nOldIndex);
            Reference < XUnoTunnel > xRef;

            if (nIndex == nOldIndex)
                break;
            if (xCurrent->hasByName(sTemp))
            {
                aAny = xCurrent->getByName(sTemp);
                aAny >>= xRef;
                xCurrent = Reference < XNameContainer > (xRef, UNO_QUERY);
            }
            else
                throw NoSuchElementException();
            nOldIndex = nIndex+1;
        }
        aRecent[aName] = xPrevious;
    }
    else
    {
        if ( aRecent.count(aName) && (nOldIndex = aName.lastIndexOf('/', nIndex)) != -1)
        {
            sTemp = aName.copy(++nOldIndex, nIndex-nOldIndex);
            if (aRecent[aName]->hasByName(sTemp) )
                return aRecent[aName]->getByName(sTemp);
            else
                aRecent.erase(aName);
        }
        nOldIndex=0;
        while ((nIndex = aName.indexOf('/', nOldIndex)) != -1)
        {
            sTemp = aName.copy (nOldIndex, nIndex - nOldIndex);
            Reference < XUnoTunnel > xChildRef;

            if (nIndex == nOldIndex)
                break;
            if (xCurrent->hasByName(sTemp))
            {
                aAny = xCurrent->getByName(sTemp);
                aAny >>= xChildRef;
                xCurrent = Reference < XNameContainer > (xChildRef, UNO_QUERY);
            }
            else
                throw NoSuchElementException();

            nOldIndex = nIndex+1;
        }
        OUString sStreamName = aName.copy( nOldIndex, aName.getLength() - nOldIndex);
        if (xCurrent->hasByName(sStreamName))
        {
            aRecent[aName] = xCurrent;
            return xCurrent->getByName(sStreamName);
        }
        else
            throw NoSuchElementException();
    }
    return aAny;
}

sal_Bool SAL_CALL ZipPackage::hasByHierarchicalName( const OUString& aName )
        throw(RuntimeException)
{
    OUString sTemp, sRoot( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );
    sal_Int32 nOldIndex = 0, nIndex;
    Any aAny;
    Reference < XNameContainer > xCurrent  (xRootFolder);
    Reference < XNameContainer > xPrevious  (NULL);

    if (aName[nOldIndex] == '/')
        nOldIndex++;

    if (aName == sRoot)
        return sal_True;
    else if (aName.lastIndexOf('/') == (nIndex = aName.getLength()-1))
    {
        if ( aRecent.count(aName) && (nOldIndex = aName.lastIndexOf('/', nIndex)) != -1)
        {
            sTemp = aName.copy(++nOldIndex, nIndex - nOldIndex);
            if (aRecent[aName]->hasByName(sTemp) )
                return sal_True;
            else
                aRecent.erase(aName);
        }
        nOldIndex=0;
        while ((nIndex = aName.indexOf('/', nOldIndex)) != -1)
        {
            sTemp = aName.copy (nOldIndex, nIndex - nOldIndex);
            Reference < XUnoTunnel > xRef;
            if (nIndex == nOldIndex)
                break;
            if (xCurrent->hasByName(sTemp))
            {
                aAny = xCurrent->getByName(sTemp);
                aAny >>= xRef;
                xPrevious = xCurrent;
                xCurrent = Reference < XNameContainer > (xRef, UNO_QUERY);
            }
            else
                return sal_False;
            nOldIndex = nIndex+1;
        }
        aRecent[aName] = xPrevious;
        return sal_True;
    }
    else
    {
        if ( aRecent.count(aName) && (nOldIndex = aName.lastIndexOf('/', nIndex)) != -1)
        {
            sTemp = aName.copy(++nOldIndex, nIndex - nOldIndex);
            if (aRecent[aName]->hasByName(sTemp) )
                return sal_True;
            else
                aRecent.erase(aName);
        }
        nOldIndex=0;
        while ((nIndex = aName.indexOf('/', nOldIndex)) != -1)
        {
            sTemp = aName.copy (nOldIndex, nIndex - nOldIndex);
            Reference < XUnoTunnel > xChildRef;
            if (nIndex == nOldIndex)
                break;
            if (xCurrent->hasByName(sTemp))
            {
                aAny = xCurrent->getByName(sTemp);
                aAny >>= xChildRef;
                xCurrent = Reference < XNameContainer > (xChildRef, UNO_QUERY);
            }
            else
                return sal_False;
            nOldIndex = nIndex+1;
        }
        OUString sStreamName = aName.copy( nOldIndex, aName.getLength() - nOldIndex);

        if (xCurrent->hasByName(sStreamName))
        {
            aRecent[aName] = xCurrent;
            return sal_True;
        }
        return sal_False;
    }
}

// XSingleServiceFactory
Reference< XInterface > SAL_CALL ZipPackage::createInstance(  )
        throw(Exception, RuntimeException)
{
    Reference < XInterface > xRef = *(new ZipPackageStream ( pZipFile ));
    return xRef;
}
Reference< XInterface > SAL_CALL ZipPackage::createInstanceWithArguments( const Sequence< Any >& aArguments )
        throw(Exception, RuntimeException)
{
    sal_Bool bArg;
    Reference < XInterface > xRef;
    aArguments[0] >>= bArg;
    if (bArg)
        xRef = *new ZipPackageFolder ( );
    else
        xRef = *new ZipPackageStream ( pZipFile );

    return xRef;
}

// XChangesBatch
void SAL_CALL ZipPackage::commitChanges(  )
        throw(WrappedTargetException, RuntimeException)
{
    std::vector < Sequence < PropertyValue > > aManList;

    // Set up output buffer. ZipPackageBuffer implements both
    // XInputStream and XOutputStream as the UCB requires an XInputStream
    // and the ZipOutputStream writes to an XOutputStream

    ZipPackageBuffer *pZipBuffer = new ZipPackageBuffer( 65535 );
    Reference < XOutputStream > xOutStream (pZipBuffer);
    ZipOutputStream *pZipOut = new ZipOutputStream( xOutStream, 65535 );

    // Make a reference to the manifest output stream so it persists
    // until the call to ZipOutputStream->finish()

    Reference < XOutputStream > xManOutStream;
    ZipPackageStream *pManifestStream = NULL;

    Reference < XZipOutputStream > xZipOut (pZipOut);
    pZipOut->setMethod(DEFLATED);
    pZipOut->setLevel(DEFAULT_COMPRESSION);

    // Remove the old META-INF directory as this will be re-generated below.
    // Pass save-contents a vector which will be used to store the entries which
    // are placed inside the Manifest et al. Note: saveContents is called
    // recursively.

    const OUString sMeta ( RTL_CONSTASCII_USTRINGPARAM ( "META-INF" ) );
    const OUString sMediaType ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) );
    const OUString sFullPath ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );
    if (xRootFolder->hasByName( sMeta ) )
        xRootFolder->removeByName( sMeta );

    Sequence < PropertyValue > aPropSeq ( 2 );
    aPropSeq [0].Name = sMediaType;
    aPropSeq [0].Value <<= pRootFolder->getPropertyValue( sMediaType ) ;
    aPropSeq [1].Name = sFullPath;
    aPropSeq [1].Value <<= OUString ( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );

    aManList.push_back( aPropSeq );
    pRootFolder->saveContents(OUString(), aManList, *pZipOut);

    OUString sManifestWriter( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.manifest.ManifestWriter" ) );
    Reference < XManifestWriter > xWriter (xFactory->createInstance( sManifestWriter ), UNO_QUERY );
    if ( xWriter.is() )
    {
        ZipEntry * pEntry = new ZipEntry;
        ZipPackageBuffer *pBuffer = new ZipPackageBuffer(65535);
        xManOutStream = Reference < XOutputStream > (*pBuffer, UNO_QUERY);

        pEntry->sName = OUString( RTL_CONSTASCII_USTRINGPARAM ( "META-INF/manifest.xml") );
        pEntry->nMethod = STORED;

        Sequence < Sequence < PropertyValue > > aManifestSequence ( aManList.size());
        Sequence < PropertyValue > * pSequence = aManifestSequence.getArray();
        for (vector < Sequence < PropertyValue > >::const_iterator aIter = aManList.begin(), aEnd = aManList.end();
             aIter != aEnd;
             aIter++, pSequence++)
            *pSequence= (*aIter);
        xWriter->writeManifestSequence ( xManOutStream,  aManifestSequence );

        pEntry->nSize = pEntry->nCompressedSize = static_cast < sal_Int32 > (pBuffer->getPosition());
        pBuffer->aBuffer.realloc(pEntry->nSize);
        CRC32 aCRC;
        aCRC.update(pBuffer->aBuffer);
        pEntry->nCrc = aCRC.getValue();

        try
        {
            pZipOut->putNextEntry(*pEntry);
            pZipOut->write(pBuffer->aBuffer, 0, pEntry->nSize);
            pZipOut->closeEntry();
        }
        catch (::com::sun::star::io::IOException & )
        {
            VOS_ENSURE( 0, "Error adding META-INF/manifest.xml to the ZipOutputStream" );
        }
    }
    try
    {
        pZipOut->finish();
    }
    catch (::com::sun::star::io::IOException & )
    {
        VOS_ENSURE( 0, "Error writing ZIP file to disk" );
    }

    xContentStream = Reference < XInputStream > (pZipBuffer);
    xContentSeek   = Reference < XSeekable > (pZipBuffer);

    pZipFile->setInputStream ( xContentStream );
    pZipBuffer->seek(0);
    try
    {
        pContent->writeStream( Reference < XInputStream > (pZipBuffer), sal_True );
    }
    catch (::com::sun::star::ucb::CommandAbortedException&)
    {
        VOS_ENSURE( 0, "Unable to write Zip File to disk!");
    }
}

sal_Bool SAL_CALL ZipPackage::hasPendingChanges(  )
        throw(RuntimeException)
{
    return sal_False;
}
Sequence< ElementChange > SAL_CALL ZipPackage::getPendingChanges(  )
        throw(RuntimeException)
{
    return Sequence < ElementChange > ( NULL, 0 );
}

/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
Reference < XInterface >SAL_CALL ZipPackage_createInstance(
    const Reference< XMultiServiceFactory > & xMgr )
{
    return Reference< XInterface >( *new ZipPackage(xMgr) );
}

OUString ZipPackage::getImplementationName()
{
#if SUPD>625
    return OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.comp.ZipPackage" ) );
#else
    return OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.package.Package" ) );
#endif
}

Sequence< OUString > ZipPackage::getSupportedServiceNames()
{
    Sequence< OUString > aNames(1);
#if SUPD>625
    aNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.comp.ZipPackage" ) );
#else
    aNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.package.Package" ) );
#endif
    return aNames;
}
sal_Bool SAL_CALL ZipPackage::supportsService( OUString const & rServiceName )
{
    return rServiceName == getSupportedServiceNames()[0];
}
Reference < XSingleServiceFactory > ZipPackage::createServiceFactory( Reference < XMultiServiceFactory > const & rServiceFactory )
{
    return cppu::createSingleFactory (rServiceFactory,
                                           getImplementationName(),
                                           ZipPackage_createInstance,
                                           getSupportedServiceNames());
}

//XInterface
Any SAL_CALL ZipPackage::queryInterface( const Type& rType )
    throw(RuntimeException)
{
        return ::cppu::queryInterface ( rType                                       ,
                                        // OWeakObject interfaces
                                        reinterpret_cast< XInterface*       > ( this )  ,
                                        static_cast< XWeak*         > ( this )  ,
                                        // my own interfaces
                                        static_cast< XInitialization*       > ( this )  ,
                                        static_cast< XSingleServiceFactory*     > ( this )  ,
                                        static_cast< XUnoTunnel*        > ( this )  ,
                                        static_cast< XHierarchicalNameAccess*       > ( this )  ,
                                        static_cast< XChangesBatch* > ( this ) );
}

void SAL_CALL ZipPackage::acquire(  )
    throw()
{
    OWeakObject::acquire();
}
void SAL_CALL ZipPackage::release(  )
    throw()
{
    OWeakObject::release();
}

// XUnoTunnel
Sequence< sal_Int8 > ZipPackage::getUnoTunnelImplementationId( void )
    throw (RuntimeException)
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

sal_Int64 SAL_CALL ZipPackage::getSomething( const Sequence< sal_Int8 >& aIdentifier )
    throw(RuntimeException)
{
    if (aIdentifier.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  aIdentifier.getConstArray(), 16 ) )
        return reinterpret_cast < sal_Int64 > ( this );
    return 0;
}

