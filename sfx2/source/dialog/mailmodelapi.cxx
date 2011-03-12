/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sfx2.hxx"
// includes --------------------------------------------------------------
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/mozilla/XPluginInstance.hpp>
#include <com/sun/star/ucb/XDataContainer.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/system/XSimpleMailClientSupplier.hpp>
#include <com/sun/star/system/SimpleMailClientFlags.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <rtl/textench.h>
#include <osl/mutex.hxx>
#include <rtl/uri.h>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <unotools/streamhelper.hxx>
#include <unotools/configitem.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <osl/thread.hxx>
#include <vcl/msgbox.hxx>

#include "mailmodelapi.hxx"
#include <sfx2/docfilt.hxx>
#include <sfx2/docfac.hxx>
#include "sfxtypes.hxx"

#include "dialog.hrc"
#include "sfxresid.hxx"
#include <sfx2/sfxuno.hxx>
#include <sfx2/fcontnr.hxx>
#include "guisaveas.hxx"

#include <unotools/tempfile.hxx>
#include <vcl/svapp.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <unotools/useroptions.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/extract.hxx>
#include <ucbhelper/content.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/urlobj.hxx>

extern sal_Bool GetPasswd_Impl( const SfxItemSet* pSet, String& rPasswd );

// --------------------------------------------------------------
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::mozilla;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::system;
using namespace ::rtl;

namespace
{
    // functions -------------------------------------------------------------

    BOOL CreateFromAddress_Impl( String& rFrom )

    /* [Description]

       This function tries to create a From-address with the help of
       IniManagers. For this the fields 'first name', 'Name' and 'Email' are
       read from the  application-ini-data. If these fields are not set,
       FALSE is returned.

       [Return value]

       TRUE:    Address could be created.
       FALSE:   Address could not be created.
    */

    {
        SvtUserOptions aUserCFG;
        String aName        = aUserCFG.GetLastName  ();
        String aFirstName   = aUserCFG.GetFirstName ();
        if ( aFirstName.Len() || aName.Len() )
        {
            if ( aFirstName.Len() )
            {
                rFrom = TRIM( aFirstName );

                if ( aName.Len() )
                    rFrom += ' ';
            }
            rFrom += TRIM( aName );
            // remove illegal characters
            rFrom.EraseAllChars( '<' );
            rFrom.EraseAllChars( '>' );
            rFrom.EraseAllChars( '@' );
        }
        String aEmailName = aUserCFG.GetEmail();
        // remove illegal characters
        aEmailName.EraseAllChars( '<' );
        aEmailName.EraseAllChars( '>' );

        if ( aEmailName.Len() )
        {
            if ( rFrom.Len() )
                rFrom += ' ';
            ( ( rFrom += '<' ) += TRIM( aEmailName ) ) += '>';
        }
        else
            rFrom.Erase();
        return ( rFrom.Len() > 0 );
    }

}

// class osl::Thread
class OMailSendThreadImpl : public ::osl::Thread
{
    public:
        OMailSendThreadImpl(
                Reference< XSimpleMailClient > xSimpleMailClient,
                Reference< XSimpleMailMessage > xSimpleMailMessage,
                const Reference< XFrame >&  _xCurrentFrame,
                long nSendFlags ) :
            m_nSendFlags( nSendFlags ),
            m_bSend( sal_False ),
            m_xSimpleMailClient( xSimpleMailClient ),
            m_xSimpleMailMessage( xSimpleMailMessage ),
            m_xCurrentFrame(_xCurrentFrame) {}

        virtual ~OMailSendThreadImpl();

        virtual void SAL_CALL run();
        virtual void SAL_CALL onTerminated();

    private:
        long        m_nSendFlags;
        sal_Bool    m_bSend;
        Reference< XSimpleMailClient > m_xSimpleMailClient;
        Reference< XSimpleMailMessage > m_xSimpleMailMessage;
        Reference< XFrame > m_xCurrentFrame;
};

OMailSendThreadImpl::~OMailSendThreadImpl()
{
}

void SAL_CALL OMailSendThreadImpl::run()
{
    try
    {
        m_xSimpleMailClient->sendSimpleMailMessage( m_xSimpleMailMessage, m_nSendFlags );
        m_bSend = sal_True;
    }
    catch ( IllegalArgumentException& )
    {
        m_bSend = sal_False;
    }
    catch ( Exception& )
    {
        m_bSend = sal_False;
    }

    if ( m_bSend == sal_False )
    {
        SolarMutexGuard aGuard;
        Reference< ::com::sun::star::awt::XWindow > xParentWindow = m_xCurrentFrame->getContainerWindow();
        Window*                                     pParentWindow = VCLUnoHelper::GetWindow( xParentWindow );

        ErrorBox aBox( pParentWindow, SfxResId( RID_ERRBOX_MAIL_CONFIG ));
        aBox.Execute();
    }
}

void SAL_CALL OMailSendThreadImpl::onTerminated()
{
    delete this;
}

// class SfxMailModel -----------------------------------------------

void SfxMailModel::ClearList( AddressList_Impl* pList )
{
    if ( pList )
    {
        for ( size_t i = 0, nCount = pList->size(); i < nCount; ++i )
            delete pList->at(i);
        pList->clear();
    }
}

void SfxMailModel::MakeValueList( AddressList_Impl* pList, String& rValueList )
{
    rValueList.Erase();
    if ( pList )
    {
        for ( size_t i = 0, nCount = pList->size(); i < nCount; ++i )
        {
            if ( rValueList.Len() > 0 )
                rValueList += ',';
            rValueList += *pList->at(i);
        }
    }
}
namespace
{
    String lcl_getFactoryName(const Reference<XInterface>& _xModel)
    {
        rtl::OUString sModuleIdentifier;
        rtl::OUString sFactoryShortName;
        Reference < XModuleManager > xModuleManager( ::comphelper::getProcessServiceFactory()->createInstance(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.ModuleManager")) ), UNO_QUERY );

        if ( _xModel.is() )
        {
            try
            {
                sModuleIdentifier = xModuleManager->identify( _xModel );
            }
            catch ( ::com::sun::star::frame::UnknownModuleException& )
            {
                DBG_WARNING( "SfxHelp::GetHelpModuleName_Impl(): unknown module (help in help?)" );
            }
            catch ( Exception& )
            {
                DBG_ERRORFILE( "SfxHelp::GetHelpModuleName_Impl(): exception of XModuleManager::identify()" );
            }
        }

        return sModuleIdentifier;
    }

    String lcl_createTempFileName(const Reference < XModel >& _xModel,const ::rtl::OUString& _sAttachmentTitle)
    {
    // create temp file name with leading chars and extension
        Reference < XStorable > xStor( _xModel, UNO_QUERY );
        String      aLeadingStr;
        if ( xStor.is() )
        {
            sal_Bool    bHasName = xStor->hasLocation();

            if ( !bHasName )
            {
                if ( _sAttachmentTitle.getLength() )
                    aLeadingStr = _sAttachmentTitle;
                else
                    aLeadingStr = String( DEFINE_CONST_UNICODE("noname") );
            }
            else
            {
                INetURLObject aFileObj(_xModel->getURL());
                if ( aFileObj.hasExtension() )
                {
                    aFileObj.removeExtension();
                    if ( _sAttachmentTitle.getLength() )
                        aLeadingStr = _sAttachmentTitle;
                    else
                        aLeadingStr = aFileObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
                    aLeadingStr += String::CreateFromAscii( "_" );
                }
                else
                {
                    if ( _sAttachmentTitle.getLength() )
                        aLeadingStr = _sAttachmentTitle;
                    else
                        aLeadingStr = aFileObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
                    aLeadingStr += String::CreateFromAscii( "_" );
                }
            }
        }
        return aLeadingStr;
    }

}
SfxMailModel::SaveResult SfxMailModel::SaveDocument( const ::rtl::OUString& _sAttachmentTitle
                                    , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _xModel
                                    , String& rFileName)
{
    SaveResult          eRet = SAVE_CANCELLED;
    try
    {
        Reference<XModifiable> xMod(_xModel,UNO_QUERY);

        // save the document
        if ( xMod.is() )
        {
            // detect filter
            const SfxFilter* pFilter = SfxFilter::GetDefaultFilter(lcl_getFactoryName(_xModel));

            sal_Bool bRet = sal_False;
            // create temp file name with leading chars and extension
            Reference < XStorable > xStor( _xModel, UNO_QUERY );
            if ( xStor.is() )
            {
                String*     pExt = NULL;
                INetURLObject aFileObj(_xModel->getURL());
                if ( aFileObj.hasExtension() )
                    pExt = new String( String::CreateFromAscii( "." ) + (OUString) aFileObj.getExtension() );

                String aLeadingStr = lcl_createTempFileName(_xModel,_sAttachmentTitle);

                if ( pFilter && !pExt )
                {
                    pExt = new String( pFilter->GetWildcard()().GetToken(0) );
                    // erase the '*' from the extension (e.g. "*.sdw")
                    pExt->Erase( 0, 1 );
                }

                {
                    ::utl::TempFile aTempFile( aLeadingStr, pExt );
                    rFileName = aTempFile.GetURL();
                }
                delete pExt;
                // save document to temp file

                // save document to temp file
                Sequence < PropertyValue > aArgs( _xModel->getArgs() );
                ::comphelper::MediaDescriptor aMedia(aArgs);

                if ( pFilter )
                {
                    ::rtl::OUString sFilter(pFilter->GetFilterName());
                    ::rtl::OUString sFilterName(RTL_CONSTASCII_USTRINGPARAM("FilterName"));
                    if ( !aMedia.createItemIfMissing(sFilterName,sFilter) )
                        aMedia[sFilterName] <<= sFilter;
                }

                ::rtl::OUString sURL(RTL_CONSTASCII_USTRINGPARAM("URL"));
                if ( !aMedia.createItemIfMissing(sURL,::rtl::OUString(rFileName)) )
                    aMedia[sURL] <<= ::rtl::OUString(rFileName);

                ::rtl::OUString sSaveTo(RTL_CONSTASCII_USTRINGPARAM("SaveTo"));
                if ( !aMedia.createItemIfMissing(sSaveTo,sal_True) )
                    aMedia[sSaveTo] <<= sal_True;

                ::rtl::OUString sReadOnly(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
                if ( !aMedia.createItemIfMissing(sReadOnly,sal_False) )
                    aMedia[sReadOnly] <<= sal_False;

                aMedia >> aArgs;

                try
                {
                    Reference < XMultiServiceFactory > xMgr = ::comphelper::getProcessServiceFactory();
                    SfxStoringHelper aHelper(xMgr);
                    aHelper.GUIStoreModel(_xModel,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SaveAs")),aArgs, sal_False, ::rtl::OUString() );
                    bRet = sal_True;
                }
                catch(Exception&)
                {
                }
            }

            eRet = bRet ? SAVE_SUCCESSFULL : SAVE_ERROR;
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("Exception catched!");
    }

    return eRet;
}
SfxMailModel::SaveResult SfxMailModel::SaveDocAsPDF( const ::rtl::OUString& _sAttachmentTitle
                                    , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _xModel
                                    , String& rFileName)
{
    SaveResult eRet = SAVE_CANCELLED;
    try
    {
        Reference<XModifiable> xMod(_xModel,UNO_QUERY);

        // save the document
        if ( xMod.is() )
        {
            // Get PDF Filter from document
            ::rtl::OUString sPDFMediaType(RTL_CONSTASCII_USTRINGPARAM("application/pdf"));

            sal_Bool bRet = sal_False;
            // create temp file name with leading chars and extension
            Reference < XStorable > xStor( _xModel, UNO_QUERY );
            if ( xStor.is() )
            {
                String aLeadingStr = lcl_createTempFileName(_xModel,_sAttachmentTitle);
                String aPDFExtension = String::CreateFromAscii( ".pdf" );
                ::utl::TempFile aTempFile( aLeadingStr, &aPDFExtension );

                rFileName = aTempFile.GetURL();

                // save document to temp file
                Sequence < PropertyValue > aArgs( _xModel->getArgs() );
                ::comphelper::MediaDescriptor aMedia(aArgs);

                ::rtl::OUString sMediaType(RTL_CONSTASCII_USTRINGPARAM("MediaType"));
                if ( !aMedia.createItemIfMissing(sMediaType,sPDFMediaType) )
                    aMedia[sMediaType] <<= sPDFMediaType;

                ::rtl::OUString sURL(RTL_CONSTASCII_USTRINGPARAM("URL"));
                if ( !aMedia.createItemIfMissing(sURL,::rtl::OUString(rFileName)) )
                    aMedia[sURL] <<= ::rtl::OUString(rFileName);
                aMedia >> aArgs;

                try
                {
                    Reference < XMultiServiceFactory > xMgr = ::comphelper::getProcessServiceFactory();
                    SfxStoringHelper aHelper(xMgr);
                    aHelper.GUIStoreModel(_xModel,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ExportToPDF")),aArgs, sal_False, ::rtl::OUString);
                    bRet = sal_True;
                }
                catch(Exception&)
                {
                }

                eRet = bRet ? SAVE_SUCCESSFULL : SAVE_CANCELLED;
            }
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("Exception catched!");
    }

    return eRet;
}
// -----------------------------------------------------------------------------
SfxMailModel::SendMailResult SfxMailModel::AttachDocument(   MailDocType _eMailDocType
                                    , const Reference< XModel >& _xModel
                                    , const ::rtl::OUString& _sAttachmentTitle)
{
    SaveResult eSaveResult;
    String aFileName;

    if ( _eMailDocType == TYPE_SELF )
        eSaveResult = SaveDocument( _sAttachmentTitle,_xModel,aFileName);
    else
        eSaveResult = SaveDocAsPDF( _sAttachmentTitle,_xModel,aFileName);

    if ( eSaveResult == SAVE_SUCCESSFULL && aFileName.Len() )
        maAttachedDocuments.push_back(aFileName);
    return eSaveResult == SAVE_SUCCESSFULL ? SEND_MAIL_OK : SEND_MAIL_ERROR;
}
// -----------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxMailModel, DoneHdl, void*, EMPTYARG )
{
    mbLoadDone = sal_True;
    return 0;
}
// -----------------------------------------------------------------------------
IMPL_LINK_INLINE_END( SfxMailModel, DoneHdl, void*, EMPTYARG )

SfxMailModel::SfxMailModel( const Reference< XFrame >& _xFrame) :

    mpToList    ( NULL ),
    mpCcList    ( NULL ),
    mpBccList   ( NULL ),
    m_xCurrentFrame ( _xFrame ),
    mePriority  ( PRIO_NORMAL ),
    mbLoadDone  ( sal_True )

{
}

SfxMailModel::~SfxMailModel()
{
    ClearList( mpToList );
    delete mpToList;
    ClearList( mpCcList );
    delete mpCcList;
    ClearList( mpBccList );
    delete mpBccList;
}

void SfxMailModel::AddAddress( const String& rAddress, AddressRole eRole )
{
    // don't add a empty address
    if ( rAddress.Len() > 0 )
    {
        AddressList_Impl* pList = NULL;
        if ( ROLE_TO == eRole )
        {
            if ( !mpToList )
                // create the list
                mpToList = new AddressList_Impl();
            pList = mpToList;
        }
        else if ( ROLE_CC == eRole )
        {
            if ( !mpCcList )
                // create the list
                mpCcList = new AddressList_Impl();
            pList = mpCcList;
        }
        else if ( ROLE_BCC == eRole )
        {
            if ( !mpBccList )
                // create the list
                mpBccList = new AddressList_Impl();
            pList = mpBccList;
        }
        else
        {
            DBG_ERRORFILE( "invalid address role" );
        }

        if ( pList )
        {
            // add address to list
            AddressItemPtr_Impl pAddress = new String( rAddress );
            pList->push_back( pAddress );
        }
    }
}

SfxMailModel::SendMailResult SfxMailModel::Send( )
{
    OSL_ENSURE(!maAttachedDocuments.empty(),"No document added!");
    SendMailResult  eResult = SEND_MAIL_ERROR;
    if ( !maAttachedDocuments.empty() )
    {

        Reference < XMultiServiceFactory > xMgr = ::comphelper::getProcessServiceFactory();
        if ( xMgr.is() )
        {
            Reference< XSimpleMailClientSupplier >  xSimpleMailClientSupplier;

            // Prefer the SimpleSystemMail service if available
            xSimpleMailClientSupplier = Reference< XSimpleMailClientSupplier >(
                xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.system.SimpleSystemMail" ))),
                UNO_QUERY );

            if ( ! xSimpleMailClientSupplier.is() )
            {
                xSimpleMailClientSupplier = Reference< XSimpleMailClientSupplier >(
                    xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.system.SimpleCommandMail" ))),
                    UNO_QUERY );
            }

            if ( xSimpleMailClientSupplier.is() )
            {
                Reference< XSimpleMailClient > xSimpleMailClient = xSimpleMailClientSupplier->querySimpleMailClient();

                if ( !xSimpleMailClient.is() )
                {
                    // no mail client support => message box!
                    return SEND_MAIL_ERROR;
                }

                // we have a simple mail client
                Reference< XSimpleMailMessage > xSimpleMailMessage = xSimpleMailClient->createSimpleMailMessage();
                if ( xSimpleMailMessage.is() )
                {
                    sal_Int32 nSendFlags = SimpleMailClientFlags::DEFAULTS;
                    if ( maFromAddress.Len() == 0 )
                    {
                        // from address not set, try figure out users e-mail address
                        CreateFromAddress_Impl( maFromAddress );
                    }
                    xSimpleMailMessage->setOriginator( maFromAddress );

                    sal_Int32 nToCount      = mpToList ? mpToList->Count() : 0;
                    sal_Int32 nCcCount      = mpCcList ? mpCcList->Count() : 0;
                    sal_Int32 nCcSeqCount   = nCcCount;

                    // set recipient (only one) for this simple mail server!!
                    if ( nToCount > 1 )
                    {
                        nCcSeqCount = nToCount - 1 + nCcCount;
                        xSimpleMailMessage->setRecipient( *mpToList->GetObject( 0 ));
                        nSendFlags = SimpleMailClientFlags::NO_USER_INTERFACE;
                    }
                    else if ( nToCount == 1 )
                    {
                        xSimpleMailMessage->setRecipient( *mpToList->GetObject( 0 ));
                        nSendFlags = SimpleMailClientFlags::NO_USER_INTERFACE;
                    }

                    // all other recipient must be handled with CC recipients!
                    if ( nCcSeqCount > 0 )
                    {
                        sal_Int32               nIndex = 0;
                        Sequence< OUString >    aCcRecipientSeq;

                        aCcRecipientSeq.realloc( nCcSeqCount );
                        if ( nCcSeqCount > nCcCount )
                        {
                            for ( sal_Int32 i = 1; i < nToCount; ++i )
                            {
                                aCcRecipientSeq[nIndex++] = *mpToList->GetObject(i);
                            }
                        }

                        for ( sal_Int32 i = 0; i < nCcCount; i++ )
                        {
                            aCcRecipientSeq[nIndex++] = *mpCcList->GetObject(i);
                        }
                        xSimpleMailMessage->setCcRecipient( aCcRecipientSeq );
                    }

                    sal_Int32 nBccCount = mpBccList ? mpBccList->Count() : 0;
                    if ( nBccCount > 0 )
                    {
                        Sequence< OUString > aBccRecipientSeq( nBccCount );
                        for ( sal_Int32 i = 0; i < nBccCount; ++i )
                        {
                            aBccRecipientSeq[i] = *mpBccList->GetObject(i);
                        }
                        xSimpleMailMessage->setBccRecipient( aBccRecipientSeq );
                    }

                    Sequence< OUString > aAttachmentSeq(&(maAttachedDocuments[0]),maAttachedDocuments.size());

                    xSimpleMailMessage->setSubject( maSubject );
                    xSimpleMailMessage->setAttachement( aAttachmentSeq );

                    // Due to the current clipboard implementation we cannot stop the main thread
                    // because the clipboard implementation calls the main thread from another thread
                    // and this would result in a deadlock!
                    // Currently we create a thread to send a message and process all remaining error
                    // handling in this thread!!

                    OMailSendThreadImpl* pMailSendThread = new OMailSendThreadImpl( xSimpleMailClient, xSimpleMailMessage, m_xCurrentFrame,nSendFlags );
                    pMailSendThread->create();

                    // Return always true as the real error handling occurss in the OMailSendThreadImpl-implementation!
                    eResult = SEND_MAIL_OK;
                }
            }
        }
    }
    else
        eResult = SEND_MAIL_CANCELLED;

    return eResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
