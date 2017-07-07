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

#include "contentenumeration.hxx"
#include <svtools/inettbc.hxx>
#include <svtools/imagemgr.hxx>

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/document/DocumentProperties.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

namespace svt
{


#define ROW_TITLE           1
#define ROW_SIZE            2
#define ROW_DATE_MOD        3
#define ROW_DATE_CREATE     4
#define ROW_IS_FOLDER       5
#define ROW_TARGET_URL      6
#define ROW_IS_HIDDEN       7
#define ROW_IS_VOLUME       8
#define ROW_IS_REMOTE       9
#define ROW_IS_REMOVABLE    10
#define ROW_IS_FLOPPY       11
#define ROW_IS_COMPACTDISC  12

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::util::DateTime;
    using ::com::sun::star::sdbc::XResultSet;
    using ::com::sun::star::sdbc::XRow;
    using ::com::sun::star::ucb::XDynamicResultSet;
    using ::com::sun::star::ucb::CommandAbortedException;
    using ::com::sun::star::ucb::XContentAccess;
    using ::com::sun::star::ucb::XCommandEnvironment;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::document::DocumentProperties;
    using ::ucbhelper::ResultSetInclude;
    using ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS;


    //= FileViewContentEnumerator


    FileViewContentEnumerator::FileViewContentEnumerator(
            const Reference< XCommandEnvironment >& _rxCommandEnv,
            ContentData& _rContentToFill, ::osl::Mutex& _rContentMutex,
            const IContentTitleTranslation* _pTranslator )
        :Thread                  ( "FileViewContentEnumerator" )
        ,m_rContent              ( _rContentToFill )
        ,m_rContentMutex         ( _rContentMutex  )
        ,m_xCommandEnv           ( _rxCommandEnv   )
        ,m_pTranslator           ( _pTranslator    )
        ,m_pResultHandler        ( nullptr            )
        ,m_bCancelled            ( false           )
        ,m_rBlackList            ( css::uno::Sequence< OUString >() )
    {
    }


    FileViewContentEnumerator::~FileViewContentEnumerator()
    {
    }


    void FileViewContentEnumerator::cancel()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_bCancelled = true;
        m_pResultHandler = nullptr;
        m_pTranslator = nullptr;
        m_aFolder.aContent = ::ucbhelper::Content();
        m_aFolder.sURL.clear();
    }


    EnumerationResult FileViewContentEnumerator::enumerateFolderContentSync(
        const FolderDescriptor& _rFolder,
        const css::uno::Sequence< OUString >& rBlackList )
    {
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            m_aFolder = _rFolder;
            m_pResultHandler = nullptr;
            m_rBlackList = rBlackList;
        }
        return enumerateFolderContent();
    }


    void FileViewContentEnumerator::enumerateFolderContent(
        const FolderDescriptor& _rFolder, IEnumerationResultHandler* _pResultHandler )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_aFolder = _rFolder;
        m_pResultHandler = _pResultHandler;

        OSL_ENSURE( m_aFolder.aContent.get().is() || !m_aFolder.sURL.isEmpty(),
            "FileViewContentEnumerator::enumerateFolderContent: invalid folder descriptor!" );

        launch();
            //TODO: a protocol is missing how to join with the launched thread
            // before exit(3), to ensure the thread is no longer relying on any
            // infrastructure while that infrastructure is being shut down in
            // atexit handlers
    }


    EnumerationResult FileViewContentEnumerator::enumerateFolderContent()
    {
        EnumerationResult eResult = EnumerationResult::ERROR;
        try
        {

            Reference< XResultSet > xResultSet;
            Sequence< OUString > aProps(12);

            aProps[0] = "Title";
            aProps[1] = "Size";
            aProps[2] = "DateModified";
            aProps[3] = "DateCreated";
            aProps[4] = "IsFolder";
            aProps[5] = "TargetURL";
            aProps[6] = "IsHidden";
            aProps[7] = "IsVolume";
            aProps[8] = "IsRemote";
            aProps[9] = "IsRemoveable";
            aProps[10] = "IsFloppy";
            aProps[11] = "IsCompactDisc";

            Reference< XCommandEnvironment > xEnvironment;
            try
            {
                FolderDescriptor aFolder;
                {
                    ::osl::MutexGuard aGuard( m_aMutex );
                    aFolder = m_aFolder;
                    xEnvironment = m_xCommandEnv;
                }
                if ( !aFolder.aContent.get().is() )
                {
                    aFolder.aContent = ::ucbhelper::Content( aFolder.sURL, xEnvironment, comphelper::getProcessComponentContext() );
                    {
                        ::osl::MutexGuard aGuard( m_aMutex );
                        m_aFolder.aContent = aFolder.aContent;
                    }
                }

                Reference< XDynamicResultSet > xDynResultSet;
                xDynResultSet = aFolder.aContent.createDynamicCursor( aProps, INCLUDE_FOLDERS_AND_DOCUMENTS );

                if ( xDynResultSet.is() )
                    xResultSet = xDynResultSet->getStaticResultSet();
            }
            catch( CommandAbortedException& )
            {
                SAL_WARN( "svtools.contnr", "createCursor: CommandAbortedException" );
            }
            catch( Exception& )
            {
            }

            if ( xResultSet.is() )
            {
                Reference< XRow > xRow( xResultSet, UNO_QUERY );
                Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );

                try
                {
                    SortingData_Impl* pData;
                    DateTime aDT;

                    bool bCancelled = false;
                    while ( !bCancelled && xResultSet->next() )
                    {
                        bool bIsHidden = xRow->getBoolean( ROW_IS_HIDDEN );
                        // don't show hidden files
                        if ( !bIsHidden || xRow->wasNull() )
                        {
                            pData = nullptr;

                            aDT = xRow->getTimestamp( ROW_DATE_MOD );
                            bool bContainsDate = !xRow->wasNull();
                            if ( !bContainsDate )
                            {
                                aDT = xRow->getTimestamp( ROW_DATE_CREATE );
                                bContainsDate = !xRow->wasNull();
                            }

                            OUString aContentURL = xContentAccess->queryContentIdentifierString();
                            OUString aTargetURL = xRow->getString( ROW_TARGET_URL );
                            bool bHasTargetURL = !xRow->wasNull() && !aTargetURL.isEmpty();

                            OUString sRealURL = bHasTargetURL ? aTargetURL : aContentURL;

                            // check for restrictions
                            {
                                ::osl::MutexGuard aGuard( m_aMutex );
                                if ( /* m_rBlackList.hasElements() && */ URLOnBlackList ( sRealURL ) )
                                    continue;
                            }

                            pData = new SortingData_Impl;
                            pData->maTargetURL = sRealURL;

                            pData->mbIsFolder = xRow->getBoolean( ROW_IS_FOLDER ) && !xRow->wasNull();
                            pData->mbIsVolume = xRow->getBoolean( ROW_IS_VOLUME ) && !xRow->wasNull();
                            pData->mbIsRemote = xRow->getBoolean( ROW_IS_REMOTE ) && !xRow->wasNull();
                            pData->mbIsRemoveable = xRow->getBoolean( ROW_IS_REMOVABLE ) && !xRow->wasNull();
                            pData->mbIsFloppy = xRow->getBoolean( ROW_IS_FLOPPY ) && !xRow->wasNull();
                            pData->mbIsCompactDisc = xRow->getBoolean( ROW_IS_COMPACTDISC ) && !xRow->wasNull();
                            pData->SetNewTitle( xRow->getString( ROW_TITLE ) );
                            pData->maSize = xRow->getLong( ROW_SIZE );

                            if ( bHasTargetURL &&
                                INetURLObject( aContentURL ).GetProtocol() == INetProtocol::VndSunStarHier )
                            {
                                ::ucbhelper::Content aCnt( aTargetURL, xEnvironment, comphelper::getProcessComponentContext() );
                                try
                                {
                                aCnt.getPropertyValue("Size") >>= pData->maSize;
                                aCnt.getPropertyValue("DateModified") >>= aDT;
                                }
                                catch (...) {}
                            }

                            if ( bContainsDate )
                            {
                                pData->maModDate = ::DateTime( aDT );
                            }

                            if ( pData->mbIsFolder )
                            {
                                SolarMutexGuard aGuard;
                                ::svtools::VolumeInfo aVolInfo( pData->mbIsVolume, pData->mbIsRemote,
                                                                pData->mbIsRemoveable, pData->mbIsFloppy,
                                                                pData->mbIsCompactDisc );
                                pData->maType = SvFileInformationManager::GetFolderDescription( aVolInfo );
                            }
                            else
                                pData->maType = SvFileInformationManager::GetFileDescription(
                                    INetURLObject( pData->maTargetURL ) );

                            // replace names on demand
                            {
                                ::osl::MutexGuard aGuard( m_aMutex );
                                if( m_pTranslator )
                                {
                                    OUString sNewTitle;
                                    bool bTranslated = false;

                                    if ( pData->mbIsFolder )
                                        bTranslated = m_pTranslator->GetTranslation( pData->GetTitle(), sNewTitle );
                                    else
                                        bTranslated = implGetDocTitle( pData->maTargetURL, sNewTitle );

                                    if ( bTranslated )
                                        pData->ChangeTitle( sNewTitle );
                                }
                            }

                            {
                                ::osl::MutexGuard aGuard( m_rContentMutex );
                                m_rContent.push_back( pData );
                            }
                        }

                        {
                            ::osl::MutexGuard aGuard( m_aMutex );
                            bCancelled = m_bCancelled;
                        }
                    }
                    eResult = EnumerationResult::SUCCESS;
                }
                catch( CommandAbortedException& )
                {
                    SAL_WARN( "svtools.contnr", "FileViewContentEnumerator::enumerateFolderContent: caught an CommandAbortedException while enumerating!" );
                }
                catch( Exception& )
                {
                    SAL_WARN( "svtools.contnr", "FileViewContentEnumerator::enumerateFolderContent: caught an exception other than CommandAbortedException while enumerating!" );
                }
            }
        }
        catch( CommandAbortedException& )
        {
            SAL_WARN( "svtools.contnr", "FileViewContentEnumerator::enumerateFolderContent: caught an CommandAbortedException!" );
        }
        catch( Exception& )
        {
            SAL_WARN( "svtools.contnr", "FileViewContentEnumerator::enumerateFolderContent: caught an exception other than CommandAbortedException!" );
        }

        IEnumerationResultHandler* pHandler = nullptr;
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            pHandler = m_pResultHandler;
            if ( m_bCancelled )
                return EnumerationResult::ERROR;
        }

        {
            ::osl::MutexGuard aGuard( m_rContentMutex );
            if ( eResult != EnumerationResult::SUCCESS )
                // clear any "intermediate" and unfinished result
                m_rContent.clear();
        }

        if ( pHandler )
            pHandler->enumerationDone( eResult );
        return eResult;
    }


    bool FileViewContentEnumerator::URLOnBlackList ( const OUString& sRealURL )
    {
        OUString entryName = sRealURL.copy( sRealURL.lastIndexOf( '/' ) + 1 );

        for (int i = 0; i < m_rBlackList.getLength() ; i++)
        {
            if ( entryName == m_rBlackList[i] )
                return true;
        }

        return false;
    }


    bool FileViewContentEnumerator::implGetDocTitle( const OUString& _rTargetURL, OUString& _rRet ) const
    {
        bool bRet = false;

        try
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if (!m_xDocProps.is())
            {
                m_xDocProps.set(DocumentProperties::create(
                            ::comphelper::getProcessComponentContext()));
            }

            assert(m_xDocProps.is());
            if (!m_xDocProps.is())
                return false;

            m_xDocProps->loadFromMedium(_rTargetURL, Sequence<PropertyValue>());

            OUString const sTitle(m_xDocProps->getTitle());
            if (!sTitle.isEmpty())
            {
                _rRet = sTitle;
                bRet = true;
            }
        }
        catch ( const Exception& )
        {
        }

        return bRet;
    }


    void FileViewContentEnumerator::execute()
    {
        enumerateFolderContent();
    }


} // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
