/*************************************************************************
 *
 *  $RCSfile: ZipPackageFolder.cxx,v $
 *
 *  $Revision: 1.49 $
 *
 *  last change: $Author: mtg $ $Date: 2001-09-14 15:14:12 $
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
#ifndef _ZIP_PACKAGE_FOLDER_HXX
#include <ZipPackageFolder.hxx>
#endif
#ifndef _ZIP_FILE_HXX
#include <ZipFile.hxx>
#endif
#ifndef _ZIP_OUTPUT_STREAM_HXX
#include <ZipOutputStream.hxx>
#endif
#ifndef _ZIP_PACKAGE_STREAM_HXX
#include <ZipPackageStream.hxx>
#endif
#ifndef _PACKAGE_CONSTANTS_HXX_
#include <PackageConstants.hxx>
#endif
#ifndef _ZIP_PACKAGE_FOLDER_ENUMERATION_HXX
#include <ZipPackageFolderEnumeration.hxx>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIP_ZIPCONSTANTS_HPP_
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#endif
#ifndef _VOS_DIAGNOSE_H_
#include <vos/diagnose.hxx>
#endif
#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif
#ifndef _RTL_DIGEST_H_
#include <rtl/digest.h>
#endif
#ifndef _CONTENT_INFO_HXX_
#include <ContentInfo.hxx>
#endif

using namespace com::sun::star::packages::zip::ZipConstants;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::container;
using namespace com::sun::star::packages;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace cppu;
using namespace rtl;
using namespace std;

ZipPackageFolder::ZipPackageFolder (void)
: ZipPackageEntry ( true )
, mbHasReleased  ( false )
{
    aEntry.nVersion     = -1;
    aEntry.nFlag        = 0;
    aEntry.nMethod      = STORED;
    aEntry.nTime        = -1;
    aEntry.nCrc         = 0;
    aEntry.nCompressedSize  = 0;
    aEntry.nSize        = 0;
    aEntry.nOffset      = -1;
}


ZipPackageFolder::~ZipPackageFolder( void )
{
    ContentHash::iterator aIter = maContents.begin();
    while ( aIter != maContents.end() )
    {
        delete (*aIter).second;
        aIter++;
    }
}
void ZipPackageFolder::copyZipEntry( ZipEntry &rDest, const ZipEntry &rSource)
{
      rDest.nVersion            = rSource.nVersion;
    rDest.nFlag             = rSource.nFlag;
    rDest.nMethod           = rSource.nMethod;
    rDest.nTime             = rSource.nTime;
    rDest.nCrc              = rSource.nCrc;
    rDest.nCompressedSize   = rSource.nCompressedSize;
    rDest.nSize             = rSource.nSize;
    rDest.nOffset           = rSource.nOffset;
    rDest.sName             = rSource.sName;
}
Any SAL_CALL ZipPackageFolder::queryInterface( const Type& rType )
    throw(RuntimeException)
{
    // cppu::queryInterface is an inline template so it's fast
    // unfortunately, it always creates an Any...we should be able to optimise
    // this with a class static containing supported interfaces
    // ...will research this further ...mtg 15/12/00
    return ::cppu::queryInterface ( rType                                       ,
                                        // OWeakObject interfaces
                                        reinterpret_cast< XInterface*       > ( this )  ,
                                        static_cast< XWeak*         > ( this )  ,
                                        // ZipPackageEntry interfaces
                                        static_cast< XNamed*        > ( this )  ,
                                        static_cast< XChild*        > ( this )  ,
                                        static_cast< XUnoTunnel*        > ( this )  ,
                                        // my own interfaces
                                        static_cast< XNameContainer*        > ( this )  ,
                                        static_cast< XEnumerationAccess*        > ( this )  ,
                                        static_cast< XPropertySet*  > ( this ) );

}

void SAL_CALL ZipPackageFolder::acquire(  )
    throw()
{
    OWeakObject::acquire();
}
void SAL_CALL ZipPackageFolder::release(  )
    throw()
{
    OWeakObject::release();
}

    // XNameContainer
void SAL_CALL ZipPackageFolder::insertByName( const OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    if (hasByName(aName))
        throw ElementExistException();
    else
    {
        Reference < XUnoTunnel > xRef;
        aElement >>= xRef;
        if ( (  aElement >>= xRef ) )
        {
            sal_Int64 nTest;
            ZipPackageEntry *pEntry;
            if ( ( nTest = xRef->getSomething ( ZipPackageEntry::getUnoTunnelImplementationId() ) ) != 0 )
                pEntry = reinterpret_cast < ZipPackageEntry * > ( nTest );
            else
                throw IllegalArgumentException();

            if (pEntry->getName() != aName )
                pEntry->setName (aName);
            doInsertByName ( pEntry, sal_True );
        }
        else
            throw IllegalArgumentException();
    }
}
void SAL_CALL ZipPackageFolder::removeByName( const OUString& Name )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    ContentHash::iterator aIter = maContents.find ( Name );
    if ( aIter == maContents.end() )
        throw NoSuchElementException();
    maContents.erase( aIter );
}
    // XEnumerationAccess
Reference< XEnumeration > SAL_CALL ZipPackageFolder::createEnumeration(  )
        throw(RuntimeException)
{
    return Reference < XEnumeration> (new ZipPackageFolderEnumeration(maContents));
}
    // XElementAccess
Type SAL_CALL ZipPackageFolder::getElementType(  )
        throw(RuntimeException)
{
    return ::getCppuType ((const Reference< XUnoTunnel > *) 0);
}
sal_Bool SAL_CALL ZipPackageFolder::hasElements(  )
        throw(RuntimeException)
{
    return maContents.size() > 0;
}
    // XNameAccess
ContentInfo& ZipPackageFolder::doGetByName( const OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    ContentHash::iterator aIter = maContents.find ( aName );
    if ( aIter == maContents.end())
        throw NoSuchElementException();
    return *(*aIter).second;
}
Any SAL_CALL ZipPackageFolder::getByName( const OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    return makeAny ( doGetByName ( aName ).xTunnel );
}
Sequence< OUString > SAL_CALL ZipPackageFolder::getElementNames(  )
        throw(RuntimeException)
{
    sal_uInt32 i=0, nSize = maContents.size();
    OUString *pNames = new OUString[nSize];
    for ( ContentHash::const_iterator aIterator = maContents.begin(), aEnd = maContents.end();
          aIterator != aEnd;
          i++,aIterator++)
        pNames[i] = (*aIterator).first;
    return Sequence < OUString > (pNames, nSize);
}
sal_Bool SAL_CALL ZipPackageFolder::hasByName( const OUString& aName )
    throw(RuntimeException)
{
    return maContents.find ( aName ) != maContents.end ();
}
    // XNameReplace
void SAL_CALL ZipPackageFolder::replaceByName( const OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    if ( hasByName( aName ) )
        removeByName( aName );
    else
        throw NoSuchElementException();
    insertByName(aName, aElement);
}
void ZipPackageFolder::saveContents(OUString &rPath, std::vector < Sequence < PropertyValue > > &rManList, ZipOutputStream & rZipOut, Sequence < sal_Int8 > &rEncryptionKey, rtlRandomPool &rRandomPool)
    throw(RuntimeException)
{
    ZipPackageFolder *pFolder = NULL;
    ZipPackageStream *pStream = NULL;
    const OUString sMediaTypeProperty ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) );
    const OUString sFullPathProperty ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );
    const OUString sInitialisationVectorProperty ( RTL_CONSTASCII_USTRINGPARAM ( "InitialisationVector" ) );
    const OUString sSaltProperty ( RTL_CONSTASCII_USTRINGPARAM ( "Salt" ) );
    const OUString sIterationCountProperty ( RTL_CONSTASCII_USTRINGPARAM ( "IterationCount" ) );
    const OUString sSizeProperty ( RTL_CONSTASCII_USTRINGPARAM ( "Size" ) );
    const OUString sDigestProperty ( RTL_CONSTASCII_USTRINGPARAM ( "Digest" ) );

    sal_Bool bHaveEncryptionKey = rEncryptionKey.getLength() ? sal_True : sal_False;

    for ( ContentHash::const_iterator aCI = maContents.begin(), aEnd = maContents.end();
          aCI != aEnd;
          aCI++)
    {
        // pTempEntry is stored in a vector by ZipOutputStream and will
        // be deleted by the ZipOutputStream destructor
        ZipEntry * pTempEntry = new ZipEntry;
        const OUString &rShortName = (*aCI).first;
        const ContentInfo &rInfo = *(*aCI).second;

        Sequence < PropertyValue > aPropSet (2);
        PropertyValue *pValue = aPropSet.getArray();

        if ( rInfo.bFolder )
            pFolder = rInfo.pFolder;
        else
            pStream = rInfo.pStream;

        if ( rInfo.bFolder )
        {
            OUString sTempName = rPath + rShortName + OUString( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );

            pValue[0].Name = sMediaTypeProperty;
            pValue[0].Value <<= pFolder->GetMediaType();
            pValue[1].Name = sFullPathProperty;
            pValue[1].Value <<= sTempName;

            rInfo.pFolder->saveContents( sTempName, rManList, rZipOut, rEncryptionKey, rRandomPool);
        }
        else
        {
            // In case the entry we are reading is also the entry we are writing, we will
            // store the ZipEntry data in pTempEntry

            ZipPackageFolder::copyZipEntry ( *pTempEntry, pStream->aEntry );
            pTempEntry->sName = rPath + rShortName;

            sal_Bool bToBeEncrypted = pStream->IsToBeEncrypted() && bHaveEncryptionKey;
            sal_Bool bToBeCompressed = bToBeEncrypted ? sal_True : pStream->IsToBeCompressed();

            pValue[0].Name = sMediaTypeProperty;
            pValue[0].Value <<= pStream->GetMediaType( );
            pValue[1].Name = sFullPathProperty;
            pValue[1].Value <<= pTempEntry->sName;

            if ( bToBeEncrypted )
            {
                Sequence < sal_uInt8 > aSalt ( 16 ), aVector ( 8 );
                Sequence < sal_Int8 > aKey ( 16 );
                rtl_random_getBytes ( rRandomPool, aSalt.getArray(), 16 );
                rtl_random_getBytes ( rRandomPool, aVector.getArray(), 8 );
                sal_Int32 nIterationCount = 1024;

                rtl_digest_PBKDF2 ( reinterpret_cast < sal_uInt8 * > (aKey.getArray()), 16,
                                    reinterpret_cast < sal_uInt8 * > (rEncryptionKey.getArray()), rEncryptionKey.getLength(),
                                    aSalt.getConstArray(), 16,
                                    nIterationCount );
                pStream->setInitialisationVector ( aVector );
                pStream->setSalt ( aSalt );
                pStream->setIterationCount ( nIterationCount );
                pStream->setKey ( aKey );

                aPropSet.realloc(7); // 6th property is size, below, 7th is Digest
                pValue = aPropSet.getArray();
                pValue[2].Name = sInitialisationVectorProperty;
                pValue[2].Value <<= aVector;
                pValue[3].Name = sSaltProperty;
                pValue[3].Value <<= aSalt;
                pValue[4].Name = sIterationCountProperty;
                pValue[4].Value <<= nIterationCount;
            }

            // If the entry is already stored in the zip file in the format we
            // want for this write...copy it raw
            if (pStream->IsPackageMember() && !bToBeEncrypted &&
                ( (pStream->aEntry.nMethod == DEFLATED && bToBeCompressed) ||
                  (pStream->aEntry.nMethod == STORED && !bToBeCompressed) ) )
            {
                try
                {
                    Reference < XInputStream > xStream = pStream->getRawStream();
                    rZipOut.putNextEntry ( *pTempEntry, pStream->getEncryptionData(), bToBeEncrypted);
                    Sequence < sal_Int8 > aSeq (n_ConstBufferSize);
                    sal_Int32 nLength = n_ConstBufferSize;
                    while ( nLength >= n_ConstBufferSize )
                    {
                        nLength = xStream->readBytes(aSeq, n_ConstBufferSize);
                        rZipOut.rawWrite(aSeq, 0, nLength);
                    }
                    rZipOut.rawCloseEntry();
                }
                catch (ZipException&)
                {
                    VOS_ENSURE( 0, "Error writing ZipOutputStream" );
                }
                catch (IOException & )
                {
                    VOS_ENSURE( 0, "Error writing ZipOutputStream" );
                }
            }
            else
            {
                Reference < XInputStream > xStream = pStream->getInputStream();
                Reference < XSeekable > xSeek (xStream, UNO_QUERY);
                if (!xSeek.is())
                    VOS_ENSURE ( 0, "Bad juju! We can only package streams which support XSeekable!");
                sal_Int32 nStreamLength = static_cast < sal_Int32 > ( xSeek->getLength() );

                if ( bToBeCompressed )
                {
                    pTempEntry->nMethod = DEFLATED;
                    pTempEntry->nCrc = pTempEntry->nCompressedSize = pTempEntry->nSize = -1;
                }
                else
                {
                    CRC32 aCRC32;
                    pTempEntry->nMethod = STORED;
                    pTempEntry->nCompressedSize = pTempEntry->nSize = nStreamLength;
                    aCRC32.updateStream ( xStream );
                    pTempEntry->nCrc = aCRC32.getValue();
                }

                if (bToBeEncrypted)
                {
                    // Need to store the uncompressed size in the manifest
                    pValue[5].Name = sSizeProperty;
                    pValue[5].Value <<= nStreamLength;
                }

                try
                {
                    rZipOut.putNextEntry ( *pTempEntry, pStream->getEncryptionData(), bToBeEncrypted);
                    sal_Int32 nLength = n_ConstBufferSize;
                    Sequence < sal_Int8 > aSeq (n_ConstBufferSize);
                    while ( nLength >= n_ConstBufferSize )
                    {
                        nLength = xStream->readBytes(aSeq, n_ConstBufferSize);
                        rZipOut.write(aSeq, 0, nLength);
                    }
                    pStream->SetPackageMember ( sal_True );
                    rZipOut.closeEntry();
                }
                catch (ZipException&)
                {
                    VOS_ENSURE( 0, "Error writing ZipOutputStream" );
                }
                catch (IOException & )
                {
                    VOS_ENSURE( 0, "Error writing ZipOutputStream" );
                }
                if (bToBeEncrypted)
                {
                    // Need to store the uncompressed size in the manifest
                    pValue[6].Name = sDigestProperty;
                    pValue[6].Value <<= pStream->getDigest();
                }
            }

            // Then copy it back afterwards...
            ZipPackageFolder::copyZipEntry ( pStream->aEntry, *pTempEntry );
            pStream->aEntry.sName = rShortName;
            pStream->aEntry.nOffset *= -1;
        }
        rManList.push_back (aPropSet);
    }
}

void ZipPackageFolder::releaseUpwardRef( void )
{
    mbHasReleased = true;
    for ( ContentHash::const_iterator aCI = maContents.begin();
          aCI!=maContents.end();
          aCI++)
    {
        ContentInfo &rInfo = * (*aCI).second;
        if ( rInfo.bFolder && ! rInfo.pFolder->HasReleased () )
            rInfo.pFolder->releaseUpwardRef();
        else
            rInfo.pStream->clearParent();
    }
    clearParent();
}

Sequence< sal_Int8 >& ZipPackageFolder::getUnoTunnelImplementationId( void )
    throw (RuntimeException)
{
    static ::cppu::OImplementationId * pId = 0;
    static Sequence < sal_Int8 > aIDSeq;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
            aIDSeq = pId->getImplementationId();
        }
    }
    return aIDSeq;
}

sal_Int64 SAL_CALL ZipPackageFolder::getSomething( const Sequence< sal_Int8 >& aIdentifier )
    throw(RuntimeException)
{
    sal_Int64 nMe = 0;
    if (aIdentifier.getLength() == 16 &&
        ( 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  aIdentifier.getConstArray(), 16 ) ||
          0 == rtl_compareMemory(ZipPackageEntry::getUnoTunnelImplementationId().getConstArray(),  aIdentifier.getConstArray(), 16 ) ) )
        nMe = reinterpret_cast < sal_Int64 > ( this );
    return nMe;
}
void SAL_CALL ZipPackageFolder::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("MediaType")))
        aValue >>= sMediaType;
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Size") ) )
        aValue >>= aEntry.nSize;
    else
        throw UnknownPropertyException();
}
Any SAL_CALL ZipPackageFolder::getPropertyValue( const OUString& PropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Any aAny;
    if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
        return makeAny ( sMediaType );
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Size" ) ) )
        return makeAny ( aEntry.nSize );
    else
        throw UnknownPropertyException();
}

void ZipPackageFolder::doInsertByName ( ZipPackageEntry *pEntry, sal_Bool bSetParent )
        throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    ContentInfo *pInfo;

    if ( pEntry->IsFolder() )
        pInfo = new ContentInfo ( static_cast < ZipPackageFolder *> ( pEntry ) );
    else
        pInfo = new ContentInfo ( static_cast < ZipPackageStream *> ( pEntry ) );

    maContents[pEntry->aEntry.sName] = pInfo;
    if ( bSetParent )
        pEntry->setParent ( *this );
}
