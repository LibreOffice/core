/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVL_SOURCE_PASSWORDCONTAINER_PASSWORDCONTAINER_HXX
#define INCLUDED_SVL_SOURCE_PASSWORDCONTAINER_PASSWORDCONTAINER_HXX

#include <utility>
#include <vector>
#include <map>
#include <mutex>
#include <optional>
#include <com/sun/star/task/XPasswordContainer2.hpp>
#include <com/sun/star/task/PasswordRequestMode.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>

#include <unotools/configitem.hxx>
#include <ucbhelper/interactionrequest.hxx>

#include <rtl/random.h>
#include <rtl/ref.hxx>
#include <osl/mutex.hxx>

#include "syscreds.hxx"

#define MEMORY_RECORD         0
#define PERSISTENT_RECORD     1


class NamePasswordRecord
{
    OUString                    m_aName;

    // there are two lists of passwords, memory passwords and persistent passwords
    bool                        m_bHasMemoryPasswords;
    ::std::vector< OUString >   m_aMemoryPasswords;

    // persistent passwords are encrypted in one string
    bool                        m_bHasPersistentPassword;
    OUString                    m_aPersistentPassword;
    OUString                    m_aPersistentIV;

    void InitArrays( bool bHasMemoryList, ::std::vector< OUString >&& aMemoryList,
                     bool bHasPersistentList, const OUString& aPersistentList, const OUString& aPersistentIV )
    {
        m_bHasMemoryPasswords = bHasMemoryList;
        if ( bHasMemoryList )
            m_aMemoryPasswords = aMemoryList;

        m_bHasPersistentPassword = bHasPersistentList;
        if ( bHasPersistentList )
        {
            m_aPersistentPassword = aPersistentList;
            m_aPersistentIV = aPersistentIV;
        }
    }

public:

    NamePasswordRecord( OUString aName )
        : m_aName(std::move( aName ))
        , m_bHasMemoryPasswords( false )
        , m_bHasPersistentPassword( false )
    {
    }

    NamePasswordRecord( OUString aName, OUString aPersistentList, OUString aPersistentIV )
        : m_aName(std::move( aName ))
        , m_bHasMemoryPasswords( false )
        , m_bHasPersistentPassword( true )
        , m_aPersistentPassword(std::move( aPersistentList ))
        , m_aPersistentIV(std::move( aPersistentIV ))
    {
    }

    NamePasswordRecord( const NamePasswordRecord& aRecord )
        : m_aName( aRecord.m_aName )
        , m_bHasMemoryPasswords( false )
        , m_bHasPersistentPassword( false )
    {
        InitArrays( aRecord.m_bHasMemoryPasswords, std::vector(aRecord.m_aMemoryPasswords),
                    aRecord.m_bHasPersistentPassword, aRecord.m_aPersistentPassword, aRecord.m_aPersistentIV );
    }

    NamePasswordRecord& operator=( const NamePasswordRecord& aRecord )
    {
        if (this != &aRecord)
        {
            m_aName = aRecord.m_aName;

            m_aMemoryPasswords.clear();
            m_aPersistentPassword.clear();
            m_aPersistentIV.clear();
            InitArrays( aRecord.m_bHasMemoryPasswords, std::vector(aRecord.m_aMemoryPasswords),
                        aRecord.m_bHasPersistentPassword, aRecord.m_aPersistentPassword, aRecord.m_aPersistentIV );
        }
        return *this;
    }

    const OUString& GetUserName() const
    {
        return m_aName;
    }

    bool HasPasswords( sal_Int8 nStatus ) const
    {
        if ( nStatus == MEMORY_RECORD )
            return m_bHasMemoryPasswords;
        if ( nStatus == PERSISTENT_RECORD )
            return m_bHasPersistentPassword;

        return false;
    }

    ::std::vector< OUString > GetMemoryPasswords() const
    {
        if ( m_bHasMemoryPasswords )
            return m_aMemoryPasswords;

        return ::std::vector< OUString >();
    }

    OUString GetPersistentPasswords() const
    {
        if ( m_bHasPersistentPassword )
            return m_aPersistentPassword;

        return OUString();
    }

    OUString GetPersistentIV() const
    {
        if ( m_bHasPersistentPassword )
            return m_aPersistentIV;

        return OUString();
    }

    void SetMemoryPasswords( ::std::vector< OUString >&& aMemList )
    {
        m_aMemoryPasswords = std::move(aMemList);
        m_bHasMemoryPasswords = true;
    }

    void SetPersistentPasswords( const OUString& aPersList, const OUString& aPersIV )
    {
        m_aPersistentPassword = aPersList;
        m_aPersistentIV = aPersIV;
        m_bHasPersistentPassword = true;
    }

    void RemovePasswords( sal_Int8 nStatus )
    {
        if ( nStatus == MEMORY_RECORD )
        {
            m_bHasMemoryPasswords = false;
            m_aMemoryPasswords.clear();
        }
        else if ( nStatus == PERSISTENT_RECORD )
        {
            m_bHasPersistentPassword = false;
            m_aPersistentPassword.clear();
            m_aPersistentIV.clear();
        }
    }

};


typedef ::std::pair< const OUString, ::std::vector< NamePasswordRecord > > PairUrlRecord;
typedef ::std::map< OUString, ::std::vector< NamePasswordRecord > > PasswordMap;

// org.openoffice.Office.Common/Passwords/StorageVersion bump if details of
// how password details are saved changes. Enables migration from previous
// schemes.
constexpr sal_Int32 nCurrentStorageVersion = 1;

class PasswordContainer;

class StorageItem
    : public ::utl::ConfigItem
{
private:
    PasswordContainer*     mainCont;
    bool                   hasEncoded;
    OUString               mEncoded;
    OUString               mEncodedIV;

    virtual void            ImplCommit() override;

public:
    StorageItem( PasswordContainer* point, const OUString& path ) :
        ConfigItem( path, ConfigItemMode::NONE ),
        mainCont( point ),
        hasEncoded( false )
    {
        css::uno::Sequence< OUString > aNode { path + "/Store" };
        EnableNotification( aNode );
    }

    PasswordMap getInfo();
    void update( const OUString& url, const NamePasswordRecord& rec );
    void remove( const OUString& url, const OUString& rec );
    void clear();

    sal_Int32 getStorageVersion();

    bool getEncodedMasterPassword( OUString& aResult, OUString& aResultIV );
    void setEncodedMasterPassword( const OUString& aResult, const OUString& aResultIV, bool bAcceptEmpty = false );
    void setUseStorage( bool bUse );
    bool useStorage();

    virtual void            Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
};


class PasswordContainer : public ::cppu::WeakImplHelper<
        css::task::XPasswordContainer2,
        css::lang::XServiceInfo,
        css::lang::XEventListener >
{
private:
    PasswordMap      m_aContainer;
    std::optional<StorageItem> m_xStorageFile;
    std::mutex mMutex;
    OUString m_aMasterPassword; // master password is set when the string is not empty
    css::uno::Reference< css::lang::XComponent > mComponent;
    SysCredentialsConfig mUrlContainer;

    static OUString createIV();

    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::task::UserRecord > CopyToUserRecordSequence(
                                        const ::std::vector< NamePasswordRecord >& original,
                                        const css::uno::Reference< css::task::XInteractionHandler >& Handler );

    css::task::UserRecord CopyToUserRecord(
                                        const NamePasswordRecord& aRecord,
                                        bool& io_bTryToDecode,
                                        const css::uno::Reference< css::task::XInteractionHandler >& aHandler );

    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::task::UserRecord > FindUsr(
                                        const ::std::vector< NamePasswordRecord >& userlist,
                                        std::u16string_view name,
                                        const css::uno::Reference< css::task::XInteractionHandler >& Handler );
    /// @throws css::uno::RuntimeException
    bool createUrlRecord(
        const PasswordMap::iterator & rIter,
        bool bName,
        std::u16string_view aName,
        const css::uno::Reference< css::task::XInteractionHandler >& aHandler,
        css::task::UrlRecord & rRec  );

    /// @throws css::uno::RuntimeException
    css::task::UrlRecord find(
        const OUString& aURL,
        std::u16string_view aName,
        bool bName, // only needed to support empty user names
        const css::uno::Reference< css::task::XInteractionHandler >& aHandler  );

    static OUString GetDefaultMasterPassword();

    static OUString RequestPasswordFromUser(
                    css::task::PasswordRequestMode aRMode,
                    const css::uno::Reference< css::task::XInteractionHandler >& xHandler );

    /// @throws css::uno::RuntimeException
    OUString const & GetMasterPassword( const css::uno::Reference< css::task::XInteractionHandler >& Handler );

    /// @throws css::uno::RuntimeException
    void UpdateVector( const OUString& url, ::std::vector< NamePasswordRecord >& toUpdate, NamePasswordRecord const & rec, bool writeFile );

    /// @throws css::uno::RuntimeException
    void PrivateAdd( const OUString& aUrl,
                              const OUString& aUserName,
                              const css::uno::Sequence< OUString >& aPasswords,
                              char  aMode,
                              const css::uno::Reference< css::task::XInteractionHandler >& Handler );

    /// @throws css::uno::RuntimeException
    static ::std::vector< OUString > DecodePasswords( std::u16string_view aLine, std::u16string_view aIV, std::u16string_view aMasterPassword, css::task::PasswordRequestMode mode );

    /// @throws css::uno::RuntimeException
    static OUString EncodePasswords(const std::vector< OUString >& lines, std::u16string_view aIV, std::u16string_view aMasterPassword );

public:
    PasswordContainer( const css::uno::Reference< css::uno::XComponentContext >& );
    virtual ~PasswordContainer() override;

    virtual void SAL_CALL add( const OUString& aUrl,
                               const OUString& aUserName,
                               const css::uno::Sequence< OUString >& aPasswords,
                               const css::uno::Reference< css::task::XInteractionHandler >& Handler  ) override;

    virtual void SAL_CALL addPersistent( const OUString& aUrl,
                                            const OUString& aUserName,
                                         const css::uno::Sequence< OUString >& aPasswords,
                                          const css::uno::Reference< css::task::XInteractionHandler >& Handler  ) override;

    virtual css::task::UrlRecord SAL_CALL
                            find( const OUString& aUrl,
                                  const css::uno::Reference< css::task::XInteractionHandler >& Handler  ) override;

    virtual css::task::UrlRecord SAL_CALL
                            findForName( const OUString& aUrl,
                                         const OUString& aUserName,
                                         const css::uno::Reference< css::task::XInteractionHandler >& Handler  ) override;

    virtual void SAL_CALL remove( const OUString& aUrl,
                                  const OUString& aUserName ) override;

    virtual void SAL_CALL removePersistent( const OUString& aUrl,
                                            const OUString& aUserName ) override;

    virtual void SAL_CALL removeAllPersistent() override;

    virtual css::uno::Sequence< css::task::UrlRecord > SAL_CALL
                            getAllPersistent( const css::uno::Reference< css::task::XInteractionHandler >& Handler ) override;

    // XServiceInfo
    virtual OUString SAL_CALL    getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL            supportsService( const OUString& ServiceName ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
                                        getSupportedServiceNames(  ) override;

    // XEventListener
    virtual void SAL_CALL        disposing( const css::lang::EventObject& Source ) override;

    // XMasterPasswordHandling
    virtual sal_Bool SAL_CALL authorizateWithMasterPassword( const css::uno::Reference< css::task::XInteractionHandler >& xHandler ) override;
    virtual sal_Bool SAL_CALL changeMasterPassword( const css::uno::Reference< css::task::XInteractionHandler >& xHandler ) override;
    virtual void SAL_CALL removeMasterPassword() override;
    virtual sal_Bool SAL_CALL hasMasterPassword(  ) override;
    virtual sal_Bool SAL_CALL allowPersistentStoring( sal_Bool bAllow ) override;
    virtual sal_Bool SAL_CALL isPersistentStoringAllowed(  ) override;

    // XMasterPasswordHandling2
    virtual sal_Bool SAL_CALL useDefaultMasterPassword( const css::uno::Reference< css::task::XInteractionHandler >& xHandler ) override;
    virtual sal_Bool SAL_CALL isDefaultMasterPasswordUsed(  ) override;

    // XUrlContainer
    virtual void SAL_CALL addUrl( const OUString& Url, sal_Bool MakePersistent ) override;
    virtual OUString SAL_CALL findUrl( const OUString& Url ) override;
    virtual void SAL_CALL removeUrl( const OUString& Url ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getUrls( sal_Bool OnlyPersistent ) override;

    void            Notify();
private:
    bool authorizateWithMasterPassword( std::unique_lock<std::mutex>& rGuard, const css::uno::Reference< css::task::XInteractionHandler >& xHandler );
    css::uno::Sequence< css::task::UrlRecord > getAllPersistent( std::unique_lock<std::mutex>& rGuard, const css::uno::Reference< css::task::XInteractionHandler >& Handler );
    void removeAllPersistent(std::unique_lock<std::mutex>& rGuard);
    void removeMasterPassword(std::unique_lock<std::mutex>& rGuard);
};


class MasterPasswordRequest_Impl : public ucbhelper::InteractionRequest
{
    ::rtl::Reference< ucbhelper::InteractionSupplyAuthentication > m_xAuthSupplier;

public:
    MasterPasswordRequest_Impl( css::task::PasswordRequestMode Mode );

    const ::rtl::Reference< ucbhelper::InteractionSupplyAuthentication > &
    getAuthenticationSupplier() const { return m_xAuthSupplier; }

};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
