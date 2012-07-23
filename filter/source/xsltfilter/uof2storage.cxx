/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 **************************************************************/

 // MARKER(update_precomp.py): autogen include statement, do not remove
//This file is about the conversion of the UOF v2.0 and ODF document format
#include "precompiled_filter.hxx"


#include "uof2storage.hxx"

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <comphelper/storagehelper.hxx>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>

namespace XSLT{

namespace{

void lclSplitFirstElement( ::rtl::OUString& orElement, ::rtl::OUString& orRemainder, const ::rtl::OUString& rFullName )
{
    sal_Int32 nSlashPos = rFullName.indexOf('/');
    if((0 <= nSlashPos ) && (nSlashPos < rFullName.getLength()))
    {
        orElement = rFullName.copy(0, nSlashPos);
        orRemainder = rFullName.copy(nSlashPos+1);
    }
    else
    {
        orElement = rFullName;
    }
}

}

StorageBase::StorageBase( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream,
                         bool bBaseStreamAccess )
: m_xInStream(rxInStream)
, m_bBaseStreamAccess(bBaseStreamAccess)
, m_bReadOnly(true)
{
    OSL_ENSURE(m_xInStream.is(), "StorageBase::StorageBase - missing base input stream");
}

StorageBase::StorageBase( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxOutStream, bool bBaseStreamAccess )
: m_xOutStream(rxOutStream)
, m_bBaseStreamAccess(bBaseStreamAccess)
, m_bReadOnly(false)
{
    OSL_ENSURE(m_xOutStream.is(), "StorageBase::StorageBase - missing base output stream");
}

StorageBase::StorageBase( const StorageBase& rParentStorage, const ::rtl::OUString& rStorageName, bool bReadOnly )
: m_aParentPath(rParentStorage.getPath())
, m_aStorageName(rStorageName)
, m_bBaseStreamAccess(false)
, m_bReadOnly(bReadOnly)
{
}

StorageBase::~StorageBase()
{
}

bool StorageBase::isStorage() const
{
    return implIsStorage();
}

bool StorageBase::isRootStorage() const
{
    return implIsStorage() && (m_aStorageName.getLength() == 0);
}

bool StorageBase::isReadOnly() const
{
    return m_bReadOnly;
}

::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > StorageBase::getXStorage() const
{
    return implGetXStorage();
}

const ::rtl::OUString& StorageBase::getName() const
{
    return m_aStorageName;
}

::rtl::OUString StorageBase::getPath() const
{
    ::rtl::OUStringBuffer aBuffer(m_aParentPath);
    if(aBuffer.getLength() > 0)
    {
        aBuffer.append(sal_Unicode('/'));
    }

    aBuffer.append(m_aStorageName);
    return aBuffer.makeStringAndClear();
}

void StorageBase::getElementNames( ::std::vector< ::rtl::OUString >& orElementNames ) const
{
    orElementNames.clear();
    implGetElementNames(orElementNames);
}

XSLT::StorageRef StorageBase::openSubStorage( const ::rtl::OUString& rStorageName, bool bCreatedMissing )
{
    StorageRef xSubStorage;
    OSL_ENSURE(!bCreatedMissing || !m_bReadOnly, "StorageBase::openSubStorage - can not create substorage in read-only mode");
    if(!bCreatedMissing || !m_bReadOnly)
    {
        ::rtl::OUString aElement, aRemainder;
        lclSplitFirstElement(aElement, aRemainder, rStorageName);
        if(aElement.getLength() > 0)
            xSubStorage = getSubStorage(aElement, bCreatedMissing);
        if(xSubStorage.get() && (aRemainder.getLength() > 0))
            xSubStorage = xSubStorage->openSubStorage(aRemainder, bCreatedMissing);
    }

    return xSubStorage;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > StorageBase::openInputStream( const ::rtl::OUString& rStreamName )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInStream;
    ::rtl::OUString aElement, aRemainder;
    lclSplitFirstElement(aElement, aRemainder, rStreamName);
    if(aElement.getLength() > 0)
    {
        if(aRemainder.getLength() > 0)
        {
            StorageRef xSubStorage = getSubStorage(aElement, false);
            if(xSubStorage.get())
                xInStream = xSubStorage->openInputStream(aRemainder);
        }
        else
        {
            xInStream = implOpenInputStream(aElement);
        }
    }
    else if(m_bBaseStreamAccess)
    {
        xInStream = m_xInStream;
    }

    return xInStream;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > StorageBase::openOutputStream( const ::rtl::OUString& rStreamName )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xOutStream;
    OSL_ENSURE(!m_bReadOnly, "StorageBase::openOutputStream - can not create output stream in read-only mode");
    if(!m_bReadOnly)
    {
        ::rtl::OUString aElement, aRemainder;
        lclSplitFirstElement(aElement, aRemainder, rStreamName);
        if(aElement.getLength() > 0)
        {
            if(aRemainder.getLength() > 0)
            {
                StorageRef xSubStorage = getSubStorage(aElement, true);
                if(xSubStorage.get())
                    xOutStream = xSubStorage->openOutputStream(aRemainder);
            }
            else
            {
                xOutStream = implOpenOutputStream(aElement);
            }
        }
        else if(m_bBaseStreamAccess)
        {
            xOutStream = m_xOutStream->getOutputStream();
        }
    }

    return xOutStream;
}

void StorageBase::copyToStorage( StorageBase& rDestStrg, const ::rtl::OUString& rElementName )
{
    OSL_ENSURE(rDestStrg.isStorage() && !rDestStrg.isReadOnly(), "StorageBase::copyToStorage - invalid destination");
    OSL_ENSURE(rElementName.getLength() > 0, "StorageBase::copyToStorage - invalid element name");
    if(rDestStrg.isStorage() && !rDestStrg.isReadOnly() && (rElementName.getLength() > 0))
    {
        StorageRef xSubStrg = openSubStorage(rElementName, false);
        if(xSubStrg.get())
        {
            StorageRef xDestSubStrg = rDestStrg.openSubStorage(rElementName, true);
            if(xDestSubStrg.get())
                xSubStrg->copyStorageToStorage(*xDestSubStrg);
        }
        else
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInStrm = openInputStream(rElementName);
            if(xInStrm.get())
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xOutStm = rDestStrg.openOutputStream(rElementName);
                if(xOutStm.is())
                {
                    /*BinaryXInputStream aInStrm(xInStrm, true);
                    BinaryXOutputStream aOutStrm(xOutStm, true);
                    aInStrm.copyToStream(aOutStrm);*/
                }
            }
        }
    }
}

void StorageBase::copyStorageToStorage( StorageBase& rDestStrg )
{
    OSL_ENSURE(rDestStrg.isStorage() && !rDestStrg.isReadOnly(), "StorageBase::copyStorageToStorage - invalid destination");
    if(rDestStrg.isStorage() && !rDestStrg.isReadOnly())
    {
        ::std::vector< ::rtl::OUString > aElements;
        getElementNames(aElements);
        for( ::std::vector< ::rtl::OUString >::iterator aIter = aElements.begin();
            aIter != aElements.end(); ++aIter)
            copyToStorage(rDestStrg, *aIter);
    }
}

void StorageBase::commit()
{
    OSL_ENSURE(!m_bReadOnly, "StorageBase::commit - can not commit in read-only mode");
    if(!m_bReadOnly)
    {
        m_aSubStorages.forEachMem(&StorageBase::commit);
        implCommit();
    }
}

XSLT::StorageRef StorageBase::getSubStorage( const ::rtl::OUString& rElementName, bool bCreateMissing )
{
    StorageRef& rxSubStrg = m_aSubStorages[rElementName];
    if(!rxSubStrg)
        rxSubStrg = implOpenSubStorage(rElementName, bCreateMissing);
    return rxSubStrg;
}

ZipStorage::ZipStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream )
: StorageBase(rxInStream, false)
{
    OSL_ENSURE(rxFactory.is(), "ZipStorage::ZipStorage - missing service factory");
    //create base storage object
    try
    {
        m_xStorage = ::comphelper::OStorageHelper::GetStorageOfFormatFromInputStream(
            ZIP_STORAGE_FORMAT_STRING, rxInStream, rxFactory, sal_True);
    }
    catch( ::com::sun::star::uno::Exception&)
    {
    }
}

ZipStorage::ZipStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxStream )
: StorageBase(rxStream, false)
{
    OSL_ENSURE(rxFactory.is(), "ZipStorage::ZipStorage - missing service factory");
    try
    {
        using namespace ::com::sun::star::embed::ElementModes;
        m_xStorage = ::comphelper::OStorageHelper::GetStorageOfFormatFromStream(
            ZIP_STORAGE_FORMAT_STRING, rxStream, READWRITE | TRUNCATE, rxFactory, sal_True);
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        OSL_ENSURE(false, "ZipStorage::ZipStorage - can not open output storage");
    }
}

ZipStorage::ZipStorage( const ZipStorage& rParentStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage, const ::rtl::OUString& rElementName )
: StorageBase(rParentStorage, rElementName, rParentStorage.isReadOnly())
, m_xStorage(rxStorage)
{
    OSL_ENSURE(m_xStorage.is(), "ZipStorage::ZipStorage - missing storage");
}

ZipStorage::~ZipStorage()
{

}

bool ZipStorage::implIsStorage() const
{
    return m_xStorage.is();
}

::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > ZipStorage::implGetXStorage() const
{
    return m_xStorage;
}

void ZipStorage::implGetElementNames( ::std::vector< ::rtl::OUString >& orElementNames ) const
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aNames;
    if(m_xStorage.is())
    {
        try
        {
            aNames = m_xStorage->getElementNames();
            if(aNames.getLength() > 0)
                orElementNames.insert(orElementNames.end(), aNames.getConstArray(), aNames.getConstArray()+aNames.getLength());
        }
        catch( ::com::sun::star::uno::Exception& )
        {
        }
    }
}

XSLT::StorageRef ZipStorage::implOpenSubStorage( const ::rtl::OUString& rElementName, bool bCreatedMissing )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > xSubXStorage;
    bool bMissing = false;
    if(m_xStorage.is())
    {
        try
        {
            if(m_xStorage->isStorageElement(rElementName))
            {
                xSubXStorage = m_xStorage->openStorageElement(rElementName, ::com::sun::star::embed::ElementModes::READ);
            }
        }
        catch( ::com::sun::star::container::NoSuchElementException& )
        {
            bMissing = true;
        }
        catch( ::com::sun::star::uno::Exception& )
        {
        }
    }

    if(bMissing && bCreatedMissing)
    {
        try
        {
            xSubXStorage = m_xStorage->openStorageElement(rElementName, ::com::sun::star::embed::ElementModes::READWRITE);
        }
        catch( ::com::sun::star::uno::Exception& )
        {
        }
    }

    StorageRef xSubStorage;
    if(xSubXStorage.is())
        xSubStorage.reset(new ZipStorage( *this, xSubXStorage, rElementName ));

    return xSubStorage;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > ZipStorage::implOpenInputStream( const ::rtl::OUString& rElementName )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInStream;
    if(m_xStorage.is())
    {
        try
        {
            xInStream.set(m_xStorage->openStreamElement(
                rElementName, ::com::sun::star::embed::ElementModes::READ), ::com::sun::star::uno::UNO_QUERY);
        }
        catch( ::com::sun::star::uno::Exception& )
        {
        }
    }

    return xInStream;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > ZipStorage::implOpenOutputStream( const ::rtl::OUString& rElementName )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xOutStream;
    if(m_xStorage.is())
    {
        try
        {
            xOutStream.set(m_xStorage->openStreamElement(
                rElementName, ::com::sun::star::embed::ElementModes::READWRITE), ::com::sun::star::uno::UNO_QUERY);
        }
        catch( ::com::sun::star::uno::Exception& )
        {
        }
    }

    return xOutStream;
}

void ZipStorage::implCommit() const
{
    try
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::embed::XTransactedObject > (m_xStorage,
            ::com::sun::star::uno::UNO_QUERY_THROW)->commit();
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }
}


UOF2Storage::UOF2Storage( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream )
: m_pMainStorage(new ZipStorage(rxFactory, rxInStream))
{
}

UOF2Storage::UOF2Storage( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxOutStream )
: m_pMainStorage(new ZipStorage(rxFactory, rxOutStream))
{
}

UOF2Storage::~UOF2Storage()
{
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > UOF2Storage::getMetaInputStream() const
{
    return m_pMainStorage->openInputStream(METAELEMNAME);
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > UOF2Storage::getMetaOutputStream( bool /*bMissingCreate*/ )
{
    return m_pMainStorage->openOutputStream(METAELEMNAME);
}

bool UOF2Storage::isValidUOF2Doc() const
{
    if(!m_pMainStorage->isStorage())
        return false;

    return ((getMetaInputStream().is())
        && (m_pMainStorage->openInputStream(UOFELEMNAME).is())
        && (m_pMainStorage->openInputStream(CONTENTELEMNAME).is()));
}

}
