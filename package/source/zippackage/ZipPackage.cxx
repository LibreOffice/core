/*************************************************************************
 *
 *  $RCSfile: ZipPackage.cxx,v $
 *
 *  $Revision: 1.54 $
 *
 *  last change: $Author: mtg $ $Date: 2001-08-22 16:12:18 $
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
#ifndef _ZIP_ENUMERATION_HXX
#include <ZipEnumeration.hxx>
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
#ifndef _THREADED_BUFFER_HXX
#include <ThreadedBuffer.hxx>
#endif
#ifndef _PACKAGE_CONSTANTS_HXX_
#include <PackageConstants.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIP_ZIPCONSTANTS_HPP_
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_MANIFEST_XMANIFESTREADER_HPP_
#include <com/sun/star/packages/manifest/XManifestReader.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEFILEIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveFileIOException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEWRONGMEDIUMEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveWrongMediumException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_IOERRORCODE_HPP
#include <com/sun/star/ucb/IOErrorCode.hpp>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _INTERACTION_REQUEST_HXX_
#include <InteractionRequest.hxx>
#endif
#include <memory>

using namespace rtl;
using namespace ucb;
using namespace std;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::io;
using namespace com::sun::star::util;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::beans;
using namespace com::sun::star::packages;
using namespace com::sun::star::registry;
using namespace com::sun::star::container;
using namespace com::sun::star::registry;
using namespace com::sun::star::container;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::packages::manifest;
using namespace com::sun::star::packages::zip::ZipConstants;

struct SuffixGenerator
{
    sal_Char mpSuffix[30], *mpSuffixMid, *mpSuffixEnd;
    sal_uInt16 mnDigits;
    SuffixGenerator(): mnDigits (2), mpSuffixMid ( mpSuffix), mpSuffixEnd ( mpSuffix + 2)
    {
        memset ( mpSuffix, 0, 30 );
        mpSuffixMid[0] = 'a';
        mpSuffixMid[1] = 'a'; // first generated suffix will be .ab
    }
    void generateFileName( OUString &rFileName, const OUString &rPrefix );
    void generateFileName( OUString &rFileName, const OUString &rPrefix, sal_uInt16 nDiskNum);
};

void SuffixGenerator::generateFileName( OUString &rFileName, const OUString &rPrefix )
{
    // same file prefix, different extension
    OUStringBuffer aStringBuf;
    sal_Char *p;
    for ( p = mpSuffixEnd-1; *p == 'z'; p--)
        *p = 'a';
    ++(*p);
    if (*p == 'z' && p == mpSuffixMid )
    {
        ++mnDigits;
        ++mpSuffixMid;
        *mpSuffixEnd++ = 'a';
        *mpSuffixEnd++ = 'a';
    }
    aStringBuf.append ( rPrefix );
    aStringBuf.appendAscii ( mpSuffix );
    rFileName = aStringBuf.makeStringAndClear();
}
void SuffixGenerator::generateFileName( OUString &rFileName, const OUString &rPrefix, sal_uInt16 nDiskNum)
{
    // same file prefix, different extension
    OUStringBuffer aStringBuf;
    aStringBuf.append ( rPrefix );
    aStringBuf.appendAscii ( nDiskNum < 10 ? "00" : nDiskNum < 100 ? "0" : "" );
    aStringBuf.append ( static_cast < sal_Int32 > ( nDiskNum-1) );

    rFileName = aStringBuf.makeStringAndClear();
}

ZipPackage::ZipPackage (const Reference < XMultiServiceFactory > &xNewFactory)
: pContent(NULL)
, pZipFile(NULL)
, pRootFolder(NULL)
, xContentStream (NULL)
, xContentSeek (NULL)
, xRootFolder (NULL)
, xFactory(xNewFactory)
, bHasEncryptedEntries ( sal_False )
, bSpanned( sal_False )
, nSegmentSize ( 0 )
{
    pRootFolder = new ZipPackageFolder();
    xRootFolder = Reference < XNameContainer > (pRootFolder );
}

ZipPackage::~ZipPackage( void )
{
    delete pContent;
    delete pZipFile;
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
    auto_ptr < ZipEnumeration > pEnum ( pZipFile->entries() );
    Reference< XNameContainer > xCurrent;
    ZipPackageStream *pPkgStream;
    ZipPackageFolder *pPkgFolder;
    Any aAny;

    while (pEnum->hasMoreElements())
    {
        xCurrent  = xRootFolder;
        sal_Int32 nOldIndex = 0,nIndex = 0;
        const ZipEntry & rEntry = *pEnum->nextElement();
        const OUString &rName = rEntry.sName;

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
            pPkgStream = new ZipPackageStream( *this );
            pPkgStream->SetPackageMember( sal_True );
            pPkgStream->setZipEntry( rEntry );
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
                const OUString sPropFullPath ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );
                const OUString sPropMediaType ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) );
                const OUString sPropInitialisationVector ( RTL_CONSTASCII_USTRINGPARAM ( "InitialisationVector" ) );
                const OUString sPropSalt ( RTL_CONSTASCII_USTRINGPARAM ( "Salt" ) );
                const OUString sPropIterationCount ( RTL_CONSTASCII_USTRINGPARAM ( "IterationCount" ) );
                const OUString sPropSize ( RTL_CONSTASCII_USTRINGPARAM ( "Size" ) );

                Sequence < Sequence < PropertyValue > > aManifestSequence = xReader->readManifestSequence ( xSink->getInputStream() );
                sal_Int32 nLength = aManifestSequence.getLength();
                const Sequence < PropertyValue > *pSequence = aManifestSequence.getConstArray();
                ZipPackageStream *pStream = NULL;
                ZipPackageFolder *pFolder = NULL;

                for (sal_Int32 i = 0; i < nLength ; i++, pSequence++)
                {
                    OUString sPath, sMediaType;
                    const PropertyValue *pValue = pSequence->getConstArray();
                    const Any *pSalt = NULL, *pVector = NULL, *pCount = NULL, *pSize = NULL;
                    for (sal_Int32 j = 0, nNum = pSequence->getLength(); j < nNum; j++ )
                    {
                        if (pValue[j].Name.equals( sPropFullPath ) )
                            pValue[j].Value >>= sPath;
                        else if (pValue[j].Name.equals( sPropMediaType ) )
                            pValue[j].Value >>= sMediaType;
                        else if (pValue[j].Name.equals( sPropSalt ) )
                            pSalt = &(pValue[j].Value);
                        else if (pValue[j].Name.equals( sPropInitialisationVector ) )
                            pVector = &(pValue[j].Value);
                        else if (pValue[j].Name.equals( sPropIterationCount ) )
                            pCount = &(pValue[j].Value);
                        else if (pValue[j].Name.equals( sPropSize ) )
                            pSize = &(pValue[j].Value);
                    }
                    if (sPath.getLength() && hasByHierarchicalName ( sPath ) )
                    {
                        Any aAny = getByHierarchicalName( sPath );
                        Reference < XUnoTunnel > xTunnel;
                        aAny >>= xTunnel;
                        sal_Int64 nTest=0;
                        if ((nTest = xTunnel->getSomething(ZipPackageFolder::getUnoTunnelImplementationId())) != 0)
                        {
                            pFolder = reinterpret_cast < ZipPackageFolder* > ( nTest );
                            pFolder->SetMediaType ( sMediaType );
                        }
                        else
                        {
                            pStream = reinterpret_cast < ZipPackageStream* > ( xTunnel->getSomething(ZipPackageStream::getUnoTunnelImplementationId()));
                            pStream->SetMediaType ( sMediaType );

                            if (pSalt && pVector && pCount && pSize)
                            {
                                Sequence < sal_uInt8 > aSequence;
                                sal_Int32 nCount, nSize;

                                *pSalt >>= aSequence;
                                pStream->setSalt ( aSequence );

                                *pVector >>= aSequence;
                                pStream->setInitialisationVector ( aSequence );

                                *pCount >>= nCount;
                                pStream->setIterationCount ( nCount );

                                *pSize >>= nSize;
                                pStream->setSize ( nSize );

                                pStream->SetToBeEncrypted ( sal_True );
                                bHasEncryptedEntries = sal_True;
                            }
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
    if (! (aArguments[0] >>= sURL))
        throw com::sun::star::uno::Exception ( OUString::createFromAscii ( "Bad URL." ),
            static_cast < ::cppu::OWeakObject * > ( this ) );
    pContent = new Content(sURL, Reference < XCommandEnvironment >() );
    sal_Bool bBadZipFile = sal_False, bHaveZipFile = sal_True;

    Reference < XActiveDataSink > xSink = new ZipPackageSink;
    try
    {
        if (pContent->openStream ( xSink ) )
            xContentStream = xSink->getInputStream();
        if (xContentStream.is())
        {
            xContentSeek = Reference < XSeekable > (xContentStream, UNO_QUERY);
            Sequence < sal_Int8 > aSequence ( 4 );
            xContentStream->readBytes( aSequence, 4 );
            xContentSeek->seek ( 0 );
            const sal_Int8 *pSeq = aSequence.getConstArray();
            if (pSeq[0] == 'P' &&
                pSeq[1] == 'K' &&
                pSeq[2] == 7 &&
                pSeq[3] == 8 )
                bSpanned = sal_True;
       }
        else
            bHaveZipFile = sal_False;
    }
    catch (com::sun::star::uno::Exception&)
    {
        // Exception derived from uno::Exception thrown. This probably
        // means the file doesn't exist...we'll create it at
        // commitChanges time
        bHaveZipFile = sal_False;
    }
    if ( bHaveZipFile )
    {
        try
        {
            if ( bSpanned)
            {
                xContentStream = unSpanFile ( xContentStream );
                pZipFile = new ZipFile ( xContentStream, sURL );
            }
            else
                pZipFile = new ZipFile ( xContentStream, sal_True);
            getZipFileContents();
        }
        catch ( IOException & )
        {
            bBadZipFile = sal_True;
        }
        catch ( ZipException & )
        {
            bBadZipFile = sal_True;
        }
        if ( bBadZipFile )
        {
            // clean up the memory, and tell the UCB about the error
            delete pZipFile; pZipFile = NULL;
            delete pContent; pContent = NULL;
            throw com::sun::star::uno::Exception ( OUString::createFromAscii ( "Bad Zip File." ),
                static_cast < ::cppu::OWeakObject * > ( this ) );
        }
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
    Reference < XInterface > xRef = *(new ZipPackageStream ( *this ));
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
        xRef = *new ZipPackageStream ( *this );

    return xRef;
}

char * ImplGetChars( const OUString & rString )
{
    // Memory leak ? oh yeah! Who cares, this function lives until this feature works
    // and no longer
    sal_Int32 nLength = rString.getLength();
    const sal_Unicode *pString = rString.getStr();
    char * pChar = new char [nLength+1];
    for ( sal_Int16 i = 0; i < nLength; i++ )
    {
        pChar[i] = static_cast < char > (pString[i]);
    }
    pChar[nLength] = '\0';
    return pChar;
}
// XChangesBatch
void SAL_CALL ZipPackage::commitChanges(  )
        throw(WrappedTargetException, RuntimeException)
{
    ThreadedBuffer *pBuffer;
    Reference < XOutputStream > xOutBuffer = (pBuffer = new ThreadedBuffer ( n_ConstBufferSize, *this ));
    Reference < XInputStream > xInBuffer ( pBuffer );
    ZipOutputStream aZipOut ( xOutBuffer, nSegmentSize != 0);
    pBuffer->setZipOutputStream ( aZipOut );

    aZipOut.setMethod(DEFLATED);
    aZipOut.setLevel(DEFAULT_COMPRESSION);

    // Remove the old META-INF directory as this will be re-generated below.
    // Pass save-contents a vector which will be used to store information
    // that should be stored in the manifest.

    const OUString sMeta ( RTL_CONSTASCII_USTRINGPARAM ( "META-INF" ) );
    if (xRootFolder->hasByName( sMeta ) )
        xRootFolder->removeByName( sMeta );
    /// >>>>>>>>
    // Then create a tempfile...
    OUString sServiceName ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.io.TempFile" ) );

    Reference < XOutputStream > xTempOut = Reference < XOutputStream > ( xFactory->createInstance ( sServiceName ), UNO_QUERY );
#ifdef MTG_DEBUG
    fprintf(stderr, "We have a %s tempfile!\n", xTempOut.is() ? "good" : "bad" );
#endif
    Reference < XInputStream > xTempIn = Reference < XInputStream > ( xTempOut, UNO_QUERY );
    Reference < XSeekable > xTempSeek = Reference < XSeekable > ( xTempOut, UNO_QUERY );
    // Then write the full package to the temp file...
    Sequence < sal_Int8 > aBuffer;
    sal_Int64 nRead;

    do
    {
        nRead = xInBuffer->readBytes ( aBuffer, n_ConstBufferSize );
        xTempOut->writeBytes( aBuffer );
    }
    while ( nRead == n_ConstBufferSize );

    // seek back to the beginning of the temp file so we can read segments from it
    xTempSeek->seek ( 0 );
    xContentStream = xTempIn;
    xContentSeek = xTempSeek;
    pZipFile->setInputStream ( xTempIn );
    // <<<<<<<<<<


    if (!nSegmentSize )
    {
        try
        {
            pContent->writeStream ( xTempIn, sal_True );
        }
        catch (::com::sun::star::uno::Exception& r)
        {
            throw WrappedTargetException( OUString::createFromAscii( "Unable to write Zip File to disk!" ),
                    static_cast < OWeakObject * > ( this ), makeAny( r ) );
        }
        /*
        Reference < XActiveDataSink > xSink = new ZipPackageSink;
         * We want to reference the temp file, not the one we just wrote
        try
        {
            // Update our references to point to the new file
            if (pContent->openStream ( xSink ) )
                xContentStream = xSink->getInputStream();
            xContentSeek = Reference < XSeekable > (xContentStream, UNO_QUERY);
            pZipFile->setInputStream ( xContentStream );
        }
        catch (com::sun::star::uno::Exception& r)
        {
            throw WrappedTargetException( OUString::createFromAscii( "Unable to read Zip File content!" ),
                static_cast < OWeakObject * > ( this ), makeAny( r ) );
        }
        */
    }
    else
    {
        // We want to span...first, make sure we have an interaction handler...
        getInteractionHandler();

        sal_Int16 nDiskNum = 0;
        VolumeInfo aInfo ( osl_VolumeInfo_Mask_FreeSpace | osl_VolumeInfo_Mask_DeviceHandle | osl_VolumeInfo_Mask_Attributes );
        FileBase::RC aRC = Directory::getVolumeInfo ( sURL, aInfo );
#ifdef MTG_DEBUG
        fprintf(stderr, "MTG: url is %s getVolumeInfo returned %d\n", ImplGetChars ( sURL), aRC);
        fprintf(stderr, "MTG: isRemovable returned %d\n", aInfo.getRemoveableFlag() );
        fprintf(stderr, "MTG: isValid osl_VolumeInfo_Mask_FreeSpace returned %d\n", aInfo.isValid(osl_VolumeInfo_Mask_FreeSpace) );
        fprintf(stderr, "MTG: isValid osl_VolumeInfo_Mask_DeviceHandle returned %d\n", aInfo.isValid(osl_VolumeInfo_Mask_DeviceHandle) );
        fprintf(stderr, "MTG: isValid osl_VolumeInfo_Mask_Attributes returned %d\n", aInfo.isValid(osl_VolumeInfo_Mask_Attributes) );
        fprintf(stderr, "MTG: getFreeSpace returned %ld\n", static_cast < sal_Int32 > ( aInfo.getFreeSpace() ) );
        fprintf(stderr, "MTG: getUsedSpace returned %ld\n", static_cast < sal_Int32 > ( aInfo.getUsedSpace() ) );
        fprintf(stderr, "MTG: getTotalSpace returned %ld\n", static_cast < sal_Int32 > ( aInfo.getTotalSpace() ) );
#endif
        VolumeDevice aDevice = aInfo.getDeviceHandle();
        OUString sFileName, sMountPath = aDevice.getMountPath();
#ifdef MTG_DEBUG
        fprintf(stderr, "MTG: mount path is %s %d\n", ImplGetChars ( sMountPath ), aRC);
#endif
        sal_Int32 nLastSlash = sURL.lastIndexOf ( '/' );
        OUString sFilePrefix = sURL.copy ( 1 + nLastSlash,  sURL.lastIndexOf ( '.' ) - nLastSlash );

#ifdef MTG_DEBUG
        fprintf(stderr, "MTG: url is %s first getVolumeInfo on %s returned %d\n", ImplGetChars ( sURL), ImplGetChars ( sMountPath), aRC);
#endif
        sal_Bool bIsRemovable = aInfo.getRemoveableFlag();
#ifdef MTG_DEBUG
        fprintf(stderr, "MTG: Removable flag is %d\n", bIsRemovable);
#endif
        SegmentEnum eRet = e_Finished;
        SuffixGenerator aGenerator;
        do
        {
            //pBuffer->nextSegment( ++nDiskNum);
            nDiskNum++;
            sal_Int64 nCurrentPos = xTempSeek->getPosition();
            if ( nDiskNum == 1 )
                sFileName = sURL.copy ( 1 + nLastSlash );
            else
                aGenerator.generateFileName ( sFileName, sFilePrefix, nDiskNum );
            if ( bIsRemovable )
            {
                if ( nDiskNum > 1 && RequestDisk( sMountPath, nDiskNum ) < 0 )
                    return;

                do
                {
                    eRet = writeSegment ( sFileName, sMountPath, xTempIn, nDiskNum );
                    if (eRet == e_Aborted)
                        return;
                    else if ( eRet == e_Retry )
                        xTempSeek->seek ( nCurrentPos );
                }
                while ( eRet == e_Retry );
            }
            else
            {
                OUString sFullPath = sURL.copy ( 0, nLastSlash + 1 ) + sFileName;
                if ( xTempSeek->getLength() > static_cast < sal_Int64 > ( aInfo.getFreeSpace() ) )
                {
                    // no room on the hard drive, display a message and return
                    HandleError (  osl_File_E_NOSPC, EC_YES, sFullPath );
                    return;
                }
                else if ( nSegmentSize < 0 )
                {
                    try
                    {
                        pContent->writeStream ( xTempIn, sal_True );
                    }
                    catch (::com::sun::star::uno::Exception& r)
                    {
                        throw WrappedTargetException( OUString::createFromAscii( "Unable to write Zip File to disk!" ),
                                static_cast < OWeakObject * > ( this ), makeAny( r ) );
                    }
                }
                else
                {
                    do
                    {
                        eRet = writeSegment ( sFullPath, xTempIn );
                        if (eRet == e_Aborted)
                            return;
                        else if ( eRet == e_Retry )
                            xTempSeek->seek ( nCurrentPos );
                    }
                    while ( eRet == e_Retry );
                }
            }
        }
        while ( eRet != e_Finished );
    }
#ifdef MTG_DEBUG
    fprintf ( stderr, "MTG: ZipPackage Commit finished\n");
#endif
}

sal_Int32 ZipPackage::RequestDisk ( OUString &rMountPath, sal_Int16 nDiskNum)
{
    VolumeInfo aInfo ( osl_VolumeInfo_Mask_FreeSpace | osl_VolumeInfo_Mask_DeviceHandle | osl_VolumeInfo_Mask_Attributes );
    VolumeDevice aDevice;
    FileBase::RC aRC;

    do
    {
        aRC = Directory::getVolumeInfo ( rMountPath, aInfo );
        if ( aRC == FileBase::E_None )
            aDevice = aInfo.getDeviceHandle();
        else
        {
            if ( ! HandleError ( osl_File_E_INVAL, EC_RETRY|EC_ABORT, rMountPath) )
                return -1;
        }
    }
    while ( aRC != FileBase::E_None );
#ifdef UNX
    do
    {
        aRC = aDevice.unmount();
        if ( aRC != FileBase::E_None )
        {
            if ( ! HandleError ( osl_File_E_ACCES, EC_RETRY|EC_ABORT, rMountPath) )
                return -1;
        }
    }
    while ( aRC != FileBase::E_None );
#endif

    Any aExceptionAny, aMediumException;
    InteractiveWrongMediumException aException;
    aMediumException <<= static_cast < sal_Int16 > (nDiskNum-1);
    aException.Medium = aMediumException;
    aExceptionAny <<= aException;
    if ( !HandleError ( aExceptionAny, EC_YES|EC_ABORT ) )
        return -1;
#ifdef UNX
    do
    {
        aRC = aDevice.automount();

        if ( aRC != FileBase::E_None )
        {
            if ( ! HandleError ( osl_File_E_ACCES, EC_RETRY|EC_ABORT, rMountPath ) )
                return -1;
        }
    }
    while ( aRC != FileBase::E_None );
    OUString aNewMountPath ( aDevice.getMountPath() );

    if (aNewMountPath != rMountPath)
        rMountPath = aNewMountPath;
#endif
    return FileBase::E_None;
}
SegmentEnum ZipPackage::writeSegment ( const OUString &rFileName, OUString &rMountPath, Reference < XInputStream > &xInBuffer, const sal_Int16 nDiskNum )
{
    File *pFile = NULL;
    FileBase::RC aRC;
    sal_Bool bDynamicSpan = nSegmentSize < 0;
    Sequence < sal_Int8 > aBuffer;

    sal_Int32 nRead = n_ConstBufferSize;
    sal_uInt64 nLeft, nWritten;
    VolumeInfo aInfo ( osl_VolumeInfo_Mask_FreeSpace | osl_VolumeInfo_Mask_DeviceHandle | osl_VolumeInfo_Mask_Attributes );

#ifdef MTG_DEBUG
    fprintf (stderr, "MTG: In writeSegment, disk num is %d, file is %s, dir is %s\n",
                     nDiskNum, ImplGetChars(rFileName), ImplGetChars(rMountPath));
#endif
    do
    {
        aRC = Directory::getVolumeInfo ( rMountPath, aInfo );
#ifdef MTG_DEBUG
        fprintf(stderr, "MTG: getVolumeInfo returned %d\n", aRC );
#endif
        if (aRC == FileBase::E_None )
        {
            sal_Bool bReCheck;
            OUStringBuffer aBuffer;
            aBuffer.append ( rMountPath );
            if ( rMountPath.lastIndexOf ( '/' ) != rMountPath.getLength()-1 )
                aBuffer.appendAscii ( "/" );
            aBuffer.append ( rFileName );
            OUString sFullPath ( aBuffer.makeStringAndClear() );
            do
            {
                bReCheck = sal_False;
                sal_uInt64 nFree = aInfo.getFreeSpace();
#ifdef MTG_DEBUG
                fprintf(stderr, "MTG: free is  %d\n", static_cast < sal_Int32 > ( nFree ) );
#endif
                if ( (  bDynamicSpan && nFree < 1000         ) ||
                     ( !bDynamicSpan && nFree < nSegmentSize ) )
                {
                    if ( !HandleError (  osl_File_E_NOSPC, EC_RETRY|EC_ABORT, sFullPath )
                      || RequestDisk ( rMountPath, nDiskNum ) < 0 )
                    {
                        if ( pFile )
                            delete pFile;
                        return e_Aborted;
                    }
                    else
                    {
                        aRC = Directory::getVolumeInfo ( rMountPath, aInfo );
                        bReCheck = sal_True;
                    }
                }
                else
                {
                    nLeft = bDynamicSpan ? nFree : nSegmentSize;
#ifdef MTG_DEBUG
                    fprintf(stderr, "MTG: left is %ld\n", static_cast < sal_Int32 > ( nLeft ) );
#endif
                }
            }
            while ( bReCheck );
#ifdef MTG_DEBUG
            fprintf( stderr, "MTG: sDirectoryName is %s sFileName is %s FullPath is %s\n",
                              ImplGetChars ( rMountPath ), ImplGetChars ( rFileName ), ImplGetChars ( sFullPath ) );
#endif
            pFile = new File ( sFullPath );
            aRC = pFile->open ( osl_File_OpenFlag_Create | osl_File_OpenFlag_Write );
            if ( aRC == FileBase::E_EXIST )
                aRC = pFile->open ( osl_File_OpenFlag_Write );
            if ( aRC != FileBase::E_None )
            {
                if ( ! HandleError (  (oslFileError) aRC, EC_RETRY|EC_ABORT, sFullPath ) )
                {
                    delete pFile;
                    return e_Aborted;
                }
            }
        }
#ifdef MTG_DEBUG
        fprintf(stderr, "MTG: file open returned %d\n", aRC );
#endif
    }
    while (aRC != FileBase::E_None );

    // Now! We should have an open file on a disk which has at least nSegmentSize if not
    // dynamic spanning and 1000 bytes if dynamic spanning.


    // Let's read it all into the buffer in case something goes wrong and also
    // so that the spannable checks in ByteChucker and ZipOutputStream work

    nRead = xInBuffer->readBytes ( aBuffer, static_cast < sal_Int32 > ( nLeft ) );

    aRC = pFile->write ( aBuffer.getConstArray(), nRead, nWritten );
#ifdef MTG_DEBUG
    fprintf ( stderr, "MTG: write returned %d\n", aRC );
#endif

    sal_Bool bRetry = sal_False;

    if ( nWritten != nRead || aRC != FileBase::E_None )
        bRetry = sal_True;
    else
    {
        aRC = pFile->close ();
#ifdef MTG_DEBUG
        fprintf ( stderr, "MTG: close returned %d\n", aRC );
#endif
        if ( aRC != FileBase::E_None )
            bRetry = sal_True;
    }
    delete pFile;
    return nRead < nLeft ? e_Finished : bRetry ? e_Retry : e_Success;
}

SegmentEnum ZipPackage::writeSegment ( const OUString &rFileName, Reference < XInputStream > &xInBuffer )
{
    FileBase::RC aRC;
    Sequence < sal_Int8 > aBuffer ( nSegmentSize );

    sal_uInt64 nWritten;

    sal_Int32 nRead = xInBuffer->readBytes ( aBuffer, static_cast < sal_Int32 > ( nSegmentSize ) );

    File aFile ( rFileName );
    aRC = aFile.open ( osl_File_OpenFlag_Create | osl_File_OpenFlag_Write );
    if ( aRC == FileBase::E_EXIST )
        aRC = aFile.open ( osl_File_OpenFlag_Write );
    if ( aRC != FileBase::E_None )
    {
        if ( ! HandleError (  (oslFileError) aRC, EC_RETRY|EC_ABORT, rFileName ) )
            return e_Aborted;
    }

    aRC = aFile.write ( aBuffer.getConstArray(), nRead, nWritten );
#ifdef MTG_DEBUG
    fprintf ( stderr, "MTG: write returned %d\n", aRC );
#endif

    sal_Bool bRetry = sal_False;
    if ( nWritten != nRead || aRC != FileBase::E_None )
        bRetry = sal_True;
    else
    {
        aRC = aFile.close ();
#ifdef MTG_DEBUG
        fprintf ( stderr, "MTG: close returned %d\n", aRC );
#endif
        if ( aRC != FileBase::E_None )
            bRetry = sal_True;
    }
    return nRead < nSegmentSize ? e_Finished : bRetry ? e_Retry : e_Success;
}

SegmentEnum ZipPackage::readSegment ( const OUString &rFileName, OUString &rMountPath, Reference < XOutputStream > &xTempOut, const sal_Int16 nDiskNum )
{
    File *pFile = NULL;
    FileBase::RC aRC;
    SegmentEnum eRet;

    OUStringBuffer aStringBuffer;
    aStringBuffer.append ( rMountPath );
    if ( rMountPath.lastIndexOf ( '/' ) != rMountPath.getLength()-1 )
        aStringBuffer.appendAscii ( "/" );
    aStringBuffer.append ( rFileName );
    OUString sFullPath ( aStringBuffer.makeStringAndClear() );

    DirectoryItem aItem;
    do
    {
        aRC = DirectoryItem::get ( sFullPath, aItem );
        if ( aRC != FileBase::E_None &&  !HandleError (  (oslFileError) aRC, EC_RETRY|EC_ABORT, sFullPath ) )
            return e_Aborted;
    }
    while (aRC != FileBase::E_None );
    do
    {
        pFile = new File ( sFullPath );
        aRC = pFile->open ( osl_File_OpenFlag_Read );
        if ( aRC != FileBase::E_None )
        {
            delete pFile;
            if ( ! HandleError (  (oslFileError) aRC, EC_RETRY|EC_ABORT, sFullPath ) )
                return e_Aborted;
        }
    }
    while (aRC != FileBase::E_None );

    FileStatus aStatus ( FileStatusMask_FileSize );
    aItem.getFileStatus ( aStatus );
    sal_uInt64 nRead, nLeft = aStatus.getFileSize(), nToRead;

    Sequence < sal_Int8 > aBuffer ( n_ConstBufferSize );
    do
    {
        nToRead = nLeft < n_ConstBufferSize ? nLeft : n_ConstBufferSize;
        aRC = pFile->read ( static_cast < void* > ( aBuffer.getArray() ), nToRead, nRead );
        if ( aRC != FileBase::E_None )
        {
            if ( ! HandleError (  (oslFileError) aRC, EC_RETRY|EC_ABORT, sFullPath ) )
                return e_Aborted;
        }
        else
        {
            Sequence < sal_Int8 > aReadBuf ( aBuffer.getConstArray(), static_cast < sal_Int32 > ( nRead ) );
            nLeft -= nRead;
            xTempOut->writeBytes ( aReadBuf );
            if (nLeft == 0)
                eRet = checkEnd ( aReadBuf ) ? e_Finished : e_Success;
        }
    }
    while (nLeft > 0 );

    aRC = pFile->close ();
    delete pFile;
    return eRet;
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
    throw (RuntimeException)
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
                                        static_cast< XPropertySet*      > ( this )  ,
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

Reference< XPropertySetInfo > SAL_CALL ZipPackage::getPropertySetInfo(  )
        throw(RuntimeException)
{
    return Reference < XPropertySetInfo > (NULL);
}
void SAL_CALL ZipPackage::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("HasEncryptedEntries") ) )
        throw IllegalArgumentException (); // This property is read-only
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("EncryptionKey") ) )
    {
        if (!( aValue >>= aEncryptionKey ) )
            throw IllegalArgumentException();
    }
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("SegmentSize") ) )
    {
        if (!( aValue >>= nSegmentSize ) )
            throw IllegalArgumentException();
    }
    else
        throw UnknownPropertyException();
}
Any SAL_CALL ZipPackage::getPropertyValue( const OUString& PropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Any aAny;
    if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "EncryptionKey" ) ) )
    {
        aAny <<= aEncryptionKey;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "SegmentSize" ) ) )
    {
        aAny <<= nSegmentSize;
        return aAny;
    }
    if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "HasEncryptedEntries" ) ) )
    {
        aAny <<= bHasEncryptedEntries;
        return aAny;
    }
    throw UnknownPropertyException();
}
void SAL_CALL ZipPackage::addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackage::removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackage::addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackage::removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void ZipPackage::getInteractionHandler()
{
    if ( ! xInteractionHandler.is() )
    {
        OUString sServiceName ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.task.InteractionHandler" ) );
        xInteractionHandler = Reference < XInteractionHandler > ( xFactory->createInstance ( sServiceName ), UNO_QUERY );
    }
}

IOErrorCode Impl_OSLFileErrorToUCBIoErrorCode ( oslFileError aRC )
{
    IOErrorCode eReturn = IOErrorCode_UNKNOWN;
    switch ( aRC )
    {
        // open
        case osl_File_E_NOMEM://        not enough memory for allocating structures <br>
            eReturn = IOErrorCode_OUT_OF_MEMORY;
        break;
        case osl_File_E_NAMETOOLONG://  pathname was too long<br>
            eReturn = IOErrorCode_NAME_TOO_LONG;
        break;
        case osl_File_E_NOENT://        No such file or directory<br>
            eReturn = IOErrorCode_NOT_EXISTING;
        break;
        case osl_File_E_ACCES://        permission denied<P>
            eReturn = IOErrorCode_ACCESS_DENIED;
        break;
        case osl_File_E_ISDIR://        Is a directory<p>
            eReturn = IOErrorCode_INVALID_ACCESS;
        break;
        case osl_File_E_NOTDIR://       Not a directory<br>
            eReturn = IOErrorCode_NO_DIRECTORY;
        break;
        case osl_File_E_NXIO://         No such device or address<br>
            eReturn = IOErrorCode_INVALID_DEVICE;
        break;
        case osl_File_E_NODEV://        No such device<br>
            eReturn = IOErrorCode_INVALID_DEVICE;
        break;
        case osl_File_E_ROFS://         Read-only file system<br>
            eReturn = IOErrorCode_ACCESS_DENIED;
        break;
        case osl_File_E_FAULT://        Bad address<br>
            eReturn = IOErrorCode_INVALID_DEVICE;
        break;
        case osl_File_E_LOOP://         Too many symbolic links encountered<br>
        break;
        case osl_File_E_MFILE://        too many open files used by the process<br>
        break;
        case osl_File_E_NFILE://        too many open files in the system<br>
        break;
        case osl_File_E_EXIST://        File exists<br>
            eReturn = IOErrorCode_CANT_CREATE;
        break;
        case osl_File_E_MULTIHOP://     Multihop attempted<br>
        break;
        case osl_File_E_FBIG://         File too large<br>
            eReturn = IOErrorCode_INVALID_LENGTH;
        break;

        // write
        case osl_File_E_AGAIN://        Operation would block<br>
        break;
        case osl_File_E_NOLCK://        No record locks available<br>
        break;
        case osl_File_E_NOSPC://        No space left on device<br>
            eReturn = IOErrorCode_OUT_OF_DISK_SPACE;
        break;
        case osl_File_E_INVAL://        the format of the parameters was not valid<p>
            eReturn = IOErrorCode_INVALID_PARAMETER;
        break;

        // close
        case osl_File_E_BADF://         Bad file<br>
            eReturn = IOErrorCode_NO_FILE;
        break;
        case osl_File_E_INTR://         function call was interrupted<br>
            eReturn = IOErrorCode_ABORT;
        break;
        case osl_File_E_NOLINK://       Link has been severed<br>
        break;
        case osl_File_E_IO://           I/O error<p>
            eReturn = IOErrorCode_GENERAL;
        break;
    }
    return eReturn;
}

sal_Bool ZipPackage::HandleError ( Any &rAny, sal_uInt16 eContinuations )
{
    InteractionRequest* pRequest;
    Reference < XInteractionRequest > xRequest ( pRequest = new InteractionRequest ( rAny, eContinuations ));
    xInteractionHandler->handle ( xRequest );
    const sal_uInt16 nSelection = pRequest->getSelection();
    return nSelection == EC_YES || nSelection == EC_RETRY;
}

sal_Bool ZipPackage::HandleError ( oslFileError aRC, sal_uInt16 eContinuations, const OUString &rFileName )
{
    Any aAny;
    InteractiveFileIOException aException;
    aException.Code = Impl_OSLFileErrorToUCBIoErrorCode ( aRC );
    aException.FileName = rFileName;
    aAny <<= aException;
    return HandleError (aAny, eContinuations );
}

Reference < XInputStream > ZipPackage::unSpanFile ( Reference < XInputStream > &rStream )
{
    const OUString sServiceName ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.io.TempFile" ) );
    Reference < XInputStream > xTempIn = Reference < XInputStream > ( xFactory->createInstance ( sServiceName ), UNO_QUERY );
    Reference < XOutputStream > xTempOut = Reference < XOutputStream > ( xTempIn, UNO_QUERY );
    Reference < XSeekable > xTempSeek = Reference < XSeekable > ( xTempIn, UNO_QUERY );
    Reference < XSeekable > xInSeek = Reference < XSeekable > ( rStream, UNO_QUERY );

    Sequence < sal_Int8 > aBuffer;
    sal_Int64 nRead;
    xInSeek->seek ( 0 ); // Skip spanned header
    do
    {
        nRead = rStream->readBytes ( aBuffer, n_ConstBufferSize );
        xTempOut->writeBytes ( aBuffer );
    }
    while ( nRead == n_ConstBufferSize );

    // Check if the buffer just read is the last one
    if ( checkEnd ( aBuffer ) )
        return xTempIn;

    sal_Int16 nDiskNum = 1;
    VolumeInfo aInfo ( osl_VolumeInfo_Mask_FreeSpace | osl_VolumeInfo_Mask_DeviceHandle | osl_VolumeInfo_Mask_Attributes );
    FileBase::RC aRC = Directory::getVolumeInfo ( sURL, aInfo );
    VolumeDevice aDevice = aInfo.getDeviceHandle();
    sal_Bool bIsRemovable = aInfo.getRemoveableFlag();

    sal_Int32 nLastSlash = sURL.lastIndexOf ( '/' );
    OUString sFileName, sMountPath = aDevice.getMountPath();
    const OUString sFilePrefix = sURL.copy ( 1 + nLastSlash,  sURL.lastIndexOf ( '.' ) - nLastSlash );
    SegmentEnum eRet = e_Finished;
    SuffixGenerator aGenerator;
    do
    {
        //pBuffer->nextSegment( ++nDiskNum);
        nDiskNum++;
        aGenerator.generateFileName ( sFileName, sFilePrefix, nDiskNum );
        if ( bIsRemovable )
        {
            // We need an interaction handler to request disks
            getInteractionHandler();
            if ( RequestDisk( sMountPath, nDiskNum ) < 0 )
                return Reference < XInputStream > ();
            eRet = readSegment ( sFileName, sMountPath, xTempOut, nDiskNum );
            if (eRet == e_Aborted)
                return Reference < XInputStream > ();
        }
        else
        {
            OUString sFullPath = sURL.copy ( 0, nLastSlash + 1 ) + sFileName;
            Reference < XInputStream > xStream;
            Content aContent (sFullPath, Reference < XCommandEnvironment >() );
            Reference < XActiveDataSink > xSink = new ZipPackageSink;
            try
            {
                if (aContent.openStream ( xSink ) )
                    xStream = xSink->getInputStream();
                if ( xStream.is() )
                {
                    do
                    {
                        nRead = xStream->readBytes ( aBuffer, n_ConstBufferSize );
                        xTempOut->writeBytes ( aBuffer );
                    }
                    while ( nRead == n_ConstBufferSize );
                    eRet = checkEnd ( aBuffer ) ? e_Finished : e_Success;
                }
            }
            catch (com::sun::star::uno::Exception&)
            {
                // bad juju
                //
            }
        }
    }
    while ( eRet != e_Finished );

    return xTempIn;
}

sal_Bool ZipPackage::checkEnd ( Sequence < sal_Int8 > &rSequence )
{
    sal_Int32 nLength, nPos, nEnd;

    nLength = static_cast <sal_Int32 > (rSequence.getLength());
    // Start at what should be the beginning of the end header
    if ( nLength == 0 || nLength < ENDHDR )
        return sal_False;
    nPos = nLength - ENDHDR - ZIP_MAXNAMELEN;
    // We'll only look for the END signature within a sane range (ZIP_MAXNAMELEN)
    nEnd = nPos >= 0 ? nPos : 0;

    const sal_Int8 *pBuffer = rSequence.getConstArray();
    nPos = nLength - ENDHDR;
    while ( nPos >= nEnd )
    {
        if (pBuffer[nPos] == 'P' && pBuffer[nPos+1] == 'K' && pBuffer[nPos+2] == 5 && pBuffer[nPos+3] == 6 )
            return sal_True;
        nPos--;
    }
    return sal_False;
}
