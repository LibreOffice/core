/*************************************************************************
 *
 *  $RCSfile: ZipPackage.cxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: mtg $ $Date: 2001-03-08 12:22:40 $
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
#include "ZipPackage.hxx"
#endif

using namespace rtl;
using namespace com::sun::star::io;
using namespace com::sun::star::registry;
using namespace com::sun::star::util;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::package;
using namespace com::sun::star::lang;
using namespace com::sun::star::package::ZipConstants;

ZipPackage::ZipPackage (Reference < XInputStream > &xNewInput,
                        const Reference < XMultiServiceFactory > &xNewFactory)
: pContent(NULL)
, pZipFile(NULL)
, xZipFile (NULL)
, pRootFolder(NULL)
, xRootFolder (NULL)
, xContentStream (xNewInput)
, xContentSeek (xNewInput, UNO_QUERY)
, xFactory(xNewFactory)
{
    try
    {
        pZipFile    = new ZipFile(xContentStream, sal_True);
    }
    catch (ZipException&)// rException)
    {
        VOS_ENSURE( 0, "ZipException thrown...bad ZipFile"); // rException.Message );
    }

    xZipFile    = Reference < XZipFile > ( pZipFile );
    pRootFolder = new ZipPackageFolder( );
    xRootFolder = Reference < XNameContainer > ( pRootFolder );
    getZipFileContents();
}

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

void ZipPackage::destroyFolderTree( Reference < XUnoTunnel > xFolder )
{

    ZipPackageFolder *pCurrentFolder = reinterpret_cast < ZipPackageFolder* > (xFolder->getSomething(ZipPackageFolder::getUnoTunnelImplementationId()));
    Reference < XEnumeration > xEnum = pCurrentFolder->createEnumeration();
    while (xEnum->hasMoreElements())
    {
        Reference < XUnoTunnel > xTunnel;
        Any aAny = xEnum->nextElement();

        ZipPackageFolder *pFolder = NULL;
        ZipPackageStream *pStream = NULL;
        sal_Bool bIsFolder = sal_False;

        aAny >>= xTunnel;
        try
        {
            pFolder = reinterpret_cast < ZipPackageFolder* > (xTunnel->getSomething(ZipPackageFolder::getUnoTunnelImplementationId()));
            bIsFolder = sal_True;
        }
        catch (RuntimeException&)
        {
            pStream = reinterpret_cast < ZipPackageStream* > (xTunnel->getSomething(ZipPackageStream::getUnoTunnelImplementationId()));
            bIsFolder = sal_False;
        }
        if (bIsFolder)
            destroyFolderTree(xTunnel);
        else
            pStream->release();
    }
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
            /*
             * disabled this funky but useless functionality on 8/03/2001 due to problems
             * with copying streams - mtg
             *
             * It may be of some use to future generations :)
            if (isZipFile(aEntry))
            {
                Reference < XInputStream > xContentStream = pZipFile->getInputStream(aEntry);
                ZipPackage *pInZip = new ZipPackage (xContentStream, xFactory );
                pPkgFolder = pInZip->getRootFolder();
                pPkgFolder->setName(sStreamName);
                pPkgFolder->pPackage = pInZip;
                pPkgFolder->xPackage = Reference < XSingleServiceFactory > (pInZip);
                try
                {
                    pPkgFolder->setParent( Reference < XInterface >(xCurrent, UNO_QUERY));
                }
                catch ( NoSupportException& )
                {
                    VOS_ENSURE( 0, "setParent threw an exception: attempted to set Parent to non-existing interface!");
                }
            }
            else
            */
            {
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
            try
            {
                ManifestReader aReader(*this, xSink->getInputStream(), xFactory);
                aReader.Read();
            }
            catch ( com::sun::star::xml::sax::SAXException & )
            {
                VOS_ENSURE( 0,  "SAX threw an exception when reading XML Manifest!");
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
ZipPackageBuffer & SAL_CALL ZipPackage::writeToBuffer(  )
        throw(WrappedTargetException, RuntimeException)
{
    std::vector < ManifestEntry * > aManList;
    Any aAny;

    // Set up output buffer. ZipPackageBuffer implements both
    // XInputStream and XOutputStream as the UCB requires an XInputStream
    // and the ZipOutputStream writes to an XOutputStream

    ZipPackageBuffer *pZipBuffer = new ZipPackageBuffer( 65535 );
    Reference < XOutputStream > xOutStream (pZipBuffer);
    ZipOutputStream *pZipOut = new ZipOutputStream( xOutStream, 65535 );
    Reference < XZipOutputStream > xZipOut (pZipOut);
    pZipOut->setMethod(DEFLATED);
    pZipOut->setLevel(DEFAULT_COMPRESSION);

    sal_Bool bAddMetaFolder = sal_False;

    // Remove the old META-INF directory as this will be re-generated below.
    // Pass save-contents a vector which will be used to store the entries which
    // are placed inside the Manifest et al. Note: saveContents is called
    // recursively.

    const OUString sMeta ( RTL_CONSTASCII_USTRINGPARAM ( "META-INF" ) );
    if (xRootFolder->hasByName( sMeta ) )
        xRootFolder->removeByName( sMeta );

    ManifestEntry *pMan = new ManifestEntry;
    ZipPackageFolder::copyZipEntry(pMan->aEntry, pRootFolder->aEntry);
    pMan->aEntry.sName = OUString( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );
    try
    {
        Any aAny = pRootFolder->getPropertyValue(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType") ) );
        aAny >>= pMan->sMediaType;
    }
    catch (::com::sun::star::beans::UnknownPropertyException & )
    {
        VOS_ENSURE( 0, "MediaType is an unknown property!!" );
    }

    aManList.push_back(pMan);
    pRootFolder->saveContents(OUString(), aManList, *pZipOut);

    ZipPackageFolder *pMetaInfFolder = new ZipPackageFolder();
    ZipPackageStream *pManifestStream = new ZipPackageStream( pZipFile );
    aAny <<= Reference < XUnoTunnel > (pMetaInfFolder);
    xRootFolder->insertByName(sMeta, aAny);

    ZipPackageBuffer *pBuffer = new ZipPackageBuffer(65535);
    Reference < XOutputStream > xManOutStream (pBuffer);

    pManifestStream->aEntry.sName = OUString( RTL_CONSTASCII_USTRINGPARAM ( "META-INF/manifest.xml") );
    pManifestStream->aEntry.nMethod = STORED;
    pManifestStream->bPackageMember = sal_True;

    try
    {
        ManifestWriter aWriter ( xManOutStream, xFactory, aManList);
        aWriter.Write();
    }
    catch ( com::sun::star::xml::sax::SAXException & )
    {
        VOS_ENSURE( 0,  "SAX threw an exception when writing XML Manifest!");
    }
    pManifestStream->setInputStream(Reference < XInputStream > (xManOutStream, UNO_QUERY));

    pManifestStream->aEntry.nSize = pManifestStream->aEntry.nCompressedSize = static_cast < sal_Int32 > (pBuffer->getPosition());
    pBuffer->aBuffer.realloc(pManifestStream->aEntry.nSize);
    CRC32 aCRC;
    aCRC.update(pBuffer->aBuffer);
    pManifestStream->aEntry.nCrc = aCRC.getValue();

    try
    {
        pZipOut->putNextEntry(pManifestStream->aEntry);
        pZipOut->write(pBuffer->aBuffer, 0, pManifestStream->aEntry.nSize);
        pZipOut->closeEntry();
        pZipOut->finish();
    }
    catch (::com::sun::star::io::IOException & )
    {
        VOS_ENSURE( 0, "Error writing ZipOutputStream" );
    }

    aAny <<= Reference < XUnoTunnel > (pManifestStream);
    pMetaInfFolder->insertByName(OUString( RTL_CONSTASCII_USTRINGPARAM ( "manifest.xml") ) , aAny);
    pManifestStream->aEntry.nOffset *=-1;

    xContentStream = Reference < XInputStream > (pZipBuffer);
    xContentSeek   = Reference < XSeekable > (pZipBuffer);

    // If we have a valid pZipFile pointer, then we opened a stream
    // earlier and read from it
    // Otherwise we are writing a new ZipFile
    pZipBuffer->seek(0);
    /*
    try
    {
        pZipFile    = new ZipFile(xContentStream, sal_False);
        xZipFile    = Reference < XZipFile > ( pZipFile );
        pRootFolder->updateReferences ( pZipFile );
    }
    catch (ZipException&)// rException)
    {
        VOS_ENSURE( 0, "ZipException thrown - bad ZipFile " );//rException.Message);
    }
    */

    pZipFile->setInputStream ( xContentStream );
    pZipFile->updateFromManList( aManList );
    for (sal_uInt32 i=0 ; i < aManList.size(); i++)
    {
        aManList[i]->aEntry.sName = aManList[i]->sShortName;
        delete aManList[i];
    }

    pZipBuffer->seek(0);
        // If we are writing the zip file for the first time, pZipBuffer becomes
        // the xContentStream. If so, it will have a refcount of 1 before the following
        // call.
        //
        // Otherwise, it will have a refcount of 0, and will be deleted after the
        // following call
        //
        // (at least...that's the plan!) mtg 6/12/00
    return *pZipBuffer;
    // Likewise, pBuffer and pZipOut will be deleted automagically due to the xZipOut and xOutStream
}
// XChangesBatch
void SAL_CALL ZipPackage::commitChanges(  )
        throw(WrappedTargetException, RuntimeException)
{
    try
    {
        pContent->writeStream(Reference < XInputStream > (&writeToBuffer()), sal_True);
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

sal_Bool ZipPackage::isZipFile(com::sun::star::package::ZipEntry &rEntry)
{
    if (rEntry.nMethod == STORED)
    {
        if (rEntry.nSize < 98) // smallest possible zip file size
            return sal_False;
    }
    return (pZipFile->getHeader(rEntry) == LOCSIG);
}

/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
Reference < XInterface >SAL_CALL ZipPackage_create(
    const Reference< XMultiServiceFactory > & xMgr )
{
    return Reference< XInterface >( *new ZipPackage(xMgr) );
    //return Reference < XInterface > (NULL);
}

extern "C" void SAL_CALL component_getImplementationEnvironment(
                const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
Sequence< OUString > ZipPackage_getSupportedServiceNames()
{
    Sequence< OUString > seqNames(1);
    seqNames.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.package.Package" ) );
    return seqNames;
}

/**
 * This function creates an implementation section in the registry and another subkey
 * for each supported service.
 * @param pServiceManager generic uno interface providing a service manager
 * @param pRegistryKey generic uno interface providing registry key to write
 */
extern "C" sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, void* pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
            reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                 OUString( RTL_CONSTASCII_USTRINGPARAM ( "/com.sun.star.package.Package/UNO/SERVICES" ) ) ) );
            const Sequence< OUString > & rSNL = ZipPackage_getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );
           return sal_True;
        }
        catch ( InvalidRegistryException& )
        {
            VOS_ENSURE( 0, "InvalidRegistryException detected\n");
            return sal_False;
        }

    }
    return sal_False;
}


/**
 * This function is called to get service factories for an implementation.
 * @param pImplName name of implementation
 * @param pServiceManager generic uno interface providing a service manager to instantiate components
 * @param pRegistryKey registry data key to read and write component persistent data
 * @return a component factory (generic uno interface)
 */
extern "C" void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;
    // which implementation is demanded?
    if (pServiceManager && !rtl_str_compare( pImplName, "com.sun.star.package.Package" ))
    {
        OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.package.Package") );
        Reference< XSingleServiceFactory > xFactory(
           cppu::createSingleFactory( // helper function from cppuhelper lib
           reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
           OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.package.Package") ),
           ZipPackage_create, ZipPackage_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
           xFactory->acquire();
           pRet = xFactory.get();
        }
    }
    return pRet;
}
//XInterface
Any SAL_CALL ZipPackage::queryInterface( const Type& rType )
    throw(RuntimeException)
{
    // Ask for my own supported interfaces ...
    Any aReturn ( ::cppu::queryInterface    (   rType                                       ,
                                                static_cast< XInitialization*               > ( this )  ,
                                                static_cast< XSingleServiceFactory*     > ( this )  ,
                                                static_cast< XUnoTunnel*                    > ( this )  ,
                                                static_cast< XHierarchicalNameAccess*> ( this ) ,
                                                static_cast< XChangesBatch*             > ( this ) ) );

    // If searched interface supported by this class ...
    if ( aReturn.hasValue () == sal_True )
    {
        // ... return this information.
        return aReturn ;
    }
    else
    {
        // Else; ... ask baseclass for interfaces!
        return OWeakObject::queryInterface ( rType ) ;
    }
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

