/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#include "contentenumeration.hxx"
#include <svl/urlfilter.hxx>
#include <svtools/inettbc.hxx>
#include <svtools/imagemgr.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
/** === end UNO includes === **/
#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

#include <memory>

//........................................................................
namespace svt
{
//........................................................................

#define ROW_TITLE           1
#define ROW_SIZE            2
#define ROW_DATE_MOD        3
#define ROW_DATE_CREATE     4
#define ROW_IS_FOLDER       5
#define ROW_TARGET_URL      6
#define ROW_IS_HIDDEN       7
#define ROW_IS_VOLUME       8
#define ROW_IS_REMOTE       9
#define ROW_IS_REMOVEABLE   10
#define ROW_IS_FLOPPY       11
#define ROW_IS_COMPACTDISC  12

#define CONVERT_DATETIME( aUnoDT, aToolsDT ) \
    aToolsDT = ::DateTime( Date( aUnoDT.Day, aUnoDT.Month, aUnoDT.Year ), \
                           Time( aUnoDT.Hours, aUnoDT.Minutes, aUnoDT.Seconds, aUnoDT.HundredthSeconds ) );

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::util::DateTime;
    using ::com::sun::star::sdbc::XResultSet;
    using ::com::sun::star::sdbc::XRow;
    using ::com::sun::star::ucb::XDynamicResultSet;
    using ::com::sun::star::ucb::CommandAbortedException;
    using ::com::sun::star::ucb::XContentAccess;
    using ::com::sun::star::ucb::XCommandEnvironment;
    using ::com::sun::star::beans::XPropertySet;
    using ::rtl::OUString;
    using ::ucbhelper::ResultSetInclude;
    using ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS;

    //====================================================================
    //= FileViewContentEnumerator
    //====================================================================
    //--------------------------------------------------------------------
    FileViewContentEnumerator::FileViewContentEnumerator(
            const Reference< XCommandEnvironment >& _rxCommandEnv,
            ContentData& _rContentToFill, ::osl::Mutex& _rContentMutex,
            const IContentTitleTranslation* _pTranslator )
        :m_rContent              ( _rContentToFill )
        ,m_rContentMutex         ( _rContentMutex  )
        ,m_refCount              ( 0               )
        ,m_xCommandEnv           ( _rxCommandEnv   )
        ,m_pFilter               ( NULL            )
        ,m_pTranslator           ( _pTranslator    )
        ,m_bCancelled            ( false           )
        ,m_rBlackList            ( ::com::sun::star::uno::Sequence< ::rtl::OUString >() )
    {
    }

    //--------------------------------------------------------------------
    FileViewContentEnumerator::~FileViewContentEnumerator()
    {
    }

    //--------------------------------------------------------------------
    void FileViewContentEnumerator::cancel()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_bCancelled = true;
        m_pResultHandler = NULL;
        m_pTranslator = NULL;
        m_pFilter = NULL;
        m_aFolder.aContent = ::ucbhelper::Content();
        m_aFolder.sURL = String();
    }

    //--------------------------------------------------------------------
    EnumerationResult FileViewContentEnumerator::enumerateFolderContentSync(
        const FolderDescriptor& _rFolder,
        const IUrlFilter* _pFilter,
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rBlackList )
    {
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            m_aFolder = _rFolder;
            m_pFilter = _pFilter;
            m_pResultHandler = NULL;
            m_rBlackList = rBlackList;
        }
        return enumerateFolderContent();
    }

    //--------------------------------------------------------------------
    void FileViewContentEnumerator::enumerateFolderContent(
        const FolderDescriptor& _rFolder, const IUrlFilter* _pFilter, IEnumerationResultHandler* _pResultHandler )
    {
        // ensure that we don't get deleted while herein
        acquire();
            // the matching "release" will be called in onTerminated
            // Note that onTerminated is only called if run was left normally.
            // If somebody terminates the thread from the outside, then onTerminated
            // will never be called. However, our terminate method is not accessible
            // to our clients, so the only class which could misbehave is this class
            // here itself ...

        ::osl::MutexGuard aGuard( m_aMutex );
        m_aFolder = _rFolder;
        m_pFilter = _pFilter;
        m_pResultHandler = _pResultHandler;

        OSL_ENSURE( m_aFolder.aContent.get().is() || m_aFolder.sURL.Len(),
            "FileViewContentEnumerator::enumerateFolderContent: invalid folder descriptor!" );

        // start the thread
        create();
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL FileViewContentEnumerator::acquire()
    {
        return osl_incrementInterlockedCount( &m_refCount );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL FileViewContentEnumerator::release()
    {
        if ( 0 == osl_decrementInterlockedCount( &m_refCount ) )
        {
            delete this;
            return 0;
        }
        return m_refCount;
    }

    //--------------------------------------------------------------------
    EnumerationResult FileViewContentEnumerator::enumerateFolderContent()
    {
        EnumerationResult eResult = ERROR;
        try
        {

            Reference< XResultSet > xResultSet;
            Sequence< OUString > aProps(12);

            aProps[0] = OUString::createFromAscii( "Title" );
            aProps[1] = OUString::createFromAscii( "Size" );
            aProps[2] = OUString::createFromAscii( "DateModified" );
            aProps[3] = OUString::createFromAscii( "DateCreated" );
            aProps[4] = OUString::createFromAscii( "IsFolder" );
            aProps[5] = OUString::createFromAscii( "TargetURL" );
            aProps[6] = OUString::createFromAscii( "IsHidden" );
            aProps[7] = OUString::createFromAscii( "IsVolume" );
            aProps[8] = OUString::createFromAscii( "IsRemote" );
            aProps[9] = OUString::createFromAscii( "IsRemoveable" );
            aProps[10] = OUString::createFromAscii( "IsFloppy" );
            aProps[11] = OUString::createFromAscii( "IsCompactDisc" );

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
                    aFolder.aContent = ::ucbhelper::Content( aFolder.sURL, xEnvironment );
                    {
                        ::osl::MutexGuard aGuard( m_aMutex );
                        m_aFolder.aContent = aFolder.aContent;
                    }
                }

                Reference< XDynamicResultSet > xDynResultSet;
                ResultSetInclude eInclude = INCLUDE_FOLDERS_AND_DOCUMENTS;
                xDynResultSet = aFolder.aContent.createDynamicCursor( aProps, eInclude );

                if ( xDynResultSet.is() )
                    xResultSet = xDynResultSet->getStaticResultSet();
            }
            catch( CommandAbortedException& )
            {
                DBG_ERRORFILE( "createCursor: CommandAbortedException" );
            }
            catch( Exception& )
            {
            }

            bool bCancelled = false;
            if ( xResultSet.is() )
            {
                Reference< XRow > xRow( xResultSet, UNO_QUERY );
                Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );

                try
                {
                    SortingData_Impl* pData;
                    DateTime aDT;

                    while ( !bCancelled && xResultSet->next() )
                    {
                        sal_Bool bIsHidden = xRow->getBoolean( ROW_IS_HIDDEN );
                        // don't show hidden files
                        if ( !bIsHidden || xRow->wasNull() )
                        {
                            pData = NULL;

                            aDT = xRow->getTimestamp( ROW_DATE_MOD );
                            sal_Bool bContainsDate = !xRow->wasNull();
                            if ( !bContainsDate )
                            {
                                aDT = xRow->getTimestamp( ROW_DATE_CREATE );
                                bContainsDate = !xRow->wasNull();
                            }

                            OUString aContentURL = xContentAccess->queryContentIdentifierString();
                            OUString aTargetURL = xRow->getString( ROW_TARGET_URL );
                            sal_Bool bHasTargetURL = !xRow->wasNull() && aTargetURL.getLength() > 0;

                            OUString sRealURL = bHasTargetURL ? aTargetURL : aContentURL;

                            // check for restrictions
                            {
                                ::osl::MutexGuard aGuard( m_aMutex );
                                if ( m_pFilter && !m_pFilter->isUrlAllowed( sRealURL ) )
                                    continue;

                                if ( /* m_rBlackList.hasElements() && */ URLOnBlackList ( sRealURL ) )
                                    continue;
                            }

                            pData = new SortingData_Impl;
                            pData->maTargetURL = sRealURL;

                            pData->mbIsFolder = xRow->getBoolean( ROW_IS_FOLDER ) && !xRow->wasNull();
                            pData->mbIsVolume = xRow->getBoolean( ROW_IS_VOLUME ) && !xRow->wasNull();
                            pData->mbIsRemote = xRow->getBoolean( ROW_IS_REMOTE ) && !xRow->wasNull();
                            pData->mbIsRemoveable = xRow->getBoolean( ROW_IS_REMOVEABLE ) && !xRow->wasNull();
                            pData->mbIsFloppy = xRow->getBoolean( ROW_IS_FLOPPY ) && !xRow->wasNull();
                            pData->mbIsCompactDisc = xRow->getBoolean( ROW_IS_COMPACTDISC ) && !xRow->wasNull();
                            pData->SetNewTitle( xRow->getString( ROW_TITLE ) );
                            pData->maSize = xRow->getLong( ROW_SIZE );

                            if ( bHasTargetURL &&
                                INetURLObject( aContentURL ).GetProtocol() == INET_PROT_VND_SUN_STAR_HIER )
                            {
                                ::ucbhelper::Content aCnt( aTargetURL, xEnvironment );
                                try
                                {
                                aCnt.getPropertyValue( OUString::createFromAscii( "Size" ) ) >>= pData->maSize;
                                aCnt.getPropertyValue( OUString::createFromAscii( "DateModified" ) ) >>= aDT;
                                }
                                catch (...) {}
                            }

                            if ( bContainsDate )
                            {
                                CONVERT_DATETIME( aDT, pData->maModDate );
                            }

                            if ( pData->mbIsFolder )
                            {
                                ::vos::OGuard aGuard( Application::GetSolarMutex() );
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
                                    sal_Bool bTranslated = sal_False;

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
                    eResult = SUCCESS;
                }
                catch( CommandAbortedException& )
                {
                    DBG_ERRORFILE( "FileViewContentEnumerator::enumerateFolderContent: caught an CommandAbortedException while enumerating!" );
                }
                catch( Exception& )
                {
                    DBG_ERRORFILE( "FileViewContentEnumerator::enumerateFolderContent: caught an exception other than CommandAbortedException while enumerating!" );
                }
            }
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "FileViewContentEnumerator::enumerateFolderContent: caught an CommandAbortedException!" );
        }
        catch( Exception& )
        {
            DBG_ERRORFILE( "FileViewContentEnumerator::enumerateFolderContent: caught an exception other than CommandAbortedException!" );
        }

        IEnumerationResultHandler* pHandler = NULL;
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            pHandler = m_pResultHandler;
            if ( m_bCancelled )
                return ERROR;
        }

        {
            ::osl::MutexGuard aGuard( m_rContentMutex );
            if ( eResult != SUCCESS )
                // clear any "intermediate" and unfinished result
                m_rContent.clear();
        }

        if ( pHandler )
            pHandler->enumerationDone( eResult );
        return eResult;
    }

    //--------------------------------------------------------------------

    sal_Bool FileViewContentEnumerator::URLOnBlackList ( const ::rtl::OUString& sRealURL )
    {
        ::rtl::OUString entryName = sRealURL.copy( sRealURL.lastIndexOf( rtl::OUString::createFromAscii("/")) +1 );

        for (int i = 0; i < m_rBlackList.getLength() ; i++)
        {
            if ( entryName.equals(  m_rBlackList[i] ) )
                return true;
        }

        return false;
    }

    //--------------------------------------------------------------------
    sal_Bool FileViewContentEnumerator::implGetDocTitle( const OUString& _rTargetURL, OUString& _rRet ) const
    {
        sal_Bool bRet = sal_False;

        try
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if( !m_xDocInfo.is() )
            {
                m_xDocInfo = m_xDocInfo.query(
                    ::comphelper::getProcessServiceFactory()->createInstance(
                        String( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.StandaloneDocumentInfo") )
                    )
                );
            }

            DBG_ASSERT( m_xDocInfo.is(), "FileViewContentEnumerator::implGetDocTitle: no DocumentProperties service!" );
            if ( !m_xDocInfo.is() )
                return sal_False;

            m_xDocInfo->loadFromURL( _rTargetURL );
            Reference< XPropertySet > xPropSet( m_xDocInfo, UNO_QUERY );

            Any aAny = xPropSet->getPropertyValue( OUString::createFromAscii( "Title" ) );

            OUString sTitle;
            if ( ( aAny >>= sTitle ) && sTitle.getLength() > 0 )
            {
                _rRet = sTitle;
                bRet = sal_True;
            }
        }
        catch ( const Exception& )
        {
        }

        return bRet;
    }

    //--------------------------------------------------------------------
    void SAL_CALL FileViewContentEnumerator::run()
    {
        enumerateFolderContent();
    }

    //--------------------------------------------------------------------
    void SAL_CALL FileViewContentEnumerator::onTerminated()
    {
        release();
    }

//........................................................................
} // namespace svt
//........................................................................

