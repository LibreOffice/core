/*************************************************************************
 *
 *  $RCSfile: ZipPackageFolder.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: mtg $ $Date: 2000-11-29 05:19:35 $
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
#ifndef _ZIP_PACKAGE_FOLDER_HXX
#include "ZipPackageFolder.hxx"
#endif

using namespace com::sun::star::package::ZipConstants;
using namespace com::sun::star;
using namespace rtl;

ZipPackageFolder::ZipPackageFolder (void) //ZipOutputStream &rStream)//ZipPackage &rInPackage)
//: rZipOut(rStream)
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
uno::Any SAL_CALL ZipPackageFolder::queryInterface( const uno::Type& rType )
    throw(uno::RuntimeException)
{
    // Ask for my own supported interfaces ...
    uno::Any aReturn    ( ::cppu::queryInterface    (   rType                                       ,
                                                static_cast< container::XNamed*     > ( this )  ,
                                                static_cast< container::XChild*     > ( this )  ,
                                                static_cast< container::XNameContainer*     > ( this )  ,
                                                static_cast< container::XEnumerationAccess*     > ( this )  ,
                                                static_cast< lang::XUnoTunnel*      > ( this )  ,
                                                static_cast< beans::XPropertySet*   > ( this ) ) ) ;

    // If searched interface supported by this class ...
    if ( aReturn.hasValue () == sal_True )
    {
        // ... return this information.
        return aReturn ;
    }
    else
    {
        // Else; ... ask baseclass for interfaces!
        return ZipPackageEntry::queryInterface ( rType ) ;
    }
}
void SAL_CALL ZipPackageFolder::acquire(  )
    throw()
{
    ZipPackageEntry::acquire();
}
void SAL_CALL ZipPackageFolder::release(  )
    throw()
{
    ZipPackageEntry::release();
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
        xNamed->setName (sName);
        aContents[sName] = xRef;
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
        throw(container::NoSuchElementException());
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
        throw (container::NoSuchElementException());

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
        throw(container::NoSuchElementException());
    insertByName(aName, aElement);
}
    //XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL ZipPackageFolder::getPropertySetInfo(  )
        throw(uno::RuntimeException)
{
    return uno::Reference < beans::XPropertySetInfo > (NULL);
}
void SAL_CALL ZipPackageFolder::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
        throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (aPropertyName == OUString::createFromAscii("MediaType"))
        aValue >>= sMediaType;
    else if (aPropertyName == OUString::createFromAscii("Size"))
        aValue >>= aEntry.nSize;
    else
        throw beans::UnknownPropertyException();
}
uno::Any SAL_CALL ZipPackageFolder::getPropertyValue( const ::rtl::OUString& PropertyName )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (PropertyName == OUString::createFromAscii("MediaType"))
    {
        uno::Any aAny;
        aAny <<= sMediaType;
        return aAny;
    }
    else if (PropertyName == OUString::createFromAscii("Size"))
    {
        uno::Any aAny;
        aAny <<= aEntry.nSize;
        return aAny;
    }
    else
        throw beans::UnknownPropertyException();
}
void SAL_CALL ZipPackageFolder::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL ZipPackageFolder::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL ZipPackageFolder::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL ZipPackageFolder::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void ZipPackageFolder::saveContents(rtl::OUString &rPath, std::vector < ManifestEntry *> &rManList, ZipOutputStream & rZipOut)
{
    uno::Reference < lang::XUnoTunnel > xTunnel;
    package::ZipEntry *aEntry = NULL;
    ZipPackageFolder *pFolder = NULL;
    ZipPackageStream *pStream = NULL;
    sal_Bool bIsFolder;
    TunnelHash::const_iterator aCI = aContents.begin();

    //rPath = rPath + getName();

    for (;aCI!=aContents.end();aCI++)
    {
        xTunnel = uno::Reference < lang::XUnoTunnel> ((*aCI).second, uno::UNO_QUERY);
        try
        {
            pFolder = reinterpret_cast < ZipPackageFolder* > (xTunnel->getSomething(ZipPackageFolder::getUnoTunnelImplementationId()));
            bIsFolder = sal_True;
        }
        catch (uno::RuntimeException)
        {
            pStream = reinterpret_cast < ZipPackageStream* > (xTunnel->getSomething(ZipPackageStream::getUnoTunnelImplementationId()));
            bIsFolder = sal_False;
        }

        if (bIsFolder)
        {
            time_t nTime = time(NULL);
            ManifestEntry *pMan = new ManifestEntry;
            pMan->sShortName = (*aCI).first;
            pFolder->aEntry.sName = rPath + pMan->sShortName + OUString::createFromAscii("/");
            pFolder->aEntry.nTime = ZipOutputStream::tmDateToDosDate ( *localtime(&nTime));
            pFolder->aEntry.nCrc = 0;
            pFolder->aEntry.nSize = 0;
            pFolder->aEntry.nCompressedSize = 0;
            rZipOut.putNextEntry(pFolder->aEntry);
            rZipOut.closeEntry();
            pMan->sMediaType = OUString::createFromAscii("");
            pMan->pEntry = &(pFolder->aEntry);
            pFolder->saveContents(pFolder->aEntry.sName, rManList, rZipOut);
            //setEntry(pMan->aEntry, pFolder->aEntry);
            rManList.push_back (pMan);
        }
        else
        {
            ManifestEntry *pMan = new ManifestEntry;
            pMan->sShortName = (*aCI).first;
            pStream->aEntry.sName = rPath + pMan->sShortName;

            uno::Reference < io::XInputStream > xStream = pStream->getInputStream();
            uno::Reference < io::XSeekable > xSeek (xStream, uno::UNO_QUERY);
            sal_Bool bTrackLength = sal_True;

            pStream->aEntry.nCrc = -1;
            pStream->aEntry.nSize = -1;
            pStream->aEntry.nCompressedSize = -1;
            if (xSeek.is())
            {
                xSeek->seek(0);
                if (pStream->aEntry.nMethod == STORED)
                {
                    pStream->aEntry.nSize = pStream->aEntry.nCompressedSize = xSeek->getLength();
                    bTrackLength = sal_False;
                }
            }

            rZipOut.putNextEntry(pStream->aEntry);
            while (1)
            {
                uno::Sequence < sal_Int8 > aSeq (65535);
                sal_Int64 nLength;
                nLength = xStream->readBytes(aSeq, 65535);
                if (nLength < 65535)
                    aSeq.realloc(nLength);
                rZipOut.write(aSeq, 0, nLength);
                if (bTrackLength)
                    pStream->aEntry.nSize+=nLength;
                if (nLength < 65535) // EOF
                    break;
            }
            if (bTrackLength)
                pStream->aEntry.nCompressedSize = pStream->aEntry.nSize;
            pStream->bPackageMember = sal_True;
            rZipOut.closeEntry();
            uno::Any aAny = pStream->getPropertyValue(OUString::createFromAscii("MediaType"));
            aAny >>= pMan->sMediaType;
            pMan->pEntry = &(pStream->aEntry);
            //setEntry(pMan->aEntry, pStream->aEntry);
            rManList.push_back (pMan);
        }
    }
}
void ZipPackageFolder::setEntry(package::ZipEntry &rDest, package::ZipEntry &rSrc)
{
    rDest.nVersion = rSrc.nVersion;
    rDest.nFlag = rSrc.nFlag;
    rDest.nMethod = rSrc.nMethod;
    rDest.nTime = rSrc.nTime;
    rDest.nCrc = rSrc.nCrc;
    rDest.nCompressedSize = rSrc.nCompressedSize;
    rDest.nSize = rSrc.nSize;
    rDest.nOffset = rSrc.nOffset;
    rDest.sName = rSrc.sName;
    rDest.extra = rSrc.extra;
    rDest.sComment = rSrc.sComment;
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
        try
        {
            pFolder = reinterpret_cast < ZipPackageFolder* > (xTunnel->getSomething(ZipPackageFolder::getUnoTunnelImplementationId()));
            bIsFolder = sal_True;
        }
        catch (uno::RuntimeException)
        {
            pStream = reinterpret_cast < ZipPackageStream* > (xTunnel->getSomething(ZipPackageStream::getUnoTunnelImplementationId()));
            bIsFolder = sal_False;
        }

        if (bIsFolder)
            pFolder->updateReferences(pNewZipFile);
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
        return reinterpret_cast<sal_Int64>(this);

    throw uno::RuntimeException();
}
