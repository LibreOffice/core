/*************************************************************************
 *
 *  $RCSfile: ZipPackageFolder.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: mtg $ $Date: 2001-03-07 16:09:44 $
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
#include "ZipPackageFolder.hxx"
#endif

using namespace com::sun::star::package::ZipConstants;
using namespace com::sun::star;
using namespace cppu;
using namespace rtl;

ZipPackageFolder::ZipPackageFolder (void)
: pPackage( NULL )
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
void ZipPackageFolder::copyZipEntry( com::sun::star::package::ZipEntry &rDest, const com::sun::star::package::ZipEntry &rSource)
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
uno::Any SAL_CALL ZipPackageFolder::queryInterface( const uno::Type& rType )
    throw(uno::RuntimeException)
{
    // cppu::queryInterface is an inline template so it's fast
    // unfortunately, it always creates an Any...we should be able to optimise
    // this with a class static containing supported interfaces
    // ...will research this further ...mtg 15/12/00
    return ::cppu::queryInterface ( rType                                       ,
                                        // OWeakObject interfaces
                                        reinterpret_cast< uno::XInterface*      > ( this )  ,
                                        static_cast< uno::XWeak*            > ( this )  ,
                                        // ZipPackageEntry interfaces
                                        static_cast< container::XNamed*     > ( this )  ,
                                        static_cast< container::XChild*     > ( this )  ,
                                        static_cast< lang::XUnoTunnel*      > ( this )  ,
                                        // my own interfaces
                                        static_cast< container::XNameContainer*     > ( this )  ,
                                        static_cast< container::XEnumerationAccess*     > ( this )  ,
                                        static_cast< beans::XPropertySet*   > ( this ) );

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
void SAL_CALL ZipPackageFolder::insertByName( const ::rtl::OUString& aName, const uno::Any& aElement )
        throw(lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException)
{
    OUString sName;
    if (aName.indexOf('/', 0 ) == 0)
        sName = aName.copy(1, aName.getLength());
    else
        sName = aName;

    if (hasByName(sName))
        throw container::ElementExistException();
    else
    {
        uno::Reference < lang::XUnoTunnel > xRef;
        aElement >>= xRef;
        uno::Reference < container::XNamed > xNamed (xRef, uno::UNO_QUERY);
        uno::Reference < container::XChild > xChild (xRef, uno::UNO_QUERY);
        uno::Reference < uno::XInterface > xInterface (*this);
        xNamed->setName (sName);
        aContents[sName] = xRef;
        try
        {
            xChild->setParent (xInterface);
        }
        catch ( lang::NoSupportException& )
        {
            VOS_ENSURE( 0, "setParent threw an exception: attempted to set Parent to non-existing interface!");
        }
    }
}
void SAL_CALL ZipPackageFolder::removeByName( const ::rtl::OUString& Name )
        throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    OUString sName;
    if (Name.indexOf('/', 0 ) == 0)
        sName = Name.copy(1, Name.getLength());
    else
        sName = Name;
    if (!aContents.count(sName))
        throw container::NoSuchElementException();
    aContents.erase(sName);
}
    // XEnumerationAccess
uno::Reference< container::XEnumeration > SAL_CALL ZipPackageFolder::createEnumeration(  )
        throw(uno::RuntimeException)
{
    return uno::Reference < container::XEnumeration> (new ZipPackageFolderEnumeration(aContents));
}
    // XElementAccess
uno::Type SAL_CALL ZipPackageFolder::getElementType(  )
        throw(uno::RuntimeException)
{
    return ::getCppuType ((const uno::Reference< lang::XUnoTunnel > *) 0);
}
sal_Bool SAL_CALL ZipPackageFolder::hasElements(  )
        throw(uno::RuntimeException)
{
    return aContents.size() > 0;
}
    // XNameAccess
uno::Any SAL_CALL ZipPackageFolder::getByName( const ::rtl::OUString& aName )
        throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aAny;
    OUString sName;
    if (aName.indexOf('/', 0 ) == 0)
        sName = aName.copy(1, aName.getLength());
    else
        sName = aName;
    if (!aContents.count(sName))
        throw container::NoSuchElementException();

    TunnelHash::const_iterator aCI = aContents.find(sName);
    aAny <<= (*aCI).second;
    return aAny;
}
uno::Sequence< ::rtl::OUString > SAL_CALL ZipPackageFolder::getElementNames(  )
        throw(uno::RuntimeException)
{
    sal_uInt32 i=0, nSize = aContents.size();
    OUString *pNames = new OUString[nSize];
    for (TunnelHash::const_iterator aIterator = aContents.begin() ; aIterator != aContents.end(); i++,aIterator++)
        pNames[i] = (*aIterator).first;
    return uno::Sequence < OUString > (pNames, nSize);
}
sal_Bool SAL_CALL ZipPackageFolder::hasByName( const ::rtl::OUString& aName )
        throw(uno::RuntimeException)
{
    OUString sName;
    if (aName.indexOf('/', 0 ) == 0)
        sName = aName.copy(1, aName.getLength());
    else
        sName = aName;
    return aContents.count(sName);
}
    // XNameReplace
void SAL_CALL ZipPackageFolder::replaceByName( const ::rtl::OUString& aName, const uno::Any& aElement )
        throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    OUString sName;
    if (aName.indexOf('/', 0 ) == 0)
        sName = aName.copy(1, aName.getLength());
    else
        sName = aName;
    if (hasByName(aName))
        removeByName(aName);
    else
        throw container::NoSuchElementException();
    insertByName(aName, aElement);
}
void ZipPackageFolder::saveContents(rtl::OUString &rPath, std::vector < ManifestEntry *> &rManList, ZipOutputStream & rZipOut)
    throw(uno::RuntimeException)
{
    uno::Reference < lang::XUnoTunnel > xTunnel;
    ZipPackageFolder *pFolder = NULL;
    ZipPackageStream *pStream = NULL;
    sal_Bool bIsFolder = sal_False;
    TunnelHash::const_iterator aCI = aContents.begin();

    for (;aCI!=aContents.end();aCI++)
    {
        xTunnel = uno::Reference < lang::XUnoTunnel> ((*aCI).second, uno::UNO_QUERY);
        sal_Int64 nTest = 0;
        pFolder = NULL;
        pStream = NULL;

        if ((nTest = xTunnel->getSomething(ZipPackageFolder::getUnoTunnelImplementationId())) != 0)
        {
            pFolder = reinterpret_cast < ZipPackageFolder* > ( nTest );
            bIsFolder = sal_True;
        }
        else
        {
            // If this getSomething call returns 0, it means that
            // something evil has crept into the contents hash_map, which
            // should mean that something has gone very wrong somewhere, and someone
            // else should deal with it

            nTest = xTunnel->getSomething(ZipPackageStream::getUnoTunnelImplementationId());
            if (nTest == 0)
                throw uno::RuntimeException();
            pStream = reinterpret_cast < ZipPackageStream* > ( nTest );
            bIsFolder = sal_False;
        }

        if (bIsFolder)
        {
            if (pFolder->pPackage)
            {
                // This ZipPackageFolder is the root folder of a zipfile contained within
                // this zipfile. Things will get a little strange from here on in...
                ManifestEntry *pMan = new ManifestEntry;
                pMan->sShortName = (*aCI).first;

                ZipPackageFolder::copyZipEntry(pMan->aEntry, pFolder->aEntry);
                pMan->aEntry.sName = rPath + pMan->sShortName;

                pMan->aEntry.nCrc = -1;
                pMan->aEntry.nSize = -1;
                pMan->aEntry.nCompressedSize = -1;
                pMan->aEntry.nMethod = STORED;

                try
                {
                    rZipOut.putNextEntry(pMan->aEntry);
                    ZipPackageBuffer &rBuffer = pFolder->pPackage->writeToBuffer();
                    pMan->aEntry.nSize = pMan->aEntry.nCompressedSize = static_cast < sal_Int32 > (rBuffer.getLength());
                    rZipOut.write(rBuffer.aBuffer, 0, static_cast < sal_Int32 > (rBuffer.getLength()));
                    rZipOut.closeEntry();
                }
                catch (::com::sun::star::io::IOException & )
                {
                    VOS_ENSURE( 0, "Error writing ZipOutputStream" );
                }
                // Then copy it back
                try
                {
                    uno::Any aAny = pFolder->getPropertyValue(OUString( RTL_CONSTASCII_USTRINGPARAM ("MediaType") ) );
                    aAny >>= pMan->sMediaType;
                }
                catch (::com::sun::star::beans::UnknownPropertyException & )
                {
                    VOS_ENSURE( 0, "MediaType is an unknown property!!" );
                }
                ZipPackageFolder::copyZipEntry(pFolder->aEntry, pMan->aEntry);
                pFolder->aEntry.nOffset *= -1;
                rManList.push_back (pMan);
            }
            else
            {
                // In case the entry we are reading is also the entry we are writing, we will
                // store the ZipEntry data in the ManifestEntry struct and then update the
                // ZipEntry data in the ZipPackageFolder later

                ManifestEntry *pMan = new ManifestEntry;
                pMan->sShortName = (*aCI).first;

                // First copy current data to ManifestEntry
                ZipPackageFolder::copyZipEntry(pMan->aEntry, pFolder->aEntry);

                pMan->aEntry.sName = rPath + pMan->sShortName + OUString( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );
                pMan->aEntry.nTime = ZipOutputStream::getCurrentDosTime();
                pMan->aEntry.nCrc = 0;
                pMan->aEntry.nSize = 0;
                pMan->aEntry.nCompressedSize = 0;
                pMan->aEntry.nMethod = STORED;

                try
                {
                    rZipOut.putNextEntry(pMan->aEntry);
                    rZipOut.closeEntry();
                }
                catch (::com::sun::star::io::IOException & )
                {
                    VOS_ENSURE( 0, "Error writing ZipOutputStream" );
                }
                try
                {
                    uno::Any aAny = pFolder->getPropertyValue(OUString( RTL_CONSTASCII_USTRINGPARAM ( "MediaType") ) );
                    aAny >>= pMan->sMediaType;
                }
                catch (::com::sun::star::beans::UnknownPropertyException & )
                {
                    VOS_ENSURE( 0, "MediaType is an unknown property!!" );
                }

                // Then copy it back
                ZipPackageFolder::copyZipEntry(pFolder->aEntry, pMan->aEntry);
                pFolder->aEntry.nOffset *= -1;
                pFolder->saveContents(pFolder->aEntry.sName, rManList, rZipOut);
                rManList.push_back (pMan);
            }
        }
        else
        {
            // In case the entry we are reading is also the entry we are writing, we will
            // store the ZipEntry data in the ManifestEntry struct and then update the
            // ZipEntry data in the ZipPackageStream later

            ManifestEntry *pMan = new ManifestEntry;
            pMan->sShortName = (*aCI).first;
#if SUPD>617
            uno::Any aAny = pStream->getPropertyValue(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Compressed") ) );
#else
            uno::Any aAny = pStream->getPropertyValue(OUString( RTL_CONSTASCII_USTRINGPARAM ( "Compress") ) );
#endif
            sal_Bool bToBeCompressed;
            aAny >>= bToBeCompressed;

            // Copy current info to pMan...
            ZipPackageFolder::copyZipEntry(pMan->aEntry, pStream->aEntry);
            pMan->aEntry.sName = rPath + pMan->sShortName;

            // If the entry is already stored in the zip file in the format we
            // want for this write...copy it raw
            if (pStream->bPackageMember &&
                ( (pMan->aEntry.nMethod == DEFLATED && bToBeCompressed) ||
                  (pMan->aEntry.nMethod == STORED && !bToBeCompressed) ) )
            {
                try
                {
                    uno::Reference < io::XInputStream > xStream = pStream->getRawStream( pMan->aEntry );
                    try
                    {
                        rZipOut.putNextEntry(pMan->aEntry);
                        while (1)
                        {
                            uno::Sequence < sal_Int8 > aSeq (65535);
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
                    catch (package::ZipException&)
                    {
                    }
                }
                catch (::com::sun::star::io::IOException & )
                {
                    VOS_ENSURE( 0, "Error writing ZipOutputStream" );
                }
            }
            else
            {
                uno::Reference < io::XInputStream > xStream = pStream->getInputStream();
                uno::Reference < io::XSeekable > xSeek (xStream, uno::UNO_QUERY);
                sal_Bool bTrackLength = sal_True;

                pMan->aEntry.nCrc = -1;
                pMan->aEntry.nSize = -1;
                pMan->aEntry.nCompressedSize = -1;

                if (bToBeCompressed)
                    pMan->aEntry.nMethod = DEFLATED;
                else
                    pMan->aEntry.nMethod = STORED;

                if (xSeek.is())
                {
                    xSeek->seek(0);
                    if (pMan->aEntry.nMethod == STORED)
                    {
                        pMan->aEntry.nSize = pMan->aEntry.nCompressedSize = static_cast < sal_Int32 > (xSeek->getLength());
                        bTrackLength = sal_False;
                    }
                    // Some implementations of XInputStream/XSeekable change the current position
                    // in the stream! Evil evil bad bad!
                    xSeek->seek(0);
                }

                try
                {
                    rZipOut.putNextEntry(pMan->aEntry);
                    while (1)
                    {
                        uno::Sequence < sal_Int8 > aSeq (65535);
                        sal_Int32 nLength;
                        nLength = xStream->readBytes(aSeq, 65535);
                        if (nLength < 65535)
                            aSeq.realloc(nLength);
                        rZipOut.write(aSeq, 0, nLength);
                        if (bTrackLength)
                            pMan->aEntry.nSize+=nLength;
                        if (nLength < 65535) // EOF
                            break;
                    }
                    if (bTrackLength)
                        pMan->aEntry.nCompressedSize = pStream->aEntry.nSize;
                    pStream->bPackageMember = sal_True;
                    rZipOut.closeEntry();
                }
                catch (::com::sun::star::io::IOException & )
                {
                    VOS_ENSURE( 0, "Error writing ZipOutputStream" );
                }
            }

            try
            {
                uno::Any aAny = pStream->getPropertyValue(OUString( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) ) );
                aAny >>= pMan->sMediaType;
            }
            catch (::com::sun::star::beans::UnknownPropertyException & )
            {
                VOS_ENSURE( 0, "MediaType is an unknown property!!" );
            }
            // Then copy it back afterwards...
            ZipPackageFolder::copyZipEntry(pStream->aEntry, pMan->aEntry);
            pStream->aEntry.nOffset *= -1;
            rManList.push_back (pMan);
        }
    }
}

void ZipPackageFolder::releaseUpwardRef( void )
{
    uno::Reference < lang::XUnoTunnel > xTunnel;
    ZipPackageFolder *pFolder = NULL;
    ZipPackageStream *pStream = NULL;
    sal_Bool bIsFolder;
    TunnelHash::const_iterator aCI = aContents.begin();

    for (;aCI!=aContents.end();aCI++)
    {
        xTunnel = uno::Reference < lang::XUnoTunnel> ((*aCI).second, uno::UNO_QUERY);
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
void ZipPackageFolder::updateReferences( ZipFile * pNewZipFile)
{
    uno::Reference < lang::XUnoTunnel > xTunnel;
    ZipPackageFolder *pFolder = NULL;
    ZipPackageStream *pStream = NULL;
    sal_Bool bIsFolder;
    TunnelHash::const_iterator aCI = aContents.begin();

    for (;aCI!=aContents.end();aCI++)
    {
        xTunnel = uno::Reference < lang::XUnoTunnel> ((*aCI).second, uno::UNO_QUERY);
        sal_Int64 nTest=0;
        if ((nTest = xTunnel->getSomething(ZipPackageFolder::getUnoTunnelImplementationId())) != 0)
        {
            pFolder = reinterpret_cast < ZipPackageFolder* > ( nTest );
            bIsFolder = sal_True;
        }
        else
        {
            // If this getSomething call returns 0, it means that
            // something evil has crept into the contents hash_map, which
            // should mean that something has gone very wrong somewhere, and someone
            // else should deal with it

            nTest = xTunnel->getSomething(ZipPackageStream::getUnoTunnelImplementationId());
            if (nTest == 0)
                throw uno::RuntimeException();
            pStream = reinterpret_cast < ZipPackageStream* > ( nTest );
            bIsFolder = sal_False;
        }

        if (bIsFolder)
        {
            //if pPackage is set,then this is the root folder of a different ZipPackage and
            // should not be changed
            if (!pFolder->pPackage)
                pFolder->updateReferences(pNewZipFile);
        }
        else
            pStream->pZipFile = pNewZipFile;
    }
}

uno::Sequence< sal_Int8 > ZipPackageFolder::getUnoTunnelImplementationId( void )
    throw (uno::RuntimeException)
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

sal_Int64 SAL_CALL ZipPackageFolder::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
    throw(uno::RuntimeException)
{
    if (aIdentifier.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  aIdentifier.getConstArray(), 16 ) )
        return reinterpret_cast < sal_Int64 > ( this );

    return 0;
}
