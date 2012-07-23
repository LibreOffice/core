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
#ifndef FILTER_SOURCE_XSLTFILTER_UOF2STORAGE_HXX
#define FILTER_SOURCE_XSLTFILTER_UOF2STORAGE_HXX

#include "containerhelper.hxx"

#include <vector>

#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star {
    namespace embed { class XStorage; }
    namespace io {
        class XInputStream;
        class XOutputStream;
        class XStream;
    }
    namespace lang{
        class XMultiServiceFactory;
    }
}}}

namespace XSLT{

const ::rtl::OUString METAELEMNAME = ::rtl::OUString::createFromAscii("_meta/meta.xml");
const ::rtl::OUString UOFELEMNAME = ::rtl::OUString::createFromAscii("uof.xml");
const ::rtl::OUString CONTENTELEMNAME = ::rtl::OUString::createFromAscii("content.xml");

class StorageBase;
typedef ::boost::shared_ptr< StorageBase > StorageRef;

class StorageBase
{
public:
    explicit StorageBase(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream,
        bool bBaseStreamAccess);

    explicit StorageBase(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxOutStream,
        bool bBaseStreamAccess);

    virtual ~StorageBase();

    /************************************************************************
    * Returns true, if the object represents a valid storage
    ************************************************************************/
    bool isStorage() const;

    /************************************************************************
    * Returns true, if the object represent the root storage
    ************************************************************************/
    bool isRootStorage()const;

    /************************************************************************
    * Returns true, if the storage operates in read-only mode(based on an
    * input stream
    ************************************************************************/
    bool isReadOnly()const;

    /************************************************************************
    * Returns the com.sun.star.embed.XStorage interface of the current storage
    ************************************************************************/
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        getXStorage() const;

    /** Returns the element name of this storage */
    const ::rtl::OUString& getName() const;

    ::rtl::OUString getPath() const;

    void getElementNames( ::std::vector< ::rtl::OUString >& orElementNames ) const;

    StorageRef openSubStorage( const ::rtl::OUString& rStorageName, bool bCreatedMissing );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
        openInputStream(const ::rtl::OUString& rStreamName);

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
        openOutputStream(const ::rtl::OUString& rStreamName);

    void copyToStorage(StorageBase& rDestStrg, const ::rtl::OUString& rElementName );

    void copyStorageToStorage(StorageBase& rDestStrg);

    void commit();
protected:
    explicit StorageBase( const StorageBase& rParentStorage, const ::rtl::OUString& rStorageName, bool bReadOnly );
private:
    StorageBase( const StorageBase& );
    StorageBase& operator = (const StorageBase& );

    virtual bool implIsStorage() const = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        implGetXStorage() const = 0;

    virtual void implGetElementNames( ::std::vector< ::rtl::OUString >& orElementNames ) const = 0;

    virtual StorageRef implOpenSubStorage( const ::rtl::OUString& rElementName, bool bCreate ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
        implOpenInputStream( const ::rtl::OUString& rElementName ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
        implOpenOutputStream( const ::rtl::OUString& rElementName ) = 0;

    virtual void implCommit() const = 0;

    StorageRef getSubStorage( const ::rtl::OUString& rElementName, bool bCreateMissing );
private:
    typedef RefMap< ::rtl::OUString, StorageBase > SubStorageMap;

    SubStorageMap m_aSubStorages;///Map of direct sub storages.
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xInStream;///Cached base input stream(to keep it alive)
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xOutStream;/// Cached base output stream(to keep it alive)
    ::rtl::OUString m_aParentPath;///Full path of parent storage
    ::rtl::OUString m_aStorageName;///Name of this storage, if it is a substorage
    bool m_bBaseStreamAccess;///True = access base streams with empty stream name.
    bool m_bReadOnly; ///True = storage opened read-only (based on input stream)
};

class ZipStorage : public StorageBase
{
public:
    explicit ZipStorage(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream);

    explicit ZipStorage(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxStream);

    virtual ~ZipStorage();

private:
    explicit ZipStorage(
        const ZipStorage& rParentStorage,
        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage,
        const ::rtl::OUString& rElementName);

    virtual bool implIsStorage() const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        implGetXStorage() const;

    virtual void implGetElementNames( ::std::vector< ::rtl::OUString >& orElementNames ) const;

    virtual StorageRef implOpenSubStorage( const ::rtl::OUString& rElementName, bool bCreatedMissing );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
        implOpenInputStream( const ::rtl::OUString& rElementName );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
        implOpenOutputStream( const ::rtl::OUString& rElementName );

    virtual void implCommit() const;

private:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > XStorageRef;
    XStorageRef m_xStorage;
};

class UOF2Storage
{
public:
    explicit UOF2Storage( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream );

    explicit UOF2Storage( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxOutStream );

    ~UOF2Storage();

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > getMetaInputStream() const;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > getMetaOutputStream(bool bMissingCreate);

    bool isValidUOF2Doc() const;// just for UOF2 document import

    StorageRef getMainStorageRef(){ return m_pMainStorage; }
private:
    StorageRef m_pMainStorage;
};

}
#endif
