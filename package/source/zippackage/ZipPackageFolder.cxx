/*************************************************************************
 *
 *  $RCSfile: ZipPackageFolder.cxx,v $
 *
 *  $Revision: 1.36 $
 *
 *  last change: $Author: mtg $ $Date: 2001-04-27 14:56:07 $
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
#ifndef _ZIP_PACKAGE_FOLDER_ENUMERATION_HXX
#include <ZipPackageFolderEnumeration.hxx>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIPCONSTANTS_HPP_
#include <com/sun/star/packages/ZipConstants.hpp>
#endif
#ifndef _VOS_DIAGNOSE_H_
#include <vos/diagnose.hxx>
#endif
#ifndef _RTL_RANDOM_H_
#include <rtl/random.h>
#endif
#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

using namespace com::sun::star::packages::ZipConstants;
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
    rDest.extra             = rSource.extra;
    rDest.sComment          = rSource.sComment;
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
    OUString sName;
    if (aName.indexOf('/', 0 ) == 0)
        sName = aName.copy(1, aName.getLength());
    else
        sName = aName;

    if (hasByName(sName))
        throw ElementExistException();
    else
    {
        Reference < XUnoTunnel > xRef;
        aElement >>= xRef;
        Reference < XNamed > xNamed (xRef, UNO_QUERY);
        Reference < XChild > xChild (xRef, UNO_QUERY);
        Reference < XInterface > xInterface (*this);
        xNamed->setName (sName);
        aContents[sName] = xRef;
        try
        {
            xChild->setParent (xInterface);
        }
        catch ( NoSupportException& )
        {
            VOS_ENSURE( 0, "setParent threw an exception: attempted to set Parent to non-existing interface!");
        }
    }
}
void SAL_CALL ZipPackageFolder::removeByName( const OUString& Name )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    OUString sName;
    if (Name.indexOf('/', 0 ) == 0)
        sName = Name.copy(1, Name.getLength());
    else
        sName = Name;
    if (!aContents.count(sName))
        throw NoSuchElementException();
    aContents.erase(sName);
}
    // XEnumerationAccess
Reference< XEnumeration > SAL_CALL ZipPackageFolder::createEnumeration(  )
        throw(RuntimeException)
{
    return Reference < XEnumeration> (new ZipPackageFolderEnumeration(aContents));
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
    return aContents.size() > 0;
}
    // XNameAccess
Any SAL_CALL ZipPackageFolder::getByName( const OUString& aName )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    Any aAny;
    OUString sName;
    if (aName.indexOf('/', 0 ) == 0)
        sName = aName.copy(1, aName.getLength());
    else
        sName = aName;
    TunnelHash::const_iterator aCI = aContents.find(sName);

    if (aCI == aContents.end())
        throw NoSuchElementException();

    aAny <<= (*aCI).second;
    return aAny;
}
Sequence< OUString > SAL_CALL ZipPackageFolder::getElementNames(  )
        throw(RuntimeException)
{
    sal_uInt32 i=0, nSize = aContents.size();
    OUString *pNames = new OUString[nSize];
    for (TunnelHash::const_iterator aIterator = aContents.begin() ; aIterator != aContents.end(); i++,aIterator++)
        pNames[i] = (*aIterator).first;
    return Sequence < OUString > (pNames, nSize);
}
sal_Bool SAL_CALL ZipPackageFolder::hasByName( const OUString& aName )
        throw(RuntimeException)
{
    OUString sName;
    if (aName.indexOf('/', 0 ) == 0)
        sName = aName.copy(1, aName.getLength());
    else
        sName = aName;
    return aContents.count(sName);
}
    // XNameReplace
void SAL_CALL ZipPackageFolder::replaceByName( const OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    OUString sName;
    if (aName.indexOf('/', 0 ) == 0)
        sName = aName.copy(1, aName.getLength());
    else
        sName = aName;
    if (hasByName(aName))
        removeByName(aName);
    else
        throw NoSuchElementException();
    insertByName(aName, aElement);
}
void ZipPackageFolder::saveContents(OUString &rPath, std::vector < Sequence < PropertyValue > > &rManList, ZipOutputStream & rZipOut)
    throw(RuntimeException)
{
    Reference < XUnoTunnel > xTunnel;
    ZipPackageFolder *pFolder = NULL;
    ZipPackageStream *pStream = NULL;
    TunnelHash::const_iterator aCI = aContents.begin(), aEnd = aContents.end();
    const OUString sMediaType ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) );
    const OUString sFullPath ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );
    const OUString sInitialisationVector ( RTL_CONSTASCII_USTRINGPARAM ( "InitialisationVector" ) );
    const OUString sSalt ( RTL_CONSTASCII_USTRINGPARAM ( "Salt" ) );
    const OUString sIterationCount ( RTL_CONSTASCII_USTRINGPARAM ( "IterationCount" ) );

    // Get a random number generator and seed it with current timestamp
    TimeValue aTime;
    osl_getSystemTime( &aTime );
    rtlRandomPool aRandomPool = rtl_random_createPool ();
    rtl_random_addBytes ( aRandomPool, &aTime, 8 );

    for (; aCI!=aEnd ; aCI++)
    {
        xTunnel = Reference < XUnoTunnel> ((*aCI).second, UNO_QUERY);
        sal_Int64 nTest = 0;
        Sequence < PropertyValue > aPropSet (2);
        ZipEntry * pTempEntry = new ZipEntry;
        pFolder = NULL;
        pStream = NULL;
        sal_Bool bIsFolder = sal_True;

        if ((nTest = xTunnel->getSomething(ZipPackageFolder::getUnoTunnelImplementationId())) != 0)
            pFolder = reinterpret_cast < ZipPackageFolder* > ( nTest );
        else
        {
            // If this getSomething call returns 0, it means that
            // something evil has crept into the contents hash_map, which
            // should mean that something has gone very wrong somewhere, and someone
            // else should deal with it

            pStream = reinterpret_cast < ZipPackageStream *> ( xTunnel->getSomething ( ZipPackageStream::getUnoTunnelImplementationId() ) );
            if (!pStream)
                throw RuntimeException();
            bIsFolder = sal_False;
        }
        if (bIsFolder)
        {
            // In case the entry we are reading is also the entry we are writing, we will
            // store the ZipEntry data in pTempEntry
            ZipPackageFolder::copyZipEntry ( *pTempEntry, pFolder->aEntry );
            pTempEntry->sName = rPath + (*aCI).first + OUString( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );
            pTempEntry->nTime = ZipOutputStream::getCurrentDosTime();
            pTempEntry->nCrc = 0;
            pTempEntry->nSize = 0;
            pTempEntry->nCompressedSize = 0;
            pTempEntry->nMethod = STORED;

            try
            {
                vos::ORef < EncryptionData > xEmpty;
                rZipOut.putNextEntry(*pTempEntry, xEmpty);
                rZipOut.closeEntry();
            }
            catch (IOException & )
            {
                VOS_ENSURE( 0, "Error writing ZipOutputStream" );
            }

            aPropSet[0].Name = sMediaType;
            aPropSet[0].Value <<= pFolder->GetMediaType();
            aPropSet[1].Name = sFullPath;
            aPropSet[1].Value <<= pFolder->aEntry.sName;

            // Copy back the zip entry and make the offset negative so that we
            // know it's point at the beginning of the LOC
            ZipPackageFolder::copyZipEntry ( pFolder->aEntry, *pTempEntry );
            pFolder->aEntry.nOffset *= -1;
            pFolder->saveContents(pFolder->aEntry.sName, rManList, rZipOut);
            pFolder->aEntry.sName = (*aCI).first;
        }
        else
        {
            // In case the entry we are reading is also the entry we are writing, we will
            // store the ZipEntry data in pTempEntry

            ZipPackageFolder::copyZipEntry ( *pTempEntry, pStream->aEntry );
            pTempEntry->sName = rPath + (*aCI).first;
            sal_Bool bToBeCompressed = pStream->IsToBeCompressed();
            sal_Bool bToBeEncrypted = pStream->IsToBeEncrypted();

            aPropSet[0].Name = sMediaType;
            aPropSet[0].Value <<= pStream->GetMediaType( );
            aPropSet[1].Name = sFullPath;
            aPropSet[1].Value <<= pStream->aEntry.sName;

            if ( bToBeEncrypted)
            {
                Sequence < sal_Int8 > aSalt ( 16 ), aVector ( 8 );
                rtl_random_getBytes ( aRandomPool, aSalt.getArray(), 16 );
                rtl_random_getBytes ( aRandomPool, aVector.getArray(), 8 );
                sal_Int64 nIterationCount = 1024;

                pStream->setInitialisationVector ( aVector );
                pStream->setSalt ( aSalt );
                pStream->setIterationCount ( nIterationCount );

                aPropSet.realloc(5);
                aPropSet[2].Name = sInitialisationVector;
                aPropSet[2].Value <<= aVector;
                aPropSet[3].Name = sSalt;
                aPropSet[3].Value <<= aSalt;
                aPropSet[4].Name = sIterationCount;
                aPropSet[4].Value <<= nIterationCount;
            }

            // If the entry is already stored in the zip file in the format we
            // want for this write...copy it raw
            if (pStream->IsPackageMember() &&
                ( (pTempEntry->nMethod == DEFLATED && bToBeCompressed) ||
                  (pTempEntry->nMethod == STORED && !bToBeCompressed) ) )
            {
                try
                {
                    Reference < XInputStream > xStream = pStream->getRawStream( *pTempEntry );
                    try
                    {
                        rZipOut.putNextEntry ( *pTempEntry, pStream->getEncryptionData(), bToBeEncrypted );
                        while (1)
                        {
                            Sequence < sal_Int8 > aSeq (65535);
                            sal_Int32 nLength;
                            nLength = xStream->readBytes(aSeq, 65535);
                            if (nLength < 65535)
                                aSeq.realloc(nLength);
                            rZipOut.rawWrite(aSeq);
                            if (nLength < 65535) // EOF
                                break;
                        }
                        rZipOut.rawCloseEntry();
                    }
                    catch (ZipException&)
                    {
                    }
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
                sal_Bool bTrackLength = sal_True;

                pTempEntry->nCrc = -1;
                pTempEntry->nSize = -1;
                pTempEntry->nCompressedSize = -1;
                pTempEntry->nMethod = bToBeCompressed ? DEFLATED : STORED;

                if (xSeek.is())
                {
                    xSeek->seek(0);
                    if (pTempEntry->nMethod == STORED)
                    {
                        pTempEntry->nSize = pTempEntry->nCompressedSize = static_cast < sal_Int32 > (xSeek->getLength());
                        bTrackLength = sal_False;
                    }
                    // Some implementations of XInputStream/XSeekable change the current position
                    // in the stream! Evil evil bad bad!
                    xSeek->seek(0);
                }

                try
                {
                    rZipOut.putNextEntry ( *pTempEntry, pStream->getEncryptionData(), bToBeEncrypted );
                    while (1)
                    {
                        Sequence < sal_Int8 > aSeq (65535);
                        sal_Int32 nLength;
                        nLength = xStream->readBytes(aSeq, 65535);
                        if (nLength < 65535)
                            aSeq.realloc(nLength);
                        rZipOut.write(aSeq, 0, nLength);
                        if (bTrackLength)
                            pTempEntry->nSize+=nLength;
                        if (nLength < 65535) // EOF
                            break;
                    }
                    if (bTrackLength)
                        pTempEntry->nCompressedSize = pStream->aEntry.nSize;
                    pStream->SetPackageMember ( sal_True );
                    rZipOut.closeEntry();
                }
                catch (IOException & )
                {
                    VOS_ENSURE( 0, "Error writing ZipOutputStream" );
                }
            }


            // Then copy it back afterwards...
            ZipPackageFolder::copyZipEntry ( pStream->aEntry, *pTempEntry );
            pStream->aEntry.sName = (*aCI).first;
            pStream->aEntry.nOffset *= -1;
        }
        rManList.push_back (aPropSet);
    }
}

void ZipPackageFolder::releaseUpwardRef( void )
{
    Reference < XUnoTunnel > xTunnel;
    ZipPackageFolder *pFolder = NULL;
    ZipPackageStream *pStream = NULL;
    sal_Bool bIsFolder;
    TunnelHash::const_iterator aCI = aContents.begin();

    for (;aCI!=aContents.end();aCI++)
    {
        xTunnel = Reference < XUnoTunnel> ((*aCI).second, UNO_QUERY);
        sal_Int64 nTest=0;
        if ((nTest = xTunnel->getSomething(ZipPackageFolder::getUnoTunnelImplementationId())) != 0)
        {
            pFolder = reinterpret_cast < ZipPackageFolder* > ( nTest );
            bIsFolder = sal_True;
        }
        else
        {
            nTest = xTunnel->getSomething(ZipPackageStream::getUnoTunnelImplementationId());
            pStream = reinterpret_cast < ZipPackageStream* > ( nTest );
            bIsFolder = sal_False;
        }

        if (bIsFolder)
        {
            pFolder->releaseUpwardRef();
            pFolder->clearParent();
        }
        else
            pStream->clearParent();
    }
}

Sequence< sal_Int8 > ZipPackageFolder::getUnoTunnelImplementationId( void )
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

sal_Int64 SAL_CALL ZipPackageFolder::getSomething( const Sequence< sal_Int8 >& aIdentifier )
    throw(RuntimeException)
{
    if (aIdentifier.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  aIdentifier.getConstArray(), 16 ) )
        return reinterpret_cast < sal_Int64 > ( this );

    return 0;
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
    {
        aAny <<= sMediaType;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Size" ) ) )
    {
        aAny <<= aEntry.nSize;
        return aAny;
    }
    else
        throw UnknownPropertyException();
}
