/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <svl/itemprop.hxx>
#include <svl/urihelper.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <tools/shl.hxx>
#include <unotools/tempfile.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/timer.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/text/MailMergeType.hpp>
#include <com/sun/star/text/MailMergeEvent.hpp>
#include <com/sun/star/text/XMailMergeListener.hpp>
#include <com/sun/star/text/XMailMergeBroadcaster.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include "com/sun/star/mail/XSmtpService.hpp"
#include <sfx2/viewfrm.hxx>
#include <sfx2/event.hxx>
#include <swevent.hxx>
#include <unomailmerge.hxx>
#include <swdll.hxx>
#include <swmodule.hxx>
#include <unoprnms.hxx>
#include <unomap.hxx>
#include <swunohelper.hxx>
#include <docsh.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <view.hxx>
#include <dbmgr.hxx>
#include <unotxdoc.hxx>
#include <prtopt.hxx>
#include <wrtsh.hxx>
#include <shellio.hxx>
#include <mmconfigitem.hxx>
#include <mailmergehelper.hxx>

#include <unomid.h>

#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace SWUnoHelper;

typedef ::utl::SharedUNOComponent< XInterface > SharedComponent;

osl::Mutex &    GetMailMergeMutex()
{
    static osl::Mutex   aMutex;
    return aMutex;
}

enum CloseResult
{
    eSuccess,       
    eVetoed,        
    eFailed         
};
static CloseResult CloseModelAndDocSh(
       Reference< frame::XModel > &rxModel,
       SfxObjectShellRef &rxDocSh )
{
    CloseResult eResult = eSuccess;

    rxDocSh = 0;

    
    
    
    Reference< util::XCloseable > xClose( rxModel, UNO_QUERY );
    if (xClose.is())
    {
        try
        {
            
            
            xClose->close( sal_True );
        }
        catch (const util::CloseVetoException&)
        {
            
            
            eResult = eVetoed;
        }
        catch (const uno::RuntimeException&)
        {
            eResult = eFailed;
        }
    }
    return eResult;
}

static bool LoadFromURL_impl(
        Reference< frame::XModel > &rxModel,
        SfxObjectShellRef &rxDocSh,
        const OUString &rURL,
        bool bClose )
    throw (RuntimeException)
{
    
    Reference< frame::XModel > xTmpModel;
    Sequence < PropertyValue > aArgs( 1 );
    aArgs[0].Name = "Hidden";
    sal_Bool bVal = sal_True;
    aArgs[0].Value <<= bVal;
    try
    {
        Reference < XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
        xTmpModel = Reference < XModel >( xDesktop->loadComponentFromURL(
                rURL, "_blank", 0, aArgs ), UNO_QUERY );
    }
    catch (const Exception&)
    {
        return false;
    }

    
    SwDocShell *pTmpDocShell = 0;
    Reference < XUnoTunnel > xTunnel( xTmpModel, UNO_QUERY );
    if (xTunnel.is())
    {
        SwXTextDocument* pTextDoc = reinterpret_cast<SwXTextDocument *>(
                xTunnel->getSomething( SwXTextDocument::getUnoTunnelId() ));
        pTmpDocShell = pTextDoc ? pTextDoc->GetDocShell() : 0;
    }

    bool bRes = false;
    if (xTmpModel.is() && pTmpDocShell)    
    {
        if (bClose)
            CloseModelAndDocSh( rxModel, rxDocSh );
        
        rxModel = xTmpModel;
        rxDocSh = pTmpDocShell;
        bRes = true;
    }
    else
    {
        
        SfxObjectShellRef xTmpDocSh = pTmpDocShell;
        CloseModelAndDocSh( xTmpModel, xTmpDocSh );
    }

    return bRes;
}

namespace
{
    class DelayedFileDeletion : public ::cppu::WeakImplHelper1< util::XCloseListener >
    {
    protected:
        ::osl::Mutex                    m_aMutex;
        Reference< util::XCloseable >   m_xDocument;
        Timer                           m_aDeleteTimer;
        OUString                        m_sTemporaryFile;
        sal_Int32                       m_nPendingDeleteAttempts;

    public:
        DelayedFileDeletion( const Reference< XModel >& _rxModel,
                             const OUString& _rTemporaryFile );

    protected:
        ~DelayedFileDeletion( );

        
        virtual void SAL_CALL queryClosing( const EventObject& _rSource, sal_Bool _bGetsOwnership ) throw (util::CloseVetoException, RuntimeException);
        virtual void SAL_CALL notifyClosing( const EventObject& _rSource ) throw (RuntimeException);

        
        virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);

    private:
        void implTakeOwnership( );
        DECL_LINK( OnTryDeleteFile, void* );

    private:
        DelayedFileDeletion( const DelayedFileDeletion& );                  
        DelayedFileDeletion& operator=( const DelayedFileDeletion& );       
    };

    DelayedFileDeletion::DelayedFileDeletion( const Reference< XModel >& _rxModel, const OUString& _rTemporaryFile )
        :
        m_xDocument( _rxModel, UNO_QUERY )
        ,m_sTemporaryFile( _rTemporaryFile )
        ,m_nPendingDeleteAttempts( 0 )
    {
        osl_atomic_increment( &m_refCount );
        try
        {
            if ( m_xDocument.is() )
            {
                m_xDocument->addCloseListener( this );
                
                acquire();
            }
            else {
                OSL_FAIL("DelayedFileDeletion::DelayedFileDeletion: model is no component!" );
            }
        }
        catch (const Exception&)
        {
            OSL_FAIL("DelayedFileDeletion::DelayedFileDeletion: could not register as event listener at the model!" );
        }
        osl_atomic_decrement( &m_refCount );
    }

    IMPL_LINK_NOARG(DelayedFileDeletion, OnTryDeleteFile)
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        bool bSuccess = false;
        try
        {
            sal_Bool bDeliverOwnership = ( 0 == m_nPendingDeleteAttempts );
                
                
            m_xDocument->close( bDeliverOwnership );
            bSuccess = true;
        }
        catch (const util::CloseVetoException&)
        {
            
            if ( m_nPendingDeleteAttempts )
            {
                
                --m_nPendingDeleteAttempts;
                m_aDeleteTimer.Start();
            }
            else
                bSuccess = true;    
        }
        catch (const Exception&)
        {
            OSL_FAIL("DelayedFileDeletion::OnTryDeleteFile: caught a strange exception!" );
            bSuccess = true;
                
        }

        if ( bSuccess )
        {
            SWUnoHelper::UCB_DeleteFile( m_sTemporaryFile );
            aGuard.clear();
            release();  
        }
        return 0L;
    }

    void DelayedFileDeletion::implTakeOwnership( )
    {
        
        try
        {
            m_xDocument->removeCloseListener( this );
        }
        catch (const Exception&)
        {
            OSL_FAIL("DelayedFileDeletion::implTakeOwnership: could not revoke the listener!" );
        }

        m_aDeleteTimer.SetTimeout( 3000 );  
        m_aDeleteTimer.SetTimeoutHdl( LINK( this, DelayedFileDeletion, OnTryDeleteFile ) );
        m_nPendingDeleteAttempts = 3;   
        m_aDeleteTimer.Start( );
    }

    void SAL_CALL DelayedFileDeletion::queryClosing( const EventObject& , sal_Bool _bGetsOwnership ) throw (util::CloseVetoException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( _bGetsOwnership )
            implTakeOwnership( );

        
        
        throw util::CloseVetoException( );
    }

    void SAL_CALL DelayedFileDeletion::notifyClosing( const EventObject&  ) throw (RuntimeException)
    {
        OSL_FAIL("DelayedFileDeletion::notifyClosing: how this?" );
        
        
        
    }

    void SAL_CALL DelayedFileDeletion::disposing( const EventObject&  ) throw (RuntimeException)
    {
        OSL_FAIL("DelayedFileDeletion::disposing: how this?" );
        
        
        
    }

    DelayedFileDeletion::~DelayedFileDeletion( )
    {
    }
}

static bool DeleteTmpFile_Impl(
        Reference< frame::XModel > &rxModel,
        SfxObjectShellRef &rxDocSh,
        const OUString &rTmpFileURL )
{
    bool bRes = false;
    if (!rTmpFileURL.isEmpty())
    {
        bool bDelete = true;
        if ( eVetoed == CloseModelAndDocSh( rxModel, rxDocSh ) )
        {
            
            
            Reference< XEventListener > xEnsureDelete( new DelayedFileDeletion( rxModel, rTmpFileURL ) );
                
                
            bDelete = false;
        }

        rxModel = 0;
        rxDocSh = 0; 

        if ( bDelete )
        {
            if ( !SWUnoHelper::UCB_DeleteFile( rTmpFileURL ) )
            {
                Reference< XEventListener > xEnsureDelete( new DelayedFileDeletion( rxModel, rTmpFileURL ) );
                    
            }
        }
        else
            bRes = true;    
    }
    return bRes;
}

SwXMailMerge::SwXMailMerge() :
    aEvtListeners   ( GetMailMergeMutex() ),
    aMergeListeners ( GetMailMergeMutex() ),
    aPropListeners  ( GetMailMergeMutex() ),
    pPropSet( aSwMapProvider.GetPropertySet( PROPERTY_MAP_MAILMERGE ) ),
    nDataCommandType(sdb::CommandType::TABLE),
    nOutputType(MailMergeType::PRINTER),
    bEscapeProcessing(sal_True),     
    bSinglePrintJobs(sal_False),
    bFileNameFromColumn(sal_False),
    bSendAsHTML(sal_False),
    bSendAsAttachment(sal_False),
    bSaveAsSingleFile(sal_False),
    bDisposing(sal_False),
    m_pMgr(0)
{
    
    
    xDocSh = new SwDocShell( SFX_CREATE_MODE_STANDARD );
    xDocSh->DoInitNew( 0 );
    SfxViewFrame *pFrame = SfxViewFrame::LoadHiddenDocument( *xDocSh, 0 );
    SwView *pView = (SwView*) pFrame->GetViewShell();
    pView->AttrChangedNotify( &pView->GetWrtShell() ); 
    xModel = xDocSh->GetModel();
}

SwXMailMerge::~SwXMailMerge()
{
    if (!aTmpFileName.isEmpty())
        DeleteTmpFile_Impl( xModel, xDocSh, aTmpFileName );
    else    
    {
        
        
        
        if ( eVetoed == CloseModelAndDocSh( xModel, xDocSh ) )
            OSL_FAIL("owner ship transfered to vetoing object!" );

        xModel = 0;
        xDocSh = 0; 
    }
}


class MailMergeExecuteFinalizer {
public:
    MailMergeExecuteFinalizer(SwXMailMerge *mailmerge) {
        OSL_ENSURE( mailmerge, "mailmerge object missing" );
        this->m_aMailMerge = mailmerge;
    }
    ~MailMergeExecuteFinalizer() {
        osl::MutexGuard pMgrGuard( GetMailMergeMutex() );
        m_aMailMerge->m_pMgr = 0;
    }

private:
    
    MailMergeExecuteFinalizer(const MailMergeExecuteFinalizer&) {}

    SwXMailMerge *m_aMailMerge;
};

uno::Any SAL_CALL SwXMailMerge::execute(
        const uno::Sequence< beans::NamedValue >& rArguments )
    throw (IllegalArgumentException, Exception, RuntimeException)
{
    SolarMutexGuard aGuard;
    MailMergeExecuteFinalizer aFinalizer(this);

    
    
    

    uno::Sequence< uno::Any >           aCurSelection   = aSelection;
    uno::Reference< sdbc::XResultSet >  xCurResultSet   = xResultSet;
    uno::Reference< sdbc::XConnection > xCurConnection  = xConnection;
    uno::Reference< frame::XModel >     xCurModel       = xModel;
    OUString   aCurDataSourceName       = aDataSourceName;
    OUString   aCurDataCommand          = aDataCommand;
    OUString   aCurFilter               = aFilter;
    OUString   aCurDocumentURL          = aDocumentURL;
    OUString   aCurOutputURL            = aOutputURL;
    OUString   aCurFileNamePrefix       = aFileNamePrefix;
    sal_Int32  nCurDataCommandType      = nDataCommandType;
    sal_Int16  nCurOutputType           = nOutputType;
    sal_Bool   bCurEscapeProcessing     = bEscapeProcessing;
    sal_Bool   bCurSinglePrintJobs      = bSinglePrintJobs;
    sal_Bool   bCurFileNameFromColumn   = bFileNameFromColumn;

    SfxObjectShellRef xCurDocSh = xDocSh;   

    const beans::NamedValue *pArguments = rArguments.getConstArray();
    sal_Int32 nArgs = rArguments.getLength();
    for (sal_Int32 i = 0;  i < nArgs;  ++i)
    {
        const OUString &rName   = pArguments[i].Name;
        const Any &rValue       = pArguments[i].Value;

        sal_Bool bOK = sal_True;
        if (rName == UNO_NAME_SELECTION)
            bOK = rValue >>= aCurSelection;
        else if (rName == UNO_NAME_RESULT_SET)
            bOK = rValue >>= xCurResultSet;
        else if (rName == UNO_NAME_CONNECTION)
            bOK = rValue >>= xCurConnection;
        else if (rName == UNO_NAME_MODEL)
            throw PropertyVetoException("Property is read-only: " + rName, static_cast < cppu::OWeakObject * > ( this ) );
        else if (rName == UNO_NAME_DATA_SOURCE_NAME)
            bOK = rValue >>= aCurDataSourceName;
        else if (rName == UNO_NAME_DAD_COMMAND)
            bOK = rValue >>= aCurDataCommand;
        else if (rName == UNO_NAME_FILTER)
            bOK = rValue >>= aCurFilter;
        else if (rName == UNO_NAME_DOCUMENT_URL)
        {
            bOK = rValue >>= aCurDocumentURL;
            if (!aCurDocumentURL.isEmpty()
                && !LoadFromURL_impl( xCurModel, xCurDocSh, aCurDocumentURL, false ))
                throw RuntimeException("Failed to create document from URL: " + aCurDocumentURL, static_cast < cppu::OWeakObject * > ( this ) );
        }
        else if (rName == UNO_NAME_OUTPUT_URL)
        {
            bOK = rValue >>= aCurOutputURL;
            if (!aCurOutputURL.isEmpty())
            {
                if (!UCB_IsDirectory(aCurOutputURL))
                    throw IllegalArgumentException("URL does not point to a directory: " + aCurOutputURL, static_cast < cppu::OWeakObject * > ( this ), 0 );
                if (UCB_IsReadOnlyFileName(aCurOutputURL))
                    throw IllegalArgumentException("URL is read-only: " + aCurOutputURL, static_cast < cppu::OWeakObject * > ( this ), 0 );
            }
        }
        else if (rName == UNO_NAME_FILE_NAME_PREFIX)
            bOK = rValue >>= aCurFileNamePrefix;
        else if (rName == UNO_NAME_DAD_COMMAND_TYPE)
            bOK = rValue >>= nCurDataCommandType;
        else if (rName == UNO_NAME_OUTPUT_TYPE)
            bOK = rValue >>= nCurOutputType;
        else if (rName == UNO_NAME_ESCAPE_PROCESSING)
            bOK = rValue >>= bCurEscapeProcessing;
        else if (rName == UNO_NAME_SINGLE_PRINT_JOBS)
            bOK = rValue >>= bCurSinglePrintJobs;
        else if (rName == UNO_NAME_FILE_NAME_FROM_COLUMN)
            bOK = rValue >>= bCurFileNameFromColumn;
        else if (rName == UNO_NAME_SUBJECT)
            bOK = rValue >>= sSubject;
        else if (rName == UNO_NAME_ADDRESS_FROM_COLUMN)
            bOK = rValue >>= sAddressFromColumn;
        else if (rName == UNO_NAME_SEND_AS_HTML)
            bOK = rValue >>= bSendAsHTML;
        else if (rName == UNO_NAME_MAIL_BODY)
            bOK = rValue >>= sMailBody;
        else if (rName == UNO_NAME_ATTACHMENT_NAME)
            bOK = rValue >>= sAttachmentName;
        else if (rName == UNO_NAME_ATTACHMENT_FILTER)
            bOK = rValue >>= sAttachmentFilter;
        else if (rName == UNO_NAME_COPIES_TO)
            bOK = rValue >>= aCopiesTo;
        else if (rName == UNO_NAME_BLIND_COPIES_TO)
            bOK = rValue >>= aBlindCopiesTo;
        else if (rName == UNO_NAME_SEND_AS_ATTACHMENT)
            bOK = rValue >>= bSendAsAttachment;
        else if (rName == UNO_NAME_PRINT_OPTIONS)
            bOK = rValue >>= aPrintSettings;
        else if (rName == UNO_NAME_SAVE_AS_SINGLE_FILE)
            bOK = rValue >>= bSaveAsSingleFile;
        else if (rName == UNO_NAME_SAVE_FILTER)
            bOK = rValue >>= sSaveFilter;
        else if (rName == UNO_NAME_SAVE_FILTER_OPTIONS)
            bOK = rValue >>= sSaveFilterOptions;
        else if (rName == UNO_NAME_SAVE_FILTER_DATA)
            bOK = rValue >>= aSaveFilterData;
        else if (rName == UNO_NAME_IN_SERVER_PASSWORD)
            bOK = rValue >>= sInServerPassword;
        else if (rName == UNO_NAME_OUT_SERVER_PASSWORD)
            bOK = rValue >>= sOutServerPassword;
        else
            throw UnknownPropertyException( OUString( "Property is unknown: " ) + rName, static_cast < cppu::OWeakObject * > ( this ) );

        if (!bOK)
            throw IllegalArgumentException("Property type mismatch or property not set: " + rName, static_cast < cppu::OWeakObject * > ( this ), 0 );
    }

    
    
    if ( aCurSelection.getLength() )
    {
        Sequence< Any > aTranslated( aCurSelection.getLength() );

        bool bValid = false;
        Reference< sdbcx::XRowLocate > xRowLocate( xCurResultSet, UNO_QUERY );
        if ( xRowLocate.is() )
        {

            const Any* pBookmarks = aCurSelection.getConstArray();
            const Any* pBookmarksEnd = pBookmarks + aCurSelection.getLength();
            Any* pTranslated = aTranslated.getArray();

            try
            {
                bool bEverythingsFine = true;
                for ( ; ( pBookmarks != pBookmarksEnd ) && bEverythingsFine; ++pBookmarks )
                {
                    if ( xRowLocate->moveToBookmark( *pBookmarks ) )
                        *pTranslated <<= xCurResultSet->getRow();
                    else
                        bEverythingsFine = false;
                }
                if ( bEverythingsFine )
                    bValid = true;
            }
            catch (const Exception&)
            {
                bValid = false;
            }
        }

        if ( !bValid )
        {
            throw IllegalArgumentException(
                OUString( "The current 'Selection' does not describe a valid array of bookmarks, relative to the current 'ResultSet'." ),
                static_cast < cppu::OWeakObject * > ( this ),
                0
            );
        }

        aCurSelection = aTranslated;
    }

    SfxViewFrame*   pFrame = SfxViewFrame::GetFirst( xCurDocSh, sal_False);
    SwView *pView = PTR_CAST( SwView, pFrame->GetViewShell() );
    if (!pView)
        throw RuntimeException();
    SwWrtShell &rSh = *pView->GetWrtShellPtr();

    
    
    
    
    if (!aCurDocumentURL.isEmpty())
        pView->AttrChangedNotify( &pView->GetWrtShell() );

    SharedComponent aRowSetDisposeHelper;
    if (!xCurResultSet.is())
    {
        if (aCurDataSourceName.isEmpty() || aCurDataCommand.isEmpty() )
        {
            OSL_FAIL("PropertyValues missing or unset");
            throw IllegalArgumentException("Either the ResultSet or DataSourceName and DataCommand must be set.", static_cast < cppu::OWeakObject * > ( this ), 0 );
        }

        //
        
        //
        Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
        if (xMgr.is())
        {
            Reference< XInterface > xInstance = xMgr->createInstance( "com.sun.star.sdb.RowSet" );
            aRowSetDisposeHelper.reset( xInstance, SharedComponent::TakeOwnership );
            Reference< XPropertySet > xRowSetPropSet( xInstance, UNO_QUERY );
            OSL_ENSURE( xRowSetPropSet.is(), "failed to get XPropertySet interface from RowSet" );
            if (xRowSetPropSet.is())
            {
                if (xCurConnection.is())
                    xRowSetPropSet->setPropertyValue( "ActiveConnection",  makeAny( xCurConnection ) );
                xRowSetPropSet->setPropertyValue( "DataSourceName",    makeAny( aCurDataSourceName ) );
                xRowSetPropSet->setPropertyValue( "Command",           makeAny( aCurDataCommand ) );
                xRowSetPropSet->setPropertyValue( "CommandType",       makeAny( nCurDataCommandType ) );
                xRowSetPropSet->setPropertyValue( "EscapeProcessing",  makeAny( bCurEscapeProcessing ) );
                xRowSetPropSet->setPropertyValue( "ApplyFilter",       makeAny( sal_True ) );
                xRowSetPropSet->setPropertyValue( "Filter",            makeAny( aCurFilter ) );

                Reference< sdbc::XRowSet > xRowSet( xInstance, UNO_QUERY );
                if (xRowSet.is())
                    xRowSet->execute(); 
                if( !xCurConnection.is() )
                    xCurConnection.set( xRowSetPropSet->getPropertyValue( "ActiveConnection" ), UNO_QUERY );
                xCurResultSet = Reference< sdbc::XResultSet >( xRowSet, UNO_QUERY );
                OSL_ENSURE( xCurResultSet.is(), "failed to build ResultSet" );
            }
        }
    }

    svx::ODataAccessDescriptor aDescriptor;
    aDescriptor.setDataSource(aCurDataSourceName);
    aDescriptor[ svx::daConnection ]         <<= xCurConnection;
    aDescriptor[ svx::daCommand ]            <<= aCurDataCommand;
    aDescriptor[ svx::daCommandType ]        <<= nCurDataCommandType;
    aDescriptor[ svx::daEscapeProcessing ]   <<= bCurEscapeProcessing;
    aDescriptor[ svx::daCursor ]             <<= xCurResultSet;
    
    
    aDescriptor[ svx::daSelection ]          <<= aCurSelection;

    sal_uInt16 nMergeType;
    switch (nCurOutputType)
    {
        case MailMergeType::PRINTER : nMergeType = DBMGR_MERGE_MAILMERGE; break;
        case MailMergeType::FILE    : nMergeType = DBMGR_MERGE_MAILFILES; break;
        case MailMergeType::MAIL    : nMergeType = DBMGR_MERGE_MAILING; break;
        default:
            throw IllegalArgumentException("Invalid value of property: OutputType", static_cast < cppu::OWeakObject * > ( this ), 0 );
    }

    SwNewDBMgr* pMgr = rSh.GetNewDBMgr();
    
    rSh.CalcLayout();
    OSL_ENSURE( pMgr, "database manager missing" );
    m_pMgr = pMgr;

    SwMergeDescriptor aMergeDesc( nMergeType, rSh, aDescriptor );

    boost::scoped_ptr< SwMailMergeConfigItem > pMMConfigItem;
    uno::Reference< mail::XMailService > xInService;
    if (MailMergeType::PRINTER == nCurOutputType)
    {
        IDocumentDeviceAccess* pIDDA = rSh.getIDocumentDeviceAccess();
        SwPrintData aPrtData( pIDDA->getPrintData() );
        aPrtData.SetPrintSingleJobs( bCurSinglePrintJobs );
        pIDDA->setPrintData( aPrtData );
        
        
        aMergeDesc.bPrintAsync = sal_False;
        aMergeDesc.aPrintOptions = aPrintSettings;
        aMergeDesc.bCreateSingleFile = true;
    }
    else /* FILE and MAIL*/
    {
        INetURLObject aURLObj;
        aURLObj.SetSmartProtocol( INET_PROT_FILE );

        if (!aCurDocumentURL.isEmpty())
        {
            
            
            aURLObj.SetSmartURL( aCurDocumentURL );
            if (aCurFileNamePrefix.isEmpty())
                aCurFileNamePrefix = aURLObj.GetBase(); 
            if (aCurOutputURL.isEmpty())
            {
                aURLObj.removeSegment();
                aCurOutputURL = aURLObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
            }
        }
        else    
        {
            if (aCurOutputURL.isEmpty())
                throw RuntimeException("OutputURL is not set and can not be obtained.", static_cast < cppu::OWeakObject * > ( this ) );
        }

        aURLObj.SetSmartURL( aCurOutputURL );
        OUString aPath = aURLObj.GetMainURL( INetURLObject::DECODE_TO_IURI );

        OUString aDelim = OUString(INET_PATH_TOKEN);
        if (aPath.getLength() >= aDelim.getLength() &&
            aPath.copy( aPath.getLength() - aDelim.getLength() ) == aDelim)
            aPath += aDelim;
        if (bCurFileNameFromColumn)
            pMgr->SetEMailColumn( aCurFileNamePrefix );
        else
        {
            aPath += aCurFileNamePrefix;
            pMgr->SetEMailColumn( OUString() );
        }
        pMgr->SetSubject( aPath );
        if(MailMergeType::FILE == nCurOutputType)
        {
            aMergeDesc.sSaveToFilter = sSaveFilter;
            aMergeDesc.sSaveToFilterOptions = sSaveFilterOptions;
            aMergeDesc.aSaveToFilterData = aSaveFilterData;
            aMergeDesc.bCreateSingleFile = bSaveAsSingleFile;
        }
        else
        {
            pMgr->SetEMailColumn( sAddressFromColumn );
            if(sAddressFromColumn.isEmpty())
                throw RuntimeException("Mail address column not set.", static_cast < cppu::OWeakObject * > ( this ) );
            aMergeDesc.sSaveToFilter     = sAttachmentFilter;
            aMergeDesc.sSubject          = sSubject;
            aMergeDesc.sMailBody         = sMailBody;
            aMergeDesc.sAttachmentName   = sAttachmentName;
            aMergeDesc.aCopiesTo         = aCopiesTo;
            aMergeDesc.aBlindCopiesTo    = aBlindCopiesTo;
            aMergeDesc.bSendAsHTML       = bSendAsHTML;
            aMergeDesc.bSendAsAttachment = bSendAsAttachment;

            aMergeDesc.bCreateSingleFile = sal_False;
            pMMConfigItem.reset(new SwMailMergeConfigItem);
            aMergeDesc.pMailMergeConfigItem = pMMConfigItem.get();
            aMergeDesc.xSmtpServer = SwMailMergeHelper::ConnectToSmtpServer(
                    *pMMConfigItem,
                    xInService,
                    sInServerPassword, sOutServerPassword );
            if( !aMergeDesc.xSmtpServer.is() || !aMergeDesc.xSmtpServer->isConnected())
                throw RuntimeException("Failed to connect to mail server.", static_cast < cppu::OWeakObject * > ( this ) );
        }
    }

    
    const SfxFilter *pSfxFlt = SwIoSystem::GetFilterOfFormat(
            OUString( FILTER_XML ),
            SwDocShell::Factory().GetFilterContainer() );
    OUString aExtension(comphelper::string::stripStart(pSfxFlt->GetDefaultExtension(), '*'));
    utl::TempFile aTempFile( OUString("SwMM"), &aExtension );
    aTmpFileName = aTempFile.GetURL();

    Reference< XStorable > xStorable( xCurModel, UNO_QUERY );
    bool bStoredAsTemporary = false;
    if ( xStorable.is() )
    {
        try
        {
            xStorable->storeAsURL( aTmpFileName, Sequence< PropertyValue >() );
            bStoredAsTemporary = true;
        }
        catch (const Exception&)
        {
        }
    }
    if ( !bStoredAsTemporary )
        throw RuntimeException("Failed to save temporary file.", static_cast < cppu::OWeakObject * > ( this ) );

    pMgr->SetMergeSilent( sal_True );       
    const SwXMailMerge *pOldSrc = pMgr->GetMailMergeEvtSrc();
    OSL_ENSURE( !pOldSrc || pOldSrc == this, "Ooops... different event source already set." );
    pMgr->SetMailMergeEvtSrc( this );   

    SFX_APP()->NotifyEvent(SfxEventHint(SW_EVENT_MAIL_MERGE, SwDocShell::GetEventName(STR_SW_EVENT_MAIL_MERGE), xCurDocSh));
    sal_Bool bSucc = pMgr->MergeNew( aMergeDesc );
    SFX_APP()->NotifyEvent(SfxEventHint(SW_EVENT_MAIL_MERGE_END, SwDocShell::GetEventName(STR_SW_EVENT_MAIL_MERGE_END), xCurDocSh));

    pMgr->SetMailMergeEvtSrc( pOldSrc );

    if ( xCurModel.get() != xModel.get() )
    {   
        DeleteTmpFile_Impl( xCurModel, xCurDocSh, aTmpFileName );
        aTmpFileName = "";
    }
    

    if (!bSucc)
        throw Exception("Mail merge failed. Sorry, no further information available.", static_cast < cppu::OWeakObject * > ( this ) );

    
    if(xInService.is() && xInService->isConnected())
        xInService->disconnect();
    if(aMergeDesc.xSmtpServer.is() && aMergeDesc.xSmtpServer->isConnected())
        aMergeDesc.xSmtpServer->disconnect();

    return makeAny( sal_True );
}

void SAL_CALL SwXMailMerge::cancel() throw (com::sun::star::uno::RuntimeException)
{
    
    
    osl::MutexGuard pMgrGuard( GetMailMergeMutex() );
    if (m_pMgr)
        m_pMgr->MergeCancel();
}

void SwXMailMerge::LaunchMailMergeEvent( const MailMergeEvent &rEvt ) const
{
    cppu::OInterfaceIteratorHelper aIt( ((SwXMailMerge *) this)->aMergeListeners );
    while (aIt.hasMoreElements())
    {
        Reference< XMailMergeListener > xRef( aIt.next(), UNO_QUERY );
        if (xRef.is())
            xRef->notifyMailMergeEvent( rEvt );
    }
}

void SwXMailMerge::launchEvent( const PropertyChangeEvent &rEvt ) const
{
    cppu::OInterfaceContainerHelper *pContainer =
            aPropListeners.getContainer( rEvt.PropertyHandle );
    if (pContainer)
    {
        cppu::OInterfaceIteratorHelper aIt( *pContainer );
        while (aIt.hasMoreElements())
        {
            Reference< XPropertyChangeListener > xRef( aIt.next(), UNO_QUERY );
            if (xRef.is())
                xRef->propertyChange( rEvt );
        }
    }
}


uno::Reference< beans::XPropertySetInfo > SAL_CALL SwXMailMerge::getPropertySetInfo(  )
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    static Reference< XPropertySetInfo > aRef = pPropSet->getPropertySetInfo();
    return aRef;
}

void SAL_CALL SwXMailMerge::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue )
    throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertySimpleEntry* pCur = pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pCur)
        throw UnknownPropertyException();
    else if (pCur->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException();
    else
    {
        void *pData = NULL;
        switch (pCur->nWID)
        {
            case WID_SELECTION :                pData = &aSelection;  break;
            case WID_RESULT_SET :               pData = &xResultSet;  break;
            case WID_CONNECTION :               pData = &xConnection;  break;
            case WID_MODEL :                    pData = &xModel;  break;
            case WID_DATA_SOURCE_NAME :         pData = &aDataSourceName;  break;
            case WID_DATA_COMMAND :             pData = &aDataCommand;  break;
            case WID_FILTER :                   pData = &aFilter;  break;
            case WID_DOCUMENT_URL :             pData = &aDocumentURL;  break;
            case WID_OUTPUT_URL :               pData = &aOutputURL;  break;
            case WID_DATA_COMMAND_TYPE :        pData = &nDataCommandType;  break;
            case WID_OUTPUT_TYPE :              pData = &nOutputType;  break;
            case WID_ESCAPE_PROCESSING :        pData = &bEscapeProcessing;  break;
            case WID_SINGLE_PRINT_JOBS :        pData = &bSinglePrintJobs;  break;
            case WID_FILE_NAME_FROM_COLUMN :    pData = &bFileNameFromColumn;  break;
            case WID_FILE_NAME_PREFIX :         pData = &aFileNamePrefix;  break;
            case WID_MAIL_SUBJECT:              pData = &sSubject; break;
            case WID_ADDRESS_FROM_COLUMN:       pData = &sAddressFromColumn; break;
            case WID_SEND_AS_HTML:              pData = &bSendAsHTML; break;
            case WID_SEND_AS_ATTACHMENT:        pData = &bSendAsAttachment; break;
            case WID_MAIL_BODY:                 pData = &sMailBody; break;
            case WID_ATTACHMENT_NAME:           pData = &sAttachmentName; break;
            case WID_ATTACHMENT_FILTER:         pData = &sAttachmentFilter;break;
            case WID_PRINT_OPTIONS:             pData = &aPrintSettings; break;
            case WID_SAVE_AS_SINGLE_FILE:       pData = &bSaveAsSingleFile; break;
            case WID_SAVE_FILTER:               pData = &sSaveFilter; break;
            case WID_SAVE_FILTER_OPTIONS:       pData = &sSaveFilterOptions; break;
            case WID_SAVE_FILTER_DATA:          pData = &aSaveFilterData; break;
            case WID_COPIES_TO:                 pData = &aCopiesTo; break;
            case WID_BLIND_COPIES_TO:           pData = &aBlindCopiesTo;break;
            case WID_IN_SERVER_PASSWORD:        pData = &sInServerPassword; break;
            case WID_OUT_SERVER_PASSWORD:       pData = &sOutServerPassword; break;
            default :
                OSL_FAIL("unknown WID");
        }
        Any aOld( pData, pCur->aType );

        bool bChanged = false;
        sal_Bool bOK = sal_True;
        if (aOld != rValue)
        {
            if (pData == &aSelection)
                bOK = rValue >>= aSelection;
            else if (pData == &xResultSet)
                bOK = rValue >>= xResultSet;
            else if (pData == &xConnection)
                bOK = rValue >>= xConnection;
            else if (pData == &xModel)
                bOK = rValue >>= xModel;
            else if (pData == &aDataSourceName)
                bOK = rValue >>= aDataSourceName;
            else if (pData == &aDataCommand)
                bOK = rValue >>= aDataCommand;
            else if (pData == &aFilter)
                bOK = rValue >>= aFilter;
            else if (pData == &aDocumentURL)
            {
                OUString aText;
                bOK = rValue >>= aText;
                if (!aText.isEmpty()
                    && !LoadFromURL_impl( xModel, xDocSh, aText, true ))
                    throw RuntimeException("Failed to create document from URL: " + aText, static_cast < cppu::OWeakObject * > ( this ) );
                aDocumentURL = aText;
            }
            else if (pData == &aOutputURL)
            {
                OUString aText;
                bOK = rValue >>= aText;
                if (!aText.isEmpty())
                {
                    if (!UCB_IsDirectory(aText))
                        throw IllegalArgumentException("URL does not point to a directory: " + aText, static_cast < cppu::OWeakObject * > ( this ), 0 );
                    if (UCB_IsReadOnlyFileName(aText))
                        throw IllegalArgumentException("URL is read-only: " + aText, static_cast < cppu::OWeakObject * > ( this ), 0 );
                }
                aOutputURL = aText;
            }
            else if (pData == &nDataCommandType)
                bOK = rValue >>= nDataCommandType;
            else if (pData == &nOutputType)
                bOK = rValue >>= nOutputType;
            else if (pData == &bEscapeProcessing)
                bOK = rValue >>= bEscapeProcessing;
            else if (pData == &bSinglePrintJobs)
                bOK = rValue >>= bSinglePrintJobs;
            else if (pData == &bFileNameFromColumn)
                bOK = rValue >>= bFileNameFromColumn;
            else if (pData == &aFileNamePrefix)
                bOK = rValue >>= aFileNamePrefix;
            else if (pData == &sSubject)
                bOK = rValue >>= sSubject;
            else if (pData == &sAddressFromColumn)
                bOK = rValue >>= sAddressFromColumn;
            else if (pData == &bSendAsHTML)
                bOK = rValue >>= bSendAsHTML;
            else if (pData == &bSendAsAttachment)
                bOK = rValue >>= bSendAsAttachment;
            else if (pData == &sMailBody)
                bOK = rValue >>= sMailBody;
            else if (pData == &sAttachmentName)
                bOK = rValue >>= sAttachmentName;
            else if (pData == &sAttachmentFilter)
                bOK = rValue >>= sAttachmentFilter;
            else if (pData == &aPrintSettings)
                bOK = rValue >>= aPrintSettings;
            else if (pData == &bSaveAsSingleFile)
                bOK = rValue >>= bSaveAsSingleFile;
            else if (pData == &sSaveFilter)
                bOK = rValue >>= sSaveFilter;
            else if (pData == &sSaveFilterOptions)
                bOK = rValue >>= sSaveFilterOptions;
            else if (pData == &aSaveFilterData)
                bOK = rValue >>= aSaveFilterData;
            else if (pData == &aCopiesTo)
                bOK = rValue >>= aCopiesTo;
            else if (pData == &aBlindCopiesTo)
                bOK = rValue >>= aBlindCopiesTo;
            else if(pData == &sInServerPassword)
                bOK = rValue >>= sInServerPassword;
            else if(pData == &sOutServerPassword)
                bOK = rValue >>= sInServerPassword;
            else {
                OSL_FAIL("invalid pointer" );
            }
            OSL_ENSURE( bOK, "set value failed" );
            bChanged = true;
        }
        if (!bOK)
            throw IllegalArgumentException("Property type mismatch or property not set: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ), 0 );

        if (bChanged)
        {
            PropertyChangeEvent aChgEvt( (XPropertySet *) this, rPropertyName,
                    sal_False, pCur->nWID, aOld, rValue );
            launchEvent( aChgEvt );
        }
    }
}

uno::Any SAL_CALL SwXMailMerge::getPropertyValue(
        const OUString& rPropertyName )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    Any aRet;

    const SfxItemPropertySimpleEntry* pCur = pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pCur)
        throw UnknownPropertyException();
    else
    {
        switch (pCur->nWID)
        {
            case WID_SELECTION :                aRet <<= aSelection;  break;
            case WID_RESULT_SET :               aRet <<= xResultSet;  break;
            case WID_CONNECTION :               aRet <<= xConnection;  break;
            case WID_MODEL :                    aRet <<= xModel;  break;
            case WID_DATA_SOURCE_NAME :         aRet <<= aDataSourceName;  break;
            case WID_DATA_COMMAND :             aRet <<= aDataCommand;  break;
            case WID_FILTER :                   aRet <<= aFilter;  break;
            case WID_DOCUMENT_URL :             aRet <<= aDocumentURL;  break;
            case WID_OUTPUT_URL :               aRet <<= aOutputURL;  break;
            case WID_DATA_COMMAND_TYPE :        aRet <<= nDataCommandType;  break;
            case WID_OUTPUT_TYPE :              aRet <<= nOutputType;  break;
            case WID_ESCAPE_PROCESSING :        aRet <<= bEscapeProcessing;  break;
            case WID_SINGLE_PRINT_JOBS :        aRet <<= bSinglePrintJobs;  break;
            case WID_FILE_NAME_FROM_COLUMN :    aRet <<= bFileNameFromColumn;  break;
            case WID_FILE_NAME_PREFIX :         aRet <<= aFileNamePrefix;  break;
            case WID_MAIL_SUBJECT:              aRet <<= sSubject; break;
            case WID_ADDRESS_FROM_COLUMN:       aRet <<= sAddressFromColumn; break;
            case WID_SEND_AS_HTML:              aRet <<= bSendAsHTML; break;
            case WID_SEND_AS_ATTACHMENT:        aRet <<= bSendAsAttachment; break;
            case WID_MAIL_BODY:                 aRet <<= sMailBody; break;
            case WID_ATTACHMENT_NAME:           aRet <<= sAttachmentName; break;
            case WID_ATTACHMENT_FILTER:         aRet <<= sAttachmentFilter;break;
            case WID_PRINT_OPTIONS:             aRet <<= aPrintSettings; break;
            case WID_SAVE_AS_SINGLE_FILE:       aRet <<= bSaveAsSingleFile; break;
            case WID_SAVE_FILTER:               aRet <<= sSaveFilter; break;
            case WID_SAVE_FILTER_OPTIONS:       aRet <<= sSaveFilterOptions; break;
            case WID_SAVE_FILTER_DATA:          aRet <<= aSaveFilterData; break;
            case WID_COPIES_TO:                 aRet <<= aCopiesTo; break;
            case WID_BLIND_COPIES_TO:           aRet <<= aBlindCopiesTo;break;
            case WID_IN_SERVER_PASSWORD:        aRet <<= sInServerPassword; break;
            case WID_OUT_SERVER_PASSWORD:       aRet <<= sOutServerPassword; break;
            default :
                OSL_FAIL("unknown WID");
        }
    }

    return aRet;
}

void SAL_CALL SwXMailMerge::addPropertyChangeListener(
        const OUString& rPropertyName,
        const uno::Reference< beans::XPropertyChangeListener >& rListener )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!bDisposing && rListener.is())
    {
        const SfxItemPropertySimpleEntry* pCur = pPropSet->getPropertyMap().getByName( rPropertyName );
        if (pCur)
            aPropListeners.addInterface( pCur->nWID, rListener );
        else
            throw UnknownPropertyException();
    }
}

void SAL_CALL SwXMailMerge::removePropertyChangeListener(
        const OUString& rPropertyName,
        const uno::Reference< beans::XPropertyChangeListener >& rListener )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!bDisposing && rListener.is())
    {
        const SfxItemPropertySimpleEntry* pCur = pPropSet->getPropertyMap().getByName( rPropertyName );
        if (pCur)
            aPropListeners.removeInterface( pCur->nWID, rListener );
        else
            throw UnknownPropertyException();
    }
}

void SAL_CALL SwXMailMerge::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*rListener*/ )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    
    OSL_FAIL("not implemented");
}

void SAL_CALL SwXMailMerge::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*rListener*/ )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    
    OSL_FAIL("not implemented");
}


void SAL_CALL SwXMailMerge::dispose()
    throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!bDisposing)
    {
        bDisposing = sal_True;

        EventObject aEvtObj( (XPropertySet *) this );
        aEvtListeners.disposeAndClear( aEvtObj );
        aMergeListeners.disposeAndClear( aEvtObj );
        aPropListeners.disposeAndClear( aEvtObj );
    }
}

void SAL_CALL SwXMailMerge::addEventListener(
        const Reference< XEventListener >& rxListener )
    throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL SwXMailMerge::removeEventListener(
        const Reference< XEventListener >& rxListener )
    throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}

void SAL_CALL SwXMailMerge::addMailMergeEventListener(
        const uno::Reference< XMailMergeListener >& rxListener )
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!bDisposing && rxListener.is())
        aMergeListeners.addInterface( rxListener );
}

void SAL_CALL SwXMailMerge::removeMailMergeEventListener(
        const uno::Reference< XMailMergeListener >& rxListener )
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!bDisposing && rxListener.is())
        aMergeListeners.removeInterface( rxListener );
}

OUString SAL_CALL SwXMailMerge::getImplementationName()
    throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return SwXMailMerge_getImplementationName();
}

sal_Bool SAL_CALL SwXMailMerge::supportsService( const OUString& rServiceName )
    throw(RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SwXMailMerge::getSupportedServiceNames()
    throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return SwXMailMerge_getSupportedServiceNames();
}

uno::Sequence< OUString > SAL_CALL SwXMailMerge_getSupportedServiceNames()
    throw()
{
    uno::Sequence< OUString > aNames(2);
    OUString *pName = aNames.getArray();
    pName[0] = "com.sun.star.text.MailMerge";
    pName[1] = "com.sun.star.sdb.DataAccessDescriptor";
    return aNames;
}

OUString SAL_CALL SwXMailMerge_getImplementationName()
    throw()
{
    return OUString( "SwXMailMerge" );
}

uno::Reference< uno::XInterface > SAL_CALL SwXMailMerge_createInstance(
        const uno::Reference< XMultiServiceFactory > & /*rSMgr*/)
    throw( uno::Exception )
{
    SolarMutexGuard aGuard;

    
    SwGlobals::ensure();
    uno::Reference< uno::XInterface > xRef = (cppu::OWeakObject *) new SwXMailMerge();
    return xRef;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
