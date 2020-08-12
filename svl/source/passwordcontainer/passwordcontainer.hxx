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

#include <vector>
#include <map>
#include <com/sun/star/task/XPasswordContainer2.hpp>
#include <com/sun/star/task/PasswordRequestMode.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>

#include <unotools/configitem.hxx>
#include <ucbhelper/interactionrequest.hxx>

#include <rtl/ref.hxx>
#include <osl/mutex.hxx>

#include "syscreds.hxx"

#define MEMORY_RECORD         0
#define PERSISTENT_RECORD     1


class NamePassRecord
{
    OUString                                       m_aName;

    // there are two lists of passwords, memory passwords and persistent passwords
    bool                                                  m_bHasMemPass;
    ::std::vector< OUString >                      m_aMemPass;

    // persistent passwords are encrypted in one string
    bool                                                  m_bHasPersPass;
    OUString                                       m_aPersPass;

    void InitArrays( bool bHasMemoryList, const ::std::vector< OUString >& aMemoryList,
                     bool bHasPersistentList, const OUString& aPersistentList )
    {
        m_bHasMemPass = bHasMemoryList;
        if ( bHasMemoryList )
            m_aMemPass = aMemoryList;

        m_bHasPersPass = bHasPersistentList;
        if ( bHasPersistentList )
            m_aPersPass = aPersistentList;
    }

public:

    NamePassRecord( const OUString& aName )
        : m_aName( aName )
        , m_bHasMemPass( false )
        , m_bHasPersPass( false )
    {
    }

    NamePassRecord( const OUString& aName, const OUString& aPersistentList )
        : m_aName( aName )
        , m_bHasMemPass( false )
        , m_bHasPersPass( true )
        , m_aPersPass( aPersistentList )
    {
    }

    NamePassRecord( const NamePassRecord& aRecord )
        : m_aName( aRecord.m_aName )
        , m_bHasMemPass( false )
        , m_bHasPersPass( false )
    {
        InitArrays( aRecord.m_bHasMemPass, aRecord.m_aMemPass, aRecord.m_bHasPersPass, aRecord.m_aPersPass );
    }

    NamePassRecord& operator=( const NamePassRecord& aRecord )
    {
        if (this != &aRecord)
        {
            m_aName = aRecord.m_aName;

            m_aMemPass.clear();
            m_aPersPass.clear();
            InitArrays( aRecord.m_bHasMemPass, aRecord.m_aMemPass, aRecord.m_bHasPersPass, aRecord.m_aPersPass );
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
            return m_bHasMemPass;
        if ( nStatus == PERSISTENT_RECORD )
            return m_bHasPersPass;

        return false;
    }

    ::std::vector< OUString > GetMemPasswords() const
    {
        if ( m_bHasMemPass )
            return m_aMemPass;

        return ::std::vector< OUString >();
    }

    OUString GetPersPasswords() const
    {
        if ( m_bHasPersPass )
            return m_aPersPass;

        return OUString();
    }

    void SetMemPasswords( const ::std::vector< OUString >& aMemList )
    {
        m_aMemPass = aMemList;
        m_bHasMemPass = true;
    }

    void SetPersPasswords( const OUString& aPersList )
    {
        m_aPersPass = aPersList;
        m_bHasPersPass = true;
    }

    void RemovePasswords( sal_Int8 nStatus )
    {
        if ( nStatus == MEMORY_RECORD )
        {
            m_bHasMemPass = false;
            m_aMemPass.clear();
        }
        else if ( nStatus == PERSISTENT_RECORD )
        {
            m_bHasPersPass = false;
            m_aPersPass.clear();
        }
    }

};


typedef ::std::pair< const OUString, ::std::vector< NamePassRecord > > PairUrlRecord;
typedef ::std::map< OUString, ::std::vector< NamePassRecord > > PassMap;


class PasswordContainer;

class StorageItem
    : public ::utl::ConfigItem
{
private:
    PasswordContainer*     mainCont;
    bool                   hasEncoded;
    OUString        mEncoded;

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

    PassMap getInfo();
    void update( const OUString& url, const NamePassRecord& rec );
    void remove( const OUString& url, const OUString& rec );
    void clear();

    bool getEncodedMP( OUString& aResult );
    void setEncodedMP( const OUString& aResult, bool bAcceptEmpty = false );
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
    PassMap      m_aContainer;
    std::unique_ptr<StorageItem> m_pStorageFile;
    ::osl::Mutex mMutex;
    OUString m_aMasterPasswd; // master password is set when the string is not empty
    css::uno::Reference< css::lang::XComponent > mComponent;
    SysCredentialsConfig mUrlContainer;

    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::task::UserRecord > CopyToUserRecordSequence(
                                        const ::std::vector< NamePassRecord >& original,
                                        const css::uno::Reference< css::task::XInteractionHandler >& Handler );

    css::task::UserRecord CopyToUserRecord(
                                        const NamePassRecord& aRecord,
                                        bool& io_bTryToDecode,
                                        const css::uno::Reference< css::task::XInteractionHandler >& aHandler );

    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::task::UserRecord > FindUsr(
                                        const ::std::vector< NamePassRecord >& userlist,
                                        const OUString& name,
                                        const css::uno::Reference< css::task::XInteractionHandler >& Handler );
/// @throws css::uno::RuntimeException
bool createUrlRecord(
    const PassMap::iterator & rIter,
    bool bName,
    const OUString & aName,
    const css::uno::Reference< css::task::XInteractionHandler >& aHandler,
    css::task::UrlRecord & rRec  );

/// @throws css::uno::RuntimeException
css::task::UrlRecord find(
    const OUString& aURL,
    const OUString& aName,
    bool bName, // only needed to support empty user names
    const css::uno::Reference< css::task::XInteractionHandler >& aHandler  );

    static OUString GetDefaultMasterPassword();

    static OUString RequestPasswordFromUser(
                    css::task::PasswordRequestMode aRMode,
                    const css::uno::Reference< css::task::XInteractionHandler >& xHandler );

    /// @throws css::uno::RuntimeException
    OUString const & GetMasterPassword( const css::uno::Reference< css::task::XInteractionHandler >& Handler );

    /// @throws css::uno::RuntimeException
    void UpdateVector( const OUString& url, ::std::vector< NamePassRecord >& toUpdate, NamePassRecord const & rec, bool writeFile );

    /// @throws css::uno::RuntimeException
    void PrivateAdd( const OUString& aUrl,
                              const OUString& aUserName,
                              const css::uno::Sequence< OUString >& aPasswords,
                              char  aMode,
                              const css::uno::Reference< css::task::XInteractionHandler >& Handler );

    /// @throws css::uno::RuntimeException
    static ::std::vector< OUString > DecodePasswords( const OUString& aLine, const OUString& aMasterPassword, css::task::PasswordRequestMode mode );

    /// @throws css::uno::RuntimeException
    static OUString EncodePasswords(const std::vector< OUString >& lines, const OUString& aMasterPassword );

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
