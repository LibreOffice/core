/*************************************************************************
 *
 *  $RCSfile: contentenumeration.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-10-22 12:32:58 $
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

#ifndef SVTOOLS_SOURCE_CONTNR_CONTENTENUMERATION_HXX
#include "contentenumeration.hxx"
#endif

#ifndef SVTOOLS_URL_FILTER_HXX
#include "urlfilter.hxx"
#endif
#ifndef _SVTOOLS_INETTBC_HXX
#include "inettbc.hxx"
#endif
#ifndef _SVTOOLS_IMAGEMGR_HXX
#include "imagemgr.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XDYNAMICRESULTSET_HPP_
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
/** === end UNO includes === **/

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

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
    using ::ucb::ResultSetInclude;
    using ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS;
    using ::ucb::Content;

    //====================================================================
    //= FileViewContentEnumerator
    //====================================================================
    //--------------------------------------------------------------------
    FileViewContentEnumerator::FileViewContentEnumerator(
            const Reference< XCommandEnvironment >& _rxCommandEnv,
            ContentData& _rContentToFill, ::osl::Mutex& _rContentMutex,
            const IContentTitleTranslation* _pTranslator )
        :m_refCount              ( 0               )
        ,m_xCommandEnv           ( _rxCommandEnv   )
        ,m_rContent              ( _rContentToFill )
        ,m_rContentMutex         ( _rContentMutex  )
        ,m_pFilter               ( NULL            )
        ,m_pTranslator           ( _pTranslator    )
        ,m_bCancelled            ( false           )
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
        m_aFolder.aContent = ::ucb::Content();
        m_aFolder.sURL = String();
    }

    //--------------------------------------------------------------------
    EnumerationResult FileViewContentEnumerator::enumerateFolderContentSync( const FolderDescriptor& _rFolder, const IUrlFilter* _pFilter )
    {
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            m_aFolder = _rFolder;
            m_pFilter = _pFilter;
            m_pResultHandler = NULL;
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

            try
            {
                ::osl::MutexGuard aGuard( m_aMutex );

                if ( !m_aFolder.aContent.get().is() )
                    m_aFolder.aContent = Content( m_aFolder.sURL, m_xCommandEnv );

                Reference< XDynamicResultSet > xDynResultSet;
                ResultSetInclude eInclude = INCLUDE_FOLDERS_AND_DOCUMENTS;
                xDynResultSet = m_aFolder.aContent.createDynamicCursor( aProps, eInclude );

                if ( xDynResultSet.is() )
                    xResultSet = xDynResultSet->getStaticResultSet();
            }
            catch( CommandAbortedException& )
            {
                DBG_ERRORFILE( "createCursor: CommandAbortedException" );
            }
            catch( Exception& e )
            {
                e; // make compiler happy
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
                                Content aCnt( aTargetURL, NULL );
                                aCnt.getPropertyValue( OUString::createFromAscii( "Size" ) ) >>= pData->maSize;
                                aCnt.getPropertyValue( OUString::createFromAscii( "DateModified" ) ) >>= aDT;
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
                        String( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.DocumentProperties") )
                    )
                );
            }

            DBG_ASSERT( m_xDocInfo.is(), "FileViewContentEnumerator::implGetDocTitle: no DocumentProperties service!" );
            if ( !m_xDocInfo.is() )
                return sal_False;

            m_xDocInfo->read( _rTargetURL );
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

